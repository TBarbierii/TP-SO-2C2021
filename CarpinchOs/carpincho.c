#include "carpincho.h"
#include <stdlib.h>
#include <stdio.h>
#include <matelib.h>

int main(){
    mate_instance* referencia = malloc(sizeof(mate_instance));

    mate_init(referencia, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/configProcesos.config");
    mate_sem_init(referencia,"SEM2", 1);
    mate_sem_wait(referencia, "SEM2"); 
    mate_sem_post(referencia, "SEM2");
    mate_sem_destroy(referencia,"SEM2");

    

    mate_close(referencia);
    free(referencia);

    return 0;
}