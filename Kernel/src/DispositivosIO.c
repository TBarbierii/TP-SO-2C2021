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

    sem_wait(dispositivo->activadorDispositivo);
    
    /*tiempo que pasa en el bloqueo */
    sleep(dispositivo->duracionRafaga);


    pthread_mutex_lock(dispositivo->mutex);
        proceso_kernel* procesoLiberado = list_remove(dispositivo->listaDeProcesosEnEspera, 0);
    pthread_mutex_unlock(dispositivo->mutex);

    ponerEnElReadyIndicado(procesoLiberado);



}


void agregarProcesoADispositivo(proceso_kernel* proceso, dispositivoIO* device){

    /*lo agrego en la lista de espera del dispositivo y en la lista de bloqueados general */
    pthread_mutex_lock(device->mutex);
        list_add(device->listaDeProcesosEnEspera,proceso);
    pthread_mutex_unlock(device->mutex);

    pthread_mutex_lock(modificarBlocked);
        list_add(procesosBlocked,proceso);
    pthread_mutex_unlock(modificarBlocked);

    /*alerto que hay un nuevo proceso que quiere ejecutar el Dispositivo IO */
    sem_post(device->activadorDispositivo);
    
}