#ifndef KERNEL_H
#define KERNEL_H


#include<commons/log.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include <semaphore.h>
#include <string.h>
#include <commons/string.h>
#include "PlanificadorLargoPlazo.h"
#include "Semaforos.h"
#include <pthread.h>

/* Procesos */
typedef struct{
    int pid ;
    int conexion; // es el socket del proceso -- duda
    int estimacionAnterior; // es para el algoritmo SJF 
    int tiempoDeEspera; // para HRRN
    int ultimaRafaga ; 

}proceso ;


/* contador de procesos */
int cantidadDeProcesosActual;

pthread_mutex_t* contadorProcesos;

/* variables obtenidas del config*/

char* ipMemoria;
char* puertoMemoria;
char* algoritmoPlanificacion;
int retardoCPU;
int gradoMultiProgramacion;
int gradoMultiProcesamiento;

/* colas de planificacion */

t_list* procesosNew;
t_list* procesosReady;
t_list* procesosExec;
t_list* procesosExit; //esta quiza ni la necesitemos
t_list* procesosSuspendedBlock;
t_list* procesosSuspendedReady;


/* lista de semaforos actuales inicializados */
t_list* semaforosActuales;

/* estructura de los semaforos */
typedef struct /* */
{
    char* nombre;
    int valor;
    sem_t* semaforoActual; //quiza esto no va a ser necesario, xq quiza tengamos que hacerlo mas teorico nosotros
    t_list* listaDeProcesosEnEspera;

}semaforo;


/* lista de dispositivos IO inicializados*/
t_list* dispositivosIODisponibles;
/* estructura dispositivos IO */
typedef struct 
{
    char* nombre;
    int duracionRafaga;
    t_list* listaDeProcesosEnEspera;
}dispositivoIO;

//Semaforos compartidos en los distintos planificadores

pthread_mutex_t * modificarReady;
pthread_mutex_t * modificarNew;
pthread_mutex_t * modificarExec;
pthread_mutex_t * modificarExit;
pthread_mutex_t * modificarSuspendedReady;

pthread_mutex_t * nivelMultiProgramacionBajaPrioridad; //esto es para el planificador de largo plazo

sem_t* hayProcesosNew;
sem_t* hayProcesosReady;

sem_t* nivelMultiprocesamiento;
sem_t* nivelMultiProgramacionGeneral;

sem_t* procesoNecesitaEntrarEnReady;



/* funciones */
void inicializarListas();
void inicializarSemaforosGlobales();
void finalizarListas();
t_config* inicializarConfig();

void obtenerValoresDelConfig(t_config* configActual);
void inicializarDispositivosIO(char ** dispositivos, char** duraciones);
void finalizarConfig(t_config* configUsado);
void finalizarDispositivosIO();

#endif