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
int tipo_asignacion;


/* Variables globales */

t_list* lista_swap_files;
t_log* logger_swamp;


/* Estructuras */

typedef struct {
    char* path;     
    void* swap_file;   // tendria que estar aca o creado en cada funcion
    t_list* particiones_swap;
    
}swap_files;

typedef struct {
    int num_particion; 
    int pid;
    int esta_libre;
    int inicio_particion;
    int num_pagina;
    int hay_contenido //esto es para que el proceso, cuando busque una pagina de las que le pertenecen, 
                      //pueda buscar las que no estan siendo utilizadas actualemnte, en caso de utilizarla se pone en 1, en caso de que se libere, la ponemos en 0.
}particion;


/* Declaracion de funciones */

/* Inicializacion */
void obtener_valores_config(t_config*);
void crear_archivos_swap(t_list*, int);
t_list* crear_lista_particiones(int);

/*  Particiones  */
particion* buscar_particion_libre_asignacion_dinamica(char*);
particion* particion_nueva(int);
int cantidad_frames_disponibles(char*);

/*    Paginas    */
int pagina_libre(particion*);
//void guardar_pagina(int);

/*  Asignaciones  */
void asignacion_fija();
void asignacion_dinamica();

/*   Auxiliares   */
int verificar_pid_en_swap_file(uint32_t, char*);

#endif