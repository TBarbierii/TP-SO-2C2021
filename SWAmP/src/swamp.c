#include "swamp.h"
#include "swamp_lib.c"

int main(){
    t_log* logger = log_create("./cfg/proceso3.log", "PROCESO3", true, LOG_LEVEL_INFO);
    log_info(logger, "Soy el proceso 3! %s", mi_funcion_compartida());
    log_destroy(logger);

}