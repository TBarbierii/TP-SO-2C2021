#include "Memoria.h"


void administrar_allocs(t_memalloc alloc){

    bool buscarCarpincho(t_carpincho* c){
		return c->id_carpincho == alloc.pid;
	}
    //semaforo?
    t_carpincho* carpincho = list_find(carpinchos, (void*)buscarCarpincho);
    //semaforo?
    //si no lo encuentra crearlo
    if (carpincho == NULL){
     carpincho = malloc(sizeof(t_carpincho));
     carpincho->id_carpincho = alloc.pid;
     carpincho->tabla_de_paginas = list_create();
     carpincho->allocs = list_create();
     //tlb
     list_add(carpincho, carpinchos);
    }

    uint32_t desplazamiento_alloc = buscar_o_agregar_espacio(carpincho, alloc.tamanio); //aca se crearian las paginas en el carpincho. Solo sirven para calcular la DF creo

    uint32_t numero_de_pagina = administrar_paginas(carpincho);

    uint32_t marco = escribir_en_memoria(carpincho); //aca se graba en memoria los allocs reservados. Devuelve el id_marco



}


uint32_t buscar_o_agregar_espacio(t_carpincho* carpincho, uint32_t tamanioPedido){ //devuelve desplazamiento (inicio del espacio) respecto a la pagina

   if(list_size(carpincho->allocs) == 0){
       heapMetadata* alloc = malloc(sizeof(heapMetadata));
       alloc->nextAlloc = NULL;
       alloc->isFree = true;
       alloc->nextAlloc = tamanioPedido + 9;

       heapMetadata* next_alloc = malloc(sizeof(heapMetadata));
       next_alloc->isFree = true;
       next_alloc->prevAlloc = 0;
       next_alloc->nextAlloc = NULL;

        return generarDireccionLogica(generadorIdsPaginas(), 9); //el espacio del primer alloc siempre va a empezar en 9

   }else{//buscar espacio. se podria delegar todo esto porque tambien aca habria que hacer lo de dividir y consolidar allocs

        uint32_t espacioEncontrado, cantidadDeAllocs = list_size(carpincho->allocs);
        heapMetadata *allocActual, *allocSiguiente;

        for(uint32_t i = 0; i < cantidadDeAllocs; i++){

            allocActual = list_get(carpincho->allocs, i);
            allocSiguiente = list_get(carpincho->allocs, i + 1);

            if(allocActual->isFree){
                espacioEncontrado = allocActual->nextAlloc - allocSiguiente->prevAlloc - 9;
            }

            if (tamanioPedido == espacioEncontrado || tamanioPedido < espacioEncontrado + 9) // +9 porque si es mas chico en lo que sobra tiene que entrar el otro heap (aunque tambien habria que agregar +minimo_espacio_aceptable)
            break; //sale del for
        }
        
        //falta dividir en caso de que sobre lugar
        //todo esto que sigue es para calcualr el desplazamiento de donde empieza el espacio libre
        uint32_t posicionAllocActual = allocSiguiente->prevAlloc;
        uint32_t paginaDelAllocActual;

        for(uint32_t i=1; i <= list_size(carpincho->tabla_de_paginas); i++){

            if(posicionAllocActual < tamanioPagina * i){
            paginaDelAllocActual = i;
            break; 
            }    
        }

        uint32_t desplazamiento =  tamanioPagina - (paginaDelAllocActual * tamanioPagina - posicionAllocActual) +9 ; //el desplazamiento relativo a la pagina

        return generarDireccionLogica(generadorIdsPaginas, desplazamiento);
   }

}

void administrar_paginas(t_carpincho* carpincho){

        heapMetadata *anteultimoAlloc = list_get(carpincho->allocs, list_size(carpincho->allocs)-2);

        uint32_t posicionUltimoAlloc = anteultimoAlloc->nextAlloc;

        uint32_t cantidadDePaginasNecesarias = ceil(posicionUltimoAlloc/tamanioPagina);

        uint32_t cantidadDePaginasACrear =cantidadDePaginasNecesarias - list_size(carpincho->tabla_de_paginas);

        for (uint32_t i=0; i < cantidadDePaginasACrear; i++){

            t_pagina *pagina = malloc(sizeof(t_pagina));

            pagina->esNueva=true;
            pagina->id_pagina = generadorIdsPaginas();

            list_add(carpincho->tabla_de_paginas, pagina);    
        }

}

uint32_t asignarPaginas(t_carpincho* carpincho){
    
    if(strcmp(tipoAsignacion, "FIJA") == 0){

        bool noEstanAsignados(t_marco* marco){
            return marco->proceso_asignado == -1;
        }

        t_list *marcos_sin_asignar = list_filter(marcos, noEstanAsignados);

        t_list *marcos_a_asignar = list_take(marcos_sin_asignar; marcosMaximos);

        void marcarOcupados(t_marco marco){
            marco->estaLibre = false;
            marco->proceso_asignado = carpincho->id_carpincho;
        }

        list_iterate(marcos_a_asignar, marcarOcupados);

        escribir_marcos(marcos_a_asignar, carpincho); //aca escribir las paginas nuevas en los marcos_asignados. diferenciar los algoritmos en el caso de que haya que reemplazar. aca es la comunicacion con swap 

    }
    if(strcmp(tipoAsignacion, "DINAMICA"){
        
        //recorres todos los marcos hasta encontrar alguno(o mas) libres
    }

}

void escribir_marcos(t_list* marcos_a_asignar, t_carpincho* carpincho){

    void* stream_allocs = generar_stream_allocs(carpincho);

    void escribir_paginas_en_marcos(t_marco* marco){

        for(uint32_t i=0; i<list_size(carpincho->tabla_de_paginas); i++){
            t_pagina* pagina = list_get(carpincho->tabla_de_paginas, i);
            if(pagina->esNueva){
                memcpy(memoriaPrincipal + marco->comienzo, stream_allocs + (i*tamanioPagina), tamanioPagina);
            }
        }

    }

    list_iterate(marcos_a_asignar, (void*)escribir_paginas_en_marcos);

}

void* generar_stream_allocs(t_carpincho* carpincho){

    void* stream_allocs = malloc(tamanioPagina * list_size(carpincho->tabla_de_paginas));
 
        uint32_t desplazamiento =0, cantidadDeAllocs = list_size(carpincho->allocs);
        heapMetadata *allocActual;

            for(uint32_t i = 0; i < cantidadDeAllocs; i++){

            allocActual = list_get(carpincho->allocs, i);

            memcpy(stream_allocs + desplazamiento, allocActual, sizeof(heapMetadata));//llena con el primer alloc

            desplazamiento = allocActual->nextAlloc;

        }

    return stream_allocs;
}