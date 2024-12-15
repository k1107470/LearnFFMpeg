package com.example.learnffmpeg;

public class FFBase {
    private static FFBase m_base;
    // Used to load the 'learnffmpeg' library on application startup.
    static {
        System.loadLibrary("learn-ffmpeg");
        if (m_base == null) {
            m_base = new FFBase();
        }
    }

    public static String autoPlay() {
        return m_base.play();
    }

    public static void initGLSurfaceView(Object surface) {
        m_base.initView(surface);
    }

//    public static void openUrl(String url) {
//        m_base.open(url);
//    }
//
//    public static void openUrl(String url, Object surface) {
//        m_base.open2(url, surface);
//    }
//
//    public static void playAudioSource(String url) {
//        m_base.playAudio(url);
//    }
//
//    public static void playYuv(String url, Object surface) {
//        m_base.playYUV(url, surface);
//    }
    /**
     * A native method that is implemented by the 'learnffmpeg' native library,
     * which is packaged with this application.
     */
    public native String play();

    public native void initView(Object surface);

//    public native boolean open(String url);
//    public native boolean open2(String url, Object surface);
//    public native void playAudio(String url);
//    public native void playYUV(String url, Object surface);
}
