#include "PlanificadorMedianoPlazo.h"

void planificadorLargoPlazo(){

    pthread_t  t1, t2;

    pthread_create(&t1, NULL,(void *) thread1_PMP, NULL);
    pthread_create(&t2, NULL,(void *) thread2_PMP, NULL);
    
    pthread_detach(t1);
    pthread_detach(t2);
}


void thread1_PMP(){

}
void thread2_PMP(){

    while(1){
        sem_wait(procesoNecesitaEntrarEnReady);
        sem_wait(nivelMultiProgramacionGeneral);

        pthread_mutex_lock(modificarSuspendedReady);
            int verdadero = list_is_empty(procesosSuspendedReady);
        pthread_mutex_unlock(modificarSuspendedReady);
            if(verdadero){

                pthread_mutex_unlock(nivelMultiProgramacionBajaPrioridad);
            }else{

               pthread_mutex_lock(modificarSuspendedReady);
                    proceso* procesoParaPreparar = list_remove(procesosSuspendedReady,0);
                pthread_mutex_unlock(modificarSuspendedReady);

                pthread_mutex_lock(modificarReady);
                    list_add(procesosReady,procesoParaPreparar);
                pthread_mutex_unlock(modificarReady);

                sem_post(hayProcesosReady);
            }
    }
}