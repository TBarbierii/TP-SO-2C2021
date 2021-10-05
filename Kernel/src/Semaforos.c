#include "Semaforos.h"

int crearSemaforo(char* nombreSem, unsigned int valorSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_DEBUG);

    bool semaforoYaCreado(semaforo* semaforoNuevo){ //se dice creado porque va a tener el mismo nombre 
        if(strcmp(semaforoNuevo->nombre, nombreSem) == 0){
        return 1;
        }
        return 0;
    }



    pthread_mutex_lock(controladorSemaforos);
        t_list* listaTemporal = list_filter(semaforosActuales, semaforoYaCreado); //vamos a usar esta lista para ver si esta vacia, si no encontro  un semaforo con tal nombre, sera vacia, entonces lo vamos a crear desde 0
                                                                                  // sino no lo vamos a crear y avisamos que ya estaba creado
        int estaCreado = !(list_is_empty(listaTemporal));
        list_destroy(listaTemporal);
    pthread_mutex_unlock(controladorSemaforos);


    if(!estaCreado){
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
        return 0;
    }else{
        log_warning(logger,"Se esta intentando crear un semaforo: %s pero ya esta creado", nombreSem);
        log_destroy(logger);
        return 1;
    }
    

}


int destruirSemaforo(char* nombreSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);
    
    bool buscarPorNombre(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre ,nombreSem) == 0){
            return 1;
        }else{
            return 0;
        }
    }


    pthread_mutex_lock(controladorSemaforos);
        semaforo* semaforoNuevo = list_remove_by_condition(semaforosActuales, buscarPorNombre);
    pthread_mutex_unlock(controladorSemaforos);

    if(semaforoNuevo != NULL){
        log_info(logger,"Se va a destruir un semaforo llamado: %s", nombreSem);    
        /*aca deberiamos sacar a todos los elementos que se encuentran bloqueados en el semaforo y ponerlos en ready */
        //eso lo tengo con una funcion quiza que lo ponga en ready si estaba en blocked, y en suspended- reday si estaba en suspended-blocked
        list_destroy(semaforoNuevo->listaDeProcesosEnEspera);
        pthread_mutex_destroy(semaforoNuevo->semaforoActual);
        
        free(semaforoNuevo->semaforoActual);
        free(semaforoNuevo->nombre);
        free(semaforoNuevo);
        log_destroy(logger);
        return 0;
    }else{
        log_warning(logger,"Se esta intentando destruir un semaforo: %s, el cual no existe", nombreSem);
        log_destroy(logger);
        return 1;
    }

    
    
}