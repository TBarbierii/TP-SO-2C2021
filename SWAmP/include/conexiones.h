#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "swamp_lib.h"
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
uint32_t recibir_operacion(uint32_t);
void enviar_pagina(void*, int);
void recibir_pagina(t_buffer*, t_log*);
void atender_solicitud_pedido_de_pagina(t_buffer* buffer, int conexion);
void notificar_escritura_de_pagina(int conexion);

#endif