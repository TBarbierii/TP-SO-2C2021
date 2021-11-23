################################################################################
# Makefile para compilar las pruebas del tp CarpinchOS
################################################################################
LP = /home/utnso/tp-2021-2c-UCM-20-SO/MateLib
CONFIG = $(LP)/cfg/configProcesos.config

# Agrego las libs
LIBS := -lmate -lpthread -lcommons

# Agrego los archivos a compilar (todos los archivos *.c en esta carpeta)
CAPY_SRCS := $(wildcard *.c)

# Agrego los nombres de los carpinchos (que van a estar en una carpeta "build")
CARPINCHOS := $(CAPY_SRCS:%.c=build/%)

# Agrego los headers de matelib
HEADERS := -I. -I "../MateLib/src/"

# Agrego comandos extra
RM := rm -rf
CC := gcc

# compile --> Crea la carpeta "build" y compila todos los carpinchos
compile: build $(CARPINCHOS)

build:
	mkdir $@

build/%: %.c
	$(CC) -L"../MateLib/src/" $(HEADERS)  -o "$@" $^ $(LIBS)

run: compile

	LD_LIBRARY_PATH="../MateLib/src/" ./build/PruebaBase_Carpincho1 $(CONFIG)

# clean --> Limpia todos los carpinchos
clean:
	-$(RM) build

.PHONY:
	compile clean