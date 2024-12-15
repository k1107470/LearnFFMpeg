//
// Created by jinshan on 2024/10/15.
//

#ifndef LEARNFFMPEG_IAUDIOPLAYER_H
#define LEARNFFMPEG_IAUDIOPLAYER_H


#include "IObserver.h"
#include "FFParameter.h"
#include <list>
#include <mutex>

class IAudioPlayer : public IObserver{
public:
    // 缓冲后阻塞
    virtual void Update(FFData data);
    // 获取缓冲队列数据,如没有阻塞
    virtual FFData GetData();
    virtual bool StartPlay(FFParameter out) = 0;
    // 最大缓冲
    int maxFrame = 100;
protected:
    std::list<FFData> frames;
    std::mutex frameMutex;
};


#endif //LEARNFFMPEG_IAUDIOPLAYER_H
