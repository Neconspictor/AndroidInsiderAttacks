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
    void* hookToNativeTrampoline_;
    size_t trampolineSize;
    void* targetToHookTrampoline;
    void* targetToBackupTrampoline;

    int methodIndex;

    int dex_code_item_offset_;
    int dex_method_index_;
    int method_index_;
    int declaring_class_;

    bool activated;

    int accessFlagsBackupArtBackup;
    int accessFlagsHookArtBackup;
    int accessFlagsTargetArtBackup;

    static int kAccNative;
public:

    Hook(void* hookArtMethod, void* backupArtMethod, void* targetArtMethod, int nativeHookAddress,
         unsigned char* trampoline, size_t size) throw(HookException);

    ~Hook();

    static void* genJniTrampoline(unsigned char* trampoline, unsigned int size) throw(AllocationException);

    void activate(bool activate);

    bool isActivated();

    void* getBackupMethod();

    void resetHotnessCount(void *artMethod);

private:
    void addNativeFlag(void* artMethod);
    void directHookToNativeFunction();
    void directTargetToHookMethod();

    static void* generateExecutableCode(size_t size) throw (AllocationException);
    static void* genRedirectTrampoline(void* hookArtMethod, void* backupArtMethod) throw(AllocationException);

    int readAccessFlags(void* artMethod);
    void setAccessFlags(void* artMethod, int flags);

    void removeTargetRedirection();
};


#endif //DEMOAPP_HOOK_H