#ifndef PROCESO1_H
#define PROCESO1_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include "shared_utils.h"
#include <commons/config.h>
#include <commons/string.h>


typedef struct mate_instance
{
    void *group_info;

    uint32_t pid;
    char* ipBackEnd;
    char* puertoBackEnd;
    uint32_t backEndConectado;
    t_log* loggerProceso;
    t_config* configUtilizado;

} mate_instance;


typedef enum{

    ERROR = -1,
    KERNEL = 0,
    MEMORIA = 1

}backend;



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

int conectarseABackEnd(mate_instance *lib_ref);

void inicializarPrimerasCosas(mate_instance *lib_ref, char *config);

void recibir_mensaje(int conexion, mate_instance* lib_ref);

void agregarInfoAdministrativa(mate_instance* lib_ref, t_buffer* buffer);

void liberarEstructurasDeProceso(mate_instance* lib_ref);

/* ------- Solicitudes  --------------------- */

void solicitarIniciarPatota(int conexion, mate_instance* lib_ref);

void solicitarCerrarPatota(int conexion, mate_instance* lib_ref);



#endif