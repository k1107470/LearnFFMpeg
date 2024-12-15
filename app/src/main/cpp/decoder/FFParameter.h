//
// Created by jinshan on 2024/10/14.
//

#ifndef LEARNFFMPEG_FFPARAMETER_H
#define LEARNFFMPEG_FFPARAMETER_H

struct AVCodecParameters;

class FFParameter {
public:
    AVCodecParameters *parameters;
    unsigned int channels = 2;
    unsigned int sample_rate = 44100;
};


#endif //LEARNFFMPEG_FFPARAMETER_H
