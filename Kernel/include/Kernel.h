#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>

t_list* procesosNew;
t_list* procesosReady;
t_list* procesosExec;
t_list* procesosExit;
t_list* procesosBlockIO;
t_list* procesosSuspendedBlock;
t_list* procesosSuspendedReady;



void inicializarListas();
void finalizarListas();

#endif