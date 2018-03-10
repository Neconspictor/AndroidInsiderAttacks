#include "Util.h"

using namespace util;
using namespace std;

#define LOG_TAG "EVIL_LIB::UTIL"

jobject util::getGlobalContext(JNIEnv *env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);

    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(at, getApplication);
    return context;
}

jmethodID util::getMethod(JNIEnv* env, jobject obj, const char* methodName, const char* methodSig) {
    jclass clazz = env->GetObjectClass(obj);
    return env->GetMethodID(clazz, methodName, methodSig);
}

std::string util::getInternalStorageDir(JNIEnv* env) {
    jobject context = getGlobalContext(env);

    jmethodID getFilesDir = getMethod(env, context, "getFilesDir", "()Ljava/io/File;");
    jobject dirobj = env->CallObjectMethod(context, getFilesDir);
    jmethodID getStoragePath = getMethod(env, dirobj, "getAbsolutePath", "()Ljava/lang/String;");
    jstring path = (jstring)env->CallObjectMethod(dirobj, getStoragePath);
    const char *pathstr=env->GetStringUTFChars(path, NULL);
    std::string internalStorageDir(pathstr);
    env->ReleaseStringUTFChars(path, pathstr);

    logE(LOG_TAG, "%s", internalStorageDir.c_str());



    return internalStorageDir;
}


jobject util::createDexClassLoader(const char *dexFilePath, JNIEnv* env) {
    jobject context = getGlobalContext(env);

    jclass pathClassLoaderClass = env->FindClass("dalvik/system/PathClassLoader");
    jmethodID constructor = env->GetMethodID(pathClassLoaderClass, "<init>",
                                              "(Ljava/lang/String;Ljava/lang/ClassLoader;)V");

    jclass contextClass = env->GetObjectClass(context);
    jmethodID getClassLoaderMethod = env->GetMethodID(contextClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject parentClassLoader = env->CallObjectMethod(context, getClassLoaderMethod);

    // IMPORTANT: Do not delete this jstring, otherwise the app is not loaded properly!
    jstring dexFilePathJString = env->NewStringUTF(dexFilePath);

    jobject loader = env->NewObject(pathClassLoaderClass, constructor, dexFilePathJString, parentClassLoader);

    return loader;
}

jstring util::getExceptionMessage(JNIEnv* env, jthrowable ex){
    jclass clazz = env->GetObjectClass(ex);
    jmethodID getMessage = env->GetMethodID(clazz,
                                            "getMessage",
                                            "()Ljava/lang/String;");
    return (jstring)env->CallObjectMethod(ex, getMessage);
}