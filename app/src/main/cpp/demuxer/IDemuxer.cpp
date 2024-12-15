//
// Created by jinshan on 2024/10/13.
//

#include "IDemuxer.h"
#include "LogUtil.h"

#define TAG "FF-IDemuxer"

void IDemuxer::Main() {
    LOGI(TAG, "IDemuxer Thread start");
    while(!isExit) {
        FFData d =  Read();
        if (d.size <= 0) break;
        if (d.size > 0) {
            Notify(d);
        }
    }
}
