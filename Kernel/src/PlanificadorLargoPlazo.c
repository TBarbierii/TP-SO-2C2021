#include "PlanificadorLargoPlazo.h"

void planificadorLargoPlazo(){

    t_log* logger = log_create(".cfg/PlanificadorLargoPlazo.log","PlanificadorLargoPlazo", 0, LOG_LEVEL_DEBUG);

    while(1){
        sem_wait(hayProcesosNew);
        pthread_mutex_lock(nivelMultiProgramacionBajaPrioridad); //baja prioridad son los procesos que estan en New

        pthread_mutex_lock(modificarNew);
            proceso* procesoNuevo = (proceso*) list_remove(procesosNew,0);
            log_info(logger,"Se esta sacando un carpincho de la cola de new. Carpincho: ", string_itoa(procesoNuevo->pid));   
        pthread_mutex_unlock(modificarNew);

        pthread_mutex_lock(modificarReady);
            list_add(procesosReady,procesoNuevo);
            log_info(logger,"Se pone en la cola de ready a un nuevo carpincho. Carpincho: ", string_itoa(procesoNuevo->pid));
        pthread_mutex_unlock(modificarReady);

        sem_post(hayProcesosReady);
    }

    log_destroy(logger);

}
