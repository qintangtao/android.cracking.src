#ifndef _LOG_UTILS_
#define _LOG_UTILS_
#include <android/log.h>

#define ENABLE_DEBUG		1

#if ENABLE_DEBUG
	#define TAG "INJECT"
	#define	LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
	#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
	#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#else
	#define LOGD(...)
	#define LOGI(...)
	#define LOGE(...)
#endif

#endif //_LOG_UTILS_