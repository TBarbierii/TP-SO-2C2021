#include "Conexiones.h"

uint32_t recibir_operacion(uint32_t socket_cliente)
{
	uint32_t cod_op;

	if(recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) != 0){
		return cod_op;
	}else
	{
		close(socket_cliente); //que onda esto? si tenemos las conexiones abiertas con matelib no habria que cerrarla.
		return -1;
	}
}

void* recibir_buffer(uint32_t * size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void atender_solicitudes_multihilo(char* ip_servidor, char* puerto){
	//crea un hilo por cada cliente que se conecta y lo atiende. El servidor ya estaria levantado

	 uint32_t servidor = iniciar_servidor(ip_servidor, puerto);
	 pthread_t* cliente;
	while(1){
		uint32_t conexion_cliente = esperar_cliente(servidor);
		pthread_create(&cliente,NULL,(void*)atender_solicitudes_memoria,conexion_cliente);
		pthread_detach(cliente);
	}

}

void atender_solicitudes_memoria(uint32_t conexion){
	uint32_t cod_op = recibir_operacion(conexion);
	t_log* logger =  log_create("cfg/Servidor.log","Servidor",0,LOG_LEVEL_DEBUG);

	switch(cod_op)
	{
	
	case INICIALIZAR_ESTRUCTURA: //cuando no hay kernel. Si hay nunca llega este mensaje
		inicializar_carpincho(conexion, logger);
	case MEMALLOC:
		recibir_memalloc(conexion);	
	case MEMFREE:
		recibir_memfree(conexion);
	case MEMREAD:
		recibir_memread(conexion);
	case MEMWRITE:	
		recibir_memwrite(conexion);
	case CERRAR_INSTANCIA:
		cerrar_carpincho(conexion, logger);
		break;
	case -1:
		log_error(logger, "el cliente se desconecto. Terminando servidor");
		break;
	default:
		log_warning(logger, "Entro al default");
		break;
	}
}


uint32_t recibir_memalloc(int socket_cliente) //devuelve DL del comienzo del bloque (no del heap)
{
	uint32_t size, offset;
	t_memalloc *alloc = malloc(sizeof(t_memalloc));
	void* buffer = recibir_buffer(&size, socket_cliente);
	
	memcpy(alloc->pid, buffer, sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(alloc->tamanio, buffer + offset,sizeof(uint32_t));

	free(buffer);
	printf("\nLLego el proceso para allocar: \n Pid: %i \nTamanio: %i", alloc->pid, alloc->tamanio);
	
	uint32_t direccionLogica = administrar_allocs(alloc);



}

void inicializar_carpincho(int conexion ,t_log* logger){


	t_carpincho* carpincho = malloc(sizeof(t_carpincho));
	
		carpincho->id_carpincho = generadorIdsCarpinchos();
		carpincho->tabla_de_paginas = list_create();
		carpincho->allocs = list_create();
		carpincho->conexion = conexion;
		log_info(logger,"Agregamos nuevo carpincho a memoria, y su pid es: %d",carpincho->id_carpincho);

		//mutex
		list_add(carpinchos, carpincho);
		//mutex
		log_info(logger,"Agregamos un carpincho a la lista de carpinchos, para que se le asigne memoria, y su pid es: %d",carpincho->id_carpincho);


		enviarInformacionAdministrativaDelProceso(carpincho);
	
}

void enviarInformacionAdministrativaDelProceso(t_carpincho* carpincho){

	t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    paquete->buffer->size = sizeof(uint32_t) *2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t valorBackEnd = MEMORIA;
    int desplazamiento = 0;
	
    memcpy(paquete->buffer->stream + desplazamiento, &(carpincho->id_carpincho) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(valorBackEnd) , sizeof(uint32_t));
    
	enviarPaquete(paquete, carpincho->conexion);

}
uint32_t cerrar_carpincho(uint32_t conexion,t_log* logger){

	uint32_t pidProcesoAEliminar;
	uint32_t size;

	void* buffer = recibir_buffer(&size, conexion);

	memcpy(&pidProcesoAEliminar, buffer, sizeof(uint32_t));

	bool buscarProcesoPorPid(t_carpincho* carpincho){
		return carpincho->id_carpincho == pidProcesoAEliminar;
	};

	t_carpincho* carpincho =list_remove_by_condition(carpinchos, buscarProcesoPorPid);
	log_info(logger,"Sacamos al carpincho. Pid: %d", carpincho->id_carpincho);

	informarCierreDeProceso(carpincho,logger);

	log_info(logger,"Se nos va el carpincho: %d", carpincho->id_carpincho);
	free(carpincho); 

}

void informarCierreDeProceso(t_carpincho* carpincho,t_log* loggerActual){

	t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t valorReturn = 0;


	memcpy(paquete->buffer->stream, &(valorReturn) , sizeof(uint32_t));

	log_info(loggerActual,"Enviamos que queremos cerrar el carpincho");
    enviarPaquete(paquete, carpincho->conexion);
	//cerrar conexion?

}

uint32_t recibir_memfree(int socket_cliente) {

	uint32_t size, offset=0, carpincho, direccionLogica;
	void* buffer = recibir_buffer(&size, socket_cliente);
	
	memcpy(&carpincho, buffer, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&direccionLogica, buffer + offset, sizeof(uint32_t));

	free(buffer);

	printf("\nRecibimos memfree: \n Pid: %i \nDirecLogica: %i", carpincho, direccionLogica);

	liberar_alloc(carpincho, direccionLogica);

	return 0;
}

uint32_t recibir_memread(int socket_cliente) {

	uint32_t size, offset, carpincho, direccion_logica, tamanio;
	void* buffer = recibir_buffer(&size, socket_cliente);
	
	memcpy(&carpincho, buffer, sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(&direccion_logica, buffer + offset,sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(&tamanio, buffer + offset, sizeof(uint32_t));

	free(buffer);

	printf("\nRecibimos memread: \n Pid: %i \nDirecLogica: %i \nTamanio", carpincho, direccion_logica, tamanio);

	void* leido = leer_memoria(direccion_logica, carpincho, tamanio);
	//enviar_contenido_leido(); Retorna un void*

	return 0;
}

uint32_t recibir_memwrite(int socket_cliente) {

	uint32_t size, offset, carpincho, direccion_logica, tamanio;
	void* buffer = recibir_buffer(&size, socket_cliente);
	
	memcpy(&carpincho, buffer, sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(&direccion_logica, buffer + offset,sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(&tamanio, buffer + offset, sizeof(uint32_t));
	offset =+ sizeof(uint32_t);

	void* contenido = malloc(tamanio);

	memcpy(contenido, buffer + offset, tamanio);

	free(buffer);

	printf("\nRecibimos memwrite: \n Pid: %i \nDirecLogica: %i \nTamanio", carpincho, direccion_logica, tamanio);

	//escribir_memoria(carpincho, direccion_logica, contenido, tamanio); Retorna un entero si se pudo escribir o no

	return 0;
}



/* mensajes con swamp */

void enviar_tipo_asignacion(char* tipoAsignacion){//mandar al principio despues de leer config

	uint32_t tipo;

	if(strcmp(tipoAsignacion, "FIJA") == 0) tipo = 1;
	if(strcmp(tipoAsignacion, "DINAMICA") == 0) tipo = 0;

	t_paquete *paquete = crear_paquete(TIPOASIGNACION);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(tipo) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);

}

void enviar_pagina(uint32_t id_pagina, void* contenido){

	t_paquete *paquete = crear_paquete(ENVIAR_PAGINA);

	paquete->buffer->size = sizeof(uint32_t) + tamanioPagina;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(id_pagina) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, contenido , tamanioPagina);

	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);



}

void pedir_pagina(uint32_t id_pagina){

	uint32_t size;

	t_paquete *paquete = crear_paquete(ENVIAR_PAGINA);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(id_pagina) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);

	void* buffer = recibir_buffer(&size, conexionSwamp);

	//t_pagina *pagina = 

	//desarmar buffer. armar pagina y ponerla de nuevo en memoria


}