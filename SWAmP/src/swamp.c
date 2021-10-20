#include "swamp.h"

int main(){
    t_log* logger_swamp = log_create("./cfg/logger_swamp.log", "SWAmP", true, LOG_LEVEL_INFO);
    t_config* config_swamp = config_create("./cfg/swamp.config");
    lista_swap_files = list_create();

    obtener_valores_config(config_swamp);



    free(tipo_asignacion);
    log_destroy(logger_swamp);
    config_destroy(config_swamp);
    list_destroy(lista_swap_files);
}