#ifndef PROCESO3_H
#define PROCESO3_H

#include <stdio.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <math.h>
#include "shared_utils.h"

/* Varibales obtenidas del config */

char* ipSWAmP;
char* puertoSWAmP;
uint32_t tamanio;
uint32_t tamanioPagina;
char* algoritmoReemplazoMMU;
char* tipoAsignacion;
uint32_t marcosMaximos;
uint32_t cantidadEntradasTLB;
char *algoritmoReemplazoTLB;
uint32_t retardoAciertoTLB;
uint32_t retardoFAlloTLB;

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

    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;

}heapMetadata;

typedef struct{
    uint32_t pagina;
    uint8_t estaPartido;
    heapMetadata heap;
}alloc;

typedef struct {

    uint32_t id_marco;
    uint32_t comienzo_en_memoria;

}t_marco;

typedef struct {

    uint32_t id_pagina;
    uint8_t esNueva;
    uint32_t tamanio;
    t_marco marco;
    uint8_t presente;

}t_pagina;


typedef struct {

    uint32_t id_carpincho;
    t_list* tabla_de_paginas;//esto seria una lista de paginas
    t_list* allocs;
    //una tlb por cada carpincho?

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
void administrar_allocs(t_memalloc);
void finalizarConfig(t_config* configUsado);
void obtenerValoresDelConfig(t_config* configActual);
void inicializarMemoria();
void liberarMemoria();

uint32_t escribir_en_memoria(t_carpincho*);


#endif