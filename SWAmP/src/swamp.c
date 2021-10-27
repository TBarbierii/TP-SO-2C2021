#include "swamp.h"

int main(){
    t_log* logger_swamp = log_create("./cfg/logger_swamp.log", "SWAmP", true, LOG_LEVEL_DEBUG);
    t_config* config_swamp = config_create("./cfg/swamp.config");
    lista_swap_files = list_create();

    obtener_valores_config(config_swamp,logger_swamp);

    escribirContenidoSobreElArchivo("COMBIENE",1,0,0,"/home/utnso/swap1.bin",logger_swamp);
    //ACA SOLO VA A ESTAR EL SERVIDOR, xq va a ser paginacion bajo demanda, hasta que nos manden peticiones, no hacemoas mas nada
    //iniciar_servidor_swamp();

    leer_contenido(0, 0, logger_swamp);
    
    log_destroy(logger_swamp);
    config_destroy(config_swamp);
    destruirArchivosSwapFiles();
    list_destroy(lista_swap_files);
}