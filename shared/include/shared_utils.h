#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/collections/list.h>

/* KERNEL */
typedef enum tipo_bloqueo{
	NO_BLOQUEADO = -1,
	BLOCK_SEM,
	BLOCK_IO
}tipo_bloqueo;





typedef struct proceso_kernel{
    
    uint32_t pid ;
    int conexion; // es el socket del proceso
	int conexionConMemoria; //socket para las tareas que necesitemos la memoria

    double tiempoDeEspera; // para HRRN
    double ultimaRafagaEjecutada ; // este es el real Anterior para SJF
    double rafagaEstimada; //para JFS
    double responseRatio; // HRRN

	int vuelveDeBloqueo; //esto lo vamos a utilizar para ver si lo utlimo que realizo fue un bloqueo o no
	t_list* listaRecursosRetenidos;
	t_list* listaRecursosSolicitados;
	
    struct timespec tiempoDeArriboColaReady; //esto nos va a servir cuando queremos calcular el tiempo que estuvo esperando un proceso en la cola de Ready, donde esta variable va a ser el inicio de cuando entro a ready

    struct proceso_kernel* procesoApuntadoDeadlock;

}proceso_kernel ;

/* MEMORIA */

typedef enum{
    PRIMERA_VEZ,
    AGREGAR_ALLOC
}stream_alloc;


typedef struct {

    int32_t prevAlloc;
    int32_t nextAlloc;
    uint8_t isFree;

} __attribute__((packed)) heapMetadata;


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
    t_list* tabla_de_paginas;
    uint32_t conexion;
    uint32_t tlb_hit;
    uint32_t tlb_miss;
    uint32_t punteroClock;
    uint32_t contadorPag;

}t_carpincho;


typedef struct {

    t_list* paginas;

}tablaDePagina;




/* ----------------------------- */
typedef enum{
	INICIALIZAR_ESTRUCTURA,
	CERRAR_INSTANCIA,
	INICIAR_SEMAFORO,
	SEM_WAIT,
	SEM_SIGNAL,
	CERRAR_SEMAFORO,
	CONECTAR_IO,
	MEMALLOC,
	MEMFREE,
	MEMREAD,
	MEMWRITE,
	ESCRITURA_PAGINA,
	LECTURA_PAGINA,
	TIPOASIGNACION,
	SEM_WAIT_NOBLOQUEANTE,
	SUSPENSION_PROCESO, //este codigo va a ser por el cual desde el kernel le vamos a avisar que un proceso se suspende y vamos a solicitar suspender todas sus paginas
	CONSULTAR_ESPACIO,
	FALLO_IO,
	FINALIZAR_PROCESO
}cod_operacion;

typedef struct buffer
{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct paquete
{
	cod_operacion codigo_operacion;
	t_buffer* buffer;
} t_paquete;



uint32_t iniciar_servidor(char* ip_servidor, char* puerto);
int esperar_cliente(int socket_servidor);
int crear_conexion(char *ip, char* puerto);
void* serializar_paquete(t_paquete* paquete, int bytes);
void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(cod_operacion codigo);
void enviarPaquete(t_paquete* paquete, int conexion);

#endif