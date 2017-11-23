#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
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

int hook_entry(char *a) {
	LOGD("Hook success, pid=%d\n", getpid());
	LOGD("Hello %s\n", a);
	return 0;
}