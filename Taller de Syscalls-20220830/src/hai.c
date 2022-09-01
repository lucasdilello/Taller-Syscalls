#include <signal.h> /* constantes como SIGINT*/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int counter = 0;

void child(int sig) {
	counter++;
	printf("ya va\n");
}

void parent(int sig) {
	int status;
	if (wait(&status) < 0) { perror("waitpid"); exit(1); }
	exit(0);
}



int main(int argc, char* argv[]) {
  	pid_t child_id;

	if (argc <= 1) {
		//printf("Argumento invalido\n");
		printf("ERROR argumento invalido");
		exit(1);
	}

	child_id = fork();
	if (child_id == -1) { perror("ERROR"); return 1; }
	if (child_id == 0) {

		while(counter < 5){
			signal(SIGURG, child);
		}
		
		kill(getppid(), SIGCHLD);
		execvp(argv[1], argv+1);
		perror("ERROR"); exit(1);

	} else {
		signal(SIGCHLD, parent);

		while (1) {
			sleep(1);
			printf("sup!\n");
			kill(child_id, SIGURG);
		}
		
	}
	return 0;
}

