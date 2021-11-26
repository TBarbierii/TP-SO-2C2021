CC=gcc

compilarmatelib:
	cd src/ &&\
	$(CC) -c -Wall -I"src/" -fpic matelib.c && \
	$(CC) -shared matelib.o -o libmate.so && \
	rm matelib.o
