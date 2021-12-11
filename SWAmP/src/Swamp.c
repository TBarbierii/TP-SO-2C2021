#include "Swamp.h"

/* ---------------- SWAmP ---------------- */
int main(int argc, char *argv[]){
    
    logger_swamp = log_create("./cfg/logger_swamp.log", "SWAmP", true, LOG_LEVEL_TRACE);

    if(argc < 2){
        perror("Falta path de archivo de configuración.");
        return -1;
    }
    
    t_config* config_swamp = config_create(argv[1]);
    lista_swap_files = list_create();

    obtener_valores_config(config_swamp,logger_swamp);

    iniciar_servidor_swamp();

    log_destroy(logger_swamp);
    config_destroy(config_swamp);
    destruirArchivosSwapFiles();
    list_destroy(lista_swap_files);
}