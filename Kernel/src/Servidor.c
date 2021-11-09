#include "Servidor.h"


void atenderSolicitudesKernel(){
	//crea un hilo por cada cliente que se conecta y lo atiende. El servidor ya estaria levantado

	t_log* logger =  log_create("cfg/ServidorActual.log","Servidor",1,LOG_LEVEL_DEBUG);

	int servidor = iniciar_servidor(ipMemoria, puertoServer); // devuelve el socket del servidor
	
	log_info(logger,"Inicializamos el servidor para que se nos unan los carpinchos");

	while(1){
		pthread_t cliente;
		int conexion = esperar_cliente(servidor);
		log_info(logger,"Se unio un carpincho");
		
		pthread_create(&cliente,NULL,(void*)atenderMensajeEnKernel,(void *) conexion);
		pthread_detach(cliente);
	}

	log_destroy(logger);

}





int atenderMensajeEnKernel(int conexion) {

	t_log* logger =  log_create("cfg/OperacionesServer.log","Operaciones", 1, LOG_LEVEL_DEBUG);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_info(logger,"Fallo en recibir la info de la conexion");
		return -1;
	}

	log_info(logger,"Recibimos la informacion de un carpincho");
	log_info(logger,"El codigo de operacion es: %d",paquete->codigo_operacion);

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);


	if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
    }
	
	int valorOperacion = paquete->codigo_operacion;

	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
			log_info(logger,"Vamos a inicializar un carpincho");
			inicializarProcesoNuevo(conexion, logger);
			break;

        case CERRAR_INSTANCIA:;
			log_info(logger,"Vamos a eliminar un carpincho");
			cerrarProceso(paquete->buffer, logger);
        break;

		case INICIAR_SEMAFORO:;
			log_info(logger,"Vamos a inicializar un semaforo");
			iniciarSemaforo(paquete->buffer, conexion);
		break;

        case SEM_WAIT:;
			log_info(logger,"Vamos a hacer un wait de un semaforo");
			valorOperacion = hacerWaitDeSemaforo(paquete->buffer, conexion);
        break;

        case SEM_SIGNAL:;
			log_info(logger,"Vamos a hacer un post de un semaforo");
			hacerPostDeSemaforo(paquete->buffer, conexion);
        break;

        case CERRAR_SEMAFORO:;
			log_info(logger,"Vamos a cerrar un semaforo");
			cerrarSemaforo(paquete->buffer, conexion);
        break;

        case CONECTAR_IO:;
			log_info(logger,"Vamos a realizar una peticion a un dispositivo IO");
			valorOperacion = conectarDispositivoIO(paquete->buffer, conexion);
        break;

		default:;
		log_info(logger,"No se metio por ningun lado wtf");
		break;
	}
	
	

    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
	log_destroy(logger);

	return valorOperacion;
	
}

void inicializarProcesoNuevo(int conexion ,t_log* logger){


	proceso_kernel* procesoNuevo =(proceso_kernel*) malloc(sizeof(proceso_kernel));
	
	establecerConexionConLaMemoria(procesoNuevo, logger);
	
	log_info(logger,"Un nuevo carpincho se une a la manada del kernel, y su pid es: %d",procesoNuevo->pid);
		

	procesoNuevo->conexion = conexion;
	procesoNuevo->rafagaEstimada = estimacion_inicial;
	procesoNuevo->tiempoDeEspera = 0;
	procesoNuevo->ultimaRafagaEjecutada = 0;
	procesoNuevo->vuelveDeBloqueo = NO_BLOQUEADO; //con esto vamos a decir que no realizo ningunn bloqueo todavia
	


	pthread_mutex_lock(modificarNew);
		list_add(procesosNew, procesoNuevo);
		log_info(logger,"Agregamos un carpincho a la lista de news, para que el planificador de largo plazo lo analize, y su pid es: %d",procesoNuevo->pid);
	pthread_mutex_unlock(modificarNew);


	enviarInformacionAdministrativaDelProceso(procesoNuevo);
	sem_post(hayProcesosNew);
	sem_post(procesoNecesitaEntrarEnReady);

	

}




void cerrarProceso(t_buffer* bufferActual,t_log* logger){

	void* stream = bufferActual->stream;
	uint32_t pidProcesoAEliminar;
	int desplazamiento = 0;

	memcpy(&(pidProcesoAEliminar), stream+desplazamiento, sizeof(uint32_t));

	bool buscarProcesoPorPid(proceso_kernel* proceso){
		if(proceso->pid == pidProcesoAEliminar){
			return 1;
		}
		return 0;
	}

	
	

	pthread_mutex_lock(modificarExec); //busco al proceso y lo saco
		proceso_kernel* procesoActual =list_remove_by_condition(procesosExec, buscarProcesoPorPid);
		log_info(logger,"Sacamos al carpincho de ejecucion. Pid: %d", procesoActual->pid);
	pthread_mutex_unlock(modificarExec);

	/* le notificamos a memoria que haga todo el trabajo de sacar las cosas de memoria y swap */
	finalizarEnMemoria(procesoActual, logger);
	log_info(logger,"Le notificamos a memoria para que libere la memoria del carpincho: %d", pidProcesoAEliminar);



	informarCierreDeProceso(procesoActual,logger);
	sem_post(nivelMultiprocesamiento); //aumento el grado de multiprogramacion y multiprocesamiento
	sem_post(nivelMultiProgramacionGeneral);

	/* esta funcion de liberar al proceso lo hace el planificador de largo plazo digamos... */
	liberarProceso(procesoActual);
}


void enviarInformacionAdministrativaDelProceso(proceso_kernel* proceso){

	t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;
	
    memcpy(paquete->buffer->stream + desplazamiento, &(proceso->pid) , sizeof(uint32_t));
    
	enviarPaquete(paquete, proceso->conexion);

}



void informarCierreDeProceso(proceso_kernel* proceso,t_log* loggerActual){

	t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t valorReturn = 0;
	int desplazamiento = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valorReturn) , sizeof(uint32_t));

	log_info(loggerActual,"Enviamos que queremos cerrar el carpincho");
    enviarPaquete(paquete,proceso->conexion);

}


void iniciarSemaforo(t_buffer * buffer, int conexion){
	
	void* stream = buffer->stream;
	int desplazamiento = 0;
	int tamanioNombre;
	int valor;

	memcpy(&(tamanioNombre), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	char* nombre = malloc(tamanioNombre);
	memcpy(nombre, stream+desplazamiento, tamanioNombre);
	desplazamiento += tamanioNombre;

	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));
	
	int valorReturn = crearSemaforo(nombre,valor);
	
	avisarInicializacionDeSemaforo(conexion,valorReturn);

	//free(nombre); ROMPE TODO CON ESTO

}


void cerrarSemaforo(t_buffer * buffer, int conexion){
	
	void* stream = buffer->stream;
	int desplazamiento = 0;
	int tamanioNombre;

	memcpy(&(tamanioNombre), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	char* nombre = malloc(tamanioNombre);
	memcpy(nombre, stream+desplazamiento, tamanioNombre);
	
	int valorReturn = destruirSemaforo(nombre);
	
	avisarDestruccionDeSemaforo(conexion,valorReturn);

	//free(nombre); ROMPE TODO CON ESTO, pero la duda era el porque?, sera xq se libera cuando se libera el buffer?

}

void hacerPostDeSemaforo(t_buffer * buffer, int conexion){
	
	void* stream = buffer->stream;
	int desplazamiento = 0;
	int tamanioNombre;

	memcpy(&(tamanioNombre), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	char* nombre = malloc(tamanioNombre);
	memcpy(nombre, stream+desplazamiento, tamanioNombre);
	
	int valorReturn = realizarSignalDeSemaforo(nombre);
	
	avisarPostDeSemaforo(conexion,valorReturn);

	free(nombre);
}

int hacerWaitDeSemaforo(t_buffer * buffer, int conexion){
	
	void* stream = buffer->stream;
	int desplazamiento = 0;
	int tamanioNombre;
	int pid;

	memcpy(&(pid), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(tamanioNombre), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	char* nombre = malloc(tamanioNombre);
	memcpy(nombre, stream+desplazamiento, tamanioNombre);
	
	int valorReturn = realizarWaitDeSemaforo(nombre, pid);
	if(valorReturn != 0){
		avisarWaitDeSemaforo(conexion,valorReturn);
	}

	free(nombre);

	if(valorReturn == 0){
		return SEM_WAIT;
	}else {
	return SEM_WAIT_NOBLOQUEANTE; }
}


int conectarDispositivoIO(t_buffer* buffer, int conexion){
    
	int desplazamiento = 0;
    int pid;
	int tamanioNombre;
	char* nombreDispositivo;
	

    memcpy(&(pid), buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(tamanioNombre), buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

	nombreDispositivo = malloc(tamanioNombre);
    memcpy(nombreDispositivo, buffer->stream + desplazamiento , tamanioNombre);

	int valorRetorno = realizarOperacionIO(pid, nombreDispositivo);
	//en caso de que no se realizo la operacion IO xq no se pudo, lo aviso ahora, en caso de que se realiza el bloqueo, lo realizo despues
	if(valorRetorno == 1){
		avisarconexionConDispositivoIO(conexion, valorRetorno);
	}

	free(nombreDispositivo);

	if(valorRetorno == 0){
		return CONECTAR_IO;
	}else {
	return FALLO_IO; }
	

}


/* AVISOS A MATELIB DE SEMAFOROS */


void avisarInicializacionDeSemaforo(int conexion, int valor){

	t_paquete* paquete = crear_paquete(INICIAR_SEMAFORO);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	int desplazamiento = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);

}

void avisarDestruccionDeSemaforo(int conexion, int valor){

	t_paquete* paquete = crear_paquete(CERRAR_SEMAFORO);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	int desplazamiento = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);

}


void avisarPostDeSemaforo(int conexion, int valor){

	t_paquete* paquete = crear_paquete(SEM_SIGNAL);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	int desplazamiento = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);

}

void avisarWaitDeSemaforo(int conexion, int valor){


	if(valor == 2){
		//esto lo vamos a usar como que funciono todo, pero a diferencia de que haga un wait y se bloquee, el 2 vamos a hacer que no se bloquee
		t_paquete* paquete = crear_paquete(SEM_WAIT);
		paquete->buffer->size = sizeof(uint32_t);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		int desplazamiento = 0;
		int valorNuevo = 0;
		memcpy(paquete->buffer->stream + desplazamiento, &(valorNuevo) , sizeof(uint32_t));

		enviarPaquete(paquete,conexion);
	}else{
		t_paquete* paquete = crear_paquete(SEM_WAIT);
		paquete->buffer->size = sizeof(uint32_t);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		int desplazamiento = 0;

		memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

		enviarPaquete(paquete,conexion);
	}

	

}


void avisarconexionConDispositivoIO(int conexion, int valor){

	t_paquete* paquete = crear_paquete(CONECTAR_IO);
	paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
	int desplazamiento = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);

}








/* CONEXIONES Por parte del KERNEL con MEMORIA */

void establecerConexionConLaMemoria(proceso_kernel* proceso,t_log* logger){
	
	int conexionNueva = crear_conexion(ipMemoria, puertoMemoria);
	//log_info(logger,"La conexion con la memoria es: %d", conexionNueva);
	proceso->conexionConMemoria = conexionNueva;
	inicializarEnMemoria(proceso, logger);

}


void inicializarEnMemoria(proceso_kernel* proceso, t_log* logger){

	int valor = 1;
	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion == 1){
		t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    	enviarPaquete(paquete,proceso->conexionConMemoria);
		
		proceso->pid = atenderMensajeDeMemoria(proceso->conexionConMemoria); 
	}else{
		proceso->pid = -1;
	}

}

void finalizarEnMemoria(proceso_kernel* proceso, t_log* logger){

	
	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion == 1){
		t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
		paquete->buffer->size = sizeof(uint32_t);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		int desplazamiento = 0;

		memcpy(paquete->buffer->stream + desplazamiento, &(proceso->pid) , sizeof(uint32_t));
		enviarPaquete(paquete,proceso->conexionConMemoria);
		
		int sePudoInicializar = atenderMensajeDeMemoria(proceso->conexionConMemoria); 
	}

}


int atenderMensajeDeMemoria(int conexion) {

	t_log* logger =  log_create("cfg/OperacionesServer.log","Memoria", 1, LOG_LEVEL_ERROR);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_error(logger,"Fallo en recibir la info de la conexion");
		return -1;
	}

	log_info(logger,"Recibimos la informacion de Memoria");
	log_info(logger,"El codigo de operacion es: %d",paquete->codigo_operacion);

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);


	if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
    }
	
	int valorOperacion;

	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
			valorOperacion = notificacionInicializacionDeMemoria(paquete->buffer, logger);
			break;
        case CERRAR_INSTANCIA:;
			valorOperacion = notificacionFinalizacionMemoria(paquete->buffer, logger);
        	break;
		case MEMALLOC:;
			break;
		case MEMFREE:;
			break;
		case MEMREAD:;
			break;
		case MEMWRITE:;
			break;
		case SUSPENSION_PROCESO:;
			break;
		default:;
		log_info(logger,"No se metio por ningun lado wtf");
		break;
	}
	
	

    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
	log_destroy(logger);

	return valorOperacion;
	
}

/* notificaciones de memoria */
int notificacionInicializacionDeMemoria(t_buffer* buffer,t_log* logger){

	void* stream = buffer->stream;
	int offset = 0;
	int pid;

	memcpy(&(pid), stream+offset, sizeof(uint32_t));

	return pid;

}


int notificacionFinalizacionMemoria(t_buffer* buffer,t_log* logger){

	void* data = buffer->stream;
	int desplazamiento = 0;
	int valor;


	memcpy(&(valor), data + desplazamiento, sizeof(uint32_t));

	if(valor == 0){
		log_info(logger, "Se pudo realizar toda la finalizacion en Memoria y SWAmP");
	}else
	{
		log_error(logger,"Algo fallo en la finalizacion en Memoria y SWAmP");
	}
	

	return valor;

}



/*esto es algo general para avisar cuando no se pudo realizar algo de memoria a la matelib */

int validacionConexionConMemoria(proceso_kernel* proceso, t_log* logger){
	if(proceso->conexionConMemoria == -1){
		log_error(logger,"La conexion con la Memoria fallo");
		return 0;
	}
	return 1;
}




void notificarQueNoSePudoRealizarTareaConMemoria(cod_operacion operacionSolicitada, proceso_kernel* proceso){

		t_paquete* paquete = crear_paquete(operacionSolicitada);
		paquete->buffer->size = sizeof(uint32_t);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		
		int desplazamiento = 0;
		int valor = 1;

		memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		enviarPaquete(paquete,proceso->conexion);
	
}