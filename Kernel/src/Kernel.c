#include "Kernel.h"


void inicializarListas(){
    procesosNew = list_create();
    procesosReady = list_create();
    procesosExec = list_create();
    procesosExit = list_create();
    procesosBlocked = list_create();
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

    modificarBlocked = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarBlocked,NULL);

    modificarSuspendedReady = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarSuspendedReady,NULL);

    modificarSuspendedBlocked = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(modificarSuspendedBlocked,NULL);

    
    controladorSemaforos = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(controladorSemaforos,NULL);

    controladorIO = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(controladorIO,NULL);
    

    /* semaforos */

    nivelMultiProgramacionBajaPrioridad = malloc(sizeof(sem_t));
    sem_init(nivelMultiProgramacionBajaPrioridad,1,0);

    procesosDisponiblesParaEjecutar = malloc(sizeof(sem_t));
    sem_init(procesosDisponiblesParaEjecutar,1,0);

    hayProcesosNew = malloc(sizeof(sem_t));
    sem_init(hayProcesosNew,1,0);

    hayProcesosReady = malloc(sizeof(sem_t));
    sem_init(hayProcesosReady,1,0);

    procesoNecesitaEntrarEnReady = malloc(sizeof(sem_t));
    sem_init(procesoNecesitaEntrarEnReady,1,0);

    nivelMultiProgramacionGeneral = malloc(sizeof(sem_t));
    sem_init(nivelMultiProgramacionGeneral,1,gradoMultiProgramacion);

    nivelMultiprocesamiento = malloc(sizeof(sem_t));
    sem_init(nivelMultiprocesamiento,1,gradoMultiProcesamiento);

    signalSuspensionProceso = malloc(sizeof(sem_t));
    sem_init(signalSuspensionProceso,1,0);
}

void finalizarSemaforosGlobales(){

    
    pthread_mutex_destroy(modificarNew);
    free(modificarNew);
    pthread_mutex_destroy(modificarReady);
    free(modificarReady);
    pthread_mutex_destroy(modificarExec);
    free(modificarExec);
    pthread_mutex_destroy(modificarExit);
    free(modificarExit);
    pthread_mutex_destroy(modificarBlocked);
    free(modificarBlocked);
    pthread_mutex_destroy(modificarSuspendedReady);
    free(modificarSuspendedReady);
    pthread_mutex_destroy(modificarSuspendedBlocked);
    free(modificarSuspendedBlocked);

    pthread_mutex_destroy(contadorProcesos);
    free(contadorProcesos);
    pthread_mutex_destroy(controladorSemaforos);
    free(controladorSemaforos);
    pthread_mutex_destroy(controladorIO);
    free(controladorIO);
    
    sem_destroy(nivelMultiProgramacionBajaPrioridad);
    free(nivelMultiProgramacionBajaPrioridad);
    
    sem_destroy(procesosDisponiblesParaEjecutar);
    free(procesosDisponiblesParaEjecutar);
    sem_destroy(hayProcesosNew);
    free(hayProcesosNew);
    sem_destroy(hayProcesosReady);
    free(hayProcesosReady);
    sem_destroy(nivelMultiProgramacionGeneral);
    free(nivelMultiProgramacionGeneral);
    sem_destroy(nivelMultiprocesamiento);
    free(nivelMultiprocesamiento);

    sem_destroy(signalSuspensionProceso);
    free(signalSuspensionProceso);


}


void finalizarListas(){
    list_destroy(procesosNew);
    list_destroy(procesosReady);
    list_destroy(procesosExec);
    list_destroy(procesosExit);
    list_destroy(procesosBlocked);
    list_destroy(procesosSuspendedReady);
    list_destroy(procesosSuspendedBlock);
    list_destroy(semaforosActuales);
}


t_config* inicializarConfig(char* path){
    //return config_create("cfg/ConfiguracionKernel.config");
    return config_create(path);
}

void obtenerValoresDelConfig(t_config* configActual){

    ipKernel = config_get_string_value(configActual, "IP_KERNEL");
    ipMemoria = config_get_string_value(configActual, "IP_MEMORIA");
    puertoMemoria = config_get_string_value(configActual, "PUERTO_MEMORIA");
    puertoServer = config_get_string_value(configActual, "PUERTO_ESCUCHA");
    algoritmoPlanificacion = config_get_string_value(configActual, "ALGORITMO_PLANIFICACION");
    estimacion_inicial = config_get_double_value(configActual, "ESTIMACION_INICIAL");
    alfa = config_get_double_value(configActual, "ALFA");
    gradoMultiProgramacion = config_get_int_value(configActual, "GRADO_MULTIPROGRAMACION");
    gradoMultiProcesamiento = config_get_int_value(configActual, "GRADO_MULTIPROCESAMIENTO");
    tiempoDeadlock = config_get_int_value(configActual, "TIEMPO_DEADLOCK");
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
        /* un semaforo activador para avisarle que un proceso lo va a ejecutar y otro para agregar o quitar elementos de su lista de espera */
        nuevoDispositivo->activadorDispositivo = malloc(sizeof(sem_t));
        sem_init(nuevoDispositivo->activadorDispositivo,1,0);

        nuevoDispositivo->mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(nuevoDispositivo->mutex,NULL);



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



int main(int argc, char *argv[]){

    t_log* logger = log_create("cfg/KernelActual.log","KernelActual",1,LOG_LEVEL_INFO);

    valorIdSemaforos = 0;
    cantidadDeProcesosActual = 0;

    inicializarListas();

    if(argc < 2){
        perror("Falta path de archivo de configuración.");
        return -1;
    }

    t_config* configActual = inicializarConfig(argv[1]);

    
    log_info(logger,"[GENERAL] Se inicializan el config y las listas de planificacion");
    
    obtenerValoresDelConfig(configActual);

    log_info(logger,"[GENERAL] Se inicializan los valores provenientes del config");

    inicializarSemaforosGlobales();

    log_info(logger,"[GENERAL] Se inicializan los semaforos utilizados para la sincronizacion de los planificadores");
    

    ejecutarDispositivosIO();
  
    
// toda la logica de los planificadores y del servidor 
    
    pthread_t servidor, pCortoPlazo, pLargoPlazo, pMedianoPlazo, algoritmoDeadlock;
		
    pthread_create(&servidor,NULL,(void*)atenderSolicitudesKernel,NULL);
    pthread_create(&pLargoPlazo,NULL,(void*)planificadorLargoPlazo,NULL);
    pthread_create(&pCortoPlazo,NULL,(void*)planificadorCortoPlazo,NULL);
    pthread_create(&pMedianoPlazo,NULL,(void*)planificadorMedianoPlazo,NULL);
    pthread_create(&algoritmoDeadlock, NULL, (void*)ejecutarAlgoritmoDeadlock,NULL);    
      
    pthread_join(servidor,NULL);
    pthread_join(pMedianoPlazo,NULL);
    pthread_join(pLargoPlazo,NULL);
    pthread_join(pCortoPlazo,NULL);
    pthread_join(algoritmoDeadlock,NULL);
      
     
// ------------------------------------------ 



    finalizarListas();
    finalizarSemaforosGlobales();
    finalizarDispositivosIO();
    finalizarConfig(configActual);

    log_info(logger,"[GENERAL] Se finaliza la ejecucion del modulo del kernel, cerrando todas las variables asignadas");

    log_destroy(logger);

    return 0;
}



