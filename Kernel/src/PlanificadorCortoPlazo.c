#include "PlanificadorCortoPlazo.h"

void planificadorCortoPlazo(){

    while(1){
        sem_wait(hayProcesosReady);
        sem_wait(nivelMultiprocesamiento);
        
        pthread_mutex_lock(modificarReady);
            // TODO: ordenar la lista de readys segun el criterio 
            proceso* procesoListoParaEjecutar = list_remove(procesosReady, 0);
        pthread_mutex_unlock(modificarReady);
    
        pthread_mutex_lock(modificarExec);
            list_add(procesosExec, procesoListoParaEjecutar);
        pthread_mutex_unlock(modificarExec);

        pthread_t hiloDeEjecucion;

        pthread_create(&hiloDeEjecucion, NULL, (void *) rutinaDeProceso, procesoListoParaEjecutar);
        pthread_detach(hiloDeEjecucion);



    }
}

void replanificarSegunAlgoritmo(){

}

void rutinaDeProceso(proceso* procesoEjecutando){

    while(1){

        int codigoOperacion = atenderMensajeEnKernel(procesoEjecutando->conexion);
        if(rompoElHiloSegunElCodigo(codigoOperacion)){
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

/* Planificación SJF */

proceso* calcularEstimacion(proceso* unCarpincho) {
	unCarpincho->rafagaEstimada = (alfa * estimacion_inicial)+ ((unCarpincho->ultimaRafagaEjecutada) * (1 - alfa) );
	return unCarpincho;
}

bool comparadorDeRafagas(proceso* unCarpincho, proceso* otroCarpincho) {
	return unCarpincho->rafagaEstimada <= otroCarpincho->rafagaEstimada;
}

void aplicarSJF() {


	if (!list_is_empty(procesosReady) && list_is_empty(procesosExec))  //enrealidad no es necesario que la lista de exec este vacia
		{  
		t_list* aux = list_map(procesosReady, (void*) calcularEstimacion); //mapeo la lista de procesos en REady con calcularEStimación para asi definir que proceso pone a ejecutar.
		list_sort(aux, (void*) comparadorDeRafagas); //con list_sort se ordena la lista segun un criterio (en este caso va a ordenarlo por las rafagas)

		}
			
	}


/* HRNN  */

void AumentarTiempoEspera(proceso* unCarpincho){
	unCarpincho->tiempoDeEspera++;
}


void aplicarHRRN(){

	if (!list_is_empty(procesosReady) && list_is_empty(procesosExec)) 
	{
		t_list* aux = list_map(procesosReady, (void*) calcularHRRN);
		list_sort(aux, (void*) comparadorHRRN); // a implementar


	}
} 