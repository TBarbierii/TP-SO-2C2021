#include "conexiones.h"


int iniciar_servidor_swamp() {

	t_log* logger_nuevo =  log_create("cfg/Servidor.log","Servidor",1,LOG_LEVEL_DEBUG);

	int servidor = iniciar_servidor(ip_swap, puerto_swap); // devuelve el socket del servidor
	
	log_info(logger_nuevo,"Inicializamos el servidor para que se nos una la RAM");

	while(1){
		int conexion = esperar_cliente(servidor);
		log_info(logger_nuevo,"Nos llega una solicitud de memoria para realizar algo");
		atender_mensaje_ram(conexion);
	}

	log_destroy(logger_nuevo);
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

int atender_mensaje_ram(int conexion) {

	t_log* logger_servidor =  log_create("cfg/OperacionesServidor.log","a",1,LOG_LEVEL_DEBUG);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_error(logger_servidor,"Fallo en recibir la info de la conexion");
		return -1;
	}

	log_info(logger_servidor,"Recibimos la informacion de la RAM");
	log_info(logger_servidor,"El codigo de operacion es: %d",paquete->codigo_operacion);

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);
	log_info(logger_servidor,"El tamaño del paquete es %d", paquete->buffer->size);


	if(paquete->buffer->size > 0){
		paquete->buffer->stream = malloc(paquete->buffer->size);
		recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
	}
	

	switch(paquete->codigo_operacion){

        case ENVIAR_PAGINA:;
			atender_solicitud_pedido_de_pagina(paquete->buffer, conexion);
			break;

        case RECIBIR_PAGINA:;
			recibir_pagina(paquete->buffer);
			notificar_escritura_de_pagina(conexion);
        	break;

		case TIPOASIGNACION:;
			recibir_tipo_asignacion(paquete->buffer, logger_servidor);
			close(conexion);
       		break;

		default:;
		log_info(logger_servidor,"No se metio por ningun lado wtf");
		break;



	}
	
	
	int valorOperacion = paquete->codigo_operacion;

    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
	log_destroy(logger_servidor);

	return valorOperacion;
	
}
// Habria otra forma para mandarlo para manejar los tipos de asignacion?
uint32_t recibir_tipo_asignacion(t_buffer* buffer, t_log* logger) {

	void* data = buffer->stream;

	memcpy(&(tipo_asignacion), data, sizeof(uint32_t));
	
	if(tipo_asignacion == 0) {
		log_info(logger, "Tipo de asignacion es DINAMICA");
	}else if(tipo_asignacion == 1) {
		log_info(logger, "Tipo de asignacion es FIJA");		
	}else{
		log_info(logger, "Tipo de asignacion incorrecto wtf");
	}
	
	return tipo_asignacion;
}

void recibir_pagina(t_buffer* buffer) {

	void* data = buffer->stream;
	void* contenido;
	int desplazamiento = 0;
	uint32_t id_pagina;
	uint32_t PID;

	memcpy(&(PID), data + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(id_pagina), data + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contenido), data + desplazamiento , tamanio_pagina);

	escribirContenido(contenido, id_pagina, PID, logger_swamp);

}


void enviar_pagina(void* contenido, int conexion) {

    t_paquete *paquete = crear_paquete(ENVIAR_PAGINA);

	paquete->buffer->size = tamanio_pagina;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

    memcpy(paquete->buffer->stream + desplazamiento, contenido , tamanio_pagina);

	enviarPaquete(paquete, conexion);
}

void atender_solicitud_pedido_de_pagina(t_buffer* buffer, int conexion) {

	void* data = buffer->stream;
	int desplazamiento = 0;
	uint32_t id_pagina;
	uint32_t PID;

	memcpy(&(PID), data + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(id_pagina), data + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

	leer_contenido(PID, id_pagina, conexion, logger_swamp);

}

void notificar_escritura_de_pagina(int conexion) {

	t_paquete *paquete = crear_paquete(RECIBIR_PAGINA);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;
	int valor = 1;

    memcpy(paquete->buffer->stream + desplazamiento, &(valor), sizeof(uint32_t));

	enviarPaquete(paquete, conexion);

}