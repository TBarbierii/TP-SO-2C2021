#include "Memoria.h"

t_config* inicializarConfig(){
    return config_create("cfg/ConfiguracionMemoria.config");
}

void inicializarListas(){
    carpinchos = list_create();
    marcos = list_create();
    TLB = list_create();
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
    marcosMaximos = config_get_int_value(configActual, "MARCOS_POR_PROCESO");
    cantidadEntradasTLB = config_get_int_value(configActual, "CANTIDAD_ENTRADAS_TLB");
    algoritmoReemplazoTLB = config_get_string_value(configActual, "ALGORITMO_REEMPLAZO_TLB");
    retardoAciertoTLB = config_get_int_value(configActual, "RETARDO_ACIERTO_TLB");
    retardoFAlloTLB = config_get_int_value(configActual, "RETARDO_FALLO_TLB");
    ipSWAmP = config_get_string_value(configActual, "IP_SWAMP");
    puertoSWAmP = config_get_string_value(configActual, "PUERTO_SWAMP");
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

    id_pag = 1;
    id_carpincho = 0;

    hits_totales=0;
    miss_totales=0;

}

void finalizarTodo(t_config* configActual){
   
    finalizarConfig(configActual);
    finalizarMemoria();
    finalizarListas();


    pthread_mutex_destroy(listaCarpinchos);
    free(listaCarpinchos);
    pthread_mutex_destroy(controladorIds);
    free(controladorIds);
    
}

void finalizarMemoria() {
    while(!list_is_empty(marcos)){
        t_marco* marcoARemover = list_remove(marcos, 0);
        free(marcoARemover);
    }
    free(memoriaPrincipal);

}

int main(){

    t_config* configActual = inicializarConfig();
    obtenerValoresDelConfig(configActual);
    inicializarTodo();
    inicializarMemoria();

    if (signal(SIGINT, manejador_de_seniales) == SIG_ERR)
	{
		perror ("No se puede cambiar signal");
	}

    //enviar_tipo_asignacion(tipoAsignacion);
    //enviar_pagina(0, 0, "buenass--------------buenass--------------");

    atender_solicitudes_multihilo();
    /*
    t_memalloc *alloc = malloc(sizeof(t_memalloc));
    
    alloc->pid = 6;
    alloc->tamanio = 45;

     uint32_t direccionLogica = administrar_allocs(alloc);

    alloc->pid = 1;
    alloc->tamanio = 43;

    uint32_t direccionLogica1 = administrar_allocs(alloc);

    alloc->pid = 1;
    alloc->tamanio = 35;

    uint32_t direccionLogica2 = administrar_allocs(alloc);
    
    alloc->pid = 1;
    alloc->tamanio = 10;

    uint32_t direccionLogica3 = administrar_allocs(alloc);



    uint32_t direccionLogica = administrar_allocs(alloc);

    alloc->pid = 2;
    alloc->tamanio = 20;

    uint32_t direccionLogica1 = administrar_allocs(alloc);

    alloc->pid = 3;
    alloc->tamanio = 30;

    uint32_t direccionLogica2 = administrar_allocs(alloc);

    alloc->pid = 2;
    alloc->tamanio = 26;

    uint32_t direccionLogica3 = administrar_allocs(alloc);

    alloc->pid = 3;
    alloc->tamanio = 15;

    uint32_t direccionLogica4 = administrar_allocs(alloc);

        alloc->pid = 6;
    alloc->tamanio = 43;

    uint32_t direccionLogica5 = administrar_allocs(alloc);

        alloc->pid = 3;
    alloc->tamanio = 30;

    uint32_t direccionLogica6 = administrar_allocs(alloc);

        alloc->pid = 2;
    alloc->tamanio = 10;

    uint32_t direccionLogica7 = administrar_allocs(alloc);

        alloc->pid = 6;
    alloc->tamanio = 35;

    uint32_t direccionLogica8 = administrar_allocs(alloc);

    alloc->pid = 2;
    alloc->tamanio = 16;

    uint32_t direccionLogica9 = administrar_allocs(alloc);

            alloc->pid = 3;
    alloc->tamanio = 15;

    uint32_t direccionLogica10 = administrar_allocs(alloc);

            alloc->pid = 2;
    alloc->tamanio = 30;

    uint32_t direccionLogica11 = administrar_allocs(alloc);

    alloc->pid = 6;
    alloc->tamanio = 10;

    uint32_t direccionLogica12 = administrar_allocs(alloc);

            alloc->pid = 3;
    alloc->tamanio = 10;

    uint32_t direccionLogica13 = administrar_allocs(alloc);

    printf("\nCantidad de miss totates %i", miss_totales);
    printf("\nCantidad de hits totates %i", hits_totales);

    //liberar_alloc(8, direccionLogica2);
    
    t_memalloc *alloc4 = malloc(sizeof(t_memalloc));
    alloc4->pid = 8;
    alloc4->tamanio =5;

    //uint32_t direccionLogica4 = administrar_allocs(alloc4); 

    void* prueba  = "-----------------------------------------1-45";
    void* prueba1 = "----------------2-20";
    void* prueba2 = "--------------------------3-30";
    void* prueba3 = "----------------------2-26";
    void* prueba4 = "-----------3-15";
    void* prueba5 = "---------------------------------------1-43";
    void* prueba6 = "--------------------------3-30";
    void* prueba7 = "------2-10";
    void* prueba8 = "-------------------------------1-35";
    void* prueba9 = "------------2-16";
    void* prueba10 = "-----------3-15";
    void* prueba11 = "--------------------------2-30";
    void* prueba12 = "------1-10";
    void* prueba13 = "------3-10";


    escribir_memoria(1, direccionLogica, prueba, 45);
    escribir_memoria(2, direccionLogica1, prueba1, 20);
    escribir_memoria(3, direccionLogica2, prueba2, 30);
    escribir_memoria(2, direccionLogica3, prueba3, 26);
    escribir_memoria(3, direccionLogica4, prueba4, 15);
    escribir_memoria(1, direccionLogica5, prueba5, 43);
    escribir_memoria(3, direccionLogica6, prueba6, 30);
    escribir_memoria(2, direccionLogica7, prueba7, 10);
    escribir_memoria(1, direccionLogica8, prueba8, 35);
    escribir_memoria(2, direccionLogica9, prueba9, 16);
    escribir_memoria(3, direccionLogica10, prueba10, 15);
    escribir_memoria(2, direccionLogica11, prueba11, 30);
    escribir_memoria(1, direccionLogica12, prueba12, 10);
    escribir_memoria(3, direccionLogica13, prueba13, 10);

    void* hola = leer_memoria(direccionLogica, 1, 45);
    void* hola1 = leer_memoria(direccionLogica1, 2, 20);
    void* hola2 = leer_memoria(direccionLogica2, 3, 30);
    void* hola3 = leer_memoria(direccionLogica3, 2, 26);
    void* hola4 = leer_memoria(direccionLogica4, 3, 15);
    void* hola5 = leer_memoria(direccionLogica5, 1, 43);
    void* hola6 = leer_memoria(direccionLogica6, 3, 30);
    void* hola7 = leer_memoria(direccionLogica7, 2, 10);
    void* hola8 = leer_memoria(direccionLogica8, 1, 35);
    void* hola9 = leer_memoria(direccionLogica9, 2, 16);
    void* hola10 = leer_memoria(direccionLogica10, 3, 15);
    void* hola11 = leer_memoria(direccionLogica11, 2, 30);
    void* hola12 = leer_memoria(direccionLogica12, 1, 10);
    void* hola13 = leer_memoria(direccionLogica13, 3, 10);

    printf("\nSe leyo: %s", (char*)hola);
    printf("\nSe leyo: %s", (char*)hola1);
    printf("\nSe leyo: %s", (char*)hola2);
    printf("\nSe leyo: %s", (char*)hola3);
    printf("\nSe leyo: %s", (char*)hola4);
    printf("\nSe leyo: %s", (char*)hola5);
    printf("\nSe leyo: %s", (char*)hola6);
    printf("\nSe leyo: %s", (char*)hola7);
    printf("\nSe leyo: %s", (char*)hola8);
    printf("\nSe leyo: %s", (char*)hola9);
    printf("\nSe leyo: %s", (char*)hola10);
    printf("\nSe leyo: %s", (char*)hola11);
    printf("\nSe leyo: %s", (char*)hola12);
    printf("\nSe leyo: %s\n", (char*)hola13);

    */
    finalizarTodo(configActual);
    return 0;
}

