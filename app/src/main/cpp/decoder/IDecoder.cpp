//
// Created by jinshan on 2024/10/14.
//

#include "IDecoder.h"
#include "LogUtil.h"

#define TAG "FF-IDecoder"


void IDecoder::Update(FFData pkt) {
//    LOGI(TAG, "Update pkt isAudio = %d coder is audio = %d", pkt.isAudio, isAudio);
    // 判断pkt和decoder的类型是否一致
    if (pkt.isAudio != isAudio) {
        return;
    }
    while(!isExit) {
        packsMutex.lock();
        if (packs.size() < maxPacks) {
//            LOGD(TAG, "push pack pts = %ld isAudio = %d size %d", pkt.pts, pkt.isAudio, pkt.size);
            packs.push_back(pkt);
            packsMutex.unlock();
            break;
        }
        packsMutex.unlock();
        FFSleep(1);
    }
}

void IDecoder::Main() {
    LOGI(TAG, "Thread Main Start isAudio[%d]", isAudio);
    while(!isExit) {
        packsMutex.lock();
        if (packs.empty()) {
            packsMutex.unlock();
            FFSleep(1);
            continue;
        }
        FFData pack = packs.front();
        packs.pop_front();
//        LOGD(TAG, "Get pkt = %ld isAudio = %d size = %d", pack.pts, pack.isAudio, pack.size);
        // 发送数据到解码线程，一个数据包可能解码多个结果(音频)
        if (this->SendPacket(pack)) {
            while (!isExit) {
                // 获取解码数据
                FFData frame = this->ReceivePkt();
                if (frame.size == 0) {
                    break;
                }
//                LOGD(TAG, "receive frame size = %d isAudio = %d", frame.size, frame.isAudio);
                this->Notify(frame);
            }
        }
        pack.Drop();
        packsMutex.unlock();
    }
}

