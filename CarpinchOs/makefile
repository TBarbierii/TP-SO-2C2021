LP = /home/utnso/tp-2021-2c-UCM-20-SO/MateLib
CC = gcc
CONFIG = $(LP)/cfg/configProcesos.config

matelib:
	cd $(LP)/src/ && \
	$(CC) -c -Wall -I"$(LP)/src/" -fpic matelib.c && \
	$(CC) -shared matelib.o -o libmate.so && \
	rm matelib.o

carpincho: matelib
	$(CC) -g3 -Wall -L"$(LP)/src/" -I"$(LP)/src/" carpincho.c -lmate -lpthread -lrt -lcommons -lreadline -lcunit -o carpincho

run: carpincho
	LD_LIBRARY_PATH="$(LP)/src/" ./carpincho $(CONFIG)
	
.PHONY: clean

clean:
	rm $(LP)/src/libmate.so ./carpincho