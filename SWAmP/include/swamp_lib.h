#ifndef SWAMP_LIB_H
#define SWAMP_LIB_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <semaphore.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>


/* variables obtenidas del config */

char* ip_swap;
char* puerto_swap;
uint32_t tamanio_swap;
uint32_t tamanio_pagina;
int marcos_maximos;
uint32_t retardo_swap;

char* ip_ram;
char* puerto_ram;
char* tipo_asignacion;


/* Variables globales */

t_list* lista_particiones;
t_list* lista_swap_files;
t_log* logger_swamp;


/* Estructuras */

typedef struct {
    char* path;
    int fd_swap;       
    void* swap_file;   
    t_list* particiones_swap;
    t_list* procesos_swap;
}swap_files;

typedef struct {
    int num_particion; 
    int pid;
    int esta_libre;
    void* inicio_particion;
}particion;

typedef struct {
    uint32_t num_pagina;
}t_pagina;

typedef struct {
    swap_files* file_swap;
    particion* frame;
    t_pagina* pagina;
}pagina_y_particion_swap;


/* Declaracion de funciones */

/* Inicializacion */
void obtener_valores_config(t_config*);
void crear_archivos_swap(t_list*, int);
t_list* crear_lista_particiones(int);

/*  Particiones  */
particion* buscar_particion_libre(char*);
particion* particion_nueva(int);
int cantidad_frames_disponibles(char*);

/*    Paginas    */
t_pagina* buscar_pagina(uint32_t);
//void guardar_pagina(int);

/*  Asignaciones  */
void asignacion_fija();
void asignacion_dinamica();

/*   Auxiliares   */
int verificar_pid_en_swap_file(uint32_t, char*);

#endif