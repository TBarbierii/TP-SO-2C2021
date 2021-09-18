#ifndef SWAMP_LIB_H
#define SWAMP_LIB_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <semaphore.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>


/* variables obtenidas del config */

char* ip_swap;
int puerto_swap;
int tamanio_swap;
int tamanio_pagina;
int marcos_maximos;
int retardo_swap;


/* Variables globales */

t_list* lista_swap_files;
t_log* logger_swamp;


/* Estructuras */

typedef struct {
    char* path;
    int fd_swap;
    void* swap_file;
    t_list* particiones_swap;
}swap_files;

typedef struct {
    int num_particion; //quiza para el orden xq creo q importaba
    int pid;
    int numero_marco;
    int esta_libre;
}particion;


/* Declaracion de funciones */

void obtenerValoresDelConfig(t_config* configActual);
void crear_archivos_swap(t_list* archivos_swap, int cantidadParticiones);
t_list* crearListaDeParticiones(int cantidadParticiones);
particion* particionNueva(int numero);

#endif