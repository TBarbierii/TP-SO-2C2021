#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include "Memoria.h"
#include "memoriaVirtual_suspencion.h"
#include "Conexiones.h"
#include <string.h>
#include "Memoria.h"


uint32_t administrar_allocs(uint32_t, uint32_t);

uint32_t administrar_paginas(t_carpincho* , uint32_t, t_list* );

void* generar_buffer_allocs(uint32_t, heapMetadata*,uint32_t, stream_alloc, int32_t);

void escribirMemoria(void* buffer, t_list* paginas, t_list* marcos_a_asignar, t_carpincho* carpincho );

t_list* reservarMarcos(uin32_t);

int buscarSiguienteHeapLibre(heapMetadata* , int32_t* , t_carpincho* , int32_t*, int32_t* );

t_list* buscarMarcosLibres(t_carpincho* carpincho);

uint32_t crearAllocNuevo(int* pagina, int tamanio, heapMetadata* heap, int posicionUltimoHeap, t_carpincho *carpincho, int32_t*);


#endif