#include "Memoria.h"

t_config* inicializarConfig(){
    return config_create("cfg/ConfiguracionMemoria.config");
}

void inicializarListas(){
    carpinchos = list_create();
    marcos = list_create();
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
}

void finalizarMemoria() {
}

int main(){

    t_config* configActual = inicializarConfig();
    obtenerValoresDelConfig(configActual);
    inicializarTodo();
    inicializarMemoria();

    //enviar_tipo_asignacion(tipoAsignacion);
    //enviar_pagina(0, 0, "buenass--------------");

    //atender_solicitudes_multihilo();

    t_memalloc *alloc = malloc(sizeof(t_memalloc));
    alloc->pid = 8;
    alloc->tamanio = 70;

        t_memalloc *alloc1 = malloc(sizeof(t_memalloc));
    alloc1->pid = 8;
    alloc1->tamanio =20 ;

     t_carpincho* carpincho = malloc(sizeof(t_carpincho));
     carpincho->id_carpincho = alloc->pid;
     carpincho->tabla_de_paginas = list_create();
     carpincho->allocs = list_create();
list_add(carpinchos, carpincho);

        t_memalloc *alloc2 = malloc(sizeof(t_memalloc));
    alloc2->pid = 2;
    alloc2->tamanio = 50;
         t_carpincho* carpincho1 = malloc(sizeof(t_carpincho));
     carpincho1->id_carpincho = alloc2->pid;
     carpincho1->tabla_de_paginas = list_create();
     carpincho1->allocs = list_create();
     list_add(carpinchos, carpincho1);

        t_memalloc *alloc3 = malloc(sizeof(t_memalloc));
    alloc3->pid = 7;
    alloc3->tamanio = 19;
         t_carpincho* carpincho2 = malloc(sizeof(t_carpincho));
     carpincho2->id_carpincho = alloc3->pid;
     carpincho2->tabla_de_paginas = list_create();
     carpincho2->allocs = list_create();
     list_add(carpinchos, carpincho2);

    t_memalloc *alloc4 = malloc(sizeof(t_memalloc));
    alloc3->pid = 12;
    alloc3->tamanio = 20;

    t_carpincho* carpincho4 = malloc(sizeof(t_carpincho));
     carpincho4->id_carpincho = alloc4->pid;
     carpincho4->tabla_de_paginas = list_create();
     carpincho4->allocs = list_create();
     list_add(carpinchos, carpincho4);




    

    uint32_t direccionLogica = administrar_allocs(alloc);

    uint32_t direccionLogica2 = administrar_allocs(alloc2);

    uint32_t direccionLogica3 = administrar_allocs(alloc3);
   
    uint32_t direccionLogica1 = administrar_allocs(alloc1);

    uint32_t direcLogica4 = administrar_allocs(alloc4);

    void* prueba = "Esta es una prueba de un texto de tre paginas osea como max 70 bytes\0";
    void* prueba1 = "carpincho 8, de 20\0";
    void* prueba2 = "carpincho pid 2 dreservo alloc de 50 bytes\0";
    void* prueba3 = "carpin 7 pidio 19\0";

    escribir_memoria(8, direccionLogica, prueba, 70);

    escribir_memoria(8, direccionLogica1, prueba1, 20);
    escribir_memoria(2, direccionLogica2, prueba2, 44);
    escribir_memoria(7, direccionLogica3, prueba3, 19);
    
    void* hola = leer_memoria(direccionLogica, 8, 70);

    t_pagina *pagina = list_get(carpincho->tabla_de_paginas, 0);

    enviar_pagina(carpincho->id_carpincho, pagina->id_pagina, hola);

 
    void* hola1 = leer_memoria(direccionLogica1, 8, 20);
    void* hola2 = leer_memoria(direccionLogica2, 2, 44);
    void* hola3 = leer_memoria(direccionLogica3, 7, 19);

    printf("\nSe leyo: %s", (char*)hola);
    printf("\nSe leyo: %s", (char*)hola1);
    printf("\nSe leyo: %s", (char*)hola2);
    printf("\nSe leyo: %s\n", (char*)hola3);


    finalizarConfig(configActual);
    return 0;
}

