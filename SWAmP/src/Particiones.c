#include "Swamp_lib.h"

/* ---------------- Particiones ---------------- */
particion* particion_nueva(int numero){
    particion* particion_nueva = malloc(sizeof(particion));
    particion_nueva->esta_libre = 1;
    particion_nueva->num_particion = numero;
    particion_nueva->hay_contenido = 0;
    return particion_nueva;
}

particion* buscar_particion_libre_asignacion_dinamica(char* path_swap) {

    swap_files* archivoBuscado = encontrar_swap_file(path_swap);

    if(archivoBuscado != NULL) {
        //log_info(logger_swamp, "Se encontro el archivo buscado");
        particion* particion_nueva = list_find(archivoBuscado->particiones_swap, pagina_libre);
        if(particion_nueva != NULL) {
            return particion_nueva;
        }
    }
    
    return NULL;
}

particion* primer_particion_libre(swap_files* archivo){
    return list_find(archivo->particiones_swap, pagina_libre);
}

particion* primer_particion_disponible_para_escribir(swap_files* archivo, int PID){

    int particionDisponible(particion* particionActual){
        if(particionActual->pid == PID && particionActual->hay_contenido == 0){
            return 1;
        }
        return 0;
    } 

    return list_find(archivo->particiones_swap, particionDisponible);

}


/* ----------------   Paginas  ----------------  */
int pagina_libre(particion* particion_nueva) {
    if(particion_nueva->esta_libre == 1) {
        return 1;
    }
    return 0;
}