#include "Swamp_lib.h"


/* ---------------- Finalizacion ---------------- */
void destruirArchivosSwapFiles(){

    while(!list_is_empty(lista_swap_files)){
        swap_files* archivoSwap = list_remove(lista_swap_files,0);
        free(archivoSwap->path);
        eliminarParticiones(archivoSwap->particiones_swap);
        free(archivoSwap);
    }

}

void eliminarParticiones(t_list* listaParticiones){
    
    while(!list_is_empty(listaParticiones)){
        particion* particionAEliminar = list_remove(listaParticiones,0);
        free(particionAEliminar);
    }
    list_destroy(listaParticiones);
}

void limpiar_marcos_de_proceso(int PID, char* path_swap) {

    swap_files* archivo_swap = encontrar_swap_file(path_swap);
    path_swap = archivo_swap->path;
    t_list* marcos_de_proceso = list_create();

    int cantidad_marcos_asignados = list_size(marcos_de_proceso);
    int offset_particion = 0;
    char caracter_nulo = '\0';

    int fd = open(path_swap, O_RDWR, (mode_t) 0777);
    void* contenido_archivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    truncate(path_swap, tamanio_swap);

    if(tipo_asignacion == 1) {
        for(int i = 0; i < cantidad_marcos_asignados; i++) {
            particion* particion_a_vaciar = list_get(marcos_de_proceso, i);
            particion_a_vaciar->esta_libre = 1;
            particion_a_vaciar->hay_contenido = 0;
            particion_a_vaciar->num_pagina = 0; //Que deberia poner aca?
            particion_a_vaciar->pid = 0;        //Que deberia poner aca?
            memcpy(contenido_archivo + offset_particion, &caracter_nulo, sizeof(char));
            offset_particion = particion_a_vaciar->inicio_particion + tamanio_pagina;
        }
    }else{

    }
    munmap(contenido_archivo, tamanio_swap);
    close(fd);
    list_destroy(marcos_de_proceso);
}