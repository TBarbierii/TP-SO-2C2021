#ifndef PLANIFICADOR_MEDIANO_PLAZO_H
#define PLANIFICADOR_MEDIANO_PLAZO_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"



/* funciones */

void planificadorMedianoPlazo();
void thread1_PMP(t_log* logger);
void thread2_PMP(t_log* logger);

#endif