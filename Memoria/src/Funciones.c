#include "Memoria.h"


uint32_t administrar_allocs(t_memalloc* alloc){ //que kernel mande los carpinchos en el init

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
     carpincho->allocs = list_create();
     //tlb
     list_add(carpinchos, carpincho);
    }

    uint32_t posicionAlloc = buscar_o_agregar_espacio(carpincho, alloc->tamanio); //aca se crearian las paginas en el carpincho. Solo sirven para calcular la DF creo

    uint32_t direccionLogica = administrar_paginas(carpincho, posicionAlloc);

    asignarPaginas(carpincho); //aca se graba en memoria los allocs reservados.

    return direccionLogica;

}


uint32_t buscar_o_agregar_espacio(t_carpincho* carpincho, uint32_t tamanioPedido){ //devuelve desplazamiento (inicio del espacio) respecto a la pagina

   if(list_size(carpincho->allocs) == 0){
       heapMetadata* alloc = malloc(sizeof(heapMetadata));
       alloc->prevAlloc = -1;
       alloc->isFree = false;
       alloc->nextAlloc = tamanioPedido + 9;

       list_add(carpincho->allocs, alloc);

       heapMetadata* next_alloc = malloc(sizeof(heapMetadata));
       next_alloc->isFree = true;
       next_alloc->prevAlloc = 0;
       next_alloc->nextAlloc = -1;

       list_add(carpincho->allocs, next_alloc);

        return 0; 

   }else{//buscar espacio. se podria delegar todo esto porque tambien aca habria que hacer lo de dividir y consolidar allocs

        uint32_t espacioEncontrado = 0, cantidadDeAllocs = list_size(carpincho->allocs), posicionUltimoAlloc ;
        heapMetadata *allocActual, *allocSiguiente, *anteUltimoAlloc = list_get(carpincho->allocs, list_size(carpincho->allocs)-2);

        for(uint32_t i = 0; i < cantidadDeAllocs; i++){

            posicionUltimoAlloc = anteUltimoAlloc->nextAlloc;
            allocActual = list_get(carpincho->allocs, i);
            allocSiguiente = list_get(carpincho->allocs, i + 1);

            if(allocActual->isFree && allocSiguiente != NULL){
                espacioEncontrado = allocActual->nextAlloc - allocSiguiente->prevAlloc - 9;
            }
            if(allocSiguiente == NULL){//llego al ultimo alloc (allocActual) y no encontro espacio. hay que crear uno mas
               
                allocActual->nextAlloc = posicionUltimoAlloc + 9 + tamanioPedido;
                espacioEncontrado = tamanioPedido;

                heapMetadata* allocNuevo = malloc(sizeof(heapMetadata));
                allocNuevo->prevAlloc = posicionUltimoAlloc;
                allocNuevo->isFree = true;
                allocNuevo->nextAlloc = -1;

                list_add(carpincho->allocs, allocNuevo);
                allocSiguiente = allocNuevo;
            }

            if (tamanioPedido == espacioEncontrado || tamanioPedido < espacioEncontrado + 9) // +9 porque si es mas chico en lo que sobra tiene que entrar el otro heap (aunque tambien habria que agregar +minimo_espacio_aceptable)
            break; //sale del for
        }
        
        //falta dividir en caso de que sobre lugar 

        uint32_t posicionAllocActual = allocSiguiente->prevAlloc;
        allocActual->isFree = false;

        return posicionAllocActual;
   }

}

uint32_t administrar_paginas(t_carpincho* carpincho, uint32_t posicionAlloc){

        heapMetadata *anteultimoAlloc = list_get(carpincho->allocs, list_size(carpincho->allocs)-2);

        uint32_t posicionUltimoAlloc = anteultimoAlloc->nextAlloc;

        uint32_t cantidadDePaginasNecesarias = ceil((float)posicionUltimoAlloc/tamanioPagina);

        uint32_t cantidadDePaginasACrear =cantidadDePaginasNecesarias - list_size(carpincho->tabla_de_paginas);

        for (uint32_t i=0; i < cantidadDePaginasACrear; i++){

            t_pagina *pagina = malloc(sizeof(t_pagina));

            pagina->esNueva=true;
            pagina->id_pagina = generadorIdsPaginas();

            list_add(carpincho->tabla_de_paginas, pagina);    
        }

        //todo esto que sigue es para calcualr el desplazamiento de donde empieza el espacio libre
        uint32_t i;
        t_pagina *paginaDelAllocActual;

        for(i=1; i <= list_size(carpincho->tabla_de_paginas); i++){

            if(posicionAlloc < tamanioPagina * i){
            paginaDelAllocActual = list_get(carpincho->tabla_de_paginas, i-1);
            break; 
            }    
        }

        uint32_t desplazamiento =  tamanioPagina - (i * tamanioPagina - posicionAlloc) +9 ; //el desplazamiento relativo a la pagina

    return generarDireccionLogica(paginaDelAllocActual->id_pagina, desplazamiento);

}

uint32_t asignarPaginas(t_carpincho* carpincho){
    
    t_list *marcos_a_asignar;

    if(strcmp(tipoAsignacion, "FIJA") == 0){

        bool noEstanAsignados(t_marco* marco){
            return marco->proceso_asignado == -1;
        };
        
        bool buscarMarcosDelProceso(t_marco* marco){
            return marco->proceso_asignado == carpincho->id_carpincho;
        };

        marcos_a_asignar = list_filter(marcos, (void*)buscarMarcosDelProceso);

        if(list_size(marcos_a_asignar)==0){

        t_list *marcos_sin_asignar = list_filter(marcos, (void*)noEstanAsignados);

        marcos_a_asignar = list_take(marcos_sin_asignar, marcosMaximos);
        

        void marcarOcupados(t_marco *marco){
            //marco->estaLibre = false;
            marco->proceso_asignado = carpincho->id_carpincho;
        };

        list_iterate(marcos_a_asignar, (void*)marcarOcupados);
        }

        escribir_marcos(marcos_a_asignar, carpincho); //aca escribir las paginas nuevas en los marcos_asignados. diferenciar los algoritmos en el caso de que haya que reemplazar. aca es la comunicacion con swap 

    }
    if(strcmp(tipoAsignacion, "DINAMICA") ==0 ){
        
        bool noEstanAsignados(t_marco* marco){
        return marco->proceso_asignado == -1;
        };

        t_list *marcos_libres = list_filter(marcos, (void*)noEstanAsignados);
    
        bool contarPagsNuevas(t_pagina *pag){
            return pag->esNueva;
        };

        uint32_t pagNuevas = list_count_satisfying(carpincho->tabla_de_paginas, (void*)contarPagsNuevas);

        marcos_a_asignar = list_take(marcos_libres, pagNuevas);

        void marcarOcupados(t_marco *marco){
            //marco->estaLibre = false;
            marco->proceso_asignado = carpincho->id_carpincho;
        }

        list_iterate(marcos_a_asignar, (void*)marcarOcupados);

        escribir_marcos(marcos_a_asignar, carpincho);
    }
   
   return 0;
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

void escribir_marcos(t_list* marcos_a_asignar, t_carpincho* carpincho){

    void* stream_allocs = generar_stream_allocs(carpincho);
    uint32_t contador =0;

    if(strcmp(tipoAsignacion, "FIJA") == 0){

        //los marcos a asignar son los reservados para el proceso. pueden estar todos libres, algunos libres o todos ocupados 
        //para lru ver si sirve la funcion clock()
    }


    void escribir_paginas_en_marcos(t_marco* marco){

            t_pagina* pagina = list_get(carpincho->tabla_de_paginas, contador);
            if(pagina != NULL  && pagina->esNueva && marco->estaLibre){
            memcpy(memoriaPrincipal + marco->comienzo, stream_allocs + (contador*tamanioPagina), tamanioPagina);
            pagina->esNueva = false;
            pagina->marco = marco;
            pagina->presente = true;
            marco->estaLibre = false;
            }
            contador++;
    }

    list_iterate(marcos_a_asignar, (void*)escribir_paginas_en_marcos);
    free(stream_allocs);
}

void* generar_stream_allocs(t_carpincho* carpincho){

    uint32_t cantPaginas = list_size(carpincho->tabla_de_paginas);
    uint32_t reserva = tamanioPagina * cantPaginas;
    void* stream_allocs = malloc(reserva);
 
        uint32_t desplazamiento =0, cantidadDeAllocs = list_size(carpincho->allocs);
        heapMetadata *allocActual;

            for(uint32_t i = 0; i < cantidadDeAllocs; i++){

            allocActual = list_get(carpincho->allocs, i);
            

            memcpy(stream_allocs + desplazamiento, allocActual, sizeof(heapMetadata));//llena con el primer alloc

            desplazamiento = allocActual->nextAlloc;

        }

    return stream_allocs;
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