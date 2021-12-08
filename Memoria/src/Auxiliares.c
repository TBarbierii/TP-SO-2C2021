#include "Memoria.h"
#include "memoriaVirtual_suspencion.h"

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