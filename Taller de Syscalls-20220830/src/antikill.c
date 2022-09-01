#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
	int status, sysno, rax, signal;

	if (argc <= 1) {
		fprintf(stderr, "Uso: %s commando [argumentos ...]\n", argv[0]);
		exit(1);
	}

	/* Fork en dos procesos */
	pid_t child = fork();
	if (child == 0) {
		/* Solo se ejecuta en el hijo */
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL)) {
			perror("ERROR child ptrace(PTRACE_TRACEME, ...)");
			exit(1);
		}
		execvp(argv[1], argv+1);
		/* Si vuelve de exec() hubo un error */
		perror("ERROR child exec(...)");
		exit(1);
	} else {
		/* Solo se ejecuta en el padre */
		while(1) {
			if (wait(&status) < 0) { perror("wait"); break; }
			if (WIFEXITED(status)) break; /* Proceso terminado */

			sysno = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);//nro de syscall en 8 * ORIG_RAX
			rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL); 
			signal = ptrace(PTRACE_PEEKUSER, child, 8 * RSI, NULL);
			if (rax == -ENOSYS) { 
				if (sysno == SYS_kill ) { //chequeo si la syscall a ejecutar es kill
					fprintf(stderr, "se envío la señal %i\n", signal);
					if(signal == SIGKILL){
						
						ptrace(PTRACE_POKEUSER, child, 8*ORIG_RAX, SYS_exit);
						ptrace(PTRACE_POKEUSER, child, 8*RDI, 1);
						
					break;
					}
				if(sysno == SYS_exit){
					break;
				}
			
					}
			}

			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
		ptrace(PTRACE_DETACH, child, NULL, NULL);
	}
	return 0;
}
