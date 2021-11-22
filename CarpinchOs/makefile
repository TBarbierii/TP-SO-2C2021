CC=gcc


matelib:
	cd ../MateLib/src && \
	$(CC) -c -Wall -I"../include/" -fpic matelib.c && \
	$(CC) -shared matelib.o -o libmate.so && \
	rm matelib.o

carpincho: matelib
	$(CC) -g3 -Wall -L"../MateLib/src/" -I"../MateLib/src/" carpincho.c -lmate -lpthread -lrt -lcommons -lreadline -lcunit -o carpincho

run: carpincho
	LD_LIBRARY_PATH="../MateLib/src/" ./carpincho

clean:
	rm ../MateLib/src/libmate.so ./carpincho