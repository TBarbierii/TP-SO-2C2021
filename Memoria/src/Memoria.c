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

    id_pag = 1;
    id_carpincho = 1;
    id_marco = 0;
    hits_totales=0;
    miss_totales=0;
}

void finalizarTodo(t_config* configActual){
   
    finalizarConfig(configActual);
    finalizarMemoria();
    finalizarListas();
    pthread_mutex_destroy(listaCarpinchos);
    free(listaCarpinchos);
    
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

    enviar_tipo_asignacion(tipoAsignacion);
    //enviar_pagina(0, 0, "buenass--------------buenass--------------");

    //atender_solicitudes_multihilo();

    t_memalloc *alloc = malloc(sizeof(t_memalloc));
    
    alloc->pid = 1;
    alloc->tamanio = 45;


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

        alloc->pid = 1;
    alloc->tamanio = 43;

    uint32_t direccionLogica5 = administrar_allocs(alloc);

        alloc->pid = 3;
    alloc->tamanio = 30;

    uint32_t direccionLogica6 = administrar_allocs(alloc);

        alloc->pid = 2;
    alloc->tamanio = 10;

    uint32_t direccionLogica7 = administrar_allocs(alloc);

        alloc->pid = 1;
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

            alloc->pid = 1;
    alloc->tamanio = 10;

    uint32_t direccionLogica12 = administrar_allocs(alloc);

            alloc->pid = 3;
    alloc->tamanio = 10;

    uint32_t direccionLogica13 = administrar_allocs(alloc);

    //liberar_alloc(8, direccionLogica2);
    
    t_memalloc *alloc4 = malloc(sizeof(t_memalloc));
    alloc4->pid = 8;
    alloc4->tamanio =5;

    //uint32_t direccionLogica4 = administrar_allocs(alloc4); 

    void* prueba  = "--------------------------------------------------------------------70";
    void* prueba1 = "------------------20";
    void* prueba2 = "------------------------------------------------50";
    void* prueba3 = "---------------------------------35";

    escribir_memoria(8, direccionLogica, prueba, 70);

    escribir_memoria(8, direccionLogica1, prueba1, 20);
    escribir_memoria(8, direccionLogica2, prueba2, 50);
    escribir_memoria(8, direccionLogica3, prueba3, 35);
    
    void* hola = leer_memoria(direccionLogica, 8, 70);

    //t_pagina *pagina = list_get(carpincho->tabla_de_paginas, 0);

    //enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, hola);

 
    void* hola1 = leer_memoria(direccionLogica1, 8, 20);
    void* hola2 = leer_memoria(direccionLogica2, 8, 50);
    void* hola3 = leer_memoria(direccionLogica3, 8, 35);

    printf("\nSe leyo: %s", (char*)hola);
    printf("\nSe leyo: %s", (char*)hola1);
    printf("\nSe leyo: %s", (char*)hola2);
    printf("\nSe leyo: %s\n", (char*)hola3);
   

    finalizarTodo(configActual);
    return 0;
}

