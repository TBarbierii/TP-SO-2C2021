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

    struct stat* sb;

    char caracter_llenado = '\0';

    while(! list_is_empty(archivos_swap)) {

        swap_files* nuevo_swap = malloc(sizeof(swap_files));

        char* path_swap = (char*) list_remove(archivos_swap, 0);

        int fd = open(path_swap, O_CREAT | O_RDWR, (mode_t) 0777);

        truncate(path_swap, tamanio_swap);

        nuevo_swap->swap_file = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        int estado = stat(nuevo_swap->path, sb);

        if(estado != -1) {
            log_info(logger_swamp, "Archivos de swap creados y de tamaño: %i", tamanio_swap);
        }else{
            exit(-1);
        }

        memcpy(nuevo_swap->swap_file, &caracter_llenado, sizeof(char));


        /* faltaria crear las particiones para el archivo swap */
        nuevo_swap->particiones_swap = crearListaDeParticiones(cantidadParticiones);

    }

}

t_list* crearListaDeParticiones(int cantidadParticiones){

    listaParticiones = list_create();
    int offset_particion = 0;

    for(int i=0; i < cantidadParticiones ; i++){
        particion* particion_swap = malloc(sizeof(particion));
        swap_files* nuevo_swap = malloc(sizeof(swap_files));
        particion_swap->num_particion = i;
        particion_swap->inicio_particion = nuevo_swap->swap_file + offset_particion;
        particion_swap->esta_libre = 1;
        offset_particion += tamanio_pagina;
        list_add(listaParticiones, particion_swap);
    }
    return listaParticiones;
}


particion* particionNueva(int numero){
    particion* particionNueva = malloc(sizeof(particion));
    particionNueva->esta_libre = 1;
    particionNueva->num_particion = numero;
    return particionNueva;
}

particion* buscar_particion_libre(char* path_swap) {
    particion* frame = malloc(sizeof(particion));
    swap_files* file = malloc(sizeof(swap_files));

    file->path = path_swap;

    int fd = open(file->path, O_RDWR);

	for(int i=0; i < listaParticiones->elements_count; i++){
		frame = list_get(listaParticiones,i);

		if(frame->esta_libre){
			return frame;
		}
	}
	return NULL;
}

int cantidad_frames_disponibles(char* path_swap) {
    particion* frame = malloc(sizeof(particion));
    swap_files* file = malloc(sizeof(swap_files));
    int frames_libres = 0;

    file->path = path_swap;

    int fd = open(file->path, O_RDWR);

    for(int i=0; i < listaParticiones->elements_count; i++){
		frame = list_get(listaParticiones,i);
    
		if(frame->esta_libre){
            frames_libres++;
		}
	}
    return frames_libres;
}

void asignacion_fija(int pid) {



}


int iniciar_servidor_swamp(void) {

	int socket_swamp;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_swap, puerto_swap, &hints, &servinfo);

	socket_swamp = socket(servinfo->ai_family, 
    	                	servinfo->ai_socktype,
        	            	servinfo->ai_protocol);

	bind(socket_swamp, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_swamp, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a ram");

    return socket_swamp;
}

void* conexion_ram() {
    
}