#include "matelib.h"



//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config){

    int conexion = inicializarPrimerasCosas(lib_ref,config);
    

    if(conexion == -1){
        lib_ref->group_info->backEndConectado = ERROR;
        return lib_ref->group_info->backEndConectado;
    }else{

        solicitarIniciarPatota(conexion, lib_ref);
        return recibir_mensaje(conexion, lib_ref);
    }
    
}



int mate_close(mate_instance *lib_ref){

    if(validarConexionPosible(MEMORIA, lib_ref->group_info->backEndConectado)==1){
        log_info(lib_ref->group_info->loggerProceso,"Se ha solicitado cerrar el carpincho, este es un hasta adios");

        solicitarCerrarPatota(lib_ref->group_info->conexionConBackEnd, lib_ref);
        int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
        return valorRetorno;
    }else{
        perror("Se esta intentando realizar una operacion general, pero como la conexion fue erronea no se pudo");
        return -1;
    }    
}






//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    
        
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos inicializar un semaforo de nombre: %s, con valor : %d", sem, value);
    inicializarSemaforo(lib_ref->group_info->conexionConBackEnd, sem, value);
    int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    return valorRetorno;

}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos destruir un semaforo de nombre: %s", sem);
    liberarSemaforo(lib_ref->group_info->conexionConBackEnd, sem);
    int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    return valorRetorno;
    
}



int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos hacer un wait sobre el semaforo: %s", sem);
    realizarWaitSemaforo(lib_ref->group_info->conexionConBackEnd, sem, lib_ref->group_info->pid);
    int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    return valorRetorno;
      
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
    
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos hacer un signal sobre el semaforo: %s", sem);
    realizarPostSemaforo(lib_ref->group_info->conexionConBackEnd, sem);
    int valorRetorno = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    return valorRetorno;
      
}




//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void* msg){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar una operacion IO sobre el dispositivo: %s", io);
    realizarLlamadoDispositivoIO(lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, io);
    int valor = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    return valor;
    
}







//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
    
    log_info(lib_ref->group_info->loggerProceso,"Solicitamos realizar un memalloc de size:%d", size);
    realizarMemAlloc(lib_ref->group_info->conexionConBackEnd, lib_ref->group_info->pid, size);
    mate_pointer valor = (int) recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    return valor;
    
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){
    
    
    /* toda la logica de lo que tiene que hacer */
    return 0;
    
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){
    
    if(validarConexionPosible(MEMORIA, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion en Memoria, pero hubo un fallo en la conexion");
        return -1;
    } 
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){
    
    if(validarConexionPosible(MEMORIA, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion en Memoria, pero hubo un fallo en la conexion");
        return -1;
    }   
}







//--------- Funciones extras---------//

int inicializarPrimerasCosas(mate_instance *lib_ref, char *config){

    lib_ref->group_info = malloc(sizeof(mate_struct));

    t_config* datosBackEnd = config_create(config);
    char* ipBackEnd = config_get_string_value(datosBackEnd,"IP_BACKEND");
    char* puertoBackEnd = config_get_string_value(datosBackEnd,"PUERTO_BACKEND");
    int conexionConBackEnd = crear_conexion(ipBackEnd, puertoBackEnd);
    
    lib_ref->group_info->config = datosBackEnd;
    

    return conexionConBackEnd;
}


void* recibir_mensaje(int conexion, mate_instance* lib_ref) {

	t_paquete* paquete = malloc(sizeof(t_paquete));


	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		perror("Fallo en recibir la info de la conexion");
        return -1;
	}
    int valorRetorno;
    //void* retornoMensaje;

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
            break;
        case MEMREAD:;
            break;
        case MEMWRITE:;
            break;
        default:;
            break;
	}


    if(paquete->buffer->size > 0){
	    free(paquete->buffer->stream);
    }

	free(paquete->buffer);
	free(paquete);
/* 
    if(paquete->codigo_operacion == CONECTAR_IO || paquete->codigo_operacion == MEMREAD){
        return retornoMensaje;
    }
*/
    return valorRetorno;
}


/* Respuestas del Backend */

int agregarInfoAdministrativa(int conexion, mate_instance* lib_ref, t_buffer* buffer){
	void* stream = buffer->stream;
	int offset = 0;

	memcpy(&(lib_ref->group_info->pid), stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(lib_ref->group_info->backEndConectado), stream+offset, sizeof(uint32_t));

    if(lib_ref->group_info->pid < 0 ){
            perror("No se pudo crear la instancia :(");
            return -1;
    }else{

    lib_ref->group_info->conexionConBackEnd = conexion;
        
        
    char* nombreLog = string_new();
    string_append(&nombreLog, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/Proceso");
    char* pidCarpincho = string_itoa((int) lib_ref->group_info->pid);
    string_append(&nombreLog, pidCarpincho);
    string_append(&nombreLog, ".log");

    lib_ref->group_info->loggerProceso = log_create(nombreLog,"loggerContenidoProceso",1,LOG_LEVEL_DEBUG);
    
    log_info(lib_ref->group_info->loggerProceso,"Se ha creado el carpincho, y se ha logrado conectar correctamente al backend:%d ",lib_ref->group_info->backEndConectado);

    free(pidCarpincho);
    free(nombreLog);

    return 0;
    }

}



int liberarEstructurasDeProceso(t_buffer* buffer, mate_instance* lib_ref){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 0){
        log_info(lib_ref->group_info->loggerProceso,"Se pudo cerrar el proceso correctamente en el BackEnd");
    }else{
        log_error(lib_ref->group_info->loggerProceso,"No se pudo cerrar el proceso correctamente en el BackEnd");
    }



    log_info(lib_ref->group_info->loggerProceso,"Se liberan todas las estructuras del proceso");
    log_destroy(lib_ref->group_info->loggerProceso);
    config_destroy(lib_ref->group_info->config);
    close(lib_ref->group_info->conexionConBackEnd);
    free(lib_ref->group_info);

    return valor;

}


int notificacionDeCreacionDeSemaforo(t_buffer* buffer, t_log* logger){
    
    void* stream = buffer->stream;
	int desplazamiento = 0;
	int valor;
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor == 0){
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

    if(valor == 0){
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

    if(valor == 0){
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

    if(valor == 0){
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

    //habria que innicializarlo??
    //void* mensajeRecibido;
    //int espacioDeMensaje;


	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    if(valor == 0){
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
	memcpy(&(valor), stream+desplazamiento, sizeof(uint32_t));

    if(valor < 0){
        log_info(logger,"No se pudo hacer el memalloc del size solicitado");
    }else{
        log_error(logger,"Se pudo realizar el memalloc");
    }

    return valor;
}



/* ------- Solicitudes  --------------------- */


/* estructuracion */
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

    paquete->buffer->size = sizeof(uint32_t) *2 + sizeof(int32_t) + size;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pid) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(origin) , sizeof(int32_t));
    desplazamiento += sizeof(int32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &(size) , sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

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


/*VALIDACIONES PARA REALIZAR TAREAS */

int validarConexionPosible(int tipoSolicitado, int tipoActual){

    
    if(tipoSolicitado == MEMORIA){ 
        if(tipoActual == KERNEL || tipoActual == MEMORIA){ //si deseo hacer una operacion de tipo Memoria, con que el backend no sea un error de conexion, se puede. EL kernel hara de pasamanos
            return 1;
        }
    }

    return 0; //el otro caso seria que el tipoActual sea error o que no cumpla las condiciones prestablecidas, entonces retorna 0 en referencia que no se podra hacer
}


int main(){
    mate_instance* referencia = malloc(sizeof(mate_instance)); //porque rompe si hacemos el malloc en el mate_init?

    mate_init(referencia, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/configProcesos.config");
    
    //mate_sem_init(referencia,"SEM1",1);
    mate_sem_post(referencia,"SEM1");
    //mate_sem_wait(referencia,"SEM1");
    //mate_sem_wait(referencia,"SEM1");
    mate_call_io(referencia,"laguna","asd");
   
    mate_close(referencia);
    free(referencia);

    return 0;
}

