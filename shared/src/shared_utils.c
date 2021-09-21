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

//atender solicitudes deberia estar en cada modulo

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

    if(socket_cliente==-1){
        perror("No se pudo crear la conexion");
        return -1;
    }

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
        perror("No se pudo conectar al servidor");
        freeaddrinfo(server_info);
        return -1;
    };
    
	freeaddrinfo(server_info);
	return socket_cliente;
}



void* serializar_paquete(t_paquete* paquete, int bytes)
{	
	void * contenido_serializado = malloc(bytes);
	
	int desplazamiento = 0;
	
	memcpy(contenido_serializado + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	
	memcpy(contenido_serializado + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	
	memcpy(contenido_serializado + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return contenido_serializado;
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}


t_paquete* crear_paquete(cod_operacion codigo)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}


void enviarPaquete(t_paquete* paquete, int conexion){

    int bytes = paquete->buffer->size + sizeof(cod_operacion) + sizeof(uint32_t);
    void* contenido_a_enviar= serializar_paquete(paquete, bytes);
    send(conexion, contenido_a_enviar,bytes,0);
    free(contenido_a_enviar);
}
