#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include <semaphore.h>


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
t_list* procesosExit;
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


void inicializarListas();
void finalizarListas();
t_config* inicializarConfig();
void finalizarConfig(t_config* configUsado);


#endif