#include "swamp_lib.h"



/* Inicializacion */
void obtener_valores_config(t_config* config_actual, t_log* logger){

   

    ip_swap = config_get_string_value(config_actual, "IP");
    puerto_swap = config_get_string_value(config_actual, "PUERTO");
    tamanio_swap = config_get_int_value(config_actual, "TAMANIO_SWAP");
    tamanio_pagina = config_get_int_value(config_actual, "TAMANIO_PAGINA");
    char** file_swap = config_get_array_value(config_actual, "ARCHIVOS_SWAP");
    marcos_maximos = config_get_int_value(config_actual, "MARCOS_MAXIMOS");
    retardo_swap = config_get_int_value(config_actual, "RETARDO_SWAP");
    
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

    char* caracter_llenado = '\0';
    

    while(!list_is_empty(archivos_swap)) {

        char* path_swap = (char*) list_remove(archivos_swap, 0);
        int sizeNombre = string_length(path_swap)+1;

        swap_files* nuevoArchivo = malloc(sizeof(swap_files));
        nuevoArchivo->path = (char*) malloc(sizeNombre * sizeof(char));
        strcpy(nuevoArchivo->path, path_swap);

        int fd = open(path_swap, O_CREAT | O_RDWR, (mode_t) 0777);
        
        truncate(path_swap, tamanio_swap);
        
        void* contenidoArchivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        memcpy(contenidoArchivo, &caracter_llenado, sizeof(char));
    
        munmap(contenidoArchivo, tamanio_swap);
        
        close(fd);
        
        nuevoArchivo->particiones_swap = crear_lista_particiones((tamanio_swap/tamanio_pagina));
        list_add(lista_swap_files, nuevoArchivo);

        log_info(logger,"Se crea el archivo SWAP en el path: %s y de size: %d",nuevoArchivo->path, tamanio_swap);
        
        free(path_swap);

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



/*  Particiones  */

particion* particion_nueva(int numero){
    particion* particion_nueva = malloc(sizeof(particion));
    particion_nueva->esta_libre = 1;
    particion_nueva->num_particion = numero;
    particion_nueva->hay_contenido = 0;
    return particion_nueva;
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

particion* buscar_particion_libre_asignacion_dinamica(char* path_swap) {

    swap_files* archivoBuscado = encontrar_swap_file(path_swap);

    if(archivoBuscado != NULL) {
        log_info(logger_swamp, "Se encontro el archivo buscado");
        particion* particion_nueva = list_find(archivoBuscado->particiones_swap, pagina_libre);
        if(particion_nueva != NULL) {
            return particion_nueva;
        }
    }
    
    return NULL;
}


int cantidad_frames_disponibles(char* path_swap) {
    
    swap_files* archivoSwap = encontrar_swap_file(path_swap);
    t_list* particiones_libres;

    //aca vamos a poner las particiones libres de un archivo
    particiones_libres = list_filter(archivoSwap->particiones_swap, pagina_libre); 

    int cantidad_frames =  list_size(particiones_libres);
    
    list_destroy(particiones_libres);

    return cantidad_frames;
}



/*    Paginas    */
int pagina_libre(particion* particion_nueva) {
    if(particion_nueva->esta_libre == 1) {
        return 1;
    }
    return 0;
}




/*  Asignaciones  */
void manejar_asignacion() {

	if(tipo_asignacion == 1) {
        log_info(logger_swamp, "Tipo de asignacion a utilizar es FIJA");
    }else{
        log_info(logger_swamp, "Tipo de asignacion a utilizar es DINAMICA");
    }
}





/*   Auxiliares   */
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



/* vamos a probar si podemos escribir sobre el primer archivo swapFile */
swap_files* primerSwapFileDisponible(){

    return list_get(lista_swap_files,0);
}


/* Finalizacion */

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