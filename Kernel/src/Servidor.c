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

int atenderMensajeEnKernel(int conexion) {

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
			inicializarProcesoNuevo(conexion);
			break;
        case CERRAR_INSTANCIA:;
			cerrarProceso(paquete->buffer);
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

	return valorOperacion;
	
}



void inicializarProcesoNuevo(int conexion){

	proceso* procesoNuevo = malloc(sizeof(proceso));
	pthread_mutex_lock(contadorProcesos);
		procesoNuevo->pid = cantidadDeProcesosActual;
		cantidadDeProcesosActual++;
	pthread_mutex_unlock(contadorProcesos);
	procesoNuevo->conexion = conexion;
	procesoNuevo->estimacionAnterior = 1 /*tengo q asignarle al del config */;
	procesoNuevo->tiempoDeEspera = 0;
	procesoNuevo->ultimaRafaga = 0;

	pthread_mutex_lock(modificarNew);
		list_add(procesosNew, procesoNuevo);
	pthread_mutex_unlock(modificarNew);
	sem_post(hayProcesosNew);
	sem_post(procesoNecesitaEntrarEnReady);

	enviarInformacionAdministrativaDelProceso(procesoNuevo);

}

void enviarInformacionAdministrativaDelProceso(proceso* proceso){

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


void cerrarProceso(t_buffer* bufferActual){

	void* stream = bufferActual->stream;
	uint32_t pidProcesoAEliminar;
	int desplazamiento = 0;

	memcpy(&(pidProcesoAEliminar), stream+desplazamiento, sizeof(uint32_t));

	cerrarProcesoKernelYDemasConexiones(pidProcesoAEliminar);

}


void cerrarProcesoKernelYDemasConexiones(uint32_t pidActual){

	/* notificar a memoria */
	int buscarProcesoPorPid(proceso* proceso){
		if(proceso->pid == pidActual){
			return 1;
		}
		return 0;
	}

	pthread_mutex_lock(modificarExec); //busco al proceso y lo saco
		proceso* procesoActual =list_remove_by_condition(procesosExec, buscarProcesoPorPid);
	pthread_mutex_lock(modificarExec);

	sem_post(nivelMultiprocesamiento); //aumento el grado de multiprogramacion y multiprocesamiento
	sem_post(nivelMultiProgramacionGeneral);

	informarCierreDeProceso(procesoActual);


	close(procesoActual->conexion); //cierro la conexion

	free(procesoActual); //libero la estructura

}

void informarCierreDeProceso(proceso* procesoActual){

	t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);
    enviarPaquete(paquete,procesoActual->conexion);

}