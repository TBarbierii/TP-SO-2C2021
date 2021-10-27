#ifndef SWAMP_LIB_H
#define SWAMP_LIB_H

#include <stdio.h>
#include <stdlib.h>
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
int tamanio_swap;
int tamanio_pagina;
int marcos_maximos;
int retardo_swap;

char* ip_ram;
char* puerto_ram;
int tipo_asignacion;


/* Variables globales */

t_list* lista_swap_files;
t_log* logger_swamp;


/* Estructuras */

typedef struct {
    char* path;     
    t_list* particiones_swap;
}swap_files;

typedef struct {
    int num_particion; 
    int pid;
    int esta_libre;
    int inicio_particion;
    int num_pagina;
    int hay_contenido; //esto es para que el proceso, cuando busque una pagina de las que le pertenecen, 
                      //pueda buscar las que no estan siendo utilizadas actualemnte, en caso de utilizarla se pone en 1, en caso de que se libere, la ponemos en 0.
}particion;


/* Declaracion de funciones */

/* Inicializacion */
void obtener_valores_config(t_config* config_actual, t_log* logger);
void crear_archivos_swap(t_list* archivos_swap, int cantidad_particiones, t_log* logger);
t_list* crear_lista_particiones(int);

/*  Particiones  */
particion* buscar_particion_libre_asignacion_dinamica(char*);
particion* particion_nueva(int);
int cantidad_frames_disponibles(char*);
swap_files* encontrar_swap_file(char* path_swap);


/*    Paginas    */
int pagina_libre(particion*);
void leer_contenido(uint32_t pid, uint32_t id_pagina, t_log* logger);

/*  Asignaciones  */
void manejar_asignacion();

/*   Auxiliares   */
int verificar_pid_en_swap_file(uint32_t, char*);
int pid_se_encuentra_en_particion(swap_files*, uint32_t);
void* encontrar_swap_file_en_base_a_pid(uint32_t);

/* Finalizacion */
void eliminarParticiones(t_list* listaParticiones);
void destruirArchivosSwapFiles();

void escribirContenido();
void escribirContenidoSobreElArchivo(void* mensajeAEscribir, int marco, int pagina, int pid, char* nombreArchivo,t_log* logger);



#endif