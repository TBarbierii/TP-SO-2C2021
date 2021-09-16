#include "matelib.h"

int main(){
    t_log* logger = log_create("./cfg/proceso1.log", "PROCESO1", true, LOG_LEVEL_INFO);
    log_info(logger, "Soy el proceso 1! %s", mi_funcion_compartida());
    log_destroy(logger);
}