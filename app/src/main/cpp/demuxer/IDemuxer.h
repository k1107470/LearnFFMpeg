//
// Created by jinshan on 2024/10/13.
//

#ifndef LEARNFFMPEG_IDEMUXER_H
#define LEARNFFMPEG_IDEMUXER_H


#include "FFData.h"
#include "thread/FFThread.h"
#include "IObserver.h"
#include "FFParameter.h"

class IDemuxer : public IObserver{
public:
    virtual bool Open(const char *url) = 0;

    virtual FFParameter GetVPara() = 0;
    virtual FFParameter GetAPara() = 0;

    virtual FFData Read() = 0;

    long duration = 0;
protected:
    void Main() override;

};


#endif //LEARNFFMPEG_IDEMUXER_H
