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
        semaforoNuevo->mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(semaforoNuevo->mutex,NULL);

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
        pthread_mutex_destroy(semaforoNuevo->mutex);
        
        free(semaforoNuevo->mutex);
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


int realizarSignalDeSemaforo(char* nombreSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);

    bool semaforoConNombreSolicitado(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre ,nombreSem) == 0){
            return 1;
        }else{
            return 0;
        }
    }


    //arancamos a buscar el elemento si se encuentra en la lista de semaforos
    pthread_mutex_lock(controladorSemaforos);
        semaforo* semaforoActual = list_find(semaforosActuales, semaforoConNombreSolicitado);
    pthread_mutex_unlock(controladorSemaforos);


    if(semaforoActual != NULL){ //si el semaforo existe, realizamos los respectivos cambios

        pthread_mutex_lock(semaforoActual->mutex);

        semaforoActual->valor++;

        log_info(logger,"Se realizo un signal del semaforo: %s y el valor se incremento a: %d", nombreSem, semaforoActual->valor);

        if(!list_is_empty(semaforoActual->listaDeProcesosEnEspera)){ // si tiene procesos esperando, liberamos 1
            
            proceso_kernel* procesoLiberado = list_remove(semaforoActual->listaDeProcesosEnEspera, 0);
            log_info(logger,"Se libera un proceso de la lista de espera del semaforo: %s", nombreSem);
            pthread_mutex_unlock(semaforoActual->mutex);

            /* sacamos al proceso de la lista de bloqueados */
            ponerEnElReadyIndicado(procesoLiberado);

            


        }else{ // si no tiene procesos esperando, solo notificamos que se cambios el valor del semaforo
            
            log_info(logger,"Como el semaforo: %s, no tiene procesos esperando solo se aumenta su valor", nombreSem);
            pthread_mutex_unlock(semaforoActual->mutex);
        }

        
        return 0;
    
    }else{ //si no existe avisamos que se quiso hacer un acmbio sobre un semaforo que no existe
        
        log_warning(logger,"Se esta intentando hacer un signal de un semaforo: %s, el cual no existe", nombreSem);
        log_destroy(logger);
        return 1;

    }
}



/* esta es tanto para semaforos como para IO */
void ponerEnElReadyIndicado(proceso_kernel* procesoBuscado){
    
    bool seEncuentraProceso(proceso_kernel* procesoActual){ 
        if(procesoActual->pid == procesoBuscado->pid){
        return 1;
        }
        return 0;
    }



    pthread_mutex_lock(modificarSuspendedBlocked);
        proceso_kernel* procesoActual = list_remove_by_condition(procesosSuspendedBlock, seEncuentraProceso);                                                    
    pthread_mutex_unlock(modificarSuspendedBlocked);

        if(procesoActual != NULL){ //si esta, se agrega a esta lista, y se notifica que hay un procesoo que quiere entrar a READY

            pthread_mutex_lock(modificarSuspendedReady);
            list_add(procesosSuspendedReady, procesoBuscado);
            pthread_mutex_unlock(modificarSuspendedReady);

            sem_post(procesoNecesitaEntrarEnReady); //alertamos que hay un proceso que solicita entrar en ready

        }else{ //si esta solo bloqueado, se aagrega directo a ready
            
            pthread_mutex_lock(modificarReady);
            list_add(procesosReady, procesoBuscado);
            pthread_mutex_unlock(modificarReady);
            
            sem_post(hayProcesosReady); //alertamos que hay un proceso nuevo en ready

        }

}