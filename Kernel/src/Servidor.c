#include "Servidor.h"


void atenderSolicitudesKernel(char* ip_servidor, char* puerto){
	//crea un hilo por cada cliente que se conecta y lo atiende. El servidor ya estaria levantado

	 int servidor = iniciar_servidor(ip_servidor, puerto); // devuelve el socket del servidor
	 pthread_t* cliente;
	while(1){
		int conexion = esperar_cliente(servidor);
		pthread_create(&cliente,NULL,(void*)atenderMensajeEnKernel,conexion);
		pthread_detach(cliente);
	}

}

void atenderMensajeEnKernel(int conexion) {

	t_paquete* paquete = malloc(sizeof(paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		perror("Fallo en recibir la info de la conexion");
	}

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);

	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
			break;
        case CERRAR_INSTANCIA:;
            break;
		case INICIAR_SEMAFORO:;
            break;
        case SEM_WAIT:;
            break;
        case SEM_SIGNAL:;
            break;
        case CERRAR_SEMAFORO:;
            break;
        case CONECTAR_IO:;
            break;
	}


    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
}