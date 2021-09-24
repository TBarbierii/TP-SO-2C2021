#include "PlanificadorCortoPlazo.h"

void planificadorCortoPlazo(){

    t_log* logger = log_create(".cfg/PlanificadorCortoPlazo.log","PlanificadorCortoPlazo", 0, LOG_LEVEL_DEBUG);

    log_debug(logger, "El algoritmo utilizado para planificar en el Corto Plazo sera: ", algoritmoPlanificacion);
    
    while(1){
        sem_wait(hayProcesosReady);
        sem_wait(nivelMultiprocesamiento);
        
        pthread_mutex_lock(modificarReady);

            replanificacion(); //en este momento replanifico y ordeno la lista de readys segun el criterio seleccionado
            proceso* procesoListoParaEjecutar = list_remove(procesosReady, 0);
            log_debug(logger, "Se saca un carpincho de la lista de readys para ejecutar. Carpincho: ", string_itoa(procesoListoParaEjecutar->pid));

        pthread_mutex_unlock(modificarReady);
    
        pthread_mutex_lock(modificarExec);
            list_add(procesosExec, procesoListoParaEjecutar);
            log_debug(logger, "Se agrega un nuevo carpincho a la lista de carpinchos en ejecucion. Carpincho: ", string_itoa(procesoListoParaEjecutar->pid));
        pthread_mutex_unlock(modificarExec);

        pthread_t hiloDeEjecucion;
        log_debug(logger, "Se crea un hilo para el carpincho. Carpincho: ", string_itoa(procesoListoParaEjecutar->pid));
        pthread_create(&hiloDeEjecucion, NULL, (void *) rutinaDeProceso, procesoListoParaEjecutar);
        pthread_detach(hiloDeEjecucion);

    }

    log_destroy(logger);

}

void replanificarSegunAlgoritmo(){ 

}

void rutinaDeProceso(proceso* procesoEjecutando){
    
    char* nombreLogger = string_new();
    string_append(&nombreLogger,".cfg/Carpincho");
    string_append(&nombreLogger, string_itoa(procesoEjecutando->pid));
    string_append(&nombreLogger,".log");

    t_log* logger = log_create(nombreLogger,"Carpincho", 0, LOG_LEVEL_DEBUG);

    free(nombreLogger);

    while(1){
        
        log_info(logger, "Se ejecuto tarea de conexion");
        int codigoOperacion = atenderMensajeEnKernel(procesoEjecutando->conexion);
        if(rompoElHiloSegunElCodigo(codigoOperacion)){
            log_info(logger, "Se realizo una operacion que termina con la ejecucion del Carpincho por el momento");
            break;
        }
    }

}

int rompoElHiloSegunElCodigo(int codigo){
    if(codigo == CERRAR_INSTANCIA || codigo == SEM_WAIT || codigo == SEM_SIGNAL || codigo == CONECTAR_IO){
        return 1;
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

void calcularEstimacion(proceso* unCarpincho) {
	unCarpincho->rafagaEstimada = (alfa * estimacion_inicial)+ ((unCarpincho->ultimaRafagaEjecutada) * (1 - alfa) );
}

bool comparadorDeRafagas(proceso* unCarpincho, proceso* otroCarpincho) {
	return unCarpincho->rafagaEstimada <= otroCarpincho->rafagaEstimada;
}

void aplicarSJF() {

    //lo del IF LO saco, ya que lo de aplicar SJF siempre se hace cuando haya grado de multiprocesamiento y haya procesos en ready esperando, asi que no seria necesario evaluar tal condicion

	list_iterate(procesosReady, (void*) calcularEstimacion); //esto va a iterar cada elemento de la lista, y va a aplicarle el cambio de la ultima rafaga
	
    //podriamos hacer que se calcule los procesos su ultima estimacion, y no seria necesario crear una nueva lista, ya que podriamos evaluarla con la lista de ready

    list_sort(procesosReady, (void*) comparadorDeRafagas); //con list_sort se ordena la lista segun un criterio (en este caso va a ordenarlo por las rafagas)

				
}








/* HRNN  */

void AumentarTiempoEspera(proceso* unCarpincho){
	unCarpincho->tiempoDeEspera++;
}

void CalcularResponseRatio(proceso* unCarpincho) {
	calcularEstimacion(unCarpincho);
	unCarpincho->responseRatio = 1 + (unCarpincho->tiempoDeEspera / unCarpincho->rafagaEstimada);
}

bool comparadorResponseRatio(proceso* unCarpincho, proceso* otroCarpincho) {
	return unCarpincho->responseRatio >= otroCarpincho->responseRatio;
}
void aplicarHRRN(){

		list_iterate(procesosReady, (void*) CalcularResponseRatio); //calcula la estimación de todos los procesos para después ordenarla segun la priorirdad
		list_sort(procesosReady, (void*) comparadorResponseRatio); // lo mismo que SJF, ordena segun un criterio (en este caso el de mayor responseRAtio)

} 

