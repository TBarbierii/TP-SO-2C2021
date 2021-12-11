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

void atender_solicitudes_carpinchos(){

	 uint32_t servidor = iniciar_servidor(ip, puerto);
	 
	 log_info(loggerServidor, "SE INICIO EL SERVIDOR");
	 pthread_t cliente;
	while(1){
		uint32_t conexion_cliente = esperar_cliente(servidor);
		pthread_create(&cliente,NULL,(void*)atender_solicitudes_memoria,conexion_cliente);
		pthread_detach(cliente);
	}

}

void atender_solicitudes_memoria(uint32_t conexion){
	
	uint32_t DL;
	
	while (1){
	
		uint32_t cod_op = recibir_operacion(conexion);

		switch(cod_op){
		
			case INICIALIZAR_ESTRUCTURA: 
				inicializar_carpincho(conexion, loggerServidor);
				break;
			case MEMALLOC:
				DL = recibir_memalloc(conexion, loggerServidor);

				t_paquete* paquete = crear_paquete(MEMALLOC);
				paquete->buffer->size = sizeof(uint32_t);
				paquete->buffer->stream = malloc(paquete->buffer->size);
				int desplazamiento = 0;

				memcpy(paquete->buffer->stream + desplazamiento, &(DL) , sizeof(uint32_t));

				enviarPaquete(paquete, conexion);
					
				break;
			case MEMFREE:
				recibir_memfree(conexion, loggerServidor);
				break;
			case MEMREAD:
				recibir_memread(conexion, loggerServidor);
				break;
			case MEMWRITE:	
				recibir_memwrite(conexion, loggerServidor);
				break;
			case SUSPENSION_PROCESO:
				recibir_suspencion(conexion, loggerServidor);
				break;
			case CERRAR_INSTANCIA:
				cerrar_carpincho(conexion, loggerServidor);
				break;
				/*casos no validos de operacion kernel */
			case INICIAR_SEMAFORO:;
				responderOperacionNoValida(conexion, INICIAR_SEMAFORO, loggerServidor);
				break;
			case SEM_WAIT:;
				responderOperacionNoValida(conexion, SEM_WAIT, loggerServidor);
				break;
			case SEM_SIGNAL:;
				responderOperacionNoValida(conexion, SEM_SIGNAL, loggerServidor);
				break;
			case CERRAR_SEMAFORO:;
				responderOperacionNoValida(conexion, CERRAR_SEMAFORO, loggerServidor);
				break;
			case CONECTAR_IO:;
				responderOperacionNoValida(conexion, CONECTAR_IO, loggerServidor);
				break;
			case -1:
				log_error(loggerServidor, "el cliente se desconecto. Terminando servidor");
				close(conexion);
				break;
			default:
				log_warning(loggerServidor, "Entro al default");
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
		log_error(loggerServidor, "el cliente se desconecto. Terminando servidor");
		break;
	default:
		log_error(loggerServidor, "Se recibio un codigo de operacion incorrecto");
		break;
	}


}


uint32_t recibir_memalloc(int socket_cliente, t_log* logger) //devuelve DL del comienzo del bloque (no del heap)
{
	uint32_t offset = 0;
	uint32_t pid, tamanio;
	void* buffer = recibir_buffer(socket_cliente);
	
	memcpy(&pid, buffer, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&tamanio, buffer + offset,sizeof(uint32_t));
	
	free(buffer);
	
	log_debug(logger, "[MATE_MEMALLOC] El carpincho: %i pide allocar: %i bytes", pid, tamanio);
	
	uint32_t direccionLogica = administrar_allocs(pid, tamanio);

	return direccionLogica;

}

void inicializar_carpincho(int conexion ,t_log* logger){

	uint32_t size;
	//este size es del tamaño del buffer, pero como no nos interesa lo guardamos pero no lo usamos
	recv(conexion, &size, sizeof(uint32_t), MSG_WAITALL);
	
	t_carpincho* carpincho = malloc(sizeof(t_carpincho));

		pthread_mutex_lock(controladorIds);
		carpincho->id_carpincho = id_carpincho;
		id_carpincho++;
		pthread_mutex_unlock(controladorIds);

		carpincho->tabla_de_paginas = list_create();
		carpincho->conexion = conexion;
		log_debug(logger,"[MATE_INIT] Se inicializo el carpincho: %d",carpincho->id_carpincho);

		carpincho->contadorPag=0;
		carpincho->tlb_hit=0;
     	carpincho->tlb_miss=0;
     	carpincho->punteroClock=0;

		pthread_mutex_lock(listaCarpinchos);
		list_add(carpinchos, carpincho);
		list_add(carpinchosMetricas, carpincho);
		pthread_mutex_unlock(listaCarpinchos);


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

	free(buffer);

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

	list_destroy(marcosALiberar);

	bool buscarProcesoPorPid(t_carpincho* carpincho){
		return carpincho->id_carpincho == pidProcesoAEliminar;
	};
	pthread_mutex_lock(listaCarpinchos);
	t_carpincho* carpincho =list_remove_by_condition(carpinchos, buscarProcesoPorPid);
	pthread_mutex_unlock(listaCarpinchos);
	log_debug(logger,"[MATE_CLOSE] Sacamos al carpincho: %d", carpincho->id_carpincho);

	informarCierreDeProceso(carpincho,logger);

	finalizar_swap(pidProcesoAEliminar);


	void destructor(t_pagina* pagina){

		pthread_mutex_lock(TLB_mutex);
		bool buscarPag(t_pagina* p){
			return p->id_pagina == pagina->id_pagina && p->id_carpincho == pagina->id_carpincho;
		};
		
		list_remove_by_condition(TLB, (void*)buscarPag);
		pthread_mutex_unlock(TLB_mutex);

		free(pagina);
	};
	list_destroy_and_destroy_elements(carpincho->tabla_de_paginas, (void*)destructor);



}

void informarCierreDeProceso(t_carpincho* carpincho,t_log* loggerActual){

	t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t valorReturn = 1;


	memcpy(paquete->buffer->stream, &(valorReturn) , sizeof(uint32_t));

	log_info(loggerActual,"Enviamos que queremos cerrar el carpincho %i", carpincho->id_carpincho);
    enviarPaquete(paquete, carpincho->conexion);
	//cerrar hilo

}

uint32_t recibir_suspencion(conexion, logger){

	uint32_t pid;
	void* buffer = recibir_buffer(conexion);
	
	memcpy(&pid, buffer, sizeof(uint32_t));

	free(buffer);

	log_debug(logger, "[SUSPENSION] EL carpincho: %i pide suspender", pid);

	suspender_proceso(pid);

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

	int32_t confirmacion;
	
	if(pagina == NULL){
		 confirmacion =-5;
	}else{
		 confirmacion =1;
	}

	log_debug(logger, "[MATE_MEMFREE] EL carpincho: %i pide liberar la direcLogica: %i", carpincho, direccionLogica);

	liberar_alloc(carpincho, direccionLogica);

	t_paquete *paquete = crear_paquete(MEMFREE);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	offset=0;
	

    memcpy(paquete->buffer->stream + offset, &confirmacion, sizeof(int32_t));

	enviarPaquete(paquete, socket_cliente);

	log_info(loggerServidor, "El carpincho: %i ahora tiene %i paginas", carpincho, list_size(capybara->tabla_de_paginas));

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

	log_debug(logger, "[MATE_MEMREAD] El carpincho %i pide leer la direcLogica %i. Tamanio %i", carpincho, direccion_logica, tamanio);

	void* leido ;

		if(pagina == NULL){
		 leido = malloc(tamanio);
		 leido = "";
	}else{
		 leido = leer_memoria(direccion_logica, carpincho, tamanio);
	}


	t_paquete *paquete = crear_paquete(MEMREAD);

	paquete->buffer->size = tamanio + sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	offset=0;

	memcpy(paquete->buffer->stream, &tamanio, sizeof(uint32_t));
	offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, leido , tamanio);

	free(leido);

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

	int32_t confirmacion;
	if(pagina == NULL){
		 confirmacion = -7;
	}else{
		 confirmacion =1;
	}

	log_debug(logger, "[MATE_MEMWRITE] El carpincho %i pide escribir la direcLogica %i. Tamanio: %i", carpincho, direccion_logica, tamanio);

	escribir_memoria(carpincho, direccion_logica, contenido, tamanio);// Retorna un entero si se pudo escribir o no
	
	free(contenido);

	t_paquete *paquete = crear_paquete(MEMWRITE);

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	offset=0;
	

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
	
	log_error(logger,"Esta tarea no nos corresponde realizarla a nosotros ya que se debe realizar en el KERNEL");

	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t desplazamiento=0;

	memcpy(paquete->buffer->stream + desplazamiento , &(valor), sizeof(uint32_t));

	enviarPaquete(paquete, conexion);

	free(buffer);
}