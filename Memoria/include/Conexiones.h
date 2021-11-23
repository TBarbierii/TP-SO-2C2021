#include "Memoria.h"


void* recibir_buffer(int);
void atender_solicitudes_multihilo();
void atender_solicitudes_memoria(uint32_t);
uint32_t recibir_memalloc(int, t_log*);
void inicializar_carpincho(int,t_log*);
int32_t recibir_memfree(int, t_log*);
int32_t recibir_memread(int, t_log*);
int32_t recibir_memwrite(int, t_log*);
uint32_t recibir_suspencion(conexion, logger);
uint32_t cerrar_carpincho(uint32_t,t_log*);
void enviar_tipo_asignacion(char*);
void enviar_pagina(uint32_t,uint32_t, void*);
uint32_t pedir_pagina(uint32_t id_pagina, uint32_t pid);
void* atender_respuestas_swap(uint32_t conexion);
void* recibirPagina(int conexion);
void* recibir_respuesta_escritura(int conexion);
uint32_t recibir_respuesta_consulta(int conexion);
uint32_t finalizar_swap(uint32_t pid);
uint32_t recibir_respuesta_cierre(int conexion);
void responderOperacionNoValida(int conexion, cod_operacion tareaRealizada, t_log* logger);