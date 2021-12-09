#ifndef MEMORIA_H
#define MEMORIA_H
#define TAMANIO_HEAP 9

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include "shared_utils.h"
#include "memalloc.h"
#include "Conexiones.h"

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
char* pathDump;

uint32_t id_pag; 
uint32_t id_carpincho;


uint32_t hits_totales;
uint32_t miss_totales;
int punteroClock;


/* Semaforos */

pthread_mutex_t * controladorIds;
pthread_mutex_t * controladorIdsPaginas;
pthread_mutex_t * listaCarpinchos;
pthread_mutex_t * TLB_mutex;
pthread_mutex_t * memoria;
pthread_mutex_t * swap;
pthread_mutex_t * marcos_sem;
pthread_mutex_t * solicitud_mutex;
pthread_mutex_t * tabla_paginas;
pthread_mutex_t * recorrer_marcos_mutex;
pthread_mutex_t * hits_sem;
pthread_mutex_t * miss_sem;

t_log* logsObligatorios;
t_log* dumpTLB;
t_log* loggerServidor; 
t_log* loggerMemalloc;



/* Variable Global */

void* memoriaPrincipal;

/*Listas */

t_list* carpinchos;
t_list* carpinchosMetricas;
t_list* marcos;
t_list* TLB;


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

void crear_marcos();

void enviar_tipo_asignacion(char* tipoAsignacion);
void atender_solicitudes_carpinchos();

void enviarInformacionAdministrativaDelProceso(t_carpincho* carpincho);
void inicializar_carpincho(int conexion ,t_log* logger);


uint32_t generadorIdsPaginas(t_carpincho*);
uint32_t aumentarIdCarpinchos();
uint32_t generarDireccionLogica(uint32_t , uint32_t);
uint32_t calcular_direccion_fisica(uint32_t carpincho, uint32_t direccionLogica);
uint32_t obtenerDesplazamiento(uint32_t);
uint32_t obtenerId(uint32_t);
void manejador_de_seniales(int numeroSenial);
void imprimir_dump(t_log* log_dump, char * time);
void dividirAllocs(t_carpincho* carpincho, int32_t posicionHeap, int32_t pagina, uint32_t tamanio, int32_t desplazamiento );
void consolidar_allocs(int desplazamientoHeapLiberado, t_pagina* pagina, int32_t prevAlloc, int32_t nextAlloc, t_carpincho* carpincho);


#endif