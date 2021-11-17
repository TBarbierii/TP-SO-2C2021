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
//int cantidadDeVecesQueProcesoRetieneASemaforo(proceso_kernel* procesoActual, semaforo* semaforoBuscado);
//int cantidadDeVecesQueProcesoPideASemaforo(proceso_kernel* procesoActual, semaforo* semaforoBuscado);
int procesoConMayorPID(proceso_kernel* p1, proceso_kernel* p2);
int indiceDondeProcesoEstaEnLaLista(int pid, t_list* lista);
void bloquearTodosLosSemaforos();
void desbloquearTodosLosSemaforos();
int procesoReteniendoProcesosYEsperando(proceso_kernel* proceso);
int procesoReteniendo(proceso_kernel* proceso);
t_list* procesosQueEstanReteniendoYEsperando(t_log* loggerActual);
void rellenarVectorDisponibles(t_list* listaSemaforos, int vector[]);
void finalizarProcesoPorDeadlock(proceso_kernel* procesoASacarPorDeadlock);




#endif