//
// Created by jinshan on 2024/10/13.
//

#include "FFData.h"

extern "C" {
#include <libavformat/avformat.h>
}


bool FFData::Alloc(int allocSize, const char *buf) {
    Drop();
    type = UCHAR_TYPE;
    if (allocSize <= 0) return false;
    this->data = new unsigned char[allocSize];
    if (!this->data) return false;
    if (buf) {
        memcpy(this->data, buf, allocSize);
    }
    this->size = allocSize;
    return true;
}

void FFData::Drop() {
    if (size == 0) return;
    if (type == AVPACKET_TYPE) {
        av_packet_free((AVPacket **) &data);
    } else {
        delete data;
    }
    data = nullptr;
    size = 0;
}

