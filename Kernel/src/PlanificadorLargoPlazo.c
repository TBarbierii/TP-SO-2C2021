#include "PlanificadorLargoPlazo.h"

void planificadorLargoPlazo(){

    t_log* logger = log_create("cfg/PlanificadorLargoPlazoActual.log","PlanificadorLargoPlazo", 1, LOG_LEVEL_DEBUG);
    log_debug(logger,"[LARGO-PLAZO] Se inicializa la planificacion de largo plazo con algoritmo FIFO");
    while(1){

        sem_wait(hayProcesosNew);
        sem_wait(nivelMultiProgramacionBajaPrioridad); //baja prioridad son los procesos que estan en New

        pthread_mutex_lock(modificarNew);
            proceso_kernel* procesoNuevo = (proceso_kernel*) list_remove(procesosNew,0);
            log_warning(logger,"\n [LARGO-PLAZO] Se esta sacando un carpincho de la cola de new, que lo vamos a poner en ready y inicializar en memoria");   
        pthread_mutex_unlock(modificarNew);


        

        //el proceso lo vamos a inicializar recien cuando el grado de multiprocesamiento lo permite, recien ahi lo inicializamos en memoria
        //esta estrategia permite performance en la memoria ya que el nivel de multiprocesamiento limita la cantidad de procesos en memoria
        establecerConexionConLaMemoria(procesoNuevo, logger);
        
        pthread_mutex_lock(modificarReady);
            list_add(procesosReady,procesoNuevo);
            clock_gettime(CLOCK_REALTIME, &(procesoNuevo->tiempoDeArriboColaReady)); //esto sirve para HRRN, para estimar cuando empezo un proceso a estar en ready y cuanto tiempo pasa ahi
        pthread_mutex_unlock(modificarReady);
	
	    log_info(logger,"[LARGO-PLAZO] Un nuevo carpincho se une a la cola de ready\n");

        sem_post(hayProcesosReady);
    }

    log_destroy(logger);

}


void liberarProceso(proceso_kernel* procesoActual){
    t_log* logger = log_create("cfg/PlanificadorLargoPlazoActual.log","PlanificadorLargoPlazo", 1, LOG_LEVEL_DEBUG);
    log_error(logger,"\n[LARGO-PLAZO] Se nos va el carpincho: %d\n", procesoActual->pid);
	//libero la estructura, nose si el clock que tiene se libera o que onda...
    log_destroy(logger);
    list_destroy(procesoActual->listaRecursosRetenidos);
	list_destroy(procesoActual->listaRecursosSolicitados);
    free(procesoActual);
}