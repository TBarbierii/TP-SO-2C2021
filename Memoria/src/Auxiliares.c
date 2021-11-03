#include "Memoria.h"

uint32_t generadorIdsPaginas(){

    return id_pag++;

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

		t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);

		bool buscarPagina(t_pagina* s){
			return s->id_pagina == id;
		};

		t_pagina* pagina = list_find(capybara->tabla_de_paginas,(void*)buscarPagina);

		direccionFisica = pagina->marco->comienzo + desplazamiento; //ver si es . o ->

	return direccionFisica;
}

uint32_t generadorIdsCarpinchos(){

    return id_carpincho++;

}

uint32_t generadorIdsMarcos(){

	return id_marco++;
	
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

	memcpy(memoriaPrincipal + marco->comienzo, buffer + (contador*tamanioPagina), tamanioPagina);

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

	t_pagina* pagina = list_find(TLB, (void*)buscarPagina);
	
	return pagina->marco->comienzo;
	
}



int buscarSiguienteHeapLibre(heapMetadata* heap, int32_t *DF, t_list* paginas, int32_t *posicionHeap, int32_t *despl){

	int pagina;
	t_pagina *paginaDeSiguienteHeap;
	int i;

	do 
	{
		int posicionSiguienteHeap = heap->nextAlloc;
		*posicionHeap = posicionSiguienteHeap;

        for(i=1; i <= list_size(paginas); i++){

            if(posicionSiguienteHeap < tamanioPagina * i){
            paginaDeSiguienteHeap = list_get(paginas, i-1);
			pagina = paginaDeSiguienteHeap->id_pagina;
            break; 
            }    
        }

        uint32_t desplazamiento =  tamanioPagina - (i * tamanioPagina - posicionSiguienteHeap); //el desplazamiento relativo a la pagina

		*DF = buscar_TLB(paginaDeSiguienteHeap->id_pagina);
		if(DF == -1){ //tlb miss
		//buscar en tabla de paginas
		//swapear
		}
		
		if(tamanioPagina - desplazamiento < TAMANIO_HEAP){ //esta cortado

			*despl = desplazamiento;
			void* buff_heap = malloc(TAMANIO_HEAP);
			memcpy(buff_heap, memoriaPrincipal + (*DF + desplazamiento), (tamanioPagina - desplazamiento));

			paginaDeSiguienteHeap = list_get(paginas, i);

			*DF = buscar_TLB(paginaDeSiguienteHeap->id_pagina);
			if(DF == -1){ //tlb miss
			//buscar en tabla de paginas
			//swapear
			}

			memcpy(buff_heap + (tamanioPagina - desplazamiento), memoriaPrincipal + *DF , TAMANIO_HEAP - (tamanioPagina - desplazamiento));

			memcpy(heap, buff_heap, TAMANIO_HEAP);
			free(buff_heap);

		}else{

		*despl = desplazamiento;
		memcpy(heap, memoriaPrincipal + *DF + desplazamiento, TAMANIO_HEAP);
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


void crearAllocNuevo(int *pagina, int tamanio, heapMetadata* heap, int posicionUltimoHeap, t_carpincho *carpincho, int32_t *desplazamiento){

	int DF = buscar_TLB(*pagina);

	if(DF == -1){ //tlb miss
		//buscar en tabla de paginas
		//swapearidsPaginas[0]
		//actualiidsPaginas[0]
	}

	heap->isFree = false;
	heap->nextAlloc = posicionUltimoHeap + TAMANIO_HEAP + tamanio;

	if(tamanioPagina - *desplazamiento < TAMANIO_HEAP){ //esta cortado. Actualiza el ultimo heap

		void* buffer_heap = malloc(TAMANIO_HEAP);
		memcpy(buffer_heap, heap, TAMANIO_HEAP);

		memcpy(memoriaPrincipal + DF + *desplazamiento, buffer_heap, tamanioPagina - *desplazamiento);

		bool buscarSigPag(t_pagina* pag){
			return pag->id_pagina > *pagina;
		};
		
		t_pagina* paginaDeSiguienteHeap = list_find(carpincho->tabla_de_paginas, (void*)buscarSigPag);
		*pagina = paginaDeSiguienteHeap->id_pagina;

		DF = buscar_TLB(paginaDeSiguienteHeap->id_pagina);
		if(DF == -1){ //tlb miss
		//buscar en tabla de paginas
		//swapear
		}

		memcpy(memoriaPrincipal + DF , buffer_heap + (tamanioPagina - *desplazamiento) , TAMANIO_HEAP - (tamanioPagina - *desplazamiento));
		*desplazamiento = - (tamanioPagina - *desplazamiento); //esto esta re trambolico porque despues le suma 9

		free(buffer_heap);

	}else{

		memcpy(memoriaPrincipal + DF + *desplazamiento, heap, TAMANIO_HEAP);

	}

	heapMetadata *nuevoHeap = malloc(TAMANIO_HEAP);

	nuevoHeap->isFree = true;
	nuevoHeap->prevAlloc = posicionUltimoHeap;
	nuevoHeap->nextAlloc = -1;

	uint32_t paginasNecesarias = ceil((float)(TAMANIO_HEAP*2 + tamanio + posicionUltimoHeap)/tamanioPagina);
	uint32_t cantidadDePaginasACrear = paginasNecesarias - list_size(carpincho->tabla_de_paginas);
	
	for(int i=0; i<cantidadDePaginasACrear; i++){

		t_pagina* pagina = malloc(sizeof(t_pagina));
		pagina->id_pagina = generadorIdsPaginas();
		pagina->presente = true;
		pagina->esNueva = true;

		pagina->ultimoUso = clock();
        pagina->modificado = true;

		//enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, "");
		//for cada pagina creada. enviarlas a swap. Y si no hay espacio devolver null y denegar el memalloc
		list_add(carpincho->tabla_de_paginas, pagina);


	}

	t_list* marcos_a_asignar = buscarMarcosLibres(carpincho);

	if(list_size(marcos_a_asignar)<cantidadDePaginasACrear){//faltan marcos. hay que liberar los que faltan (mandar a swap)

		int marcosFaltantes = cantidadDePaginasACrear - list_size(marcos_a_asignar);

		for(int i=0; i< marcosFaltantes; i++){
		reemplazarPagina(carpincho); //marcosFaltantes veces
		}//hace espacio para poner las paginas nuevas
		marcos_a_asignar = buscarMarcosLibres(carpincho);
	}

	void* buffer_allocs = generar_buffer_allocs(tamanio, nuevoHeap, cantidadDePaginasACrear, AGREGAR_ALLOC, *desplazamiento);

	bool paginas_nuevas(t_pagina* pag){
		return pag->esNueva;
	};
	
	t_list* paginasNuevas = list_filter(carpincho->tabla_de_paginas, (void*)paginas_nuevas);


	escribirMemoria(buffer_allocs, paginasNuevas, marcos_a_asignar);
	free(buffer_allocs);

	void agregarATLB(t_pagina* pag){
	list_add(TLB, pag);
	};
    
	list_iterate(paginasNuevas, (void*)agregarATLB);


}

void reemplazarPagina(t_carpincho* carpincho){

	if(strcmp(tipoAsignacion, "FIJA") == 0){

		bool paginasPresentes(t_pagina* pag){
			return pag->presente;
		};
		
		t_list* paginas_a_reemplazar = list_filter(carpincho->tabla_de_paginas, (void*)paginasPresentes);

		t_pagina* victima = algoritmo_reemplazo_MMU(paginas_a_reemplazar);   

		void* contenido = malloc(tamanioPagina);
		memcpy(contenido, memoriaPrincipal + victima->marco->comienzo, tamanioPagina);

		enviar_pagina(carpincho->id_carpincho, victima->id_pagina, contenido);

		victima->presente = false;
		victima->marco->estaLibre = true;
		//actualizar tlb

		free(contenido);
			
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

t_pagina* algoritmo_reemplazo_MMU(t_list* paginas_a_reemplazar){
	
	if(strcmp(algoritmoReemplazoMMU, "LRU") == 0){
		
		bool comparator(t_pagina* p1, t_pagina* p2){
			return p1->ultimoUso > p2->ultimoUso;
		};
		
		t_list* paginasOrdenadas = list_sorted(paginas_a_reemplazar, (void*)comparator);

		return list_get(paginasOrdenadas, 0);

	}

	if(strcmp(algoritmoReemplazoMMU, "CLOCK") == 0){

	}

}