//
// Created by necon on 08.01.2018.
//

#ifndef DEMOAPP_HOOK_H
#define DEMOAPP_HOOK_H

#include "Exceptions.h"


class Hook {
private:
    void* backupArtMethod;
    void* hookArtMethod;
    void* targetArtMethod;
    int nativeHookAddress;
    void* jniTrampoline_;
    size_t trampolineSize;
    void* targetToHookTrampoline;

    int accessFlagsBackupArtBackup;
    int accessFlagsHookArtBackup;
    int accessFlagsTargetArtBackup;
public:

    Hook(void* hookArtMethod, void* backupArtMethod, void* targetArtMethod, int nativeHookAddress,
         unsigned char* trampoline, size_t size) throw(HookException);

    ~Hook();

    static void* genJniTrampoline(unsigned char* trampoline, unsigned int size) throw(AllocationException);

    void activate();

    void deactivate();

private:
    void addNativeFlag(void* artMethod);
    void directHookToNativeFunction();
    void directTargetToHookMethod();

    static void* generateExecutableCode(size_t size) throw (AllocationException);
    static void* genRedirectTrampoline(void* hookArtMethod, void* backupArtMethod) throw(AllocationException);

    int readAccessFlags(void* artMethod);
    void setAccessFlags(void* artMethod, int flags);
};


#endif //DEMOAPP_HOOK_H