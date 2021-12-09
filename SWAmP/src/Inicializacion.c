#include "Swamp_lib.h"



/* ---------------- Inicializacion ---------------- */
void obtener_valores_config(t_config* config_actual, t_log* logger){

    ip_swap = config_get_string_value(config_actual, "IP");
    puerto_swap = config_get_string_value(config_actual, "PUERTO");
    tamanio_swap = config_get_int_value(config_actual, "TAMANIO_SWAMP");
    tamanio_pagina = config_get_int_value(config_actual, "TAMANIO_PAGINA");
    char** file_swap = config_get_array_value(config_actual, "ARCHIVOS_SWAMP");
    marcos_maximos = config_get_int_value(config_actual, "MARCOS_POR_CARPINCHO");
    retardo_swap = config_get_int_value(config_actual, "RETARDO_SWAMP");
    
    t_list* archivos_swap = list_create();
    int contador = 0;

    while(file_swap[contador] != NULL) {
        list_add(archivos_swap, file_swap[contador]);
        contador++;
    }

    crear_archivos_swap(archivos_swap, (tamanio_swap/tamanio_pagina), logger);
    
    log_info(logger, "Se han creado todos los archivos");

    free(file_swap);
    list_destroy(archivos_swap);
   
}

void crear_archivos_swap(t_list* archivos_swap, int cantidad_particiones, t_log* logger) {

    char caracter_llenado = '\0';
    int id_swap = 0;    

    while(!list_is_empty(archivos_swap)) {

        char* path_swap = (char*) list_remove(archivos_swap, 0);
        int sizeNombre = string_length(path_swap)+1;

        swap_files* nuevoArchivo = malloc(sizeof(swap_files));
        nuevoArchivo->id = id_swap;
        nuevoArchivo->path = (char*) malloc(sizeNombre * sizeof(char));
        strcpy(nuevoArchivo->path, path_swap);

        int fd = open(path_swap, O_CREAT | O_RDWR, (mode_t) 0777);
        
        if(access(path_swap, F_OK) != -1) {
            truncate(path_swap, tamanio_swap);
            void* contenidoArchivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            for(int i = 0; i < tamanio_swap; i++) {
                memcpy(contenidoArchivo + i, &(caracter_llenado), sizeof(char));
            }
            munmap(contenidoArchivo, tamanio_swap);
        }

        close(fd);
        
        nuevoArchivo->particiones_swap = crear_lista_particiones((tamanio_swap/tamanio_pagina));
        list_add(lista_swap_files, nuevoArchivo);

        log_debug(logger,"\n     Se crea el archivo SWAP:\n       Path: %s\n       Size: %d\n       Cantidad de frames disponibles: %i\n       ID: %i",nuevoArchivo->path, tamanio_swap, cantidad_frames_disponibles(nuevoArchivo), nuevoArchivo->id);
        
        free(path_swap);
        id_swap++;
    }
    
}

t_list* crear_lista_particiones(int cantidad_particiones){

    t_list* lista_particiones = list_create();
    int offset_particion = 0;

    for(int i=0; i < cantidad_particiones; i++){
        particion* particion_swap = particion_nueva(i);
        particion_swap->inicio_particion = offset_particion;
        offset_particion += tamanio_pagina;
        list_add(lista_particiones, particion_swap);
    }
    return lista_particiones;
}
