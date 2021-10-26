#ifndef DISPOSITIVOSIO_H
#define DISPOSITIVOSIO_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"
#include "Semaforos.h"

/* estructura dispositivos IO */
typedef struct 
{
    char* nombre;
    int duracionRafaga;
    t_list* listaDeProcesosEnEspera;
    sem_t* activadorDispositivo;
    pthread_mutex_t* mutex;

}dispositivoIO;

void ejecutarDispositivosIO();
void rutinaDispositivoIO(dispositivoIO* dispositivo);
void agregarProcesoADispositivo(proceso_kernel* proceso, dispositivoIO* device);
int realizarOperacionIO(int pid, char* nombreDevice);


#endif