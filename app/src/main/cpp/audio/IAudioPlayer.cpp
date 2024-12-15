//
// Created by jinshan on 2024/10/15.
//

#include "IAudioPlayer.h"
#include "LogUtil.h"

#define TAG "FF-IAudioPlayer"


FFData IAudioPlayer::GetData() {
    FFData d;
    while(!isExit) {
        frameMutex.lock();
        if (!frames.empty()) {
            d = frames.front();
            frames.pop_front();
            frameMutex.unlock();
            return d;
        }
        frameMutex.unlock();
        FFSleep(1);
    }

    return d;
}

void IAudioPlayer::Update(FFData data) {
    if (data.size <= 0 || !data.data) {
        return;
    }
    LOGI(TAG, "IAudioPlayer::Update size %d", data.size);
    while (!isExit) {
        frameMutex.lock();
        if (frames.size() > maxFrame) {
            frameMutex.unlock();
            FFSleep(1);
            continue;
        }
        frames.push_back(data);
        frameMutex.unlock();
        break;
    }
}

