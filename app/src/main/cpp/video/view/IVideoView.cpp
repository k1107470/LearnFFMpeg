//
// Created by jinshan on 2024/10/14.
//

#include "IVideoView.h"
#include "LogUtil.h"


#define TAG "FF-IVideoView"

void IVideoView::Update(FFData data) {
//    LOGI(TAG, "Update");
    this->Render(data);
}
