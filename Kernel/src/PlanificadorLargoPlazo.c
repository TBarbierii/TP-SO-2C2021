#include "PlanificadorLargoPlazo.h"

void planificadorLargoPlazo(){

    while(1){
        sem_wait(hayProcesosNew);
        pthread_mutex_lock(nivelMultiProgramacionBajaPrioridad);

        pthread_mutex_lock(modificarNew);
            proceso* procesoNuevo = (proceso*) list_remove(procesosNew,0);   
        pthread_mutex_unlock(modificarNew);

        pthread_mutex_lock(modificarReady);
            list_add(procesosReady,procesoNuevo);
        pthread_mutex_unlock(modificarReady);

        sem_post(hayProcesosReady);
    }
}
