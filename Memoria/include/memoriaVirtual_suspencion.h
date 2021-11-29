#ifndef MEMORIAVIRTUAL_SUSPENCION_H
#define MEMORIAVIRTUAL_SUSPENCION_H

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


int32_t buscar_TLB(t_pagina*);

t_marco* reemplazarPagina(t_carpincho* carpincho);

t_pagina* algoritmo_reemplazo_MMU(t_list* paginas_a_reemplazar, t_carpincho* carpincho);

uint32_t swapear(t_carpincho* carpincho, t_pagina* paginaPedida);

void algoritmo_reemplazo_TLB(t_pagina* pagina);

int32_t buscarEnTablaDePaginas(t_carpincho* carpincho, int32_t idPag);

void reemplazo(int32_t *DF, t_carpincho* carpincho, t_pagina* pagina);

// SUSPENCION //

uint32_t suspender_proceso(uint32_t pid);    

#endif