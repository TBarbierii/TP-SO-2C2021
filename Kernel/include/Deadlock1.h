#ifndef DEADLOCK1_H
#define DEADLOCK1_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"
#include "Semaforos.h"

void ejecutarAlgoritmoDeadlock();
void bloquearTodosLosSemaforos();
void desbloquearTodosLosSemaforos();
int procesoReteniendoProcesosYEsperando(proceso_kernel* proceso);
t_list* procesosQueEstanReteniendoYEsperando();
void rellenarVectorDisponibles(t_list* listaSemaforos, int vector[]);




#endif