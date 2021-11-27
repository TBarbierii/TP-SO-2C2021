#ifndef PROCESO3_H
#define PROCESO3_H
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

t_log* logsObligatorios;
t_log* dumpTLB;
t_log* loggerServidor; 


/* Estructuras Administrativas */

typedef enum{
    PRIMERA_VEZ,
    AGREGAR_ALLOC
}stream_alloc;

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

} __attribute__((packed)) heapMetadata;

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
    clock_t ultimoUso;
    bool uso;
    bool modificado;
    uint32_t id_carpincho;

}t_pagina;


typedef struct {

    uint32_t id_carpincho;
    t_list* tabla_de_paginas;//esto seria una lista de paginas
    t_list* allocs;
    uint32_t conexion;
    uint32_t tlb_hit;
    uint32_t tlb_miss;
    uint32_t punteroClock;
    uint32_t contadorPag;

}t_carpincho;


typedef struct {

    t_list* paginas;

}tablaDePagina;

/* Variable Global */

void* memoriaPrincipal;

/*Listas */

t_list* carpinchos;
t_list* marcos;
t_list* TLB;

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
uint32_t administrar_paginas(t_carpincho* , uint32_t, t_list* );
void* generar_buffer_allocs(uint32_t, heapMetadata*,uint32_t, stream_alloc, int32_t);
uint32_t asignarPaginas(t_carpincho*, t_list* );
void escribirMemoria(void* buffer, t_list* paginas, t_list* marcos_a_asignar, t_carpincho* carpincho );
uint32_t suspender_proceso(uint32_t pid);

void crear_marcos();
void liberar_alloc(uint32_t, uint32_t);
void enviarInformacionAdministrativaDelProceso(t_carpincho* carpincho);
void inicializar_carpincho(int conexion ,t_log* logger);

void* leer_memoria(uint32_t DL, uint32_t carpincho, uint32_t tam);
uint32_t escribir_memoria(uint32_t carpincho ,uint32_t direccion_logica, void* contenido, uint32_t tam);



//uint32_t asignarPaginas(t_carpincho*);

/* Auxiliares */
uint32_t generadorIdsPaginas(t_carpincho*);
uint32_t aumentarIdCarpinchos();
uint32_t generarDireccionLogica(uint32_t , uint32_t);
uint32_t calcular_direccion_fisica(uint32_t carpincho, uint32_t direccionLogica);
uint32_t obtenerDesplazamiento(uint32_t);
uint32_t obtenerId(uint32_t);
t_list* reservarMarcos(uin32_t);
int32_t buscar_TLB(t_pagina*);
int buscarSiguienteHeapLibre(heapMetadata* , int32_t* , t_carpincho* , int32_t*, int32_t* );
t_list* buscarMarcosLibres(t_carpincho* carpincho);
uint32_t crearAllocNuevo(int* pagina, int tamanio, heapMetadata* heap, int posicionUltimoHeap, t_carpincho *carpincho, int32_t*);
t_marco* reemplazarPagina(t_carpincho* carpincho);
t_pagina* algoritmo_reemplazo_MMU(t_list* paginas_a_reemplazar, t_carpincho* carpincho);
uint32_t swapear(t_carpincho* carpincho, t_pagina* paginaPedida);
void manejador_de_seniales(int numeroSenial);
void algoritmo_reemplazo_TLB(t_pagina* pagina);
int32_t buscarEnTablaDePaginas(t_carpincho* carpincho, int32_t idPag);
void reemplazo(int32_t *DF, t_carpincho* carpincho, t_pagina* pagina);
void imprimir_dump(t_log* log_dump, char * time);


#endif