#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <elf.h>
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

int (*old_fopen)(const char * l, const char * r, unsigned int size) = 0;

int new_fopen(const char * l, const char * r, unsigned int size)
{
	LOGD("Hook strncmp(%s, %s, %d)\n", l, r, size);

	if (old_fopen == 0) {
		LOGE("old fopen addr is null.\n");
		return -1;
	}

	return old_fopen(l, r, size);
}

/*

FILE * (*old_fopen)(const char * path, const char * mode) = 0;

FILE * new_fopen(const char * path, const char * mode)
{
	LOGD("Hook fopen(%s, %s)\n", path, mode);

	if (old_fopen == 0) {
		LOGE("old fopen addr is null.\n");
		return NULL;
	}

	return old_fopen(path, mode);
}*/

void* get_module_base(pid_t pid, const char* module_name)
{
	FILE *fp;
	long addr = 0;
	char *pch;
	char filename[32];
	char line[1024];

	if ( pid < 0 )
		snprintf(filename, sizeof(filename), "/proc/self/maps");
	else
		snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);


	fp = fopen(filename, "r");
	if ( fp != NULL )
	{
		//printf("open file %s \n", filename);

		while ( fgets(line, sizeof(line), fp) )
		{
			if (strstr(line, module_name))
			{
				pch = strtok(line, "-");
				addr = strtoul(pch, NULL, 16);

				if ( addr == 0x8000 )
					addr = 0;

				break;
			}
		}
	}
	else
	{
		printf("open file %s failed. \n", filename);
	}

	return (void *)addr;
}

int HookImportFunction(const char *lib_path, const char *func_name)
{
	void *base_addr;
	int fd;
	Elf32_Ehdr ehdr;
	Elf32_Shdr shdr;

	old_fopen = strncmp;
	LOGD("old fopen = %p\n", old_fopen);

	base_addr = get_module_base(getpid(), lib_path);
	LOGD("lib_base = %p\n", base_addr);

 	fd = open(lib_path, O_RDONLY);    
    if (-1 == fd) {    
        LOGE("open lib error\n");    
        return -1;    
    } 

    read(fd, &ehdr, sizeof(Elf32_Ehdr));

    unsigned long shdr_addr = ehdr.e_shoff;      
    int shnum = ehdr.e_shnum;      
    int shent_size = ehdr.e_shentsize;      
    unsigned long stridx = ehdr.e_shstrndx;

    lseek(fd, shdr_addr + stridx * shent_size, SEEK_SET);      
    read(fd, &shdr, shent_size);

    char * string_table = (char *)malloc(shdr.sh_size);      
    lseek(fd, shdr.sh_offset, SEEK_SET);      
    read(fd, string_table, shdr.sh_size);    
    lseek(fd, shdr_addr, SEEK_SET);

    int i;      
    uint32_t out_addr = 0;    
    uint32_t out_size = 0;    
    uint32_t got_item = 0;  
    int32_t got_found = 0;

	for (i = 0; i < shnum; i++) {      
        read(fd, &shdr, shent_size);      
        if (shdr.sh_type == SHT_PROGBITS) {    
            int name_idx = shdr.sh_name;      
            if (strcmp(&(string_table[name_idx]), ".got.plt") == 0   
                    || strcmp(&(string_table[name_idx]), ".got") == 0) {      
                out_addr = (uint32_t)base_addr + shdr.sh_addr;      
                out_size = shdr.sh_size;      
                LOGD("out_addr = %x, out_size = %x\n", out_addr, out_size);    
  
                for (i = 0; i < out_size; i += 4) {      
                    got_item = *(uint32_t *)(out_addr + i);
                    LOGD("index = %d, got_item = %x\n", i, got_item);
                    if (got_item  == (uint32_t)old_fopen) {      
                        LOGD("Found eglSwapBuffers in got\n");    
                        got_found = 1;  
  
                        uint32_t page_size = getpagesize();  
                        uint32_t entry_page_start = (out_addr + i) & (~(page_size - 1));  
                        mprotect((uint32_t *)entry_page_start, page_size, PROT_READ | PROT_WRITE);  
                        *(uint32_t *)(out_addr + i) = (uint32_t)new_fopen;    
  
                        break;      
                    } else if (got_item == (uint32_t)new_fopen) {      
                        LOGD("Already hooked\n");    
                        break;      
                    }      
                }     
                if (got_found)   
                    break;  
            }     
        }      
    }      
  
    free(string_table);      
    close(fd);    

    return 0;
}

int hook_entry(char *a) {
	LOGD("Hook success, pid=%d\n", getpid());
	LOGD("Hello %s\n", a);
	//HookImportFunction("/data/data/com.lzx.iteam/lib/libjpush213.so", "strncmp");
	return 0;
}