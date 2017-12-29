#include "inject_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/mman.h>

#include "ptrace_utils.h"

#if defined(__aarch64__)
const char *libc_path = "/system/lib64/libc.so";
const char *linker_path = "/system/bin/linker64";
#else
const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";
#endif

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
	if ( fp != NULL ) {
		while ( fgets(line, sizeof(line), fp) ) {
			if (strstr(line, module_name)) {
				pch = strtok(line, "-");
				addr = strtoul(pch, NULL, 16);

				if ( addr == 0x8000 )
					addr = 0;

				break;
			}
		}
		fclose(fp);
	} else {
		printf("open file %s failed. \n", filename);
	}

	return (void *)addr;
}

void* get_remote_addr(pid_t target_id, const char* module_name, void* local_addr)
{
	void *local_handle, *remote_handle, *ret_addr;

	local_handle = get_module_base(-1, module_name);
	remote_handle = get_module_base(target_id, module_name);

	//printf("[+] get_remote_addr: local[%x], remote[%x]\n", (uint32_t)local_handle, (uint32_t)remote_handle);

	ret_addr = (void *)((uintptr_t)local_addr + (uintptr_t)remote_handle - (uintptr_t)local_handle);

#if defined(__i386__)
	if (!strcmp(module_name, "/system/lib/libc.so")) {
		ret_addr += 2;
	}
#endif

	return ret_addr;
}

int find_pid_of(const char *process_name)
{
	int id;
	pid_t pid = -1;
	DIR *dir;
	FILE *fp;
	char filename[32];
	char cmdline[256];
	struct dirent *entry;

	if ( process_name == NULL)
		return -1;

	dir = opendir("/proc");
	if ( dir == NULL )
		return -1;

	while ( (entry = readdir(dir)) != NULL ) {
		id = atoi(entry->d_name);
		if ( id != 0 ) {
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if ( fp ) {
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);

				if ( strcmp(process_name, cmdline) == 0 ) {
					pid = id;
					break;
				}
			}
		}
	}

	closedir(dir);

	return pid;
}


void *ptrace_call_mmap(pid_t target_pid, long *params, struct pt_regs *regs)
{
	static void *mmap_addr = NULL;

	if (mmap_addr == NULL) {
		mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);
		if (mmap_addr == NULL)
			return NULL;
	}

	if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, params, 6, regs))
		return NULL;

	return (void *)ptrace_retval(regs);
}

void *ptrace_call_dlopen(pid_t target_pid, long *params, struct pt_regs *regs)
{
	static void *dlopen_addr = NULL;

	if (dlopen_addr == NULL) {
		dlopen_addr = get_remote_addr(target_pid, linker_path, (void *)dlopen);
		if (dlopen_addr == NULL)
			return NULL;
	}

	if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, params, 2, regs))
		return NULL;

	return (void *)ptrace_retval(regs);
}

void *ptrace_call_dlsym(pid_t target_pid, long *params, struct pt_regs *regs)
{
	static void *dlsym_addr = NULL;

	if (dlsym_addr == NULL) {
		dlsym_addr = get_remote_addr(target_pid, linker_path, (void *)dlsym);
		if (dlsym_addr == NULL)
			return NULL;
	}

	if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, params, 1, regs))
		return NULL;

	return (void *)ptrace_retval(regs);
}

void *ptrace_call_dlclose(pid_t target_pid, long *params, struct pt_regs *regs)
{
	static void *dlclose_addr = NULL;

	if (dlclose_addr == NULL) {
		dlclose_addr = get_remote_addr(target_pid, linker_path, (void *)dlclose);
		if (dlclose_addr == NULL)
			return NULL;
	}

	if (ptrace_call_wrapper(target_pid, "dlclose", dlclose_addr, params, 1, regs))
		return NULL;

	return (void *)ptrace_retval(regs);
}

/*
int ptrace_call_dlerror(pid_t target_pid, void *func_addr, struct pt_regs *regs) 
{
	void *error_base;
	char line[1024];

	if (ptrace_call_wrapper(target_pid, "dlerror", func_addr, NULL, 0, &regs))
		return -1;

	error_base = (void *)ptrace_retval(&regs);

	ptrace_readdata(target_pid, (uint8_t *)error_base, (uint8_t *)line, strlen(line));
	printf("[+]ptrace_call_dlerror. %s\n", line);
}
*/

int print_remote_string(pid_t pid, uint8_t *src)
{
	char line[1024];

	if (ptrace_readdata(pid, src, (uint8_t *)line, sizeof(line)))
		return -1;

	printf("ptrace_readdatastring: %s\n", line);

	return 0;
}

#if 1
int inject_remote_process(pid_t target_pid, const char *library_path, const char *func_name, void *param, size_t param_size)
{
#define FUNCTION_NAME_ADDR_OFFSET 0x100
#define FUNCTION_PARAM_ADDR_OFFSET 0x200
	void *mmap_base, *so_handle, *hook_entry_addr;
	struct pt_regs regs, original_regs;

	long params[10];

	printf("[+] Injecting process: %d\n", target_pid);

	if (ptrace_attach(target_pid) == -1) {
		printf("ptrace_attach faled.\n");
		return -1;
	}

	if (ptrace_getregs(target_pid, &regs) == -1) {
		printf("ptrace_getregs faled.\n");
		ptrace_detach(target_pid);
		return -1;
	}

	memcpy(&original_regs, &regs, sizeof(regs));

	params[0] = 0;	//addr
	params[1] = 0x4000; //size
	params[2] = PROT_READ | PROT_WRITE | PROT_EXEC;	//prot
	params[3] = MAP_ANONYMOUS | MAP_PRIVATE; //flags
	params[4] = 0; //fd
	params[5] = 0; //offset
	mmap_base = ptrace_call_mmap(target_pid, params, &regs);
	if (mmap_base == NULL) {
		printf("ptrace_call_mmap failed.\n");
		return -1;
	}

	ptrace_writestring(target_pid, mmap_base, library_path);

	params[0] = (long)mmap_base;
	params[1] = RTLD_NOW | RTLD_GLOBAL;
	so_handle = ptrace_call_dlopen(target_pid, params, &regs);
	if (so_handle == NULL) {
		printf("ptrace_call_dlopen failed.\n");
		return -1;
	}

	printf("[+]Write function name in target process. %s\n", func_name);
	ptrace_writestring(target_pid, mmap_base+FUNCTION_NAME_ADDR_OFFSET, func_name);

	params[0] = (long)so_handle;
	params[1] = (long)(mmap_base+FUNCTION_NAME_ADDR_OFFSET);
	hook_entry_addr = ptrace_call_dlsym(target_pid, params, &regs);
	if (hook_entry_addr == NULL) {
		printf("ptrace_call_dlsym failed.\n");
		return -1;
	}

	printf("[+]Write params in target process.\n");
	ptrace_writestring(target_pid, mmap_base+FUNCTION_PARAM_ADDR_OFFSET, (const char *)param);

	params[0] = (long)(mmap_base+FUNCTION_PARAM_ADDR_OFFSET);
	if (ptrace_call_wrapper(target_pid, "hook_entry", hook_entry_addr, params, 1, &regs) == -1) {
		printf("hook_entry failed.\n");
		return -1;
	}

#if 0
	//关闭后inject的so会被卸载 hook则失效
	printf("[+] Calling dlclose in target process.\n");
	params[0] = (long)so_handle;
	if (ptrace_call_dlclose(target_pid, "dlclose", dlclose_addr, params, 1, &regs) == -1)
		return -1;
#endif

fail:

	ptrace_setregs(target_pid, &original_regs);
	ptrace_detach(target_pid);

	return 0;
}
#else
int inject_remote_process(pid_t target_pid, const char *library_path, const char *func_name, void *param, size_t param_size)
{
#define FUNCTION_NAME_ADDR_OFFSET 0x100
#define FUNCTION_PARAM_ADDR_OFFSET 0x200

	void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;
	void *mmap_base, *so_handle, *hook_entry_addr;
	struct pt_regs regs, original_regs;

	long params[10];

	printf("[+] Injecting process: %d\n", target_pid);

	if (ptrace_attach(target_pid) == -1)
		return -1;

	if (ptrace_getregs(target_pid, &regs) == -1)
		return -1;

	memcpy(&original_regs, &regs, sizeof(regs));

	mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);
	
	printf( "[+] from library %s get func addr, mmap: %x\n", libc_path, (uint32_t)mmap_addr);

	//printf("[+] Calling mmap in target process.\n");
	params[0] = 0;	//addr
	params[1] = 0x4000; //size
	params[2] = PROT_READ | PROT_WRITE | PROT_EXEC;	//prot
	params[3] = MAP_ANONYMOUS | MAP_PRIVATE; //flags
	params[4] = 0; //fd
	params[5] = 0; //offset

	if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, params, 6, &regs) == -1)
		return -1;

	mmap_base = (void *)ptrace_retval(&regs);
	//printf("mmap_base=%x\n", (uint32_t)mmap_base);
	if (mmap_base == NULL)
		return -1;

	dlopen_addr = get_remote_addr(target_pid, linker_path, (void *)dlopen);
	dlsym_addr = get_remote_addr(target_pid, linker_path, (void *)dlsym);
	dlclose_addr = get_remote_addr(target_pid, linker_path, (void *)dlclose);
	dlerror_addr = get_remote_addr(target_pid, linker_path, (void *)dlerror);

	printf( "[+] from library %s get func addr, dlopen: %x, dlsym: %x, dlclose: %x,dlerror: %x\n", 
		linker_path, (uint32_t)dlopen_addr, (uint32_t)dlsym_addr, (uint32_t)dlclose_addr, (uint32_t)dlerror_addr);

	printf("[+]Write library path in target process. %s\n", library_path);
	ptrace_writestring(target_pid, mmap_base, library_path);

	print_remote_string(target_pid, (uint8_t *)(mmap_base));

	//printf("[+] Calling dlopen in target process.\n");
	params[0] = (long)mmap_base;
	params[1] = RTLD_NOW | RTLD_GLOBAL;
	if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, params, 2, &regs) == -1)
		return -1;

	so_handle = (void *)ptrace_retval(&regs);
	//printf("so_handle=%x\n", (uint32_t)so_handle);
	if (so_handle == NULL)
		return -1;

	printf("[+]Write function name in target process. %s\n", func_name);
	ptrace_writestring(target_pid, mmap_base+FUNCTION_NAME_ADDR_OFFSET, func_name);

	print_remote_string(target_pid, (uint8_t *)(mmap_base+FUNCTION_NAME_ADDR_OFFSET));

	//printf("[+] Calling dlsym in target process.\n");
	params[0] = (long)so_handle;
	params[1] = (long)(mmap_base+FUNCTION_NAME_ADDR_OFFSET);
	if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, params, 2, &regs) == -1)
		return -1;

	hook_entry_addr = (void *)ptrace_retval(&regs);
	//printf("hook_entry_addr=%x\n", (uint32_t)so_handle);
	if (hook_entry_addr == NULL)
		return -1;

	printf("[+]Write params in target process.\n");
	ptrace_writestring(target_pid, mmap_base+FUNCTION_PARAM_ADDR_OFFSET, (const char *)param);

	//printf("[+] Calling hook_entry in target process.\n");
	params[0] = (long)(mmap_base+FUNCTION_PARAM_ADDR_OFFSET);
	if (ptrace_call_wrapper(target_pid, "hook_entry", hook_entry_addr, params, 1, &regs) == -1)
		return -1;

	//Enter 后会detach测试程序才会运行起来，否则处于暂停状态
	printf("Press enter to dlclose and detach\n");
	getchar();

#if 0
	//关闭后inject的so会被卸载 hook则失效
	//printf("[+] Calling dlclose in target process.\n");
	params[0] = (long)so_handle;
	if (ptrace_call_wrapper(target_pid, "dlclose", dlclose_addr, params, 1, &regs) == -1)
		return -1;
#endif

	ptrace_setregs(target_pid, &original_regs);
	ptrace_detach(target_pid);

	return 0;
}

#endif