#ifndef MEMWRITE_READ_FREE_H
#define MEMWRITE_READ_FREE_H
#include "Memoria.h"
#include "memoriaVirtual_suspencion.h"

void* leer_memoria(uint32_t DL, uint32_t carpincho, uint32_t tam);

void liberar_alloc(uint32_t, uint32_t);

void* leer_memoria(uint32_t DL, uint32_t carpincho, uint32_t tam);


#endif