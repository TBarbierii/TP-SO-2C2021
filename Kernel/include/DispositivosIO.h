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


void ejecutarDispositivosIO();
void rutinaDispositivoIO(dispositivoIO* dispositivo);
void agregarProcesoADispositivo(proceso_kernel* proceso, dispositivoIO* device);



#endif