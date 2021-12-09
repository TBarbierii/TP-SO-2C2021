#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "Swamp_lib.h"
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <semaphore.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>


// Declaracion de funciones

int iniciar_servidor_swamp();
int atender_mensaje_ram(int conexion);
uint32_t recibir_tipo_asignacion(t_buffer*, t_log*);
uint32_t recibir_operacion(uint32_t socket_cliente);
void enviar_pagina(void* contenido, int conexion);
void recibir_pagina(t_buffer*, t_log*);
void atender_solicitud_pedido_de_pagina(t_buffer* buffer, int conexion, t_log* logger);
void notificar_escritura_de_pagina(int conexion);
void atender_solicitud_cierre_proceso(t_buffer* buffer, t_log* logger);
void notificar_finalizacion_de_proceso(int conexion);
void atender_solicitud_consulta_espacio(t_buffer* buffer, int conexion, t_log* logger);
void notificar_insuficiencia_de_espacio_para_proceso(int conexion, int valor);

#endif