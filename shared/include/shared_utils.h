#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <commons/log.h>
#include <stdbool.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/log.h>

t_log* logger;

uint32_t iniciar_servidor(char* ip_servidor, char* puerto);
uint32_t esperar_cliente(uint32_t socket_servidor);

#endif