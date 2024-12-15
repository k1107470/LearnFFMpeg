//
// Created by jinshan on 2024/10/15.
//

#include "SLAudioPlayer.h"
#include "LogUtil.h"
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>


#define TAG "FF-SLAudioPlayer"

static SLObjectItf engineSL = nullptr;
static SLEngineItf eng = nullptr;
static SLObjectItf mix = nullptr;
static SLObjectItf player = nullptr; // 播放器
static SLPlayItf iPlayer = nullptr; // 播放器接口
static SLAndroidSimpleBufferQueueItf pcmQue = nullptr; // 缓冲队列

SLAudioPlayer::SLAudioPlayer() {
    buf = new unsigned char[1024 * 1024];
}

SLAudioPlayer::~SLAudioPlayer() {
    delete buf;
    buf = nullptr;
}


static SLEngineItf CreateSL() {
    SLresult ret;
    SLEngineItf en;
    ret = slCreateEngine(&engineSL, 0, 0, 0, 0, 0);
    if (ret != SL_RESULT_SUCCESS) return nullptr;

    ret = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if (ret != SL_RESULT_SUCCESS) return nullptr;

    ret = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &en);
    if (ret != SL_RESULT_SUCCESS) return nullptr;

    return en;
}

void SLAudioPlayer::PlayCall(void *bufQueue) {
    SLAndroidSimpleBufferQueueItf bf = (SLAndroidSimpleBufferQueueItf)bufQueue;
//    LOGE(TAG, "%s start", __FUNCTION__ );
    // 阻塞的
    FFData d = GetData();
    if (d.size <= 0) {
        LOGE(TAG, "GetData size is invalid");
        return;
    }
    if (!buf) {
        return;
    }
    memcpy(buf, d.data, d.size);

    (*bf)->Enqueue(bf, buf, d.size);
    d.Drop();
}

static void PcmCall(SLAndroidSimpleBufferQueueItf bf, void *context) {
    SLAudioPlayer *audioPlayer = (SLAudioPlayer *)context;
    if (!audioPlayer) {
        LOGE(TAG, "%s failed context is null", __FUNCTION__ );
        return;
    }
    audioPlayer->PlayCall((void *)bf);
}


bool SLAudioPlayer::StartPlay(FFParameter out) {
    SLresult ret;

    // 1. 创建引擎
    eng = CreateSL();
    if (eng == nullptr) {
        LOGE(TAG, "CreateSL failed ");
        return false;
    }
    LOGI(TAG, "CreateSL Success ");


    // 2. 创建混音器
    mix = nullptr;
    (*eng)->CreateOutputMix(eng, &mix, 0, nullptr, nullptr);
    (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (mix == nullptr) {
        LOGE(TAG, "CreateMix failed ");
        return false;
    }
    LOGI(TAG, "CreateMix Success ");


    SLDataLocator_OutputMix outmix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSink = {&outmix, 0};

    // 3. 配置音频信息
    // 缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue que = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10
    };
    // 音频格式
    SLDataFormat_PCM pcm =  {
            SL_DATAFORMAT_PCM,
            out.channels, // 通道数量：2表示立体声 <<--外部设置
            out.sample_rate * 1000, // 采样率：44.1 kHz <<--外部设置
            SL_PCMSAMPLEFORMAT_FIXED_16, // 位深度：16位
            SL_PCMSAMPLEFORMAT_FIXED_16, // 容器大小：16位
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 通道掩码
            SL_BYTEORDER_LITTLEENDIAN // 字节序：小端
    };
    SLDataSource dataSource = {&que, &pcm};

    // 4. 创建播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean  req[] = {SL_BOOLEAN_TRUE};

    ret = (*eng)->CreateAudioPlayer(eng, &player, &dataSource, &audioSink,
                                    sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (ret != SL_RESULT_SUCCESS) {
        LOGE(TAG, "CreateAudioPlayer failed");
        return false;
    }
    LOGI(TAG, "CreateAudioPlayer success");
    (*player)->Realize(player, SL_BOOLEAN_FALSE);
    // 获取player接口
    ret = (*player)->GetInterface(player, SL_IID_PLAY, &iPlayer);
    if (ret != SL_RESULT_SUCCESS) {
        LOGE(TAG, "GetInterface SL_IID_PLAY failed");
    }
    ret = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (ret != SL_RESULT_SUCCESS) {
        LOGE(TAG, "GetInterface SL_IID_BUFFERQUEUE failed");
    }

    // 设置回调函数， 播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue, PcmCall, this);

    // 设置播放状态
    (*iPlayer)->SetPlayState(iPlayer, SL_PLAYSTATE_PLAYING);

    (*pcmQue)->Enqueue(pcmQue, "", 1);
    return true;
}



