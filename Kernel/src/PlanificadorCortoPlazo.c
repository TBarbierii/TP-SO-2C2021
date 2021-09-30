#include "PlanificadorCortoPlazo.h"


void rutinaDeProceso(proceso_kernel* procesoEjecutando){
    
    clock_t arranqueEjecucion = clock();

    t_log* logger = log_create("cfg/PlanificadorCortoPlazoActual.log","PlanificadorCortoPlazo", 0, LOG_LEVEL_DEBUG);


    while(1){
        
        log_info(logger, "Se ejecuto tarea de conexion");
        int codigoOperacion = atenderMensajeEnKernel(procesoEjecutando->conexion);
        log_info(logger, "La tarea realizada fue: %d", codigoOperacion);
        if(rompoElHiloSegunElCodigo(codigoOperacion) == 1){
            log_info(logger, "Se realizo una operacion que termina con la ejecucion del Carpincho por el momento para bloquearlo");
            
            clock_t finEjecucion = clock();

            procesoEjecutando->ultimaRafagaEjecutada = (double)(finEjecucion - arranqueEjecucion) / CLOCKS_PER_SEC;
            break;
        }else if(rompoElHiloSegunElCodigo(codigoOperacion) == 2){
            log_info(logger, "Se realizo una operacion que termina con la ejecucion del Carpincho");
            sem_post(nivelMultiProgramacionGeneral);
            sem_post(nivelMultiprocesamiento);
            break;
        }
    }

    log_destroy(logger);

}



void planificadorCortoPlazo(){

    t_log* logger = log_create("cfg/PlanificadorCortoPlazoActual.log","PlanificadorCortoPlazo", 0, LOG_LEVEL_DEBUG);
    log_debug(logger, "El algoritmo utilizado para planificar en el Corto Plazo sera: %s",algoritmoPlanificacion);
  
    while(1){
    
        sem_wait(hayProcesosReady);
        sem_wait(nivelMultiprocesamiento);
        
        pthread_mutex_lock(modificarReady);

            replanificacion(); //en este momento replanifico y ordeno la lista de readys segun el criterio seleccionado
            proceso_kernel* procesoListoParaEjecutar = list_remove(procesosReady, 0);
            log_debug(logger, "Se saca un carpincho de la lista de readys para ejecutar. Carpincho: %d",procesoListoParaEjecutar->pid);
        pthread_mutex_unlock(modificarReady);
    
        pthread_mutex_lock(modificarExec);
            list_add(procesosExec, procesoListoParaEjecutar);
            log_debug(logger, "Se agrega un nuevo carpincho a la lista de carpinchos en ejecucion. Carpincho: %d", procesoListoParaEjecutar->pid);
        pthread_mutex_unlock(modificarExec);

        pthread_t hiloDeEjecucion;
        log_debug(logger, "Se crea un hilo para el carpincho. Carpincho: %d", procesoListoParaEjecutar->pid);
        pthread_create(&hiloDeEjecucion, NULL, (void *) rutinaDeProceso, procesoListoParaEjecutar);
        pthread_detach(hiloDeEjecucion);

    }

    log_destroy(logger);

}


int rompoElHiloSegunElCodigo(int codigo){
    if( codigo == SEM_WAIT || codigo == SEM_SIGNAL || codigo == CONECTAR_IO){
        return 1;
    }else if(codigo == CERRAR_INSTANCIA ){
        return 2;
    }
    return 0;
}


/* seleccionar Planificador */
void replanificacion(){
    if(strcmp(algoritmoPlanificacion, "HRRN") == 0){
        aplicarHRRN();
    }else if(strcmp(algoritmoPlanificacion, "SJF") == 0){
        aplicarHRRN();
    }else{
        perror("No se puede replanificar porque no hay un algoritmo identificado");
    }
}



/* Planificación SJF */

void calcularEstimacion(proceso_kernel* unCarpincho) {
	unCarpincho->rafagaEstimada = (alfa * unCarpincho->rafagaEstimada)+ ((unCarpincho->ultimaRafagaEjecutada) * (1 - alfa) );
}

bool comparadorDeRafagas(proceso_kernel* unCarpincho, proceso_kernel* otroCarpincho) {
	return unCarpincho->rafagaEstimada < otroCarpincho->rafagaEstimada;
}

void aplicarSJF() {

    list_sort(procesosReady, (void*) comparadorDeRafagas); //con list_sort se ordena la lista segun un criterio (en este caso va a ordenarlo por las rafagas)
				
}



/* HRNN  */

void AumentarTiempoEspera(proceso_kernel* unCarpincho){ //con lo de las funciones clock creo que esto no seria necesario 
	unCarpincho->tiempoDeEspera++; //esta funcion quiza no sea necesaria
}

void CalcularResponseRatio(proceso_kernel* unCarpincho) {
    clock_t finTiempoEsperando = clock();
    unCarpincho->tiempoDeEspera = (double)(finTiempoEsperando - unCarpincho->tiempoDeArriboColaReady) / CLOCKS_PER_SEC;

	unCarpincho->responseRatio = 1 + (unCarpincho->tiempoDeEspera / unCarpincho->rafagaEstimada);
}

bool comparadorResponseRatio(proceso_kernel* unCarpincho, proceso_kernel* otroCarpincho) {
	return unCarpincho->responseRatio > otroCarpincho->responseRatio;
}


void aplicarHRRN(){

		list_iterate(procesosReady, (void*) CalcularResponseRatio); //calcula la estimación de todos los procesos para después ordenarla segun la priorirdad
		list_sort(procesosReady, (void*) comparadorResponseRatio); // lo mismo que SJF, ordena segun un criterio (en este caso el de mayor responseRAtio)

} 

