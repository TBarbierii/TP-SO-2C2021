#include "PlanificadorMedianoPlazo.h"

void planificadorMedianoPlazo(){

    t_log* logger = log_create("cfg/PlanificadorMedianoPlazoActual.log","PlanificadorMedianoPlazo", 0, LOG_LEVEL_DEBUG);
    
    log_debug(logger,"Se inicializa la planificacion de mediano plazo");

    pthread_t  t1, t2;

    pthread_create(&t1, NULL,(void *) thread1_PMP, logger); //este tenemos que analizarlo
    pthread_create(&t2, NULL,(void *) thread2_PMP, logger);
    
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

    log_destroy(logger);
}


void thread1_PMP(t_log* logger){

    log_debug(logger,"Incializado el thread que permite suspender procesos para evitar IO bound");

    while(1){
        sem_wait(signalSuspensionProceso);
        log_info(logger,"Se bloqueo un proceso, analizamos si solo hay procesos IO bound");
        pthread_mutex_lock(modificarNew);
        pthread_mutex_lock(modificarReady);
        pthread_mutex_lock(modificarBlocked);

            if(list_is_empty(procesosReady) && !list_is_empty(procesosNew) && !list_is_empty(procesosBlocked)){
                proceso_kernel* procesoASuspender = list_remove(procesosBlocked, list_size(procesosBlocked)-1); // sacamos al último proceso bloqueado
                log_info(logger,"Se encontro que hay solo procesos IO Bound ocupando la Multiprogramacion, por lo tanto ponemos uno en Suspended-Ready");
                list_add(procesosSuspendedBlock, procesoASuspender);
                notificarSuspensionDeProceso(procesoASuspender);
                sem_post(nivelMultiProgramacionGeneral);
            
            }else{
            log_info(logger,"No hay solo procesos IO bound, por lo tanto se sigue todo como estaba");
            }
        pthread_mutex_unlock(modificarNew);
        pthread_mutex_unlock(modificarReady);
        pthread_mutex_unlock(modificarBlocked);
    }

}
void thread2_PMP(t_log* logger){


    log_debug(logger,"Incializado el thread que permite ingreso de un proceso suspendido/new a ready");

    while(1){
        
        sem_wait(procesoNecesitaEntrarEnReady);
        sem_wait(nivelMultiProgramacionGeneral);
        log_info(logger,"Hay un proceso que esta queriendo entrar en Ready, analizamos si es un proceso que quiere entrar desde NEW o Suspended Ready");
        pthread_mutex_lock(modificarSuspendedReady);
            int ningunElementoEnSuspendidoReady = list_is_empty(procesosSuspendedReady);
        pthread_mutex_unlock(modificarSuspendedReady);
            if(ningunElementoEnSuspendidoReady){
                log_info(logger,"No hay procesos en Suspended Ready, agregamos uno de NEW en Ready");
                pthread_mutex_unlock(nivelMultiProgramacionBajaPrioridad);
            }else{
                log_info(logger,"Se busca un proceso de Suspended Ready y se pasa a Ready");
                pthread_mutex_lock(modificarSuspendedReady);
                    proceso_kernel* procesoParaPreparar = list_remove(procesosSuspendedReady,0);
                pthread_mutex_unlock(modificarSuspendedReady);

                pthread_mutex_lock(modificarReady);
                    list_add(procesosReady,procesoParaPreparar);
                pthread_mutex_unlock(modificarReady);
                sem_post(hayProcesosReady);
            }
    }
}