//
// Created by jinshan on 2024/10/15.
//

#ifndef LEARNFFMPEG_SLAUDIOPLAYER_H
#define LEARNFFMPEG_SLAUDIOPLAYER_H


#include "IAudioPlayer.h"

class SLAudioPlayer : public IAudioPlayer{
public:
    bool StartPlay(FFParameter out) override;
    void PlayCall(void *bufQueue);

    SLAudioPlayer();
    virtual ~SLAudioPlayer();
protected:
    unsigned char* buf = 0;
};


#endif //LEARNFFMPEG_SLAUDIOPLAYER_H
