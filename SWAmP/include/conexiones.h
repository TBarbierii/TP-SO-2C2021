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
void atender_conexion_ram(uint32_t);
void recibir_tipo_asignacion(t_buffer*);
uint32_t recibir_operacion(uint32_t);
void enviar_pagina(uint32_t, void*);
void recibir_pagina(uint32_t);

#endif