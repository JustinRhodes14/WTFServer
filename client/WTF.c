#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#define SA struct sockaddr 
char* combineString(char*,char*);
int compareString(char*,char*);
void configure(char*,char*);
char* copyString(char*,char*);
void create(char*);
int extractInfo(char*);
void func(int,char*,char*,char*,int);
char* readConf(int);
void stopSig(int);
char* substring(char*,int,int);
void writeTo(int,char*);


int sockfd, connfd; 

void stopSig(int signum) { 
	printf("\nStopping connection to server\n");
	(void) signal(SIGINT,SIG_DFL);
	close(sockfd);
	exit(0);
}

int main(int argc, char** argv) 
{ 	
	
	(void) signal(SIGINT,stopSig);
	if (compareString("configure\0",argv[1]) == 0) {
		configure(argv[2],argv[3]);
		printf("Successfully created .configure file\n");
		return 0;
	}
	int conf = open("./.configure",O_RDONLY);
	if (conf == -1) {
		printf("Fatal Error:No configure file present\n");
		exit(0);
	}
	
	char* confInfo = readConf(conf);
	int split = extractInfo(confInfo);
	char* ip = substring(confInfo,0,split);
	int port = atoi(substring(confInfo,split+1,-1));
	printf("IP: %s, PORT: %d\n",ip,port);
	struct sockaddr_in servaddr, cli; 
	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(ip); 
	servaddr.sin_port = htons(port); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

	// function for chat 
	if (argc == 3) {
		func(sockfd,argv[1],argv[2],NULL,-1); 
	}
	// close the socket
	close(sockfd); 
}

char* combineString(char* str1, char* str2) {
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	char* result = (char*)malloc((len1+len2)*sizeof(char) + 1);
	memset(result,'\0',(len1+len2+1));
	int i;
	int j = 0;
	for ( i = 0; i < len1; i++) {
		result[i] = str1[i];
		j++;
	}
	for ( i = 0; i < len2; i++) {
		result[j] = str2[i];
		j++;
	}
	return result;
}

int compareString(char* str1, char* str2) {
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	int shorter = 0;
	int len = len1;
	if (len1 > len2) {
		shorter = 1;//first one is shorter
		len = len1;
	} else if (len2 < len1) {
		shorter = 2;//second one is shorter
		len = len2;
	} else {
		shorter = 0;//equal length
		len = len1;
	}

	int i;
	for ( i = 0; i < len; i++) {
		if (str1[i] != str2[i]) {
			return ((int)str1[i] - (int)str2[i]);//negative if str1 is lesser, positive if str1 is greater
		}	
	}
	if (len1 < len2) {
		return -1;
	} else if (len2 < len1) {
		return 1;
	}
	return 0;//equal strings
}

void configure(char* ip, char* port) {
	int fd;
	fd = open ("./.configure", O_WRONLY | O_CREAT | O_TRUNC,00600);
	writeTo(fd,ip);
	writeTo(fd,"\n\0");
	writeTo(fd,port);
}

char* copyString(char* to, char* from) {
	int length = strlen(from);
	to = (char*)malloc(length * sizeof(char) + 1);
	memset(to,'\0',(length+1));
	int i;
	for ( i = 0; i < length; i++) {
		to[i] = from[i];
	}
	return to;
}

void create(char* projectName) {
	struct stat st = {0};
	
	if (stat(projectName,&st) == -1) {
		mkdir(projectName,0700);
	}
}

int extractInfo(char* word) {
	int counter = 0;
	while (word[counter] != '\n') {
		counter++;
	}
	return counter;
}

void func(int sockfd,char* action, char* projname,char* fname,int version) 
{ 
	char buff[80]; 
	bzero(buff, sizeof(buff)); 

	char* total = combineString(action, " ");
	total = combineString(total, projname);

	if(compareString("create", action) == 0) { 
		write(sockfd, total, strlen(total)); 
		create(projname);

		read(sockfd, buff, sizeof(buff)); 
		
		printf("%s\n", buff);
		if ((strncmp(buff, "Exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
		} 

		char* newBuff = "";
		newBuff = combineString(newBuff, "test message");

		write(sockfd, newBuff, strlen(newBuff));
	}
	/*
	char buffer[256];
	if (compareString("create",action) == 0) {
		char* total = combineString(action,"\n\0");
		total = combineString(total,projname); 
		writeTo(sockfd,total);
		create(projname);
		recv(sockfd,&buffer,sizeof(buffer),0);
		printf("%s\n",buffer);
		//char* message = readConf(sockfd);
		//printf("Message: %s\n",message);
	}
	*/
} 

char* readConf(int conFD) {
	int status = 1;
	int bytesRead = 0;
	char* confInfo = "";
	while (status > 0) {
		char buffer[101];
		memset(buffer,'\0',101);
		int readIn = 0;
		do {
			status = read(conFD,buffer,100 - readIn);
			if (status == 0) {
				break;
			}
			readIn += status;
		}while (readIn < 100);
		confInfo = combineString(confInfo,buffer);
	}
	return confInfo;
}

char* substring(char* str, int start, int end) {
	char* result;
	if (end == -1) {
		int length = strlen(str);
		result = (char*)malloc((length-start)*sizeof(char) + 1);
		memset(result,'\0',(length-start + 1));
		int i;
		int j = 0;
		for ( i = start; i < length; i++) {
			result[j] = str[i];
			j++;
		}
	} else {
		result = (char*)malloc((end-start)*sizeof(char) + 1);
		memset(result,'\0',(end-start + 1));
		int i;
		int j = 0;
		for ( i = start; i < end; i++) {
			result[j] = str[i];
			j++;
		}	
	}
	return result;
}

void writeTo(int fd, char* word) {
	int bytesWritten = 0;
	int bytestoWrite = strlen(word);
	while (bytesWritten < bytestoWrite) {
		bytesWritten = write(fd,word,bytestoWrite - bytesWritten);
	}
}


