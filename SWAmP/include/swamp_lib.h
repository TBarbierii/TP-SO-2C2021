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
uint32_t tamanio_swap;
uint32_t tamanio_pagina;
int marcos_maximos;
uint32_t retardo_swap;

char* ip_ram;
char* puerto_ram;

/* Variables globales */

t_list* lista_particiones;
t_list* lista_swap_files;
t_log* logger_swamp;


/* Estructuras */

typedef struct {
    char* path;
    int num_swap;       // Es necesario?
    void* swap_file;   // Es necesario?
    t_list* particiones_swap;
}swap_files;

typedef struct {
    int num_particion; //quiza para el orden xq creo q importaba
    int pid;
    int esta_libre;
    void* inicio_particion;
}particion;

typedef struct {
    uint32_t tamanio;
    uint32_t num_pagina;
}pagina;

typedef struct {
    pagina* pagina;
    particion* frame;
}pagina_y_particion;

/* Declaracion de funciones */

void obtener_valores_config(t_config* configActual);
void crear_archivos_swap(t_list* archivos_swap, int cantidadParticiones);
t_list* crearListaDeParticiones(int cantidadParticiones);
particion* particionNueva(int numero);
particion* buscar_particion_libre(char*);
int cantidad_frames_disponibles(char*);

#endif