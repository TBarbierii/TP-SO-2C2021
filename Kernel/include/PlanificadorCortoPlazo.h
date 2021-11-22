#ifndef PLANIFICADOR_CORTO_PLAZO_H
#define PLANIFICADOR_CORTO_PLAZO_H


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
#include <pthread.h>
#include "Kernel.h"
#include <time.h>

void planificadorCortoPlazo();
void replanificarSegunAlgoritmo();
void rutinaDeProceso();
int rompoElHiloSegunElCodigo(int codigo);
void inicializarHilosCPU();



void replanificacion(t_log* logger);

/* SJF */
void calcularEstimacion(proceso_kernel* unCarpincho);
bool comparadorDeRafagas(proceso_kernel* unCarpincho, proceso_kernel* otroCarpincho);
void aplicarSJF();

/* HRRN */
void AumentarTiempoEspera(proceso_kernel* unCarpincho);
void CalcularResponseRatio(proceso_kernel* unCarpincho);
bool comparadorResponseRatio(proceso_kernel* unCarpincho, proceso_kernel* otroCarpincho);
void aplicarHRRN();


#endif