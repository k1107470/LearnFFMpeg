//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_GLVIDEOVIEW_H
#define LEARNFFMPEG_GLVIDEOVIEW_H


#include "IVideoView.h"
#include "texture/ITexture.h"

class GLVideoView : public IVideoView{
public:
    void SetRender(void *win) override;
    void Render(FFData data) override;
protected:
    void *view = nullptr;
    ITexture *texture = nullptr;
};


#endif //LEARNFFMPEG_GLVIDEOVIEW_H
