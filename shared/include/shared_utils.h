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

/* Procesos */
typedef struct proceso{
    
    uint32_t pid ;
    int conexion; // es el socket del proceso
    double tiempoDeEspera; // para HRRN
    double ultimaRafagaEjecutada ; // este es el real Anterior para SJF
    double rafagaEstimada; //para JFS
    int responseRatio; // HRRN
    clock_t tiempoDeArriboColaReady; //esto nos va a servir cuando queremos calcular el tiempo que estuvo esperando un proceso en la cola de Ready, donde esta variable va a ser el inicio de cuando entro a ready

}proceso_kernel ;




t_log* logger;

typedef enum backend{

    ERROR = -1,
    KERNEL = 0,
    MEMORIA = 1 

}backend;


typedef enum{
	INICIALIZAR_ESTRUCTURA = 0,
	CERRAR_INSTANCIA = 1,
	INICIAR_SEMAFORO = 2,
	SEM_WAIT = 3,
	SEM_SIGNAL = 4,
	CERRAR_SEMAFORO = 5,
	CONECTAR_IO = 6

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
uint32_t esperar_cliente(uint32_t socket_servidor);
int crear_conexion(char *ip, char* puerto);
void* serializar_paquete(t_paquete* paquete, int bytes);
void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(cod_operacion codigo);
void enviarPaquete(t_paquete* paquete, int conexion);

#endif