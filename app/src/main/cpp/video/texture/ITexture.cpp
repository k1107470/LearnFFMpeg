#include "LogUtil.h"
#include "EGL/FFEGL.h"
#include "shader/FFShader.h"
#include "ITexture.h"

#define TAG "FF-Texture"

class FFTexture : public ITexture {
public:
    FFShader sh;
    ITEXTURE_TYPE m_type;
    bool Init(void *win, ITEXTURE_TYPE type) override {
        this->m_type = type;
        if (!win) {
            LOGE(TAG, "init failed window is null");
            return false;
        }
        if (!FFEGL::GetInstance()->Init(win)) {
            return false;
        }
        sh.Init((FFSHADER_TYPE)type);
        return true;
    }
    void Draw(unsigned char* data[], int width, int height) override {
        // yuv420p
        sh.GetAndBindTextures(0, width, height, data[0]);
        if (m_type == ITEXTURE_YUV420P) {
            sh.GetAndBindTextures(1, width / 2, height / 2, data[1]);
            sh.GetAndBindTextures(2, width / 2, height / 2, data[2]);
        } else {
            sh.GetAndBindTextures(1, width / 2, height / 2, data[1], true);
        }
        sh.Draw();
        FFEGL::GetInstance()->Draw();
    }
};

ITexture *ITexture::Create() {
    LOGE(TAG, "create");
    return new FFTexture;
}