#ifndef PROCESO3_H
#define PROCESO3_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include "shared_utils.h"

/* Varibales obtenidas del config */

char* ipSWAmP;
char* puertoSWAmP;
int tamanio;
char* algoritmoReemplazoMMU;
char* tipoAsignacion;
int marcosMaximos;
int cantidadEntradasTLB;
char *algoritmoReemplazoTLB;
int retardoAciertoTLB;
int retardoFAlloTLB;

/*Funciones */

t_config* inicializarConfig();
void finalizarConfig(t_config* configUsado);
void obtenerValoresDelConfig(t_config* configActual);

#endif