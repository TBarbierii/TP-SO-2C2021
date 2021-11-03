#include "Swamp_lib.h"



/* ---------------- Inicializacion ---------------- */
void obtener_valores_config(t_config* config_actual, t_log* logger){

    ip_swap = config_get_string_value(config_actual, "IP");
    puerto_swap = config_get_string_value(config_actual, "PUERTO");
    tamanio_swap = config_get_int_value(config_actual, "TAMANIO_SWAP");
    tamanio_pagina = config_get_int_value(config_actual, "TAMANIO_PAGINA");
    char** file_swap = config_get_array_value(config_actual, "ARCHIVOS_SWAP");
    marcos_maximos = config_get_int_value(config_actual, "MARCOS_POR_CARPINCHO");
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
        
        truncate(path_swap, tamanio_swap);
        void* contenidoArchivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        
        if(access(path_swap, F_OK) != -1) {
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


/*
// ---------------- Particiones ---------------- /
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


// ----------------   Paginas  ----------------  /
int pagina_libre(particion* particion_nueva) {
    if(particion_nueva->esta_libre == 1) {
        return 1;
    }
    return 0;
}
*/
/*
/ ---------------- Asignaciones ---------------- /
void manejar_asignacion() {

	if(tipo_asignacion == 1) {
        log_info(logger_swamp, "Tipo de asignacion a utilizar es FIJA");
    }else{
        log_info(logger_swamp, "Tipo de asignacion a utilizar es DINAMICA");
    }
}

int asignacion_dinamica(int pid, swap_files* archivo){

    particion* particionActual =  primer_particion_libre(archivo);
    if(particionActual != NULL){
        particionActual->esta_libre = 0;
        particionActual->pid = pid;
        return 1;
    }
    return 0;
}

int asignar_marcos_maximos(int pid, swap_files* archivo ){

    if(cantidad_frames_disponibles(archivo->path) < marcos_maximos){
        return 0;
    }else{
        asignar_marcos_proceso(pid,archivo);
        return 1;        
    }
}

void asignar_marcos_proceso(int pid,swap_files* archivo){

    
    for(int i=0; i < marcos_maximos; i++){ //busca las primeras n(cantidad) paginas que esten libres y se las asigna de una ya 
        particion* particionActual =  primer_particion_libre(archivo);
        particionActual->esta_libre = 0;
        particionActual->hay_contenido = 0;
        particionActual->pid = pid;
    }
}
*/

/*
// ---------------- Escritura ---------------- /
void escribirContenido(void* mensajeAEscribir, int id_pagina, int PID, t_log* logger){


    swap_files* archivoAEscribir = escritura_en_archivo_en_base_tipo_asignacion(PID, logger);

    escribirContenidoSobreElArchivo(mensajeAEscribir, id_pagina, PID, archivoAEscribir->path, logger);


}

void escribirContenidoSobreElArchivo(void* mensajeAEscribir, int pagina, int pid, char* nombreArchivo, t_log* logger){

    swap_files* archivoAEscribir = encontrar_swap_file(nombreArchivo);

    char* contenido = (char *) mensajeAEscribir; 
    int size = string_length(contenido);

    if(archivoAEscribir != NULL){

        particion* particionAmodificar = primer_particion_disponible_para_escribir(archivoAEscribir,pid);

        if(particionAmodificar != NULL){
            particionAmodificar->hay_contenido = 1;
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

swap_files* escritura_en_archivo_en_base_tipo_asignacion(int pid, t_log* logger){

    swap_files* archivo = encontrar_swap_file_en_base_a_pid(pid);

    if(archivo == NULL){ //aca lo asignamos a los correspondientes marcos solamente
        swap_files* archivoConMasEspacio = buscar_archivo_con_mayor_espacio();

        if(tipo_asignacion == 1){

            int retorno= asignar_marcos_maximos(pid, archivoConMasEspacio);
            if(retorno == 1){
                log_info(logger,"Todo bien se realizo con la asignacion Fija");
            }else{
                log_info(logger,"Fallo en la asignacion Fija");
            }

        }else{
            int retorno = asignacion_dinamica(pid, archivoConMasEspacio);
            if(retorno == 1){
                log_info(logger,"Todo bien se realizo con la asignacion Dinamica");
            }else{
                log_info(logger,"Fallo en la asignacion Dinamica");
            }
        }
        return archivoConMasEspacio;
    }
    return archivo;
}


/ ---------------- Lectura ---------------- /
void leer_contenido(uint32_t PID, uint32_t id_pagina, int conexion, t_log* logger){
    
    swap_files* archivo_swap = encontrar_swap_file_en_base_a_pid(PID);
    if(archivo_swap == NULL) {
        log_error(logger, "No se encontro el archivo");
    }else{

        void* contenido_a_leer = malloc(tamanio_pagina);

        int fd = open(archivo_swap->path, O_RDWR, (mode_t) 0777);
        truncate(archivo_swap->path, tamanio_swap);
        int buscar_pagina_en_particion(particion* particion_buscada){
            if(particion_buscada->esta_libre == 0) {
                if(particion_buscada->num_pagina == id_pagina && particion_buscada->pid == PID){
                    return 1;
                }
            }
            return 0;
        }

        void* contenido_archivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        particion* particion_a_leer = list_find(archivo_swap->particiones_swap, buscar_pagina_en_particion);
    
        if(particion_a_leer != NULL) {
            memcpy(contenido_a_leer, contenido_archivo + particion_a_leer->inicio_particion, tamanio_pagina);
            
            //ya que sino no nos deja loggearlo 
            char* contenidoParaLoggear= malloc(tamanio_pagina+1);
            char valorsitoParaString = '\0';
            memcpy(contenidoParaLoggear, contenido_a_leer, tamanio_pagina);
            memcpy(contenidoParaLoggear + tamanio_pagina, &(valorsitoParaString),1);
            log_info(logger,"Se leyo el contenido del archivo: %s", archivo_swap->path);
            log_info(logger,"El contenido leido es %s", contenidoParaLoggear);
            free(contenidoParaLoggear);

            enviar_pagina(contenido_a_leer, conexion);
            particion_a_leer->hay_contenido = 0;
            if(tipo_asignacion == 0) {
                particion_a_leer->esta_libre = 1;
            }
        }

        munmap(contenido_archivo, tamanio_swap);
        close(fd);
        //este contenido deberia enviarse
        enviar_pagina(contenido_a_leer,conexion);
        free(contenido_a_leer);
    }
}
*/
/*
/ ---------------- Auxiliares ---------------- /
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
/ vamos a probar si podemos escribir sobre el primer archivo swapFile /
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
*/

/*
/ ---------------- Finalizacion ---------------- /
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

void limpiar_marcos_de_proceso(int PID, char* path_swap) {

    swap_files* archivo_swap = encontrar_swap_file(path_swap);
    path_swap = archivo_swap->path;
    t_list* marcos_de_proceso = list_create();

    int cantidad_marcos_asignados = list_size(marcos_de_proceso);
    int offset_particion = 0;
    char caracter_nulo = '\0';

    int fd = open(path_swap, O_RDWR, (mode_t) 0777);
    void* contenido_archivo = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    truncate(path_swap, tamanio_swap);

    if(tipo_asignacion == 1) {
        for(int i = 0; i < cantidad_marcos_asignados; i++) {
            particion* particion_a_vaciar = list_get(marcos_de_proceso, i);
            particion_a_vaciar->esta_libre = 1;
            particion_a_vaciar->hay_contenido = 0;
            particion_a_vaciar->num_pagina = 0; //Que deberia poner aca?
            particion_a_vaciar->pid = 0;        //Que deberia poner aca?
            memcpy(contenido_archivo + offset_particion, &caracter_nulo, sizeof(char));
            offset_particion = particion_a_vaciar->inicio_particion + tamanio_pagina;
        }
    }else{

    }
    munmap(contenido_archivo, tamanio_swap);
    close(fd);
    list_destroy(marcos_de_proceso);
}
*/