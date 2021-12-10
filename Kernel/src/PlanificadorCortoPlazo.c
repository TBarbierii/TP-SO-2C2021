#include "PlanificadorCortoPlazo.h"

void rutinaDeProceso(){
    
    while(1){

        //como son hilos creados al comienzo, aca deberiamos sacar al primer proceso que este en exec y agregarlo
        sem_wait(procesosDisponiblesParaEjecutar);

        t_log* logger = log_create("cfg/PlanificadorCortoPlazoActual.log","Thread-CPU", 1, LOG_LEVEL_DEBUG);
        //vamos a sacar al primero que esta en READY, replanificando por las dudas
        pthread_mutex_lock(modificarReady);
            replanificacion(logger); //en este momento replanifico y ordeno la lista de readys segun el criterio seleccionado
            proceso_kernel* procesoListoParaEjecutar = list_remove(procesosReady, 0);
            log_debug(logger, "\n[CORTO-PLAZO] Se saca un carpincho de la lista de readys para ejecutar. Carpincho: %d, de estimacionActual:%f segundos, y de responseRatio:%f",procesoListoParaEjecutar->pid, procesoListoParaEjecutar->rafagaEstimada, procesoListoParaEjecutar->responseRatio);
        pthread_mutex_unlock(modificarReady);
    
        //lo ponemos en la lista de exec
        pthread_mutex_lock(modificarExec);
            list_add(procesosExec, procesoListoParaEjecutar);
            log_warning(logger, "[CORTO-PLAZO] Se agrega un nuevo carpincho a la lista de carpinchos en ejecucion. Carpincho: %d", procesoListoParaEjecutar->pid);
        pthread_mutex_unlock(modificarExec);
        

        


        //le ponemos el momento donde entro en EXEC en este THREAD:
        //clock_t start  = clock();
        struct timespec begin, end; 
        clock_gettime(CLOCK_REALTIME, &begin);

        //aca recien le vamos a notificar al proceso que se inicializa y su id, es decir cuando esta en ejecucion
        if(procesoListoParaEjecutar->vuelveDeBloqueo == NO_BLOQUEADO){
                log_debug(logger, "[THREAD-CPU] Se envia info del pid a Matelib");
                enviarInformacionAdministrativaDelProceso(procesoListoParaEjecutar);
        }



        //lo que ejecuta el proceso que agregamos
        while(1){
            
            
            //si viene de un bloqueo debemos notificarle a la matelib que se pudo o no realizar
            if(procesoListoParaEjecutar->vuelveDeBloqueo == BLOCK_IO){

                //le avisamos que se pudo realizar la IO
                log_info(logger, "[THREAD-CPU] Le avisamos al proceso que se pudo ejecutar el IO correctamente");
                int valor = 1;
                avisarconexionConDispositivoIO(procesoListoParaEjecutar->conexion, valor);

            }else if (procesoListoParaEjecutar->vuelveDeBloqueo == BLOCK_SEM){
                //le avisamos que se pudo realizar el WAIT
                log_info(logger, "[THREAD-CPU] Le avisamos al proceso que se pudo ejecutar el SEM_WAIT correctamente");
                int valor = 1;
                avisarWaitDeSemaforo(procesoListoParaEjecutar->conexion, valor);
            }


            procesoListoParaEjecutar->vuelveDeBloqueo = NO_BLOQUEADO;
            
            log_info(logger, "[THREAD-CPU] Se ejecuto tarea de conexion");
            int codigoOperacion = atenderMensajeEnKernel(procesoListoParaEjecutar->conexion);


            if(rompoElHiloSegunElCodigo(codigoOperacion) == 1){

                log_info(logger, "[THREAD-CPU] Se realizo una operacion que termina con la ejecucion del Carpincho por el momento para bloquearlo");
                //clock_t end = clock();
                clock_gettime(CLOCK_REALTIME, &end);
                long seconds = end.tv_sec - begin.tv_sec;
                long nanoseconds = end.tv_nsec - begin.tv_nsec;
                double elapsed = seconds + nanoseconds*1e-9;

                //double tiempoPasado = (end - start) / CLOCKS_PER_SEC;
                log_info(logger,"[THREAD-CPU] La ultima rafaga ejecutada del proceso es de: %f\n",elapsed);
                procesoListoParaEjecutar->ultimaRafagaEjecutada = elapsed;

                calcularEstimacion(procesoListoParaEjecutar); //aaca calculo la ultima estimacion nueva en base a la ultima rafaga ejecutada
                sem_post(nivelMultiprocesamiento);
                break;

            }else if(rompoElHiloSegunElCodigo(codigoOperacion) == 2){
                
                log_info(logger, "[THREAD-CPU] Se realizo una operacion que termina con la ejecucion del Carpincho y los sacamos definitivamente\n");
                sem_post(nivelMultiprocesamiento); //aumento el grado de multiprogramacion y multiprocesamiento
	            sem_post(nivelMultiProgramacionGeneral);
                break;

            }

        }

        log_destroy(logger);
    }


}




void planificadorCortoPlazo(){

    t_log* logger = log_create("cfg/PlanificadorCortoPlazoActual.log","PlanificadorCortoPlazo", 1, LOG_LEVEL_DEBUG);
    log_debug(logger, "[CORTO-PLAZO] El algoritmo utilizado para planificar en el Corto Plazo sera: %s",algoritmoPlanificacion);
    inicializarHilosCPU();
    while(1){
    
        sem_wait(hayProcesosReady);
        sem_wait(nivelMultiprocesamiento);
        //esto lo unico que va a hacer es avisarle a los hilos que hay un nuevo proceso disponible y que lo ponga en ejecucion
        sem_post(procesosDisponiblesParaEjecutar);
    }

    log_destroy(logger);

}

void inicializarHilosCPU(){
    for (int i = 0; i < gradoMultiProcesamiento ; i++)
    {
        pthread_t hiloDeEjecucion;
        pthread_create(&hiloDeEjecucion, NULL, (void *) rutinaDeProceso, NULL);
        pthread_detach(hiloDeEjecucion);
    }
    
    
}


int rompoElHiloSegunElCodigo(int codigo){
    
    if(codigo == SEM_WAIT || codigo == CONECTAR_IO){
        return 1;
    }else if(codigo == CERRAR_INSTANCIA ){
        return 2;
    }
    return 0;

}


/* seleccionar Planificador */
void replanificacion(t_log* logger){

    if(strcmp(algoritmoPlanificacion, "HRRN") == 0){
        log_debug(logger,"[CORTO-PLAZO] Se replanifica con HRRN");
        aplicarHRRN();
    }else if(strcmp(algoritmoPlanificacion, "SJF") == 0){
        log_debug(logger,"[CORTO-PLAZO] SE replanifica con SJF");
        aplicarSJF();
    }else{
        log_error(logger,"[CORTO-PLAZO] No se puede replanificar porque no hay un algoritmo identificado");
    }

}



/* Planificación SJF */

void calcularEstimacion(proceso_kernel* unCarpincho) {
	
    double nuevaRafaga = (alfa * unCarpincho->rafagaEstimada)+ ((unCarpincho->ultimaRafagaEjecutada) *(1 - alfa));
    unCarpincho->rafagaEstimada = nuevaRafaga;
}

bool comparadorDeRafagas(proceso_kernel* unCarpincho, proceso_kernel* otroCarpincho) {
	
    return unCarpincho->rafagaEstimada <= otroCarpincho->rafagaEstimada;

}

void mostrarRafagaEstimada(proceso_kernel* unCarpincho){
    t_log* logger =log_create("cfg/PlanificadorCortoPlazoActual.log","Replanificacion", 1, LOG_LEVEL_DEBUG);
    log_info(logger,"[CORTO-PLAZO] El proceso:%d tiene una rafaga de:%f",unCarpincho->pid, unCarpincho->rafagaEstimada);
    log_destroy(logger);  
}

void aplicarSJF() {
    list_iterate(procesosReady, (void*) CalcularResponseRatio);
    list_iterate(procesosReady, (void*) mostrarRafagaEstimada);
    list_sort(procesosReady, (void*) comparadorDeRafagas); //con list_sort se ordena la lista segun un criterio (en este caso va a ordenarlo por las rafagas)
				
}



/* HRNN  */

void AumentarTiempoEspera(proceso_kernel* unCarpincho){ //con lo de las funciones clock creo que esto no seria necesario 
	
    unCarpincho->tiempoDeEspera++; //esta funcion quiza no sea necesaria

}

void CalcularResponseRatio(proceso_kernel* unCarpincho) {

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    
    long seconds = end.tv_sec - unCarpincho->tiempoDeArriboColaReady.tv_sec;
    long nanoseconds = end.tv_nsec - unCarpincho->tiempoDeArriboColaReady.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    unCarpincho->tiempoDeEspera = elapsed;
	unCarpincho->responseRatio = 1 + (unCarpincho->tiempoDeEspera / unCarpincho->rafagaEstimada);

}

bool comparadorResponseRatio(proceso_kernel* unCarpincho, proceso_kernel* otroCarpincho) {
	
    return unCarpincho->responseRatio >= otroCarpincho->responseRatio;

}

void mostrarResponseRatio(proceso_kernel* unCarpincho){
    t_log* logger=log_create("cfg/PlanificadorCortoPlazoActual.log","Replanificacion", 1, LOG_LEVEL_DEBUG);
    log_info(logger,"[CORTO-PLAZO] El proceso:%d tiene un ratio de:%f",unCarpincho->pid, unCarpincho->responseRatio);
    log_destroy(logger);  
}

void aplicarHRRN(){

	list_iterate(procesosReady, (void*) CalcularResponseRatio); //calcula la estimación de todos los procesos para después ordenarla segun la priorirdad
	list_iterate(procesosReady, (void*) mostrarResponseRatio);
    list_sort(procesosReady, (void*) comparadorResponseRatio); // lo mismo que SJF, ordena segun un criterio (en este caso el de mayor responseRAtio)
    
} 

