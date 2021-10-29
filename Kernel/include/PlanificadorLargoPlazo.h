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
#include "Servidor.h"


/* funciones */

void planificadorLargoPlazo();
void liberarProceso(proceso_kernel* procesoActual);

#endif
