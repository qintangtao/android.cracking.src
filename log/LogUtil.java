package com.qin.log;

import android.text.TextUtils;
import android.util.Log;

public class LogUtil {

    private static int LOG_MAXLENGTH = 2000;

    private interface OnCallback {
        void onMessage(String TAG, String msg);
    }

    public static void d(String tag, String msg) {
        split(tag, msg, new OnCallback() {
            @Override
            public void onMessage(String tag, String msg) {
                Log.d(tag, msg);
            }
        });
    }

    public static void i(String tag, String msg) {
        split(tag, msg, new OnCallback() {
            @Override
            public void onMessage(String tag, String msg) {
                Log.i(tag, msg);
            }
        });
    }

    public static void e(String tag, String msg) {
        split(tag, msg, new OnCallback() {
            @Override
            public void onMessage(String tag, String msg) {
                Log.e(tag, msg);
            }
        });
    }

    static void split(String tag, String msg, OnCallback callback) {
        if (!TextUtils.isEmpty(msg) && callback != null) {
            String header = String.format("#%s#",
                    String.valueOf(System.currentTimeMillis()));
            if (msg.length() > LOG_MAXLENGTH) {
                for (int i = 0; i < msg.length(); i += LOG_MAXLENGTH) {
                    if (i + LOG_MAXLENGTH < msg.length()) {
                        callback.onMessage(tag,
                                header + msg.substring(i, i + LOG_MAXLENGTH));
                    } else {
                        callback.onMessage(tag,
                                header + msg.substring(i, msg.length()));
                    }
                }
            } else {
                callback.onMessage(tag, header + msg);
            }
        }
    }
}
