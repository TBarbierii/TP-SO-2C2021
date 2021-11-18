#include "Memoria.h"

uint32_t generadorIdsPaginas(){

	int id;
	pthread_mutex_lock(controladorIdsPaginas);
	if(id_pag%10 == 0){ // no pueden ser multiplos de 10 porque al darlos vuelta lo toma como octal
		id = id_pag++;
	}
	
     id = id_pag++;
	 pthread_mutex_unlock(controladorIdsPaginas);
	 
	 return id;

}

uint32_t dar_vuelta_id(uint32_t num){

	char* id = string_new();

	uint32_t id_retornado;
	id = string_itoa(num);
	id = string_reverse(id);
	uint32_t veces = 3 - string_length(id);

	char* ceros = string_repeat('0', veces);

	char* a = string_new();
	string_append(&a, id);
	string_append(&a, ceros);

	return id_retornado = atoi(a);

}



uint32_t generarDireccionLogica(uint32_t id, uint32_t desplazamiento){

	uint32_t ID;
	ID = dar_vuelta_id(id);

	char* ID_STRING = string_itoa(ID);
	char* DESPLAZAMIENTO =  string_itoa(desplazamiento);

	char* a = string_new();
		string_append(&a, ID_STRING);
		string_append(&a, DESPLAZAMIENTO);

	uint32_t direccionLogica = atoi(a);
	return direccionLogica;
}

uint32_t obtenerId(uint32_t num){


	char* id = string_new();

	uint32_t id_retornado;

	id = string_itoa(num);
	id  = string_substring(id, 0, 3);

	id = string_reverse(id);


	return id_retornado = atoi(id);

}

uint32_t obtenerDesplazamiento(uint32_t num){

		char* id = string_new();

		uint32_t id_retornado;

		id = string_itoa(num);
		id =  string_substring_from(id, 3);

		return id_retornado = atoi(id);

}

uint32_t calcular_direccion_fisica(uint32_t carpincho, uint32_t direccionLogica){

	uint32_t direccionFisica;

	uint32_t id = obtenerId(direccionLogica);

	uint32_t desplazamiento = obtenerDesplazamiento(direccionLogica);

		bool buscarCarpincho(t_carpincho* s){
			return s->id_carpincho == carpincho;
		};
		pthread_mutex_lock(listaCarpinchos);
		t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);
		pthread_mutex_unlock(listaCarpinchos);

		bool buscarPagina(t_pagina* s){
			return s->id_pagina == id;
		};

		t_pagina* pagina = list_find(capybara->tabla_de_paginas,(void*)buscarPagina);

		direccionFisica = pagina->marco->comienzo + desplazamiento; //ver si es . o ->

	return direccionFisica;
}

uint32_t aumentarIdCarpinchos(){
	pthread_mutex_lock(controladorIds);
    id_carpincho++;
	pthread_mutex_unlock(controladorIds);

}



t_list* reservarMarcos(uint32_t pid){

		t_list* marcos_a_asignar;

		bool noEstanAsignados(t_marco* marco){
		return marco->proceso_asignado == -1;
        };

	 	t_list *marcos_sin_asignar = list_filter(marcos, (void*)noEstanAsignados);

		if(strcmp(tipoAsignacion, "FIJA") == 0){
			 
			marcos_a_asignar = list_take(marcos_sin_asignar, marcosMaximos);
			
			void marcarOcupados(t_marco *marco){
			marco->proceso_asignado = pid;
       		};

       		list_iterate(marcos_a_asignar, (void*)marcarOcupados);

			return marcos_a_asignar;   
			
		}
		if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
			return marcos_sin_asignar;
		}else{
			return NULL;
		}
}

void escribirMemoria(void* buffer, t_list* paginas, t_list* marcos_a_asignar ){
	    
	int contador = 0; 

	void escribir_paginas_en_marcos(t_pagina* pag){

	t_marco* marco = list_get(marcos_a_asignar, contador);

	if(marco->estaLibre){
	
	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + marco->comienzo, buffer + (contador*tamanioPagina), tamanioPagina);
	pthread_mutex_unlock(memoria);


	pag->marco = marco;
	pag->esNueva = false;
	marco->estaLibre = false;
	}
	contador++;
	};

	list_iterate(paginas, (void*)escribir_paginas_en_marcos);

}

int32_t buscar_TLB(uint32_t idPag){

	bool buscarPagina(t_pagina *pag){
		return pag->id_pagina == idPag;
	};
	pthread_mutex_lock(TLB_mutex);
	t_pagina* pagina = list_find(TLB, (void*)buscarPagina);
	pthread_mutex_unlock(TLB_mutex);

	if(pagina == NULL){
		return -1;
	}
	
	return pagina->marco->comienzo;
	
}



int buscarSiguienteHeapLibre(heapMetadata* heap, int32_t *DF, t_carpincho* carpincho, int32_t *posicionHeap, int32_t *despl){

	int pagina;
	t_pagina *paginaDeSiguienteHeap;
	int i;

	do 
	{
		int posicionSiguienteHeap = heap->nextAlloc;
		*posicionHeap = posicionSiguienteHeap;

        for(i=1; i <= list_size(carpincho->tabla_de_paginas); i++){

            if(posicionSiguienteHeap < tamanioPagina * i){
            paginaDeSiguienteHeap = list_get(carpincho->tabla_de_paginas, i-1);
			pagina = paginaDeSiguienteHeap->id_pagina;
            break; 
            }    
        }

        uint32_t desplazamiento =  tamanioPagina - (i * tamanioPagina - posicionSiguienteHeap); //el desplazamiento relativo a la pagina

		*DF = buscar_TLB(paginaDeSiguienteHeap->id_pagina);
		if(*DF == -1){ //tlb miss
            *DF = buscarEnTablaDePaginas(carpincho, paginaDeSiguienteHeap->id_pagina);
            if(*DF == -1) *DF = swapear(carpincho, paginaDeSiguienteHeap);
		    carpincho->tlb_miss++;
			miss_totales++;
        }else{//hit
            carpincho->tlb_hit++;
			hits_totales++;
        }
		
		if(tamanioPagina - desplazamiento < TAMANIO_HEAP){ //esta cortado
			
			*despl = desplazamiento;
			void* buff_heap = malloc(TAMANIO_HEAP);
			pthread_mutex_lock(memoria);
			memcpy(buff_heap, memoriaPrincipal + (*DF + desplazamiento), (tamanioPagina - desplazamiento));
			pthread_mutex_unlock(memoria);
			
			paginaDeSiguienteHeap->ultimoUso = clock();
			paginaDeSiguienteHeap->uso = true;

			paginaDeSiguienteHeap = list_get(carpincho->tabla_de_paginas, i);

			*DF = buscar_TLB(paginaDeSiguienteHeap->id_pagina);

			if(*DF == -1){ //tlb miss
            	*DF = buscarEnTablaDePaginas(carpincho, paginaDeSiguienteHeap->id_pagina);
            	if(*DF == -1) *DF = swapear(carpincho, paginaDeSiguienteHeap);
			    carpincho->tlb_miss++;
				miss_totales++;
			}else{//hit
				carpincho->tlb_hit++;
				hits_totales++;
			}

			pthread_mutex_lock(memoria);
			memcpy(buff_heap + (tamanioPagina - desplazamiento), memoriaPrincipal + *DF , TAMANIO_HEAP - (tamanioPagina - desplazamiento));
			pthread_mutex_unlock(memoria);
			
			paginaDeSiguienteHeap->ultimoUso = clock();
			paginaDeSiguienteHeap->uso = true;

			memcpy(heap, buff_heap, TAMANIO_HEAP);
			free(buff_heap);

		}else{

		*despl = desplazamiento;

		pthread_mutex_lock(memoria);
		memcpy(heap, memoriaPrincipal + *DF + desplazamiento, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		paginaDeSiguienteHeap->ultimoUso = clock();
		paginaDeSiguienteHeap->uso = true;

		}

	} while (!(heap->isFree));

	return pagina;
}


t_list* buscarMarcosLibres(t_carpincho* carpincho){

		if(strcmp(tipoAsignacion, "FIJA") == 0){
			 
			bool buscarMarcosDelProceso(t_marco* marco){
				return marco->proceso_asignado == carpincho->id_carpincho;
			};

			t_list *marcos_del_proceso = list_filter(marcos, (void*)buscarMarcosDelProceso);

			bool marcosLibres(t_marco* marco){
				return marco->estaLibre;
			};
			
			t_list* marcos_a_asignar = list_filter(marcos_del_proceso, (void*)marcosLibres);

			return marcos_a_asignar;   
			
		}
		if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
		bool noEstanAsignados(t_marco* marco){
			return marco->proceso_asignado == -1;
        };

	 	t_list *marcos_sin_asignar = list_filter(marcos, (void*)noEstanAsignados);
			return marcos_sin_asignar;
		
		}else{
			return NULL;
		}
}


uint32_t crearAllocNuevo(int *pagina, int tamanio, heapMetadata* heap, int posicionUltimoHeap, t_carpincho *carpincho, int32_t *desplazamiento){

	uint32_t paginasNecesarias = ceil((float)(TAMANIO_HEAP*2 + tamanio + posicionUltimoHeap)/tamanioPagina);
	uint32_t cantidadDePaginasACrear = paginasNecesarias - list_size(carpincho->tabla_de_paginas);

	/* int conexion = consultar_espacio(carpincho->id_carpincho, cantidadDePaginasACrear);

	uint32_t respuesta = (uint32_t)atender_respuestas_swap(conexion);

	if(respuesta == 0) return 0;*/
	
	//primero pregunta si hay lugar en swap

	bool buscarPag(t_pagina* pag){
    return pag->id_pagina == *pagina;
    };

    t_pagina* pag = list_find(carpincho->tabla_de_paginas, (void*)buscarPag);
	
	
	int DF = buscar_TLB(pag->id_pagina);

	if(DF == -1){ //tlb miss
        DF = buscarEnTablaDePaginas(carpincho, pag->id_pagina);
        if(DF == -1) DF = swapear(carpincho, pag);
	    carpincho->tlb_miss++;
		miss_totales++;
    }else{//hit
        carpincho->tlb_hit++;
		hits_totales++;
    }

	heap->isFree = false;
	heap->nextAlloc = posicionUltimoHeap + TAMANIO_HEAP + tamanio;

	if(tamanioPagina - *desplazamiento < TAMANIO_HEAP){ //esta cortado. Actualiza el ultimo heap

		void* buffer_heap = malloc(TAMANIO_HEAP);
		memcpy(buffer_heap, heap, TAMANIO_HEAP);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + *desplazamiento, buffer_heap, tamanioPagina - *desplazamiento);
		pthread_mutex_unlock(memoria);


		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;

		bool buscarSigPag(t_pagina* pag){
			return pag->id_pagina > *pagina;
		};
		
		t_pagina* paginaDeSiguienteHeap = list_find(carpincho->tabla_de_paginas, (void*)buscarSigPag);
		*pagina = paginaDeSiguienteHeap->id_pagina;

		DF = buscar_TLB(paginaDeSiguienteHeap->id_pagina);
		if(DF == -1){ //tlb miss
            DF = buscarEnTablaDePaginas(carpincho, paginaDeSiguienteHeap->id_pagina);
            if(DF == -1) DF = swapear(carpincho, paginaDeSiguienteHeap);
		    carpincho->tlb_miss++;
			miss_totales++;
        }else{//hit
            carpincho->tlb_hit++;
			hits_totales++;
        }

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF , buffer_heap + (tamanioPagina - *desplazamiento) , TAMANIO_HEAP - (tamanioPagina - *desplazamiento));
		pthread_mutex_unlock(memoria);
		
		paginaDeSiguienteHeap->modificado = true;
		paginaDeSiguienteHeap->ultimoUso = clock();
		paginaDeSiguienteHeap->uso = true;

		*desplazamiento = - (tamanioPagina - *desplazamiento); //esto esta re trambolico porque despues le suma 9

		free(buffer_heap);

	}else{

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + *desplazamiento, heap, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;

	}

	//Hasta aca se actualiza el ultimo heap.
	//Se crea el nuevo que seria el ultimo.

	heapMetadata *nuevoHeap = malloc(TAMANIO_HEAP);

	nuevoHeap->isFree = true;
	nuevoHeap->prevAlloc = posicionUltimoHeap;
	nuevoHeap->nextAlloc = -1;

	
	for(int i=0; i<cantidadDePaginasACrear; i++){

		t_pagina* pagina = malloc(sizeof(t_pagina));
		pagina->id_pagina = generadorIdsPaginas();
		pagina->esNueva = true;
		pagina->uso = true;
		pagina->ultimoUso = clock();
        pagina->modificado = true;

		enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, "");

		list_add(carpincho->tabla_de_paginas, pagina);


	}

	if(cantidadDePaginasACrear == 0){ //hay que crear el alloc en la misma pag. TODO verificar que este presente?

			pthread_mutex_lock(memoria);
			memcpy(memoriaPrincipal + DF + (*desplazamiento + TAMANIO_HEAP) + tamanio, nuevoHeap, TAMANIO_HEAP);
			pthread_mutex_unlock(memoria);
			
			//poner modificado a la pag correspondiente. una variable que diga si entro al caso cortado o no
		return;
	}

	t_list* marcos_a_asignar = buscarMarcosLibres(carpincho);

	if(list_size(marcos_a_asignar)<cantidadDePaginasACrear){//faltan marcos. hay que liberar los que faltan (mandar a swap)

		int marcosFaltantes = cantidadDePaginasACrear - list_size(marcos_a_asignar);

		for(int i=0; i< marcosFaltantes; i++){
		reemplazarPagina(carpincho); //marcosFaltantes veces
		}//hace espacio para poner las paginas nuevas
		marcos_a_asignar = buscarMarcosLibres(carpincho);
	}

	if(cantidadDePaginasACrear == 1 && (*desplazamiento + TAMANIO_HEAP + tamanio) < tamanioPagina){ //actualiza el primer pedacito del heap cortado al final de la misma pagina

		DF = buscar_TLB(pag->id_pagina);

		if(DF == -1){ //tlb miss
            DF = buscarEnTablaDePaginas(carpincho, pag->id_pagina);
            if(DF == -1) DF = swapear(carpincho, pag);
		    carpincho->tlb_miss++;
			miss_totales++;
        }else{//hit
            carpincho->tlb_hit++;
			hits_totales++;
        }

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + (*desplazamiento + TAMANIO_HEAP) + tamanio, nuevoHeap, tamanioPagina - (*desplazamiento + TAMANIO_HEAP + tamanio));
		pthread_mutex_unlock(memoria);
		
		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;

	}

	void* buffer_allocs = generar_buffer_allocs(tamanio, nuevoHeap, cantidadDePaginasACrear, AGREGAR_ALLOC, *desplazamiento);

	bool paginas_nuevas(t_pagina* pag){
		return pag->esNueva;
	};
	t_list* paginasNuevas = list_filter(carpincho->tabla_de_paginas, (void*)paginas_nuevas);


	void ponerlasPresentes(t_pagina* pag){
		pag->presente = true;
	};
	list_iterate(paginasNuevas, (void*)ponerlasPresentes);

	escribirMemoria(buffer_allocs, paginasNuevas, marcos_a_asignar);

	free(buffer_allocs);

	void agregarATLB(t_pagina* pag){
		algoritmo_reemplazo_TLB(pag);
	};
	list_iterate(paginasNuevas, (void*)agregarATLB);


}

t_marco* reemplazarPagina(t_carpincho* carpincho){

	if(strcmp(tipoAsignacion, "FIJA") == 0){

		bool paginasPresentes(t_pagina* pag){
			return pag->presente;
		};
		
		t_list* paginas_a_reemplazar = list_filter(carpincho->tabla_de_paginas, (void*)paginasPresentes);

		t_pagina* victima = algoritmo_reemplazo_MMU(paginas_a_reemplazar, carpincho);   

		void* contenido = malloc(tamanioPagina);
		pthread_mutex_lock(memoria);
		memcpy(contenido, memoriaPrincipal + victima->marco->comienzo, tamanioPagina);
		pthread_mutex_unlock(memoria);

		if(victima->modificado)
		enviar_pagina(carpincho->id_carpincho, victima->id_pagina, contenido);
		

		victima->presente = false;
		victima->marco->estaLibre = true;
		
		bool quitarDeTLB(t_pagina* pag){
			return victima->id_pagina == pag->id_pagina;
		};
		pthread_mutex_lock(TLB_mutex);
		list_remove_by_condition(TLB, (void*)quitarDeTLB);// se quita directamente la pagina que se mando a swap.
		pthread_mutex_unlock(TLB_mutex);
		free(contenido);

		return victima->marco;	
	}

	if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
		bool noEstanAsignados(t_marco* marco){
			return marco->proceso_asignado == -1;
        };

	 	t_list *marcos_sin_asignar = list_filter(marcos, (void*)noEstanAsignados);
			return marcos_sin_asignar;
		
	}else{
		return NULL;
	}


}

t_pagina* algoritmo_reemplazo_MMU(t_list* paginas_a_reemplazar, t_carpincho* carpincho){
	
	if(strcmp(algoritmoReemplazoMMU, "LRU") == 0){
		
		bool comparator(t_pagina* p1, t_pagina* p2){
			return p1->ultimoUso < p2->ultimoUso;
		};
		
		t_list* paginasOrdenadas = list_sorted(paginas_a_reemplazar, (void*)comparator);

		return list_get(paginasOrdenadas, 0);

	}

	if(strcmp(algoritmoReemplazoMMU, "CLOCK") == 0){

		
		bool comparator(t_pagina* p1, t_pagina* p2){
			return p1->marco->id_marco < p2->marco->id_marco;
		};
		
		t_list* paginasOrdenadas = list_sorted(paginas_a_reemplazar, (void*)comparator);

		int puntero = carpincho->punteroClock;

		segundoIntento:

		for(int a = 0; a < list_size(paginasOrdenadas);a++){ //primera vuelta

			t_pagina* candidata = list_get(paginasOrdenadas, puntero);
			if(candidata->uso == 0 && candidata->modificado == 0){
				carpincho->punteroClock++;
				if(carpincho->punteroClock >= list_size(paginasOrdenadas)){
					carpincho->punteroClock = 0;
				}
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
				return candidata;
			}else{
				puntero++;
				candidata->uso = false;
				if(puntero >= list_size(paginasOrdenadas)){
					puntero = 0;
				}
			}

		}

		goto segundoIntento; //si llegó hasta aca es porque hizo las dos vueltas y tiene que empezar de nuevo


	}

}

void algoritmo_reemplazo_TLB(t_pagina* pagina){

	if(list_size(TLB) == cantidadEntradasTLB){

		if(strcmp(algoritmoReemplazoMMU, "LRU") == 0){
			
			bool comparator(t_pagina* p1, t_pagina* p2){
				return p1->ultimoUso < p2->ultimoUso;
			};
			pthread_mutex_lock(TLB_mutex);
			t_list* paginasOrdenadas = list_sorted(TLB, (void*)comparator);
			pthread_mutex_unlock(TLB_mutex);

			list_remove(paginasOrdenadas, 0);

			pthread_mutex_lock(TLB_mutex);
			list_add(TLB, pagina);
			pthread_mutex_unlock(TLB_mutex);

		}

		if(strcmp(algoritmoReemplazoMMU, "FIFO") == 0){
			pthread_mutex_lock(TLB_mutex);
			list_remove(TLB, 0);

			list_add(TLB, pagina);
			pthread_mutex_unlock(TLB_mutex);
		}

	}else{
		pthread_mutex_lock(TLB_mutex);
		list_add(TLB, pagina);
		pthread_mutex_unlock(TLB_mutex);
	}
}

uint32_t swapear(t_carpincho* carpincho, t_pagina* paginaPedida){
	
	t_marco* marcoLiberado = reemplazarPagina(carpincho);
	uint32_t conexion = pedir_pagina(paginaPedida->id_pagina, carpincho->id_carpincho);
	void* contenido = atender_respuestas_swap(conexion);
	paginaPedida->marco = marcoLiberado;
	paginaPedida->marco->estaLibre = false;
	paginaPedida->presente = true;
	paginaPedida->ultimoUso = clock();
	paginaPedida->uso = true;
	paginaPedida->modificado = false;

	heapMetadata* heap = malloc(TAMANIO_HEAP);
	memcpy(heap, contenido, TAMANIO_HEAP);

	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + paginaPedida->marco->comienzo, contenido, tamanioPagina);
	pthread_mutex_unlock(memoria);


	algoritmo_reemplazo_TLB(paginaPedida);

	return marcoLiberado->comienzo;
}

void manejador_de_seniales(int numeroSenial){
	
	if(numeroSenial == SIGINT){
		//imprimir todo lo de la tlb hits and misses ahre
		exit(EXIT_SUCCESS);
	}

	if(numeroSenial == SIGUSR1){
		printf("\nMATI KPO");
	}

	if(numeroSenial == SIGUSR2){
		//vaciar tlb
	}

}

int32_t buscarEnTablaDePaginas(t_carpincho* carpincho, int32_t idPag){

	bool buscarPaginaPresente(t_pagina* pag){
		return pag->id_pagina == idPag && pag->presente;
	};

	t_pagina* pagina = list_find(carpincho->tabla_de_paginas, (void*)buscarPaginaPresente);

	if(pagina == NULL){
		return -1;
	}

	return pagina->marco->comienzo;

}