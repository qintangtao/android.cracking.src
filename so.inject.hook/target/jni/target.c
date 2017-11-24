#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <elf.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <android/log.h>


#define ENABLE_DEBUG		1



#if defined(__aarch64__)
const char *libc_path = "/system/lib64/libc.so";
const char *linker_path = "/system/bin/linker64";
#else
const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";
#endif

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

int main(int argc, char** argv) 
{
	FILE* fp;
	int maxRetry = 100;

	printf("main called.\n");
	
	while ( maxRetry-- > 0 ) {
		printf("fopen(%s), %d\n", libc_path, maxRetry);
		fp = fopen(libc_path, "r");
		if ( fp ) {
			sleep(3);
			printf("fclose(%s)\n", libc_path);
			fclose(fp);
		} else {
			sleep(3);
			printf("fopen failed.\n");
		}		
	}

	printf("main done.\n");
	
	return 0;
}