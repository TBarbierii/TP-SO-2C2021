#include "Memoria.h"

t_config* inicializarConfig(char* path){
    //return config_create("cfg/ConfiguracionMemoria.config");
    return config_create(path);
}

void inicializarListas(){
    carpinchos = list_create();
    marcos = list_create();
    TLB = list_create();
    carpinchosMetricas = list_create();
}

void finalizarListas(){
    list_destroy(carpinchos);
    list_destroy(marcos);
}

void obtenerValoresDelConfig(t_config* configActual){

    ip = config_get_string_value(configActual, "IP");
    puerto = config_get_string_value(configActual, "PUERTO");
    tamanio = config_get_int_value(configActual, "TAMANIO");
    tamanioPagina = config_get_int_value(configActual, "TAMANIO_PAGINA");
    algoritmoReemplazoMMU = config_get_string_value(configActual, "ALGORITMO_REEMPLAZO_MMU");
    tipoAsignacion = config_get_string_value(configActual, "TIPO_ASIGNACION");
    marcosMaximos = config_get_int_value(configActual, "MARCOS_POR_CARPINCHO");
    cantidadEntradasTLB = config_get_int_value(configActual, "CANTIDAD_ENTRADAS_TLB");
    algoritmoReemplazoTLB = config_get_string_value(configActual, "ALGORITMO_REEMPLAZO_TLB");
    retardoAciertoTLB = config_get_int_value(configActual, "RETARDO_ACIERTO_TLB");
    retardoFAlloTLB = config_get_int_value(configActual, "RETARDO_FALLO_TLB");
    ipSWAmP = config_get_string_value(configActual, "IP_SWAMP");
    puertoSWAmP = config_get_string_value(configActual, "PUERTO_SWAMP");
    pathDump = config_get_string_value(configActual, "PATH_DUMP_TLB");
}

void finalizarConfig(t_config* configUsado){
    config_destroy(configUsado);
}

void inicializarMemoria() {
    memoriaPrincipal = malloc(tamanio);
    crear_marcos();
}

void inicializarTodo(){
    
    inicializarListas();

    logsObligatorios =  log_create("cfg/logsObligatorios.log","Log",1,LOG_LEVEL_TRACE);
    loggerServidor =  log_create("cfg/Servidor.log","ServidorYSwamp",1,LOG_LEVEL_DEBUG);
    loggerMemalloc =  log_create("cfg/Memalloc.log","Memalloc",1,LOG_LEVEL_INFO);

    listaCarpinchos = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(listaCarpinchos,NULL);

    controladorIds = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(controladorIds,NULL);

    controladorIdsPaginas = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(controladorIdsPaginas,NULL);

    TLB_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(TLB_mutex ,NULL);
    
    memoria = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(memoria,NULL);

    swap =  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(swap,NULL);
    
    marcos_sem =  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(marcos_sem,NULL);

    solicitud_mutex =  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(solicitud_mutex,NULL);

    tabla_paginas =  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(tabla_paginas,NULL);
    
    recorrer_marcos_mutex =  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(recorrer_marcos_mutex,NULL);

    hits_sem=  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(hits_sem,NULL);

    miss_sem =  malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(miss_sem,NULL);

    id_carpincho = 1;

    hits_totales=0;
    miss_totales=0;
    punteroClock=0;

}

void finalizarTodo(t_config* configActual){
   
    finalizarConfig(configActual);
    finalizarMemoria();
    finalizarListas();


    pthread_mutex_destroy(listaCarpinchos);
    free(listaCarpinchos);
    pthread_mutex_destroy(controladorIds);
    free(controladorIds);
    pthread_mutex_destroy(controladorIdsPaginas);
    free(controladorIdsPaginas);
    pthread_mutex_destroy(TLB_mutex);
    free(TLB_mutex);
    pthread_mutex_destroy(memoria);
    free(memoria);
    pthread_mutex_destroy(swap);
    free(swap);
    pthread_mutex_destroy(marcos_sem);
    free(marcos_sem);


    
}

void finalizarMemoria() {

    pthread_mutex_lock(marcos_sem);
    while(!list_is_empty(marcos)){
        t_marco* marcoARemover = list_remove(marcos, 0);
        free(marcoARemover);
    }
    pthread_mutex_unlock(marcos_sem);


    pthread_mutex_lock(memoria);
    free(memoriaPrincipal);
    pthread_mutex_unlock(memoria);

}

int main(int argc, char *argv[]){

    //t_config* configActual = inicializarConfig();

     if(argc < 2){
        perror("Falta path de archivo de configuración.");
        return -1;
    } 

    t_config* configActual = inicializarConfig(argv[1]);

    obtenerValoresDelConfig(configActual);
    inicializarTodo();
    inicializarMemoria();

    if (signal(SIGINT, manejador_de_seniales) == SIG_ERR)
	{
		perror ("No se puede cambiar signal");
	}
    if (signal(SIGUSR1, manejador_de_seniales) == SIG_ERR)
	{
		perror ("No se puede cambiar signal");
	}
    if (signal(SIGUSR2, manejador_de_seniales) == SIG_ERR)
	{
		perror ("No se puede cambiar signal");
	}

    enviar_tipo_asignacion(tipoAsignacion);

    atender_solicitudes_carpinchos();
    
    finalizarTodo(configActual);
    
    return 0;
}

