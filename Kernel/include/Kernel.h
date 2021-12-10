#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
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
#include "PlanificadorMedianoPlazo.h"
#include "PlanificadorCortoPlazo.h"
#include "Semaforos.h"
#include "Servidor.h"
#include "DispositivosIO.h"
#include "Deadlock1.h"



/* contador de procesos */
uint32_t cantidadDeProcesosActual;

pthread_mutex_t* contadorProcesos;

/* variables obtenidas del config*/

char* ipKernel;
char* ipMemoria;
char* puertoMemoria;
char* puertoServer;
char* algoritmoPlanificacion;
double estimacion_inicial;
double alfa;
/* listas de dispositivos_io, duraciones_io */
int retardoCPU;
int gradoMultiProgramacion;
int gradoMultiProcesamiento;
int tiempoDeadlock;

/* colas de planificacion */

t_list* procesosNew;
t_list* procesosReady;
t_list* procesosExec;
t_list* procesosBlocked;
t_list* procesosExit; //esta quiza ni la necesitemos
t_list* procesosSuspendedBlock;
t_list* procesosSuspendedReady;




/* lista de semaforos actuales inicializados */
t_list* semaforosActuales;


/* lista de dispositivos IO inicializados*/
t_list* dispositivosIODisponibles;



//Semaforos compartidos en los distintos planificadores

pthread_mutex_t * modificarReady;
pthread_mutex_t * modificarNew;
pthread_mutex_t * modificarExec;
pthread_mutex_t * modificarExit;
pthread_mutex_t * modificarSuspendedReady;
pthread_mutex_t * modificarBlocked;
pthread_mutex_t * modificarSuspendedBlocked;

sem_t * nivelMultiProgramacionBajaPrioridad; //esto es para el planificador de largo plazo

sem_t* hayProcesosNew;
sem_t* hayProcesosReady;

sem_t* nivelMultiprocesamiento;
sem_t* nivelMultiProgramacionGeneral;
sem_t* signalSuspensionProceso; // este semaforo lo vamos a utilizar desde block para que el algortimo de medianoplazo verifique si tiene que pasar un proceso a suspendido


sem_t* procesoNecesitaEntrarEnReady;

/* otros mutex */

/* semaforo para alertar a los threads CPU que hay procesos para ejecutar */
sem_t* procesosDisponiblesParaEjecutar;



//mutex de semaforos Y IO

pthread_mutex_t* controladorSemaforos;
pthread_mutex_t* controladorIO;


/* funciones */
void inicializarListas();
void inicializarSemaforosGlobales();
void finalizarListas();
t_config* inicializarConfig(char*);

void obtenerValoresDelConfig(t_config* configActual);
void inicializarDispositivosIO(char ** dispositivos, char** duraciones);
void finalizarConfig(t_config* configUsado);
void finalizarDispositivosIO();




#endif