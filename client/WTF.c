#include <errno.h>
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
#include <openssl/sha.h>
#define SA struct sockaddr
int addFile(char*,char*); 
char* combineString(char*,char*);
int compareString(char*,char*);
char* compHash(char*);
void configure(char*,char*);
char* copyString(char*,char*);
int create(char*);
int extractInfo(char*);
void extractMan(char*);
void func(int,char*,char*,char*,int);
char* readConf(int);
char* readManifest(int);
int removeFile(char*,char*);
void stopSig(int);
char* substring(char*,int,int);
int tableComphash(char*);
void tableInit(int);
void tableInsert(char*,char*,char*,char*);
int tableSearch(char*);
void writeTo(int,char*);

typedef unsigned char byte;

int sockfd, connfd; 

typedef struct _hashNode {
	char* version;
	char* code;
	char* filepath;
	char* shacode;
	struct _hashNode* next;
}hashNode;

typedef struct _hashTable {
	int size;
	hashNode** table;
}hashTable;

hashTable* table;

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
	} else if (compareString("add\0",argv[1]) == 0) {
		tableInit(100);
		addFile(argv[2],argv[3]);
		return 0;
	} else if (compareString("remove\0",argv[1]) == 0) {
		tableInit(100);
		removeFile(argv[2],argv[3]);
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
//file name takes in the path rather than the blank file name
int addFile(char* projName, char* filename) {
	DIR *d;
	struct dirent *dir;
	if (!(d = opendir(projName))) {
		printf("%s is not a valid project name\n",projName);
		return -1;
	}


	int fd = open(filename,O_RDONLY);
	if (fd == -1) {
		printf("%s is not a valid file name\n",filename);
		return -1;
	}
	char* manFile = combineString(projName,"/.Manifest\0");
	int manFD = open(manFile,O_RDONLY);
	char* num = readManifest(manFD);
	close(manFD);
	
	if (tableSearch(filename) != -1) {
		printf("Warning: File already in .Manifest, commit before you add %s again.\n",filename);
		return 0; //warning
	}

	int manFD2 = open(manFile,O_RDWR);
	//memset(buffer,'\0',256);
	//read(manFD2,buffer,2);
	int t = lseek(manFD2,0,SEEK_END);
	
	char* hashedStuff = "";

	char* toHash = readConf(fd);
	hashedStuff = combineString(hashedStuff,compHash(toHash));
	
	char* letter = combineString(num," !AD \0");
	char* result = combineString(letter,filename);
	result = combineString(result, " \0");
	result = combineString(result,hashedStuff);
	
	writeTo(manFD2,result);
	write(manFD2,"\n",1);
	printf("Successfully added %s to .Manifest of %s.\n",filename,projName);
	close(fd);
	return 1; //success
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

char* compHash(char* fileContent) {
	int DataLen = strlen(fileContent);
	//SHA_CTX shactx;
	byte digest[SHA256_DIGEST_LENGTH];
	int i;

	//byte* testdata = (byte*)malloc(DataLen);
	//for (i=0; i<DataLen; i++) testdata[i] = 0;

	//SHA1_Init(&shactx);
	//SHA1_Update(&shactx, testdata, DataLen);
	//SHA1_Final(digest, &shactx);
	SHA256(fileContent,DataLen,digest);
	unsigned char* hash = malloc(SHA256_DIGEST_LENGTH * 2);
	for (i=0; i<SHA256_DIGEST_LENGTH; i++)
		//printf("%02x",digest[i]);
		sprintf((char*)(hash+(i*2)),"%02x", digest[i]);
	return hash;
}

void configure(char* ip, char* port) {
	int fd;
	fd = open ("./.configure", O_WRONLY | O_CREAT | O_TRUNC,00600);
	writeTo(fd,ip);
	writeTo(fd,"\n\0");
	writeTo(fd,port);
	close(fd);
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

int create(char* projectName) {
	struct stat st = {0};

	if (stat(projectName,&st) == -1) {
		mkdir(projectName,0700);
		char* histFile = combineString(projectName,"/.History\0");
		int histFD = open(histFile, O_WRONLY | O_CREAT | O_TRUNC, 00600);
		char* manFile = combineString(projectName,"/.Manifest\0");
		int manFD = open(manFile,O_WRONLY | O_CREAT | O_TRUNC, 00600);
		writeTo(manFD,"1\n\0");
		close(manFD);
		return 1;
	} else {
		printf("Project name already exists on client\n");
		return -1;
	}
}

int extractInfo(char* word) {
	int counter = 0;
	while (word[counter] != '\n') {
		counter++;
	}
	return counter;
}

void extractMan(char* manWord) {
	char** manInfo = (char**)malloc(4 * sizeof(char*));
	int i;
	int count = 0;
	int start = 0;
	for (i = 0; i < strlen(manWord); i++) {
		if (manWord[i] == ' ') {
			manInfo[count] = substring(manWord,start,i);
			start = i + 1;
			count++;		
		}
		if (count == 3) {
			manInfo[count] = substring(manWord,start,-1);
		}
	}
	tableInsert(manInfo[0],manInfo[1],manInfo[2],manInfo[3]);
	//printf("%s, %s, %s, %s\n",manInfo[0],manInfo[1],manInfo[2],manInfo[3]);
}

void func(int sockfd,char* action, char* projname,char* fname,int version) 
{ 
	//char buff[80]; 
	char buff[256];
	bzero(buff,sizeof(buff));
	if (compareString("create",action) == 0) {
		char* total = combineString(action," \0");
		total = combineString(total,projname); 
		write(sockfd,total,strlen(total));
		read(sockfd,buff,sizeof(buff));
		if (compareString(buff,"Project already exists on server\n\0") == 0) {
			printf("Project already exists on server, need to clone the project or pick a new name\n");
		} else {
			int cr = create(projname);
			printf("%s\n",buff);
		}
	} else if (compareString("destroy", action) == 0) {
		char* total = combineString(action," \0");
		total = combineString(total,projname); 
		write(sockfd,total,strlen(total));
		read(sockfd,buff,sizeof(buff));
		if (compareString(buff,"Destroy failed. Project does not exist on server\n\0") == 0) {
			printf("Destroy failed. Project does not exit on server\n");
		} else {
			printf("Successfully destroyed project on server...\n");
		}
	}	
	/*for (;;) { 
	  bzero(buff, sizeof(buff)); 
	  printf("Enter the string : "); 
	  n = 0; 
	  while ((buff[n++] = getchar()) != '\n') 
	  ; 
	  write(sockfd, buff, sizeof(buff)); 
	  bzero(buff, sizeof(buff)); 
	  read(sockfd, buff, sizeof(buff)); 
	  printf("From Server : %s", buff); 
	  if ((strncmp(buff, "exit", 4)) == 0) { 
	  printf("Client Exit...\n"); 
	  break; 
	  } 
	  } */
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

char* readManifest(int manFD) {
	int status = 1;
	int bytesRead = 0;
	char* holder;
	char* numRet;
	bool moreStuff = false;
	bool first = true;
	
	while (status > 0) {
		char buffer[101];
		memset(buffer,'\0',101);
		int readIn = 0;
		do {
			status = read(manFD,buffer,100 - readIn);
			if (status == 0) {
				break;
			}
			readIn+= status;
		}while(readIn < 100);
		int end = 0;
		int start = 0;
		while (end < 100) {
			char* temp;
			if (buffer[end] == '\n') {
				temp = substring(buffer,start,end);
				if (first == true) {
					numRet = copyString(numRet,temp);
					first = false;
				} else if (moreStuff == true) {
					holder = combineString(holder,temp);
					moreStuff = false;
					extractMan(holder);
				} else {
					extractMan(temp);
				}
				start = end + 1;
			}
			if (end == 99) {
				if (moreStuff == true) {
					holder = combineString(holder,buffer);
				} else {
					holder = substring(buffer,start, -1);	
				}
				moreStuff = true;
			}
			if (buffer[end] == '\0') {
				break;	
			}
			end++;
		}
	}
	return numRet;
}

int removeFile(char* projName, char* filename) {
	DIR *d;
	struct dirent *dir;
	if (!(d = opendir(projName))) {
		printf("%s is not a valid project name\n",projName);
		return -1;
	}


	int fd = open(filename,O_RDONLY);
	if (fd == -1) {
		printf("%s is not a valid file name\n",filename);
		return -1;
	}
	char* manFile = combineString(projName,"/.Manifest\0");
	int manFD = open(manFile,O_RDONLY);
	char* num = readManifest(manFD);
	close(manFD);
	
	int indexVal = tableSearch(filename);
	if ( indexVal == -1) {
		printf("Warning: File not present in .Manifest, add before you remove.\n",filename);
		return 0; //warning
	} else {
		remove(filename);
		int manFD2 = open(manFile,O_WRONLY | O_CREAT | O_TRUNC,00600);
		num = combineString(num,"\n\0");
		writeTo(manFD2,num);
		int i;
		for (i = 0; i < table->size; i++) {
			//printf("hi\n");
			hashNode* temp = table->table[i];
			while (temp) {
				if (compareString(temp->filepath,filename) == 0) {
					writeTo(manFD2,temp->version);
					writeTo(manFD2," ");
					writeTo(manFD2,"!RM\0");
					writeTo(manFD2," ");
					writeTo(manFD2,temp->filepath);
					writeTo(manFD2," ");
					writeTo(manFD2,temp->shacode);
					writeTo(manFD2,"\n");
					temp = temp->next;
				} else {
					writeTo(manFD2,temp->version);
					writeTo(manFD2," ");
					writeTo(manFD2,temp->code);
					writeTo(manFD2," ");
					writeTo(manFD2,temp->filepath);
					writeTo(manFD2," ");
					writeTo(manFD2,temp->shacode);
					writeTo(manFD2,"\n");
					temp = temp->next;
				}
			}
		}
		
	}
	close(fd);
	printf("Successfully removed %s from %s and updated .Manifest\n",filename,projName);
	return 1;
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

int tableComphash(char* filepath) {
	int len = strlen(filepath);
	int code = 0;
	int i;
	for (i = 0; i < len; i++) {
		code += (filepath[0] - 0);
	}
	return (code % table->size);
}

void tableInit(int size) {
	table = (hashTable*)malloc(sizeof(hashTable));
	table->size = size;
	table->table = (hashNode**)malloc(size * sizeof(hashNode*));
	int i;
	for (i = 0; i < size; i++) {
		table->table[i] = NULL;
	}
}

void tableInsert(char* version, char* code, char* filepath, char* shacode) {
	int index = -1;
	index = tableComphash(filepath);
	if (index == -1) {
		printf("Error in hashInsert\n");
		exit(0);
	}
	hashNode* temp = table->table[index];
	hashNode* toInsert = (hashNode*)malloc(sizeof(hashNode));
	hashNode* temp2 = temp;
	while (temp2) {
		temp2 = temp2->next;
	}
	toInsert->version = version;
	toInsert->code = code;
	toInsert->filepath = filepath;
	toInsert->shacode = shacode;
	toInsert->next = temp;
	table->table[index] = toInsert;
}

int tableSearch(char* filepath) {
	int index = -1;
	index = tableComphash(filepath);
	if (index == -1) {
		printf("Error in hashInsert\n");
		exit(0);
	}
	hashNode* temp = table->table[index];
	hashNode* temp2 = temp;
	while (temp2) {
		if (compareString(temp2->filepath,filepath) == 0) {
			return index;
		}
		temp2 = temp2->next;
	}
	return -1;	
}

void writeTo(int fd, char* word) {
	int bytesWritten = 0;
	int bytestoWrite = strlen(word);
	while (bytesWritten < bytestoWrite) {
		bytesWritten = write(fd,word,bytestoWrite - bytesWritten);
	}
}


