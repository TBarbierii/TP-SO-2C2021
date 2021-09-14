#include "shared_utils.h"

uint32_t iniciar_servidor(char* ip_servidor, char* puerto)
{
	uint32_t socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_servidor, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

uint32_t esperar_cliente(uint32_t socket_servidor)
{
	struct sockaddr_in dir_cliente;
	uint32_t tam_direccion = sizeof(struct sockaddr_in);

	uint32_t socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

void atender_solicitudes_multihilo(char* ip_servidor, char* puerto){
	//crea un hilo por cada cliente que se conecta y lo atiende. El servidor ya estaria levantado

	uint32_t servidor = iniciar_servidor(ip_servidor, puerto);
	pthread_t* cliente;
	while(1){
		uint32_t conexion_cliente = esperar_cliente(servidor);
	//	pthread_create(&cliente,NULL,(void*)atender_solicitud,conexion_cliente);
		pthread_detach(cliente);
	}

}