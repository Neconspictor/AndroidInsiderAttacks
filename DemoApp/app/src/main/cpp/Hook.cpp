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
 * Data is based on https://android.googlesource.com/platform/art/+/android-8.0.0_r36/runtime/art_method.h
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



// 1. set eax/r0/x0 to the hook ArtMethod addr
// 2. jump into its entry point
// b8 78 56 34 12 ; mov eax, 0x12345678 (addr of the native hook method)
// ff 70 1C ; push dword [eax + 0x1C]
// c3 ; ret
unsigned char redirectTrampoline[] = {
        0xb8, 0x78, 0x56, 0x34, 0x12,
        0xff, 0x70, 0x1C,
        0xc3
};

// 1. save eax register
// 2. clear hotness_count of the backup ArtMethod(only after Android N)
// 3. restore eax register
// 4. jump to original backup entry point
unsigned char resetHotnessCountCode[] = {
        0x50, // push eax  offset: 0
        0xb8, 0x21, 0x43, 0x65, 0x87, // mov eax, 0x87654321 (addr of the backup method) offset: 1
        0x66, 0xc7, 0x40, 0x12, 0x00, 0x00, // mov    WORD PTR [eax],0x0     offset: 6
        0x58, // pop eax    offset: 12
        0x68, 0x21, 0x43, 0x65, 0x87, // push 0x87654321 (addr of the original backup entry point)    offset: 13
        0xc3 // ret  -> jumps to address that lies on top of the stack  offset: 18
};

//https://android.googlesource.com/platform/art/+/android-8.0.0_r36/runtime/modifiers.h
int Hook::kAccNative = 0x0100;

Hook::Hook(void* nativeHookHelperArtMethod, void* backupArtMethod, void* targetArtMethod, ADDRESS_POINTER nativeHookAddress)
throw(HookException) : targetToHookHelperPatchCode(NULL){

    this->nativeHookHelperArtMethod = nativeHookHelperArtMethod;
    this->backupArtMethod = backupArtMethod;
    this->targetArtMethod = targetArtMethod;
    this->nativeHookAddress = nativeHookAddress;


    targetToHookHelperPatchCode = genRedirectPatchCode(nativeHookHelperArtMethod);

    // copy the target art-method to the backup art-method location.
    // the backup art-method needn't be restored later. So this should be safe.
    memcpy(this->backupArtMethod, this->targetArtMethod, ART_METHOD.artMethodSize);

    directHookToNativeFunction();

    directTargetToHookMethod();

    setupBackupHotnessResetPatch();
}

Hook::~Hook(){}

void Hook::activate(bool activate) {
    activated = activate;
}

bool Hook::isActivated() {
    return activated;
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
    addNativeFlag(nativeHookHelperArtMethod);

    //The Jni trampoline will call native code at the data_ pointer. So let us point the data_ pointer
    // to the native hook function.
    memcpy((char *) nativeHookHelperArtMethod + ART_METHOD.data_, (char *) &nativeHookAddress, sizeof(ADDRESS_POINTER));
}

void Hook::directTargetToHookMethod() {

    memcpy((char *) targetArtMethod + ART_METHOD.entry_point_from_quick_compiled_code_,
           &this->targetToHookHelperPatchCode, sizeof(ADDRESS_POINTER));

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
}


void* Hook::genRedirectPatchCode(void *hookArtMethod) throw(AllocationException) {
    unsigned char *targetAddr;

    targetAddr = (unsigned char*)generateExecutableCode(sizeof(redirectTrampoline));
    memcpy(targetAddr, redirectTrampoline, sizeof(redirectTrampoline));

    // replace with the actual ArtMethod addr

    memcpy(targetAddr+1, &hookArtMethod, 4);
    return targetAddr;
}

void *Hook::getBackupMethod() {
    return backupArtMethod;
}

void Hook::setupBackupHotnessResetPatch() {
    unsigned char* newEntryPoint = (unsigned char*)generateExecutableCode(sizeof(resetHotnessCountCode));
    memcpy(newEntryPoint, resetHotnessCountCode, sizeof(resetHotnessCountCode));

    // set backup method addresses
    memcpy(newEntryPoint+2, (char*)&backupArtMethod, sizeof(ADDRESS_POINTER));

    //important: use the value of the entry point, not the address (since the address will be changed)!
    memcpy(newEntryPoint+14, (char*)backupArtMethod + ART_METHOD.entry_point_from_quick_compiled_code_, sizeof(ADDRESS_POINTER));

    //set the new entry point
    memcpy((char*)backupArtMethod + ART_METHOD.entry_point_from_quick_compiled_code_, &newEntryPoint, sizeof(ADDRESS_POINTER));
}

short Hook::getBackupHotnessCount() {
    short* pointer =  (short*)((char*)backupArtMethod + ART_METHOD.hotness_count_);
    return *pointer;
}