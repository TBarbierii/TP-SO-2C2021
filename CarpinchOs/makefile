CC=gcc

matelib:
	cd ../MateLib/src && \
	$(CC) -c -Wall -I"." -fpic matelib.c && \
	$(CC) -shared matelib.o -o libmate.so && \
	rm matelib.o

carpincho: matelib
	$(CC) -g3 -Wall -L"../MateLib/src/" -I"../MateLib/src/" carpincho.c -lmate -lpthread -lrt -lcommons -lreadline -lcunit -o example

run: carpincho
	LD_LIBRARY_PATH="../MateLib/src/" ./example

clean:
	rm ../MateLib/src/libmate.so ./example