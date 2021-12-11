#include "Memoria.h"
#include "memoriaVirtual_suspencion.h"

void manejador_de_seniales(int numeroSenial){
	
	if(numeroSenial == SIGINT){

			int cantidadCarpinchos = list_size(carpinchosMetricas);

			for (int i=0; i<cantidadCarpinchos; i++){
					
			pthread_mutex_lock(listaCarpinchos);
			t_carpincho* carp = list_get(carpinchosMetricas, i);
			pthread_mutex_unlock(listaCarpinchos);

			log_info(logsObligatorios, "Carpincho %i. Hits: %i Misses: %i.", carp->id_carpincho, carp->tlb_hit, carp->tlb_miss);

			}

			log_info(logsObligatorios, "Hits totales: %i. Misses totales %i.", hits_totales, miss_totales);

			void liberarCarpinchos(t_carpincho* c){
				free(c);
			};

			list_clean_and_destroy_elements(carpinchosMetricas, (void*)liberarCarpinchos);
		
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
		pthread_mutex_lock(TLB_mutex);
		list_clean(TLB);
		pthread_mutex_unlock(TLB_mutex);
	}

}

void imprimir_dump(t_log* log_dump, char * time){
	
	log_info(log_dump,"--------------------------------------------------------------------------");
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

	int32_t primerHeapNext, ultimoHeapPrev;
	int32_t nuevoHeapPrev = posicionHeap, nuevoHeapNext;
	int32_t posicionNuevoHeap = posicionHeap + TAMANIO_HEAP + tamanio;

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
	nuevoHeap->prevAlloc = nuevoHeapPrev;

	int nextAlloc = primerHeap->nextAlloc;

	primerHeap->isFree=false;
	primerHeap->nextAlloc = posicionNuevoHeap;

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

	
	} else{
		pthread_mutex_lock(memoria);
		memcpy(ultimoHeap, memoriaPrincipal + pag->marco->comienzo + (nextAlloc - posicionHeap - TAMANIO_HEAP), TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

	}


	ultimoHeap->prevAlloc = posicionNuevoHeap;
	
	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + pag->marco->comienzo + (nextAlloc - posicionHeap - TAMANIO_HEAP), ultimoHeap,TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);


	pthread_mutex_lock(memoria);
	memcpy(memoriaPrincipal + pag->marco->comienzo + desplazamiento + TAMANIO_HEAP + tamanio, nuevoHeap, TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);


	free(primerHeap);
	free(ultimoHeap);
	free(nuevoHeap);
}

void consolidar_allocs(int desplazamientoHeapLiberado, t_pagina* pagina, int32_t prevAlloc, int32_t nextAlloc, t_carpincho* carpincho){
	
	heapMetadata* heapLiberado = malloc(TAMANIO_HEAP);
	heapMetadata* heapAnterior = malloc(TAMANIO_HEAP);
	heapMetadata* heapPosterior = malloc(TAMANIO_HEAP);
	heapMetadata* heapSiguienteDelSiguiente = malloc(TAMANIO_HEAP);

	t_pagina* paginaDelSiguienteDelSiguiente; 
	t_pagina* paginaDelHeapAnterior;

	int desplazamiento, idPagina, lecturaAnterior = 0, lecturaSiguienteSiguiente = 0, nextNextAlloc;
	int32_t DF;

	bool buscarPag(t_pagina* pag){
    return pag->id_pagina == idPagina;
    };

	//leer heap liberado

	DF = buscar_TLB(pagina);

	reemplazo(&DF, carpincho, pagina);

	pthread_mutex_lock(memoria);
	memcpy(heapLiberado, memoriaPrincipal + pagina->marco->comienzo + desplazamientoHeapLiberado - TAMANIO_HEAP, TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);

	//leer el alloc anterior

	if (heapLiberado->prevAlloc != -1 ){ // lee el anterior solo si existe un anterior

		idPagina= prevAlloc/tamanioPagina;

		paginaDelHeapAnterior = list_find(carpincho->tabla_de_paginas, (void*)buscarPag);

		desplazamiento = prevAlloc % tamanioPagina;

		DF = buscar_TLB(paginaDelHeapAnterior);

		reemplazo(&DF, carpincho, paginaDelHeapAnterior);

		pthread_mutex_lock(memoria);
		memcpy(heapAnterior, memoriaPrincipal + paginaDelHeapAnterior->marco->comienzo + desplazamiento, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		lecturaAnterior = 1;
	}

	//leer el alloc posterior

	idPagina= nextAlloc/tamanioPagina;

	t_pagina* paginaDelHeapPosterior = list_find(carpincho->tabla_de_paginas, (void*)buscarPag);

	desplazamiento = nextAlloc % tamanioPagina;

	DF = buscar_TLB(paginaDelHeapPosterior);

	reemplazo(&DF, carpincho, paginaDelHeapPosterior);

	pthread_mutex_lock(memoria);
	memcpy(heapPosterior, memoriaPrincipal + paginaDelHeapPosterior->marco->comienzo + desplazamiento, TAMANIO_HEAP);
	pthread_mutex_unlock(memoria);

	//lee el siguiente del siguiente

	if(heapPosterior->nextAlloc != -1){

		nextNextAlloc = heapPosterior->nextAlloc;
		
		idPagina= nextNextAlloc/tamanioPagina;

		paginaDelSiguienteDelSiguiente = list_find(carpincho->tabla_de_paginas, (void*)buscarPag);

		desplazamiento = nextNextAlloc % tamanioPagina;

		DF = buscar_TLB(paginaDelSiguienteDelSiguiente);

		reemplazo(&DF, carpincho, paginaDelSiguienteDelSiguiente);

		pthread_mutex_lock(memoria);
		memcpy(heapSiguienteDelSiguiente, memoriaPrincipal + paginaDelSiguienteDelSiguiente->marco->comienzo + desplazamiento, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		lecturaSiguienteSiguiente = 1;
	}
	
	//casos

	if (heapLiberado->prevAlloc == -1 && heapPosterior->nextAlloc == -1){//hay un solo alloc y hay que liberar todas las paginas



		
		void destructor(t_pagina* pag){

			pthread_mutex_lock(TLB_mutex);
			bool quitarDeTLB(t_pagina* p){
				return pag->id_pagina == p->id_pagina && pag->id_carpincho ==  p->id_carpincho;
			};
			list_remove_by_condition(TLB, (void*)quitarDeTLB);// se quita directamente la pagina que se mando a swap.
			log_trace(logsObligatorios, "[TLB] Victima: PID: %i	Página: %i	Marco: %i", pag->id_carpincho, pag->id_pagina, pag->marco->id_marco);
			pthread_mutex_unlock(TLB_mutex);

			pag->marco->estaLibre = true;
			pag->marco->proceso_asignado = -1;
//			free(pag);
		};

		list_clean_and_destroy_elements(carpincho->tabla_de_paginas, (void*)destructor);


		carpincho->contadorPag -- ;



		log_info(loggerServidor, "Se liberaron 1 paginas del carpincho %i", carpincho->id_carpincho);

	}else if(lecturaAnterior == 1 && lecturaSiguienteSiguiente == 1 && heapAnterior->isFree && heapPosterior->isFree){ //ambos estan libres

		heapAnterior->nextAlloc = heapPosterior->nextAlloc;

		desplazamiento = prevAlloc % tamanioPagina;

		DF = buscar_TLB(paginaDelHeapAnterior);

		reemplazo(&DF, carpincho, paginaDelHeapAnterior);

		pthread_mutex_lock(memoria);
		memcpy( memoriaPrincipal + paginaDelHeapAnterior->marco->comienzo + desplazamiento, heapAnterior, TAMANIO_HEAP);//graba el heap anterior actualizado
		pthread_mutex_unlock(memoria);

		heapSiguienteDelSiguiente->prevAlloc = prevAlloc;

		desplazamiento = nextNextAlloc % tamanioPagina;

		DF = buscar_TLB(paginaDelSiguienteDelSiguiente);

		reemplazo(&DF, carpincho, paginaDelSiguienteDelSiguiente);
		
		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + paginaDelSiguienteDelSiguiente->marco->comienzo + desplazamiento, heapSiguienteDelSiguiente, TAMANIO_HEAP);//graba el heap siguiente del siguiente actualizado
		pthread_mutex_unlock(memoria);

	}else if (lecturaAnterior == 1 && heapAnterior->isFree && !heapPosterior->isFree){
		
		heapAnterior->nextAlloc = nextAlloc;

		desplazamiento = prevAlloc % tamanioPagina;

		pthread_mutex_lock(memoria);
		memcpy( memoriaPrincipal + paginaDelHeapAnterior->marco->comienzo + desplazamiento, heapAnterior, TAMANIO_HEAP);//graba el heap anterior actualizado
		pthread_mutex_unlock(memoria);

		heapPosterior->prevAlloc = prevAlloc;

		desplazamiento = nextAlloc % tamanioPagina;

		DF = buscar_TLB(paginaDelSiguienteDelSiguiente);

		reemplazo(&DF, carpincho, paginaDelSiguienteDelSiguiente);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + paginaDelHeapPosterior->marco->comienzo + desplazamiento, heapPosterior, TAMANIO_HEAP);//graba el heap siguiente del siguiente actualizado
		pthread_mutex_unlock(memoria);


	}else if (lecturaAnterior == 1 && lecturaSiguienteSiguiente == 1 && heapPosterior->isFree && !heapAnterior->isFree){

		heapLiberado->nextAlloc = heapPosterior->nextAlloc;

		DF = buscar_TLB(pagina);

		reemplazo(&DF, carpincho, pagina);

		pthread_mutex_lock(memoria);
		memcpy( memoriaPrincipal + pagina->marco->comienzo + desplazamientoHeapLiberado - TAMANIO_HEAP, heapLiberado, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		heapSiguienteDelSiguiente->prevAlloc = heapPosterior->prevAlloc;

		DF = buscar_TLB(paginaDelSiguienteDelSiguiente);

		reemplazo(&DF, carpincho, paginaDelSiguienteDelSiguiente);

		pthread_mutex_lock(memoria);
		memcpy(memoriaPrincipal + paginaDelSiguienteDelSiguiente->marco->comienzo + desplazamiento, heapSiguienteDelSiguiente, TAMANIO_HEAP);//graba el heap siguiente del siguiente actualizado
		pthread_mutex_unlock(memoria);

	}else if (lecturaAnterior == 1 && lecturaSiguienteSiguiente == 0 && heapPosterior->isFree && !heapAnterior->isFree){

		heapLiberado->nextAlloc = -1;

		DF = buscar_TLB(pagina);

		reemplazo(&DF, carpincho, pagina);

		pthread_mutex_lock(memoria);
		memcpy( memoriaPrincipal + pagina->marco->comienzo + desplazamientoHeapLiberado, heapLiberado, TAMANIO_HEAP);
		pthread_mutex_unlock(memoria);

		pagina->modificado = true;
        pagina->ultimoUso = clock();
        pagina->uso = true;


	}	
	free(heapLiberado);
	free(heapAnterior);
	free(heapPosterior);
	free(heapSiguienteDelSiguiente);


}