#include "ptrace_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <dirent.h>

#define CPSR_T_MASK        ( 1u << 5 )


int ptrace_readdata(pid_t pid, uint8_t *src, uint8_t *buf, size_t size)
{
	uint32_t i, j, remain;
	uint8_t *laddr;
	size_t align = sizeof(long);

	union u {
		long val;
		char chars[sizeof(long)];
	} d;

	j = size / align;
	remain = size % align;

	laddr = buf;

	for ( i = 0; i < j; i++ ) {
		d.val = (long)ptrace(PTRACE_PEEKTEXT, pid, src, 0);
		memcpy(laddr, d.chars, align);
		src += align;
		laddr += align;
	}

	if ( remain > 0 ) {
		d.val = (long)ptrace(PTRACE_PEEKTEXT, pid, src, 0);
		memcpy(laddr, d.chars, remain);
	}

	return 0;
}

int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t *data, size_t size)
{
	uint32_t i, j, remain;
	uint8_t *laddr;
	size_t align = sizeof(long);

	union u {
		long val;
		char chars[sizeof(long)];
	} d;

	j = size / align;
	remain = size % align;

	laddr = data;

	for ( i = 0; i < j; i++ ) {
		memcpy(d.chars, laddr, align);
		ptrace(PTRACE_POKETEXT, pid, dest, d.val);
		dest += align;
		laddr += align;
	}

	if ( remain > 0 ) {
		//d.val = 0;
		//memcpy(d.chars, laddr, remain);
		//采用下面这种方式不会修改后面几个字节的数据
		d.val = (long)ptrace(PTRACE_PEEKTEXT, pid, dest, 0);
		for ( i = 0; i < remain; i++ ) {
			d.chars[i] = *laddr++;
		}
		ptrace(PTRACE_POKETEXT, pid, dest, d.val);
	}

	return 0;
}

int ptrace_writestring(pid_t pid, uint8_t *dest, const char *str)
{
	return ptrace_writedata(pid, dest, (uint8_t *)str, strlen(str)+1);
}

int ptrace_getregs(pid_t pid, struct pt_regs *regs)
{
#if defined(__aarch64__)

	struct iovec ioVec;
	ioVec.iov_base = regs;
	ioVec.iov_len = sizeof(*regs);
	if (ptrace(PTRACE_GETREGS, pid, NT_PRSTATUS, &ioVec) < 0) {
		perror("ptrace_getregs: Can not get register values");
		return -1;
	}

#else

	if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
		perror("ptrace_getregs: Can not get register values");
		return -1;
	}

#endif

	return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs *regs)
{
#if defined(__aarch64__)
	
	struct iovec ioVec;
	ioVec.iov_base = regs;
	ioVec.iov_len = sizeof(*regs);
	if (ptrace(PTRACE_SETREGS, pid, NT_PRSTATUS, &ioVec) < 0) {
		perror("ptrace_setregs: Can not set register values");
		return -1;
	}

#else

	if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
		perror("ptrace_setregs: Can not set register values");
		return -1;
	}
	return 0;

#endif

	return 0;
}

int ptrace_continue(pid_t pid)
{
	if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {
		perror("ptrace_continue");
		return -1;
	}
	return 0;
}

int ptrace_attach(pid_t pid)
{
	int stat = 0;

	if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {
		perror("ptrace_attach");
		return -1;
	}

#if 1
	waitpid(pid, &stat, WUNTRACED);
#else
	if (ptrace(PTRACE_SYSCALL, pid, NULL, 0) < 0) {
		perror("ptrace_syscall");
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);
#endif

	return 0;
}

int ptrace_detach(pid_t pid)
{
	if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {
		perror("ptrace_detach");
		return -1;
	}
	return 0;
}


#if defined(__arm__) || defined(__aarch64__)

int ptrace_call(pid_t pid, uintptr_t addr, long *params, uint32_t params_num, struct pt_regs *regs)
{
	uint32_t i;
	int stat = 0;

#if defined(__arm__)
	int params_num_registers = 4;
#else //__aarch64__
	int params_num_registers = 8;
#endif

	for (i = 0; i < params_num && i < params_num_registers; i++) {
		regs->uregs[i] = params[i];
	}

	if ( i < params_num ) {
		regs->ARM_sp -= (params_num - i) * sizeof(long);
		ptrace_writedata(pid, (uint8_t *)regs->ARM_sp, (uint8_t *)&params[i], (params_num - i) * sizeof(long));
	}

	regs->ARM_pc = addr;
	if ( regs->ARM_pc & 1 ) {
		/* thumb
		判断最后一位:
			1是thumb指令集
			0是arm指令集
		*/
		regs->ARM_pc &= (~1u);
		regs->ARM_cpsr |= CPSR_T_MASK;
	} else {
		/* arm */	
		regs->ARM_cpsr &= ~CPSR_T_MASK;
	}

	regs->ARM_lr = 0;

	if (ptrace_setregs(pid, regs) == -1 ||
			ptrace_continue(pid) == -1) {
		return -1;
	}

	waitpid(pid, &stat, WUNTRACED);
	while ( stat != 0xb7f ) {
		if (ptrace_continue(pid) == -1) {
			return -1;
		}
		waitpid(pid, &stat, WUNTRACED);
	}

	return 0;
}

#elif defined(__i386__)

int ptrace_call(pid_t pid, uintptr_t addr, long *params, uint32_t params_num, struct pt_regs *regs)
{
	int stat = 0;

	regs->esp -= (params_num) * sizeof(long);
	ptrace_writedata(pid, (uint8_t *)regs->esp, (uint8_t *)&params, (params_num ) * sizeof(long));

	regs->eip = addr;

	if (ptrace_setregs(pid, regs) == -1 || 
			ptrace_continue(pid) == -1) {
		return -1;
	}

	waitpid(pid, &stat, WUNTRACED);
	while ( stat != 0xb7f ) {
		if (ptrace_continue(pid) == -1) {
			return -1;
		}
		waitpid(pid, &stat, WUNTRACED);
	}

	return 0;
}

#else
	
	#error "ptrace_call not supported"

#endif

int ptrace_call_wrapper(pid_t target_pid, const char *func_name, void *func_addr, long *params, int param_num, struct pt_regs *regs)
{
	//printf("[+]Calling %s in target process.\n", func_name);

	if (ptrace_call(target_pid, (uintptr_t)func_addr, params, param_num, regs) == -1)
		return -1;		
	if (ptrace_getregs(target_pid, regs) == -1)
		return -1;

#if defined(__aarch64__)
	//printf("[+]return value=%lx, pc=%lx\n", (uintptr_t)ptrace_retval(regs), (uintptr_t)ptrace_ip(regs));
#else
	//printf("[+]return value=%x, pc=%x\n", (uintptr_t)ptrace_retval(regs), (uintptr_t)ptrace_ip(regs));
#endif

	return 0;
}

long ptrace_retval(struct pt_regs *regs)
{
#if defined(__arm__) || defined(__aarch64__)
	return regs->ARM_r0;
#elif defined(__i386__)
	return regs->eax;
#else
	#error "ptrace_retval not supported";
#endif
}

long ptrace_ip(struct pt_regs *regs)
{
#if defined(__arm__) || defined(__aarch64__)
	return regs->ARM_pc;
#elif defined(__i386__)
	return regs->eip;
#else
	#error "ptrace_ip not supported";
#endif
}

