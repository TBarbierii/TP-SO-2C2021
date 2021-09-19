#ifndef PROCESO3_H
#define PROCESO3_H

#include <stdio.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdbool.h>
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

    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;

}heapMetadata;


typedef struct {

    uint32_t id_pagina;
    uint32_t tamanio;
    marco marco;
    uint8_t presente;

}pagina;

//funcion para asignar paginas a medida que se crean?

typedef struct {

    uint32_t id_carpincho;
    t_list* tabla_de_paginas;//esto seria una lista de paginas
    t_list* allocs;

}carpincho;

//buscar o crear alloc
//agregarlo a la lista de allocs
//verificar si te pasaste del tamaño de la pagina
//  si me pase-> crear otra pagina y seguir
//

typedef struct {

    uint32_t id_marco;
    uint32_t comienzo_en_memoria;

}marco;

typedef struct {

    t_list* paginas;

}tablaDePagina;

/* Variable Global */

void* memoriaPrincipal;

/* Funciones */

t_config* inicializarConfig();
void finalizarConfig(t_config* configUsado);
void obtenerValoresDelConfig(t_config* configActual);
void inicializarMemoria();
void liberarMemoria();

// Conexiones
void atender_solicitudes_multihilo();
void atender_solicitudes(uint32_t);
#endif