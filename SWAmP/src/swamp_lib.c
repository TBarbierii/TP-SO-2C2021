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

    char caracter_llenado = '\0';
    

    while(!list_is_empty(archivos_swap)) {

        char* path_swap = (char*) list_remove(archivos_swap, 0);
        int sizeNombre = string_length(path_swap)+1;

        swap_files* nuevoArchivo = malloc(sizeof(swap_files));
        nuevoArchivo->path = (char*) malloc(sizeNombre * sizeof(char));
        strcpy(nuevoArchivo->path, path_swap);

        int fd = open(path_swap, O_CREAT | O_RDWR, (mode_t) 0777);
        
        if(access(path_swap, F_OK) == -1) {
		
        truncate(path_swap, tamanio_swap);
        void* contenidoArchivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        memcpy(contenidoArchivo, &(caracter_llenado), sizeof(char));
        munmap(contenidoArchivo, tamanio_swap);
        close(fd);
    
    }

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
        //log_info(logger_swamp, "Se encontro el archivo buscado");
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

void escribirContenido(void* mensajeAEscribir, int id_pagina, int PID, t_log* logger){

    /*aca vamos a verificar si hay un archivo donde se encuentre el pid, y sino vamos a buscar directamente el archivo con mas espacio */
    // Estaria bien utilizar list_get_maximum aca?
    //swap_files* archivo_swap = list_get_maximum(lista_swap_files, cantidad_frames_disponibles);
    
    swap_files* archivoAEscribir = primerSwapFileDisponible();

    particion* particion_buscada = buscar_particion_libre_asignacion_dinamica(archivoAEscribir->path);

    escribirContenidoSobreElArchivo(mensajeAEscribir, particion_buscada->num_particion, id_pagina, PID, archivoAEscribir->path, logger);

    //int tiene_paginas_en_swap_file = verificar_pid_en_swap_file(pid, archivo_swap->path);


    /*if(tipo_asignacion) {
        particion* marco = buscar_particion_libre_asignacion_dinamica(archivo_swap->path);
    }else{

    }
    
    if(tiene_paginas_en_swap_file == 1) {
        log_info(logger, "El proceso tiene paginas en este archivo de swap");
        escribirContenidoSobreElArchivo(mensajeAEscribir, marco, int pagina, int pid, char* nombreArchivo, logger);
    }else{

    }
    */





}

/* una funcion el tema de asigancion */




void escribirContenidoSobreElArchivo(void* mensajeAEscribir, int marco, int pagina, int pid, char* nombreArchivo, t_log* logger){

    swap_files* archivoAEscribir = encontrar_swap_file(nombreArchivo);

    char* contenido = (char *) mensajeAEscribir; 
    int size = string_length(contenido);

    if(archivoAEscribir != NULL){

        int buscarParticionDeseada(particion* particionBuscada){
            if(particionBuscada->num_particion == marco){
                return 1;
            }
            return 0;
        }

        particion* particionAmodificar = list_find(archivoAEscribir->particiones_swap, buscarParticionDeseada);

        if(particionAmodificar != NULL){
            
            particionAmodificar->esta_libre = 0;
            particionAmodificar->hay_contenido = 1;
            particionAmodificar->pid = pid;
            particionAmodificar->num_pagina = pagina;

            log_info(logger,"Se guardo el contenido en el archivo: %s", archivoAEscribir->path);
            log_info(logger,"Se escribe sobre la particion: %i", particionAmodificar->num_particion);
            
            int fd = open(archivoAEscribir->path, O_RDWR, (mode_t) 0777);
            truncate(archivoAEscribir->path, tamanio_swap);
            void* contenidoArchivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            if(size < tamanio_pagina){ //podria pasar que lo que nos manda memoria, ocupe menos tamaño que una pagina el contenido, por lo tanto vamos a guardar el tamaño solamente
                
                memcpy(contenidoArchivo  + (particionAmodificar->inicio_particion), contenido, size);
            
            }else{

                memcpy(contenidoArchivo  + (particionAmodificar->inicio_particion), contenido, tamanio_pagina);
                
            }   



            munmap(contenidoArchivo, tamanio_swap);
            close(fd);

            log_info(logger,"Se guardo el contenido correctamente");

        }else{
            log_info(logger,"No se encontro la siguiente particion");
        }
    }else{
        log_info(logger,"No se encontro el archivo ");
    }
    
}

void leer_contenido(uint32_t PID, uint32_t id_pagina, int conexion, t_log* logger) {
    
    swap_files* archivo_swap = encontrar_swap_file_en_base_a_pid(PID);
    if(archivo_swap == NULL) {
        log_error(logger, "No se encontro el archivo");
    }else{

        char* contenido_a_leer = malloc(tamanio_pagina);

        int fd = open(archivo_swap->path, O_RDWR, (mode_t) 0777);

        int buscar_pagina_en_particion(particion* particion_buscada){
            if(particion_buscada->esta_libre == 0) {
                if(particion_buscada->num_pagina == id_pagina && particion_buscada->pid == PID){
                    return 1;
                }
            }
            return 0;
        }

        printf("Sacar este printf\n");

        void* contenido_archivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        particion* particion_a_leer = list_find(archivo_swap->particiones_swap, buscar_pagina_en_particion);
    
        if(particion_a_leer != NULL) {
            memcpy(contenido_a_leer, contenido_archivo + particion_a_leer->inicio_particion, tamanio_pagina);
            //log_info(logger, "El contenido leido es %s", contenido_a_leer);
            enviar_pagina(contenido_a_leer, conexion);
            particion_a_leer->hay_contenido = 0;
            if(tipo_asignacion == 0) {
                particion_a_leer->esta_libre = 1;
            }
        }

        munmap(contenido_archivo, tamanio_swap);
        close(fd);
        free(contenido_a_leer);
    }
}

void* encontrar_swap_file_en_base_a_pid(uint32_t PID) {

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

    printf("Sacar este printf\n");

    particion* particion_encontrada = list_find(archivo_swap->particiones_swap, se_encuentra_pid_en_particion);

    if(particion_encontrada == NULL) {
        return 0;
    }
    return 1;
}