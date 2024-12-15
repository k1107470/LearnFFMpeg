//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_ITEXTURE_H
#define LEARNFFMPEG_ITEXTURE_H

enum ITEXTURE_TYPE {
    ITEXTURE_YUV420P = 0,
    ITEXTURE_NV12 = 25,
    ITEXTURE_NV21 = 26
};

class ITexture {
public:
    static ITexture *Create();
    virtual bool Init(void *win, ITEXTURE_TYPE type = ITEXTURE_YUV420P) = 0;
    virtual void Draw(unsigned char* data[], int width, int height) = 0;
};


#endif //LEARNFFMPEG_ITEXTURE_H
