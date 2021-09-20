#include "matelib.h"



//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config){
    
    int conexion = inicializarPrimerasCosas(lib_ref,config);
    

    if(conexion == -1){
        lib_ref->group_info->backEndConectado = ERROR;
    }else{

        solicitarIniciarPatota(conexion, lib_ref);
        recibir_mensaje(conexion, lib_ref);
        

        if(lib_ref->group_info->pid < 0 ){
            perror("No se pudo crear la instancia :(");
            return -1;
        }

        lib_ref->group_info->conexionConBackEnd = conexion;
        
        
        char* nombreLog = string_new();
        string_append(&nombreLog, "Proceso ");
        string_append(&nombreLog, string_itoa((int) lib_ref->group_info->pid));
        string_append(&nombreLog, ".log");

        lib_ref->group_info->loggerProceso = log_create(nombreLog,"loggerContenidoProceso",0,LOG_LEVEL_DEBUG);

        free(nombreLog);
    }
    return lib_ref->group_info->backEndConectado;
}



int mate_close(mate_instance *lib_ref){

    solicitarCerrarPatota(lib_ref->group_info->conexionConBackEnd, lib_ref);
    recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);

    return 0;
}






//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    return 0;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    return 0;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
    return 0;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){
    return 0;
}


//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){
    return 0;
}







//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
    return 0;
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){
    return 0;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){
    return 0;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){
    return 0;    
}







//--------- Funciones extras---------//


int inicializarPrimerasCosas(mate_instance *lib_ref, char *config){

    lib_ref->group_info = malloc(sizeof(mate_struct));

    t_config* datosBackEnd = config_create(config);
    char* ipBackEnd = config_get_string_value(datosBackEnd,"IP_BACKEND");
    char* puertoBackEnd = config_get_string_value(datosBackEnd,"PUERTO_BACKEND");
    int conexionConBackEnd = crear_conexion(ipBackEnd, puertoBackEnd);
    
    free(config);

    return conexionConBackEnd;
}


void recibir_mensaje(int conexion, mate_instance* lib_ref) {

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
            agregarInfoAdministrativa(lib_ref, paquete->buffer);
			break;
        case CERRAR_INSTANCIA:;
            liberarEstructurasDeProceso(lib_ref);
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


    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
}

void agregarInfoAdministrativa(mate_instance* lib_ref, t_buffer* buffer){
	void* stream = buffer->stream;
	int offset = 0;

	memcpy(&(lib_ref->group_info->pid), stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(lib_ref->group_info->backEndConectado), stream+offset, sizeof(uint32_t));

}



void liberarEstructurasDeProceso(mate_instance* lib_ref){
    log_destroy(lib_ref->group_info->loggerProceso);
    free(lib_ref->group_info);
    close(lib_ref->group_info->conexionConBackEnd);
    free(lib_ref);
}


/* ------- Solicitudes  --------------------- */

void solicitarIniciarPatota(int conexion, mate_instance* lib_ref){
    
    t_paquete* paquete = crear_paquete(INICIALIZAR_ESTRUCTURA);

    enviarPaquete(paquete,conexion);
}


void solicitarCerrarPatota(int conexion, mate_instance* lib_ref){
    
    t_paquete* paquete = crear_paquete(CERRAR_INSTANCIA);

    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &(lib_ref->group_info->pid), paquete->buffer->size);

    enviarPaquete(paquete,conexion);
}

 

void inicializarSemaforo(int conexion, mate_sem_name nombreSemaforo, unsigned int valor){
    t_paquete* paquete = crear_paquete(INICIAR_SEMAFORO);

    paquete->buffer->size = sizeof(uint32_t)*2 + string_length(nombreSemaforo) +1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = string_length(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , tamanioNombre);
    desplazamiento += tamanioNombre;

    memcpy(paquete->buffer->stream + desplazamiento, &(valor) , sizeof(uint32_t));

    enviarPaquete(paquete,conexion);
}


void realizarWaitSemaforo(int conexion, mate_sem_name nombreSemaforo){
    t_paquete* paquete = crear_paquete(SEM_WAIT);

    paquete->buffer->size = sizeof(uint32_t) + string_length(nombreSemaforo) +1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = string_length(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , tamanioNombre);

    enviarPaquete(paquete,conexion);
}

void realizarPostSemaforo(int conexion, mate_sem_name nombreSemaforo){
    t_paquete* paquete = crear_paquete(SEM_SIGNAL);

    paquete->buffer->size = sizeof(uint32_t) + string_length(nombreSemaforo) +1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = string_length(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , string_length(nombreSemaforo)+1);

    enviarPaquete(paquete,conexion);
}


void liberarSemaforo(int conexion, mate_sem_name nombreSemaforo){
    t_paquete* paquete = crear_paquete(CERRAR_SEMAFORO);

    paquete->buffer->size = sizeof(uint32_t) + string_length(nombreSemaforo) +1;
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;
    uint32_t tamanioNombre = string_length(nombreSemaforo)+1;

    memcpy(paquete->buffer->stream + desplazamiento, &(tamanioNombre) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, nombreSemaforo , tamanioNombre);

    enviarPaquete(paquete,conexion);
}





void realizarLlamadoDispositivoIO(mate_instance *lib_ref, mate_io_resource io, void *msg){
    
}


void enviarPaquete(t_paquete* paquete, int conexion){

    int bytes = paquete->buffer->size + sizeof(cod_operacion) + sizeof(uint32_t);
    void* contenido_a_enviar= serializar_paquete(paquete, bytes);
    send(conexion, contenido_a_enviar,bytes,0);
    free(contenido_a_enviar);
}



int main(){
    return 0;
}