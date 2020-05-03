all: ./client/WTF ./server/WTFserver WTFtest

./client/WTF: ./client/WTF.c
	gcc ./client/WTF.c -lssl -lcrypto -o ./client/WTF

./server/WTFserver: ./server/WTFserver.c
	gcc ./server/WTFserver.c -o ./server/WTFserver -lpthread

WTFtest:
	gcc WTFtest.c -o WTFtest

clean:
	rm ./client/WTF; rm ./server/WTFserver; rm WTFtest;
