//
// Created by necon on 08.01.2018.
//

#ifndef DEMOAPP_HOOK_H
#define DEMOAPP_HOOK_H

#include "Exceptions.h"


#if defined(__i386__)
#define ADDRESS_POINTER uint32_t
#else
#error Only x86 architecture is supported!
#endif


class Hook {
private:
    void* backupArtMethod;
    void* nativeHookHelperArtMethod;
    void* targetArtMethod;
    ADDRESS_POINTER nativeHookAddress;
    void* helperToNativePatchCode;
    size_t patchCodeSize;
    void* targetToHookHelperPatchCode;

    bool activated;

    static int kAccNative;
public:

    Hook(void* nativeHookHelperArtMethod, void* backupArtMethod, void* targetArtMethod, ADDRESS_POINTER nativeHookAddress) throw(HookException);

    ~Hook();

    void activate(bool activate);

    bool isActivated();

    void* getBackupMethod();

    short getBackupHotnessCount();

private:
    void addNativeFlag(void* artMethod);
    void directHookToNativeFunction();
    void directTargetToHookMethod();

    static void* generateExecutableCode(size_t size) throw (AllocationException);
    static void* genRedirectPatchCode(void *hookArtMethod, void *backupArtMethod) throw(AllocationException);

    int readAccessFlags(void* artMethod);
    void setAccessFlags(void* artMethod, int flags);

    void setupBackupHotnessResetPatch();
};


#endif //DEMOAPP_HOOK_H