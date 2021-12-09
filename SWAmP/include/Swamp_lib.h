#ifndef SWAMP_LIB_H
#define SWAMP_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include "shared_utils.h"
#include "Conexiones.h"


/* ---------------- variables obtenidas del config ---------------- */

char* ip_swap;
char* puerto_swap;
int tamanio_swap;
int tamanio_pagina;
int marcos_maximos;
int retardo_swap;

char* ip_ram;
char* puerto_ram;
int tipo_asignacion;


/* ---------------- Variables globales ---------------- */

t_list* lista_swap_files;
t_log* logger_swamp;


/* ---------------- Estructuras ---------------- */

typedef struct {
    char* path;    
    int id; 
    t_list* particiones_swap;
}swap_files;

typedef struct {
    int num_particion; 
    int num_pagina;
    int pid;
    int inicio_particion;
    int esta_libre;
    int hay_contenido; 
}particion;


/* ---------------- Declaracion de funciones ---------------- */

/* ---------------- INICIALIZACION ---------------- */
void obtener_valores_config(t_config* config_actual, t_log* logger);
void crear_archivos_swap(t_list* archivos_swap, int cantidad_particiones, t_log* logger);
t_list* crear_lista_particiones(int cantidad_particiones);


/* ---------------- PARTICIONES ---------------- */
particion* buscar_particion_libre_asignacion_dinamica(char* path_swap);
particion* particion_nueva(int numero);
int cantidad_frames_disponibles(swap_files* path_swap);
swap_files* encontrar_swap_file(char* path_swap);
void vaciar_particion(particion* particion_a_vaciar, char* path_swap);
particion* buscar_particion_en_base_a_pagina(int numero_pagina, swap_files* archivo);
particion* primer_particion_libre(swap_files* archivo);
particion* primer_particion_disponible_para_escribir(swap_files* archivo, int PID);
particion* particion_disponible_para_sobreescribir(swap_files* archivo, int PID, int id_pagina);


/* ---------------- PAGINAS ---------------- */
int pagina_libre(particion* particion_nueva);
void leer_contenido(uint32_t PID, uint32_t id_pagina, int conexion, t_log* logger);


/* ---------------- ASIGNACIONES ---------------- */
void manejar_asignacion();
int asignacion_dinamica(int pid, swap_files* archivo);
int asignar_marcos_maximos(int pid, swap_files* archivo);
void asignar_marcos_proceso(int pid,swap_files* archivo);


/* ---------------- AUXILIARES ---------------- */
int verificar_pid_en_swap_file(uint32_t PID, char* path_swap);
int pid_se_encuentra_en_particion(swap_files* archivo_swap, uint32_t PID);
int cantidad_frames_disponibles_para_proceso(int PID, t_log* logger);
swap_files* encontrar_swap_file_en_base_a_pid(uint32_t PID);
swap_files* buscar_archivo_con_mayor_espacio();


/* ---------------- ESCRITURA DE ARCHIVOS ---------------- */
void escribirContenido(void* mensajeAEscribir, int id_pagina, int PID, t_log* logger);
void escribirContenidoSobreElArchivo(void* mensajeAEscribir, int pagina, int pid, char* nombreArchivo, t_log* logger);
swap_files* escritura_en_archivo_en_base_tipo_asignacion(int pid, t_log* logger);


/* ---------------- FINALIZACION ---------------- */
void eliminarParticiones(t_list* listaParticiones);
void destruirArchivosSwapFiles();
void limpiar_marcos_de_proceso(int PID);

#endif