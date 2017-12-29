#ifndef _PTRACE_UTILS_
#define _PTRACE_UTILS_
#include <unistd.h>
#include <sys/ptrace.h>

#if defined(__aarch64__)
#define pt_regs user_pt_regs
#define uregs regs  
#define ARM_pc  pc  
#define ARM_sp  sp  
#define ARM_cpsr    pstate  
#define ARM_lr      regs[30]  
#define ARM_r0      regs[0]
#define PTRACE_GETREGS PTRACE_GETREGSET  
#define PTRACE_SETREGS PTRACE_SETREGSET 
#endif

int ptrace_readdata(pid_t pid, uint8_t *src, uint8_t *buf, size_t size);
int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t *data, size_t size);
int ptrace_writestring(pid_t pid, uint8_t *dest, const char *str);
int ptrace_getregs(pid_t pid, struct pt_regs *regs);
int ptrace_setregs(pid_t pid, struct pt_regs *regs);
int ptrace_continue(pid_t pid);
int ptrace_attach(pid_t pid);
int ptrace_detach(pid_t pid);
int ptrace_call(pid_t pid, uintptr_t addr, long *params, uint32_t params_num, struct pt_regs *regs);
int ptrace_call_wrapper(pid_t target_pid, const char *func_name, void *func_addr, long *params, int param_num, struct pt_regs *regs);
long ptrace_retval(struct pt_regs *regs);
long ptrace_ip(struct pt_regs *regs);


#endif //_PTRACE_UTILS_