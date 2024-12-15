//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_FFSHADER_H
#define LEARNFFMPEG_FFSHADER_H

enum FFSHADER_TYPE {
    FFSHADER_YUV420P = 0,
    FFSHADER_NV12 = 25,
    FFSHADER_NV21 = 26
};

class FFShader {
public:
    virtual bool Init(FFSHADER_TYPE type = FFSHADER_YUV420P);
    // 获取材质并映射到内存
    virtual void GetAndBindTextures(unsigned int index, int width, int height, unsigned char* buf, bool isAlpha = false);
    virtual void Draw();
protected:
    unsigned int vsh = 0;
    unsigned int fsh = 0;
    unsigned int program = 0;
    unsigned int texts[100] = {0};
};


#endif //LEARNFFMPEG_FFSHADER_H
