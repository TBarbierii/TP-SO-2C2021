#include "Swamp_lib.h"


/* ---------------- Auxiliares ---------------- */
int verificar_pid_en_swap_file(uint32_t PID, char* path_swap) {

    swap_files* archivoSwap = encontrar_swap_file(path_swap);

    bool seEncuentraElProcesoEnElArchivo(particion* particion){
        if(particion->pid == PID){
            return 1;
        }
        return 0;
    }

    return list_any_satisfy(archivoSwap->particiones_swap, seEncuentraElProcesoEnElArchivo);

}


swap_files* encontrar_swap_file(char* path_swap) {

    int encontrar_archivo(swap_files* archivo_swap) {
        if(strcmp(path_swap, archivo_swap->path) == 0) {
            return 1;
        }
        return 0;
    }

    swap_files* swap = list_find(lista_swap_files, encontrar_archivo);

    return swap;
}

swap_files* primerSwapFileDisponible(){
/* vamos a probar si podemos escribir sobre el primer archivo swapFile */
    return list_get(lista_swap_files,0);
}

swap_files* encontrar_swap_file_en_base_a_pid(uint32_t PID) {

    bool se_encuentra_pid_en_archivo(swap_files* archivo_swap){
        if(pid_se_encuentra_en_particion(archivo_swap, PID)){
            return 1;
        }
        return 0;
    }

    swap_files* archivo = list_find(lista_swap_files, se_encuentra_pid_en_archivo);

    return archivo;
}

int pid_se_encuentra_en_particion(swap_files* archivo_swap, uint32_t PID) {

    bool se_encuentra_pid_en_particion(particion* particion_encontrada){
        if(particion_encontrada->esta_libre == 0) {
            if(particion_encontrada->pid == PID){
                return 1;
            }
        }
        return 0;
    }

    particion* particion_encontrada = list_find(archivo_swap->particiones_swap, se_encuentra_pid_en_particion);

    if(particion_encontrada == NULL) {
        return 0;
    }
    return 1;
}

swap_files* buscar_archivo_con_mayor_espacio(){

    int tieneMasEspacio(swap_files* arch1, swap_files* arch2){
        return cantidad_frames_disponibles(arch1->path) >= cantidad_frames_disponibles(arch2->path);
            
    }


    list_sort(lista_swap_files, (void*)tieneMasEspacio);
    //va a poner a los archivos mas grandes al comienzo de todo
    
    //tomamos el primero de la lista
    return list_get(lista_swap_files,0);


}

int cantidad_frames_disponibles(char* path_swap) {
    
    swap_files* archivoSwap = encontrar_swap_file(path_swap);
    t_list* particiones_libres;

    if(archivoSwap != NULL){
        //aca vamos a poner las particiones libres de un archivo
        particiones_libres = list_filter(archivoSwap->particiones_swap, pagina_libre); 
        int cantidad_frames =  list_size(particiones_libres);
        list_destroy(particiones_libres);
        return cantidad_frames;
    }
    return 0;
}