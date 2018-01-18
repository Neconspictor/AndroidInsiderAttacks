//
// Created by necon on 08.01.2018.
//


#include <sys/mman.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <sstream>

#include "Hook.h"
#include "Util.h"

using namespace std;
using namespace util;

/**
 * Data is based on https://android.googlesource.com/platform/art/+/oreo-release/runtime/art_method.h
 *  */
struct ArtMethod_x86_AndroidO {
    int declaring_class_ = 0;                       // GcRoot<mirror::Class> declaring_class_;
    int access_flags_ = 4;                          //std::atomic<std::uint32_t> access_flags_;
    int dex_code_item_offset_ = 8 ;                 //uint32_t dex_code_item_offset_;
    int dex_method_index_ = 12;                     // uint32_t dex_method_index_;
    int method_index_ = 16;                         // uint16_t method_index_;
    int hotness_count_ = 18;                        // uint16_t hotness_count_;
    // struct PtrSizedFields {
    int dex_cache_resolved_methods_ = 20;           // mirror::MethodDexCacheType* dex_cache_resolved_methods_; // offset: 20
    int data_ = 24;                                 // void* data_; // offset: 24
    int entry_point_from_quick_compiled_code_ = 28; // void* entry_point_from_quick_compiled_code_; // offset: 28
    //} ptr_sized_fields_;
    size_t artMethodSize = 32;
} ART_METHOD;


// 1. clear hotness_count of the backup ArtMethod(only after Android N)
// 2. set eax/r0/x0 to the hook ArtMethod addr
// 3. jump into its entry point

// b8 21 43 65 87 ; mov eax, 0x87654321 (addr of the backup method)
// 66 c7 40 12 00 00 ; mov word [eax + 0x12], 0
// b8 78 56 34 12 ; mov eax, 0x12345678 (addr of the hook method)
// ff 70 20 ; push dword [eax + 0x20]
// c3 ; ret
unsigned char redirectTrampoline[] = {
        0xb8, 0x21, 0x43, 0x65, 0x87,
        0x66, 0xc7, 0x40, 0x12, 0x00, 0x00,
        0xb8, 0x78, 0x56, 0x34, 0x12,
        0xff, 0x70, 0x20,
        0xc3
};

//https://android.googlesource.com/platform/art/+/oreo-release/runtime/modifiers.h
int Hook::kAccNative = 0x0100;

Hook::Hook(void* hookArtMethod, void* backupArtMethod, void* targetArtMethod, int nativeHookAddress,
unsigned char* trampoline, size_t size) throw(HookException) : hookToNativePatchCode(NULL),
                                                               targetToHookPatchCode(NULL){

    this->hookArtMethod = hookArtMethod;
    this->backupArtMethod = backupArtMethod;
    this->targetArtMethod = targetArtMethod;
    this->nativeHookAddress = nativeHookAddress;

    patchCodeSize = size;

    try {
        hookToNativePatchCode = genJniTrampoline(trampoline, patchCodeSize);
    }catch (AllocationException e) {
        std::string msg("Couldn't generate jni trampoline: ");
        msg.append(e.what());
        throw HookException(msg);
    }

    targetToHookPatchCode = genRedirectPatchCode(hookArtMethod, backupArtMethod);

    // update the cached method manually
    // first we find the array of cached methods
    void *dexCacheResolvedMethods = (void *) read32(
            (void *) ((char *) this->hookArtMethod + ART_METHOD.dex_cache_resolved_methods_));
    // then we get the dex method index of the static backup method
    int methodIndex = read32((void *) ((char *) this->backupArtMethod + ART_METHOD.dex_method_index_));
    // finally the addr of backup method is put at the corresponding location in cached methods array
    memcpy((char *) dexCacheResolvedMethods  + 4 * methodIndex,
           (&this->backupArtMethod),
           4);


    // copy the target art-method to the backup art-method location.
    // the backup art-method needn't be restored later. So this should be safe.
    memcpy(this->backupArtMethod, this->targetArtMethod, ART_METHOD.artMethodSize);


    directHookToNativeFunction();

    directTargetToHookMethod();
}

Hook::~Hook(){}

void Hook::activate(bool activate) {
    activated = activate;
}

bool Hook::isActivated() {
    return activated;
}


void* Hook::genJniTrampoline(unsigned char* trampoline, unsigned int size) throw(AllocationException) {
    logE("EVIL_LIB::HOOK", "trampoline size: %i", size);
    void* buf = generateExecutableCode(size);
    memcpy(buf, trampoline, size);
    return buf;
}

void Hook::addNativeFlag(void *artMethod) {
    int access_flags = readAccessFlags(artMethod);
    logE("EVIL_LIB::HOOK", "access flags is 0x%x", access_flags);

    // set the hook method to native so that Android O wouldn't invoke it with interpreter
    access_flags |= kAccNative;
    setAccessFlags(artMethod, access_flags);
}

void Hook::directHookToNativeFunction() {

    // set the hook method to native so that Android O wouldn't invoke it with interpreter
    addNativeFlag(hookArtMethod);

    // The entry point for native methods is at entry_point_from_quick_compiled_code_
    //Thus set entry_point_from_quick_compiled_code_ pointing to the jni trampoline
    memcpy((char *) hookArtMethod + ART_METHOD.entry_point_from_quick_compiled_code_,
           &this->hookToNativePatchCode, 4);

    //The Jni trampoline will call native code at the data_ pointer. So let us point the data_ pointer
    // to the native hook function.
    memcpy((char *) hookArtMethod + ART_METHOD.data_, (char *) &nativeHookAddress, 4);
}

void Hook::directTargetToHookMethod() {

   // memcpy((char *) targetArtMethod + ART_METHOD.data_, (char *) &nativeHookAddress, 4);

    memcpy((char *) targetArtMethod + ART_METHOD.entry_point_from_quick_compiled_code_,
           &this->targetToHookPatchCode, 4);

    // set the target method to native so that Android O wouldn't invoke it with interpreter
    addNativeFlag(targetArtMethod);

}

void* Hook::generateExecutableCode(size_t size) throw (AllocationException) {
    void* buf = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    if(buf == MAP_FAILED) {
        ostringstream ss;
        ss << "Hook::generateExecutableCode: mmap failed: ";
        ss << strerror(errno);
        throw AllocationException(ss.str());
    }
    return buf;
}

int Hook::readAccessFlags(void* artMethod) {
    char* pointer = (char *) artMethod + ART_METHOD.access_flags_;
    std::atomic<std::uint32_t>* atomicFlags = (std::atomic<std::uint32_t>*)pointer;
    return *atomicFlags;
}

void Hook::setAccessFlags(void *artMethod, int flags) {
    char* pointer = (char *) artMethod + ART_METHOD.access_flags_;
    std::atomic<std::uint32_t>* atomicFlags = (std::atomic<std::uint32_t>*)pointer;
    *atomicFlags = (std::uint32_t)flags;
    //memcpy((char *) artMethod + ART_METHOD.access_flags_, &flags, 4);
}


void* Hook::genRedirectPatchCode(void *hookArtMethod, void *backupArtMethod) throw(AllocationException) {
        unsigned char *targetAddr;

        redirectTrampoline[18] = (unsigned char)ART_METHOD.entry_point_from_quick_compiled_code_;
        targetAddr = (unsigned char*)generateExecutableCode(sizeof(redirectTrampoline));
        memcpy(targetAddr, redirectTrampoline, sizeof(redirectTrampoline));

        // replace with the actual ArtMethod addr

        memcpy(targetAddr+12, &hookArtMethod, 4);
        //if(SDKVersion >= ANDROID_N && backupMethod) {
        memcpy(targetAddr+1, &backupArtMethod, 4);
        return targetAddr;
        //}
        //else {
        //return targetAddr+11;
        //}
}

void *Hook::getBackupMethod() {
    return backupArtMethod;
}

void Hook::resetHotnessCount(void *artMethod) {
    write16((char *) artMethod + ART_METHOD.hotness_count_, 0);
}
