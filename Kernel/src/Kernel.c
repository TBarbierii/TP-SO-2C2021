#include "Kernel.h"


void inicializarListas(){
    procesosNew = list_create();
    procesosReady = list_create();
    procesosExec = list_create();
    procesosExit = list_create();
    procesosSuspendedReady = list_create();
    procesosSuspendedBlock = list_create();
    semaforosActuales = list_create();
}

void inicializarSemaforosGlobales(){

    /* mutexs */

    contadorProcesos = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(contadorProcesos,NULL);

    modificarReady = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarReady,NULL);

    modificarNew = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarNew,NULL);

    modificarExec = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarExec,NULL);

    modificarExit = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarExit,NULL);

    modificarSuspendedReady = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarSuspendedReady,NULL);

    nivelMultiProgramacionBajaPrioridad = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(nivelMultiProgramacionBajaPrioridad,NULL);

    pthread_mutex_lock(nivelMultiProgramacionBajaPrioridad);


    /* semaforos */

    hayProcesosNew = malloc(sizeof(sem_t));
    sem_init(hayProcesosNew,1,0);

    hayProcesosReady = malloc(sizeof(sem_t));
    sem_init(hayProcesosReady,1,0);

    nivelMultiProgramacionGeneral = malloc(sizeof(sem_t));
    sem_init(nivelMultiProgramacionGeneral,1,gradoMultiProgramacion);

    nivelMultiprocesamiento = malloc(sizeof(sem_t));
    sem_init(nivelMultiprocesamiento,1,gradoMultiProcesamiento);


}

void finalizarSemaforosGlobales(){

    pthread_mutex_destroy(contadorProcesos);

    pthread_mutex_destroy(modificarNew);
    pthread_mutex_destroy(modificarReady);
    pthread_mutex_destroy(modificarExec);
    pthread_mutex_destroy(modificarExit);
    pthread_mutex_destroy(modificarSuspendedReady);
    pthread_mutex_destroy(nivelMultiProgramacionBajaPrioridad);

    sem_destroy(hayProcesosNew);
    sem_destroy(hayProcesosReady);
    sem_destroy(nivelMultiProgramacionGeneral);
    sem_destroy(nivelMultiprocesamiento);

}


void finalizarListas(){
    list_destroy(procesosNew);
    list_destroy(procesosReady);
    list_destroy(procesosExec);
    list_destroy(procesosExit);
    list_destroy(procesosSuspendedReady);
    list_destroy(procesosSuspendedBlock);
    list_destroy(semaforosActuales);
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
    char** nombresDispositivosIO = config_get_array_value(configActual, "DISPOSITIVOS_IO");
    char** duracionesIO = config_get_array_value(configActual, "DURACIONES_IO");

    inicializarDispositivosIO(nombresDispositivosIO,duracionesIO);
}


void inicializarDispositivosIO(char ** dispositivos, char** duraciones){
    
    dispositivosIODisponibles = list_create();
    t_list* nombresDispositivos = list_create();
    t_list* duracionesDispositivos = list_create();


    int contador = 0;
    while(dispositivos[contador] != NULL) {
        list_add(nombresDispositivos, dispositivos[contador]);
        contador++;
    }

    contador = 0;
    while(duraciones[contador] != NULL) {
        list_add(duracionesDispositivos, duraciones[contador]);
        contador++;
    }
    
    while(!list_is_empty(nombresDispositivos)){
        char* nombreActual = (char*) list_remove(nombresDispositivos, 0);
        char* duracionActual = (char *) list_remove(duracionesDispositivos, 0);
        
        //esto en el compilador igual me tira error, para analizar que onda
        int sizeNombre = string_length(nombreActual)+1;

        dispositivoIO* nuevoDispositivo = (dispositivoIO*) malloc(sizeof(dispositivoIO));
        nuevoDispositivo->nombre = (char*) malloc(sizeof(char)*sizeNombre);
        nuevoDispositivo->listaDeProcesosEnEspera = list_create();
        strcpy(nuevoDispositivo->nombre, nombreActual);
        nuevoDispositivo->duracionRafaga = atoi(duracionActual);
        
        
        list_add(dispositivosIODisponibles, nuevoDispositivo);

        free(nombreActual);
        free(duracionActual);
    }
    free(dispositivos);
    free(duraciones);
    list_destroy(duracionesDispositivos);
    list_destroy(nombresDispositivos);
    
    
}


void finalizarConfig(t_config* configUsado){
    config_destroy(configUsado);
}


void finalizarDispositivosIO(){

    while(!(list_is_empty(dispositivosIODisponibles)) ){
        dispositivoIO* dispositivoAEliminar = list_remove(dispositivosIODisponibles,0);
        free(dispositivoAEliminar->nombre);
        list_destroy(dispositivoAEliminar->listaDeProcesosEnEspera);
        free(dispositivoAEliminar);
    }
    list_destroy(dispositivosIODisponibles);
}




int main(){
    cantidadDeProcesosActual = 0;
    inicializarListas();
    t_config* configActual = inicializarConfig();
    obtenerValoresDelConfig(configActual);
    inicializarSemaforosGlobales();

    /* toda la logica de los planificadores y del servidor */
    
    finalizarListas();
    finalizarDispositivosIO();
    finalizarConfig(configActual);
    return 0;
}


