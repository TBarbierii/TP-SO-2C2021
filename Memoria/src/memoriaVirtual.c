#include "memoriaVirtual_suspencion.h"

int32_t buscar_TLB(t_pagina* pagina){

	
	bool buscarPagina(t_pagina *pag){
		return pag->id_pagina == pagina->id_pagina && pag->id_carpincho == pagina->id_carpincho;
	};

	pthread_mutex_lock(TLB_mutex);
	t_pagina* paginaEncontrada = list_find(TLB, (void*)buscarPagina);
	pthread_mutex_unlock(TLB_mutex);

	if(paginaEncontrada == NULL){
		return -1;
	}

	return paginaEncontrada->marco->comienzo;
	
}

t_marco* reemplazarPagina(t_carpincho* carpincho){

	if(strcmp(tipoAsignacion, "FIJA") == 0){

		bool paginasPresentes(t_pagina* pag){
			return pag->presente;
		};
		
		pthread_mutex_lock(tabla_paginas);
		t_list* paginas_a_reemplazar = list_filter(carpincho->tabla_de_paginas, (void*)paginasPresentes);
		t_pagina* victima = algoritmo_reemplazo_MMU(paginas_a_reemplazar, carpincho);   
		pthread_mutex_unlock(tabla_paginas);

		log_warning(logsObligatorios, "[SWAP] Pagina víctima: Pid: %i, Página: %i, Marco: %i", carpincho->id_carpincho, victima->id_pagina, victima->marco->id_marco);

		void* contenido = malloc(tamanioPagina);
		pthread_mutex_lock(memoria);
		memcpy(contenido, memoriaPrincipal + victima->marco->comienzo, tamanioPagina);
		pthread_mutex_unlock(memoria);

		if(victima->modificado)
		enviar_pagina(carpincho->id_carpincho, victima->id_pagina, contenido);
		

		victima->presente = false;
		victima->marco->estaLibre = true;
		
		pthread_mutex_lock(TLB_mutex);

		bool quitarDeTLB(t_pagina* pag){
			return victima->id_pagina == pag->id_pagina && victima->id_carpincho ==  pag->id_carpincho;
		};
		list_remove_by_condition(TLB, (void*)quitarDeTLB);// se quita directamente la pagina que se mando a swap.
		log_trace(logsObligatorios, "[TLB] Victima: PID: %i	Página: %i	Marco: %i", victima->id_carpincho, victima->id_pagina, victima->marco->id_marco);
		pthread_mutex_unlock(TLB_mutex);
		list_destroy(paginas_a_reemplazar); 
		free(contenido);

		return victima->marco;

	}else if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
		bool paginasPresentes(t_pagina* pag){
			return pag->presente;
		};

		t_list* paginas_a_reemplazar = list_create();
		
		pthread_mutex_lock(listaCarpinchos);
		int cantidad = list_size(carpinchos);
		pthread_mutex_unlock(listaCarpinchos);
		
		for (int i=0; i<cantidad; i++){
			t_list* paginas;
			pthread_mutex_lock(listaCarpinchos);
			t_carpincho* carp = list_get(carpinchos, i);
			pthread_mutex_unlock(listaCarpinchos);

			pthread_mutex_lock(tabla_paginas);
			paginas = list_filter(carp->tabla_de_paginas, (void*)paginasPresentes);
			pthread_mutex_unlock(tabla_paginas);

			list_add_all(paginas_a_reemplazar, paginas);
			list_destroy(paginas);	
		}

		
		pthread_mutex_lock(tabla_paginas);
		t_pagina* victima = algoritmo_reemplazo_MMU(paginas_a_reemplazar, carpincho); 
		pthread_mutex_unlock(tabla_paginas);

		log_warning(logsObligatorios, "[SWAP] Pagina víctima: Pid: %i, Página: %i, Marco: %i", victima->id_carpincho, victima->id_pagina, victima->marco->id_marco);

		list_destroy(paginas_a_reemplazar);  												

		void* contenido = malloc(tamanioPagina);
		
		for(int j=0; j<tamanioPagina;j++){
            char valor = '\0';
            memcpy(contenido +j, &valor, 1);
        }

		pthread_mutex_lock(memoria);
		memcpy(contenido, memoriaPrincipal + victima->marco->comienzo, tamanioPagina);
		pthread_mutex_unlock(memoria);

		if(victima->modificado)
		enviar_pagina(victima->id_carpincho, victima->id_pagina, contenido);
		
		pthread_mutex_lock(tabla_paginas);
		victima->presente = false;
		victima->marco->estaLibre = true;
		victima->marco->proceso_asignado = -1;
		pthread_mutex_unlock(tabla_paginas);
		
		pthread_mutex_lock(TLB_mutex);
		bool quitarDeTLB(t_pagina* pag){
			return victima->id_pagina == pag->id_pagina && victima->id_carpincho == pag->id_carpincho;
		};
		list_remove_by_condition(TLB, (void*)quitarDeTLB);// se quita directamente la pagina que se mando a swap.
		pthread_mutex_unlock(TLB_mutex);
		free(contenido);

		return victima->marco;
		
	}else{
		return NULL;
	}


}

t_pagina* algoritmo_reemplazo_MMU(t_list* paginas_a_reemplazar, t_carpincho* carpincho){

	if(strcmp(algoritmoReemplazoMMU, "LRU") == 0){
		
		bool comparator(t_pagina* p1, t_pagina* p2){
			return p1->ultimoUso <= p2->ultimoUso;
		};

		t_list* paginasOrdenadas = list_sorted(paginas_a_reemplazar, (void*)comparator);

		t_pagina* pag = list_get(paginasOrdenadas, 0);

		list_destroy(paginasOrdenadas);
		return pag;
	}

	if(strcmp(algoritmoReemplazoMMU, "CLOCK-M") == 0){

		bool comparator(t_pagina* p1, t_pagina* p2){
			return p1->marco->id_marco < p2->marco->id_marco;
		};
		
		t_list* paginasOrdenadas = list_sorted(paginas_a_reemplazar, comparator);

		if(strcmp(tipoAsignacion, "FIJA") == 0){

		int puntero = carpincho->punteroClock;

		segundoIntento:

		for(int a = 0; a < list_size(paginasOrdenadas);a++){ //primera vuelta

			t_pagina* candidata = list_get(paginasOrdenadas, puntero);
			if(candidata->uso == 0 && candidata->modificado == 0){
				
				carpincho->punteroClock++;
				
				if(carpincho->punteroClock >= list_size(paginasOrdenadas)){
					carpincho->punteroClock = 0;
				}

				list_destroy(paginasOrdenadas);
				return candidata;

			}else{
				puntero++;
				if(puntero >= list_size(paginasOrdenadas)){
					puntero = 0;
				}
			}

		}

		for(int a = 0; a < list_size(paginasOrdenadas);a++){ //segunda vuelta

			t_pagina* candidata = list_get(paginasOrdenadas, puntero);
			
			if(candidata->uso == 0 && candidata->modificado == 1){
				carpincho->punteroClock++;

				if(carpincho->punteroClock >= list_size(paginasOrdenadas)){
					carpincho->punteroClock = 0;
				}

				list_destroy(paginasOrdenadas);
				return candidata;
				
			}else{
				
				puntero++;
				candidata->uso = false;
				if(puntero >= list_size(paginasOrdenadas)){
					puntero = 0;
				}
			}

		}

		goto segundoIntento; 
		//si llegó hasta aca es porque hizo las dos vueltas y tiene que empezar de nuevo

		list_destroy(paginasOrdenadas);

		}else if(strcmp(tipoAsignacion, "DINAMICA") == 0){

		punteroClock;

		segundoIntentoDinamica:

		for(int a = 0; a < list_size(paginasOrdenadas);a++){ //primera vuelta

			t_pagina* candidata = list_get(paginasOrdenadas, punteroClock);
			if(candidata->uso == 0 && candidata->modificado == 0){
				punteroClock++;
				if(punteroClock >= list_size(paginasOrdenadas)){
					punteroClock = 0;
				}
				list_destroy(paginasOrdenadas);
				return candidata;
			}else{
				punteroClock++;
				if(punteroClock >= list_size(paginasOrdenadas)){
					punteroClock = 0;
				}
			}

		}

		for(int a = 0; a < list_size(paginasOrdenadas);a++){ //segunda vuelta

			t_pagina* candidata = list_get(paginasOrdenadas, punteroClock);
			
			if(candidata->uso == 0 && candidata->modificado == 1){
				punteroClock++;
				if(punteroClock >= list_size(paginasOrdenadas)){
					punteroClock = 0;
				}
				list_destroy(paginasOrdenadas);
				return candidata;
				
			}else{
				punteroClock++;
				candidata->uso = false;
				if(punteroClock >= list_size(paginasOrdenadas)){
					punteroClock = 0;
				}
			}

		}

		goto segundoIntentoDinamica; //si llegó hasta aca es porque hizo las dos vueltas y tiene que empezar de nuevo

		list_destroy(paginasOrdenadas);
		}

	}
}

void algoritmo_reemplazo_TLB(t_pagina* pagina){

	pthread_mutex_lock(TLB_mutex);
	//recordar qus si la cantidad de entradas de TLB es igual a 0, no se deberia agregar nada
	if(list_size(TLB) == cantidadEntradasTLB && cantidadEntradasTLB != 0){

		if(strcmp(algoritmoReemplazoTLB, "LRU") == 0){
					
			
			bool comparator(t_pagina* p1, t_pagina* p2){
				return p1->ultimoUso < p2->ultimoUso;
			};

			t_list* paginasOrdenadas = list_sorted(TLB, (void*)comparator);

			if(!list_is_empty(paginasOrdenadas)) {
				t_pagina* pag = list_remove(paginasOrdenadas,0);

				//SI le ponemos un list_remove, lo saca de la lista directamente y no es necesario hacer un remove_by_condition
				//Si porque ahi lo esta sacando de la lista ordenada por el ultimo uso, y con ese id lo tiene que sacar de la TLB posta. list_sorted devuelve una nueva lista
				 
				void buscarPag(t_pagina* p){
					return p->id_pagina == pag->id_pagina;
				};

				list_remove_by_condition(TLB, (void*)buscarPag);

				log_trace(logsObligatorios, "[TLB] Victima: PID: %i	Página: %i	Marco: %i", pag->id_carpincho, pag->id_pagina, pag->marco->id_marco);

			}

			list_add(TLB, pagina);
			pthread_mutex_unlock(TLB_mutex);

			log_trace(logsObligatorios, "[TLB] NuevaEntrada: PID: %i	Página: %i	Marco: %i", pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);


		}else if(strcmp(algoritmoReemplazoTLB, "FIFO") == 0){

			t_pagina* pag = list_remove(TLB,0);
			log_trace(logsObligatorios, "[TLB] Victima: PID: %i	Página: %i	Marco: %i", pag->id_carpincho, pag->id_pagina, pag->marco->id_marco);

			list_add(TLB, pagina);
			pthread_mutex_unlock(TLB_mutex);

			log_trace(logsObligatorios, "[TLB] NuevaEntrada: PID: %i	Página: %i	Marco: %i", pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);

		}

	}else if(cantidadEntradasTLB != 0){
		
		if(pagina->presente){
			list_add(TLB, pagina);
		}
		pthread_mutex_unlock(TLB_mutex);
		
		log_trace(logsObligatorios, "[TLB] NuevaEntrada: PID: %i	Página: %i	Marco: %i", pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);

	}else{
		pthread_mutex_unlock(TLB_mutex);
	}
}

uint32_t swapear(t_carpincho* carpincho, t_pagina* paginaPedida){
	
	pthread_mutex_lock(swap);
	t_marco* marcoLiberado = reemplazarPagina(carpincho);
	uint32_t conexion = pedir_pagina(paginaPedida->id_pagina, carpincho->id_carpincho);
	void* contenido = atender_respuestas_swap(conexion);
	pthread_mutex_unlock(swap);
	pthread_mutex_lock(tabla_paginas);
	paginaPedida->marco = marcoLiberado;
	paginaPedida->marco->proceso_asignado = carpincho->id_carpincho;
	paginaPedida->marco->estaLibre = false;
	paginaPedida->presente = true;
	paginaPedida->ultimoUso = clock();
	paginaPedida->uso = true;
	paginaPedida->modificado = false;
	pthread_mutex_unlock(tabla_paginas);

	log_warning(logsObligatorios, "[SWAP] Pagina entrante: Pid: %i, Página: %i, Marco: %i", carpincho->id_carpincho, paginaPedida->id_pagina, paginaPedida->marco->id_marco);


//	heapMetadata* heap = malloc(TAMANIO_HEAP);
//	memcpy(heap, contenido, TAMANIO_HEAP);

   	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + paginaPedida->marco->comienzo, contenido, tamanioPagina);
	pthread_mutex_unlock(memoria);


	algoritmo_reemplazo_TLB(paginaPedida);
	free(contenido);
	return marcoLiberado->comienzo;
}

int32_t buscarEnTablaDePaginas(t_carpincho* carpincho, int32_t idPag){

	bool buscarPaginaPresente(t_pagina* pag){
		return pag->id_pagina == idPag && pag->presente;
	};
	pthread_mutex_lock(tabla_paginas);
	t_pagina* pagina = list_find(carpincho->tabla_de_paginas, (void*)buscarPaginaPresente);
	pthread_mutex_unlock(tabla_paginas);

	if(pagina == NULL){
		return -1;
	}

	algoritmo_reemplazo_TLB(pagina);

	return pagina->marco->comienzo;

}

void reemplazo(int32_t *DF, t_carpincho* carpincho, t_pagina* pagina){

	if(*DF == -1){ //tlb miss
		usleep(retardoFAlloTLB * 1000);
		*DF = buscarEnTablaDePaginas(carpincho, pagina->id_pagina);
		
		if(*DF == -1) *DF = swapear(carpincho, pagina);
			carpincho->tlb_miss++;

			pthread_mutex_lock(miss_sem);
			miss_totales++;
			pthread_mutex_unlock(miss_sem);

	}else{//hit
		
		usleep(retardoAciertoTLB * 1000);
		carpincho->tlb_hit++;
		pthread_mutex_lock(hits_sem);
		hits_totales++;
		pthread_mutex_unlock(hits_sem);
	}

}