#include "Memoria.h"

t_config* inicializarConfig(){
    return config_create("cfg/ConfiguracionMemoria.config");
}

void inicializarListas(){
    carpinchos = list_create();
}

void obtenerValoresDelConfig(t_config* configActual){

    ipSWAmP = config_get_string_value(configActual, "IP_SWAMP");
    puertoSWAmP = config_get_string_value(configActual, "PUERTO_SWAMP");
    tamanio = config_get_int_value(configActual, "TAMANIO");
    tamanioPagina = config_get_int_value(configActual, "TAMANIO_PAGINA");
    algoritmoReemplazoMMU = config_get_string_value(configActual, "ALGORITMO_REEMPLAZO_MMU");
    tipoAsignacion = config_get_string_value(configActual, "TIPO_ASIGNACION");
    marcosMaximos = config_get_int_value(configActual, "MARCOS_MAXIMOS");
    cantidadEntradasTLB = config_get_int_value(configActual, "CANTIDAD_ENTRADAS_TLB");
    algoritmoReemplazoTLB = config_get_string_value(configActual, "ALGORITMO_REEMPLAZO_TLB");
    retardoAciertoTLB = config_get_int_value(configActual, "RETARDO_ACIERTO_TLB");
    retardoFAlloTLB = config_get_int_value(configActual, "RETARDO_FALLO_TLB");
}

void finalizarConfig(t_config* configUsado){
    config_destroy(configUsado);
}

void inicializarMemoria() {
    memoriaPrincipal = malloc(tamanio);
}

void finalizarMemoria() {
}

int main(){

    t_config* configActual = inicializarConfig();
    obtenerValoresDelConfig(configActual);
    finalizarConfig(configActual);
    return 0;
}

