//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_FFEGL_H
#define LEARNFFMPEG_FFEGL_H


class FFEGL {
public:
    virtual bool Init(void *win) = 0;
    virtual void Draw() = 0;
    static FFEGL* GetInstance();
};


#endif //LEARNFFMPEG_FFEGL_H
