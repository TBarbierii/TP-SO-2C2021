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

	t_log* logger =  log_create("cfg/Servidor.log","Servidor",0,LOG_LEVEL_ERROR);

	t_paquete* paquete = malloc(sizeof(paquete));

	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		log_error(logger,"Fallo en recibir la info de la conexion");
	}
	
	log_info(logger,"Recibimos la informacion de un carpincho");

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);

	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
			inicializarProcesoNuevo(conexion, logger);
			break;
        case CERRAR_INSTANCIA:;
			cerrarProceso(paquete->buffer, logger);
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

	int valorOperacion = paquete->codigo_operacion;

    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
	log_destroy(logger);

	return valorOperacion;
	
}

void enviarInformacionAdministrativaDelProceso(proceso_kernel* proceso){

	t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    paquete->buffer->size = sizeof(uint32_t) *2;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    memcpy(paquete->buffer->stream + desplazamiento, &(proceso->pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

	uint32_t valorBackEnd = KERNEL;

    memcpy(paquete->buffer->stream + desplazamiento, &(valorBackEnd) , sizeof(uint32_t));

    enviarPaquete(paquete, proceso->conexion);

}

void inicializarProcesoNuevo(int conexion,t_log* logger){


	proceso_kernel* procesoNuevo = malloc(sizeof(proceso_kernel));
	
	pthread_mutex_lock(contadorProcesos);
		procesoNuevo->pid = cantidadDeProcesosActual;
		log_info(logger,"Un nuevo carpincho se une a la manada del kernel, y su pid es:",string_itoa(procesoNuevo->pid));
		cantidadDeProcesosActual++;
	pthread_mutex_unlock(contadorProcesos);

	procesoNuevo->conexion = conexion;
	procesoNuevo->rafagaEstimada = estimacion_inicial;
	procesoNuevo->tiempoDeEspera = 0;
	procesoNuevo->ultimaRafagaEjecutada = 0;

	pthread_mutex_lock(modificarNew);
		list_add(procesosNew, procesoNuevo);
	pthread_mutex_unlock(modificarNew);
	sem_post(hayProcesosNew);
	sem_post(procesoNecesitaEntrarEnReady);

	enviarInformacionAdministrativaDelProceso(procesoNuevo);

}




void cerrarProceso(t_buffer* bufferActual,t_log* logger){

	void* stream = bufferActual->stream;
	uint32_t pidProcesoAEliminar;
	int desplazamiento = 0;

	memcpy(&(pidProcesoAEliminar), stream+desplazamiento, sizeof(uint32_t));

	

	cerrarProcesoKernelYDemasConexiones(pidProcesoAEliminar, logger);

}

void informarCierreDeProceso(int conexion){

	t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
    enviarPaquete(paquete,conexion);

}


void cerrarProcesoKernelYDemasConexiones(uint32_t pidActual,t_log* logger){

	
	bool buscarProcesoPorPid(proceso_kernel* proceso){
		if(proceso->pid == pidActual){
			return 1;
		}
		return 0;
	}

	/* notificar a memoria */
	log_info(logger,"Le notificamos a memoria para que libere la memoria del carpincho: ", string_itoa(pidActual));

	pthread_mutex_lock(modificarExec); //busco al proceso y lo saco
		proceso_kernel* procesoActual =list_remove_by_condition(procesosExec, buscarProcesoPorPid);
	pthread_mutex_lock(modificarExec);

	sem_post(nivelMultiprocesamiento); //aumento el grado de multiprogramacion y multiprocesamiento
	sem_post(nivelMultiProgramacionGeneral);

	informarCierreDeProceso(procesoActual->conexion);

	log_info(logger,"Se nos va el carpincho:",string_itoa(procesoActual->pid));

	free(procesoActual); //libero la estructura

}

