#include "memalloc.h"

uint32_t administrar_allocs(uint32_t pid, uint32_t tamanio){ 

    bool buscarCarpincho(t_carpincho* c){
		return c->id_carpincho == pid;
	};
    
    pthread_mutex_lock(listaCarpinchos);
    t_carpincho* carpincho = list_find(carpinchos, (void*)buscarCarpincho);
    pthread_mutex_unlock(listaCarpinchos);


    t_list* marcos_a_asignar = reservarMarcos(carpincho->id_carpincho);
  
    uint32_t direccionLogica = administrar_paginas(carpincho, tamanio, marcos_a_asignar);
	
	log_info(loggerServidor, "El carpincho: %i ahora tiene %i paginas",carpincho->id_carpincho, list_size(carpincho->tabla_de_paginas));

    return direccionLogica;

}

uint32_t administrar_paginas(t_carpincho* carpincho, uint32_t tamanio, t_list* marcos_a_asignar){

    if(list_size(carpincho->tabla_de_paginas)==0){ //primera vez

        heapMetadata* next_alloc = malloc(sizeof(heapMetadata));
        next_alloc->isFree = true;
        next_alloc->prevAlloc = 0;
        next_alloc->nextAlloc = -1;

        uint32_t cantidadDePaginasACrear = ceil((float)(TAMANIO_HEAP*2 + tamanio)/tamanioPagina);

        pthread_mutex_lock(swap);
        int conexion = consultar_espacio(carpincho->id_carpincho, cantidadDePaginasACrear);

        uint32_t respuesta = (uint32_t)atender_respuestas_swap(conexion);
        pthread_mutex_unlock(swap);

        if(respuesta == 0) return 0;

        pthread_mutex_lock(swap);
        for(int i=0; i<cantidadDePaginasACrear; i++){

            t_pagina* pagina = malloc(sizeof(t_pagina));
            pagina->id_pagina = generadorIdsPaginas(carpincho);
            pagina->presente = false;
            pagina->ultimoUso = clock();
            pagina->uso = true;
            pagina->modificado = true;
            pagina->id_carpincho = carpincho->id_carpincho;

            void* paginaVacia = malloc(tamanioPagina);
            for(int j=0; j<tamanioPagina;j++){
                 char valor = '\0';
                memcpy(paginaVacia +j, &valor, 1);
            }
           
            enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, paginaVacia);
            
			free(paginaVacia);

            list_add(carpincho->tabla_de_paginas, pagina);
        }
        pthread_mutex_unlock(swap);

        void* buffer_allocs = generar_buffer_allocs(tamanio, next_alloc,cantidadDePaginasACrear, PRIMERA_VEZ, 0);

		pthread_mutex_lock(marcos_sem);
        escribirMemoria(buffer_allocs, carpincho->tabla_de_paginas, marcos_a_asignar, carpincho);
		pthread_mutex_unlock(marcos_sem);

        free (buffer_allocs);
        
		pthread_mutex_lock(tabla_paginas);
        list_iterate(carpincho->tabla_de_paginas, (void*)algoritmo_reemplazo_TLB);
        t_pagina* pag = list_get(carpincho->tabla_de_paginas, 0);
		pthread_mutex_unlock(tabla_paginas);

        return generarDireccionLogica(pag->id_pagina, TAMANIO_HEAP);

    }else{//busca un hueco
        
		pthread_mutex_lock(tabla_paginas);
        t_pagina* primeraPag = list_get(carpincho->tabla_de_paginas, 0);
		pthread_mutex_unlock(tabla_paginas);

        int32_t DF = buscar_TLB(primeraPag);

        reemplazo(&DF, carpincho, primeraPag);

        heapMetadata *heap = malloc(TAMANIO_HEAP);
        int32_t posicionHeap = 0;
        int32_t pagina, desplazamiento;

        pthread_mutex_lock(memoria);
        memcpy(heap, memoriaPrincipal + DF, TAMANIO_HEAP);
        pthread_mutex_unlock(memoria);

		pthread_mutex_lock(tabla_paginas);
        primeraPag->ultimoUso = clock();
        primeraPag->uso = true;
		pthread_mutex_unlock(tabla_paginas);

        while(heap->nextAlloc != -1)
        {
            if(!(heap->isFree)){
			pthread_mutex_lock(recorrer_marcos_mutex);
            pagina = buscarSiguienteHeapLibre(heap, &DF, carpincho, &posicionHeap, &desplazamiento);
			pthread_mutex_unlock(recorrer_marcos_mutex);
            }
            
            if(heap->nextAlloc == -1) break;

            uint32_t espacioEncontrado = heap->nextAlloc - posicionHeap - TAMANIO_HEAP;

            if 	   	(tamanio == espacioEncontrado){//entro justo
				break;
			}else if (tamanio + TAMANIO_HEAP < espacioEncontrado ){// +9 porque si es mas chico, en lo que sobra tiene que entrar el otro heap 
				break;
				dividirAllocs(carpincho, posicionHeap, pagina, tamanio, desplazamiento); 
            }else{
			
			pthread_mutex_lock(recorrer_marcos_mutex);
            pagina = buscarSiguienteHeapLibre(heap, &DF, carpincho, &posicionHeap, &desplazamiento);
			pthread_mutex_unlock(recorrer_marcos_mutex);
            }

        }

        if(heap->nextAlloc == -1){

            int32_t ok = crearAllocNuevo(&pagina, tamanio, heap, posicionHeap, carpincho, &desplazamiento);//pasar pagina y despl por referencia y si esta cortado modificarlo
            if(ok == 0) return 0;
        }

        if(tamanioPagina - desplazamiento < TAMANIO_HEAP && desplazamiento > 0){ //esta cortado

            desplazamiento = - (tamanioPagina - desplazamiento); 
            bool buscarSigPag(t_pagina* pag){
			return pag->id_pagina > pagina;
		    };
		
            t_pagina* paginaSig = list_find(carpincho->tabla_de_paginas, (void*)buscarSigPag);
            pagina = paginaSig->id_pagina;
        }
        uint32_t DL = generarDireccionLogica(pagina, desplazamiento + TAMANIO_HEAP);
		list_destroy(marcos_a_asignar);
		free(heap);
        return DL;
        

    }     
        

}


void* generar_buffer_allocs(uint32_t tamanio, heapMetadata* heap, uint32_t cantPaginas, stream_alloc codigo, int32_t despl){// el tamanio en el segundp caso, es el tamanio del comienzo. El heap tengo que ver

    uint32_t reserva = tamanioPagina * cantPaginas, desplazamiento = 0;
    void* stream_allocs = malloc(reserva);

    if(codigo == PRIMERA_VEZ){

        heapMetadata* alloc = malloc(sizeof(heapMetadata));
        alloc->prevAlloc = -1;
        alloc->isFree = false;
        alloc->nextAlloc = tamanio + TAMANIO_HEAP;

        memcpy(stream_allocs + desplazamiento, alloc, TAMANIO_HEAP);

        desplazamiento = alloc->nextAlloc;

        memcpy(stream_allocs + desplazamiento, heap, TAMANIO_HEAP);

        free(alloc);
        free(heap);

        return stream_allocs;
    }

    if(codigo == AGREGAR_ALLOC){


           if((despl + TAMANIO_HEAP) + tamanio < tamanioPagina){//se crea la nueva pagina con el pedacito del ultimo heap

            void* buffer_heap = malloc(TAMANIO_HEAP);
            memcpy(buffer_heap, heap, TAMANIO_HEAP);
            int offset = (despl + TAMANIO_HEAP + tamanio) - tamanioPagina;
			
            memcpy(stream_allocs, buffer_heap - offset, (offset + TAMANIO_HEAP));
			//memcpy(stream_allocs, buffer_heap - offset, (despl + TAMANIO_HEAP));

               
            free(buffer_heap);

            return stream_allocs;

           }else{ // nueva pagina iniciando con bytes reservados
        
            int bytesQueYaEstan = tamanioPagina - (despl + TAMANIO_HEAP); //bytes del tamaño solicitado que ya estan en la pagina anterior

            memcpy(stream_allocs + (tamanio - bytesQueYaEstan), heap , TAMANIO_HEAP);

            return stream_allocs;

           }
    }


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

			if(list_size(marcos_a_asignar) == marcosMaximos){
				pthread_mutex_unlock(marcos_sem); 
				return marcos_a_asignar; //Si no es la primera vez, manda los que ya tiene asignados. 
			}

			list_destroy(marcos_a_asignar);

			//TODO: ver que pasa si por ejemplo, le queremos dar 10 marcos al proceso, pero solo quedan 4? 
			t_list* marcosFinales = list_take(marcos_sin_asignar, marcosMaximos);
			
			void marcarOcupados(t_marco *marco){
			marco->proceso_asignado = pid;
       		};

       		list_iterate(marcosFinales, (void*)marcarOcupados);
			list_destroy(marcos_sin_asignar);

			pthread_mutex_unlock(marcos_sem); //puse el semaforo aca porque sino podria venir otro proceso y pisar los mismos marcos
			return marcosFinales;   
			
		}
		else if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
			return marcos_sin_asignar;
		}else{
			
			list_destroy(marcos_sin_asignar);
			return NULL;
		}
}

void escribirMemoria(void* buffer, t_list* paginas, t_list* marcos_a_asignar, t_carpincho* carpincho){
	    
	t_marco* marco;

	t_log* loggerMarcos = log_create("cfg/Marcos.log","EscrituraDeMarcos",1, LOG_LEVEL_INFO);

	bool marcoSinAsignar(t_marco* marco){
		
		 return marco->proceso_asignado == -1;
		
	}

	int contador = 0;

	void escribir_paginas_en_marcos(t_pagina* pag){

		if (contador >= list_size(marcos_a_asignar)) {
			pthread_mutex_lock(swap);
			marco = reemplazarPagina(carpincho);
			log_warning(loggerMarcos, "El carpincho %i va a reemplazar el marco %i para escribirlo", carpincho->id_carpincho, marco->id_marco);
			pthread_mutex_unlock(swap);

		} else {
			
			if(strcmp(tipoAsignacion, "DINAMICA") == 0){
				marco = list_find(marcos_a_asignar, (void*)marcoSinAsignar);
			}else if (strcmp(tipoAsignacion, "FIJA") == 0){
				marco = list_get(marcos_a_asignar, contador);
			}
		}

		if(marco->estaLibre){
		
			pthread_mutex_lock(memoria);
			memcpy(memoriaPrincipal + marco->comienzo, buffer + (contador*tamanioPagina), tamanioPagina);
			pthread_mutex_unlock(memoria);

			pthread_mutex_lock(tabla_paginas);
			pag->marco = marco;
			pag->presente = true;
			pag->esNueva = false;
			pthread_mutex_unlock(tabla_paginas);
			//aca como estamos tocando el marco que son variables globales, deberiamos poner entre los semaforos
			
			marco->estaLibre = false;
			marco->proceso_asignado=carpincho->id_carpincho;
			

			log_info(loggerMarcos, "Se escribe sobre el MARCO: %d, la PAGINA: %d, del carpincho :%d", marco->id_marco, pag->id_pagina, carpincho->id_carpincho);
			
		}
		contador++;
	};

	list_iterate(paginas, (void*)escribir_paginas_en_marcos);

	list_destroy(marcos_a_asignar);

	log_destroy(loggerMarcos);


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
			pthread_mutex_lock(tabla_paginas);	
            paginaDeSiguienteHeap = list_get(carpincho->tabla_de_paginas, i-1);
			pthread_mutex_unlock(tabla_paginas);
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
			
			pthread_mutex_lock(tabla_paginas);
			paginaDeSiguienteHeap->ultimoUso = clock();
			paginaDeSiguienteHeap->uso = true;

			paginaDeSiguienteHeap = list_get(carpincho->tabla_de_paginas, i);
			pthread_mutex_unlock(tabla_paginas);

			*DF = buscar_TLB(paginaDeSiguienteHeap);

			reemplazo(DF, carpincho, paginaDeSiguienteHeap);

			pthread_mutex_lock(memoria);
			memcpy(buff_heap + (tamanioPagina - desplazamiento), memoriaPrincipal + *DF , TAMANIO_HEAP - (tamanioPagina - desplazamiento));
			pthread_mutex_unlock(memoria);
			
			pthread_mutex_lock(tabla_paginas);
			paginaDeSiguienteHeap->ultimoUso = clock();
			paginaDeSiguienteHeap->uso = true;
			pthread_mutex_unlock(tabla_paginas);

			memcpy(heap, buff_heap, TAMANIO_HEAP);
			free(buff_heap);

		}else{

		*despl = desplazamiento;

		pthread_mutex_lock(memoria);
		memcpy(heap, memoriaPrincipal + *DF + desplazamiento, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		pthread_mutex_lock(tabla_paginas);
		paginaDeSiguienteHeap->ultimoUso = clock();
		paginaDeSiguienteHeap->uso = true;
		pthread_mutex_unlock(tabla_paginas);

		}

		log_info(loggerMemalloc, "Se leyo un heap del carpincho: %i", carpincho->id_carpincho);

	} while (!(heap->isFree));

	return pagina;
}

t_list* buscarMarcosLibres(t_carpincho* carpincho){

		if(strcmp(tipoAsignacion, "FIJA") == 0){
			 
			pthread_mutex_lock(marcos_sem);
			bool buscarMarcosDelProceso(t_marco* marco){
				return marco->proceso_asignado == carpincho->id_carpincho;
			};
		
			t_list *marcos_del_proceso = list_filter(marcos, (void*)buscarMarcosDelProceso);


			bool marcosLibres(t_marco* marco){
				return marco->estaLibre;
			};
			
			t_list* marcos_a_asignar = list_filter(marcos_del_proceso, (void*)marcosLibres);
			pthread_mutex_unlock(marcos_sem);

			return marcos_a_asignar;   
			
		}
		if(strcmp(tipoAsignacion, "DINAMICA") == 0){
		
			pthread_mutex_lock(marcos_sem);
			bool noEstanAsignados(t_marco* marco){
				return marco->proceso_asignado == -1;
			};

			t_list *marcos_sin_asignar = list_filter(marcos, (void*)noEstanAsignados);
			pthread_mutex_unlock(marcos_sem);

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
	
	pthread_mutex_unlock(listaCarpinchos);
	if(respuesta == 0){ 
		return 0;
	}
	
	//primero pregunta si hay lugar en swap

	bool buscarPag(t_pagina* pag){
    return pag->id_pagina == *pagina;
    };
	

	pthread_mutex_lock(tabla_paginas);
    t_pagina* pag = list_find(carpincho->tabla_de_paginas, (void*)buscarPag);
	pthread_mutex_unlock(tabla_paginas);
	
	int DF = buscar_TLB(pag);

	reemplazo(&DF, carpincho, pag);

	heap->isFree = false;
	heap->nextAlloc = posicionUltimoHeap + TAMANIO_HEAP + tamanio;

	if(tamanioPagina - *desplazamiento < TAMANIO_HEAP){ //esta cortado. Actualiza el ultimo heap

		void* buffer_heap = malloc(TAMANIO_HEAP);

		memcpy(buffer_heap, heap, TAMANIO_HEAP);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + *desplazamiento, buffer_heap, tamanioPagina - *desplazamiento);
		pthread_mutex_unlock(memoria);

		pthread_mutex_lock(tabla_paginas);
		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;
		pthread_mutex_unlock(tabla_paginas);

		bool buscarSigPag(t_pagina* pag){
			return pag->id_pagina > *pagina;
		};
		
		pthread_mutex_lock(tabla_paginas);
		t_pagina* paginaDeSiguienteHeap = list_find(carpincho->tabla_de_paginas, (void*)buscarSigPag);
		pthread_mutex_unlock(tabla_paginas);
		*pagina = paginaDeSiguienteHeap->id_pagina;

		int DF = buscar_TLB(paginaDeSiguienteHeap);
		reemplazo(&DF, carpincho, paginaDeSiguienteHeap);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF , buffer_heap + (tamanioPagina - *desplazamiento) , TAMANIO_HEAP - (tamanioPagina - *desplazamiento));
		pthread_mutex_unlock(memoria);
		
		pthread_mutex_lock(tabla_paginas);
		paginaDeSiguienteHeap->modificado = true;
		paginaDeSiguienteHeap->ultimoUso = clock();
		paginaDeSiguienteHeap->uso = true;
		pthread_mutex_unlock(tabla_paginas);

		*desplazamiento = - (tamanioPagina - *desplazamiento); //esto esta re trambolico porque despues le suma 9

		free(buffer_heap);
		//free(heap);

	}else{

		//*desplazamiento = - (tamanioPagina - *desplazamiento);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + *desplazamiento, heap, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		pthread_mutex_lock(tabla_paginas);
		pag->modificado = true;
		pag->ultimoUso = clock();
		pag->uso = true;
		pthread_mutex_unlock(tabla_paginas);
		//free(heap);

	}

	//Hasta aca se actualiza el ultimo heap.
	//Se crea el nuevo que seria el ultimo.

	heapMetadata *nuevoHeap = malloc(TAMANIO_HEAP);

	nuevoHeap->isFree = true;
	nuevoHeap->prevAlloc = posicionUltimoHeap;
	nuevoHeap->nextAlloc = -1;
	
	pthread_mutex_lock(swap);
	for(int i=0; i<cantidadDePaginasACrear; i++){

		t_pagina* paginaNueva = malloc(sizeof(t_pagina));
		paginaNueva->id_pagina = generadorIdsPaginas(carpincho);
		paginaNueva->esNueva = true;
		paginaNueva->presente = false;
		paginaNueva->uso = true;
		paginaNueva->ultimoUso = clock();
        paginaNueva->modificado = true;
		paginaNueva->id_carpincho = carpincho->id_carpincho;

		void* paginaVacia = malloc(tamanioPagina);

		for(int j=0; j<tamanioPagina;j++){
            char valor = '\0';
            memcpy(paginaVacia +j, &valor, 1);
        }

		enviar_pagina(carpincho->id_carpincho, paginaNueva->id_pagina, paginaVacia);

		free(paginaVacia);
		list_add(carpincho->tabla_de_paginas, paginaNueva);

	}
	pthread_mutex_unlock(swap);


	if(cantidadDePaginasACrear == 0){ //hay que crear el alloc en la misma pag.

			t_pagina* p = list_get(carpincho->tabla_de_paginas, list_size(carpincho->tabla_de_paginas)-1);

			int DF = buscar_TLB(p);
			reemplazo(&DF, carpincho, p);

			pthread_mutex_lock(memoria);
			memcpy(memoriaPrincipal + DF + (*desplazamiento + TAMANIO_HEAP) + tamanio, nuevoHeap, TAMANIO_HEAP);
			pthread_mutex_unlock(memoria);

			p->modificado = true;
			p->uso = true;
			p->ultimoUso = clock();

			free(nuevoHeap);
			
		return 1;
	}

	t_list* marcos_a_asignar = buscarMarcosLibres(carpincho);


	if(cantidadDePaginasACrear == 1 && (*desplazamiento + TAMANIO_HEAP + tamanio) < tamanioPagina){ //actualiza el primer pedacito del heap cortado al final de la misma pagina

		t_pagina* p = list_get(carpincho->tabla_de_paginas, list_size(carpincho->tabla_de_paginas)-2);

		DF = buscar_TLB(p);

		reemplazo(&DF, carpincho, p);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + DF + (*desplazamiento + TAMANIO_HEAP) + tamanio, nuevoHeap, tamanioPagina - (*desplazamiento + TAMANIO_HEAP + tamanio));
		pthread_mutex_unlock(memoria);
		pthread_mutex_lock(tabla_paginas);
		p->modificado = true;
		p->ultimoUso = clock();
		p->uso = true;
		pthread_mutex_unlock(tabla_paginas);

		//free(nuevoHeap);


	}

	void* buffer_allocs = generar_buffer_allocs(tamanio, nuevoHeap, cantidadDePaginasACrear, AGREGAR_ALLOC, *desplazamiento);

	bool paginas_nuevas(t_pagina* pag){
		return pag->esNueva;
	};
	pthread_mutex_lock(tabla_paginas);
	t_list* paginasNuevas = list_filter(carpincho->tabla_de_paginas, (void*)paginas_nuevas);

	pthread_mutex_unlock(tabla_paginas);

	pthread_mutex_lock(marcos_sem);
	escribirMemoria(buffer_allocs, paginasNuevas, marcos_a_asignar, carpincho);
	pthread_mutex_unlock(marcos_sem);

	free(buffer_allocs);
	free(nuevoHeap);
	
	list_iterate(paginasNuevas, (void*)algoritmo_reemplazo_TLB);

	list_destroy(paginasNuevas);

	return 1;
}