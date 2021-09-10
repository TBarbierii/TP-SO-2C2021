#include "Modulo2.h"

int main(){
    t_log* logger = log_create("./cfg/proceso2.log", "PROCESO1", true, LOG_LEVEL_INFO);
    log_info(logger, "Soy el proceso 2! %s", mi_funcion_compartida());
    log_destroy(logger);
}