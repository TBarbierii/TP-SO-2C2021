#include "/home/utnso/tp-2021-2c-UCM-20-SO/SWAmP/include/swamp_lib.h"
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include "shared_utils.h"
#include <semaphore.h>
#include <commons/collections/list.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

void enviar_tipo_asignacion(char* tipoAsignacion){//mandar al principio despues de leer config

	uint32_t tipo;

	if(strcmp(tipoAsignacion, "FIJA") == 0) {
		tipo = 1;
	}else if(strcmp(tipoAsignacion, "DINAMICA") == 0) {
		tipo = 0;
	}else{
		tipo = -1;
	}

	t_paquete *paquete = crear_paquete(TIPOASIGNACION);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(tipo) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

	uint32_t conexionSwamp = crear_conexion("127.0.0.1", "5001");

	enviarPaquete(paquete, conexionSwamp);


}

int main() {

    enviar_tipo_asignacion("SANTIVERGAS");
    
    return 0;
}