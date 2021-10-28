#include "Memoria.h"


void* recibir_buffer(uint32_t* , int );
void atender_solicitudes_multihilo();
void atender_solicitudes_memoria(uint32_t);
uint32_t recibir_memalloc(int, t_log*);
void inicializar_carpincho(int,t_log*);
uint32_t recibir_memfree(int, t_log*);
uint32_t recibir_memread(int, t_log*);
uint32_t recibir_memwrite(int, t_log*);
uint32_t cerrar_carpincho(uint32_t,t_log*);
void enviar_tipo_asignacion(char*);
void enviar_pagina(uint32_t,uint32_t, void*);
void pedir_pagina(uint32_t);