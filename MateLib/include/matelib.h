#ifndef PROCESO1_H
#define PROCESO1_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>
#include "shared_utils.h"
#include <commons/config.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <unistd.h>





typedef struct
{
    uint32_t pid; // identificador de cada proceso que se vaya a instanciar.
    
    int conexionConBackEnd; // se obtiene informacion del backend (memoria o kernel) - guarda el socket al servidor que se esta conectando.
    backend backEndConectado; // id de backend al que nos estamos conectando. En caso de que sea -1 error.
    t_log* loggerProceso; // el log nos va a dar la referencia a la hora de planificar al proceso.

}mate_struct;


typedef struct mate_instance
{
    mate_struct* group_info; // Tenemos un puntero a la estructura del "Carpincho" (es decir su estructura administrativa). 

} mate_instance;


typedef char *mate_io_resource;

typedef char *mate_sem_name;

typedef int32_t mate_pointer;

//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config);

int mate_close(mate_instance *lib_ref);

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg);

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size);

int mate_memfree(mate_instance *lib_ref, mate_pointer addr);

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size);


/*------ Funciones extras --------*/

// int conectarseABackEnd(char* ipBackEnd, char*puertoBackEnd);

int inicializarPrimerasCosas(mate_instance *lib_ref, char *config);

void recibir_mensaje(int conexion, mate_instance* lib_ref);

void agregarInfoAdministrativa(mate_instance* lib_ref, t_buffer* buffer);

void liberarEstructurasDeProceso(mate_instance* lib_ref);

/* ------- Solicitudes  --------------------- */

void solicitarIniciarPatota(int conexion, mate_instance* lib_ref);

void solicitarCerrarPatota(int conexion, mate_instance* lib_ref);

void inicializarSemaforo(int conexion, mate_sem_name nombreSemaforo, unsigned int valor);

void realizarWaitSemaforo(int conexion, mate_sem_name nombreSemaforo);

void realizarPostSemaforo(int conexion, mate_sem_name nombreSemaforo);

void liberarSemaforo(int conexion, mate_sem_name nombreSemaforo);

void realizarLlamadoDispositivoIO(mate_instance *lib_ref, mate_io_resource io, void *msg);

int validarConexionPosible(int tipoSolicitado, int tipoActual);



#endif