#include "Kernel.h"


void inicializarListas(){
    procesosNew = list_create();
    procesosReady = list_create();
    procesosExec = list_create();
    procesosExit = list_create();
    procesosSuspendedReady = list_create();
    procesosSuspendedBlock = list_create();
}

void finalizarListas(){
    list_destroy(procesosNew);
    list_destroy(procesosReady);
    list_destroy(procesosExec);
    list_destroy(procesosExit);
    list_destroy(procesosSuspendedReady);
    list_destroy(procesosSuspendedBlock);
}


t_config* inicializarConfig(){
    return config_create("cfg/ConfiguracionKernel.config");
}

void obtenerValoresDelConfig(t_config* configActual){

    ipMemoria = config_get_string_value(configActual, "IP_MEMORIA");
    puertoMemoria = config_get_string_value(configActual, "PUERTO_MEMORIA");
    algoritmoPlanificacion = config_get_string_value(configActual, "ALGORITMO_PLANIFICACION");
    retardoCPU = config_get_int_value(configActual, "RETARDO_CPU");
    gradoMultiProgramacion = config_get_int_value(configActual, "GRADO_MULTIPROGRAMACION");
    gradoMultiProcesamiento = config_get_int_value(configActual, "GRADO_MULTIPROCESAMIENTO");
}


void finalizarConfig(t_config* configUsado){
    config_destroy(configUsado);
}

int main(){
    inicializarListas();
    t_config* configActual = inicializarConfig();
    obtenerValoresDelConfig(configActual);

    /* toda la logica de los planificadores y del servidor */
    
    finalizarListas();
    finalizarConfig(configActual);
    return 0;
}


