#include <cstdio>
#include <cstring>
#include <jni.h>
#include <android/native_window_jni.h>
//
//extern "C" {
//#include <libavcodec/jni.h>
//}

#include "Log/LogUtil.h"
#include "demuxer/IDemuxer.h"
#include "demuxer/FFDemuxer.h"

#include "IDecoder.h"
#include "FFDecoder.h"

#include "EGL/FFEGL.h"
#include "shader/FFShader.h"
#include "view/IVideoView.h"
#include "view/GLVideoView.h"

#include "FFResample.h"
#include "IAudioPlayer.h"
#include "SLAudioPlayer.h"

#define TAG "FF-PlayerJNI"


#define PACKAGE_NAME "com/example/learnffmpeg/FFBase"
#define JAVA_STRING "Ljava/lang/String;"  // 定义宏
#define JAVA_OBJECT "Ljava/lang/Object;"  // 定义宏

jstring JNICALL autoplay(JNIEnv *env, jobject /*thiz*/);
void JNICALL initView(JNIEnv *env, jobject thiz, jobject surface);

static JNINativeMethod methods[] = {
        {"play", "()" JAVA_STRING "", (void *) autoplay},
        {"initView", "(" JAVA_OBJECT ")V", (void *)initView}
};


class TestObs : public IObserver {
public:
    void Update(FFData data) {
//        LOGI(TAG, "testObs update data size = %d", data.size);
    }
};

IVideoView *view = nullptr;

jstring JNICALL autoplay(JNIEnv *env, jobject /*thiz*/) {
    LOGI(TAG, "%s start", __FUNCTION__ );
    const char * str = "hello world!";

//    TestObs *tobs = new TestObs();
    IDemuxer *demuxer = new FFDemuxer;
    demuxer->Open("/sdcard/wukong_1080.mp4");


    IDecoder *vdecoder = new FFDecoder;
    IDecoder *adecoder = new FFDecoder;

    vdecoder->Open(demuxer->GetVPara(), true);
    adecoder->Open(demuxer->GetAPara());

//    demuxer->AddObs(tobs);

    demuxer->AddObs(adecoder);
    demuxer->AddObs(vdecoder);

    view = new GLVideoView();
    vdecoder->AddObs(view);

    IResample *resample = new FFResample();
    FFParameter outPara = demuxer->GetAPara();
    resample->Open(demuxer->GetAPara(), outPara);
    adecoder->AddObs(resample);

    IAudioPlayer *audioPlayer = new SLAudioPlayer();
    audioPlayer->StartPlay(outPara);
    resample->AddObs(audioPlayer);

    demuxer->Start();
    vdecoder->Start();
//    adecoder->Start();

//    FFSleep(100);
//    demuxer->Stop();
//    while (1) {
//        demuxer->Read();
//    }

    return env->NewStringUTF(str);
}

void JNICALL initView(JNIEnv *env, jobject thiz, jobject surface) {
    LOGD(TAG, "%s start", __FUNCTION__ );
    ANativeWindow * window =  ANativeWindow_fromSurface(env, surface);

    view->SetRender(window);
//    FFEGL::GetInstance()->Init(window);
//    FFShader shader;
//    shader.Init();

}


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI(TAG, "%s start", __FUNCTION__);

    JNIEnv  *env;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE(TAG, "vm get env failed");
        return JNI_ERR;
    }
    jclass clazz = env->FindClass(PACKAGE_NAME);
    if (!clazz) {
        LOGE(TAG, "cannot find base class");
        return JNI_ERR;
    }
    // 注册native方法
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != JNI_OK) {
        LOGE(TAG, "register native failed");
        return JNI_ERR;
    }
    FFDecoder::InitHard(vm);
    LOGI(TAG, "%s success!", __FUNCTION__ );
    return JNI_VERSION_1_4;
}