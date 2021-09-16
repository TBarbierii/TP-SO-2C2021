#ifndef SEMAFOROS_H
#define SEMAFOROS_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"

void crearSemaforo(char* nombreSem, unsigned int valorSem);
void destruirSemaforo(char* nombreSem);

#endif