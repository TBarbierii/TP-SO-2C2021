#include "conexiones.h"


int iniciar_servidor_swamp() {

	t_log* logger =  log_create("cfg/ServidorActual.log","Servidor",0,LOG_LEVEL_DEBUG);

	int servidor = iniciar_servidor(ip_swap, puerto_swap); // devuelve el socket del servidor
	
	log_info(logger,"Inicializamos el servidor para que se nos una la RAM");

	while(1){
		int conexion = esperar_cliente(servidor);
		log_info(logger,"Se unio un carpincho");
		recibir_operacion(conexion);
	}

	log_destroy(logger);
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

	t_log* logger =  log_create("cfg/ServidorActual.log","Servidor",0,LOG_LEVEL_DEBUG);

	t_paquete* paquete = malloc(sizeof(paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_error(logger,"Fallo en recibir la info de la conexion");
		return -1;
	}

	log_info(logger,"Recibimos la informacion de la RAM");
	log_info(logger,"El codigo de operacion es: %d",paquete->codigo_operacion);

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);
	log_info(logger,"El tamaño del paquete es %d", paquete->buffer->size);


	if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
	}
	

	switch(paquete->codigo_operacion){
        case ENVIAR_PAGINA:;
			//enviar_pagina(uint32_t id_pagina, void* contenido, int conexion);
			break;

        case RECIBIR_PAGINA:;
			// Faltaria modificar para recibir el PID
			//recibir_pagina(uint32_t id_pagina);
        	break;

		case TIPOASIGNACION:;
			//recibir_tipo_asignacion()
			close(conexion);
       		break;

		default:;
		log_info(logger,"No se metio por ningun lado wtf");
		break;
	}
	
	
	int valorOperacion = paquete->codigo_operacion;

    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
	log_destroy(logger);

	return valorOperacion;
	
}
// Habria otra forma para mandarlo para manejar los tipos de asignacion?
uint32_t recibir_tipo_asignacion(t_buffer* buffer) {

	void* data = buffer->stream;
	uint32_t tipo;

	memcpy(&(tipo), data, sizeof(uint32_t));

	if(tipo == 1) {
		tipo_asignacion = malloc(sizeof(char) * 4 + 1);	
		strcpy(tipo_asignacion, "FIJA");
	} 
	if(tipo == 0) {
		tipo_asignacion = malloc(sizeof(char) * 8 + 1);	
		strcpy(tipo_asignacion, "DINAMICA");
	} 
	return tipo;
}
// Faltaria modificar para recibir el PID
void recibir_pagina(t_buffer* buffer) {

	void* data = buffer->stream;
	void* contenido;
	int desplazamiento = 0;
	uint32_t id_pagina;

	memcpy(&(id_pagina), data + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contenido), data + desplazamiento , tamanio_pagina);

	// Deberia tener el PID para ver en que archivo guardar la pagina

}

void enviar_pagina(uint32_t id_pagina, void* contenido, int conexion) {

    t_paquete *paquete = crear_paquete(ENVIAR_PAGINA);

	//contenido = buscar_pagina(uint32_t id_pagina);

	paquete->buffer->size = sizeof(uint32_t) + tamanio_pagina;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(id_pagina) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, contenido , tamanio_pagina);

	enviarPaquete(paquete, conexion);
}
