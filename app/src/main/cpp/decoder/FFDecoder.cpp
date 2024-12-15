#include "FFDecoder.h"
#include "LogUtil.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include "libavcodec/jni.h"
}

#define TAG "FF-Decoder"

void FFDecoder::InitHard(void *vm) {
    if (av_jni_set_java_vm(vm, nullptr) != 0) {
        LOGE(TAG, "av_jni_set_java_vm failed");
    }
}

bool FFDecoder::Open(FFParameter para, bool  isHW) {
    LOGI(TAG, "Open start");
    if (!para.parameters) return false;
    AVCodec *codec =  avcodec_find_decoder(para.parameters->codec_id);
    if (isHW && codec->type == AVMEDIA_TYPE_VIDEO) {
        codec = avcodec_find_decoder_by_name("h264_mediacodec");
    }

    if (!codec) {
        LOGE(TAG, "avcodec find decoder %d failed", para.parameters->codec_id);
        return false;
    }
    if (isHW && codec->type == AVMEDIA_TYPE_VIDEO) {
        LOGI(TAG, "avcodec find video hw codec success");
    }

    codec_ctx = avcodec_alloc_context3(codec);
//    avcodec_parameters_from_context(para.parameters, codec_ctx);
    avcodec_parameters_to_context(codec_ctx, para.parameters);
    codec_ctx->thread_count = 8;
    int ret = avcodec_open2(codec_ctx, nullptr, nullptr);
    if (ret != 0) {
        LOGE(TAG, "avcodec_open2 failed: %s", av_err2str(ret));
        return false;
    }

    LOGI(TAG, "avcodec open success");

    if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        isAudio = true;
    } else {
        isAudio = false;
    }

    return true;
}

bool FFDecoder::SendPacket(FFData pkt) {
    if (!pkt.data || pkt.size < 0) return false;

    if (!codec_ctx) {
        LOGE(TAG, "%s failed: codec ctx is null", __FUNCTION__ );
        return false;
    }
    int ret = avcodec_send_packet(codec_ctx, (AVPacket *)pkt.data);
    if (ret != 0) {
        LOGE(TAG, "%s avcodec_send_packet failed: %s",__FUNCTION__, av_err2str(ret));
    }

    return true;
}

FFData FFDecoder::ReceivePkt() {
    FFData data;
    if (!codec_ctx) {
        LOGE(TAG, "receive pkt failed: codec ctx is null");
        return data;
    }
    if (!frame) {
        LOGI(TAG, "alloc frame");
        frame = av_frame_alloc();
    }

    int ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret != 0) {
//        LOGE(TAG, "%s avcodec_receive_frame failed: %s",__FUNCTION__, av_err2str(ret));
        return data;
    }

    data.data = (unsigned char*)frame;
    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        // yuv * height
        data.size = (frame->linesize[0] + frame->linesize[1] + frame->linesize[2]) * frame->height;
        data.width = frame->width;
        data.height = frame->height;
    } else {
        // 音频帧大小 样本字节数 * 单通道样本数 * 通道数
        data.size = av_get_bytes_per_sample((AVSampleFormat)frame->format)
                * frame->nb_samples * frame->channels;
    }
    memcpy(data.datas, frame->data, sizeof(data.datas));
    data.format = frame->format;
    return data;
}


