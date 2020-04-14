all: WTFserver WTF

WTFserver: WTFserver.c
	gcc WTFserver.c -o WTFserver
WTF: WTF.c
	gcc WTF.c -o WTF
clean:
	rm ./WTF; rm ./WTFserver
