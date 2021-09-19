#include "Memoria.h"

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

			case /*memalloc*/:

			case /*memfree*/:
            case /*memwrite*/:
            case /*memread*/:	
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				break;
			default:
				log_warning(logger, "Entro al default");
				break;
			}
}
}
