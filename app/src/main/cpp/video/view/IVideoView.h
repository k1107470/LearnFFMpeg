//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_IVIDEOVIEW_H
#define LEARNFFMPEG_IVIDEOVIEW_H


#include "FFData.h"
#include "IObserver.h"

class IVideoView : public IObserver{
public:
    virtual void SetRender(void *win) = 0;
    virtual void Render(FFData data) = 0;
    virtual void Update(FFData data);
};


#endif //LEARNFFMPEG_IVIDEOVIEW_H
