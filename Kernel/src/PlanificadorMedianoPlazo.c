#include "PlanificadorMedianoPlazo.h"

void planificadorMedianoPlazo(){

    t_log* logger = log_create("cfg/PlanificadorMedianoPlazoActual.log","PlanificadorMedianoPlazo", 1, LOG_LEVEL_TRACE);
    
    log_debug(logger,"[MEDIANO-PLAZO] Se inicializa la planificacion de mediano plazo");

    pthread_t t1, t2;

    pthread_create(&t1, NULL,(void *) thread1_PMP, logger);
    pthread_create(&t2, NULL,(void *) thread2_PMP, logger);
    
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

    log_destroy(logger);
}


void thread1_PMP(t_log* logger){

    log_debug(logger,"[MEDIANO-PLAZO] Incializado el thread que permite suspender procesos para evitar IO bound");

    while(1){
        sem_wait(signalSuspensionProceso);
        pthread_mutex_lock(modificarReady);
        pthread_mutex_lock(modificarNew);
        pthread_mutex_lock(modificarBlocked);

        log_info(logger,"\n[MEDIANO-PLAZO] Llega un proceso a NEW o a BLOCKED, analizamos si solo hay procesos IO bound");

        if(list_is_empty(procesosReady) && !list_is_empty(procesosNew) && !list_is_empty(procesosBlocked)){
            proceso_kernel* procesoASuspender = list_remove(procesosBlocked, list_size(procesosBlocked)-1);
            log_trace(logger,"[MEDIANO-PLAZO] Se encontro que hay solo procesos IO Bound ocupando la Multiprogramacion, por lo tanto ponemos uno en Suspended-Blocked, el cual es: %d\n",procesoASuspender->pid);
            list_add(procesosSuspendedBlock, procesoASuspender);
            notificarSuspensionDeProceso(procesoASuspender, logger);
            sem_post(nivelMultiProgramacionGeneral);
            
        }else{
            log_info(logger,"[MEDIANO-PLAZO] No hay solo procesos IO bound, por lo tanto se sigue todo como estaba\n");
        }
        pthread_mutex_unlock(modificarNew);
        pthread_mutex_unlock(modificarReady);
        pthread_mutex_unlock(modificarBlocked);
    }

}
void thread2_PMP(t_log* logger){


    log_debug(logger,"\n[MEDIANO-PLAZO] Incializado el thread que permite ingreso de un proceso suspendido/new a ready");

    while(1){
        
        sem_wait(procesoNecesitaEntrarEnReady);
        sem_wait(nivelMultiProgramacionGeneral);
        log_info(logger,"[MEDIANO-PLAZO] Hay un proceso que esta queriendo entrar en Ready, analizamos si es un proceso que quiere entrar desde NEW o Suspended Ready");
        pthread_mutex_lock(modificarSuspendedReady);
            int verdadero = list_is_empty(procesosSuspendedReady);
        pthread_mutex_unlock(modificarSuspendedReady);
            if(verdadero){
                log_info(logger,"[MEDIANO-PLAZO] No hay procesos en Suspended Ready, agregamos uno de NEW en Ready\n");
                sem_post(nivelMultiProgramacionBajaPrioridad);
            }else{
                log_trace(logger,"[MEDIANO-PLAZO] Se busca un proceso de Suspended Ready y se pasa a Ready\n");
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