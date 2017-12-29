#ifndef _INJECT_UTILS_
#define _INJECT_UTILS_
#include <unistd.h>


void* get_module_base(pid_t pid, const char* module_name);
void* get_remote_addr(pid_t target_id, const char* module_name, void* local_addr);
int find_pid_of(const char *process_name);

void *ptrace_call_mmap(pid_t target_pid, long *params, struct pt_regs *regs);
void *ptrace_call_dlopen(pid_t target_pid, long *params, struct pt_regs *regs);
void *ptrace_call_dlsym(pid_t target_pid, long *params, struct pt_regs *regs);



int inject_remote_process(pid_t target_pid, const char *library_path, const char *func_name, void *param, size_t param_size);



#endif //_INJECT_UTILS_