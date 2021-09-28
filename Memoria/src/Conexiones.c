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

void* recibir_buffer(int* size, int socket_cliente)
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
		pthread_create(&cliente,NULL,(void*)atender_solicitudes,conexion_cliente);
		pthread_detach(cliente);
	}

}

void atender_solicitudes(uint32_t conexion){
uint32_t cod_op = recibir_operacion(conexion);
	
	switch(cod_op)
	{
	

	case INICIALIZAR_ESTRUCTURA: //caso de que no haya kernel
		//recibir_mateinit();
		//devolver pid, y un 1;
	case MEMALLOC:
		//recibir_memalloc();
		
	case MEMFREE:
		//recibir_memfree();
	case MEMREAD:
		//recibir_memread();
	case MEMWRITE:	
		//recibir_memwrite();
	case CERRAR_INSTANCIA:
		//RECIBIR_cerrar();
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
	t_memalloc alloc;
	void* buffer = recibir_buffer(&size, socket_cliente);
	
	memcpy(alloc.pid, buffer, sizeof(uint32_t));
	offset =+ sizeof(uint32_t);
	memcpy(alloc.tamanio, buffer + offset,sizeof(uint32_t));

	free(buffer);

	administrar_allocs(alloc);

	//escribir_en_memoria(dl); //aca se graba en memoria los allocs reservados. Devuelve el comienzo del marco

	return 0;
}