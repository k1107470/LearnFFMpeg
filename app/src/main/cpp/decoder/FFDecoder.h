//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_FFDECODER_H
#define LEARNFFMPEG_FFDECODER_H


#include "IDecoder.h"
struct AVCodecContext;
struct AVFrame;
class FFDecoder : public IDecoder{
public:
    static void InitHard(void *vm);
    bool Open(FFParameter para, bool isHW = false) override;
    bool SendPacket(FFData pkt) override ;
    FFData ReceivePkt() override ;
protected:
    AVCodecContext *codec_ctx = nullptr;
    AVFrame *frame = nullptr;
    int maxThreadCount = 8;
};


#endif //LEARNFFMPEG_FFDECODER_H
