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
#include "DispositivosIO.h"
#include "PlanificadorLargoPlazo.h"

void atenderSolicitudesKernel();
int atenderMensajeEnKernel(int conexion);

void enviarInformacionAdministrativaDelProceso(proceso_kernel* proceso);
void inicializarProcesoNuevo(int conexion,t_log* logger);

void cerrarProceso(t_buffer* bufferActual,t_log* logger);
void informarCierreDeProceso(proceso_kernel* proceso,t_log* loggerActual);

void iniciarSemaforo(t_buffer * buffer, int conexion);
void avisarInicializacionDeSemaforo(int conexion, int valor);

void cerrarSemaforo(t_buffer * buffer, int conexion);
void avisarDestruccionDeSemaforo(int conexion, int valor);

void hacerPostDeSemaforo(t_buffer * buffer, int conexion);
void avisarPostDeSemaforo(int conexion, int valor);

int hacerWaitDeSemaforo(t_buffer * buffer, int conexion);
void avisarWaitDeSemaforo(int conexion, int valor);


int conectarDispositivoIO(t_buffer* buffer, int conexion);
void avisarconexionConDispositivoIO(int conexion, int valor);


/* CONEXIONES Por parte del KERNEL con MEMORIA */
void establecerConexionConLaMemoria(proceso_kernel* proceso,t_log* logger);
int atenderMensajeDeMemoria(proceso_kernel* proceso);
void inicializarEnMemoria(proceso_kernel* proceso, t_log* logger);
void finalizarEnMemoria(proceso_kernel* proceso, t_log* logger);

/* CONEXIONES INTERMEDIAS */

/* recibos desde matelib*/
void memAlloc(t_buffer* buffer, t_log* logger);
void memFree(t_buffer* buffer, t_log* logger);
void memRead(t_buffer* buffer, t_log* logger);
int memWrite(t_buffer* buffer, t_log* logger);

/* solicitudes a memoria */
void realizarMemAlloc(int conexion, uint32_t pid, int size);
void realizarMemFree(int conexion, uint32_t pid, int32_t addr);
void realizarMemRead(int conexion, uint32_t pid, int32_t origin, int size);
void realizarMemWrite(int conexion, uint32_t pid, void *origin, int32_t dest, int size);

/* recibos de memoria y envio a matelib*/
void notificacionMemAlloc(proceso_kernel* proceso, t_buffer* buffer, t_log* logger);
void notificacionMemFree(proceso_kernel* proceso, t_buffer* buffer, t_log* logger);
void notificacionMemRead(proceso_kernel* proceso, t_buffer* buffer, t_log* logger);
void notificacionMemWrite(proceso_kernel* proceso, t_buffer* buffer, t_log* logger);
int notificacionInicializacionDeMemoria(t_buffer* buffer,t_log* logger);
int notificacionFinalizacionMemoria(t_buffer* buffer,t_log* logger);
void notificacionSuspensionProceso(proceso_kernel* proceso, t_buffer* buffer, t_log* logger);

/*esto es algo general para avisar cuando no se pudo realizar algo de memoria a la matelib */
int validacionConexionConMemoria(proceso_kernel* proceso, t_log* logger);
void notificarQueNoSePudoRealizarTareaConMemoria(cod_operacion operacionSolicitada, proceso_kernel* proceso);
void notificarSuspensionDeProceso(proceso_kernel* proceso, t_log* logger);


#endif