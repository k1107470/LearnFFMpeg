//
// Created by jinshan on 2024/10/15.
//

#ifndef LEARNFFMPEG_IRESAMPLE_H
#define LEARNFFMPEG_IRESAMPLE_H


#include "IObserver.h"
#include "FFParameter.h"

class IResample : public IObserver {
public:
    virtual bool Open(FFParameter in, FFParameter out = FFParameter()) = 0;
    virtual FFData Resample(FFData indata) = 0;
    virtual void Update(FFData data);
    int outChannels = 2;
    int outFormat = 1; // 对应ffmpeg swresample 音频格式 s16
};


#endif //LEARNFFMPEG_IRESAMPLE_H
