#include "Conexiones.h"

uint32_t recibir_operacion(uint32_t socket_cliente)
{
	uint32_t cod_op;

	if(recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) != 0){
		return cod_op;
	}else
	{
		close(socket_cliente); 
		return -1;
	}
}

void* recibir_buffer(int socket_cliente)
{
	void * buffer;
	int size;

	recv(socket_cliente, &(size), sizeof(uint32_t), MSG_WAITALL);
	buffer = malloc(size);
	recv(socket_cliente, buffer, size, MSG_WAITALL);

	return buffer;
}

void atender_solicitudes_multihilo(){

	 uint32_t servidor = iniciar_servidor(ip, puerto);
	 
	 printf("\nSe inicio el servidor.\n");
	 pthread_t cliente;
	while(1){
		uint32_t conexion_cliente = esperar_cliente(servidor);
		pthread_create(&cliente,NULL,(void*)atender_solicitudes_memoria,conexion_cliente);
		pthread_detach(cliente);
	}

}

void atender_solicitudes_memoria(uint32_t conexion){
	
	t_log* logger =  log_create("cfg/Servidor.log","Servidor",1,LOG_LEVEL_DEBUG);
	uint32_t DL;
	
	while (1){
	
		uint32_t cod_op = recibir_operacion(conexion);

		switch(cod_op){
		
			case INICIALIZAR_ESTRUCTURA: //cuando no hay kernel. Si hay nunca llega este mensaje
				inicializar_carpincho(conexion, logger);
				break;
			case MEMALLOC:
				DL = recibir_memalloc(conexion, logger);

				t_paquete* paquete = crear_paquete(MEMALLOC);
				paquete->buffer->size = sizeof(uint32_t);
				paquete->buffer->stream = malloc(paquete->buffer->size);
				int desplazamiento = 0;

				memcpy(paquete->buffer->stream + desplazamiento, &(DL) , sizeof(uint32_t));

				enviarPaquete(paquete, conexion);
					
				break;
			case MEMFREE:
				recibir_memfree(conexion, logger);
				break;
			case MEMREAD:
				recibir_memread(conexion, logger);
				break;
			case MEMWRITE:	
				recibir_memwrite(conexion, logger);
				break;
			case CERRAR_INSTANCIA:
				cerrar_carpincho(conexion, logger);
				break;
				/*casos no validos de operacion kernel */
			case INICIAR_SEMAFORO:;
				responderOperacionNoValida(conexion, INICIAR_SEMAFORO, logger);
				break;
			case SEM_WAIT:;
				responderOperacionNoValida(conexion, SEM_WAIT, logger);
				break;
			case SEM_SIGNAL:;
				responderOperacionNoValida(conexion, SEM_SIGNAL, logger);
				break;
			case CERRAR_SEMAFORO:;
				responderOperacionNoValida(conexion, CERRAR_SEMAFORO, logger);
				break;
			case CONECTAR_IO:;
				responderOperacionNoValida(conexion, CONECTAR_IO, logger);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				break;
			default:
				log_warning(logger, "Entro al default");
				break;
		}
		
		if(cod_op == CERRAR_INSTANCIA ){
			break;
		}
	}
}

void* atender_respuestas_swap(uint32_t conexion){
	
	uint32_t cod_op = recibir_operacion(conexion);

	switch(cod_op)
	{
	
	case LECTURA_PAGINA:
		return recibirPagina(conexion);
	break;
	case ESCRITURA_PAGINA:
		return recibir_respuesta_escritura(conexion);
	break;
	case CONSULTAR_ESPACIO:
		return recibir_respuesta_consulta(conexion);
	break;
	case FINALIZAR_PROCESO:
		return recibir_respuesta_cierre(conexion);
	case -1:
		log_error(logger, "el cliente se desconecto. Terminando servidor");
		break;
	default:
		log_warning(logger, "Entro al default");
		break;
	}

}


uint32_t recibir_memalloc(int socket_cliente, t_log* logger) //devuelve DL del comienzo del bloque (no del heap)
{
	uint32_t offset;
	t_memalloc *alloc = malloc(sizeof(t_memalloc));
	void* buffer = recibir_buffer(socket_cliente);
	
	memcpy(&(alloc->pid), buffer, sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(&(alloc->tamanio), buffer + offset,sizeof(uint32_t));

	free(buffer);
	log_info(logger, "\nLLego el proceso para allocar: \n Pid: %i \nTamanio: %i", alloc->pid, alloc->tamanio);
	
	uint32_t direccionLogica = administrar_allocs(alloc);

	return direccionLogica;

}

void inicializar_carpincho(int conexion ,t_log* logger){

	uint32_t size;
	recv(conexion, &size, sizeof(uint32_t), MSG_WAITALL);
	
	t_carpincho* carpincho = malloc(sizeof(t_carpincho));

		pthread_mutex_lock(controladorIds);
		carpincho->id_carpincho = id_carpincho;
		id_carpincho++;
		pthread_mutex_unlock(controladorIds);

		carpincho->tabla_de_paginas = list_create();
		carpincho->conexion = conexion;
		log_info(logger,"Agregamos nuevo carpincho a memoria, y su pid es: %d",carpincho->id_carpincho);

		pthread_mutex_lock(listaCarpinchos);
		list_add(carpinchos, carpincho);
		pthread_mutex_unlock(listaCarpinchos);

		log_info(logger,"Agregamos un carpincho a la lista de carpinchos, para que se le asigne memoria, y su pid es: %d",carpincho->id_carpincho);


		enviarInformacionAdministrativaDelProceso(carpincho);
	
}

void enviarInformacionAdministrativaDelProceso(t_carpincho* carpincho){

	t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;
	
    memcpy(paquete->buffer->stream + desplazamiento, &(carpincho->id_carpincho) , sizeof(uint32_t));
    
	enviarPaquete(paquete, carpincho->conexion);

}
uint32_t cerrar_carpincho(uint32_t conexion,t_log* logger){

	uint32_t pidProcesoAEliminar;

	void* buffer = recibir_buffer(conexion);

	memcpy(&pidProcesoAEliminar, buffer, sizeof(uint32_t));

	bool buscarMarcos(t_marco* marco){
		return marco->proceso_asignado == pidProcesoAEliminar;
	};

	pthread_mutex_lock(marcos_sem);
	t_list* marcosALiberar = list_filter(marcos, (void*)buscarMarcos);
	pthread_mutex_unlock(marcos_sem);

	void liberar_marcos(t_marco* marco){
		marco->proceso_asignado = -1;
		marco->estaLibre = true;
	};

	list_iterate(marcosALiberar, (void*)liberar_marcos);

	bool buscarProcesoPorPid(t_carpincho* carpincho){
		return carpincho->id_carpincho == pidProcesoAEliminar;
	};
	pthread_mutex_lock(listaCarpinchos);
	t_carpincho* carpincho =list_remove_by_condition(carpinchos, buscarProcesoPorPid);
	pthread_mutex_unlock(listaCarpinchos);
	log_info(logger,"Sacamos al carpincho. Pid: %d", carpincho->id_carpincho);

	informarCierreDeProceso(carpincho,logger);

	finalizar_swap(pidProcesoAEliminar);
	//loguear

	log_info(logger,"Se nos va el carpincho: %d", carpincho->id_carpincho);
	free(carpincho); 

}

void informarCierreDeProceso(t_carpincho* carpincho,t_log* loggerActual){

	t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t valorReturn = 1;


	memcpy(paquete->buffer->stream, &(valorReturn) , sizeof(uint32_t));

	log_info(loggerActual,"Enviamos que queremos cerrar el carpincho", carpincho->id_carpincho);
    enviarPaquete(paquete, carpincho->conexion);
	//cerrar hilo

}

int32_t recibir_memfree(int socket_cliente, t_log* logger) {

	uint32_t offset=0, carpincho, direccionLogica;
	void* buffer = recibir_buffer(socket_cliente);
	
	memcpy(&carpincho, buffer, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&direccionLogica, buffer + offset, sizeof(uint32_t));

	free(buffer);

	int id = obtenerId(direccionLogica);

	bool buscarCarpincho(t_carpincho* carp){
		return carp->id_carpincho == carpincho;
	};

	pthread_mutex_lock(listaCarpinchos);
	t_carpincho* capybara = list_find(carpinchos, (void*)buscarCarpincho);
	pthread_mutex_unlock(listaCarpinchos);
	
	bool buscarPagina(t_pagina* pag){
		return pag->id_pagina == id;
	};
	t_pagina* pagina = list_find(capybara->tabla_de_paginas, (void*)buscarPagina);

	if(pagina == NULL){
		return -5;
	}

	log_info(logger, "\nRecibimos memfree: \n Pid: %i \nDirecLogica: %i", carpincho, direccionLogica);

	liberar_alloc(carpincho, direccionLogica);

	t_paquete *paquete = crear_paquete(MEMFREE);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	offset=0;
	uint32_t confirmacion =1;

    memcpy(paquete->buffer->stream + offset, &confirmacion, sizeof(uint32_t));

	enviarPaquete(paquete, socket_cliente);

	return 0;
}

int32_t recibir_memread(int socket_cliente, t_log* logger) {

	int32_t offset=0, carpincho, direccion_logica, tamanio;
	void* buffer = recibir_buffer(socket_cliente);
	
	memcpy(&carpincho, buffer, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&direccion_logica, buffer + offset,sizeof(int32_t));
	offset += sizeof(uint32_t);
	memcpy(&tamanio, buffer + offset, sizeof(uint32_t));

	free(buffer);

	int id = obtenerId(direccion_logica);

	bool buscarCarpincho(t_carpincho* carp){
		return carp->id_carpincho == carpincho;
	};
	pthread_mutex_lock(listaCarpinchos);
	t_carpincho* capybara = list_find(carpinchos, (void*)buscarCarpincho);
	pthread_mutex_unlock(listaCarpinchos);

	bool buscarPagina(t_pagina* pag){
		return pag->id_pagina == id;
	};
	t_pagina* pagina = list_find(capybara->tabla_de_paginas, (void*)buscarPagina);

	if(pagina == NULL){
		return -6;
	}

	log_info(logger, "\nRecibimos memread: \n Pid: %i \nDirecLogica: %i \nTamanio", carpincho, direccion_logica, tamanio);

	void* leido = leer_memoria(direccion_logica, carpincho, tamanio);//si es invalida la direccion devolver contenido vacio

	t_paquete *paquete = crear_paquete(MEMREAD);

	paquete->buffer->size = tamanio + sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	offset=0;

	memcpy(paquete->buffer->stream, &tamanio, sizeof(uint32_t));
	offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, leido , tamanio);

	enviarPaquete(paquete, socket_cliente);

	return 0;
}

int32_t recibir_memwrite(int socket_cliente, t_log* logger) {

	int32_t offset=0, carpincho, direccion_logica, tamanio;
	void* buffer = recibir_buffer(socket_cliente);
	
	memcpy(&carpincho, buffer, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&direccion_logica, buffer + offset,sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(&tamanio, buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	void* contenido = malloc(tamanio);

	memcpy(contenido, buffer + offset, tamanio);

	free(buffer);

	int id = obtenerId(direccion_logica);

	bool buscarCarpincho(t_carpincho* carp){
		return carp->id_carpincho == carpincho;
	};
	pthread_mutex_lock(listaCarpinchos);
	t_carpincho* capybara = list_find(carpinchos, (void*)buscarCarpincho);
	pthread_mutex_unlock(listaCarpinchos);

	bool buscarPagina(t_pagina* pag){
		return pag->id_pagina == id;
	};
	t_pagina* pagina = list_find(capybara->tabla_de_paginas, (void*)buscarPagina);

	if(pagina == NULL){
		return -7;
	}

	log_info(logger, "\nRecibimos memwrite: \n Pid: %i \nDirecLogica: %i \nTamanio: %i", carpincho, direccion_logica, tamanio);

	escribir_memoria(carpincho, direccion_logica, contenido, tamanio);// Retorna un entero si se pudo escribir o no
	
	//deberia liberar el contenido
	free(contenido);

	t_paquete *paquete = crear_paquete(MEMWRITE);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	offset=0;
	uint32_t confirmacion =1;

    memcpy(paquete->buffer->stream + offset, &confirmacion, sizeof(uint32_t));

	enviarPaquete(paquete, socket_cliente);

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

void enviar_pagina(uint32_t pid, uint32_t id_pagina, void* contenido){

	t_paquete *paquete = crear_paquete(ESCRITURA_PAGINA);

	paquete->buffer->size = sizeof(uint32_t)*2 + tamanioPagina;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &(id_pagina) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, contenido , tamanioPagina);

	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);



}

uint32_t pedir_pagina(uint32_t id_pagina, uint32_t pid){

	uint32_t size;

	t_paquete *paquete = crear_paquete(LECTURA_PAGINA);

	paquete->buffer->size = sizeof(uint32_t)*2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;
	
	memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &(id_pagina) , sizeof(uint32_t));
    

	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);

	return conexionSwamp;
}

uint32_t consultar_espacio(uint32_t pid, uint32_t cantPaginas){

	uint32_t size;

	t_paquete *paquete = crear_paquete(CONSULTAR_ESPACIO);

	paquete->buffer->size = sizeof(uint32_t)*2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;
	
	memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + desplazamiento, &(cantPaginas) , sizeof(uint32_t));
    

	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);

	return conexionSwamp;
}

uint32_t finalizar_swap(uint32_t pid){

	uint32_t size;

	t_paquete *paquete = crear_paquete(FINALIZAR_PROCESO);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;
	
	memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    
	uint32_t conexionSwamp = crear_conexion(ipSWAmP, puertoSWAmP);

	enviarPaquete(paquete, conexionSwamp);

	return conexionSwamp;
}

uint32_t recibir_respuesta_cierre(conexion){
	
	uint32_t offset=0;
	void* buffer = recibir_buffer(conexion);
	uint32_t respuesta;
	
	memcpy(&respuesta, buffer, tamanioPagina);

	free(buffer);

	return respuesta;

}

void* recibirPagina(int conexion){

	uint32_t offset=0;
	void* buffer = recibir_buffer(conexion);
	void* contenido = malloc(tamanioPagina);
	
	memcpy(contenido, buffer, tamanioPagina);

	free(buffer);

	return contenido;

}

uint32_t recibir_respuesta_consulta(int conexion){

	void* buffer = recibir_buffer(conexion);
	uint32_t respuesta;

	memcpy(&respuesta, buffer, sizeof(uint32_t));

	free(buffer);

	return respuesta;
}

void* recibir_respuesta_escritura(int conexion){
	
	uint32_t offset=0;
	void* buffer = recibir_buffer(conexion);
	void* respuesta = malloc(sizeof(uint32_t));
	
	memcpy(respuesta, buffer, sizeof(uint32_t));

	free(buffer);

	return respuesta;
}


void responderOperacionNoValida(int conexion, cod_operacion tareaRealizada, t_log* logger){

	int valor = 0;

	void* buffer= recibir_buffer(conexion);

	t_paquete *paquete = crear_paquete(tareaRealizada);
	
	log_info(logger,"Esta tarea no nos corresponde realizarla a nosotros ya que se debe realizar en el KERNEL");

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento , &(valor), sizeof(uint32_t));

	enviarPaquete(paquete, conexion);

	free(buffer);
}