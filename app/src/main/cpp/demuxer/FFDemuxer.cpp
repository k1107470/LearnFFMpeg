#include "FFDemuxer.h"
#include "Log/LogUtil.h"

extern "C" {
#include <libavformat/avformat.h>
}

#define TAG "FF-Demuxer"

#define PRINT_AV_ERROR(ret, fn) LOGE(TAG, "%s failed %s", fn, av_err2str(ret))
#define TIME_MS_BASE 1000


FFDemuxer::FFDemuxer() noexcept{
    static bool isFirst = true;
    if (isFirst) {
        isFirst = false;

        av_register_all();
        avcodec_register_all();
        avformat_network_init();
    }
}

bool FFDemuxer::Open(const char *url) {
    LOGI(TAG, "Open file %s start", url);

    int ret = avformat_open_input(&fmt_ctx, url, nullptr, nullptr);
    if (ret != 0) {
        PRINT_AV_ERROR(ret, "avformat_open_input");
        return false;
    }
    LOGI(TAG, "avformat_open_input %s success", url);

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret != 0) {
        PRINT_AV_ERROR(ret, "avformat_find_stream_info");
        return false;
    }

    this->duration = fmt_ctx->duration / (AV_TIME_BASE / TIME_MS_BASE);
    LOGI(TAG, "get media duration %ld", this->duration);
    GetVPara();
    GetAPara();
    return true;
};

FFData FFDemuxer::Read() {
//    LOGI(TAG, "%s start", __FUNCTION__ );
    if (!fmt_ctx) return FFData();
    FFData d;
    AVPacket *pkt = av_packet_alloc();
    int ret = av_read_frame(fmt_ctx, pkt);
    if (ret != 0) {
        av_packet_free(&pkt);
        pkt = nullptr;
//    if (ret == AVERROR_EOF) return FFData();
        return FFData();
    }
    d.data = (unsigned char*)(pkt);
    d.size = pkt->size;
    d.pts = pkt->pts;
//    LOGI(TAG, "read pkt pts = %ld size = %d", d.pts, d.size);

    if (pkt->stream_index == audioStream) {
        d.isAudio = true;
//        LOGE(TAG, "pkt is audio");
    } else if (pkt->stream_index == videoStream) {
        d.isAudio = false;
//        LOGE(TAG, "pkt is video");
    } else {
        av_packet_free(&pkt);
        pkt = nullptr;
        return FFData();
    }

    return d;
}

FFParameter FFDemuxer::GetVPara() {
    FFParameter para;
    if (!fmt_ctx) {
        LOGE(TAG, "find video failed, ctx is null");
        return para;
    }
    int streamID = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (streamID < 0) {
        LOGE(TAG, "find vidoe stream id failed");
        return para;
    }

    videoStream = streamID;
    LOGE(TAG, "set videoStream %d", streamID);

    para.parameters = fmt_ctx->streams[streamID]->codecpar;
    return para;
}

FFParameter FFDemuxer::GetAPara() {
    FFParameter para;
    if (!fmt_ctx) {
        LOGE(TAG, "find audio failed, ctx is null");
        return para;
    }
    int streamID = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamID < 0) {
        LOGE(TAG, "find audio stream id failed");
        return para;
    }

    audioStream = streamID;
    LOGE(TAG, "set audioStream %d", streamID);

    para.parameters = fmt_ctx->streams[streamID]->codecpar;
    para.parameters->channels = fmt_ctx->streams[streamID]->codecpar->channels;
    para.parameters->sample_rate = fmt_ctx->streams[streamID]->codecpar->sample_rate;
    return para;
}
