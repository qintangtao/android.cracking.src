package com.qin.log;

import android.text.TextUtils;
import android.util.Log;

public class LogUtil {

    private static int LOG_MAXLENGTH = 4000;

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
        if (!TextUtils.isEmpty(msg)) {
            if (msg.length() > LOG_MAXLENGTH) {
                if (callback != null) {
                    callback.onMessage(tag, msg);
                }
            } else {
                for (int i = 0; i < msg.length(); i += LOG_MAXLENGTH) {
                    if (i + LOG_MAXLENGTH < msg.length()) {
                        if (callback != null) {
                            callback.onMessage(tag,
                                    msg.substring(i, i + LOG_MAXLENGTH));
                        }
                    } else {
                        if (callback != null) {
                            callback.onMessage(tag,
                                    msg.substring(i, msg.length()));
                        }
                    }
                }
            }
        }
    }
}
