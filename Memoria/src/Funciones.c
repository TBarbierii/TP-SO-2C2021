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

        return 9; //el espacio del primer alloc siempre va a empezar en 9

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
        
        //todo esto que sigue es para calcualr el desplazamiento de donde empieza el espacio libre
        uint32_t posicionAllocActual = allocSiguiente->prevAlloc;
        uint32_t paginaDelAllocActual;

        for(uint32_t i=1; i <= list_size(carpincho->tabla_de_paginas); i++){

            if(posicionAllocActual < tamanioPagina * i){
            paginaDelAllocActual = i;
            break; 
            }    
        }

        return tamanioPagina - (paginaDelAllocActual * tamanioPagina - posicionAllocActual) +9 ; //el desplazamiento relativo a la pagina

   }

}

uint32_t administrar_paginas(t_carpincho* carpincho){

        heapMetadata *anteultimoAlloc = list_get(carpincho->allocs, list_size(carpincho->allocs)-2);

        uint32_t posicionUltimoAlloc = anteultimoAlloc->nextAlloc;

        uint32_t cantidadDePaginasNecesarias = ceil(posicionUltimoAlloc/tamanioPagina);

        uint32_t cantidadDePaginasACrear =cantidadDePaginasNecesarias - list_size(carpincho->tabla_de_paginas);

        //aca quede. crear paginas y agregar a tabla de paginas 

}

uint32_t escribir_en_memoria(t_carpincho* carpincho){
    //verificar los marcos asignados del proceso? para ver si que no se pase de los maximos
    //aca entra lo de asignacion
    return 0;
}