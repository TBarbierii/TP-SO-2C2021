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

void limpiar_marcos_de_proceso(int PID) {

    swap_files* archivo_swap = encontrar_swap_file_en_base_a_pid(PID);
    t_list* marcos_de_proceso;

    if(archivo_swap != NULL) {
        bool encontrar_marcos_proceso(particion* particion_proceso){
            return (particion_proceso->pid == PID);
        }

        marcos_de_proceso = list_filter(archivo_swap->particiones_swap, encontrar_marcos_proceso);

        while(! (list_is_empty(marcos_de_proceso))) {
            particion* particion_a_vaciar = list_remove(marcos_de_proceso, 0);
            particion_a_vaciar->esta_libre = 1;
            particion_a_vaciar->hay_contenido = 0;
            vaciar_particion(particion_a_vaciar, archivo_swap->path);
        }
        list_destroy(marcos_de_proceso);
        log_error(logger_swamp, "Proceso %i finalizado con exito", PID);
    }else{
        log_error(logger_swamp, "EL PROCESO %i NO GUARDO NADA EN SWAP", PID);
    }
}
