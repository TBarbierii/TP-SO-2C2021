#include "Memoria.h"


void* recibir_buffer(uint32_t* , int );
void atender_solicitudes_multihilo();
void atender_solicitudes(uint32_t);
uint32_t recibir_memalloc(int);
void inicializar_carpincho(int,t_log*);
uint32_t recibir_memfree(int);
uint32_t recibir_memread(int);
uint32_t recibir_memwrite(int);
uint32_t cerrar_carpincho(int,t_log*);
void enviar_tipo_asignacion(char*);
void enviar_pagina(uint32_t, void*);
void pedir_pagina(uint32_t);