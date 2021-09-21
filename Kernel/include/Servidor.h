#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"


void atenderSolicitudesKernel(char* ip_servidor, char* puerto);
int atenderMensajeEnKernel(int conexion);
void inicializarProcesoNuevo(int conexion);

#endif