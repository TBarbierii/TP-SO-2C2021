#include "shared_utils.h"

uint32_t iniciar_servidor(char* ip_servidor, char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


    getaddrinfo(ip_servidor, puerto, &hints, &servinfo);

    socket_servidor = socket(servinfo->ai_family, 
                         servinfo->ai_socktype, 
                         servinfo->ai_protocol);

	int activado = 1;
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR,&activado, sizeof(activado));					 

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}





int esperar_cliente(int socket_servidor)
{
	
	int socket_cliente = accept(socket_servidor, NULL, NULL);

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

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

    if(socket_cliente==-1){
        perror("No se pudo crear la conexion");
        return -1;
    }

	int conexion = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);
	if(conexion == -1){
		return conexion;
	}
	return socket_cliente;
}



void* serializar_paquete(t_paquete* paquete, int bytes)
{	
	void * contenido_serializado = malloc(bytes);
	
	int desplazamiento = 0;
	
	memcpy(contenido_serializado + desplazamiento, &(paquete->codigo_operacion), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	
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

	int bytes = 0;
    bytes = paquete->buffer->size + sizeof(cod_operacion) + sizeof(uint32_t);
    void* contenido_a_enviar= serializar_paquete(paquete, bytes);
    send(conexion, contenido_a_enviar,bytes,0);
    free(contenido_a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

}
