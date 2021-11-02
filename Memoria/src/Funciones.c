#include "Memoria.h"


uint32_t administrar_allocs(t_memalloc* alloc){ //que kernel mande los carpinchos en el init

    t_list* marcos_a_asignar = list_create();//probar sacar el list_create

    bool buscarCarpincho(t_carpincho* c){
		return c->id_carpincho == alloc->pid;
	};
    //semaforo?
    t_carpincho* carpincho = list_find(carpinchos, (void*)buscarCarpincho);
    //semaforo?
    //si no lo encuentra crearlo
    if (carpincho == NULL){
     carpincho = malloc(sizeof(t_carpincho));
     carpincho->id_carpincho = alloc->pid;
     carpincho->tabla_de_paginas = list_create();

     marcos_a_asignar = reservarMarcos(carpincho->id_carpincho);
    

     list_add(carpinchos, carpincho);
    }

    uint32_t direccionLogica = administrar_paginas(carpincho, alloc->tamanio, marcos_a_asignar);


    return direccionLogica;

}



uint32_t administrar_paginas(t_carpincho* carpincho, uint32_t tamanio, t_list* marcos_a_asignar){

    if(list_size(carpincho->tabla_de_paginas)==0){ //primera vez

        heapMetadata* alloc = malloc(sizeof(heapMetadata));
        alloc->prevAlloc = -1;
        alloc->isFree = false;
        alloc->nextAlloc = tamanio + TAMANIO_HEAP;

        heapMetadata* next_alloc = malloc(sizeof(heapMetadata));
        next_alloc->isFree = true;
        next_alloc->prevAlloc = 0;
        next_alloc->nextAlloc = -1;

        uint32_t cantidadDePaginasACrear = ceil((float)(TAMANIO_HEAP*2 + tamanio)/tamanioPagina);

        for(int i=0; i<cantidadDePaginasACrear; i++){

            t_pagina* pagina = malloc(sizeof(t_pagina));
            pagina->id_pagina = generadorIdsPaginas();
            pagina->presente = true;
            //ver esto que onda con el tema de swap.
            pagina->ultimoUso = clock();
            pagina->modificado = true;

            //enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, "");

            list_add(carpincho->tabla_de_paginas, pagina);
        }

        //for cada pagina creada. enviarlas a swap. Y si no hay espacio devolver null

        void* buffer_allocs = generar_buffer_allocs(tamanio, next_alloc,cantidadDePaginasACrear, PRIMERA_VEZ, 0);

        escribirMemoria(buffer_allocs, carpincho->tabla_de_paginas, marcos_a_asignar);


        free (buffer_allocs);


        void agregarATLB(t_pagina* pag){
            list_add(TLB, pag);
        };
        list_iterate(carpincho->tabla_de_paginas, (void*)agregarATLB);

        t_pagina* pag = list_get(carpincho->tabla_de_paginas, 0);
        return generarDireccionLogica(pag->id_pagina, TAMANIO_HEAP);

    }else{//busca un hueco
        
        t_pagina* primeraPag = list_get(carpincho->tabla_de_paginas, 0);

        int32_t DF = buscar_TLB(primeraPag->id_pagina);

        if(DF == -1){ //tlb miss
            //buscar en tabla de paginas
            //swapear
            //actualizar tlb
        }

        heapMetadata *heap = malloc(TAMANIO_HEAP); //liberar
        int32_t posicionHeap = 0;
        int32_t pagina, desplazamiento;

        memcpy(heap, memoriaPrincipal + DF, TAMANIO_HEAP);


        while(heap->nextAlloc != -1)
        {
            if(!(heap->isFree)){
            pagina = buscarSiguienteHeapLibre(heap, &DF, carpincho->tabla_de_paginas, &posicionHeap, &desplazamiento);
            }
            
            if(heap->nextAlloc == -1) break;

            uint32_t espacioEncontrado = heap->nextAlloc - posicionHeap - TAMANIO_HEAP;

            if (tamanio == espacioEncontrado || tamanio < espacioEncontrado + TAMANIO_HEAP){ // +9 porque si es mas chico en lo que sobra tiene que entrar el otro heap (aunque tambien habria que agregar +minimo_espacio_aceptable)
                break;
            }else{

            pagina = buscarSiguienteHeapLibre(heap, &DF, carpincho->tabla_de_paginas, &posicionHeap, &desplazamiento);
            }

        }

        //dividir

        if(heap->nextAlloc == -1){

            crearAllocNuevo(&pagina, tamanio, heap, posicionHeap, carpincho, &desplazamiento);//pasar pagina y despl por referencia y si esta cortado modificarlo
        }

        return generarDireccionLogica(pagina, desplazamiento + TAMANIO_HEAP);
        

    }     
        

}



void crear_marcos(){

    uint32_t cantidad_marcos = tamanio/tamanioPagina;


        for(uint32_t i=0; i<cantidad_marcos; i++){

            t_marco* marco = malloc(sizeof(t_marco));

            marco->id_marco = generadorIdsMarcos();
            marco->proceso_asignado = -1;
            marco->estaLibre = true;
            marco->comienzo = i * tamanioPagina; //Aca deberia haber una funcion recursiva que le vaya cambiando donde empieza
                  //El primero en 0, el segundo 0 + tamanioPagina y asi.

            list_add(marcos, marco);

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

        return stream_allocs;
    }
        if(codigo == AGREGAR_ALLOC){

        int bytesQueYaEstan = tamanioPagina - (despl + TAMANIO_HEAP);

        memcpy(stream_allocs + (tamanio - bytesQueYaEstan), heap , TAMANIO_HEAP);

        return stream_allocs;
    }


}

void liberar_alloc(uint32_t carpincho, uint32_t DL){

    uint32_t id = obtenerId(DL);

    uint32_t DF = calcular_direccion_fisica(carpincho, DL);

    uint32_t posicionHeap = DF - 9;

    bool buscarCarpincho(t_carpincho* s){
	return s->id_carpincho == id;
	};

	t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);
    
    int32_t contador = 0;
    bool buscarHeap(heapMetadata *alloc){
        contador++;
        return alloc->nextAlloc == posicionHeap;
    };

    heapMetadata *alloc = list_find(capybara->allocs, (void*)buscarHeap);

    heapMetadata *allocPosta = list_get(capybara->allocs,contador);

    allocPosta->isFree = true;

    //consolidar_allocs y liberar_paginas

}

void* leer_memoria(uint32_t DL, uint32_t carpincho, uint32_t tam){

    uint32_t id = obtenerId(DL);

	uint32_t desplazamiento = obtenerDesplazamiento(DL);

    bool buscarCarpincho(t_carpincho* s){
	return s->id_carpincho == carpincho;
	};

	t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);

    uint32_t DF = calcular_direccion_fisica(carpincho, DL);

    void* leido = malloc(tam);

    bool estaCortado = desplazamiento + tam > tamanioPagina;

    if(estaCortado){

        int32_t contador = 0;
       
        bool buscar_pagina_inicio(t_pagina* pagina){
            contador++;
            return pagina->id_pagina == id; 
        };
 
        uint32_t acumulador = (tamanioPagina - desplazamiento), cantPaginas = 0;

        while(acumulador <= tam){
            acumulador += tamanioPagina;
            cantPaginas++;
        }

        uint32_t resto = tamanioPagina - (acumulador - tam);

        t_pagina* primeraPagina = list_find(capybara->tabla_de_paginas, (void*)buscar_pagina_inicio);

        uint32_t bytesPrimeraLectura =tamanioPagina - desplazamiento;
        //ver si las paginas estan presentes

        memcpy(leido, memoriaPrincipal + DF, bytesPrimeraLectura);

        if(cantPaginas > 1){
            for(int i=0; i<cantPaginas-1; i++){
                t_pagina* paginaSiguiente = list_get(capybara->tabla_de_paginas, contador);
                memcpy(leido + bytesPrimeraLectura, memoriaPrincipal + (paginaSiguiente->marco->comienzo), tamanioPagina);
                contador++;
                bytesPrimeraLectura += tamanioPagina;
            }
        }

        t_pagina* ultimaPagina = list_get(capybara->tabla_de_paginas, contador);
         memcpy(leido + bytesPrimeraLectura, memoriaPrincipal + (ultimaPagina->marco->comienzo), resto);


    }else{
        memcpy(leido, memoriaPrincipal + DF, tam);
    }
     
 
    return leido;

}

uint32_t escribir_memoria(uint32_t carpincho ,uint32_t direccion_logica, void* contenido, uint32_t tam){

    uint32_t id = obtenerId(direccion_logica);

	uint32_t desplazamiento = obtenerDesplazamiento(direccion_logica);

    bool buscarCarpincho(t_carpincho* s){
	return s->id_carpincho == carpincho;
	};

	t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);

    uint32_t DF = calcular_direccion_fisica(carpincho, direccion_logica);

    bool estaCortado = desplazamiento + tam > tamanioPagina;


    if(estaCortado){

        int32_t contador = 0;
       
        bool buscar_pagina_inicio(t_pagina* pagina){
            contador++;
            return pagina->id_pagina == id; 
        };
        
        uint32_t acumulador = (tamanioPagina - desplazamiento), cantPaginas = 0;

        while(acumulador <= tam){
            acumulador += tamanioPagina;
            cantPaginas++;
        }

        uint32_t resto = tamanioPagina - (acumulador - tam);

        t_pagina* primeraPagina = list_find(capybara->tabla_de_paginas, (void*)buscar_pagina_inicio);

        uint32_t bytesPrimeraEscritura =tamanioPagina - desplazamiento;
        //ver si las paginas estan presentes

       memcpy(memoriaPrincipal + DF, contenido , bytesPrimeraEscritura);

        if(cantPaginas > 1){
            for(int i=0; i<cantPaginas-1; i++){
                t_pagina* paginaSiguiente = list_get(capybara->tabla_de_paginas, contador);
                memcpy(memoriaPrincipal + (paginaSiguiente->marco->comienzo), contenido + bytesPrimeraEscritura, tamanioPagina);
                contador++;
                bytesPrimeraEscritura += tamanioPagina;
            }
        }

        t_pagina* ultimaPagina = list_get(capybara->tabla_de_paginas, contador);
        memcpy(memoriaPrincipal + (ultimaPagina->marco->comienzo), contenido + bytesPrimeraEscritura, resto);

    }else{
        memcpy(memoriaPrincipal + DF, contenido , tam);
        return 0;
    }
     
 
}