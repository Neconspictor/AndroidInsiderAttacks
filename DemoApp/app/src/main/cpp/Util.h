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

    static inline uint32_t read32(void *addr) {
        return *((uint32_t *)addr);
    }

    static inline uint16_t read16(void *addr) {
        return *((uint16_t *)addr);
    }

    static inline void write32(void *addr, uint32_t value) {
        *((uint32_t *)addr) = value;
    }

    static inline void write16(void *addr, uint16_t value) {
        *((uint16_t *)addr) = value;
    }
}


#endif //BACHELOR_SEMINAR_UTIL_H
