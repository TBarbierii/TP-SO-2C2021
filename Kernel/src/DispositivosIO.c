#include "DispositivosIO.h"


void ejecutarDispositivosIO(){


    t_log* loggerDevicesIO = log_create("cfg/DispositivosIO.log","DispositivosIO", 0, LOG_LEVEL_DEBUG);

    t_list* listaParaEjecutarHilosDeDispositivos = list_create();

    pthread_mutex_lock(controladorIO);
    list_add_all(listaParaEjecutarHilosDeDispositivos, dispositivosIODisponibles);
    pthread_mutex_unlock(controladorIO);

    while(!list_is_empty(listaParaEjecutarHilosDeDispositivos)){

        dispositivoIO* dispositivoActual= list_remove(listaParaEjecutarHilosDeDispositivos,0);
        /* crear el hilo y bla bla bla */
        pthread_t hiloDeEjecucion;
        log_debug(loggerDevicesIO, "Se crea un hilo para ejecutar el Dispositivio IO: %s, con tiempo de espera: %d", dispositivoActual->nombre, dispositivoActual->duracionRafaga);
        pthread_create(&hiloDeEjecucion, NULL, (void *) rutinaDispositivoIO, dispositivoActual);
        pthread_detach(hiloDeEjecucion);
    }

    log_destroy(loggerDevicesIO);
}



void rutinaDispositivoIO(dispositivoIO* dispositivo){

     t_log* loggerDevicesIO = log_create("cfg/DispositivosIO.log","DispositivosIO", 0, LOG_LEVEL_DEBUG);

    while(1){
        sem_wait(dispositivo->activadorDispositivo);

        log_info(loggerDevicesIO,"Un nuevo proceso entro a solicitar el recurso y tendra que esperar durante:%d", dispositivo->duracionRafaga);   
        /*tiempo que pasa en el bloqueo */
        sleep(dispositivo->duracionRafaga);

    
        pthread_mutex_lock(dispositivo->mutex);
            proceso_kernel* procesoLiberado = list_remove(dispositivo->listaDeProcesosEnEspera, 0);
        pthread_mutex_unlock(dispositivo->mutex);

        log_info(loggerDevicesIO,"Termino de ejecutar el proceso:%d sobre el dispositivo IO:%s, lo ponemos en ready si el nivel de multiprogramacion lo permite", procesoLiberado->pid, dispositivo->nombre);   
        ponerEnElReadyIndicado(procesoLiberado);
    }

    log_destroy(loggerDevicesIO);

}


int realizarOperacionIO(int pid, char* nombreDevice){

    int procesoBuscado(proceso_kernel* procesoBuscado){
        if(procesoBuscado->pid == pid){
            return 1;
        }
        return 0;
    }

    //busco si el proceso esta en ejecucion
    pthread_mutex_lock(modificarExec);
    proceso_kernel* procesoEncontrado = list_remove_by_condition(procesosExec, procesoBuscado);
    pthread_mutex_unlock(modificarExec);

    if(procesoEncontrado != NULL){
        
        int dispositivoBuscado(dispositivoIO* dispositivo){
            if(strcmp(dispositivo->nombre, nombreDevice) == 0){
                return 1;
            }
            return 0;
        }
        //buscamos si se encuentra el dispositivo IO para usar 
        pthread_mutex_lock(controladorIO);
        dispositivoIO* dispositivoEncontrado = list_find(dispositivosIODisponibles, dispositivoBuscado);
        pthread_mutex_unlock(controladorIO);

        if(dispositivoEncontrado != NULL){
            
            agregarProcesoADispositivo(procesoEncontrado, dispositivoEncontrado);
            return 0;

        }

    }

    return 1;

}


void agregarProcesoADispositivo(proceso_kernel* proceso, dispositivoIO* device){
    
    t_log* loggerDevicesIO = log_create("cfg/DispositivosIO.log","DispositivosIO", 0, LOG_LEVEL_INFO);

    /*lo agrego en la lista de espera del dispositivo y en la lista de bloqueados general */
    pthread_mutex_lock(device->mutex);
        list_add(device->listaDeProcesosEnEspera,proceso);
    pthread_mutex_unlock(device->mutex);

    pthread_mutex_lock(modificarBlocked);
        list_add(procesosBlocked,proceso);
        sem_post(signalSuspensionProceso);
    pthread_mutex_unlock(modificarBlocked);

    log_info(loggerDevicesIO,"Agregamos al proceso en bloqueo y en el dispositivo: %s", device->nombre);

    /*alerto que hay un nuevo proceso que quiere ejecutar el Dispositivo IO */
    sem_post(device->activadorDispositivo);

    log_destroy(loggerDevicesIO);
    
}