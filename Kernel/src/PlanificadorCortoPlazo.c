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

