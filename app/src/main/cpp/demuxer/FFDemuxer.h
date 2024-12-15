//
// Created by jinshan on 2024/10/13.
//

#ifndef LEARNFFMPEG_FFDEMUXER_H
#define LEARNFFMPEG_FFDEMUXER_H


#include "IDemuxer.h"

struct AVFormatContext;

class FFDemuxer : public IDemuxer{
public:
    FFDemuxer() noexcept;

    bool Open(const char * url) override;

    FFParameter GetVPara() override;

    FFParameter GetAPara() override;

    FFData Read() override;
private:

    AVFormatContext *fmt_ctx = nullptr;
    int audioStream = 1;
    int videoStream = 0;
};


#endif //LEARNFFMPEG_FFDEMUXER_H
