//
// Created by necon on 14.01.2018.
//

#ifndef DEMOAPP_NATIVEHOOK_H
#define DEMOAPP_NATIVEHOOK_H


#include "../Exceptions.h"


using FlowFunction = std::function<void()>;

class NativeHook {
private:
    void* backupArtMethod;
    void* backupRedirectArtMethod;
    void* redirectArtMethod;
    void* targetArtMethod;
    int nativeHookAddress;
    void* redirectToNativeTrampoline_;
    void* targetToRedirectTrampoline;
    void* backupRedirectToBackupTrampoline;
    size_t trampolineSize;

    int methodIndex;

    bool activated;
    int accessFlagsTargetArtBackup;
    FlowFunction flowFunction;
    std::shared_ptr<void> jumpTrampoline;

public:

    NativeHook(void* redirectArtMethod, void* targetArtMethod,  void* backupArtMethod, int nativeHookAddress,
         unsigned char* trampoline, size_t size, void* newRef) throw(HookException);

    ~NativeHook();

    static void* genJniTrampoline(unsigned char* trampoline, unsigned int size) throw(AllocationException);

    void activate(bool activate);

    bool isActivated();

    void* getBackupMethod();

    void resetHotnessCount(void *artMethod);

    static int kAccNative;
protected:
    static unsigned char jumpToMachineCode[];

private:
    void addNativeFlag(void* artMethod);
    void addStaticFlag(void* artMethod);
    void directRedirectToNativeFunction();
    void directTargetToRedirectMethod();

    static void* generateExecutableCode(size_t size) throw (AllocationException);

    int readAccessFlags(void* artMethod);
    void unsetFlag(void* method, int flag);
    void setAccessFlags(void* artMethod, int flags);

    void removeTargetRedirection();

    void *genRedirectTrampoline(void *redirectArtMethod, void *backupArtMethod,
                                unsigned char* redirectTrampoline,
                                size_t size) throw(AllocationException);

    void directBackupRedirectToBackupMethod();

    void directTargetToNative();
};



#endif //DEMOAPP_NATIVEHOOK_H
