#include "Memoria.h"

uint32_t generadorIdsPaginas(t_carpincho* carp){

	return carp->contadorPag++;
	
}

uint32_t dar_vuelta_id(uint32_t num){

	uint32_t id_retornado;
	char* id = string_itoa(num);
	char* dadoVuelta = string_reverse(id);
	uint32_t veces = 3 - string_length(dadoVuelta);


	char* ceros = string_repeat('0', veces);

	char* a = string_new();
	string_append(&a, dadoVuelta);
	string_append(&a, ceros);
	
	id_retornado = atoi(a);

	free(id);
	free(a);
	free(ceros);
	free(dadoVuelta);
	return id_retornado;

}



uint32_t generarDireccionLogica(uint32_t id, uint32_t desplazamiento){


	char* primerNumero = "1";

	char* id_char = string_itoa(id);
	uint32_t veces = 3 - string_length(id_char);


	char* ceros = string_repeat('0', veces);

	char* a = string_new();
	string_append(&a, primerNumero);
	string_append(&a, ceros);
	string_append(&a, id_char);


	char* DESPLAZAMIENTO =  string_itoa(desplazamiento);

	char* b = string_new();
		string_append(&b,a );
		string_append(&b, DESPLAZAMIENTO);

	uint32_t direccionLogica = atoi(b);

	free(a);
	free(DESPLAZAMIENTO);
	free(ceros);
	free(b);
	free(id_char);

	return direccionLogica;
}

uint32_t obtenerId(uint32_t num){


	//char* id;// = string_new();

	uint32_t id_retornado;

	char* DL = string_itoa(num);
	char* substring  = string_substring(DL, 1, 3);

	id_retornado = atoi(substring);

	free(DL);
	free(substring);


	return id_retornado;

}

uint32_t obtenerDesplazamiento(uint32_t num){

		
		uint32_t id_retornado;

		char* DL = string_itoa(num);
		char* substring =  string_substring_from(DL, 4);

		if(strlen(DL)==0){
			return 0;
		}

		 id_retornado = atoi(substring);

		 free(DL);
		 free(substring);

		 return id_retornado;

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

		pthread_mutex_lock(marcos_sem);
	 	t_list * marcos_sin_asignar = list_filter(marcos, (void*)noEstanAsignados);
		pthread_mutex_unlock(marcos_sem);

		if(strcmp(tipoAsignacion, "FIJA") == 0){

			bool contarMarcos(t_marco* marco){
        	return marco->proceso_asignado == pid;
    		};
  
    		pthread_mutex_lock(marcos_sem);
    		t_list* marcos_a_asignar = list_filter(marcos, (void*)contarMarcos);
    		pthread_mutex_unlock(marcos_sem); 

			if(list_size(marcos_a_asignar) == marcosMaximos){
				return marcos_a_asignar; //Si no es la primera vez, manda los que ya tiene asignados. 
			}
			 
			marcos_a_asignar = list_take(marcos_sin_asignar, marcosMaximos);
			
			void marcarOcupados(t_marco *marco){
			marco->proceso_asignado = pid;
       		};

       		list_iterate(marcos_a_asignar, (void*)marcarOcupados);

			list_destroy(marcos_sin_asignar);

			return marcos_a_asignar;   
			
		}
		else if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
			return marcos_sin_asignar;
		}else{
			
			list_destroy(marcos_sin_asignar);
			return NULL;
		}
}

void escribirMemoria(void* buffer, t_list* paginas, t_list* marcos_a_asignar, t_carpincho* carpincho ){
	    
	int contador = 0; 

	void escribir_paginas_en_marcos(t_pagina* pag){

		t_marco* marco = list_get(marcos_a_asignar, contador);

		if(marco->estaLibre){
		
			pthread_mutex_lock(memoria);
			memcpy(memoriaPrincipal + marco->comienzo, buffer + (contador*tamanioPagina), tamanioPagina);
			pthread_mutex_unlock(memoria);

			pag->marco = marco;
			pag->presente = true;
			pag->esNueva = false;
			marco->estaLibre = false;
			marco->proceso_asignado=carpincho->id_carpincho;
			
		}
		contador++;
	};

	list_iterate(paginas, (void*)escribir_paginas_en_marcos);

	list_destroy(marcos_a_asignar);

}

int32_t buscar_TLB(t_pagina* pagina){

	pthread_mutex_lock(TLB_mutex);

	bool buscarPagina(t_pagina *pag){
		return pag->id_pagina == pagina->id_pagina && pag->id_carpincho == pagina->id_carpincho;
	};

	t_pagina* paginaEncontrada = list_find(TLB, (void*)buscarPagina);
	pthread_mutex_unlock(TLB_mutex);

	if(paginaEncontrada == NULL){
		return -1;
	}

		return paginaEncontrada->marco->comienzo;
	
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

		*DF = buscar_TLB(paginaDeSiguienteHeap);

		reemplazo(DF, carpincho, paginaDeSiguienteHeap);
		
		if(tamanioPagina - desplazamiento < TAMANIO_HEAP){ //esta cortado
			
			*despl = desplazamiento;
			void* buff_heap = malloc(TAMANIO_HEAP);
			pthread_mutex_lock(memoria);
			memcpy(buff_heap, memoriaPrincipal + (*DF + desplazamiento), (tamanioPagina - desplazamiento));
			pthread_mutex_unlock(memoria);
			
			paginaDeSiguienteHeap->ultimoUso = clock();
			paginaDeSiguienteHeap->uso = true;

			paginaDeSiguienteHeap = list_get(carpincho->tabla_de_paginas, i);

			*DF = buscar_TLB(paginaDeSiguienteHeap);

			reemplazo(DF, carpincho, paginaDeSiguienteHeap);

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
	
	pthread_mutex_lock(listaCarpinchos);
	int conexion = consultar_espacio(carpincho->id_carpincho, cantidadDePaginasACrear);

	uint32_t respuesta = (uint32_t)atender_respuestas_swap(conexion);
	
	printf("\nRespuesta consulta: %i\n", respuesta);
	pthread_mutex_unlock(listaCarpinchos);
	if(respuesta == 0){ 
		return 0;
	}
	
	//primero pregunta si hay lugar en swap

	bool buscarPag(t_pagina* pag){
    return pag->id_pagina == *pagina;
    };

    t_pagina* pag = list_find(carpincho->tabla_de_paginas, (void*)buscarPag);
	
	
	int DF = buscar_TLB(pag);

	/* if(DF == -1){ //tlb miss
        DF = buscarEnTablaDePaginas(carpincho, pag->id_pagina);
        if(DF == -1) DF = swapear(carpincho, pag);
	    carpincho->tlb_miss++;
		miss_totales++;
    }else{//hit
        carpincho->tlb_hit++;
		hits_totales++;
	}*/

	reemplazo(&DF, carpincho, pag);

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

		reemplazo(&DF, carpincho, paginaDeSiguienteHeap);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF , buffer_heap + (tamanioPagina - *desplazamiento) , TAMANIO_HEAP - (tamanioPagina - *desplazamiento));
		pthread_mutex_unlock(memoria);
		
		paginaDeSiguienteHeap->modificado = true;
		paginaDeSiguienteHeap->ultimoUso = clock();
		paginaDeSiguienteHeap->uso = true;

		*desplazamiento = - (tamanioPagina - *desplazamiento); //esto esta re trambolico porque despues le suma 9

		free(buffer_heap);
		free(heap);

	}else{

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + *desplazamiento, heap, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;
		free(heap);

	}

	//Hasta aca se actualiza el ultimo heap.
	//Se crea el nuevo que seria el ultimo.

	heapMetadata *nuevoHeap = malloc(TAMANIO_HEAP);

	nuevoHeap->isFree = true;
	nuevoHeap->prevAlloc = posicionUltimoHeap;
	nuevoHeap->nextAlloc = -1;

	pthread_mutex_lock(swap);
	for(int i=0; i<cantidadDePaginasACrear; i++){

		t_pagina* pagina = malloc(sizeof(t_pagina));
		pagina->id_pagina = generadorIdsPaginas(carpincho);
		pagina->esNueva = true;
		pagina->presente = false;
		pagina->uso = true;
		pagina->ultimoUso = clock();
        pagina->modificado = true;
		pagina->id_carpincho = carpincho->id_carpincho;

		enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, "");

		list_add(carpincho->tabla_de_paginas, pagina);

	}
	pthread_mutex_unlock(swap);

	if(cantidadDePaginasACrear == 0){ //hay que crear el alloc en la misma pag. TODO verificar que este presente?

			pthread_mutex_lock(memoria);
			memcpy(memoriaPrincipal + DF + (*desplazamiento + TAMANIO_HEAP) + tamanio, nuevoHeap, TAMANIO_HEAP);
			pthread_mutex_unlock(memoria);

			free(nuevoHeap);
			
			//poner modificado a la pag correspondiente. una variable que diga si entro al caso cortado o no
		return 1;
	}

	t_list* marcos_a_asignar = buscarMarcosLibres(carpincho);

	if(list_size(marcos_a_asignar)<cantidadDePaginasACrear){//faltan marcos. hay que liberar los que faltan (mandar a swap)

		int marcosFaltantes = cantidadDePaginasACrear - list_size(marcos_a_asignar);

		for(int i=0; i< marcosFaltantes; i++){
		reemplazarPagina(carpincho); //marcosFaltantes veces
		}//hace espacio para poner las paginas nuevas
		list_destroy(marcos_a_asignar);
		marcos_a_asignar = buscarMarcosLibres(carpincho);
	}

	if(cantidadDePaginasACrear == 1 && (*desplazamiento + TAMANIO_HEAP + tamanio) < tamanioPagina){ //actualiza el primer pedacito del heap cortado al final de la misma pagina

		DF = buscar_TLB(pag);

		/*if(DF == -1){ //tlb miss
            DF = buscarEnTablaDePaginas(carpincho, pag->id_pagina);
            if(DF == -1) DF = swapear(carpincho, pag);
		    carpincho->tlb_miss++;
			miss_totales++;
        }else{//hit
            carpincho->tlb_hit++;
			hits_totales++;
		}*/

		reemplazo(&DF, carpincho, pag);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + (*desplazamiento + TAMANIO_HEAP) + tamanio, nuevoHeap, tamanioPagina - (*desplazamiento + TAMANIO_HEAP + tamanio));
		pthread_mutex_unlock(memoria);
		
		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;

		free(nuevoHeap);

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

	escribirMemoria(buffer_allocs, paginasNuevas, marcos_a_asignar, carpincho);



	free(buffer_allocs);
	free(nuevoHeap);
	

	void agregarATLB(t_pagina* pag){
		algoritmo_reemplazo_TLB(pag);
	};
	list_iterate(paginasNuevas, (void*)agregarATLB);

	return 1;
}

t_marco* reemplazarPagina(t_carpincho* carpincho){

	if(strcmp(tipoAsignacion, "FIJA") == 0){

		bool paginasPresentes(t_pagina* pag){
			return pag->presente;
		};
		
		t_list* paginas_a_reemplazar = list_filter(carpincho->tabla_de_paginas, (void*)paginasPresentes);

		t_pagina* victima = algoritmo_reemplazo_MMU(paginas_a_reemplazar, carpincho);   

		log_info(logsObligatorios, "Pagina víctima: Pid: %i, Página: %i, Marco: %i", carpincho->id_carpincho, victima->id_pagina, victima->marco->id_marco);

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
		pthread_mutex_unlock(TLB_mutex);
		free(contenido);

		return victima->marco;	
	}

	if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
		bool paginasPresentes(t_pagina* pag){
			return pag->presente;
		};

		t_list* paginas_a_reemplazar = list_create();

		pthread_mutex_lock(listaCarpinchos);
		for (int i=0; i<list_size(carpinchos); i++){

			t_carpincho* carp = list_get(carpinchos, i);
			t_list* paginas = list_filter(carp->tabla_de_paginas, (void*)paginasPresentes);
			list_add_all(paginas_a_reemplazar, paginas);
			list_destroy(paginas);
		}
		pthread_mutex_unlock(listaCarpinchos);
	
		
		t_pagina* victima = algoritmo_reemplazo_MMU(paginas_a_reemplazar, carpincho); 

		log_info(logsObligatorios, "Pagina víctima: Pid: %i, Página: %i, Marco: %i", victima->id_carpincho, victima->id_pagina, victima->marco->id_marco);

		list_destroy(paginas_a_reemplazar);  												

		void* contenido = malloc(tamanioPagina);
		pthread_mutex_lock(memoria);
		memcpy(contenido, memoriaPrincipal + victima->marco->comienzo, tamanioPagina);
		pthread_mutex_unlock(memoria);

		if(victima->modificado)
		enviar_pagina(victima->id_carpincho, victima->id_pagina, contenido);
		

		victima->presente = false;
		victima->marco->estaLibre = true;
		victima->marco->proceso_asignado = -1;
		
		pthread_mutex_lock(TLB_mutex);
		bool quitarDeTLB(t_pagina* pag){
			return victima->id_pagina == pag->id_pagina;
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
			return p1->ultimoUso < p2->ultimoUso;
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
		
		t_list* paginasOrdenadas = list_sorted(paginas_a_reemplazar, (void*)comparator);

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
				free (paginasOrdenadas);
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
				free (paginasOrdenadas);
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

		free (paginasOrdenadas);
		}

		if(strcmp(tipoAsignacion, "DINAMICA") == 0){

		punteroClock;

		segundoIntentoDinamica:

		for(int a = 0; a < list_size(paginasOrdenadas);a++){ //primera vuelta

			t_pagina* candidata = list_get(paginasOrdenadas, punteroClock);
			if(candidata->uso == 0 && candidata->modificado == 0){
				punteroClock++;
				if(punteroClock >= list_size(paginasOrdenadas)){
					punteroClock = 0;
				}
				free (paginasOrdenadas);
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
				free (paginasOrdenadas);
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

		free (paginasOrdenadas);
		}

	}

}

void algoritmo_reemplazo_TLB(t_pagina* pagina){

	if(list_size(TLB) == cantidadEntradasTLB){

		if(strcmp(algoritmoReemplazoTLB, "LRU") == 0){
			
			pthread_mutex_lock(TLB_mutex);
			bool comparator(t_pagina* p1, t_pagina* p2){
				return p1->ultimoUso < p2->ultimoUso;
			};

			t_list* paginasOrdenadas = list_sorted(TLB, (void*)comparator);

			if(! list_is_empty(paginasOrdenadas)) {
				t_pagina* pag = list_get(paginasOrdenadas,0);
				pag->presente = false;

				void buscarPag(t_pagina* p){
					return p->id_pagina == pag->id_pagina;
				};

				list_remove_by_condition(TLB, (void*)buscarPag);
				
				log_info(logsObligatorios, "Entrada TLB. Victima: PID: %i	Página: %i	Marco: %i", pag->id_carpincho, pag->id_pagina, pag->marco->id_marco);

			}
			pthread_mutex_unlock(TLB_mutex);


			pthread_mutex_lock(TLB_mutex);
			list_add(TLB, pagina);
			pthread_mutex_unlock(TLB_mutex);

			log_info(logsObligatorios, "Entrada TLB. NuevaEntrada: PID: %i	Página: %i	Marco: %i", pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);

		}

		if(strcmp(algoritmoReemplazoTLB, "FIFO") == 0){
			
			pthread_mutex_lock(TLB_mutex);
			
			t_pagina* pag = list_get(TLB,0);
			pag->presente = false;
			log_info(logsObligatorios, "Entrada TLB. Victima: PID: %i	Página: %i	Marco: %i", pag->id_carpincho, pag->id_pagina, pag->marco->id_marco);
			
			list_remove(TLB, 0);

			list_add(TLB, pagina);
			pthread_mutex_unlock(TLB_mutex);

			log_info(logsObligatorios, "Entrada TLB. NuevaEntrada: PID: %i	Página: %i	Marco: %i", pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);

		}

	}else{
		pthread_mutex_lock(TLB_mutex);
		list_add(TLB, pagina);
		pthread_mutex_unlock(TLB_mutex);
		
		log_info(logsObligatorios, "Entrada TLB. NuevaEntrada: PID: %i	Página: %i	Marco: %i", pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);

	}
}

uint32_t swapear(t_carpincho* carpincho, t_pagina* paginaPedida){
	
	pthread_mutex_lock(swap);
	t_marco* marcoLiberado = reemplazarPagina(carpincho);
	uint32_t conexion = pedir_pagina(paginaPedida->id_pagina, carpincho->id_carpincho);
	void* contenido = atender_respuestas_swap(conexion);
	pthread_mutex_unlock(swap);
	paginaPedida->marco = marcoLiberado;
	paginaPedida->marco->estaLibre = false;
	paginaPedida->presente = true;
	paginaPedida->ultimoUso = clock();
	paginaPedida->uso = true;
	paginaPedida->modificado = false;

	log_info(logsObligatorios, "Pagina entrante: Pid: %i, Página: %i, Marco: %i", carpincho->id_carpincho, paginaPedida->id_pagina, paginaPedida->marco->id_marco);


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
		//imprimir todo lo de la tlb hits and misses
		exit(EXIT_SUCCESS);
	}

	if(numeroSenial == SIGUSR1){

		char* time = temporal_get_string_time("%d-%m-%y %H:%M:%S");

		strcat(pathDump,"/DUMP_");
		strcat(pathDump,time);
		strcat(pathDump,".dmp");


    	dumpTLB = log_create(pathDump,"Dump", 0, LOG_LEVEL_INFO);

		imprimir_dump(dumpTLB, time);
	}

	if(numeroSenial == SIGUSR2){
		list_clean(TLB);
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

void reemplazo(int32_t *DF, t_carpincho* carpincho, t_pagina* pagina){

	if(*DF == -1){ //tlb miss
		usleep(retardoFAlloTLB * 1000);
		*DF = buscarEnTablaDePaginas(carpincho, pagina->id_pagina);
		
		if(*DF == -1) *DF = swapear(carpincho, pagina);
			carpincho->tlb_miss++;
			miss_totales++;

	}else{//hit
		
		usleep(retardoAciertoTLB * 1000);
		carpincho->tlb_hit++;
		hits_totales++;
	}

}

void imprimir_dump(t_log* log_dump, char * time){
	
	log_info(log_dump,"--------------------------------------------------------------------------");log_info(log_dump,"--------------------------------------------------------------------------\n");
	log_info(log_dump,"Dump: %s \n",time);

	pthread_mutex_lock(TLB_mutex);

	for(int i=0; i<list_size(TLB);i++){

		t_pagina* pagina = list_get(TLB,i);

		if(pagina == NULL){
		log_info(log_dump,"Entrada: %i\t Estado: Libre\t Carpincho: -\t Pagina: - \t Marco: %i",i);

		}else{
		log_info(log_dump,"Entrada: %i\t Estado: Ocupado\t 	Carpincho: %i\t Pagina: %i \t Marco: %i",i,pagina->id_carpincho, pagina->id_pagina, pagina->marco->id_marco);
		}
	
	}

	pthread_mutex_unlock(TLB_mutex);
	
	log_info(log_dump,"--------------------------------------------------------------------------------\n");

}

void dividirAllocs(t_carpincho* carpincho, int32_t posicionHeap, int32_t pagina, uint32_t tamanio, int32_t desplazamiento ){

	heapMetadata* primerHeap = malloc(TAMANIO_HEAP);
	heapMetadata* ultimoHeap = malloc(TAMANIO_HEAP);
	heapMetadata* nuevoHeap = malloc(TAMANIO_HEAP);

	bool buscarPagina(t_pagina* p){
		return p->id_pagina == pagina;
	};
	t_pagina* pag = list_find(carpincho->tabla_de_paginas, (void*)buscarPagina);

	int DL = buscar_TLB(pag);

	reemplazo(&DL, carpincho, pag);

	pthread_mutex_lock(memoria);
	memcpy(primerHeap, memoriaPrincipal + pag->marco->comienzo  + desplazamiento, TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);

	nuevoHeap->isFree=true;
	nuevoHeap->nextAlloc = primerHeap->nextAlloc;

	int nextAlloc = primerHeap->nextAlloc;

	primerHeap->isFree=false;
	primerHeap->nextAlloc = posicionHeap + TAMANIO_HEAP + tamanio;



	pthread_mutex_lock(memoria);
	memcpy( memoriaPrincipal + pag->marco->comienzo  + desplazamiento, primerHeap, TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);

	t_pagina* paginaSiguiente;
	if(nextAlloc - posicionHeap > tamanioPagina - desplazamiento - TAMANIO_HEAP){

		paginaSiguiente = list_find(carpincho->tabla_de_paginas, (void*)buscarPagina);

		int DL = buscar_TLB(pag);

		reemplazo(&DL, carpincho, pag);

		pthread_mutex_lock(memoria);
		memcpy(ultimoHeap, memoriaPrincipal + paginaSiguiente->marco->comienzo + (nextAlloc - posicionHeap - TAMANIO_HEAP), TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		nuevoHeap->prevAlloc = ultimoHeap->prevAlloc;
	
	} else{
		pthread_mutex_lock(memoria);
		memcpy(ultimoHeap, memoriaPrincipal + pag->marco->comienzo + (nextAlloc - posicionHeap - TAMANIO_HEAP), TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		nuevoHeap->prevAlloc = ultimoHeap->prevAlloc;
	}



	ultimoHeap->prevAlloc = posicionHeap + TAMANIO_HEAP + tamanio;
	
	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + pag->marco->comienzo + (nextAlloc - posicionHeap - TAMANIO_HEAP), ultimoHeap,TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);


	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + pag->marco->comienzo + desplazamiento + TAMANIO_HEAP + tamanio, nuevoHeap, TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);

}


