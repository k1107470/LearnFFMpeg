//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_IDECODER_H
#define LEARNFFMPEG_IDECODER_H

#include "FFParameter.h"
#include "IObserver.h"
#include <list>

class IDecoder : public IObserver{
public:
    virtual bool Open(FFParameter para, bool isHW = false) = 0;
    virtual bool SendPacket(FFData pkt) = 0;
    virtual FFData ReceivePkt() = 0;
    void Update(FFData pkt) override;
    bool isAudio = false;
    int maxPacks = 200;
protected:
    void Main() override;
    std::list<FFData> packs;
    std::mutex packsMutex;
};


#endif //LEARNFFMPEG_IDECODER_H
