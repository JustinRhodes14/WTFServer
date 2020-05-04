#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

void writeTo(int,char*);
pid_t serverInit();
int forkExec(char*[]);
int clientInit();

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

	char* argList[] = {"WTF","create","TestProject",NULL};
	forkExec(argList);
	//free(argList);


	int fd = open("TestProject/test1.txt",O_WRONLY | O_CREAT | O_TRUNC,00600);
	writeTo(fd,"Hello there writing this stuff to a file to see what happens hopefully it don't break but lets see what it does\n");
	close(fd);
	char* argList2[] = {"WTF","add","TestProject","TestProject/test1.txt",NULL};
	forkExec(argList2);


	waitpid(serverPID,&stat,0);
	return 0;
}
	
pid_t serverInit() {

	char* argList[] = {"WTFserver","9029",NULL};
	pid_t pid = fork();
	if (pid == -1) {
		printf("Fork failed... try again");
		exit(0);
	} else if (pid == 0) {
		chdir("./server");
		execv(argList[0],argList);	
		exit(0);
	} else {
		return pid;
	}

}

int clientInit() {
	
	char* argList[] = {"WTF","configure","127.0.0.1","9029",NULL};
	return forkExec(argList);

}

int forkExec(char* argList[]) {
	pid_t pid;
	int stat;
	pid = fork();
	if (pid == -1) {
		printf("Fork failed ... try again\n");
		exit(0);
	} else if (pid == 0) {
		chdir("./client");
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

void writeTo(int fd, char* word) {

	int bytesWritten = 0;
	int bytestoWrite = strlen(word);
	while (bytesWritten < bytestoWrite) {
		bytesWritten = write(fd,word,bytestoWrite - bytesWritten);
	}
	close(fd);

}
















