#include "matelib.h"



//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config){
    
    int conexion = inicializarPrimerasCosas(lib_ref,config);
    

    if(conexion == -1){
        lib_ref->group_info->backEndConectado = ERROR;
    }else{

        solicitarIniciarPatota(conexion, lib_ref);
        recibir_mensaje(conexion, lib_ref);
    }
    return lib_ref->group_info->backEndConectado;
}



int mate_close(mate_instance *lib_ref){

    if(validarConexionPosible(KERNEL, lib_ref->group_info->backEndConectado)==1){
        log_info(lib_ref->group_info->loggerProceso,"Se ha solicitado cerrar el carpincho, este es un hasta adios");

        solicitarCerrarPatota(lib_ref->group_info->conexionConBackEnd, lib_ref);
        log_info(lib_ref->group_info->loggerProceso,"Se envio info para cerrarlo");
        return recibir_mensaje(lib_ref->group_info->conexionConBackEnd, lib_ref);
    }else{
        perror("Se esta intentando realizar una operacion Kernel, al cual no estoy autorizado por mi Backend");
        return -1;
    }    
}






//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    if(validarConexionPosible(KERNEL, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion Kernel, al cual no estoy autorizado por mi Backend");
        return -1;
    }

}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    
    if(validarConexionPosible(KERNEL, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion Kernel, al cual no estoy autorizado por mi Backend");
        return -1;
    }
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
    
    if(validarConexionPosible(KERNEL, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion Kernel, al cual no estoy autorizado por mi Backend");
        return -1;
    }
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){
    
    if(validarConexionPosible(KERNEL, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion Kernel, al cual no estoy autorizado por mi Backend");
        return -1;
    }
}


//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){
    
    if(validarConexionPosible(KERNEL, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion Kernel, al cual no estoy autorizado por mi Backend");
        return -1;
    }
}







//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
    
    if(validarConexionPosible(MEMORIA, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion en Memoria, pero hubo un fallo en la conexion");
        return -1;
    } 
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){
    
    if(validarConexionPosible(MEMORIA, lib_ref->group_info->backEndConectado)==1){
        /* toda la logica de lo que tiene que hacer */
        return 0;
    }else{
        perror("Se esta intentando realizar una operacion en Memoria, pero hubo un fallo en la conexion");
        return -1;
    } 
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


int recibir_mensaje(int conexion, mate_instance* lib_ref) {

	t_paquete* paquete = malloc(sizeof(t_paquete));


	if(recv(conexion, &(paquete->codigo_operacion), sizeof(cod_operacion), 0) < 1){
		free(paquete);
		perror("Fallo en recibir la info de la conexion");
        return -1;
	}
    int codigoOperacion = paquete->codigo_operacion;

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);

    if(paquete->buffer->size > 0){
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
    }

    
	switch(paquete->codigo_operacion){
        case INICIALIZAR_ESTRUCTURA:;
            agregarInfoAdministrativa(conexion,lib_ref, paquete->buffer);
			break;
        case CERRAR_INSTANCIA:;
            log_info(lib_ref->group_info->loggerProceso,"Estamos recibiendo todo para cerrar la conexion y terminar");
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

    return codigoOperacion;
}

void agregarInfoAdministrativa(int conexion, mate_instance* lib_ref, t_buffer* buffer){
	void* stream = buffer->stream;
	int offset = 0;

	memcpy(&(lib_ref->group_info->pid), stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(lib_ref->group_info->backEndConectado), stream+offset, sizeof(uint32_t));

    if(lib_ref->group_info->pid < 0 ){
            perror("No se pudo crear la instancia :(");
    }else{

    lib_ref->group_info->conexionConBackEnd = conexion;
        
        
    char* nombreLog = string_new();
    string_append(&nombreLog, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/Proceso");
    char* pidCarpincho = string_itoa((int) lib_ref->group_info->pid);
    string_append(&nombreLog, pidCarpincho);
    string_append(&nombreLog, ".log");

    lib_ref->group_info->loggerProceso = log_create(nombreLog,"loggerContenidoProceso",0,LOG_LEVEL_DEBUG);
    
    log_info(lib_ref->group_info->loggerProceso,"Se ha creado el carpincho, y se ha logrado conectar correctamente al backend:%d ",lib_ref->group_info->backEndConectado);

    free(pidCarpincho);
    free(nombreLog);
    }

}



void liberarEstructurasDeProceso(mate_instance* lib_ref){
    
    log_info(lib_ref->group_info->loggerProceso,"Se liberan todas las estructuras del proceso");
    
    log_destroy(lib_ref->group_info->loggerProceso);
    config_destroy(lib_ref->group_info->config);
    close(lib_ref->group_info->conexionConBackEnd);
    free(lib_ref->group_info);
    //free(lib_ref);
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


int validarConexionPosible(int tipoSolicitado, int tipoActual){

    if(tipoSolicitado == KERNEL){ 
        if(tipoActual == KERNEL){ //si deseo hacer una operacion de tipo General (Kernel), el backend entonces debe ser de tipo kernel
            return 1;
        }
    }else if(tipoSolicitado == MEMORIA){ 
        if(tipoActual == KERNEL || tipoActual == MEMORIA){ //si deseo hacer una operacion de tipo Memoria, con que el backend no sea un error de conexion, se puede. EL kernel hara de pasamanos
            return 1;
        }
    }

    return 0; //el otro caso seria que el tipoActual sea error o que no cumpla las condiciones prestablecidas, entonces retorna 0 en referencia que no se podra hacer
}


int main(){
    mate_instance referencia;
    mate_init(&referencia, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/configProcesos.config");
    mate_close(&referencia);
    return 0;
}