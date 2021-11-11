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
        if(cantidad_frames_disponibles(arch1) == cantidad_frames_disponibles(arch2)){
            return arch1->id < arch2->id;
        }
        return cantidad_frames_disponibles(arch1) > cantidad_frames_disponibles(arch2);
    }

    
    list_sort(lista_swap_files, tieneMasEspacio);
    //va a poner a los archivos mas grandes al comienzo de todo
    
    //tomamos el primero de la lista
    return list_get(lista_swap_files,0);
}

int cantidad_frames_disponibles(swap_files* archivoSwap) {
    
    
    t_list* particiones_libres;

    if(archivoSwap != NULL){
        //aca vamos a poner las particiones libres de un archivo
        particiones_libres = list_filter(archivoSwap->particiones_swap, pagina_libre); 
        int cantidad_frames = list_size(particiones_libres);
        list_destroy(particiones_libres);
        return cantidad_frames;
    }
    
    return 0;
}

int cantidad_frames_disponibles_para_proceso(swap_files* archivo_swap, int PID) {

    int cantidad_frames = 0;

    if(archivo_swap != NULL) {
        if(tipo_asignacion == 1) {

            t_list* particiones_libres_de_proceso;
            
            bool particionesDisponiblesParaProceso(particion* particion_para_proceso) {
                if(particion_para_proceso->esta_libre == 0 && particion_para_proceso->hay_contenido == 0) {
                    if(particion_para_proceso->pid == PID) {
                        return 1;
                    }
                }
                return 0;
            }

            particiones_libres_de_proceso = list_filter(archivo_swap->particiones_swap, particionesDisponiblesParaProceso);
            cantidad_frames = list_size(particiones_libres_de_proceso);
            list_destroy(particiones_libres_de_proceso);
            log_info(logger_swamp, "Cantidad de frames disponibles para el proceso %i: %i", PID, cantidad_frames);
            return cantidad_frames;

        }else if(tipo_asignacion == 0) {
            cantidad_frames = cantidad_frames_disponibles(archivo_swap);
            log_info(logger_swamp, "Cantidad de frames disponibles para el proceso %i: %i", PID, cantidad_frames);
            return cantidad_frames;
        }
    }
    log_info(logger_swamp, "No hay espacio suficiente en el archivo %s para el proceso %i", archivo_swap->path, PID);

    return 0;
}
