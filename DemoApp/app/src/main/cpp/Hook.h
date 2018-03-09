#ifndef DEMOAPP_HOOK_H
#define DEMOAPP_HOOK_H

#include "Exceptions.h"


#if defined(__i386__)
#define ADDRESS_POINTER uint32_t
#else
#error Only x86 architecture is supported!
#endif


/**
 * A hooking class that allows to hook any java method on Android Runtime (ART).
 * Currently this class is only supported for Android 7.0+ and only for the x86 machine architecture.
 */
class Hook {
private:

    /**
     * An ArtMethod that represents a native java method. This ArtMethod is used to store the original
     * content of the hooked java method. It can be used to call the original java method from
     * the native hook function.
     */
    void* backupArtMethod;

    /**
     * Definition: native ArtMethod: An ArtMethod that has the native flag set.
     *
     * ART doesn't allow to directly jump from a non native ArtMethod to a native function. If one tries,
     * the calling of any java method inside the native function will hang up the application.
     * But this problem can be circumvented if we first direct execution to another ArtMethod, that is
     * native. And then from this ArtMethod we call the native function.
     * As strange as it sounds, this workaround is acceptable for ART.
     */
    void* nativeHookHelperArtMethod;

    /**
     * The ArtMethod that is used to call the java method that should be hooked.
     */
    void* targetArtMethod;

    /**
     * The address of the native hook function. This function will be called, if the hooked java method
     * is called from the app.
     */
    ADDRESS_POINTER nativeHookAddress;

    /**
     * Holds machine code that redirects execution flow from the hooked method to the
     * Helper ArtMethod.
     */
    void* targetToHookHelperPatchCode;

    /**
     * Specifies whether this hook is activated or deactivated
     */
    bool activated;

    /**
     * the native flag of an ArtMethod.
     */
    static int kAccNative;


public:

    /**
     * Creates a new Hook.
     *
     * IMPORTANT: The nativeHookHelperArtMethod and the backupArtMethod must have the same signature
     * than the targetArtMethod. Otherwise the hook is unstable. Furthermore nativeHookHelperArtMethod
     * has to be native. An ArtMethod is called native if it is compiled as a native java method. Only
     * setting its native flag isn't enough, as it needs to have a proper prolog that is located at its entry point.
     *
     * @param nativeHookHelperArtMethod: A valid native ArtMethod that is used as a helper for calling
     *  the native hook function.
     * @param backupArtMethod: A valid native ArtMethod that is used to store the original state of the hooked
     * ArtMethod.
     * @param targetArtMethod: The ArtMethod that should be hooked
     * @param nativeHookAddress: The address to the native function that should be called instead of
     * the hooked java method. This function is also called the 'native hook function'
     *
     * @throws HookException : If any error occurs while hooking.
     */
    Hook(void* nativeHookHelperArtMethod,
         void* backupArtMethod,
         void* targetArtMethod,
         ADDRESS_POINTER nativeHookAddress)
    throw(HookException);

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
    static void* genRedirectPatchCode(void *hookArtMethod) throw(AllocationException);

    int readAccessFlags(void* artMethod);
    void setAccessFlags(void* artMethod, int flags);

    void setupBackupHotnessResetPatch();
};


#endif //DEMOAPP_HOOK_H