//
// Created by jinshan on 2024/10/14.
//

#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include "FFEGL.h"
#include "LogUtil.h"

#define TAG "FF-EGL"

class CFFEGL : public FFEGL {
public:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;

    void Draw() override {
        if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
            LOGE(TAG, "Draw failed, display or surface is null");
            return;
        }
//        LOGI(TAG, "Draw");
        eglSwapBuffers(display, surface);
    }

    bool Init(void *win) override {
        ANativeWindow *window = (ANativeWindow *)win;

        /// EGL
        // 1. display创建和初始化
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            LOGE(TAG, "eglGetDisplay failed");
            return false;
        }
        if (EGL_TRUE != eglInitialize(display, 0, 0)) {
            LOGE(TAG, "eglInitialize failed");
            return false;
        }

        //2.surface
        // 2.1surface窗口配置
        // 输出配置
        EGLConfig config;
        EGLint configNum;
        EGLint configSpec[] = {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
        };
        if (EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum) ) {
            LOGE(TAG, "eglChooseConfig failed");
            return false;
        }
        // 创建surface
        surface = eglCreateWindowSurface(display, config, window, 0);
        if (surface == EGL_NO_SURFACE) {
            LOGE(TAG, "eglCreateWindowSurface failed");
            return false;
        }

        // 3. context 创建关联的上下文
        const EGLint  ctxAttr[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
        if (context == EGL_NO_CONTEXT) {
            LOGE(TAG, "eglCreateContext failed");
            return false;
        }
        // 关联
        if (EGL_TRUE != eglMakeCurrent(display, surface, surface, context)) {
            LOGE(TAG, "eglMakeCurrent failed");
            return false;
        }
        LOGI(TAG, "EGL init Success!");


        return true;
    }
};

FFEGL* FFEGL::GetInstance() {
    static CFFEGL egl;
    return &egl;
}