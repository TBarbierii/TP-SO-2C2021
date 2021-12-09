#include "memoriaVirtual_suspencion.h"

uint32_t suspender_proceso(uint32_t pid){

    bool buscarCarp(t_carpincho* carp){
        return carp->id_carpincho == pid;
    };
    
    pthread_mutex_lock(listaCarpinchos);
    t_carpincho* carpincho = list_find(carpinchos, (void*)buscarCarp);
    pthread_mutex_unlock(listaCarpinchos);

    bool paginasPresentes(t_pagina* pag){
	return pag->presente;
	};
		
	t_list* paginas_que_se_van = list_filter(carpincho->tabla_de_paginas, (void*)paginasPresentes);

    void volarPaginas(t_pagina* pagina){

        void* contenido = malloc(tamanioPagina);
        
		pthread_mutex_lock(memoria);
		memcpy(contenido, memoriaPrincipal + pagina->marco->comienzo, tamanioPagina);
		pthread_mutex_unlock(memoria);

		enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, contenido);

        pagina->marco->estaLibre = true;
        pagina->marco->proceso_asignado = -1;

    }
   
   list_iterate(paginas_que_se_van, (void*)volarPaginas);

    //list_destroy(paginas_que_se_van); ?

   
}