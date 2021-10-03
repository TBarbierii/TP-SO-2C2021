#include "Semaforos.h"

void crearSemaforo(char* nombreSem, unsigned int valorSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);
    
    semaforo* semaforoNuevo = malloc(sizeof(semaforo));
    semaforoNuevo->nombre = nombreSem;
    semaforoNuevo->valor = valorSem;
    log_info(logger,"Se ha creado un nuevo semaforo de nombre: %s y valor: %u",nombreSem, valorSem);
    semaforoNuevo->listaDeProcesosEnEspera = list_create();
    //le vamos a dar un mutex al semaforo para agregar y sacar elementos de la su lista de bloqueados
    semaforoNuevo->semaforoActual = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(semaforoNuevo->semaforoActual,NULL);

    /*esto se va a hacer con el uso de un mutex para controlar el uso de la lista de semaforos global */
    pthread_mutex_lock(controladorSemaforos);
        list_add(semaforosActuales, semaforoNuevo);
    pthread_mutex_unlock(controladorSemaforos);

    log_destroy(logger);

}


void destruirSemaforo(char* nombreSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);
    
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

    log_info(logger,"Se va a destruir un semaforo llamado: %s", nombreSem);    
    /*aca deberiamos sacar a todos los elementos que se encuentran bloqueados en el semaforo y ponerlos en ready */
    //eso lo tengo con una funcion quiza que lo ponga en ready si estaba en blocked, y en suspended- reday si estaba en suspended-blocked
    list_destroy(semaforoNuevo->listaDeProcesosEnEspera);
    pthread_mutex_destroy(semaforoNuevo->semaforoActual);
    free(semaforoNuevo->semaforoActual);
    free(semaforoNuevo->nombre);
    free(semaforoNuevo);

    log_destroy(logger);
    
}