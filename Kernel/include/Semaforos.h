#ifndef SEMAFOROS_H
#define SEMAFOROS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"

int crearSemaforo(char* nombreSem, unsigned int valorSem);
int destruirSemaforo(char* nombreSem);
int realizarSignalDeSemaforo(char* nombreSem);
void ponerEnElReadyIndicado(proceso_kernel* procesoBuscado);


#endif