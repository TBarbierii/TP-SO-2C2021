CC=gcc

compilarmatelib:
	cd src/ &&\
	$(CC) -c -Wall -I"src/" -fpic matelib.c && \
	$(CC) -shared matelib.o -o libmatelib.so && \
	rm matelib.o
