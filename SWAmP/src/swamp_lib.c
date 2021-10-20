#include "swamp.h"
#include "swamp_lib.h"

void obtener_valores_config(t_config* config_actual){

    int contador = 0;

    ip_swap = config_get_string_value(config_actual, "IP");
    puerto_swap = config_get_string_value(config_actual, "PUERTO");
    tamanio_swap = config_get_int_value(config_actual, "TAMANIO_SWAP");
    tamanio_pagina = config_get_int_value(config_actual, "TAMANIO_PAGINA");
    char** file_swap = config_get_array_value(config_actual, "ARCHIVOS_SWAP");
    marcos_maximos = config_get_int_value(config_actual, "MARCOS_MAXIMOS");
    retardo_swap = config_get_int_value(config_actual, "RETARDO_SWAP");
    
    t_list* archivos_swap = list_create();

    while(file_swap[contador] != NULL) {
        list_add(archivos_swap, file_swap[contador]);
        contador++;
    }

    crear_archivos_swap(archivos_swap, (tamanio_swap/tamanio_pagina));

    list_destroy(archivos_swap);
    free(file_swap); //sobre esto, nose si se deberia hacer un free, xq creo que eso se hace recien cuando haga el config_Destroy()
}

void crear_archivos_swap(t_list* archivos_swap, int cantidad_particiones) {

    struct stat* sb;
    char caracter_llenado = '\0';

    while(! list_is_empty(archivos_swap)) {
        swap_files* nuevo_swap = malloc(sizeof(swap_files));
        char* path_swap = (char*) list_remove(archivos_swap, 0);

        int fd = open(path_swap, O_CREAT | O_RDWR, (mode_t) 0777);

        truncate(path_swap, tamanio_swap);
        nuevo_swap->swap_file = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        int estado = fstat(fd, sb);

        if(estado != -1) {
            log_info(logger_swamp, "Archivos de swap creados y de tamaño: %i", tamanio_swap);
        }else{
            exit(-1);
        }

        nuevo_swap->path = path_swap;

        memcpy(nuevo_swap->swap_file, &caracter_llenado, sizeof(char));
        nuevo_swap->particiones_swap = crear_lista_particiones(cantidad_particiones);

        munmap(nuevo_swap->swap_file, tamanio_swap);
        close(fd);

        list_add(lista_swap_files, nuevo_swap);
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


particion* particion_nueva(int numero){
    particion* particion_nueva = malloc(sizeof(particion));
    particion_nueva->esta_libre = 1;
    particion_nueva->num_particion = numero;
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

    if(swap != NULL) {
        log_info(logger_swamp, "Se encontro el archivo buscado");
        particion* particion_nueva = list_find(swap->particiones_swap, pagina_libre);
        if(particion_nueva != NULL) {
            return particion_nueva;
        }
    }
    
    return NULL;
}

int pagina_libre(particion* particion_nueva) {
    if(particion_nueva->esta_libre == 1) {
        return 1;
    }
    return 0;
}

int cantidad_frames_disponibles(char* path_swap) {
    particion* frame = malloc(sizeof(particion));
    swap_files* file = malloc(sizeof(swap_files));
    int frames_libres = 0;
    
    file->path = path_swap;
    
    int fd = open(file->path, O_RDWR);

    if(fd == 0) {
        log_error(logger_swamp, "No se pudo abrir el archivo f");
    }else{
        for(int i=0; i < lista_particiones->elements_count; i++){
		    frame = list_get(lista_particiones,i);

	    	if(frame->esta_libre){
                frames_libres++;
	    	}
	    }
    }
    close(fd);
    return frames_libres;
}

void manejar_asignacion() {

	if(strcmp(tipo_asignacion, "FIJA") == 1) {
        log_info(logger_swamp, "Tipo de asignacion a utilizar es %s", tipo_asignacion);
    }else{
        log_info(logger_swamp, "Tipo de asignacion a utilizar es %s", tipo_asignacion);
    }
}

void guardar_pagina(uint32_t PID) {

    swap_files* file = malloc(sizeof(swap_files));
    
    
    if(strcmp(tipo_asignacion, "FIJA") == 1) {

    }else{

    }

}

int verificar_pid_en_swap_file(uint32_t PID, char* path_swap) {

    swap_files* file = malloc(sizeof(swap_files));
    file->path = path_swap;
    int valor = 0;

    int fd = open(file->path, O_RDWR);
    if(fd == 0) {
        log_error(logger_swamp, "Error al querer abrir el file %s", file->path);
    }

    for(int i = 0; i < file->procesos_swap->elements_count; i++) {
        valor = list_get(file->procesos_swap->elements_count, i);
        if(valor == PID) {
            log_info(logger_swamp, "PID presente en %s", file->path);
            return 1;
        }
    }
    return 0;
}