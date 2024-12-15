//
// Created by jinshan on 2024/10/15.
//

#ifndef LEARNFFMPEG_FFRESAMPLE_H
#define LEARNFFMPEG_FFRESAMPLE_H


#include "IResample.h"
struct SwrContext;

class FFResample : public IResample{
public:
    bool Open(FFParameter in, FFParameter out = FFParameter()) override;
    FFData Resample(FFData indata) override;
protected:
    SwrContext *actx;
};


#endif //LEARNFFMPEG_FFRESAMPLE_H
