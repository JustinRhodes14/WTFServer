#include <netdb.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#define SA struct sockaddr 
char* combineString(char*,char*);
int compareString(char*,char*);
char* copyString(char*,char*);
void create(char*);
int extractInfo(char*); 
void func(int connfd, int sockfd, struct sockaddr_in cli, int len); 
char* readSock(int); 
char* substring(char*,int,int);
void writeTo(int,char*);

// Driver function 
int main(int argc, char** argv) 
{ 
	if (argc != 2) {
		printf("Fatal Error: Only input one argument (one port number)\n");
		exit(0);
	}


	int port = atoi(argv[1]);
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(port); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening on port %d\n", port); 
	len = sizeof(cli); 
	//wrap accept in a while loop (wrap accept and func call in while loop
	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccept the client...\n"); 
	//also make a signal handler to stop the server
	// Function for chatting between client and server 
	func(sockfd,connfd,cli,len); 

	// After chatting close the socket 
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

	if (stat(projectName, &st) == -1) {
		mkdir(projectName,0700);
	}
}

int extractInfo(char* word) {
	int counter = 0;
	while (word[counter] != ' ') {
		counter++;
	}
	return counter;
}
// Function designed for chat between client and server. 
void func(int connfd, int sockfd, struct sockaddr_in cli, int len) 
{ 
	char buff[80];
	char* exitMessage = "";
	bzero(buff, 80); 

	read(sockfd, buff, sizeof(buff)); 

	printf("From client: %s\n", buff); 

	if(strncmp("create", buff, 6) == 0) {
		int split = extractInfo(buff);
		char* action = substring(buff, 0, split);
		char* project = substring(buff, split+1, -1);

		create(project);
		exitMessage = combineString(exitMessage, "Exit Message: hooligans..");
		write(sockfd, exitMessage, strlen(exitMessage)); 
			
		printf("Succesfully initialized project..\n");
		if (strncmp("Exit", exitMessage, 4) == 0) { 
			printf("Server Exit...\n"); 
		} 


		bzero(buff, 80);
		read(sockfd, buff, sizeof(buff));
	
		printf("new buff: %s\n", buff);
	}

	/*
	   char* clientInfo = readSock(sockfd);
	   printf("From client: %s\n",clientInfo);
	   int split = extractInfo(clientInfo);

	   char* action = substring(clientInfo,0,split);
	   char* project = substring(clientInfo,split+1,-1);

	   if (compareString("create\0",action) == 0) {
	   create(project);
	   connfd = accept(sockfd, (SA*)&cli, &len); 
	   char* message = "Successfully initialized project\n\0";
	   send(sockfd,message,sizeof(message),0);
	   }
	   */
}

char* readSock(int sockFD) {
	int status = 1;
	int bytesRead = 0;
	char* confInfo = "";
	while (status > 0) {
		char buffer[101];
		memset(buffer,'\0',101);
		int readIn = 0;
		do {
			status = read(sockFD,buffer,100 - readIn);
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
