#include "Semaforos.h"

void crearSemaforo(char* nombreSem, unsigned int valorSem){

    t_log* logger = log_create(".cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_DEBUG);
    
    semaforo* semaforoNuevo = malloc(sizeof(semaforo));
    semaforoNuevo->nombre = nombreSem;
    semaforoNuevo->valor = valorSem;
    log_info(logger,"Se ha creado un nuevo semaforo con el nombre ",semaforoNuevo->nombre,"y el valor ",string_itoa(semaforoNuevo->valor));
    semaforoNuevo->listaDeProcesosEnEspera = list_create();


    /*esto se va a hacer con el uso de un mutex para controlar el uso de la lista de semaforos global */
    pthread_mutex_lock(controladorSemaforos);
        list_add(semaforosActuales, semaforoNuevo);
    pthread_mutex_unlock(controladorSemaforos);
}



void destruirSemaforo(char* nombreSem){

    bool buscarPorNombre(void* semaforoActual){
        semaforo* semaforoNuevo = (semaforo*) semaforoActual;
        if(strcmp(semaforoNuevo->nombre ,nombreSem) == 0){
            return 1;
        }else{
            return 0;
        }
    }

    pthread_mutex_lock(controladorSemaforos);
        semaforo* semaforoNuevo = list_remove_by_condition(semaforosActuales, buscarPorNombre);
    pthread_mutex_unlock(controladorSemaforos);

    free(semaforoNuevo->nombre);
    free(semaforoNuevo);
    
}