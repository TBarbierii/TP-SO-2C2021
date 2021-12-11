#include "memwrite_read_free.h"

void liberar_alloc(uint32_t carpincho, uint32_t DL){

    bool buscarCarpincho(t_carpincho* s){
	return s->id_carpincho == carpincho;
	};
    pthread_mutex_lock(listaCarpinchos);
	t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);
    pthread_mutex_unlock(listaCarpinchos);

    uint32_t id = obtenerId(DL);

    bool buscarPag(t_pagina* pag){
    return pag->id_pagina == id;
    };

    t_pagina* pagina = list_find(capybara->tabla_de_paginas, (void*)buscarPag);

    uint32_t desplazamiento = obtenerDesplazamiento(DL);

    int32_t DF = buscar_TLB(pagina);

    reemplazo(&DF, capybara, pagina);


    heapMetadata *heap = malloc(TAMANIO_HEAP);
    int32_t prevAlloc, nextAlloc;

    if(desplazamiento < TAMANIO_HEAP){//esta cortado y empieza en la pagina anterior

        int desplazamiento1 = tamanioPagina - (TAMANIO_HEAP - desplazamiento);

        void* buffer_heap = malloc(desplazamiento);

        pthread_mutex_lock(memoria);
        memcpy(buffer_heap + (TAMANIO_HEAP - desplazamiento), memoriaPrincipal + DF, desplazamiento);
        pthread_mutex_unlock(memoria);
        
        pagina->ultimoUso = clock();
        pagina->uso = true;

        bool buscarAntPag(t_pagina* pag){
        return pag->id_pagina < id;
        };

        t_pagina* paginaAnterior = list_find(capybara->tabla_de_paginas, (void*)buscarAntPag);

        DF = buscar_TLB(paginaAnterior);

        reemplazo(&DF, capybara, paginaAnterior);

        pthread_mutex_lock(memoria);
        memcpy(buffer_heap , memoriaPrincipal + DF + desplazamiento1, TAMANIO_HEAP - desplazamiento);
        pthread_mutex_unlock(memoria);
        
        paginaAnterior->ultimoUso = clock();
        paginaAnterior->uso = true;

        memcpy(heap, buffer_heap, TAMANIO_HEAP);

        heap->isFree = true;
        prevAlloc = heap->prevAlloc;
        nextAlloc = heap->nextAlloc;

        memcpy(buffer_heap, heap, TAMANIO_HEAP);

        pthread_mutex_lock(memoria);
        memcpy(memoriaPrincipal + DF + desplazamiento1, buffer_heap, TAMANIO_HEAP - desplazamiento);
        pthread_mutex_unlock(memoria);

        paginaAnterior->modificado = true;
        paginaAnterior->ultimoUso = clock();
        paginaAnterior->uso = true;

        DF = buscar_TLB(pagina);

        reemplazo(&DF, capybara, pagina);

        pthread_mutex_lock(memoria);
        memcpy(memoriaPrincipal + DF, buffer_heap + (TAMANIO_HEAP - desplazamiento), desplazamiento);
        pthread_mutex_unlock(memoria);

        pagina->modificado = true;
        pagina->ultimoUso = clock();
        pagina->uso = true;

        free(buffer_heap);
        free(heap);

    }else{
        pthread_mutex_lock(memoria);
        memcpy(heap, memoriaPrincipal + DF + (desplazamiento - TAMANIO_HEAP), TAMANIO_HEAP);
        pthread_mutex_unlock(memoria);

        heap->isFree = true;
        prevAlloc = heap->prevAlloc;
        nextAlloc = heap->nextAlloc;

        pthread_mutex_lock(memoria);
        memcpy(memoriaPrincipal + DF + (desplazamiento - TAMANIO_HEAP), heap, TAMANIO_HEAP);
        pthread_mutex_unlock(memoria);

        pagina->modificado = true;
        pagina->ultimoUso = clock();
        pagina->uso = true;

        free(heap);
    }
    
    log_info(loggerServidor, "Se libero alloc del carpincho %i", capybara->id_carpincho);

    consolidar_allocs(desplazamiento, pagina, prevAlloc, nextAlloc, capybara);

}

uint32_t escribir_memoria(uint32_t carpincho ,uint32_t direccion_logica, void* contenido, uint32_t tam){

    uint32_t id = obtenerId(direccion_logica);

	uint32_t desplazamiento = obtenerDesplazamiento(direccion_logica);

    bool buscarCarpincho(t_carpincho* s){
	return s->id_carpincho == carpincho;
	};
    pthread_mutex_lock(listaCarpinchos);
	t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);
    pthread_mutex_unlock(listaCarpinchos);

        int32_t contador = 0;
    bool buscarPagina(t_pagina* s){
        contador++;
    return s->id_pagina == id;
    };

    pthread_mutex_lock(tabla_paginas);
    t_pagina* pagina = list_find(capybara->tabla_de_paginas,(void*)buscarPagina);
    pthread_mutex_unlock(tabla_paginas);

    int32_t presente = buscar_TLB(pagina);

    reemplazo(&presente, capybara, pagina);

    bool estaCortado = desplazamiento + tam > tamanioPagina;


    if(estaCortado){

        
        int32_t acumulador = (tamanioPagina - desplazamiento), cantPaginas = 0;

        while(acumulador <= tam){
            acumulador += tamanioPagina;
            cantPaginas++;
        }

        uint32_t resto = tamanioPagina - (acumulador - tam);


        uint32_t bytesPrimeraEscritura =tamanioPagina - desplazamiento;
        //ver si las paginas estan presentes

        pthread_mutex_lock(memoria);
        memcpy(memoriaPrincipal + pagina->marco->comienzo + desplazamiento, contenido , bytesPrimeraEscritura);
        pthread_mutex_unlock(memoria);
        
        pthread_mutex_lock(tabla_paginas);
        pagina->modificado = true;
        pagina->ultimoUso = clock();
        pagina->uso = true;
        pthread_mutex_unlock(tabla_paginas);

        if(cantPaginas > 1){
            for(int i=0; i<cantPaginas-1; i++){

                pthread_mutex_lock(tabla_paginas);
                t_pagina* paginaSiguiente = list_get(capybara->tabla_de_paginas, contador);
                pthread_mutex_unlock(tabla_paginas);

                int32_t presente = buscar_TLB(paginaSiguiente);

                reemplazo(&presente, capybara, paginaSiguiente);

                pthread_mutex_lock(memoria);
                memcpy(memoriaPrincipal + (paginaSiguiente->marco->comienzo), contenido + bytesPrimeraEscritura, tamanioPagina);
                pthread_mutex_unlock(memoria);
                
                pthread_mutex_lock(tabla_paginas);
                paginaSiguiente->modificado = true;
                paginaSiguiente->ultimoUso = clock();
                paginaSiguiente->uso = true;
                pthread_mutex_unlock(tabla_paginas);

                contador++;
                bytesPrimeraEscritura += tamanioPagina;
            }
        }

        pthread_mutex_lock(tabla_paginas);
        t_pagina* ultimaPagina = list_get(capybara->tabla_de_paginas, contador);
        pthread_mutex_unlock(tabla_paginas);

        int32_t presente = buscar_TLB(ultimaPagina);

        reemplazo(&presente, capybara, ultimaPagina);

        pthread_mutex_lock(memoria);
        memcpy(memoriaPrincipal + (ultimaPagina->marco->comienzo), contenido + bytesPrimeraEscritura, resto);
        pthread_mutex_unlock(memoria);

        pthread_mutex_lock(swap);
        ultimaPagina->modificado = true;
        ultimaPagina->ultimoUso = clock();
        ultimaPagina->uso = true;
        pthread_mutex_unlock(swap);

        log_info(loggerServidor, "Se escribio la memoria correctamente. Carpincho %i. DirecLogica %i. Tamanio %i", carpincho, direccion_logica, tam);

    }else{

        pthread_mutex_lock(memoria);
        memcpy(memoriaPrincipal + pagina->marco->comienzo + desplazamiento, contenido , tam);
        pthread_mutex_unlock(memoria);
        
        pthread_mutex_lock(tabla_paginas);
        pagina->modificado = true;
        pagina->ultimoUso = clock();
        pagina->uso = true;
        pthread_mutex_unlock(tabla_paginas);

        log_info(loggerServidor, "Se escribio la memoria correctamente. Carpincho %i. DirecLogica %i. Tamanio %i", carpincho, direccion_logica, tam);

        return 0;
    }
     
 
}


void* leer_memoria(uint32_t DL, uint32_t carpincho, uint32_t tam){

    uint32_t id = obtenerId(DL);

	uint32_t desplazamiento = obtenerDesplazamiento(DL);

    bool buscarCarpincho(t_carpincho* s){
	return s->id_carpincho == carpincho;
	};
    pthread_mutex_lock(listaCarpinchos);
	t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);
    pthread_mutex_unlock(listaCarpinchos);

    int32_t contador = 0;
    bool buscarPagina(t_pagina* s){
        contador++;
    return s->id_pagina == id;
    };

    pthread_mutex_lock(tabla_paginas);
    t_pagina* pagina = list_find(capybara->tabla_de_paginas,(void*)buscarPagina);
    pthread_mutex_unlock(tabla_paginas);

    int32_t presente = buscar_TLB(pagina);

    reemplazo(&presente, capybara, pagina);

    void* leido = malloc(tam);

    bool estaCortado = desplazamiento + tam > tamanioPagina;

    if(estaCortado){
 
        uint32_t acumulador = (tamanioPagina - desplazamiento), cantPaginas = 0;

        while(acumulador <= tam){
            acumulador += tamanioPagina;
            cantPaginas++;
        }

        uint32_t resto = tamanioPagina - (acumulador - tam);

        uint32_t bytesPrimeraLectura =tamanioPagina - desplazamiento;

        pthread_mutex_lock(memoria);
        memcpy(leido, memoriaPrincipal + pagina->marco->comienzo + desplazamiento, bytesPrimeraLectura);
        pthread_mutex_unlock(memoria);
        
        pthread_mutex_lock(tabla_paginas);
        pagina->ultimoUso = clock();
        pagina->uso = true;
        pthread_mutex_unlock(tabla_paginas);

        if(cantPaginas > 1){
            for(int i=0; i<cantPaginas-1; i++){

                pthread_mutex_lock(tabla_paginas);
                t_pagina* paginaSiguiente = list_get(capybara->tabla_de_paginas, contador);
                pthread_mutex_unlock(tabla_paginas);

                int32_t presente = buscar_TLB(paginaSiguiente);

                reemplazo(&presente, capybara, paginaSiguiente);

                pthread_mutex_lock(memoria);
                memcpy(leido + bytesPrimeraLectura, memoriaPrincipal + (paginaSiguiente->marco->comienzo), tamanioPagina);
                pthread_mutex_unlock(memoria);
                
                pthread_mutex_lock(tabla_paginas);
                paginaSiguiente->ultimoUso = clock();
                paginaSiguiente->uso = true;
                pthread_mutex_unlock(tabla_paginas);

                contador++;
                bytesPrimeraLectura += tamanioPagina;
            }
        }

        pthread_mutex_lock(tabla_paginas);
        t_pagina* ultimaPagina = list_get(capybara->tabla_de_paginas, contador);
        pthread_mutex_unlock(tabla_paginas);

        int32_t presente = buscar_TLB(ultimaPagina);

        reemplazo(&presente, capybara, ultimaPagina);

        pthread_mutex_lock(memoria);
        memcpy(leido + bytesPrimeraLectura, memoriaPrincipal + (ultimaPagina->marco->comienzo), resto);
        pthread_mutex_unlock(memoria);
        
        pthread_mutex_lock(tabla_paginas);
        ultimaPagina->ultimoUso = clock();
        ultimaPagina->uso = true;
        pthread_mutex_unlock(tabla_paginas);


    }else{
        
        pthread_mutex_lock(memoria);
        memcpy(leido, memoriaPrincipal + pagina->marco->comienzo + desplazamiento, tam);
        pthread_mutex_unlock(memoria);
        
        pthread_mutex_lock(tabla_paginas);
        pagina->ultimoUso = clock();
        pagina->uso = true;
        pthread_mutex_unlock(tabla_paginas);
    }
     
    log_info(loggerServidor, "Se leyo la memoria correctamente. Carpincho %i. DirecLogica %i. Tamanio %i", carpincho, DL, tam);
    
    return leido;

}