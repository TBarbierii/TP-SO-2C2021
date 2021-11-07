#include "Swamp_lib.h"

/* ---------------- Particiones ---------------- */
particion* particion_nueva(int numero){
    particion* particion_nueva = malloc(sizeof(particion));
    particion_nueva->esta_libre = 1;
    particion_nueva->num_particion = numero;
    particion_nueva->hay_contenido = 0;
    particion_nueva->pid = -1;
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

particion* particion_disponible_para_sobreescribir(swap_files* archivo, int PID, int id_pagina){

    int particionDisponibleParaSobreescribir(particion* particionActual){
        if(particionActual->esta_libre == 0) {
            if(particionActual->pid == PID && particionActual->hay_contenido == 1){
                return particionActual->num_pagina == id_pagina;
            }
        }
        return 0;
    } 

    return list_find(archivo->particiones_swap, particionDisponibleParaSobreescribir);
}

void vaciar_particion(int numero_particion, char* path_swap) {

    swap_files* archivo_swap = encontrar_swap_file(path_swap);
    char caracter_vacio = '\0';
    int fd = open(archivo_swap->path, O_RDWR, (mode_t) 0777);
    truncate(archivo_swap->path, tamanio_swap);

    void* contenido_archivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    particion* particion_a_vaciar = buscar_particion(numero_particion, archivo_swap);

    for(int i = 0; i < tamanio_pagina; i++) {
        memcpy(contenido_archivo + particion_a_vaciar->inicio_particion, &caracter_vacio, sizeof(char));
    }
    
    particion_a_vaciar->hay_contenido = 0;
    
    if(tipo_asignacion == 0) {
        particion_a_vaciar->esta_libre = 1;
    }

    munmap(contenido_archivo, tamanio_swap);
    close(fd);
}

particion* buscar_particion(int numero_particion, swap_files* archivo) {
    
    int particionBuscada(particion* particionActual){
        if(particionActual->num_particion == numero_particion) {
            return 1
        }
        return 0;
    } 

    return list_find(archivo->particiones_swap, particionBuscada);
}


/* ----------------   Paginas  ----------------  */
int pagina_libre(particion* particion_nueva) {
    if(particion_nueva->esta_libre == 1) {
        return 1;
    }
    return 0;
}
