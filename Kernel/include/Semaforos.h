#ifndef SEMAFOROS_H
#define SEMAFOROS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"

/* estructura de los semaforos */
typedef struct semaforo
{
    int id;
    char* nombre;
    int valor;
    pthread_mutex_t* mutex; //quiza esto no va a ser necesario, xq quiza tengamos que hacerlo mas teorico nosotros
    t_list* listaDeProcesosEnEspera;

}semaforo;

int valorIdSemaforos;

int crearSemaforo(char* nombreSem, unsigned int valorSem);
int destruirSemaforo(char* nombreSem);
int realizarSignalDeSemaforo(char* nombreSem, int pid);
void ponerEnElReadyIndicado(proceso_kernel* procesoBuscado);
int realizarWaitDeSemaforo(char* nombreSem, int pid);
void desalojarSemaforosDeProceso(proceso_kernel* procesoASacarPorDeadlock);
void sacarProcesoDeBloqueado(int PID);

#endif