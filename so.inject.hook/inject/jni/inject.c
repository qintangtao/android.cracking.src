#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <android/log.h>


#define ENABLE_DEBUG		1

#define CPSR_T_MASK        ( 1u << 5 )

const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";

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

int ptrace_readdata(pid_t pid, uint8_t *src, uint8_t *buf, size_t size)
{
	uint32_t i, j, remain;
	uint8_t *laddr;

	union u {
		long val;
		char chars[sizeof(long)];
	} d;

	j = size / 4;
	remain = size % 4;

	laddr = buf;

	for ( i = 0; i < j; i++ )
	{
		d.val = (long)ptrace(PTRACE_PEEKTEXT, pid, src, 0);
		memcpy(laddr, d.chars, 4);
		src += 4;
		laddr += 4;
	}

	if ( remain > 0 )
	{
		d.val = (long)ptrace(PTRACE_PEEKTEXT, pid, src, 0);
		memcpy(laddr, d.chars, remain);
	}

	return 0;
}

int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t *data, size_t size)
{
	uint32_t i, j, remain;
	uint8_t *laddr;

	union u {
		long val;
		char chars[sizeof(long)];
	} d;

	j = size / 4;
	remain = size % 4;

	laddr = data;

	for ( i = 0; i < j; i++ )
	{
		memcpy(d.chars, laddr, 4);
		ptrace(PTRACE_POKETEXT, pid, dest, d.val);
		dest += 4;
		laddr += 4;
	}

	if ( remain > 0 )
	{
		d.val = (long)ptrace(PTRACE_PEEKTEXT, pid, dest, 0);
		for ( i = 0; i < remain; i++ )
		{
			d.chars[i] = *laddr++;
		}
		ptrace(PTRACE_PEEKTEXT, pid, dest, d.val);
	}

	return 0;
}

int ptrace_writestring(pid_t pid, uint8_t *dest, const char *str)
{
	return ptrace_writedata(pid, dest, (uint8_t *)str, strlen(str)+1);
}


int ptrace_getregs(pid_t pid, struct pt_regs *regs)
{
	if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0)
	{
		perror("ptrace_getregs: Can not get register values");
		return -1;
	}
	return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs *regs)
{
	if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0)
	{
		perror("ptrace_setregs: Can not set register values");
		return -1;
	}
	return 0;
}

int ptrace_continue(pid_t pid)
{
	if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0)
	{
		perror("ptrace_continue");
		return -1;
	}
	return 0;
}

int ptrace_attach(pid_t pid)
{
	if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0)
	{
		perror("ptrace_attach");
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);

	if (ptrace(PTRACE_SYSCALL, pid, NULL, 0) < 0)
	{
		perror("ptrace_syscall");
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);

	return 0;
}

int ptrace_detach(pid_t pid)
{
	if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0)
	{
		perror("ptrace_detach");
		return -1;
	}
	return 0;
}

#if defined(__arm__)

int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t params_num, struct pt_regs *regs)
{
	uint32_t i;

	for (i = 0; i < params_num && i < 4; i++)
	{
		regs->uregs[i] = params[i];
	}

	if ( i < params_num )
	{
		regs->ARM_sp -= (params_num - i) * sizeof(long);
		ptrace_writedata(pid, (uint8_t *)regs->ARM_sp, (uint8_t *)&params[i], (params_num - i) * sizeof(long));
	}

	regs->ARM_pc = addr;
	if ( regs->ARM_pc & 1 )
	{
		/* thumb
		判断最后一位:
			1是thumb指令集
			0是arm指令集
		*/
		regs->ARM_pc &= (~1u);
		regs->ARM_cpsr |= CPSR_T_MASK;
	}
	else
	{
		/* arm */	
		regs->ARM_cpsr &= ~CPSR_T_MASK;
	}

	regs->ARM_lr = 0;

	if (ptrace_setregs(pid, regs) == -1 ||
		ptrace_continue(pid) == -1)
	{
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);

	return 0;
}

#elif defined(__i386__)

int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t params_num, struct pt_regs *regs)
{
	int stat = 0;

	regs->esp -= (params_num) * sizeof(long);
	ptrace_writedata(pid, (uint8_t *)regs->esp, (uint8_t *)&params, (params_num ) * sizeof(long));

	regs->eip = addr;

	if (ptrace_setregs(pid, regs) == -1 ||
		ptrace_continue(pid) == -1)
	{
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);
	while ( stat != 0xb7f )
	{
		if (ptrace_continue(pid) == -1)
		{
			return -1;
		}
		waitpid(pid, NULL, WUNTRACED);
	}

	return 0;
}

#else
	
	#error "ptrace_call not supported"

#endif

long ptrace_retval(struct pt_regs *regs)
{
#if defined(__arm__)
	return regs->ARM_r0;
#elif defined(__i386__)
	return regs->eax;
#else
	#error "ptrace_retval not supported";
#endif
}

long ptrace_ip(struct pt_regs *regs)
{
#if defined(__arm__)
	return regs->ARM_pc;
#elif defined(__i386__)
	return regs->eip;
#else
	#error "ptrace_ip not supported";
#endif
}

int ptrace_call_wrapper(pid_t target_pid, const char *func_name, void *func_addr, long *params, int param_num, struct pt_regs *regs)
{
	LOGD("[+]Calling %s in target process.\n", func_name);

	if (ptrace_call(target_pid, (uint32_t)func_addr, params, param_num, regs) == -1)
		return -1;		
	if (ptrace_getregs(target_pid, regs) == -1)
		return -1;

	LOGD("[+]Target process returned from %s, return value=%x, pc=%x\n", func_name, (uint32_t)ptrace_retval(regs), (uint32_t)ptrace_ip(regs));
	return 0;
}

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
		LOGD("open file %s \n", filename);

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
		LOGE("open file %s failed. \n", filename);
	}

	return (void *)addr;
}

void* get_remote_addr(pid_t target_id, const char* module_name, void* local_addr)
{
	void *local_handle, *remote_handle, *ret_addr;

	local_handle = get_module_base(-1, module_name);
	remote_handle = get_module_base(target_id, module_name);

	LOGD("[+] get_remote_addr: local[%x], remote[%x]\n", (uint32_t)local_handle, (uint32_t)remote_handle);

	ret_addr = (void *)((uint32_t)local_addr + (uint32_t)remote_handle - (uint32_t)local_handle);

#if defined(__i386__)
	if (!strcmp(module_name, libc_path))
	{
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

	while ( (entry = readdir(dir)) != NULL )
	{
		id = atoi(entry->d_name);
		if ( id != 0 )
		{
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if ( fp )
			{
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);

				if ( strcmp(process_name, cmdline) == 0 )
				{
					pid = id;
					break;
				}
			}
		}
	}

	closedir(dir);

	return pid;
}

int inject_remove_process(pid_t target_pid, const char *library_path, const char *func_name, void *param, size_t param_size)
{
#define FUNCTION_NAME_ADDR_OFFSET 0x100
#define FUNCTION_PARAM_ADDR_OFFSET 0x200

	void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;
	void *mmap_base, *so_handle, *hook_entry_addr;
	struct pt_regs regs, original_regs;

	long params[10];

	LOGD("[+] Injecting process: %d\n", target_pid);

	if (ptrace_attach(target_pid) != -1)
		return EXIT_SUCCESS;

	if (ptrace_getregs(target_pid, &regs))
		return -1;

	memcpy(&original_regs, &regs, sizeof(regs));

	mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);
	
	LOGD( "[+] from library %s get func addr, mmap: %x\n", libc_path, (uint32_t)mmap_addr);

	dlopen_addr = get_remote_addr(target_pid, linker_path, (void *)dlopen);
	dlsym_addr = get_remote_addr(target_pid, linker_path, (void *)dlsym);
	dlclose_addr = get_remote_addr(target_pid, linker_path, (void *)dlclose);
	dlerror_addr = get_remote_addr(target_pid, linker_path, (void *)dlerror);

	LOGD( "[+] from library %s get func addr, dlopen: %x, dlsym: %x, dlclose: %x,dlerror: %x\n", 
		linker_path, (uint32_t)dlopen_addr, (uint32_t)dlsym_addr, (uint32_t)dlclose_addr, (uint32_t)dlerror_addr);


	LOGD("[+] Calling mmap in target process.\n");
	params[0] = 0;	//addr
	params[1] = 0x4000; //size
	params[2] = PROT_READ | PROT_WRITE | PROT_EXEC;	//prot
	params[3] = MAP_ANONYMOUS | MAP_PRIVATE; //flags
	params[4] = 0; //fd
	params[5] = 0; //offset

	if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, params, 6, &regs))
		return -1;

	mmap_base = (void *)ptrace_retval(&regs);
	LOGD("mmap_base=%x\n", (uint32_t)mmap_base);

	LOGD("[+]Write library path in target process.\n");
	ptrace_writestring(target_pid, mmap_base, library_path);

	LOGD("[+] Calling dlopen in target process.\n");
	params[0] = (long)mmap_base;
	params[1] = RTLD_NOW | RTLD_GLOBAL;
	if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, params, 2, &regs))
		return -1;

	so_handle = (void *)ptrace_retval(&regs);
	LOGD("so_handle=%x\n", (uint32_t)so_handle);

	LOGD("[+]Write function name in target process.\n");
	ptrace_writestring(target_pid, mmap_base+FUNCTION_NAME_ADDR_OFFSET, func_name);

	LOGD("[+] Calling dlsym in target process(get hook_entry function address).\n");
	params[0] = (long)so_handle;
	params[1] = (long)mmap_base+FUNCTION_NAME_ADDR_OFFSET;
	if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, params, 2, &regs))
		return -1;

	hook_entry_addr = (void *)ptrace_retval(&regs);
	LOGD("hook_entry_addr=%x\n", (uint32_t)so_handle);

	LOGD("[+]Write params in target process.\n");
	ptrace_writestring(target_pid, mmap_base+FUNCTION_PARAM_ADDR_OFFSET, (const char *)param);

	LOGD("[+] Calling hook_entry in target process.\n");
	params[0] = (long)(mmap_base+FUNCTION_PARAM_ADDR_OFFSET);
	if (ptrace_call_wrapper(target_pid, "hook_entry", hook_entry_addr, params, 1, &regs))
		return -1;

	printf("Press enter to dlclose and detach\n");
	getchar();

	LOGD("[+] Calling dlclose in target process.\n");
	params[0] = (long)so_handle;
	if (ptrace_call_wrapper(target_pid, "dlclose_addr", dlclose_addr, params, 1, &regs))
		return -1;

	ptrace_setregs(target_pid, &original_regs);
	ptrace_detach(target_pid);

	return 0;
}

int main(int argc, char** argv) 
{
	char *host_process, *inject_lib, *func_name, *func_params;
	pid_t target_pid;

	if (argc <= 3)
	{
		printf("inject host_process inject_lib func_name func_params\n");
		return -1;
	}

	if (argc > 1)
	{
		host_process = argv[1];
		printf("Host process is %s\n", host_process);
	}
	
	if (argc > 2)
	{
		inject_lib = argv[2];
		printf("Inject lib is %s\n", inject_lib);
	}
	
	if (argc > 3)
		func_name = argv[3];
	else
		func_name = "hook_entry";
	printf("Function name is %s\n", func_name);

	if (argc > 4)
		func_params = argv[4];
	else
		func_params = "hello inject.";
	printf("Function params is %s\n", func_params);


	target_pid = find_pid_of(host_process);
	if (target_pid == 0)
	{
		printf("not found process id.\n");
		return -1;
	}

	inject_remove_process(target_pid, inject_lib, func_name, func_params, strlen(func_params));

	return 0;
}