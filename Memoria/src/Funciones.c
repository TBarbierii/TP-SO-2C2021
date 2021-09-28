#include "Memoria.h"


void administrar_allocs(t_memalloc alloc){

    bool buscarCarpincho(t_carpincho* c){
		return c->id_carpincho == alloc.pid;
	}
    //semaforo?a
    t_carpincho* carpincho = list_find(carpinchos, (void*)buscarCarpincho);
    //semaforo?
    //si no lo encuentra crearlo

    buscar_o_agregar_espacio(carpincho, alloc.tamanio); //aca se crearian las paginas en el carpincho. Solo sirven para calcular la DF creo

    uint32_t marco = escribir_en_memoria(carpincho); //aca se graba en memoria los allocs reservados. Devuelve el id_marco



}


void buscar_o_agregar_espacio(t_carpincho* carpincho, uint32_t tamanio){ //ver que retornar

    if(list_size(carpincho->allocs == 0)){
    heapMetadata* alloc = malloc(sizeof(heapMetadata));
    alloc->isFree = true;
    alloc->prevAlloc = NULL;
    
        if(tamanio + 9 != tamanioPagina){// crea otro alloc si sobra espacio en la pagina o si se pasa
            
            alloc->nextAlloc = 9 + tamanio;
            heapMetadata* nextAlloc = malloc(sizeof(heapMetadata));
            nextAlloc->isFree = true;
            nextAlloc->prevAlloc = alloc->nextAlloc - 9 + tamanio;
            nextAlloc->nextAlloc = NULL;
            list_add(carpincho->allocs, alloc);
            list_add(carpincho->allocs, nextAlloc);
        }else{//aca entro justo y no se crea otro alloc
        
            alloc->nextAlloc = NULL;
            list_add(carpincho->allocs, alloc);
        }
    
    uint32_t cantidad_paginas_necesarias = ceil((9 + tamanio)/tamanioPagina);

        for(int i=0; i<cantidad_paginas_necesarias; i++){    
            t_pagina* nuevaPagina = malloc(sizeof(t_pagina));
            nuevaPagina->esNueva = true;
            nuevaPagina->id_pagina = i; //generar ids unicos
            list_add(carpincho->tabla_de_paginas, nuevaPagina);
        }
    }else{//hasta aca seria si un carpincho reserva un alloc por primera vez
    //recorrer los allocs hasta encontrar espacio (teniendo en cuenta que si se tiene que se separar tiene que entrar el heap), si no hay lugar agregar al final que deberia haber uno en NULL (fusionar si es necesario) y/o agregar una pagina nueva
    uint32_t espacio_encontrado = 0;

    void verificar_espacios(heapMetadata* alloc){

        if(alloc->isFree){
        espacio_encontrado = alloc->nextAlloc - espacio_encontrado;
        }

        if(espacio_encontrado - 9 == tamanio || espacio_encontrado - 9 > tamanio + 9){//dios que poronga esto lo planearon con el orto. podria quedar un alloc de 0B?
    
        }
    
    }

    list_iterate(carpincho->allocs, (void*)verificar_espacios);
    //oljsdhgfvasdhgañnb no sirve un list iterate creo
    }

}

uint32_t escribir_en_memoria(t_carpincho* carpincho){
    //verificar los marcos asignados del proceso? para ver si que no se pase de los maximos
    //aca entra lo de asignacion
    return 0;
}