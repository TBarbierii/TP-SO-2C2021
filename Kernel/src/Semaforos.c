#include "Semaforos.h"

int crearSemaforo(char* nombreSem, unsigned int valorSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_DEBUG);

    bool semaforoYaCreado(semaforo* semaforoNuevo){ //se dice creado porque va a tener el mismo nombre 
        if(strcmp(semaforoNuevo->nombre, nombreSem) == 0){
        return 1;
        }
        return 0;
    }

    pthread_mutex_lock(controladorSemaforos);
        t_list* listaTemporal = list_filter(semaforosActuales, semaforoYaCreado); //vamos a usar esta lista para ver si esta vacia, si no encontro  un semaforo con tal nombre, sera vacia, entonces lo vamos a crear desde 0
                                                                                  // sino no lo vamos a crear y avisamos que ya estaba creado
        int estaCreado = !(list_is_empty(listaTemporal));
        list_destroy(listaTemporal);
    pthread_mutex_unlock(controladorSemaforos);


    if(!estaCreado){
        semaforo* semaforoNuevo = malloc(sizeof(semaforo));
        semaforoNuevo->nombre = nombreSem;
        semaforoNuevo->valor = valorSem;
        log_info(logger,"[SEMAFOROS] Se ha creado un nuevo semaforo de nombre: %s y valor: %u",nombreSem, valorSem);
        semaforoNuevo->listaDeProcesosEnEspera = list_create();
        //le vamos a dar un mutex al semaforo para agregar y sacar elementos de la su lista de bloqueados
        semaforoNuevo->mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(semaforoNuevo->mutex,NULL);

        /*esto se va a hacer con el uso de un mutex para controlar el uso de la lista de semaforos global */
        pthread_mutex_lock(controladorSemaforos);
            semaforoNuevo->id = valorIdSemaforos;
            list_add(semaforosActuales, semaforoNuevo);
            valorIdSemaforos++;
        pthread_mutex_unlock(controladorSemaforos);

        log_destroy(logger);
        return 1;
    }else{
        log_warning(logger,"[SEMAFOROS] Se esta intentando crear un semaforo: %s pero ya esta creado", nombreSem);
        log_destroy(logger);
        free(nombreSem);
        return 0;
    }
    

}


int destruirSemaforo(char* nombreSem){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);
    
    bool buscarPorNombre(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre ,nombreSem) == 0){
            return 1;
        }else{
            return 0;
        }
    }


    pthread_mutex_lock(controladorSemaforos);
        semaforo* semaforoNuevo = list_remove_by_condition(semaforosActuales, buscarPorNombre);
    pthread_mutex_unlock(controladorSemaforos);

    if(semaforoNuevo != NULL){
        log_info(logger,"[SEMAFOROS] Se va a destruir un semaforo llamado: %s", nombreSem);    
        /*aca deberiamos sacar a todos los elementos que se encuentran bloqueados en el semaforo y ponerlos en ready */
        //eso lo tengo con una funcion quiza que lo ponga en ready si estaba en blocked, y en suspended- reday si estaba en suspended-blocked
        list_destroy(semaforoNuevo->listaDeProcesosEnEspera);
        pthread_mutex_destroy(semaforoNuevo->mutex);
        
        free(semaforoNuevo->mutex);
        free(semaforoNuevo->nombre);
        free(semaforoNuevo);
        log_destroy(logger);
        return 1;
    }else{
        log_warning(logger,"[SEMAFOROS] Se esta intentando destruir un semaforo: %s, el cual no existe", nombreSem);
        log_destroy(logger);
        return 0;
    }


    
    
}


int realizarSignalDeSemaforo(char* nombreSem, int pid){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);

    bool semaforoConNombreSolicitado(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre ,nombreSem) == 0){
            return 1;
        }else{
            return 0;
        }
    }


    //arancamos a buscar el elemento si se encuentra en la lista de semaforos
    pthread_mutex_lock(controladorSemaforos);
        semaforo* semaforoActual = list_find(semaforosActuales, semaforoConNombreSolicitado);
    pthread_mutex_unlock(controladorSemaforos);


    if(semaforoActual != NULL){ //si el semaforo existe, realizamos los respectivos cambios

        pthread_mutex_lock(semaforoActual->mutex);

        semaforoActual->valor++;

        log_info(logger,"[SEMAFOROS] El proceso:%d realizo un signal del semaforo: %s y el valor se incremento a: %d",pid, nombreSem, semaforoActual->valor);

        bool buscarProcesoConPid(proceso_kernel* procesoBuscado){
                if(procesoBuscado->pid == pid){
                    return 1;
                }else{
                    return 0;
                }
            }

            
        pthread_mutex_lock(modificarExec);
            proceso_kernel* procesoAbloquear = list_find(procesosExec,buscarProcesoConPid);
        pthread_mutex_unlock(modificarExec);
            //le sacamos el recurso que esta reteniendo ya que no lo retendria mas
            list_remove_by_condition(procesoAbloquear->listaRecursosRetenidos, semaforoConNombreSolicitado);


        if(!list_is_empty(semaforoActual->listaDeProcesosEnEspera)){ // si tiene procesos esperando, liberamos 1
            
            proceso_kernel* procesoLiberado = list_remove(semaforoActual->listaDeProcesosEnEspera, 0);
                
                //ahora el primero que estaba bloqueado en ese semaforo, pasara a retener al semaforo
                list_add(procesoLiberado->listaRecursosRetenidos, semaforoActual);
                //y como era el unico semaforo que estaba solicitando se lo sacamos de su lista de solicitadoss
                list_remove(procesoLiberado->listaRecursosSolicitados, 0);
            pthread_mutex_unlock(semaforoActual->mutex);
                

            log_info(logger,"[SEMAFOROS] Se libera el proceso:%d, de la lista de espera del semaforo: %s", pid, nombreSem);

            /* sacamos al proceso de la lista de bloqueados */
            ponerEnElReadyIndicado(procesoLiberado);

            


        }else{ // si no tiene procesos esperando, solo notificamos que se cambios el valor del semaforo
            pthread_mutex_unlock(semaforoActual->mutex);
            log_info(logger,"[SEMAFOROS] Como el semaforo: %s, no tiene procesos esperando solo se aumenta su valor", nombreSem);
            
        }

        log_destroy(logger);
        return 1;
    
    }else{ //si no existe avisamos que se quiso hacer un acmbio sobre un semaforo que no existe
        
        log_warning(logger,"[SEMAFOROS] El proceso:%d, esta intentando hacer un signal de un semaforo: %s, el cual no existe", pid, nombreSem);
        log_destroy(logger);
        return 0;

    }
}




int realizarWaitDeSemaforo(char* nombreSem, int pid){

    t_log* logger = log_create("cfg/Semaforos.log","Semaforos",0,LOG_LEVEL_INFO);

    bool semaforoConNombreSolicitado(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre ,nombreSem) == 0){
            return 1;
        }else{
            return 0;
        }
    }

    //arancamos a buscar el elemento si se encuentra en la lista de semaforos
    pthread_mutex_lock(controladorSemaforos);
        semaforo* semaforoActual = list_find(semaforosActuales, semaforoConNombreSolicitado);
    pthread_mutex_unlock(controladorSemaforos);


    if(semaforoActual != NULL){ //si el semaforo existe, realizamos los respectivos cambios (funciona como un wait ya que modificamos el valor de dicho semaforo)

        pthread_mutex_lock(semaforoActual->mutex);

        semaforoActual->valor--;

        log_info(logger,"[SEMAFOROS] El proceso:%d realizo un wait del semaforo: %s y el valor decrecio a: %d",pid, nombreSem, semaforoActual->valor);

        bool buscarProcesoConPid(proceso_kernel* procesoBuscado){
            return procesoBuscado->pid == pid;
            }

        if(semaforoActual->valor < 0){ //si el valor es menor a 1, lo vamos a bloquear
            
            

            //lo sacamos de la lista de ejecucion
            pthread_mutex_lock(modificarExec);
            proceso_kernel* procesoAbloquear = list_remove_by_condition(procesosExec,buscarProcesoConPid);
            pthread_mutex_unlock(modificarExec);
                //le asignamos que va a solicitar el recurso, porque no lo va a poder retener
                list_add(procesoAbloquear->listaRecursosSolicitados, semaforoActual);
                list_add(semaforoActual->listaDeProcesosEnEspera, procesoAbloquear); /* lo agregamos a la lista de bloqueados del semaforo */
            pthread_mutex_unlock(semaforoActual->mutex);


            log_info(logger,"[SEMAFOROS] Se agrega el proceso con Pid:%d en bloqueado, y se agrega en la lista de espera del semaforo: %s", pid, nombreSem);
            

            /* agregamos al proceso en la lista de bloqueados */
            pthread_mutex_lock(modificarBlocked);
            list_add(procesosBlocked,procesoAbloquear);
            //ponemos que lo ultimo que realizo fue un bloqueo a un semaforo
            procesoAbloquear->vuelveDeBloqueo = BLOCK_SEM;
            pthread_mutex_unlock(modificarBlocked);
            sem_post(signalSuspensionProceso);

            log_destroy(logger);
            return 1;

        }else{ // si el valor no es <0, no se bloquearia el proceso


            //lo sacamos de la lista de ejecucion
            pthread_mutex_lock(modificarExec);
            proceso_kernel* procesoAbloquear = list_find(procesosExec,buscarProcesoConPid);
            pthread_mutex_unlock(modificarExec);
                //le asignamos que tiene el recurso como retenido
                list_add(procesoAbloquear->listaRecursosRetenidos, semaforoActual);
            pthread_mutex_unlock(semaforoActual->mutex);
            log_info(logger,"[SEMAFOROS] Se ejecuto un wait sobre un semaforo:%s, pero como el valor no era menor a 1, entonces el proceso:%d no se bloqueo", nombreSem, pid);
            

            log_destroy(logger);
            return 2;
        }
    
    }else{ //si no existe avisamos que se quiso hacer un cambio sobre un semaforo que no existe
        
        log_warning(logger,"[SEMAFOROS] Se esta intentando hacer un wait de un semaforo: %s, el cual no existe", nombreSem);
        log_destroy(logger);
        return 0;

    }
}



/* esta es tanto para semaforos como para IO */




void sacarProcesoDeBloqueado(int PID){
    
    int procesoBloqueado(proceso_kernel * proceso){
        return proceso->pid == PID;
    }

    proceso_kernel* procesoASacar = list_remove_by_condition(procesosBlocked, procesoBloqueado);
    if(procesoASacar == NULL){
        
        proceso_kernel* procesoASacarSiNoEstaEnBlock = list_remove_by_condition(procesosSuspendedBlock, procesoBloqueado);
    }


}


void ponerEnElReadyIndicado(proceso_kernel* procesoBuscado){
    
    bool seEncuentraProceso(proceso_kernel* procesoActual){ 
        if(procesoActual->pid == procesoBuscado->pid){
        return 1;
        }
        return 0;
    }



    pthread_mutex_lock(modificarSuspendedBlocked);
        proceso_kernel* procesoActual = list_remove_by_condition(procesosSuspendedBlock, seEncuentraProceso);                                                    
    pthread_mutex_unlock(modificarSuspendedBlocked);

        if(procesoActual != NULL){ //si esta, se agrega a esta lista, y se notifica que hay un procesoo que quiere entrar a READY

            pthread_mutex_lock(modificarSuspendedReady);
                list_add(procesosSuspendedReady, procesoBuscado);
            pthread_mutex_unlock(modificarSuspendedReady);

            sem_post(procesoNecesitaEntrarEnReady); //alertamos que hay un proceso que solicita entrar en ready
            
            //esto es un aviso para el planificador de mediano plazo de que hay un proceso nuevo y quiza deberia entrar si solo hay procesos de Bloqueados
            sem_post(signalSuspensionProceso);

        }else{ //si esta solo bloqueado, se aagrega directo a ready y lo sacamos de blocked

            pthread_mutex_lock(modificarBlocked);
                list_remove_by_condition(procesosBlocked, seEncuentraProceso);
            pthread_mutex_unlock(modificarBlocked);


            pthread_mutex_lock(modificarReady);
                list_add(procesosReady, procesoBuscado);
                clock_gettime(CLOCK_REALTIME, &procesoBuscado->tiempoDeArriboColaReady); //esto sirve para HRRN, para estimar cuando empezo un proceso a estar en ready y cuanto tiempo pasa ahi
            pthread_mutex_unlock(modificarReady);
            
            sem_post(hayProcesosReady); //alertamos que hay un proceso nuevo en ready

        }

}



void desalojarSemaforosDeProceso(proceso_kernel* procesoASacarPorDeadlock){

    int procesoConPid(proceso_kernel* procesoBuscado){
        return procesoBuscado->pid == procesoASacarPorDeadlock->pid;
    }
    // s3 -> 3 -> s4 -> 4 -> s1


    //primero lo sacamos de la lista de los seamforos que esta esperando
    while(!list_is_empty(procesoASacarPorDeadlock->listaRecursosSolicitados)){
        
        semaforo* semaforoLiberado = list_remove(procesoASacarPorDeadlock->listaRecursosSolicitados, 0);
        
        //por cada semaforo que esta esperando, aumentamos su valor y lo sacamos de la lista de espera del mismo
        semaforoLiberado->valor++;
        list_remove_by_condition(semaforoLiberado->listaDeProcesosEnEspera, procesoConPid);

        printf("xd");
    
    }

    while(!list_is_empty(procesoASacarPorDeadlock->listaRecursosRetenidos)){
        
        semaforo* semaforoRetenido = list_remove(procesoASacarPorDeadlock->listaRecursosRetenidos, 0);
        //4->listaRecursosRetenidos =  {s4}
        //por cada semaforo que esta reteniendo, aumentamos su valor 
        semaforoRetenido->valor++;
        
        //s4->listaProcesosEnEspera = {p3}
        if(!list_is_empty(semaforoRetenido->listaDeProcesosEnEspera)){ //si tiene cola de espera, vamos a poner al primero que retenga el semaoforo y ponerlo en el ready indicado
            
            proceso_kernel* procesoQueTendraElsemaforo = list_remove(semaforoRetenido->listaDeProcesosEnEspera, 0);
            //p3->listaRecursosRetenidos= {s3,s4}
            procesoQueTendraElsemaforo->procesoApuntadoDeadlock = NULL;
            list_add(procesoQueTendraElsemaforo->listaRecursosRetenidos, semaforoRetenido);
            //y como el proceso estaba solicitando el semaforo actual solo, lo vamos a sacar porque ahora lo retiene
            list_remove(procesoQueTendraElsemaforo->listaRecursosSolicitados, 0);
            
            proceso_kernel* procesoActual = list_remove_by_condition(procesosSuspendedBlock, procesoConPid);                                                    

            if(procesoActual == NULL){ //si esta solo bloqueado, se aagrega directo se saca de blocked
                
                list_remove_by_condition(procesosBlocked, procesoConPid);

            }

            //el otro proceso entraria en ready, ya que no va a estar pidiendo mas recursos

            int proceso_para_liberar(proceso_kernel* p){
                return procesoQueTendraElsemaforo->pid == p->pid;
            }
            
            

            proceso_kernel* proceso_Actual_liberado = list_remove_by_condition(procesosSuspendedBlock, proceso_para_liberar);

                                                             

            if(proceso_Actual_liberado != NULL){ //si esta solo bloqueado, se aagrega directo se saca de blocked
                    
                list_add(procesosSuspendedReady, procesoQueTendraElsemaforo);
                

                sem_post(procesoNecesitaEntrarEnReady); //alertamos que hay un proceso que solicita entrar en ready
                
                //esto es un aviso para el planificador de mediano plazo de que hay un proceso nuevo y quiza deberia entrar si solo hay procesos de Bloqueados
                sem_post(signalSuspensionProceso);
            
            }else{ //si esta solo bloqueado, se aagrega directo a ready y lo sacamos de blocked

                
                list_remove_by_condition(procesosBlocked, proceso_para_liberar);
                
                list_add(procesosReady, procesoQueTendraElsemaforo);
                clock_gettime(CLOCK_REALTIME, &(procesoQueTendraElsemaforo->tiempoDeArriboColaReady)); //esto sirve para HRRN, para estimar cuando empezo un proceso a estar en ready y cuanto tiempo pasa ahi
                
                
                sem_post(hayProcesosReady); //alertamos que hay un proceso nuevo en ready

            }

        }

    }

}

