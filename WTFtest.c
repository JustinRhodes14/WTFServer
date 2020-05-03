#include <stdio.h>
#include <stdlib.h>

int main() {

	printf("Hello and welcome to wtf test\n");
	system("echo \"hello\"");
	system("cd client");
	system("./server/WTFserver 9029");
	system("./client/WTF configure 127.0.0.1 9029");
	system("./client/WTF create Swag");
	return 0;
}
