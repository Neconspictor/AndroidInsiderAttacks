//
// Created by Neconspictor on 02.01.2018.
//

#ifndef BACHELOR_SEMINAR_UTIL_H
#define BACHELOR_SEMINAR_UTIL_H

#include <android/log.h>

#define LOG_TAG "EVIL_NATIVE_CODE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#endif //BACHELOR_SEMINAR_UTIL_H
