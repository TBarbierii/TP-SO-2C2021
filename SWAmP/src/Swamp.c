#include "Swamp.h"

int main(){
    logger_swamp = log_create("./cfg/logger_swamp.log", "SWAmP", true, LOG_LEVEL_DEBUG);
    t_config* config_swamp = config_create("./cfg/swamp.config");
    lista_swap_files = list_create();

    obtener_valores_config(config_swamp,logger_swamp);

    iniciar_servidor_swamp();

    log_destroy(logger_swamp);
    config_destroy(config_swamp);
    destruirArchivosSwapFiles();
    list_destroy(lista_swap_files);
}