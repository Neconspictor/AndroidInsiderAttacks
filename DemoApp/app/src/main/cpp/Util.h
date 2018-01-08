//
// Created by Neconspictor on 02.01.2018.
//

#ifndef BACHELOR_SEMINAR_UTIL_H
#define BACHELOR_SEMINAR_UTIL_H

#include <android/log.h>
#include <jni.h>
#include <string>


#define __LOG_VARARGS__(prio, tag, fmt) \
    va_list args; \
    va_start(args, fmt); \
    __android_log_vprint(prio, tag, fmt, args); \
    va_end(args);


namespace util {

    jobject getGlobalContext(JNIEnv *env);
    jmethodID getMethod(JNIEnv* env, jobject obj, const char* methodName, const char* methodSig);
    std::string getInternalStorageDir(JNIEnv* env);
    jstring getExceptionMessage(JNIEnv* env, jthrowable ex);
    jobject createDexClassLoader(const char* dexFilePath, JNIEnv* env);


    inline void logI(const char* tag, const char* fmt, ...) {
        __LOG_VARARGS__(ANDROID_LOG_INFO, tag, fmt);
    }

    inline void logW(const char* tag, const char* fmt, ...) {
        __LOG_VARARGS__(ANDROID_LOG_WARN, tag, fmt);
    }

    inline void logE(std::string tag, const char* fmt, ...) {
        __LOG_VARARGS__(ANDROID_LOG_ERROR, tag.c_str(), fmt);
    }
}


#endif //BACHELOR_SEMINAR_UTIL_H
