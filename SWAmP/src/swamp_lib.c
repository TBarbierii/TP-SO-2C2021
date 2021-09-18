#include "swamp.h"
#include "swamp_lib.h"

void obtenerValoresDelConfig(t_config* configActual){

    int contador = 0;

    ip_swap = config_get_string_value(configActual, "IP");
    puerto_swap = config_get_int_value(configActual, "PUERTO");
    tamanio_swap = config_get_int_value(configActual, "TAMANIO_SWAP");
    tamanio_pagina = config_get_int_value(configActual, "TAMANIO_PAGINA");
    char** file_swap = config_get_array_value(configActual, "ARCHIVOS_SWAP");
    marcos_maximos = config_get_int_value(configActual, "MARCOS_MAXIMOS");
    retardo_swap = config_get_int_value(configActual, "RETARDO_SWAP");
    
    t_list* archivos_swap = list_create();

    while(file_swap[contador] != NULL) {
        list_add(archivos_swap, file_swap[contador]);
        contador++;
    }

    crear_archivos_swap(archivos_swap, (tamanio_swap/tamanio_pagina));

    list_destroy(archivos_swap);
    free(file_swap); //sobre esto, nose si se deberia hacer un free, xq creo que eso se hace recien cuando haga el config_Destroy()
}

void crear_archivos_swap(t_list* archivos_swap, int cantidadParticiones) {

//    struct stat* sb;

    char caracter_llenado = '\0';

    while(! list_is_empty(archivos_swap)) {

        swap_files* nuevo_swap = malloc(sizeof(swap_files));

        char* path_swap = (char*) list_remove(archivos_swap, 0);

        int fd = open(path_swap, O_CREAT | O_RDWR, (mode_t) 0777);

        truncate(path_swap, tamanio_swap);

        nuevo_swap->swap_file = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
/*
        int estado = stat(nuevo_swap->path, sb);

        if(estado != -1) {
            log_info(logger_swamp, "Archivos de swap creados y de tamaño: %i", tamanio_swap);
        }else{
            exit(-1);
        }
*/
        memcpy(nuevo_swap->swap_file, &caracter_llenado, sizeof(char));


        /* faltaria crear las particiiones para el archivo swap */
        nuevo_swap->particiones_swap = crearListaDeParticiones(cantidadParticiones);

    }

}

t_list* crearListaDeParticiones(int cantidadParticiones){

    t_list* listaParticiones = list_create();

    for(int i=0; i < cantidadParticiones ; i++){
        list_add(listaParticiones, particionNueva(i));
    }
    return listaParticiones;
}


particion* particionNueva(int numero){
    particion* particionNueva = malloc(sizeof(particion));
    particionNueva->esta_libre = 1;
    particionNueva->num_particion = numero;
    return particionNueva;
}

