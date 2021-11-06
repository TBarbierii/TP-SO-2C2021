#include "PlanificadorCortoPlazo.h"

void rutinaDeProceso(){
    
    t_log* logger = log_create("cfg/PlanificadorCortoPlazoActual.log","Thread-CPU", 1, LOG_LEVEL_DEBUG);
    
    while(1){

        //como son hilos creados al comienzo, aca deberiamos sacar al primer proceso que este en exec y agregarlo
        sem_wait(procesosDisponiblesParaEjecutar);

        //vamos a sacar al primero que esta en READY, replanificando por las dudas
        pthread_mutex_lock(modificarReady);
            replanificacion(); //en este momento replanifico y ordeno la lista de readys segun el criterio seleccionado
            proceso_kernel* procesoListoParaEjecutar = list_remove(procesosReady, 0);
            log_debug(logger, "Se saca un carpincho de la lista de readys para ejecutar. Carpincho: %d",procesoListoParaEjecutar->pid);
        pthread_mutex_unlock(modificarReady);
    
        //lo ponemos en la lista de exec
        pthread_mutex_lock(modificarExec);
            list_add(procesosExec, procesoListoParaEjecutar);
            log_debug(logger, "Se agrega un nuevo carpincho a la lista de carpinchos en ejecucion. Carpincho: %d", procesoListoParaEjecutar->pid);
        pthread_mutex_unlock(modificarExec);
        
        //le ponemos el momento donde entro en EXEC en este THREAD:
        clock_t arranqueEjecucion = clock();


        //lo que ejecuta el proceso que agregamos
        while(1){

            
            //si viene de un bloqueo debemos notificarle a la matelib que se pudo o no realizar
            if(procesoListoParaEjecutar->vuelveDeBloqueo == BLOCK_IO){

                //le avisamos que se pudo realizar la IO
                log_info(logger, "Le avisamos al proceso que se pudo ejecutar el IO correctamente");
                int valor = 0;
                avisarconexionConDispositivoIO(procesoListoParaEjecutar->conexion, valor);

            }else if (procesoListoParaEjecutar->vuelveDeBloqueo == BLOCK_SEM){
                //le avisamos que se pudo realizar el WAIT
                log_info(logger, "Le avisamos al proceso que se pudo ejecutar el SEM_WAIT correctamente");
                int valor = 0;
                avisarWaitDeSemaforo(procesoListoParaEjecutar->conexion, valor);
            }
            
            
            log_info(logger, "Se ejecuto tarea de conexion");
            int codigoOperacion = atenderMensajeEnKernel(procesoListoParaEjecutar->conexion);
            log_info(logger, "La tarea realizada fue: %d", codigoOperacion);

            if(rompoElHiloSegunElCodigo(codigoOperacion) == 1){

                log_info(logger, "Se realizo una operacion que termina con la ejecucion del Carpincho por el momento para bloquearlo");
                clock_t finEjecucion = clock();
                procesoListoParaEjecutar->ultimaRafagaEjecutada = (double)(finEjecucion - arranqueEjecucion) / CLOCKS_PER_SEC;
                calcularEstimacion(procesoListoParaEjecutar); //aaca calculo la ultima estimacion nueva en base a la ultima rafaga ejecutada
                sem_post(nivelMultiprocesamiento);
                break;

            }else if(rompoElHiloSegunElCodigo(codigoOperacion) == 2){
                
                log_info(logger, "Se realizo una operacion que termina con la ejecucion del Carpincho");
                break;

            }

        }

    }

    log_destroy(logger);

}




void planificadorCortoPlazo(){

    t_log* logger = log_create("cfg/PlanificadorCortoPlazoActual.log","PlanificadorCortoPlazo", 0, LOG_LEVEL_DEBUG);
    log_debug(logger, "El algoritmo utilizado para planificar en el Corto Plazo sera: %s",algoritmoPlanificacion);
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

