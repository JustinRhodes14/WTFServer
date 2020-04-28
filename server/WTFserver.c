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
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#define SA struct sockaddr 
char* combineString(char*,char*);
int compareString(char*,char*);
char* copyString(char*,char*);
int create(char*);
char* checkout();
char* currverr(int);
void destroy(char*);
void extractMan(char*);
int extractInfo(char*); 
void freeLL();
void func(int);
void listDirectories(char*);
void makeDirectories(char*);
char* readManifest(int);
char* readSock(int); 
void stopSig(int);
char* substring(char*,int,int);
int tableComphash(char*);
void tableFree(int);
void tableInit(int);
void tableInsert(char*,char*,char*,char*);
int tableSearch(char*);
int traverseCommits(char*,char*);
void writeTo(int,char*);

char* directories = "";

typedef struct _comNode {
	char* commit;
	char* projname;
	struct _comNode* next;
}comNode;

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
int hashSize = 0;

int sockfd, connfd, len; 


comNode* commits = NULL;

void stopSig(int signum) {
	printf("\nStopping connection to server and client\n");
	(void) signal(SIGINT,SIG_DFL);
	close(sockfd);
	exit(0);
}

// Driver function 
int main(int argc, char** argv) 
{ 
	(void) signal(SIGINT,stopSig);
	if (argc != 2) {
		printf("Fatal Error: Only input one argument (one port number)\n");
		exit(0);
	}


	int port = atoi(argv[1]);
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
	while (!(connfd < 0)) {		
		func(connfd); 
		connfd = accept(sockfd,(SA*)&cli,&len);
	}
	// After chatting close the socket 
	close(sockfd);
}

char* checkout() {
	
	char* toSend = "sendproject:\0";
	char num[256];
	memset(num,'\0',256);
	sprintf(num,"%d:",hashSize);
	toSend = combineString(toSend,num);
	int i;
	for (i = 0; i < table->size; i++) {
		hashNode* temp = table->table[i];
		while (temp) {
			int length = strlen(temp->filepath);
			int fd = open(temp->filepath,O_RDONLY);
			char* filecontent = readSock(fd);
			char num2[256];
			memset(num2,'\0',256);
			sprintf(num2,"%d:%d:",length,strlen(filecontent));
			toSend = combineString(toSend,num2);
			toSend = combineString(toSend,temp->filepath);
			toSend = combineString(toSend,filecontent);
			temp = temp->next;
			close(fd);
		}
	}
	//printf("Tosend: %s\n",toSend);
	return toSend;
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

int create(char* projectName) {

	struct stat st = {0};

	if (stat(projectName, &st) == -1) {
		mkdir(projectName,0700);
		char* histFile = combineString(projectName,"/.History\0");
		mkdir(histFile,0700);
		char* opsFile = combineString(projectName,"/.Operations\0");
		int opsFD = open(opsFile,O_WRONLY | O_CREAT | O_TRUNC,00600);
		close(opsFD);
		char* manFile = combineString(projectName,"/.Manifest\0");
		int manFD = open(manFile,O_WRONLY | O_CREAT | O_TRUNC,00600);
		writeTo(manFD,"1\n\0");
		close(manFD);
		free(histFile);
		free(manFile);
		printf("Successfully created %s project on server\n",projectName);
		return 1;
	} else {
		printf("Project name already exists on server\n");
		return -1;
	}
}

char* currver(int manFD) {
	char* result = "";
	int i;	
	for (i = 0; i < table->size; i++) {
		hashNode* temp = table->table[i];
		while (temp) {
			result = combineString(result,temp->version);
			result = combineString(result," \0");
			result = combineString(result,temp->filepath);
			result = combineString(result,"\n\0");
			temp = temp->next;
		}
	}
	//printf("Result: %s",result);
	return result;
}

void destroy(char* path) {
	DIR* d;
	struct dirent* dir;
	if(!(d = opendir(path))) {
		return;
	}

	while((dir = readdir(d)) != NULL) {
		if (dir->d_type == DT_DIR) {
			if (compareString(dir->d_name, ".") == 0 || compareString(dir->d_name, "..") == 0) {
				continue;
			}
			char* temp = combineString(path, "/");
			temp = combineString(temp, dir->d_name);
			destroy(temp);
			rmdir(temp);
		} else {
			char* temp = combineString(path, "/");
			temp = combineString(temp, dir->d_name);
			remove(temp);
		}
	}
	closedir(d);
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

int extractInfo(char* word) {
	int counter = 0;
	while (word[counter] != ' ') {
		counter++;
	}
	return counter;
}

void freeLL() {//need to do this for specific project commits
	while(commits != NULL) {
		comNode* temp = commits;
		commits = commits->next;
		free(temp);
	}
}

// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
	char buff[256];
	bzero(buff,256);

	read(sockfd,buff,sizeof(buff));
	if (compareString(buff,"Error\0") == 0) {
		printf("Error on client side, going back to accept...\n");
		return;	
	}
	int split = extractInfo(buff);

	char* resultMessage = "";
	char* action = substring(buff,0,split);
	char* project = substring(buff,split+1,-1);
	if (compareString("create\0",action) == 0) {
		int result = create(project);
		if (result == 1) {
			resultMessage = combineString(resultMessage, "Successfully initalized project on server and client\n");
			write(sockfd,resultMessage,strlen(resultMessage));
		} else {
			resultMessage = combineString(resultMessage,"Project already exists on server\n\0");
			write(sockfd,resultMessage,strlen(resultMessage));
		}	
	} else if (compareString("destroy", action) == 0) {
		project = combineString("./", project);
		DIR* d;
		struct dirent* dir;
		if(!(d = opendir(project))) {
			resultMessage = combineString(resultMessage, "Destroy failed. Project does not exist on server\n");
			write(sockfd,resultMessage,strlen(resultMessage));
			closedir(d);
		} else {
			closedir(d);
			destroy(project);	
			rmdir(project);
			resultMessage = combineString(resultMessage, "Successfully destroyed project.\n");
			write(sockfd,resultMessage,strlen(resultMessage));
			printf("Successfully destroyed %s\n", project);
		}
	} else if (compareString("checkout",action) == 0) {
		DIR* d;
		struct dirent* dir;
		if(!(d = opendir(project))) {
			resultMessage = combineString(resultMessage, "Project does not exist on server\n");
			printf("Project does not exist on server, sending error message...\n");
			write(sockfd,resultMessage,strlen(resultMessage));
			closedir(d);
		} else {
			listDirectories(project);
			bzero(buff,sizeof(buff));
			sprintf(buff,"%d",strlen(directories));
			write(sockfd,buff,sizeof(buff));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			write(sockfd,directories,strlen(directories));
			
			tableInit(100);
			char* manFile = combineString(project,"/.Manifest\0");
			int manFD = open(manFile,O_RDONLY);
			char* manText = readSock(manFD);
			bzero(buff,sizeof(buff));
			sprintf(buff,"%d",strlen(manText));
			write(sockfd,buff,sizeof(buff));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			write(sockfd,manText,strlen(manText));


			lseek(manFD,0,SEEK_SET);
			readManifest(manFD);
			close(manFD);
			char* message = checkout();
			bzero(buff,sizeof(buff));
			sprintf(buff,"%d",strlen(message));
			write(sockfd,buff,sizeof(buff));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			write(sockfd,message,strlen(message));
			tableFree(100);
			printf("Successfully cloned project into client.\n");
		}
	} else if (compareString("currentversion",action) == 0) {
		DIR* d;
		struct dirent* dir;
		if(!(d = opendir(project))) {
			resultMessage = combineString(resultMessage, "Project does not exist on server\n");
			printf("Project does not exist on server, sending error message...\n");
			write(sockfd,resultMessage,strlen(resultMessage));
			closedir(d);
		} else {
			tableInit(100);
			char* manFile = combineString(project,"/.Manifest\0");
			int manFD = open(manFile,O_RDONLY);
			char* num = readManifest(manFD);
			lseek(manFD,0,SEEK_SET);
			//close(manFD);
			char* toWrite = currver(manFD);
			close(manFD);
			char length[256];
			memset(length,'\0',256);
			sprintf(length,"%d",strlen(toWrite));
			//char* prepend = combineString(length,"\n\0");
			//prepend = combineString(prepend,toWrite);
			write(sockfd,length,strlen(length));
			char buffer2[256];
			bzero(buffer2,sizeof(buffer2));
			read(sockfd,buffer2,sizeof(buffer2));
			write(sockfd,toWrite,strlen(toWrite));
			printf("Successfully returned currentversion to client\n");
			tableFree(100);
		}
	} else if (compareString("commit",action) == 0) {
		DIR *d;
		struct dirent *dir;
		if (!(d = opendir(project))) {
			printf("%s does not exist on server\n",project);
			write(sockfd,"Error",5);
			return;
		}
		char* manFile = combineString(project,"/.Manifest\0");
		int manFD = open(manFile,O_RDONLY);
		char* manMessage = readSock(manFD);
		char len[256];
		sprintf(len,"%d",strlen(manMessage));
		write(sockfd,len,sizeof(len));
		bzero(buff,sizeof(buff));
		read(sockfd,buff,sizeof(buff));
		write(sockfd,manMessage,strlen(manMessage));
			
		bzero(buff,sizeof(buff));
		read(sockfd,buff,sizeof(buff));
		int length = atoi(buff);
		write(sockfd,"Success",7);
		char comBuf[length+1];
		bzero(comBuf,sizeof(comBuf));
		read(sockfd,comBuf,length);
		printf("Successfully recieved .Commit file\n");
		char* comFile = combineString(project,"/.Commit\0");
		//int mitFD = open(comFile,O_WRONLY | O_CREAT | O_TRUNC,00600);
		//writeTo(mitFD,comBuf);
		if (commits == NULL) {
			commits = (comNode*)malloc(sizeof(comNode));
			commits->commit = comBuf;
			commits->projname = project;
			commits->next = NULL;
			//printf("Commit: %s\n",commits->commit);
		} else {
			comNode* temp = (comNode*)malloc(sizeof(comNode));
			temp->commit = comBuf;
			temp ->next = commits;
			temp->projname = project;
			commits = temp;	
			//printf("Commit: %s\n",commits->commit);
		}
		close(manFD);
	} else if (compareString("push",action) == 0) {
		DIR *d;
		struct dirent* dir;
		if (!(d = opendir(project))) {
			resultMessage = combineString(resultMessage, "Project does not exist on server\n");
			printf("Project does not exist on server, sending error message...\n");
			write(sockfd,resultMessage,strlen(resultMessage));
			closedir(d);	
			return;
		}
		write(sockfd,"Success",7);
		bzero(buff,sizeof(buff));
		read(sockfd,buff,sizeof(buff));
		int len = atoi(buff);
		bzero(buff,sizeof(buff));
		write(sockfd,"Success",7);
		char comBuf[len+1];
		memset(comBuf,'\0',sizeof(comBuf));
		read(sockfd,comBuf,len);
		//printf("%s\n",comBuf); com file sent over, check linked list
		if (commits == NULL) {
			printf("No commits present, sending error to server\n");
			write(sockfd,"No commits present, commit before you push\n",43);
			return;
		} else {
			int success = traverseCommits(comBuf,project);
			if (success == 1) {
				char* sysMessage = combineString("rsync -Rr \0",project);
				char* dest = combineString(project,"/.History/\0");
				tableInit(100);
				int manFD = open(combineString(project,"/.Manifest\0"),O_RDONLY);
				char* num = readManifest(manFD);
				dest = combineString(dest,num);
				mkdir(dest,0700);//makes history directory
				sysMessage = combineString(sysMessage, " \0");
				sysMessage = combineString(sysMessage, dest);
				system(sysMessage);
				
				write(sockfd,"Success",7);
				bzero(buff,sizeof(buff));
				
				read(sockfd,buff,sizeof(buff));
				if (compareString(buff,"empty") == 0) {
					write(sockfd,"noted",5);
				} else {
					int lenDirs = atoi(buff);
					char dirs[lenDirs + 1];
					memset(dirs,'\0',lenDirs+1);
					write(sockfd,"Success",7);
					read(sockfd,dirs,lenDirs);
					makeDirectories(dirs);
				}
				bzero(buff,sizeof(buff));
				
				read(sockfd,buff,sizeof(buff));
				int lenFiles = atoi(buff);
				write(sockfd,"Success",7);
				char fileBuf[lenFiles+1];	
				memset(fileBuf,'\0',lenFiles+1);
				read(sockfd,fileBuf,lenFiles);
				printf("%d\n",push(fileBuf));
				tableFree(100);
				close(manFD);
				
				int newMan = open(combineString(project,"/.Manifest\0"),O_WRONLY | O_CREAT | O_TRUNC);
				bzero(buff,sizeof(buff));
				read(sockfd,buff,sizeof(buff));
				write(sockfd,"Success",7);
				int manLen = atoi(buff);
				char manText[manLen+1];
				memset(manText,'\0',manLen+1);
				read(sockfd,manText,manLen);
				writeTo(newMan,manText);
				

				char* ops = combineString(project,"/.Operations");
				int opsFD = open(ops,O_WRONLY);
				lseek(opsFD,0,SEEK_END);
				writeTo(opsFD,comBuf);
				printf("Successful push\n");
				//just gotta free LL for this now
			} else {
				write(sockfd,"No commits matched, commit before you push\n",43);
			}
		}
			
	} else if (compareString(action,"update") == 0) {
		DIR *d;
		struct dirent dir;
		if (!(d = opendir(project))) {
			printf("%s does not exist on server\n",project);
			write(sockfd,"Error",5);
			return;
		}
			
	} else if (compareString(action,"history") == 0) {
		DIR *d;
		struct dirent dir;
		if (!(d = opendir(project))) {
			printf("%s does not exist on server\n",project);
			write(sockfd,"Error",5);
			return;
		}
		char* ops = combineString(project,"/.Operations");
		int opsFD = open(ops,O_RDONLY);
		char* opsText = readSock(opsFD);
		char len[256];
		memset(len,'\0',256);
		if (strlen(opsText) <= 1) {
			write(sockfd,"Empty",5);
			return;
		}
		sprintf(len,"%d",strlen(opsText));
		write(sockfd,len,sizeof(len));
		bzero(buff,sizeof(buff));
		read(sockfd,buff,sizeof(buff));
		write(sockfd,opsText,strlen(opsText));
		printf("Successfully returned history to client\n");
	} else if (compareString(action,"rollback") == 0) {
		int split2 = extractInfo(project);
		char* version = substring(project,split2 + 1,-1);
		project = substring(project,0,split2);
		DIR *d;
		struct dirent dir;
		if (!(d = opendir(project))) {
			printf("%s does not exist on server\n",project);
			write(sockfd,"Error",5);
			return;
		}
		char* man = combineString(project,"/.Manifest");
		tableInit(100);
		int manFD = open(man,O_RDONLY);
		char* num = readManifest(manFD);
		tableFree(100);
		if (compareString(num,version) <= 0) {
			printf("Can't rollback, versions are the same or requested version is greater than current\n");
			write(sockfd,"Error",5);
			return;
		}
		
	}
}

void listDirectories(char* path) {
	DIR *d;
	struct dirent *dir;
	if (!(d = opendir(path))) {
		return;
	}
	while ((dir = readdir(d)) != NULL) {
		if (dir->d_type == DT_DIR) {
			if (compareString(dir->d_name,".") == 0 || compareString(dir->d_name,"..") == 0) {
				continue;	
			}
			char* temp = combineString(path,"/");
			temp = combineString(temp,dir->d_name);
			directories = combineString(directories,temp);
			directories = combineString(directories,"\n\0");
			listDirectories(temp);
		}	
	}
	closedir(d);
}

void makeDirectories(char* dirs) {
	int len = strlen(dirs);
	int i;
	int start = 0;
	for (i = 0; i < len; i++) {
		if (dirs[i] == '\n') {
			mkdir(substring(dirs,start,i),0700);
			start = i+1;	
		}	
	}
}

int push(char* message) {
	message = substring(message,10,-1);//extracts sendfile: portion
	int end = 0;
	while (message[end] != ':') {
		end++;	
	}
	int length = strlen(message);
	int i;
	int counter = 0;
	char* code;
	int fileSize = 0;
	int fileBytes = 0;
	int start = end+1;
	for (i = end + 1; i < length; i++) {
		if (message[i] == ':') {
			if (counter == 0) {
				code = substring(message,start,i);
			} else if (counter == 1) {
				fileSize = atoi(substring(message,start,i));
			} else if (counter == 2) {
				fileBytes = atoi(substring(message,start,i));
			}
			counter++;
			start = i+1;
		}
		if (counter == 3) {
			if (compareString(code, "!RM\0") == 0) {
				remove(substring(message,i+1,(i+1+fileSize)));
			} else {
				int fd = open(substring(message,i+1,(i+1+fileSize)),O_WRONLY | O_CREAT | O_TRUNC, 00600);
				writeTo(fd,substring(message,(i+1+fileSize),(i+1+fileSize+fileBytes)));
				close(fd);
			}
			counter = 0;
			start = i+1+fileSize+fileBytes;
			
		}
	}
	
	return 1;
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

int tableComphash(char* filepath) {
	int len = strlen(filepath);
	int code = 0;
	int i;
	for (i = 0; i < len; i++) {
		code += (filepath[0] - 0);
	}
	return (code % table->size);
}

void tableFree(int size) {
	int i;
	for (i = 0; i < size; i++) {
		hashNode* temp = table->table[i];
		while (temp != NULL) {
			hashNode* temp2 = temp;
			temp = temp2->next;
			free(temp2);
		}
	}
	free(table);
	hashSize = 0;	
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
	hashSize++;
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

int traverseCommits(char* commit, char* project) {
	comNode* temp = commits;
	while (temp != NULL) {
		if (compareString(temp->projname,project) == 0){ 
			if (compareString(temp->commit,commit) == 0) {
				//success
				return 1;
			}
		}
		temp = temp->next;
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
