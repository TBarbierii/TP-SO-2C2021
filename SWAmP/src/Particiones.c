#include "Swamp_lib.h"

/* ---------------- Particiones ---------------- */
particion* particion_nueva(int numero){
    particion* particion_nueva = malloc(sizeof(particion));
    particion_nueva->esta_libre = 1;
    particion_nueva->num_particion = numero;
    particion_nueva->hay_contenido = 0;
    particion_nueva->pid = -1;
    particion_nueva->num_pagina = -1;
    return particion_nueva;
}

particion* buscar_particion_libre_asignacion_dinamica(char* path_swap) {

    swap_files* archivoBuscado = encontrar_swap_file(path_swap);

    if(archivoBuscado != NULL) {
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
        if(particionActual->esta_libre == 0) {
            if(particionActual->pid == PID && particionActual->hay_contenido == 0){
                return 1;
            }
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

void vaciar_particion(particion* particion_a_vaciar, char* path_swap) {

    char caracter_vacio = '\0';
    int fd = open(path_swap, O_RDWR, (mode_t) 0777);
    truncate(path_swap, tamanio_swap);

    void* contenido_archivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(int i = 0; i < tamanio_pagina; i++) {
        memcpy(contenido_archivo + particion_a_vaciar->inicio_particion + i, &caracter_vacio, sizeof(char));
    }

    munmap(contenido_archivo, tamanio_swap);
    close(fd);
}

particion* buscar_particion_en_base_a_pagina(int numero_pagina, swap_files* archivo) {
    
    int paginaBuscadaEnParticion(particion* particionActual){
        if(particionActual->num_pagina == numero_pagina) {
            return 1;
        }
        return 0;
    } 

    return list_find(archivo->particiones_swap, paginaBuscadaEnParticion);
}


/* ----------------   Paginas  ----------------  */
int pagina_libre(particion* particion_nueva) {
    if(particion_nueva->esta_libre == 1) {
        return 1;
    }
    return 0;
}
