#include "Kernel.h"


void inicializarListas(){
    procesosNew = list_create();
    procesosReady = list_create();
    procesosBlockIO = list_create();
    procesosExec = list_create();
    procesosExit = list_create();
    procesosSuspendedReady = list_create();
    procesosSuspendedBlock = list_create();
}

void finalizarListas(){
    list_destroy(procesosNew);
    list_destroy(procesosReady);
    list_destroy(procesosExec);
    list_destroy(procesosBlockIO);
    list_destroy(procesosExit);
    list_destroy(procesosSuspendedReady);
    list_destroy(procesosSuspendedBlock);
}


t_config* inicializarConfig(){
    return config_create("cfg/ConfiguracionKernel.config");
}

void finalizarConfig(t_config* configUsado){
    config_destroy(configUsado);
}


int main(){
    inicializarListas();
    t_config* configActual = inicializarConfig();
    finalizarListas();
    finalizarConfig(configActual);
    return 0;
}


