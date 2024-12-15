//
// Created by jinshan on 2024/10/14.
//

#include "GLVideoView.h"
#include "LogUtil.h"

#define TAG "FF-GLVideoView"

void GLVideoView::SetRender(void *win) {
    LOGI(TAG, "%s start", __FUNCTION__ );
    this->view = win;
}

void GLVideoView::Render(FFData data) {
//    LOGI(TAG, "Render");
    if (!view) return;
    if (!texture) {
        texture = ITexture::Create();
        texture->Init(view, static_cast<ITEXTURE_TYPE>(data.format));
    }
    texture->Draw(data.datas, data.width, data.height);
}
