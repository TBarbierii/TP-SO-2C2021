#include "Semaforos.h"

void crearSemaforo(char* nombreSem, unsigned int valorSem){

    semaforo* semaforoNuevo = malloc(sizeof(semaforo));
    semaforoNuevo->nombre = nombreSem;
    semaforoNuevo->valor = valorSem;
    semaforoNuevo->listaDeProcesosEnEspera = list_create();


    /*esto se va a hacer con el uso de un mutex para controlar el uso de la lista de semaforos global */
    list_add(semaforosActuales, semaforoNuevo);
    
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

    semaforo* semaforoActual =(semaforo*) list_remove_by_condition(semaforosActuales, buscarPorNombre);
    
    free(semaforoActual->nombre);
    free(semaforoActual);
}