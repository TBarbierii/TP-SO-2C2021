#ifndef PLANIFICADOR_LARGO_PLAZO_H
#define PLANIFICADOR_LARGO_PLAZO_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"
#include <semaphore.h>

//Variables globales

sem_t * semaforoDeMultiprogramacion;
sem_t * semaforoProcesosEnNew;

// -----

void planificadorLargoPlazo();
void inicializarSemaforos();


#endif