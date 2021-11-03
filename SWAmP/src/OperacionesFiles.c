#include "Swamp_lib.h"


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


/* ---------------- Lectura ---------------- */
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
            
            /*ya que sino no nos deja loggearlo */
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