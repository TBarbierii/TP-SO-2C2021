#include "conexiones.h"


int iniciar_servidor_swamp() {

	int socket_swamp;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_swap, puerto_swap, &hints, &servinfo);

	socket_swamp = socket(servinfo->ai_family, 
    	                	servinfo->ai_socktype,
        	            	servinfo->ai_protocol);

	bind(socket_swamp, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_swamp, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a ram");

    return socket_swamp;
}

uint32_t recibir_operacion(uint32_t socket_cliente) {
	
    uint32_t cod_op;

	if(recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) != 0){
		return cod_op;
	}else{
		close(socket_cliente);
		return -1;
	}
}

void atender_conexion_ram(uint32_t conexion) {
    
    uint32_t cod_op = recibir_operacion(conexion);
	
	switch(cod_op) {

	case TIPOASIGNACION:
		//recibir_tipo_asignacion()
		break;
	case ENVIAR_PAGINA:
		//enviar_pagina(uint32_t id_pagina);
		break;
    case RECIBIR_PAGINA:
		//recibir_pagina(uint32_t id_pagina);
		break;
	case -1:
		log_error(logger, "el cliente se desconecto. Terminando servidor");
		break;
	default:
		log_warning(logger, "Entro al default");
		break;
	}
}

void recibir_tipo_asignacion() {



}

void enviar_pagina(uint32_t id_pagina, void* contenido) {

    t_paquete *paquete = crear_paquete(ENVIAR_PAGINA);

	paquete->buffer->size = sizeof(uint32_t) + tamanio_pagina;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(id_pagina) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, contenido , tamanio_pagina);

	uint32_t conexion_ram = crear_conexion(ip_ram, puerto_ram);

	enviarPaquete(paquete, conexion_ram);
    // Creo que deberia ser distinto
}

void recibir_pagina(uint32_t id_pagina) {

}