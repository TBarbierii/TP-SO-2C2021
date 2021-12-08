#include "Swamp_lib.h"

/* ---------------- Asignaciones ---------------- */
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
        particionActual->hay_contenido = 0;
        return 1;
    }
    return 0;
}

int asignar_marcos_maximos(int pid, swap_files* archivo ){

    if(cantidad_frames_disponibles(archivo) < marcos_maximos){
        return 0;
    }else{
        asignar_marcos_proceso(pid,archivo);
        return 1;        
    }
}

void asignar_marcos_proceso(int pid,swap_files* archivo){

    for(int i=0; i < marcos_maximos; i++){  
        particion* particionActual =  primer_particion_libre(archivo);
        particionActual->esta_libre = 0;
        particionActual->hay_contenido = 0;
        particionActual->pid = pid;
    }
}