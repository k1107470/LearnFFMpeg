//
// Created by jinshan on 2024/10/13.
//

#ifndef LEARNFFMPEG_FFDATA_H
#define LEARNFFMPEG_FFDATA_H

enum FFDataType {
    AVPACKET_TYPE = 0,
    UCHAR_TYPE = 1
};

struct FFData {
    FFDataType type = AVPACKET_TYPE; // 音频数据用Alloc，视频使用AVPacket
    unsigned char* data;
    unsigned char* datas[8] = {0};
    int size = 0;
    long pts = 0;
    int width = 0;
    int height = 0;
    bool isAudio = false;
    int format = 0;
    bool Alloc(int allocSize, const char *buf = 0);
    void Drop();

};


#endif //LEARNFFMPEG_FFDATA_H
