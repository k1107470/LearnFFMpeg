#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <cstdio>
#include <cstring>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>

#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>

#include "util/Log/LogUtil.h"
//#include "audioPlayer/audioPlayer.h"

//由于 FFmpeg 库是 C 语言实现的，告诉编译器按照 C 的规则进行编译
extern "C" {
#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/jni.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libavfilter/version.h>
#include <libswresample/version.h>
#include <libswscale/version.h>


#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
};


#define TAG "native-lib"


#define PACKAGE_NAME "com/example/learnffmpeg/FFBase"
#define JAVA_STRING "Ljava/lang/String;"  // 定义宏
#define JAVA_OBJECT "Ljava/lang/Object;"  // 定义宏



#define GET_STR(x) #x
static const char* vertextShader = GET_STR(
    attribute vec4 aPosition; // 顶点坐标
    attribute vec2 aTexCoord; //材质顶点坐标
    varying vec2 vTexCoord;   // 输出的材质坐标
    void main() {
        vTexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y); // GL坐标系y轴和纹理坐标系y轴相反
        gl_Position = aPosition; // 设置顶点位置
    }
);

static const char* fragYUV420P = GET_STR(
    precision mediump float; // 精度
    varying vec2 vTexCoord; // 顶点着色器传入的坐标
    uniform sampler2D yTexture; // 输入的材质
    uniform sampler2D uTexture;
    uniform sampler2D vTexture;
    void main() {
        vec3 yuv;
        vec3 rgb;
        yuv.r = texture2D(yTexture, vTexCoord).r;
        yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;
        yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;
        rgb = mat3(1.0,          1.0,     1.0,
                   0.0,     -0.39465, 2.03211,
                   1.13983, -0.58060,     0.0) * yuv;
        gl_FragColor = vec4(rgb, 1.0);
    }
);


GLuint InitShader(const char* code, GLint type) {
    // 初始化shader
    GLuint sh = glCreateShader(type);
    if (sh == 0) {
        LOGE(TAG, "glCreateShader 0x%x failed", type);
        return 0;
    }
    // 加载shader
    glShaderSource(sh, 1, &code, 0);
    // 编译shader
    glCompileShader(sh);
    // 获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        // 获取错误信息的长度
        GLint infoLogLength;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &infoLogLength);

        // 分配空间保存错误信息
        char* infoLog = new char[infoLogLength];

        // 获取错误信息
        glGetShaderInfoLog(sh, infoLogLength, nullptr, infoLog);

        // 打印错误信息
        LOGE(TAG, "compile failed: %s", infoLog);

        // 清理资源
        delete[] infoLog;
        glDeleteShader(sh);

        return 0;  // 返回 0 表示编译失败
    }

    return sh;
}


jstring JNICALL stringFromJNI(JNIEnv *env, jobject /*thiz*/);

jboolean JNICALL open(JNIEnv *env, jobject thiz, jstring url);

jboolean JNICALL open2(JNIEnv *env, jobject thiz, jstring url, jobject surface);

void JNICALL playAudio(JNIEnv *env, jobject thiz, jstring url);

void JNICALL playYUV(JNIEnv *env, jobject thiz, jstring url, jobject surface);

void printStream(const AVFormatContext *fmt_ctx);

void PcmCall(SLAndroidSimpleBufferQueueItf bf, void *) {
    static FILE *fp = nullptr;
    static char *buf = nullptr;
    if (!buf) {
        buf = new char[1024 * 1024];
    }
    if (!fp) {
        fp = fopen("/sdcard/pcm441.pcm", "rb");
    }
    if (!fp) {
        return;
    }
    if (feof(fp) == 0) {
       size_t len =  fread(buf, 1, 1024, fp);
       if (len > 0) {
           (*bf)->Enqueue(bf, buf, len);
       }
    }


    LOGD(TAG, "PcmCall");
}

static double r2d(AVRational r) {
    return ((r.num == 0) || (r.den == 0)) ? 0 : ((double) r.num) / ((double) r.den);
}

static JNINativeMethod methods[] = {
        {"stringFromJNI", "()" JAVA_STRING "",  (void *) stringFromJNI},
        {"open",          "(" JAVA_STRING ")Z", (void *) open},
        {"open2",         "(" JAVA_STRING JAVA_OBJECT ")Z", (void *) open2},
        {"playAudio",     "(" JAVA_STRING ")V", (void *) playAudio},
        {"playYUV",       "(" JAVA_STRING JAVA_OBJECT ")V", (void *) playYUV}
};

void JNICALL playYUV(JNIEnv *env, jobject thiz, jstring url, jobject surface) {
    const char* m_url = env->GetStringUTFChars(url, 0);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    int width = 424;
    int height = 240;

    FILE *fp = fopen(m_url, "rb");
    if (!fp) {
        LOGE(TAG, "fopen %s failed: %s", m_url, strerror(errno));
        return;
    }


    /// EGL
    // 1. display创建和初始化
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGE(TAG, "eglGetDisplay failed");
        return;
    }
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        LOGE(TAG, "eglInitialize failed");
        return;
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
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };
    if (EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum) ) {
        LOGE(TAG, "eglChooseConfig failed");
        return;
    }
    // 创建surface
    EGLSurface winSurface = eglCreateWindowSurface(display, config, window, 0);
    if (winSurface == EGL_NO_SURFACE) {
        LOGE(TAG, "eglCreateWindowSurface failed");
        return;
    }

    // 3. context 创建关联的上下文
    const EGLint  ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    EGLContext  context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGE(TAG, "eglCreateContext failed");
        return;
    }
    // 关联
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        LOGE(TAG, "eglMakeCurrent failed");
        return;
    }
    LOGD(TAG, "EGL init Success!");

    // 顶点着色器、片段着色器初始化
    GLuint vsh = InitShader(vertextShader, GL_VERTEX_SHADER);
    GLuint fsh = InitShader(fragYUV420P, GL_FRAGMENT_SHADER);
    // 创建渲染程序
    GLuint program = glCreateProgram();
    if (program == 0) {
        LOGE(TAG, "glCreateProgram failed");
        return;
    }
    // 渲染程序中加入着色器
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    //链接程序
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        LOGE(TAG, "glLinkProgram failed");
        return;
    }
    // 激活程序
    glUseProgram(program);
    LOGD(TAG, "glLinkProgram success!");
    /////////////////////////////////////////

    // 加入顶点数据
    static float vet[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };
    GLuint aPos = glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(aPos);
    // 传递顶点
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 12, vet);

    // 加入材质坐标
    static float txt[] = {
            1.0f, 0.0f, // 右下
            0.0f, 0.0f, // 左下
            1.0f, 1.0f, // 右上
            0.0f, 1.0f // 左上
    };
    GLuint aTex = glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(aTex);
    // 传递纹理
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 8, txt);

    // 材质纹理初始化
    // 设置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0); // 对应纹理的第1层
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1); // 对应纹理的第2层
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2); // 对应纹理的第3层

    // 创建opengl材质
    GLuint texture[3] = {0};
    glGenTextures(3, texture);// 创建三个纹理

    //设置纹理0属性
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    // 过滤器，缩小、放大使用线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE, // gpu内部格式，亮度、灰度图，这里表示使用Y
                 width, height,
                 0, // 边框
                 GL_LUMINANCE, // 数据的像素格式，和上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr // 纹理的数据
                 );

    //设置纹理1属性
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    // 过滤器，缩小、放大使用线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE, // gpu内部格式，亮度、灰度图，这里表示使用Y
                 width / 2, height / 2,
                 0, // 边框
                 GL_LUMINANCE, // 数据的像素格式，和上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr // 纹理的数据
    );

    //设置纹理2属性
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    // 过滤器，缩小、放大使用线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE, // gpu内部格式，亮度、灰度图，这里表示使用Y
                 width / 2, height / 2,
                 0, // 边框
                 GL_LUMINANCE, // 数据的像素格式，和上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr // 纹理的数据
    );

    ///////////////////////
    // 纹理的修改和显示
    uint8_t *buf[3] = {0};
    buf[0] = new uint8_t[width * height];
    buf[1] = new uint8_t[width * height / 4];
    buf[2] = new uint8_t[width * height / 4];

    while(true) {
        if (feof(fp) == 0) {
            fread(buf[0], 1, width * height, fp);
            fread(buf[1], 1, width * height / 4, fp);
            fread(buf[2], 1, width * height / 4, fp);
        } else {
            break;
        }

        // 激活第一层纹理 ，对应yTexture, 绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        // 替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[0]);

        // 激活第一层纹理 ，对应yTexture, 绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        // 替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[1]);

        // 激活第一层纹理 ，对应yTexture, 绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        // 替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[2]);

        // 绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // 窗口显示
        eglSwapBuffers(display, winSurface);
    }
    LOGD(TAG, "%s end", __FUNCTION__ );
}

void JNICALL playAudio(JNIEnv *env, jobject thiz, jstring url) {

    SLresult ret;

    // 1. 创建引擎
    SLObjectItf engineObj = nullptr;
    SLEngineItf eng = nullptr;
    slCreateEngine(&engineObj, 0, 0, 0, 0, 0);
    (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &eng);


//    SLEngineItf eng = CreateSL();
    if (eng == nullptr) {
        LOGE(TAG, "CreateSL failed ");
    } else {
        LOGD(TAG, "CreateSL Success ");
    }

    // 2. 创建混音器
    SLObjectItf mix = nullptr;
    (*eng)->CreateOutputMix(eng, &mix, 0, nullptr, nullptr);
    (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
//    SLObjectItf mix = CreateMix(eng);
    if (mix == nullptr) {
        LOGE(TAG, "CreateMix failed ");
    } else {
        LOGD(TAG, "CreateMix Success ");
    }

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
            2, // 通道数量：2表示立体声
            SL_SAMPLINGRATE_44_1, // 采样率：44.1 kHz
            SL_PCMSAMPLEFORMAT_FIXED_16, // 位深度：16位
            SL_PCMSAMPLEFORMAT_FIXED_16, // 容器大小：16位
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 通道掩码
            SL_BYTEORDER_LITTLEENDIAN // 字节序：小端
    };
    SLDataSource dataSource = {&que, &pcm};

    // 4. 创建播放器
    SLObjectItf player = nullptr; // 播放器
    SLPlayItf iPlayer = nullptr; // 播放器接口
    SLAndroidSimpleBufferQueueItf pcmQue = nullptr; // 缓冲队列
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean  req[] = {SL_BOOLEAN_TRUE};

    ret = (*eng)->CreateAudioPlayer(eng, &player, &dataSource, &audioSink,
                                    sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (ret != SL_RESULT_SUCCESS) {
        LOGE(TAG, "CreateAudioPlayer failed");
    } else {
        LOGD(TAG, "CreateAudioPlayer success");
    }
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
    (*pcmQue)->RegisterCallback(pcmQue, PcmCall, nullptr);

    // 设置播放状态
    (*iPlayer)->SetPlayState(iPlayer, SL_PLAYSTATE_PLAYING);
}


jstring JNICALL stringFromJNI(JNIEnv *env, jobject /*thiz*/) {
    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavformat : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibavfilter : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibswresample : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    strcat(strBuffer, "\navcodec_configure : \n");
    strcat(strBuffer, avcodec_configuration());
    strcat(strBuffer, "\navcodec_license : ");
    strcat(strBuffer, avcodec_license());
    LOGD(TAG, "GetFFmpegVersion\n%s", strBuffer);
    return env->NewStringUTF(strBuffer);
}


jboolean JNICALL open(JNIEnv *env, jobject thiz, jstring url) {
    // TODO: implement open()
    LOGD(TAG, "%s", __FUNCTION__);
    const char *m_url = env->GetStringUTFChars(url, 0);

//    av_register_all();
    // 初始化网络
    avformat_network_init();

    // 初始化解码器
//    avcodec_register_all();



    int videoStream = -1;
    int audioStream = -1;



    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, m_url, nullptr, nullptr);
    if (ret != 0) {
        LOGE(TAG, "avformat_open_input %s failed: %s", m_url, av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        env->ReleaseStringUTFChars(url, m_url);
        return JNI_FALSE;
    }
    // 探测流信息
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret != 0) {
        LOGE(TAG, "avformat find stream info failed: %s", av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        env->ReleaseStringUTFChars(url, m_url);
        return JNI_FALSE;
    }

    LOGD(TAG, "duration: %ld, nb_streams = %u", fmt_ctx->duration, fmt_ctx->nb_streams);

//    printStream(fmt_ctx);
    videoStream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // 初始化音视频解码器
    AVCodec *codec =  avcodec_find_decoder(fmt_ctx->streams[videoStream]->codecpar->codec_id);
//    AVCodec *codec = avcodec_find_decoder_by_name("h264_mediacodec");
    AVCodec *acodec = avcodec_find_decoder(fmt_ctx->streams[audioStream]->codecpar->codec_id);
    if (codec == nullptr) {
        LOGE(TAG, "video codec find failed");
    }
    if (acodec == nullptr) {
        LOGE(TAG, "audio codec find failed");
    }

    // 解码器上下文
    AVCodecContext *vc = avcodec_alloc_context3(codec);
    AVCodecContext *ac = avcodec_alloc_context3(acodec);
    // 解码器上下文赋值
    avcodec_parameters_to_context(vc, fmt_ctx->streams[videoStream]->codecpar);
    avcodec_parameters_to_context(ac, fmt_ctx->streams[audioStream]->codecpar);
    // 设置音视频解码线程数
    vc->thread_count = 4;
    ac->thread_count = 1;
    // 打开解码器
    ret = avcodec_open2(vc, codec, nullptr);
    if (ret != 0) {
        LOGE(TAG, "open video codec failed: %s[%d]", av_err2str(ret), ret);
    }

    ret = avcodec_open2(ac, acodec, nullptr);
    if (ret != 0) {
        LOGE(TAG, "open audio codec failed: %s", av_err2str(ret));
    }


    // 初始化视频像素格式转换上下文
    SwsContext *vctx = nullptr;
    int outWidth = 1280;
    int outHeight = 720;



    AVPacket *packet = av_packet_alloc();
    AVFrame *vFrame = av_frame_alloc();
    AVFrame *aFrame = av_frame_alloc();

    // 视频重采样缓冲区初始化
    char *rgb = new char[1920 * 1080 *4]; // 视频输出数据

    char *pcm = new char[48000 * 4 * 2]; // 音频输出数据
    // 音频重采样上下文初始化
    SwrContext *actx =  swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(ac->channels), // 输出声道数
                              AV_SAMPLE_FMT_S16, // 输出格式 s16 （s24 flt）
                              ac->sample_rate, // 样本采样率（不变）
                              av_get_default_channel_layout(ac->channels), // 输入声道数
                              ac->sample_fmt, // 输入格式
                              ac->sample_rate, // 输入采样率
                              0, nullptr);
    ret = swr_init(actx);
    if (ret != 0) {
        LOGE(TAG, "swr_init failed: %s[%d]", av_err2str(ret), ret);
    }

//    int receiveFlag = 0;
    while(true) {
        ret = av_read_frame(fmt_ctx, packet);
        if (ret != 0 ) {
            LOGD(TAG, "read stream end");
//            av_seek_frame(fmt_ctx, videoStream, fmt_ctx->duration / 3, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
//            break;
        }


        if (packet->stream_index == videoStream) {
            ret = avcodec_send_packet(vc, packet);
            if (ret != 0) {
                LOGE(TAG, "avcodec send video packet failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }

            ret = avcodec_receive_frame(vc, vFrame);
            if (ret != 0) {
                LOGE(TAG, "avcodec receive vFrame failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }

            LOGD(TAG, "receive vFrame keyframe[%d] pts: %ld", vFrame->key_frame, vFrame->pts);
            vctx = sws_getCachedContext(vctx,
                                        vFrame->width, vFrame->height, (AVPixelFormat) vFrame->format,
                                        outWidth, outHeight, AV_PIX_FMT_RGBA,
                                        0, nullptr, nullptr, nullptr);
            if (!vctx) {
                LOGE(TAG, "sws_getCachedContext failed ");
            }


            // 视频重采样
            uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
            data[0] = (uint8_t *) (rgb);
            int lines[AV_NUM_DATA_POINTERS] = {0};
            lines[0] = outWidth * 4;

            int h = sws_scale(vctx,
                              vFrame->data, // 输入的视频数据
                              vFrame->linesize, // 输入的视频格式储存数据的平面数组，例如yuv420p 采用三个平面依次存储y、u、v
                              0, // 深度，二维视频为0
                              vFrame->height, // 输入的帧的高度
                              data, // 输出的数据
                              lines // 输出的视频格式储存数据的平面数组
                              );
            LOGD(TAG, "sws_scale h = %d", h);

//        LOGD(TAG, "read vFrame streamID = %d  pts = %ld size =  %d flag = %d",
//             packet->stream_index, packet->pts, packet->size, packet->flags);
        } else if(packet->stream_index == audioStream) {
            ret = avcodec_send_packet(ac, packet);
            if (ret != 0) {
                LOGE(TAG, "avcodec send audio packet failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }

            ret = avcodec_receive_frame(ac, aFrame);
            if (ret != 0) {
                LOGE(TAG, "avcodec receive aFrame failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }
            // 音频重采样
            uint8_t *out[2] = {0};
            out[0] = (uint8_t *)pcm;
//            LOGD(TAG, "receive aFrame pts: %ld", vFrame->pts);
            int len = swr_convert(actx, out,
                                  aFrame->nb_samples, // 输出的单个声道的采样率
                                  (const uint8_t **)(aFrame->data),  // 输入的音频数据
                                  aFrame->nb_samples  // 输入的音频单个声道的采样率
                                  );
            LOGD(TAG, "swr_convert h = %d", len);
        }




        av_packet_unref(packet);
    }

    LOGD(TAG, "release resources");
    // 释放资源
    av_frame_free(&vFrame);
    av_frame_free(&aFrame);
    av_packet_free(&packet);
    avcodec_free_context(&vc);
    avcodec_free_context(&ac);
    avformat_close_input(&fmt_ctx);
    delete[] rgb;
    rgb = nullptr;
    LOGD(TAG, "release end");
//    end:


    env->ReleaseStringUTFChars(url, m_url);
    return JNI_OK;
}


JNIEXPORT jboolean JNICALL open2(JNIEnv *env, jobject thiz, jstring url, jobject surface) {
    LOGD(TAG, "%s", __FUNCTION__);
    const char *m_url = env->GetStringUTFChars(url, 0);

    avformat_network_init();

    int videoStream = -1;
    int audioStream = -1;

    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, m_url, nullptr, nullptr);
    if (ret != 0) {
        LOGE(TAG, "avformat_open_input %s failed: %s", m_url, av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        env->ReleaseStringUTFChars(url, m_url);
        return JNI_FALSE;
    }
    // 探测流信息
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret != 0) {
        LOGE(TAG, "avformat find stream info failed: %s", av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        env->ReleaseStringUTFChars(url, m_url);
        return JNI_FALSE;
    }

    LOGD(TAG, "duration: %ld, nb_streams = %u", fmt_ctx->duration, fmt_ctx->nb_streams);

    videoStream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // 初始化音视频解码器
    AVCodec *vCodec =  avcodec_find_decoder(fmt_ctx->streams[videoStream]->codecpar->codec_id);
//    AVCodec *vCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    AVCodec *aCodec = avcodec_find_decoder(fmt_ctx->streams[audioStream]->codecpar->codec_id);
    if (vCodec == nullptr) {
        LOGE(TAG, "video vCodec find failed");
    }
    if (aCodec == nullptr) {
        LOGE(TAG, "audio vCodec find failed");
    }

    // 解码器上下文
    AVCodecContext *vCodecCtx = avcodec_alloc_context3(vCodec);
    AVCodecContext *aCodecCtx = avcodec_alloc_context3(aCodec);
    // 解码器上下文赋值
    avcodec_parameters_to_context(vCodecCtx, fmt_ctx->streams[videoStream]->codecpar);
    avcodec_parameters_to_context(aCodecCtx, fmt_ctx->streams[audioStream]->codecpar);
    // 设置音视频解码线程数
    vCodecCtx->thread_count = 4;
    aCodecCtx->thread_count = 1;
    // 打开解码器
    ret = avcodec_open2(vCodecCtx, vCodec, nullptr);
    if (ret != 0) {
        LOGE(TAG, "open video vCodec failed: %s[%d]", av_err2str(ret), ret);
    }

    ret = avcodec_open2(aCodecCtx, aCodec, nullptr);
    if (ret != 0) {
        LOGE(TAG, "open audio vCodec failed: %s", av_err2str(ret));
    }


    // 初始化视频像素格式转换上下文
    SwsContext *vctx = nullptr;
    int outWidth = 1280;
    int outHeight = 720;

    AVPacket *packet = av_packet_alloc();
    AVFrame *vFrame = av_frame_alloc();
    AVFrame *aFrame = av_frame_alloc();

    // 视频重采样缓冲区初始化
    char *rgb = new char[1920 * 1080 *4]; // 视频输出数据

    char *pcm = new char[48000 * 4 * 2]; // 音频输出数据
    // 音频重采样上下文初始化
    SwrContext *actx =  swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(aCodecCtx->channels), // 输出声道数
                              AV_SAMPLE_FMT_S16, // 输出格式 s16 （s24 flt）
                              aCodecCtx->sample_rate, // 样本采样率（不变）
                              av_get_default_channel_layout(aCodecCtx->channels), // 输入声道数
                              aCodecCtx->sample_fmt, // 输入格式
                              aCodecCtx->sample_rate, // 输入采样率
                              0, nullptr);
    ret = swr_init(actx);
    if (ret != 0) {
        LOGE(TAG, "swr_init failed: %s[%d]", av_err2str(ret), ret);
    }

    // 视频显示窗口初始化
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    int vWidth = vCodecCtx->width;
    int vHeight = vCodecCtx->height;
    ANativeWindow_setBuffersGeometry(window, outWidth, outHeight, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;


    while(true) {
        ret = av_read_frame(fmt_ctx, packet);
        if (ret != 0 ) {
            LOGD(TAG, "read stream end");
        }

        if (packet->stream_index == videoStream) {
            ret = avcodec_send_packet(vCodecCtx, packet);
            if (ret != 0) {
                LOGE(TAG, "avcodec send video packet failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }

            ret = avcodec_receive_frame(vCodecCtx, vFrame);
            if (ret != 0) {
                LOGE(TAG, "avcodec receive vFrame failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }

            LOGD(TAG, "receive vFrame keyframe[%d] pts: %ld", vFrame->key_frame, vFrame->pts);
            vctx = sws_getCachedContext(vctx,
                                        vFrame->width, vFrame->height, (AVPixelFormat) vFrame->format,
                                        outWidth, outHeight, AV_PIX_FMT_RGBA,
                                        0, nullptr, nullptr, nullptr);
            if (!vctx) {
                LOGE(TAG, "sws_getCachedContext failed ");
            }

            // 视频重采样
            uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
            data[0] = (uint8_t *) (rgb);
            int lines[AV_NUM_DATA_POINTERS] = {0};
            lines[0] = outWidth * 4;

            int h = sws_scale(vctx,
                              vFrame->data, // 输入的视频数据
                              vFrame->linesize, // 输入的视频格式储存数据的平面数组，例如yuv420p 采用三个平面依次存储y、u、v
                              0, // 深度，二维视频为0
                              vFrame->height, // 输入的帧的高度
                              data, // 输出的数据
                              lines // 输出的视频格式储存数据的平面数组
            );
            LOGD(TAG, "sws_scale h = %d", h);
            if (h > 0) {
                ANativeWindow_lock(window, &windowBuffer, nullptr); // 锁定窗口
                uint8_t *dst = (uint8_t *)windowBuffer.bits;
                memcpy(dst, rgb, outWidth * outHeight * 4); // rgb数据拷贝到窗口缓冲
                ANativeWindow_unlockAndPost(window); // 解锁窗口显示图像
            }

//        LOGD(TAG, "read vFrame streamID = %d  pts = %ld size =  %d flag = %d",
//             packet->stream_index, packet->pts, packet->size, packet->flags);
        }
        else if(packet->stream_index == audioStream) {
            ret = avcodec_send_packet(aCodecCtx, packet);
            if (ret != 0) {
                LOGE(TAG, "avcodec send audio packet failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }

            ret = avcodec_receive_frame(aCodecCtx, aFrame);
            if (ret != 0) {
                LOGE(TAG, "avcodec receive aFrame failed: %s[%d]", av_err2str(ret), ret);
                continue;
            }
            // 音频重采样
            uint8_t *out[2] = {0};
            out[0] = (uint8_t *)pcm;
//            LOGD(TAG, "receive aFrame pts: %ld", vFrame->pts);
            int len = swr_convert(actx, out,
                                  aFrame->nb_samples, // 输出的单个声道的采样率
                                  (const uint8_t **)(aFrame->data),  // 输入的音频数据
                                  aFrame->nb_samples  // 输入的音频单个声道的采样率
            );
            LOGD(TAG, "swr_convert h = %d", len);
        }

        av_packet_unref(packet);
    }

    LOGD(TAG, "release resources");
    // 释放资源
    av_frame_free(&vFrame);
    av_frame_free(&aFrame);
    av_packet_free(&packet);
    avcodec_free_context(&vCodecCtx);
    avcodec_free_context(&aCodecCtx);
    avformat_close_input(&fmt_ctx);
    delete[] rgb;
    delete[] pcm;
    rgb = nullptr;
    pcm = nullptr;

    LOGD(TAG, "release end");


    env->ReleaseStringUTFChars(url, m_url);
    return JNI_OK;
    return JNI_OK;
}

void printStream(const AVFormatContext *fmt_ctx) {
    int audioStream;
    int videoStream;
    for (int i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVStream *stream = fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;

        LOGD(TAG, "Stream #%u:", i);
        LOGD(TAG, "  fps: %lf", r2d(stream->avg_frame_rate));
        LOGD(TAG, "  Timebase: {%d / %d}", stream->time_base.num, stream->time_base.den);
        LOGD(TAG, "  Codec type: %d", codecpar->codec_type);
        LOGD(TAG, "  Codec ID: %d", codecpar->codec_id);
        LOGD(TAG, "  Codec tag: 0x%x", codecpar->codec_tag);
        LOGD(TAG, "  Format: %d", codecpar->format);

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGD(TAG, "  Width: %d\n", codecpar->width);
            LOGD(TAG, "  Height: %d\n", codecpar->height);
            audioStream = i;
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGD(TAG, "  Channel layout: %" PRIu64 "\n", codecpar->channel_layout);
            LOGD(TAG, "  Channels: %d\n", codecpar->channels);
            LOGD(TAG, "  Sample rate: %d\n", codecpar->sample_rate);
            LOGD(TAG, "  Frame size: %d\n", codecpar->frame_size);
            videoStream = i;
        }
    }
}


// 注册 native 方法
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD(TAG, "%s start", __FUNCTION__);
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass(PACKAGE_NAME);
    if (clazz == nullptr) {
        return JNI_ERR;
    }

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != JNI_OK) {
        return JNI_ERR;
    }

    LOGD(TAG, "%s success", __FUNCTION__);
    av_jni_set_java_vm(vm, nullptr);
    return JNI_VERSION_1_6;
}