#include "Deadlock1.h"

int cantidadDeVecesQueProcesoRetieneASemaforo(proceso_kernel* procesoActual, semaforo* semaforoBuscado){
    
    int procesoRetieneSemaforo(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre, semaforoBuscado->nombre) == 0){
            return 1;
        }
        return 0;
    }
    return list_count_satisfying(procesoActual->listaRecursosRetenidos, procesoRetieneSemaforo);
}

int cantidadDeVecesQueProcesoPideASemaforo(proceso_kernel* procesoActual, semaforo* semaforoBuscado){
    
    int procesoRetieneSemaforo(semaforo* semaforoActual){
        if(strcmp(semaforoActual->nombre, semaforoBuscado->nombre) == 0){
            return 1;
        }
        return 0;
    }
    return list_count_satisfying(procesoActual->listaRecursosSolicitados, procesoRetieneSemaforo);
}



void ejecutarAlgoritmoDeadlock(){
    t_log* logger = log_create("cfg/Deadlock.log","Deadlock",1,LOG_LEVEL_INFO);
    while(1){
        sleep(20);
        pthread_mutex_lock(controladorSemaforos);
        bloquearTodosLosSemaforos();
        pthread_mutex_lock(modificarBlocked);
        pthread_mutex_lock(modificarSuspendedBlocked);
        pthread_mutex_lock(modificarExec);
        pthread_mutex_lock(modificarReady);
        pthread_mutex_lock(modificarSuspendedReady);

        log_info(logger,"Se ejecuta algoritmo de deteccion y recuperacion de deadlock");


        while(1){
            int cantidadSemaforos = list_size(semaforosActuales);
            t_list* procesosPosiblesEnDeadlock = procesosQueEstanReteniendoYEsperando();
            int cantidadProcesos = list_size(procesosPosiblesEnDeadlock);
            if(cantidadDeProcesosActual <= 1){
                log_info(logger,"No puede haber deadlock ya que solo hay 1 o 0 procesos reteniendo y esperando por semaforos");
                break;
            }
            //sacamos todas las matrices y vectores para realizar el algoritmo
            int disponibilidad[cantidadSemaforos];
            int matrizRecursosRetenidos[cantidadDeProcesosActual][cantidadSemaforos];
            int matrizRecursosPeticiones[cantidadDeProcesosActual][cantidadSemaforos];

            

            rellenarVectorDisponibles(semaforosActuales, disponibilidad);
            
            
            //rellenamos las dos matrices
            for(int i = 0; i < cantidadProcesos; i++){

                for(int j= 0; j < cantidadSemaforos; j++){
                    proceso_kernel* procesoActual = list_get(procesosPosiblesEnDeadlock, i);
                    semaforo* semaforoActual = list_get(semaforosActuales, j);
                    matrizRecursosRetenidos[i][j] = cantidadDeVecesQueProcesoRetieneASemaforo(procesoActual,semaforoActual);
                }
            }
            for(int i = 0; i < cantidadProcesos; i++){

                for(int j= 0; j < cantidadSemaforos; j++){
                    proceso_kernel* procesoActual = list_get(procesosPosiblesEnDeadlock, i);
                    semaforo* semaforoActual = list_get(semaforosActuales, j);
                    matrizRecursosRetenidos[i][j] = cantidadDeVecesQueProcesoPideASemaforo(procesoActual,semaforoActual);
                }
            }

            //ahora creamos los vectores de WORK y FINISH
            int finish[cantidadDeProcesosActual];
            int work[cantidadSemaforos];
            
            for(int i= 0; i< cantidadSemaforos; i ++){
                work[i]=disponibilidad[i];
            }
            //como todos los procesos que filtramos estan reteniendo algo, todos van a comenzar con Finish[i] == false
            for(int i= 0; i< cantidadDeProcesosActual; i ++){
                finish[i]=0;
            }


            //ahora ejecutamos la secuencia de comparacion
            for(int i= 0 ; i< cantidadDeProcesosActual; i ++){
                
                int cumple = 1;
                
                for(int j=0; j< cantidadSemaforos; j ++){
                    
                    if(matrizRecursosRetenidos[i][j] > disponibilidad[j]){
                    //con que se pida mas en alguno de los semaforos, ya no cumple
                    cumple = 0;
                    }

                }

                if(finish[i] == 0  && cumple){
                    finish[i]=1;
                    for(int j=0; j< cantidadSemaforos; j ++){
                       work[j] += matrizRecursosRetenidos[i][j];         
                    }

                }
            }

            t_list* procesosEnDeadlock = list_create();
            for(int j=0; j< cantidadDeProcesosActual; j ++){
                    
                if(finish[j] == 0){
                    proceso_kernel* procesoEnElDeadlock = list_get(procesosPosiblesEnDeadlock, j);
                    list_add(procesosEnDeadlock, procesoEnElDeadlock);
                }

            }
            if(list_is_empty(procesosEnDeadlock)){
                log_info(logger,"No hay deadlock, dejamos que se ejecute todo normal");
                break;
            }
            while(!list_is_empty(procesosEnDeadlock)){
                proceso_kernel* proceso= list_remove(procesosEnDeadlock,0);
                log_info(logger,"El proceso: %d, puede ser que genere un posible deadlock", proceso->pid);
            }
            break;

        }
        

        pthread_mutex_unlock(modificarSuspendedReady);
        pthread_mutex_unlock(modificarReady);
        pthread_mutex_unlock(modificarExec);
        pthread_mutex_unlock(modificarSuspendedBlocked);
        pthread_mutex_unlock(modificarBlocked);
        desbloquearTodosLosSemaforos();
        pthread_mutex_unlock(controladorSemaforos);
    } 

    log_destroy(logger);

}


void bloquearTodosLosSemaforos(){

    int offset = 0;
    int size = list_size(semaforosActuales);

    while(offset < size){
        semaforo* semaforoActual  = list_get(semaforosActuales, offset);
        pthread_mutex_lock(semaforoActual->mutex);
        offset++;
    }
}

void desbloquearTodosLosSemaforos(){

    int offset = 0;
    int size = list_size(semaforosActuales);

    while(offset < size){
        semaforo* semaforoActual  = list_get(semaforosActuales, offset);
        pthread_mutex_unlock(semaforoActual->mutex);
        offset++;
    }
}

int procesoReteniendoProcesosYEsperando(proceso_kernel* proceso){
    return (!list_is_empty(proceso->listaRecursosRetenidos)) && (!list_is_empty(proceso->listaRecursosSolicitados));
}

t_list* procesosQueEstanReteniendoYEsperando(){

    t_list* listaFiltrada = list_create();
    //primero filtramos los que estan bloqueados
    list_add_all(listaFiltrada, procesosBlocked);
    list_add_all(listaFiltrada, procesosSuspendedBlock);

    t_list* listaFiltradaFinal = list_filter(listaFiltrada, procesoReteniendoProcesosYEsperando);

    return listaFiltradaFinal;

}





void rellenarVectorDisponibles(t_list* listaSemaforos, int vector[]){

    int size = list_size(listaSemaforos);

    for(int i = 0; i < size; i++){

        semaforo* semaforoActual = list_get(listaSemaforos, i);

        if(semaforoActual->valor < 0){
            vector[i]= 0;
        }else{
            vector[i]= semaforoActual->valor;
        }
    }


}





