#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

pid_t serverPID;
pid_t clientPID;

void stopSig(int signum) {
	(void) signal(SIGINT,SIG_DFL);
	kill(serverPID,SIGINT);
	exit(0);
}

pid_t serverInit();

int main() {
	(void) signal(SIGINT,stopSig);
	int stat;
	serverPID = serverInit();
	clientPID = clientInit();
	if (clientPID != 0) {
		printf("Client could not be initialized... try again.\n");
		kill(serverPID,SIGINT);
		return -1;
	}
	waitpid(serverPID,&stat,0);
	return 0;
}
	
pid_t serverInit() {

	char* argList[] = {"server/WTFserver","9029",NULL};
	pid_t pid = fork();
	if (pid == -1) {
		printf("Fork failed... try again");
		exit(0);
	} else if (pid == 0) {
		execv(argList[0],argList);	
		exit(0);
	} else {
		return pid;
	}

}

int clientInit() {
	
	char* argList[] = {"client/WTF","configure","127.0.0.1","9029",NULL};
	return forkExec(argList,0);

}

int forkExec(char* argList[]) {
	pid_t pid;
	int stat;
	pid = fork();
	if (pid == -1) {
		printf("Fork failed ... try again\n");
		exit(0);
	} else if (pid == 0) {
		execv(argList[0],argList);
		exit(0);
	} else {
		printf("Parent PID: %u\n",getppid());
		if (waitpid(pid,&stat,0) > 0) {
			if (WIFEXITED(stat) && !WEXITSTATUS(stat)) {
				printf("Successfully executed command\n");
			} else if ( WIFEXITED(stat) && WEXITSTATUS(stat)) {
				if (WEXITSTATUS(stat) == 127) {
					printf("Exec failed, try again...\n");
					return 1;
				} else {
					printf("Program cleaned up correctly but returned a status that wasn't zero...\n");
					return 1;	
				}
			} else {
				printf("Program didn't terminate properly\n");
				return 1;
			
			}
		} else {
			printf("Wait failed... try again\n");
			return 1;
		}
	}
	return 0; // we made it!!
}
















