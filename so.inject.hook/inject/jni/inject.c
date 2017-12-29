#include <stdio.h>
#include <stdlib.h>
#include "inject_utils.h"
#include "log_utils.h"

void print_usage(char** argv) {
	fprintf(stderr, "error usage: %s -p PID [-P PROCNAME] -l LIBNAME [-f FUNCTION] [-s FUNCPARAM]\n", argv[0]);
}

void print_arg(const char *arg_name, const char *arg_val) {
#if 1
	printf("%s=%s\n", arg_name, arg_val);
#endif
}

int main(int argc, char** argv) 
{
	pid_t target_pid = -1;
	char *proc_name = NULL;
	char *lib_path = NULL;
	char *func_name = "hook_entry";
	char *func_params = "";

	int opt;

	while ((opt = getopt(argc, argv, "p:P:l:f:s:")) != -1) {
		switch ( opt ) {
			case 'p':
				target_pid = strtol(optarg, NULL, 0);
				break;
			case 'P':
				proc_name = strdup(optarg);
				print_arg("proc_name", proc_name);
				break;
			case 'l':
				lib_path = strdup(optarg);
				print_arg("lib_path", lib_path);
				break;
			case 'f':
				func_name = strdup(optarg);
				print_arg("func_name", func_name);
				break;
			case 's':
				func_params = strdup(optarg);
				print_arg("func_params", func_params);
				break;
			default:
				print_usage(argv);
				exit(0);
		}
	}

	if (proc_name != NULL && target_pid < 0)
		target_pid = find_pid_of(proc_name);

	if (target_pid <= 0 || lib_path == NULL)  {
		print_usage(argv);
		exit(0);
	}

	inject_remote_process(target_pid, lib_path, func_name, func_params, strlen(func_params));
	return 0;
}