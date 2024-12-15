//
// Created by jinshan on 2024/10/14.
//

#include <GLES2/gl2.h>
#include "FFShader.h"
#include "LogUtil.h"

#define TAG "FF-Shader"
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

static const char* fragNV12 = GET_STR(
        precision mediump float; // 精度
        varying vec2 vTexCoord; // 顶点着色器传入的坐标
        uniform sampler2D yTexture; // 输入的材质
        uniform sampler2D uvTexture;
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uvTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(uvTexture, vTexCoord).a - 0.5;
            rgb = mat3(1.0,          1.0,     1.0,
                       0.0,     -0.39465, 2.03211,
                       1.13983, -0.58060,     0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);

static const char* fragNV21 = GET_STR(
        precision mediump float; // 精度
        varying vec2 vTexCoord; // 顶点着色器传入的坐标
        uniform sampler2D yTexture; // 输入的材质
        uniform sampler2D uvTexture;
        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uvTexture, vTexCoord).a - 0.5;
            yuv.b = texture2D(uvTexture, vTexCoord).r - 0.5;
            rgb = mat3(1.0,          1.0,     1.0,
                       0.0,     -0.39465, 2.03211,
                       1.13983, -0.58060,     0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);


static GLuint InitShader(const char* code, GLint type) {
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

bool FFShader::Init(FFSHADER_TYPE type) {
    // 顶点着色器、片段着色器初始化
    vsh = InitShader(vertextShader, GL_VERTEX_SHADER);
    if (vsh == 0) {
        LOGE(TAG, "vertex shader init failed");
        return false;
    }
    LOGI(TAG, "init vertex shader success");
    switch (type) {

        case FFSHADER_YUV420P:
            fsh = InitShader(fragYUV420P, GL_FRAGMENT_SHADER);
            break;
        case FFSHADER_NV12:
            fsh = InitShader(fragNV12, GL_FRAGMENT_SHADER);
            break;
        case FFSHADER_NV21:
            fsh = InitShader(fragNV21, GL_FRAGMENT_SHADER);
            break;
    }
    if (fsh == 0) {
        LOGE(TAG, "fragment shader init failed");
        return false;
    }
    LOGI(TAG, "init fragment shader success");

    // 创建渲染程序
    program = glCreateProgram();
    if (program == 0) {
        LOGE(TAG, "glCreateProgram failed");
        return false;
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
        return false;
    }
    // 激活程序
    glUseProgram(program);
    LOGI(TAG, "glLinkProgram success!");
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
    // 设置纹理层, 这里针对yuv420p
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0); // 对应纹理的第1层
    switch (type) {

        case FFSHADER_YUV420P:
            glUniform1i(glGetUniformLocation(program, "uTexture"), 1); // 对应纹理的第2层
            glUniform1i(glGetUniformLocation(program, "vTexture"), 2); // 对应纹理的第3层
            break;
        case FFSHADER_NV12:
        case FFSHADER_NV21:
            glUniform1i(glGetUniformLocation(program, "uvTexture"), 1); // 对应纹理的第2层
            break;
    }


    LOGI(TAG, "Init shader success");

    return true;
}

void FFShader::GetAndBindTextures(unsigned int index, int width, int height, unsigned char *buf, bool isAlpha) {
    int format = GL_LUMINANCE;
    if (isAlpha)
        format = GL_LUMINANCE_ALPHA;
    if (texts[index] == 0) {
        // 材质初始化
        // 创建opengl材质
        glGenTextures(1, &texts[index]);// 创建三个纹理

        //设置纹理0属性
        glBindTexture(GL_TEXTURE_2D, texts[index]);
        // 过滤器，缩小、放大使用线性插值
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 设置纹理的格式和大小 ps 普通的灰度图
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     format, // gpu内部格式，亮度、灰度图，这里表示使用Y
                     width, height,
                     0, // 边框
                     format, // 数据的像素格式，和上面一致
                     GL_UNSIGNED_BYTE, // 像素的数据类型
                     nullptr // 纹理的数据
        );
    }

    // 激活第一层纹理 ，对应yTexture, 绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texts[index]);
    // 替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, width, height, format, GL_UNSIGNED_BYTE, buf);

}

void FFShader::Draw() {
    if (!program) {
        LOGE(TAG, "program is null");
        return;
    }
//    LOGI(TAG, "Draw");
    // 四个顶点
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
