//
// Created by jinshan on 2024/10/13.
//

#include "FFThread.h"
#include "LogUtil.h"
#include <thread>

#define TAG "FF-Thread"
#define THREAD_STOP_TIME_OUT 200

using namespace std;

void FFSleep(int ms) {
    chrono::milliseconds du(ms);
    this_thread::sleep_for(du);
}

void FFThread::Start() {
    LOGI(TAG, "%s start", __FUNCTION__ );
    isExit = false;
    thread th (&FFThread::ThreadMain, this);
    th.detach();
}

void FFThread::ThreadMain() {
    LOGI(TAG, "%s start", __FUNCTION__ );
    isRunning = true;
    Main();
    isRunning = false;
    LOGI(TAG, "%s end", __FUNCTION__ );
}

void FFThread::Stop() {
    LOGI(TAG, "%s" , __FUNCTION__);
    isExit = true;
    for(int i = 0; i < THREAD_STOP_TIME_OUT; ++i) {
        if (!isRunning) {
            LOGI(TAG, "thread stop success");
            return;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1)); // 睡眠200微秒
    }
    LOGI(TAG, "thread stop time out");
}