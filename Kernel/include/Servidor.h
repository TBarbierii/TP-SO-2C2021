#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "Kernel.h"
#include "Semaforos.h"


void atenderSolicitudesKernel();

int atenderMensajeEnKernel(int conexion);
void enviarInformacionAdministrativaDelProceso(proceso_kernel* proceso);
void inicializarProcesoNuevo(int conexion,t_log* logger);
void cerrarProceso(t_buffer* bufferActual,t_log* logger);
void informarCierreDeProceso(proceso_kernel* proceso,t_log* loggerActual);
void iniciarSemaforo(t_buffer * buffer, int conexion);
void avisarInicializacionDeSemaforo(int conexion);


#endif