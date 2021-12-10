#include "Servidor.h"


void atenderSolicitudesKernel(){
	//crea un hilo por cada cliente que se conecta y lo atiende. El servidor ya estaria levantado

	t_log* logger =  log_create("cfg/ServidorActual.log","Servidor",1,LOG_LEVEL_DEBUG);

	int servidor = iniciar_servidor(ipKernel, puertoServer); // devuelve el socket del servidor
	
	log_debug(logger,"[SERVER] Inicializamos el servidor para que se nos unan los carpinchos");

	while(1){
		pthread_t cliente;
		int conexion = esperar_cliente(servidor);
		log_warning(logger,"[SERVER] Se unio un carpincho");
		
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
		log_error(logger,"[SERVER] Fallo en recibir la info de la conexion");
		return -1;
	}

	//log_debug(logger,"[SERVER] Recibimos la informacion de un carpincho");
//	log_info(logger,"El codigo de operacion es: %d",paquete->codigo_operacion);

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);


	if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
    }
	
	int valorOperacion = paquete->codigo_operacion;

	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
			log_info(logger,"[SERVER] Vamos a inicializar un carpincho");
			inicializarProcesoNuevo(conexion, logger);
			break;

        case CERRAR_INSTANCIA:;
			log_info(logger,"[SERVER] Vamos a eliminar un carpincho");
			cerrarProceso(paquete->buffer, logger);
        break;

		case INICIAR_SEMAFORO:;
			log_info(logger,"[SERVER] Vamos a inicializar un semaforo");
			iniciarSemaforo(paquete->buffer, conexion);
		break;

        case SEM_WAIT:;
			log_info(logger,"[SERVER] Vamos a hacer un wait de un semaforo");
			valorOperacion = hacerWaitDeSemaforo(paquete->buffer, conexion);
        break;

        case SEM_SIGNAL:;
			log_info(logger,"[SERVER] Vamos a hacer un post de un semaforo");
			hacerPostDeSemaforo(paquete->buffer, conexion);
        break;

        case CERRAR_SEMAFORO:;
			log_info(logger,"[SERVER] Vamos a cerrar un semaforo");
			cerrarSemaforo(paquete->buffer, conexion);
        break;

        case CONECTAR_IO:;
			log_info(logger,"[SERVER] Vamos a realizar una peticion a un dispositivo IO");
			valorOperacion = conectarDispositivoIO(paquete->buffer, conexion);
        break;

		case MEMALLOC:;
			log_info(logger,"[SERVER] Vamos a realizar un Mem Alloc");
			memAlloc(paquete->buffer, logger);
		break;

		case MEMFREE:;
			log_info(logger,"[SERVER] Vamos a realizar un Mem Free");
			memFree(paquete->buffer, logger);
		break;

		case MEMREAD:;
			log_info(logger,"[SERVER] Vamos a realizar un Mem Read");
			memRead(paquete->buffer, logger);
		break;

		case MEMWRITE:;
			log_info(logger,"[SERVER] Vamos a realizar un Mem Write");
			memWrite(paquete->buffer, logger);
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

	procesoNuevo->conexion = conexion;
	procesoNuevo->rafagaEstimada = estimacion_inicial/1000; //lo pasamos a segundos
	procesoNuevo->tiempoDeEspera = 0;
	procesoNuevo->ultimaRafagaEjecutada = 0;
	procesoNuevo->vuelveDeBloqueo = NO_BLOQUEADO; //con esto vamos a decir que no realizo ningunn bloqueo todavia
	procesoNuevo->listaRecursosRetenidos = list_create();
	procesoNuevo->listaRecursosSolicitados = list_create();
	procesoNuevo->responseRatio = 0;

	pthread_mutex_lock(modificarNew);
		list_add(procesosNew, procesoNuevo);
		log_info(logger,"[SERVER] Agregamos un carpincho a la lista de news, para que el planificador de largo plazo lo analize");
	pthread_mutex_unlock(modificarNew);


	
	sem_post(hayProcesosNew);
	sem_post(procesoNecesitaEntrarEnReady);
	//esto es un aviso para el planificador de mediano plazo de que hay un proceso nuevo y quiza deberia entrar si solo hay procesos de Bloqueados
    sem_post(signalSuspensionProceso);

	

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
		//log_info(logger,"Sacamos al carpincho de ejecucion. Pid: %d", procesoActual->pid);
	pthread_mutex_unlock(modificarExec);

	/* le notificamos a memoria que haga todo el trabajo de sacar las cosas de memoria y swap */
	log_info(logger,"[SERVER] Le notificamos a memoria para que libere la memoria del carpincho: %d", pidProcesoAEliminar);
	finalizarEnMemoria(procesoActual, logger);



	informarCierreDeProceso(procesoActual,logger);
	
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
	uint32_t valorReturn = 1;
	int desplazamiento = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valorReturn) , sizeof(uint32_t));

	
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

	free(nombre);
}

void hacerPostDeSemaforo(t_buffer * buffer, int conexion){
	
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
	
	int valorReturn = realizarSignalDeSemaforo(nombre, pid);
	
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
	if(valorReturn != 1){
		avisarWaitDeSemaforo(conexion,valorReturn);
	}

	free(nombre);

	if(valorReturn == 1){
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
	if(valorRetorno == 0){
		avisarconexionConDispositivoIO(conexion, valorRetorno);
	}

	free(nombreDispositivo);

	if(valorRetorno == 1){
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
		//TODO: aca podriamos utilizad recursividad?
		t_paquete* paquete = crear_paquete(SEM_WAIT);
		paquete->buffer->size = sizeof(uint32_t);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		int desplazamiento = 0;
		int valorNuevo = 1;
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
	proceso->conexionConMemoria = conexionNueva;
	inicializarEnMemoria(proceso, logger);

}


void inicializarEnMemoria(proceso_kernel* proceso, t_log* logger){

	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion == 1){
		t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    	enviarPaquete(paquete,proceso->conexionConMemoria);
		
		proceso->pid = atenderMensajeDeMemoria(proceso); 
	}else{
		proceso->pid = -1;
	}

	log_error(logger, "[SERVER] El nuevo carpincho que entra al sistema permitido por el grado de multiprogramacion es:%i", proceso->pid);

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
		
		atenderMensajeDeMemoria(proceso); 
	}

}


int atenderMensajeDeMemoria(proceso_kernel* proceso) {

	t_log* logger =  log_create("cfg/OperacionesServer.log","Memoria", 1, LOG_LEVEL_DEBUG);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	if(recv(proceso->conexionConMemoria, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_error(logger,"[MEMORIA] Fallo en recibir la info de la conexion");
		return -1;
	}

	log_warning(logger,"Recibimos la informacion de Memoria");
	//log_info(logger,"El codigo de operacion es: %d",paquete->codigo_operacion);

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(proceso->conexionConMemoria, &(paquete->buffer->size), sizeof(uint32_t), 0);


	if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(proceso->conexionConMemoria, paquete->buffer->stream, paquete->buffer->size, 0);
    }
	
	int valorOperacion = 0;

	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
			valorOperacion = notificacionInicializacionDeMemoria(paquete->buffer, logger);
			break;
        case CERRAR_INSTANCIA:;
			valorOperacion = notificacionFinalizacionMemoria(paquete->buffer, logger);
        	break;
		case MEMALLOC:;
			notificacionMemAlloc(proceso, paquete->buffer, logger);
			break;
		case MEMFREE:;
			notificacionMemFree(proceso, paquete->buffer, logger);
			break;
		case MEMREAD:;
			notificacionMemRead(proceso, paquete->buffer, logger);
			break;
		case MEMWRITE:;
			notificacionMemWrite(proceso, paquete->buffer, logger);
			break;
		case SUSPENSION_PROCESO:;
			notificacionSuspensionProceso(proceso, paquete->buffer, logger);
			break;
		default:;
		log_info(logger,"[MEMORIA] No se metio por ningun lado wtf");
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



/* CONEXIONES INTERMEDIAS */

/* recibos desde matelib*/


void memAlloc(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int pid;
	int size;

	memcpy(&(pid), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(size), stream+desplazamiento, sizeof(uint32_t));

	bool buscarProceso(proceso_kernel* proceso){
		return proceso->pid == pid;
	}


	pthread_mutex_lock(modificarExec);
	proceso_kernel* proceso = list_find(procesosExec,buscarProceso);
    pthread_mutex_unlock(modificarExec);

	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion){
		realizarMemAlloc(proceso->conexionConMemoria,pid,size);
		atenderMensajeDeMemoria(proceso);
	}else{
		notificarQueNoSePudoRealizarTareaConMemoria(MEMALLOC,proceso);
	}

}


void memFree(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int pid;
	int direccion;

	memcpy(&(pid), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(direccion), stream+desplazamiento, sizeof(uint32_t));

	bool buscarProceso(proceso_kernel* proceso){
		return proceso->pid == pid;
	}


	pthread_mutex_lock(modificarExec);
	proceso_kernel* proceso = list_find(procesosExec,buscarProceso);
    pthread_mutex_unlock(modificarExec);

	

	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion){
		realizarMemFree(proceso->conexionConMemoria,pid,direccion);
		atenderMensajeDeMemoria(proceso);
	}else{
		notificarQueNoSePudoRealizarTareaConMemoria(MEMFREE,proceso);
	}
}


void memRead(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int pid;
	int direccion;
	int size;
	

	memcpy(&(pid), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(direccion), stream+desplazamiento, sizeof(int32_t));
	desplazamiento += sizeof(int32_t);
	memcpy(&(size), stream+desplazamiento, sizeof(uint32_t));



	bool buscarProceso(proceso_kernel* proceso){
		return proceso->pid == pid;
	}


	pthread_mutex_lock(modificarExec);
	proceso_kernel* proceso = list_find(procesosExec,buscarProceso);
    pthread_mutex_unlock(modificarExec);

	

	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion){
		realizarMemRead(proceso->conexionConMemoria,pid,direccion,size);
		atenderMensajeDeMemoria(proceso);
	}else{
		notificarQueNoSePudoRealizarTareaConMemoria(MEMREAD,proceso);
	}

}


int memWrite(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int pid;
	int direccion;
	int size;
	void* contenidoAenviar;

	memcpy(&(pid), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(direccion), stream+desplazamiento, sizeof(int32_t));
	desplazamiento += sizeof(int32_t);
	memcpy(&(size), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	contenidoAenviar = malloc(size);
	memcpy(contenidoAenviar, stream+desplazamiento, size);


	bool buscarProceso(proceso_kernel* proceso){
		return proceso->pid == pid;
	}


	pthread_mutex_lock(modificarExec);
	proceso_kernel* proceso = list_find(procesosExec,buscarProceso);
    pthread_mutex_unlock(modificarExec);
	
	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion){
		realizarMemWrite(proceso->conexionConMemoria,pid,contenidoAenviar,direccion,size);
		atenderMensajeDeMemoria(proceso);
	}else{
		notificarQueNoSePudoRealizarTareaConMemoria(MEMWRITE,proceso);
	}

	free(contenidoAenviar);

}


/* solicitudes a memoria */

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


void realizarMemFree(int conexion, uint32_t pid, int32_t addr){

    t_paquete* paquete = crear_paquete(MEMFREE);

    paquete->buffer->size = sizeof(uint32_t) + sizeof(int32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(addr) , sizeof(int32_t));

    enviarPaquete(paquete,conexion);
}

void realizarMemRead(int conexion, uint32_t pid, int32_t origin, int size){
    
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

void realizarMemWrite(int conexion, uint32_t pid, void *origin, int32_t dest, int size){

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


/* recibos de memoria y envio a matelib*/

void notificacionMemAlloc(proceso_kernel* proceso, t_buffer* buffer, t_log* logger){

	void* stream = buffer->stream;
	int desplazamiento = 0;
	int valorDireccion;
	memcpy(&(valorDireccion), stream+desplazamiento, sizeof(int32_t));

	t_paquete* paquete = crear_paquete(MEMALLOC);

    paquete->buffer->size = sizeof(int32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento2 = 0;

    memcpy(paquete->buffer->stream + desplazamiento2, &(valorDireccion) , sizeof(int32_t));
    enviarPaquete(paquete, proceso->conexion);
	log_info(logger,"[MEMORIA] Se pudo establecer la conexion con memoria y se realizo un Mem Alloc");

}


void notificacionMemFree(proceso_kernel* proceso, t_buffer* buffer, t_log* logger){

	void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

	t_paquete* paquete = crear_paquete(MEMFREE);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento2 = 0;

    memcpy(paquete->buffer->stream + desplazamiento2, &(valor) , sizeof(uint32_t));
    enviarPaquete(paquete, proceso->conexion);
	log_info(logger,"[MEMORIA] Se pudo establecer la conexion con memoria y se realizo un Mem Free");

}


void notificacionMemRead(proceso_kernel* proceso, t_buffer* buffer, t_log* logger){

	void* stream = buffer->stream;
	int desplazamiento = 0;
	int size;
	


	memcpy(&(size), stream+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

    void* contenido = malloc(size);
    memcpy(contenido, stream+desplazamiento, size);




	t_paquete* paquete = crear_paquete(MEMREAD);
	paquete->buffer->size = sizeof(uint32_t) + size;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	int desplazamiento2 = 0;

    memcpy(paquete->buffer->stream + desplazamiento2, &(size) , sizeof(uint32_t));
	desplazamiento2 += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + desplazamiento2, contenido , size);

    
    enviarPaquete(paquete, proceso->conexion);

	if(size != 0){
		free(contenido);
	}
	log_info(logger,"[MEMORIA] Se pudo establecer la conexion con memoria y se realizo un Mem Read");

}

void notificacionMemWrite(proceso_kernel* proceso, t_buffer* buffer, t_log* logger){

	void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

	t_paquete* paquete = crear_paquete(MEMWRITE);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento2 = 0;

    memcpy(paquete->buffer->stream + desplazamiento2, &(valor) , sizeof(uint32_t));
    enviarPaquete(paquete, proceso->conexion);

	log_info(logger,"[MEMORIA] Se pudo establecer la conexion con memoria y se realizo un Mem Write");


}

void notificacionSuspensionProceso(proceso_kernel* proceso, t_buffer* buffer, t_log* logger) {
	void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));
	
	if(valor){
		log_info(logger,"[MEMORIA] Se pudo establecer la conexion con memoria y se suspendio el proceso");
	}else{
		log_error(logger,"[MEMORIA] Se pudo establecer la conexion con memoria y no se pudo suspender el proceso");
	}
	

	
}


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

	if(valor == 1){
		log_info(logger, "[MEMORIA] Se pudo realizar toda la finalizacion en Memoria y SWAmP");
	}else
	{
		log_error(logger,"[MEMORIA] Algo fallo en la finalizacion en Memoria y SWAmP");
	}
	

	return valor;

}


void notificarSuspensionDeProceso(proceso_kernel* proceso, t_log* logger){
	
	int validacion = validacionConexionConMemoria(proceso,logger);

	if(validacion){
		t_paquete* paquete = crear_paquete(SUSPENSION_PROCESO);
		paquete->buffer->size = sizeof(uint32_t);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		int desplazamiento = 0;

		memcpy(paquete->buffer->stream + desplazamiento, &(proceso->pid) , sizeof(uint32_t));
		enviarPaquete(paquete,proceso->conexionConMemoria);
	}
}

/*esto es algo general para avisar cuando no se pudo realizar algo de memoria a la matelib */

int validacionConexionConMemoria(proceso_kernel* proceso, t_log* logger){
	if(proceso->conexionConMemoria == -1){
		log_error(logger,"[MEMORIA] La conexion con la Memoria fallo");
		return 0;
	}
	return 1;
}


void notificarQueNoSePudoRealizarTareaConMemoria(cod_operacion operacionSolicitada, proceso_kernel* proceso){

	
	t_paquete* paquete = crear_paquete(operacionSolicitada);
	paquete->buffer->size = sizeof(uint32_t);
	paquete->buffer->stream = malloc(paquete->buffer->size);
		
	int desplazamiento = 0;
	int valor = 0;

	memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	enviarPaquete(paquete,proceso->conexion);
	
}


