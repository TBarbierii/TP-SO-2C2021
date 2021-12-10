#include "matelib.h"

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

    int bytes = paquete->buffer->size + sizeof(cod_operacion) + sizeof(uint32_t);
    void* contenido_a_enviar= serializar_paquete(paquete, bytes);
    send(conexion, contenido_a_enviar,bytes,0);
    free(contenido_a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

}


//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config){

    int conexion = iniciarConexion(lib_ref,config);
    

    if(conexion == -1){
        lib_ref->group_info->conexionConBackEnd = conexion;
        lib_ref->group_info->pid= -1;
        

        lib_ref->group_info->loggerProceso = log_create("/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/Proceso-1.log","loggerContenidoProceso",1,LOG_LEVEL_DEBUG);
        log_info(lib_ref->group_info->loggerProceso,"Se ha creado el carpincho -1, pero no se ha logrado conectar correctamente al backend");
        
        return -1;

    }else{
        solicitarIniciarCarpincho(conexion, lib_ref);
        return recibir_mensaje(conexion, lib_ref);
    }
    
}



int mate_close(mate_instance *lib_ref){

        log_info(lib_ref->group_info->loggerProceso,"Se ha solicitado cerrar el carpincho de pid:%d , este es un hasta adios", lib_ref->group_info->pid);

        if(lib_ref->group_info->conexionConBackEnd != -1){    
            solicitarCerrarCarpincho(lib_ref->group_info->conexionConBackEnd, lib_ref);
            int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
            return valorRetorno;
        }else{
            log_error(lib_ref->group_info->loggerProceso,"No se pudo conectar con el servidor, cerramos desde aca al proceso de pid:%d",lib_ref->group_info->pid);
            
            log_info(lib_ref->group_info->loggerProceso,"Se liberan todas las estructuras del proceso de PID:%d",lib_ref->group_info->pid);
            log_destroy(lib_ref->group_info->loggerProceso);
            free(lib_ref->group_info);
        }
        
}






//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos inicializar un semaforo de nombre: %s, con valor : %d", sem, value);

    if(lib_ref->group_info->conexionConBackEnd != -1){
        inicializarSemaforo(lib_ref->group_info->conexionConBackEnd, sem, value);
        int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valorRetorno;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }
        
        
    

}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos destruir un semaforo de nombre: %s", sem);

    if(lib_ref->group_info->conexionConBackEnd != -1){
        liberarSemaforo(lib_ref->group_info->conexionConBackEnd, sem);
        int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valorRetorno;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }



    
}



int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos hacer un wait sobre el semaforo: %s", sem);

    if(lib_ref->group_info->conexionConBackEnd != -1){
        realizarWaitSemaforo(lib_ref->group_info->conexionConBackEnd, sem, lib_ref->group_info->pid);
        int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        if(valorRetorno == 3){ //nos cierran la conexion por deadlock
            
            log_debug(lib_ref->group_info->loggerProceso, "Se liberan todas las estructuras del proceso con PID: %d, ya que nos avisaron de cerrarlo por DEADLOCK",lib_ref->group_info->pid);
            
            close(lib_ref->group_info->conexionConBackEnd);
            lib_ref->group_info->conexionConBackEnd= -1;
            
        }


        return valorRetorno;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }
    
      
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos hacer un signal sobre el semaforo: %s", sem);
    
    if(lib_ref->group_info->conexionConBackEnd != -1){
        realizarPostSemaforo(lib_ref->group_info->conexionConBackEnd, sem, lib_ref->group_info->pid);
        int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valorRetorno;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }
    
      
}




//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void* msg){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar una operacion IO sobre el dispositivo: %s", io);

    if(lib_ref->group_info->conexionConBackEnd != -1){    
        realizarLlamadoDispositivoIO(lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, io);
        int valor = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valor;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }
    
}







//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar un memalloc de size:%d", size);

    if(lib_ref->group_info->conexionConBackEnd != -1){
        realizarMemAlloc(lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, size);
        mate_pointer valor = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valor;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }

    
    
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar un memfree de %d", addr);

    if(lib_ref->group_info->conexionConBackEnd != -1){
        realizarMemFree( lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, addr);
        mate_pointer valor = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valor;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }

    
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *info, int size){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar un memRead %d", origin);

    if(lib_ref->group_info->conexionConBackEnd != -1){
        realizarMemRead(lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, origin, size);
        void *informacion = (void*) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);

        //esto solo lo usamos para mostrar que contenido nos llega y loggearlo
        char* contenido = malloc(size+1);
        char valorFinal = '\0';
        memcpy(contenido,informacion,size);
        memcpy(contenido+size, &(valorFinal), 1);

        log_info(lib_ref->group_info->loggerProceso, "Contenido que llego del memread: %s \n",contenido);

        free(contenido);

        if(informacion != NULL){
            memcpy(info,informacion,size);
            free(informacion);
            return 0;
        }
        else{
            return MATE_READ_FAULT;
        }
        

    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }

}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar un memWrite de %d y tamanio:%d", dest, size);
    
    if(lib_ref->group_info->conexionConBackEnd != -1){
        realizarMemWrite( lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, origin, dest, size);
        mate_pointer valor = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valor;
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se puede ejecutar esta accion porque no esta conectado al servidor");
    }

}








//--------- Funciones extras---------//

int iniciarConexion(mate_instance *lib_ref, char *config){

    lib_ref->group_info = malloc(sizeof(mate_struct));

    t_config* datosBackEnd = config_create(config);
    char* ipBackEnd = config_get_string_value(datosBackEnd,"IP_BACKEND");
    char* puertoBackEnd = config_get_string_value(datosBackEnd,"PUERTO_BACKEND");
    int conexionConBackEnd = crear_conexion(ipBackEnd, puertoBackEnd);

    config_destroy(datosBackEnd);

    return conexionConBackEnd;
}


void* recibir_mensaje(int conexion, mate_instance* lib_ref) {

	t_paquete* paquete = malloc(sizeof(t_paquete));


	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		perror("Fallo en recibir la info de la conexion");
        lib_ref->group_info->conexionConBackEnd = -1;
        return -1;
        
	}
    int valorRetorno;
    void* retornoMensaje;

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);

    if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
    }

    
	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
            valorRetorno = agregarInfoAdministrativa(conexion,lib_ref, paquete->buffer);
			break;
        case CERRAR_INSTANCIA:;
            log_info(lib_ref->group_info->loggerProceso,"Estamos recibiendo todo para cerrar la conexion y terminar");
            valorRetorno = liberarEstructurasDeProceso(paquete->buffer,lib_ref);
            break;
		case INICIAR_SEMAFORO:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado crear un semaforo y obtenemos una respuesta en base a eso");
            valorRetorno = notificacionDeCreacionDeSemaforo(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case SEM_SIGNAL:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer un signal a un semaforo y obtenemos una respuesta en base a eso");
            valorRetorno = notificacionDePostSemaforo(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case CERRAR_SEMAFORO:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado cerrar un semaforo y obtenemos una respuesta en base a eso");
            valorRetorno = notificacionDeDestruccionDeSemaforo(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case SEM_WAIT:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer un wait de un semaforo y obtenemos una respuesta en base a eso");
            valorRetorno = notificacionDeWaitSemaforo(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case CONECTAR_IO:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer una operacion IO y obtenemos una respuesta en base a eso");
            valorRetorno = notificacionIO(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case MEMALLOC:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer un memalloc");
            valorRetorno = notificacionMemAlloc(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case MEMFREE:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer un memfree");
            valorRetorno = notificacionMemFree(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case MEMREAD:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer un memread");
            retornoMensaje = notificacionMemRead(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        case MEMWRITE:;
            log_info(lib_ref->group_info->loggerProceso,"Habiamos solicitado hacer un memwrite");
            valorRetorno = notificacionMemWrite(paquete->buffer, lib_ref->group_info->loggerProceso);
            break;
        default:;
            break;
	}


    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	

    if(paquete->codigo_operacion == MEMREAD){
        free(paquete);
        return retornoMensaje;
    }
    free(paquete);
    return valorRetorno;
}


/* Respuestas del Backend */

int agregarInfoAdministrativa(int conexion, mate_instance* lib_ref, t_buffer* buffer){
	void* stream = buffer->stream;
	int offset = 0;

	memcpy(&(lib_ref->group_info->pid), stream+offset, sizeof(uint32_t));
	
    if(lib_ref->group_info->pid < 0 ){
        perror("No se pudo crear la instancia :(");
            
    }

    lib_ref->group_info->conexionConBackEnd = conexion;
        
        
    char* nombreLog = string_new();
    string_append(&nombreLog, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/Proceso");
    char* pidCarpincho = string_itoa((int) lib_ref->group_info->pid);
    string_append(&nombreLog, pidCarpincho);
    string_append(&nombreLog, ".log");

    lib_ref->group_info->loggerProceso = log_create(nombreLog,"loggerContenidoProceso",1,LOG_LEVEL_DEBUG);
    
    log_debug(lib_ref->group_info->loggerProceso,"Se ha creado el carpincho:%d, y se ha logrado conectar correctamente al backend", lib_ref->group_info->pid);

    free(pidCarpincho);
    free(nombreLog);

    return 0;
    

    

}



int liberarEstructurasDeProceso(t_buffer* buffer, mate_instance* lib_ref){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 1){
        log_debug(lib_ref->group_info->loggerProceso,"Se pudo cerrar el proceso de PID:%d correctamente en el BackEnd", lib_ref->group_info->pid);
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se pudo cerrar el proceso de PID:%d, correctamente en el BackEnd",lib_ref->group_info->pid);
    }



    log_debug(lib_ref->group_info->loggerProceso,"Se liberan todas las estructuras del proceso de PID:%d",lib_ref->group_info->pid);
    log_destroy(lib_ref->group_info->loggerProceso);
    close(lib_ref->group_info->conexionConBackEnd);
    free(lib_ref->group_info);

    return valor;

}


int notificacionDeCreacionDeSemaforo(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 1){
        log_info(logger,"Se pudo inicializar el semaforo que se solicito inicializar");
    }else{
        log_error(logger,"No se pudo inicializar el semaforo que se solicito inicializar");
    }

    return valor;
}


int notificacionDeDestruccionDeSemaforo(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 1){
        log_info(logger,"Se pudo destruir el semaforo que se solicito destruir");
    }else{
        log_error(logger,"No se pudo destruir el semaforo que se solicito destruir");
    }

    return valor;
}


int notificacionDePostSemaforo(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 1){
        log_info(logger,"Se pudo hacer el post del semaforo");
    }else{
        log_error(logger,"No see pudo realizar el post del semaforo solicitado");
    }

    return valor;
}

int notificacionDeWaitSemaforo(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 1){
        log_info(logger,"Se pudo hacer el wait del semaforo");
    }else{
        log_error(logger,"No see pudo realizar el wait al semaforo solicitado");
    }

    return valor;
}

int notificacionIO(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;

	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    if(valor == 1){
        log_info(logger,"Se pudo realizar con exito la operacion IO");
    }else{
        log_error(logger,"No see pudo realizar la operacion IO");
    }


   return valor;
}





int notificacionMemAlloc(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(int32_t));

    if(valor == 0){
        log_error(logger,"No se pudo hacer el memalloc del size solicitado");
    }else{
        log_info(logger,"Se pudo realizar el memalloc");
    }

    return valor;
}


int notificacionMemFree(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 0){
        log_error(logger,"No se pudo hacer el memfree del size solicitado");
        valor = MATE_FREE_FAULT;
    }else{
        log_info(logger,"Se pudo realizar el memfree");
    }

    return valor;
}


void* notificacionMemRead(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int size;

	memcpy(&(size), stream+desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    log_info(logger,"El tamanio de lo que vamos a leer es de:%d bytes",size);

    //si no hay contenido en esa direccion, devolvemos nulo
    if(size != 0){
        void* contenido = malloc(size);
        memcpy(contenido, stream+desplazamiento, size);
        return contenido;
    }

    return NULL;
}


int notificacionMemWrite(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 1){
        log_info(logger,"Se pudo hacer el memwrite solicitado");
    }else{
        log_error(logger,"No se pudo realizar el memwrite");
        valor = MATE_WRITE_FAULT;
    }

    return valor;
}



/* ------- Solicitudes  --------------------- */


/* estructuracion */
void solicitarIniciarCarpincho(int conexion, mate_instance* lib_ref){
    
    t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    enviarPaquete(paquete,conexion);
}


void solicitarCerrarCarpincho(int conexion, mate_instance* lib_ref){
    
    t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &(lib_ref->group_info->pid), paquete->buffer->size);

    enviarPaquete(paquete,conexion);
}

 /* semaforos */

void inicializarSemaforo(int conexion, mate_sem_name nombreSemaforo, unsigned int valor){
    
    t_paquete* paquete = crear_paquete(INICIAR_SEMAFORO);

    paquete->buffer->size = sizeof(uint32_t)*2 + strlen(nombreSemaforo)+1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = strlen(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , tamanioNombre);
    desplazamiento += tamanioNombre;

    memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);
}


void liberarSemaforo(int conexion, mate_sem_name nombreSemaforo){
    t_paquete* paquete = crear_paquete(CERRAR_SEMAFORO);

    paquete->buffer->size = sizeof(uint32_t) + strlen(nombreSemaforo)+1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = strlen(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , tamanioNombre);

    enviarPaquete(paquete,conexion);
}


void realizarWaitSemaforo(int conexion, mate_sem_name nombreSemaforo, int pid){
    
    t_paquete* paquete = crear_paquete(SEM_WAIT);

    paquete->buffer->size = sizeof(uint32_t)*2 + string_length(nombreSemaforo) +1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = string_length(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);


    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , tamanioNombre);

    enviarPaquete(paquete,conexion);
}

void realizarPostSemaforo(int conexion, mate_sem_name nombreSemaforo, int pid){
    t_paquete* paquete = crear_paquete(SEM_SIGNAL);

    paquete->buffer->size = sizeof(uint32_t)*2 + string_length(nombreSemaforo) +1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = string_length(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , string_length(nombreSemaforo)+1);

    enviarPaquete(paquete,conexion);
}


/* IO */


void realizarLlamadoDispositivoIO(int conexion, int pid, char* io){

    t_paquete* paquete = crear_paquete(CONECTAR_IO);
    uint32_t tamanioNombre = string_length(io)+1;

    paquete->buffer->size = sizeof(uint32_t) + sizeof(uint32_t)  + tamanioNombre;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, io , tamanioNombre);

    enviarPaquete(paquete,conexion);
    
}


/* MEMORIA */

void realizarMemAlloc(int conexion, uint32_t pid, int size){

    t_paquete* paquete = crear_paquete(MEMALLOC);

    paquete->buffer->size = sizeof(uint32_t) * 2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(size) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);

}
void realizarMemFree(int conexion, uint32_t pid, mate_pointer addr){

    t_paquete* paquete = crear_paquete(MEMFREE);

    paquete->buffer->size = sizeof(uint32_t) + sizeof(int32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(addr) , sizeof(int32_t));

    enviarPaquete(paquete,conexion);
}

void realizarMemRead(int conexion, uint32_t pid, mate_pointer origin, int size){
    
    t_paquete* paquete = crear_paquete(MEMREAD);

    paquete->buffer->size = sizeof(uint32_t) *2 + sizeof(int32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(origin) , sizeof(int32_t));
    desplazamiento += sizeof(int32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(size) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);

}

void realizarMemWrite(int conexion, uint32_t pid, void *origin, mate_pointer dest, int size){

    t_paquete* paquete = crear_paquete(MEMWRITE);

    paquete->buffer->size = sizeof(uint32_t) *2 + sizeof(int32_t) + size;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(dest) , sizeof(int32_t));
    desplazamiento += sizeof(int32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(size) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, origin , size);

    enviarPaquete(paquete,conexion);    


}
