

#include "FFResample.h"
#include "LogUtil.h"
extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

#define TAG "FF-Resample"



bool FFResample::Open(FFParameter in, FFParameter out) {
    // 音频重采样上下文初始化
    actx =  swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(out.channels), // 输出声道数
                              AV_SAMPLE_FMT_S16, // 输出格式 s16 （s24 flt）<<----主要修改点
                              out.sample_rate, // 样本采样率（不变）
                              av_get_default_channel_layout(in.parameters->channels), // 输入声道数
                              (AVSampleFormat)in.parameters->format, // 输入格式
                              in.parameters->sample_rate, // 输入采样率
                              0, nullptr);
    int ret = swr_init(actx);
    if (ret != 0) {
        LOGE(TAG, "swr_init failed: %s[%d]", av_err2str(ret), ret);
        return false;
    }
    LOGI(TAG, "swr_init success");
    outChannels = in.parameters->channels;
    outFormat = AV_SAMPLE_FMT_S16;
    return true;
}

FFData FFResample::Resample(FFData indata) {
    FFData out;
    if (indata.size <= 0 || !indata.data) {
        LOGE(TAG, "indata is invalid");
        return out;
    }
    if (!actx) {
        LOGE(TAG, "context is null");
        return out;
    }
//    LOGD(TAG, "indata size %d ", indata.size);

    AVFrame *frame = (AVFrame *)indata.data;

    int outSize = outChannels * frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)outFormat);
    if (outSize <= 0) {
        LOGE(TAG, "outsize is invalid");
        return out;
    }
    out.Alloc(outSize);

    uint8_t *outArr[2] = {0};
    outArr[0] = out.data;
    int len = swr_convert(actx, outArr, frame->nb_samples, (const uint8_t **)frame->data, frame->nb_samples);
    if (len <= 0) {
        LOGE(TAG, "swr_convert failed");
        out.Drop();
        return out;
    }
//    LOGI(TAG, "swr_convert success = %d", len);

    return out;
}
