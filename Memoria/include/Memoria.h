#ifndef PROCESO3_H
#define PROCESO3_H

#include <stdio.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <math.h>
#include "shared_utils.h"

/* Varibales obtenidas del config */

char* ip;
char* puerto;
uint32_t tamanio;
uint32_t tamanioPagina;
char* algoritmoReemplazoMMU;
char* tipoAsignacion;
uint32_t marcosMaximos;
uint32_t cantidadEntradasTLB;
char *algoritmoReemplazoTLB;
uint32_t retardoAciertoTLB;
uint32_t retardoFAlloTLB;
char* ipSWAmP;
char* puertoSWAmP;

uint32_t id_pag; //inicializar en algun lado 
uint32_t id_carpincho; //inicializar
uint32_t id_marco; //inicializar

/* Semaforos */

pthread_mutex_t * listaCarpinchos;

/* Estructuras Administrativas */


typedef struct {

    uint32_t pid;
    uint32_t tamanio;

}t_memalloc;

typedef struct {

    uint32_t pid;
    uint32_t DL;

}t_memfree;

typedef struct {

    uint32_t pid;
    uint32_t DL;
    void* destino;
    uint32_t size;

}t_memread;

typedef struct {

    uint32_t pid;
    uint32_t DL;
    void* origen;
    uint32_t size;

}t_memwrite;

typedef struct {

    int32_t prevAlloc;
    int32_t nextAlloc;
    uint8_t isFree;

} __attribute__((packed)) heapMetadata ;

/*typedef struct{
    uint32_t pagina;
    uint8_t estaPartido;
    heapMetadata heap;
}alloc;*/

typedef struct {

    uint32_t id_marco;
    uint32_t comienzo;
    int32_t proceso_asignado;//al empezar inicializar todo en -1
    bool estaLibre;

}t_marco;

typedef struct {

    uint32_t id_pagina;
    uint8_t esNueva;
    t_marco* marco;
    uint8_t presente;

}t_pagina;


typedef struct {

    uint32_t id_carpincho;
    t_list* tabla_de_paginas;//esto seria una lista de paginas
    t_list* allocs;
    uint32_t conexion;

}t_carpincho;


typedef struct {

    t_list* paginas;

}tablaDePagina;

/* Variable Global */

void* memoriaPrincipal;

/*Listas */

t_list* carpinchos;
t_list* marcos;

/* Semaforos */

/* Funciones */

t_config* inicializarConfig();
void inicializarListas();
void finalizarListas();
void obtenerValoresDelConfig(t_config* configActual);
void finalizarConfig(t_config* configUsado);
void inicializarMemoria();
void inicializarTodo();
void finalizarTodo(t_config* configActual);
void finalizarMemoria();

uint32_t administrar_allocs(t_memalloc*);
uint32_t buscar_o_agregar_espacio(t_carpincho* , uint32_t );
uint32_t administrar_paginas(t_carpincho* , uint32_t );
void* generar_stream_allocs(t_carpincho* );
void escribir_marcos(t_list* , t_carpincho* );
uint32_t asignarPaginas(t_carpincho* );



void crear_marcos();
void liberarMemoria();
void enviarInformacionAdministrativaDelProceso(t_carpincho* carpincho);
void inicializar_carpincho(int conexion ,t_log* logger);

void* leer_memoria(uint32_t DL, uint32_t carpincho, uint32_t tam);
uint32_t escribir_memoria(uint32_t carpincho ,uint32_t direccion_logica, void* contenido, uint32_t tam);

void enviar_pagina(uint32_t pid, uint32_t id_pagina, void* contenido);

void pedir_pagina(uint32_t id_pagina, uint32_t pid);

uint32_t asignarPaginas(t_carpincho*);

/* Auxiliares */
uint32_t generadorIdsPaginas();
uint32_t generadorIdsCarpinchos();
uint32_t generadorIdsMarcos();
uint32_t generarDireccionLogica(uint32_t , uint32_t);
uint32_t calcular_direccion_fisica(uint32_t carpincho, uint32_t direccionLogica);
uint32_t obtenerDesplazamiento(uint32_t);
uint32_t obtenerId(uint32_t);


#endif