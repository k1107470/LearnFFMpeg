package com.example.learnffmpeg;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import javax.microedition.khronos.opengles.GL;

public class FFPlay extends GLSurfaceView implements SurfaceHolder.Callback {

    private static final String TAG = "FF-PlayView";

    private MyRenderer renderer;

    public FFPlay(Context context) {
        super(context);
        Log.d(TAG, "construct 1");
        init();
    }

    public FFPlay(Context context, AttributeSet attrs) {
        super(context, attrs);
        Log.d(TAG, "construct 2");
        init();
    }

    private void init() {
        // 设置 OpenGL ES 版本
        setEGLContextClientVersion(2);

        // 创建并设置渲染器
        renderer = new MyRenderer();
        setRenderer(renderer);

        // 可选：设置渲染模式
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated");
        FFBase.initGLSurfaceView(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
//        super.surfaceDestroyed(holder);
        Log.d(TAG, "surfaceDestroyed");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
//        super.surfaceChanged(holder, format, w, h);
        Log.d(TAG, "surfaceChanged");
    }
}
