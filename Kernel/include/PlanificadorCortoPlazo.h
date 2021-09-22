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

void planificadorCortoPlazo();
void replanificarSegunAlgoritmo();
void rutinaDeProceso(proceso* procesoEjecutando);
int rompoElHiloSegunElCodigo(int codigo);


#endif