//
// Created by jinshan on 2024/10/8.
//

#ifndef LEARNFFMPEG_LOGUTIL_H
#define LEARNFFMPEG_LOGUTIL_H

#include <jni.h>
#include <android/log.h>
#include <sys/syscall.h>
#include <unistd.h>

#define GET_THREAD_ID() syscall(SYS_gettid)

#define LOG_HEADER(fmt) "[%s:%d | %ld]" fmt "", __FILE_NAME__, __LINE__ , GET_THREAD_ID(), ##__VA_ARGS__

// 定义日志宏
#define LOGD(TAG, fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, \
        TAG, "[%s:%d | %ld]" fmt "", __FILE_NAME__, __LINE__ , GET_THREAD_ID(), ##__VA_ARGS__)
#define LOGI(TAG, fmt, ...) __android_log_print(ANDROID_LOG_INFO, \
        TAG, "[%s:%d | %ld]" fmt "", __FILE_NAME__, __LINE__, GET_THREAD_ID() , ##__VA_ARGS__)
#define LOGW(TAG, fmt, ...) __android_log_print(ANDROID_LOG_WARN, \
        TAG, "[%s:%d | %ld]" fmt "", __FILE_NAME__, __LINE__ , GET_THREAD_ID(), ##__VA_ARGS__)
#define LOGE(TAG, fmt, ...) __android_log_print(ANDROID_LOG_ERROR, \
        TAG, "[%s:%d | %ld]" fmt "", __FILE_NAME__, __LINE__ , GET_THREAD_ID(), ##__VA_ARGS__)


#endif //LEARNFFMPEG_LOGUTIL_H
