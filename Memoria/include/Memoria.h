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
    marco marco;
    uint8_t presente;

}pagina;

typedef struct {

    uint32_t id_marco;

}marco;

typedef struct {

    t_list* paginas;
    uint32_t id_carpincho;

}tablaDePagina;

/* Variable Global */

void* memoriaPrincipal;

/* Funciones */

t_config* inicializarConfig();
void finalizarConfig(t_config* configUsado);
void obtenerValoresDelConfig(t_config* configActual);
void inicializarMemoria();
void liberarMemoria();

#endif