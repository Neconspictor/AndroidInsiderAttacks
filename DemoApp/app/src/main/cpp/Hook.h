#ifndef HOOK_H
#define HOOK_H

#include "Exceptions.h"


#if defined(__i386__)
#define ADDRESS_POINTER uint32_t
#else
#error Only x86 architecture is supported!
#endif


/**
 * A hooking class that allows to hook any java method on Android Runtime (ART).
 * Currently this class is only supported for Android 7.0+ and only for the x86 machine architecture.
 *
 * Definitions that are used throughout the documentation of this class:
 *
 * ART: Android Runtime
 *
 * Native java method: A java method that was compiled with the native keyword. For a native java method
 * a corresponding native ArtMethod is created when ART loads the dex file.
 *
 * (Valid) Native ArtMethod: An ArtMethod is called native if its access flags contain the native flag.
 * But setting the native flag alone for an arbitrary ArtMethod isn't enough for an ArtMethod,
 * so that this ArtMethod can be called without provoking a crash, as it needs to have a proper prolog
 * that is located at the entry point of the ArtMethod. So when we speak of a valid native ArtMethod
 * we usually mean the ArtMethod of a native java method.
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
     * helper ArtMethod.
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
     * IMPORTANT: The java methods of nativeHookHelperArtMethod and the backupArtMethod must have
     * the same method signature than the targetArtMethod. Otherwise the hook is unstable.
     * Furthermore nativeHookHelperArtMethod has to be a valid native ArtMethod.
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

    /**
     * Releases memory allocated by this object
     */
    ~Hook();

    /**
     * Sets the state of this hook.
     * @param activate : true if the hook should be activated.
     */
    void activate(bool activate);

    /**
     * @return : true if the hook is activated.
     */
    bool isActivated();

    /**
     * @return : A pointer to the backup ArtMethod.
     */
    void* getBackupMethod();

    /**
     * @return : The hotness counter of the backup ArtMethod.
     */
    short getBackupHotnessCount();

private:

    /**
     * Adds the native flag to a given ArtMethod.
     * @param artMethod The ArtMethod that should be set to be native.
     */
    void addNativeFlag(void* artMethod);

    /**
     * Patches the hook helper ArtMethod so that execution flow is directed to the native hook function.
     */
    void directHookToNativeFunction();

    /**
     * Patches the target ArtMethod so that execution flow is directed to the Hook Helper ArtMethod.
     */
    void directTargetToHookMethod();

    /**
     * Creates memory of a given size, that has execution access permission.
     * The memory will be created with malloc and thus has to be released with the 'free' function.
     * @param size The size of the memory to be created.
     * @return A pointer to the beginning ot the created executable memory.
     */
    static void* generateExecutableCode(size_t size) throw (AllocationException);

    /**
     * Creates patch code that allows to redirect execution flow from the target ArtMethod to the
     * hook helper ArtMethod.
     * @param hookArtMethod : The hook helper ArtMethod
     * @return : a pointer to the beginning of the created patch code. The created code has to be
     * released with the 'free' function.
     */
    static void* genRedirectPatchCode(void *hookArtMethod) throw(AllocationException);

    /**
     * @param artMethod : The ArtMethod to read the access flags from
     * @return : The access flags of the given ArtMethod.
     */
    int readAccessFlags(void* artMethod);

    /**
     * Gives the specified ArtMethod a specific access flags.
     */
    void setAccessFlags(void* artMethod, int flags);

    /**
     * Ensures, that the hotness counter of the backup ArtMethod is reset each time before the backup
     * ArtMethod is called.
     */
    void setupBackupHotnessResetPatch();
};


#endif //HOOK_H