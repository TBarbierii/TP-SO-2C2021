#include "PlanificadorLargoPlazo.h"

void planificadorLargoPlazo(){

    inicializarSemaforos();

    while(1){
        sem_wait(semaforoProcesosEnNew);
        sem_wait(semaforoDeMultiprogramacion);

        char* proceso = list_remove(procesosNew,0); // todavia no sabemos la estructura del proceso por ende tampoco el tipo    
        sem_wait(modificarReady);
        list_add(procesosReady,proceso);
        sem_post(modificarReady);
    }
}

void inicializarSemaforos(){

    sem_init(semaforoDeMultiprogramacion,1, gradoMultiProgramacion);
    sem_init(semaforoProcesosEnNew, 1,0);

}
