//
// Created by necon on 14.01.2018.
//


//
// Created by necon on 08.01.2018.
//


#include <sys/mman.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <sstream>

#include "NativeHook.h"
#include "../Util.h"

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
} ART_METHOD2;

// 1. clear hotness_count of the backup ArtMethod(only after Android N)
// 2. set eax/r0/x0 to the hook ArtMethod addr
// 3. jump into its entry point

// b8 21 43 65 87 ; mov eax, 0x87654321 (addr of the backup method)
// 66 c7 40 12 00 00 ; mov word [eax + 0x12], 0
// b8 78 56 34 12 ; mov eax, 0x12345678 (addr of the hook method)
// ff 70 20 ; push dword [eax + 0x20]
// c3 ; ret
unsigned char redirectTrampoline2[] = {
        //0xb8, 0x21, 0x43, 0x65, 0x87,
        //0x66, 0xc7, 0x40, 0x12, 0x00, 0x00,
        0xb8, 0x78, 0x56, 0x34, 0x12,
        0xff, 0x70, 0x20,
        0xc3
};

// b8 78 56 34 12 ; mov eax, 0x12345678 (addr to jump to)
// ff 70 20 ; push dword [eax + 0x20]
// c3 ; ret
unsigned char NativeHook::jumpToMachineCode[] = {
        0xb8, 0x78, 0x56, 0x34, 0x12,
        0xff, 0x70, 0x20,
        0xc3
};


/*void asmFunc(uint32_t jumpAddress);

__asm__ __volatile__ ("_asmFunc: mov %eax, %0;"
:
:"r"(jumpAddress));
*/

int kAccStatic =       0x0008;  // field, method, ic
uint32_t kAccFinal =        0x0010;
uint32_t kAccSynchronized = 0x0020;
uint32_t kAccBridge =       0x0040;
uint32_t kAccVarargs =      0x0080;
uint32_t kAccAbstract =     0x0400;
uint32_t kAccStrict =       0x0800;
uint32_t kAccSynthetic =    0x1000;
uint32_t kAccDefaultConflict =      0x00800000;
uint32_t kAccUnknown =      0x00080000;


//https://android.googlesource.com/platform/art/+/oreo-release/runtime/modifiers.h
int NativeHook::kAccNative = 0x0100;


bool isStatic(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccStatic) != 0;
}

bool isNative(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & NativeHook::kAccNative) != 0;
}

bool isFinal(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccFinal) != 0;
}

bool isSynchronized(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccSynchronized) != 0;
}

bool isBridge(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccBridge) != 0;
}

bool isVarargs(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccVarargs) != 0;
}


bool isDefaultConflict(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccDefaultConflict) != 0;
}

bool isUnknown(void* method) {
    int flags = read32(method + ART_METHOD2.access_flags_);
    return (flags & kAccUnknown) != 0;
}

NativeHook::NativeHook(void* redirectArtMethod, void* targetArtMethod, void* backupArtMethod, int nativeHookAddress,
           unsigned char* trampoline, size_t size, void* newRef) throw(HookException) {

    this->backupArtMethod = backupArtMethod;
    //this->redirectArtMethod = malloc(ART_METHOD2.artMethodSize);
    this->redirectArtMethod = redirectArtMethod;
    this->targetArtMethod = targetArtMethod;
    this->nativeHookAddress = nativeHookAddress;
   // std::atomic<uint32_t >* backupAtomic = (std::atomic<uint32_t >*)((char*)this->redirectArtMethod + ART_METHOD2.access_flags_);


    //backup access flags
    accessFlagsTargetArtBackup = readAccessFlags(targetArtMethod);
    int accessFlagsRedirectArtBackup = readAccessFlags(redirectArtMethod);


    trampolineSize = size;

    try {
        redirectToNativeTrampoline_ = genJniTrampoline(trampoline, trampolineSize);
    }catch (AllocationException e) {
        std::string msg("Couldn't generate jni trampoline: ");
        msg.append(e.what());
        throw HookException(msg);
    }

    //memcpy(this->redirectArtMethod, targetArtMethod, ART_METHOD2.artMethodSize);
    //memcpy(this->redirectArtMethod + ART_METHOD2.access_flags_, test, sizeof(std::atomic<uint32_t >));
    //memcpy(this->redirectArtMethod + ART_METHOD2.declaring_class_,
    //       this->targetArtMethod + ART_METHOD2.declaring_class_, 4);

    int classRedirect = read32(this->redirectArtMethod + ART_METHOD2.declaring_class_);
    int classBackup = read32(this->backupArtMethod + ART_METHOD2.declaring_class_);
    int classTarget = read32(this->targetArtMethod + ART_METHOD2.declaring_class_);
    int classNewRef = read32(newRef + ART_METHOD2.declaring_class_);

    logE("newRef", "%i", classNewRef);
    logE("classRedirect", "%i", classRedirect);
    logE("classBackup", "%i", classBackup);
    logE("classTarget", "%i", classTarget);
    bool _isNative = isNative(this->redirectArtMethod);
    bool _isFinal = isFinal(this->redirectArtMethod);
    bool _isSynchronized = isSynchronized(this->redirectArtMethod);
    bool _isBridge = isBridge(this->redirectArtMethod);
    bool _isVarargs = isVarargs(this->redirectArtMethod);
    bool _isDefaultConflict = isDefaultConflict(this->redirectArtMethod);
    bool _isUnknown = isUnknown(this->redirectArtMethod);
    bool _isStatic = isStatic(this->redirectArtMethod);


    logE("isNative", "%i", _isNative);
    logE("isFinal", "%i", _isFinal);
    logE("isSynchronized", "%i", _isSynchronized);
    logE("_isBridge", "%i", _isBridge);
    logE("_isVarargs", "%i", _isVarargs);
    logE("_isDefaultConflict", "%i", _isDefaultConflict);
    logE("_isUnknown", "%i", _isUnknown);
    logE("_isStatic", "%i", _isStatic);

    //unsetFlag(this->redirectArtMethod, kAccUnknown);
    //unsetFlag(this->redirectArtMethod, kAccStatic);

    _isDefaultConflict = isDefaultConflict(this->targetArtMethod);
    _isUnknown = isUnknown(this->targetArtMethod);
    logE("_isDefaultConflict", "%i", _isDefaultConflict);
    logE("_isUnknown", "%i", _isUnknown);

    //setAccessFlags(this->redirectArtMethod, kAccNative | kAccSynchronized);

    int resolvedMethods = read32(this->redirectArtMethod + ART_METHOD2.dex_method_index_);
    logE("dex_code_item_offset_", "%i", resolvedMethods);


    this->backupRedirectArtMethod = generateExecutableCode(ART_METHOD2.artMethodSize);
    memcpy(this->backupRedirectArtMethod, this->targetArtMethod, ART_METHOD2.artMethodSize);

    // copy the target art-method to the backup art-method location.
    // the backup art-method needn't be restored later. So this should be safe.
    memcpy(this->backupArtMethod, this->targetArtMethod, ART_METHOD2.artMethodSize);

   // addNativeFlag(this->backupArtMethod);


    //backupRedirectToBackupTrampoline = genRedirectPatchCode(this->backupArtMethod, this->backupArtMethod);

    //directTargetToNative();

    //alloc memory c++ style...
    jumpTrampoline = shared_ptr<void>(malloc(sizeof(jumpToMachineCode)), free);

    //set jump trampoline
    memcpy(jumpTrampoline.get(), (char*)this->jumpToMachineCode, sizeof(jumpToMachineCode));

    targetToRedirectTrampoline = genRedirectTrampoline(this->redirectArtMethod,
                                                       this->backupArtMethod,
                                                       (unsigned char*)jumpTrampoline.get(),
                                                        sizeof(jumpToMachineCode));

    directRedirectToNativeFunction();
    directTargetToRedirectMethod();

    flowFunction = [&] {
        if (activated) {

        }
    };
    //directBackupRedirectToBackupMethod();
}

NativeHook::~NativeHook(){
    /*if (hookToNativeTrampoline_ != NULL) {
        munmap(hookToNativeTrampoline_, trampolineSize);
        hookToNativeTrampoline_ = NULL;
    }

    if (targetToHookTrampoline != NULL) {
        munmap(targetToHookTrampoline, sizeof(redirectTrampoline));
    }*/
}

void NativeHook::activate(bool activate) {
    //directHookToNativeFunction();

    //directTargetToHookMethod();
    activated = activate;
}

bool NativeHook::isActivated() {
    return activated;
}


void* NativeHook::genJniTrampoline(unsigned char* trampoline, unsigned int size) throw(AllocationException) {
    logE("EVIL_LIB::HOOK", "trampoline size: %i", size);
    void* buf = generateExecutableCode(size);
    memcpy(buf, trampoline, size);
    return buf;
}

void NativeHook::addNativeFlag(void *artMethod) {
    int access_flags = readAccessFlags(artMethod);
    logE("EVIL_LIB::addNativeFlag", "access flags is 0x%x", access_flags);

    // set the hook method to native so that Android O wouldn't invoke it with interpreter
    access_flags |= kAccNative;
    setAccessFlags(artMethod, access_flags);

    access_flags = readAccessFlags(artMethod);
    logE("EVIL_LIB::addNativeFlag", "access flags is now 0x%x", access_flags);
}

void NativeHook::addStaticFlag(void *artMethod) {
    int access_flags = readAccessFlags(artMethod);
    logE("EVIL_LIB::addStaticFlag", "access flags is 0x%x", access_flags);

    // set the hook method to native so that Android O wouldn't invoke it with interpreter
    access_flags |= kAccStatic;
    setAccessFlags(artMethod, access_flags);
}

void NativeHook::directRedirectToNativeFunction() {

    // set the hook method to native so that Android O wouldn't invoke it with interpreter
    addNativeFlag(redirectArtMethod);

    // The entry point for native methods is at entry_point_from_quick_compiled_code_
    //Thus set entry_point_from_quick_compiled_code_ pointing to the jni trampoline
    memcpy((char *) redirectArtMethod + ART_METHOD2.entry_point_from_quick_compiled_code_,
           (char*)&this->redirectToNativeTrampoline_, 4);

    //The Jni trampoline will call native code at the data_ pointer. So let us point the data_ pointer
    // to the native hook function.
    memcpy((char *) redirectArtMethod + ART_METHOD2.data_, (char *) &nativeHookAddress, 4);
}

void NativeHook::directTargetToRedirectMethod() {


    memcpy((char *) targetArtMethod + ART_METHOD2.entry_point_from_quick_compiled_code_,
           (char*)&this->targetToRedirectTrampoline, 4);

    // set the target method to native so that Android O wouldn't invoke it with interpreter
    addNativeFlag(targetArtMethod);
}

void NativeHook::directTargetToNative() {
// set the hook method to native so that Android O wouldn't invoke it with interpreter
    addNativeFlag(targetArtMethod);

    // The entry point for native methods is at entry_point_from_quick_compiled_code_
    //Thus set entry_point_from_quick_compiled_code_ pointing to the jni trampoline
    memcpy((char *) targetArtMethod + ART_METHOD2.entry_point_from_quick_compiled_code_,
           (char*)&this->redirectToNativeTrampoline_, 4);

    //The Jni trampoline will call native code at the data_ pointer. So let us point the data_ pointer
    // to the native hook function.
    memcpy((char *) targetArtMethod + ART_METHOD2.data_, (char *) &nativeHookAddress, 4);
}

void NativeHook::directBackupRedirectToBackupMethod() {
    memcpy((char *) backupRedirectArtMethod + ART_METHOD2.entry_point_from_quick_compiled_code_,
           (char*)&this->backupRedirectToBackupTrampoline, 4);

    // set the target method to native so that Android O wouldn't invoke it with interpreter
    addNativeFlag(backupRedirectArtMethod);
}

void NativeHook::removeTargetRedirection() {

    //memcpy((char *) targetArtMethod + ART_METHOD.entry_point_from_quick_compiled_code_,
    //       &this->targetToBackupTrampoline, 4);

    //addNativeFlag(targetArtMethod);

}

void* NativeHook::generateExecutableCode(size_t size) throw (AllocationException) {
    void* buf = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    if(buf == MAP_FAILED) {
        ostringstream ss;
        ss << "Hook::generateExecutableCode: mmap failed: ";
        ss << strerror(errno);
        throw AllocationException(ss.str());
    }
    return buf;
}

int NativeHook::readAccessFlags(void* artMethod) {
    char* pointer = (char *) artMethod + ART_METHOD2.access_flags_;
    std::atomic<std::uint32_t>* atomicFlags = (std::atomic<std::uint32_t>*)pointer;
    return *atomicFlags;
}

void NativeHook::setAccessFlags(void *artMethod, int flags) {
    char* pointer = (char *) artMethod + ART_METHOD2.access_flags_;
    std::atomic<std::uint32_t>* atomicFlags = (std::atomic<std::uint32_t>*)pointer;
    *atomicFlags = (std::uint32_t)flags;
    //memcpy((char *) artMethod + ART_METHOD.access_flags_, &flags, 4);
}

void NativeHook::unsetFlag(void* method, int flag) {
    int flags = readAccessFlags(method);
    setAccessFlags(method, flags & (~flag));
}

void *NativeHook::getBackupMethod() {
    return this->backupArtMethod;
}

void NativeHook::resetHotnessCount(void *artMethod) {
    write16((char *) artMethod + ART_METHOD2.hotness_count_, 0);
}

void* NativeHook::genRedirectTrampoline(void* redirectArtMethod, void* backupArtMethod,
                                        unsigned char* redirectTrampoline,
                                        size_t size) throw(AllocationException) {
    unsigned char *targetAddr;

    //redirectTrampoline2[18] = (unsigned char)ART_METHOD2.entry_point_from_quick_compiled_code_;
    redirectTrampoline[7] = (unsigned char)ART_METHOD2.entry_point_from_quick_compiled_code_;
    targetAddr = (unsigned char*)generateExecutableCode(sizeof(redirectTrampoline2));
    memcpy(targetAddr, redirectTrampoline, size);

    // replace with the actual ArtMethod addr

    //memcpy(targetAddr+12, &redirectArtMethod, 4);
    memcpy(targetAddr+1, &redirectArtMethod, 4);
    //if(SDKVersion >= ANDROID_N && backupMethod) {
    //memcpy(targetAddr+1, &backupArtMethod, 4);
    return targetAddr;
    //}
    //else {
    //return targetAddr+11;
    //}
}