#include "Servidor.h"


void atenderSolicitudesKernel(){
	//crea un hilo por cada cliente que se conecta y lo atiende. El servidor ya estaria levantado

	t_log* logger =  log_create("cfg/ServidorActual.log","Servidor",0,LOG_LEVEL_DEBUG);

	int servidor = iniciar_servidor(ipMemoria, puertoMemoria); // devuelve el socket del servidor
	
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

	t_log* logger =  log_create("cfg/ServidorActual.log","Servidor",0,LOG_LEVEL_DEBUG);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_error(logger,"Fallo en recibir la info de la conexion");
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
        break;

        case SEM_SIGNAL:;
        break;

        case CERRAR_SEMAFORO:;
			log_info(logger,"Vamos a cerrar un semaforo");
			cerrarSemaforo(paquete->buffer, conexion);
        break;

        case CONECTAR_IO:;
        break;

		default:;
		log_info(logger,"No se metio por ningun lado wtf");
		break;
	}
	
	
	int valorOperacion = paquete->codigo_operacion;

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
	
	pthread_mutex_lock(contadorProcesos);
		procesoNuevo->pid = cantidadDeProcesosActual;
		log_info(logger,"Un nuevo carpincho se une a la manada del kernel, y su pid es: %d",procesoNuevo->pid);
		cantidadDeProcesosActual++;
	pthread_mutex_unlock(contadorProcesos);

	procesoNuevo->conexion = conexion;
	procesoNuevo->rafagaEstimada = estimacion_inicial;
	procesoNuevo->tiempoDeEspera = 0;
	procesoNuevo->ultimaRafagaEjecutada = 0;

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

	/* notificar a memoria */
	log_info(logger,"Le notificamos a memoria para que libere la memoria del carpincho: %d", pidProcesoAEliminar);

	pthread_mutex_lock(modificarExec); //busco al proceso y lo saco
		proceso_kernel* procesoActual =list_remove_by_condition(procesosExec, buscarProcesoPorPid);
		log_info(logger,"Sacamos al carpincho de ejecucion. Pid: %d", procesoActual->pid);
	pthread_mutex_unlock(modificarExec);

	informarCierreDeProceso(procesoActual,logger);
	sem_post(nivelMultiprocesamiento); //aumento el grado de multiprogramacion y multiprocesamiento
	sem_post(nivelMultiProgramacionGeneral);

	

	log_info(logger,"Se nos va el carpincho: %d", procesoActual->pid);
	free(procesoActual); //libero la estructura, nose si el clock que tiene se libera o que onda...

}


void enviarInformacionAdministrativaDelProceso(proceso_kernel* proceso){

	t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    paquete->buffer->size = sizeof(uint32_t) *2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
	uint32_t valorBackEnd = KERNEL;
    int desplazamiento = 0;
	
    memcpy(paquete->buffer->stream + desplazamiento, &(proceso->pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(valorBackEnd) , sizeof(uint32_t));
    
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
}

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