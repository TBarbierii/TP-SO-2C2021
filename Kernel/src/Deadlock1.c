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

        usleep(tiempoDeadlock*1000);
        pthread_mutex_lock(controladorSemaforos);
        bloquearTodosLosSemaforos();
        pthread_mutex_lock(modificarBlocked);
        pthread_mutex_lock(modificarSuspendedBlocked);
        pthread_mutex_lock(modificarExec);
        pthread_mutex_lock(modificarReady);
        pthread_mutex_lock(modificarSuspendedReady);

        log_info(logger,"Se ejecuta algoritmo de deteccion y recuperacion de deadlock");

        int cantidadSemaforos = list_size(semaforosActuales);
        int disponibilidad[cantidadSemaforos];
        t_list* listaFinalADesalojar = list_create();
        rellenarVectorDisponibles(semaforosActuales, disponibilidad);

        while(1){
            
            t_list* procesosPosiblesEnDeadlock = procesosQueEstanReteniendoYEsperando(logger);
            int cantidadProcesos = list_size(procesosPosiblesEnDeadlock);
            log_info(logger,"La cantidad de procesos en posible deadlock son: %d",cantidadProcesos);
             
            if(cantidadProcesos <= 1){
                log_info(logger,"No puede haber deadlock ya que solo hay 1 o 0 procesos reteniendo y esperando por semaforos\n");
                list_destroy(procesosPosiblesEnDeadlock);
                break;
            }

            //sacamos todas las matrices y vectores para realizar el algoritmo
            
            int matrizRecursosRetenidos[cantidadProcesos][cantidadSemaforos];
            int matrizRecursosPeticiones[cantidadProcesos][cantidadSemaforos];
            
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
                    matrizRecursosPeticiones[i][j] = cantidadDeVecesQueProcesoPideASemaforo(procesoActual,semaforoActual);
                }
            }


            //ahora creamos los vectores de WORK y FINISH
            int finish[cantidadProcesos];
            int work[cantidadSemaforos];
            
            for(int i= 0; i< cantidadSemaforos; i ++){
                work[i]=disponibilidad[i];
            }
            //como todos los procesos que filtramos estan reteniendo algo, todos van a comenzar con Finish[i] == false
            for(int i= 0; i< cantidadProcesos; i ++){
                finish[i]=0;
            }


            //ahora ejecutamos la secuencia de comparacion, como podria pasar que los procesos vengan en distinto orden,
            //vamos a ahcer un while(1), donde se va a terminar cuando haga el for completo y nadie entre en el if

            while(1){
                
                int existeProcesoQueCumplaCondicion = 0;

                //aca vemos si la cantidad de recursos solicitados es menor a la disponible
                for(int i= 0 ; i< cantidadProcesos; i ++){
                    
                    int cumple = 1;
                    
                    for(int j=0; j< cantidadSemaforos; j ++){
                        
                        if(matrizRecursosPeticiones[i][j] > work[j]){
                        //con que se pida mas en alguno de los semaforos, ya no cumple
                        
                        cumple = 0;
                        }

                    }
                    //se analiza si el proceso cumple ambas condiciones
                    if(finish[i] == 0  && cumple){
                        
                        finish[i]=1;
                        existeProcesoQueCumplaCondicion = 1;

                        for(int h=0; h< cantidadSemaforos; h ++){
                        work[h] += matrizRecursosRetenidos[i][h];         
                        }

                    }
                }

                //si no existe ninguno que entro al if, ninguno cumple las condiciones, entonces el work nunca aumento, entonces decido terminar el while
                if(existeProcesoQueCumplaCondicion == 0){
                    break;
                }

            }


            //creo la lista de lo que analize y estan en deadlock
            t_list* procesosEnDeadlock = list_create();
            for(int j=0; j< cantidadProcesos; j ++){
                    
                if(finish[j] == 0){
                    proceso_kernel* procesoEnElDeadlock = list_get(procesosPosiblesEnDeadlock, j);
                    list_add(procesosEnDeadlock, procesoEnElDeadlock);
                }

            }
            if(list_size(procesosEnDeadlock)<=1){
                log_info(logger,"No hay deadlock, dejamos que se ejecute todo normal\n");
                break;
            }

            
            //ordenamos la lista para poner el de mayor id primero
            list_sort(procesosEnDeadlock,procesoConMayorPID);
            proceso_kernel* procesoASacarPorDeadlock = list_get(procesosEnDeadlock, 0);
            log_info(logger,"El proceso elegido para sacar del deadlock sera el proceso: %d\n", procesoASacarPorDeadlock->pid);

            //primero lo sacamos de bloqueado, onda va a ser uno de los bloqueados si o si, ya que tendria que estar reteniendo y pidiendo(que es por lo cual quedo bloqueado)
            sacarProcesoDeBloqueado(procesoASacarPorDeadlock->pid);

            //le sacamos todos los recursos, y despues de que no haya mas deadlock vamos a sacarle los recursos de manera real y hacer los signal correspondientes
            int indice = indiceDondeProcesoEstaEnLaLista(procesoASacarPorDeadlock->pid, procesosPosiblesEnDeadlock);
            for(int i=0; i< cantidadSemaforos; i++){
                //los recursos que tenia retenidos, se los damos como disponible ahora al vector de disponibilidad
                disponibilidad[i] += matrizRecursosRetenidos[indice][i];
            }

            list_add(listaFinalADesalojar, procesoASacarPorDeadlock);
            list_destroy(procesosPosiblesEnDeadlock);
            list_destroy(procesosEnDeadlock);

        }

        //luego de toda la simulacion y de agregar a todos a la lista de los considerados de deadlock, les eliminamos de en serio los recursos
        while(!list_is_empty(listaFinalADesalojar)){
            proceso_kernel* procesoASacarPorDeadlock = list_remove(listaFinalADesalojar, 0);
            desalojarSemaforosDeProceso(procesoASacarPorDeadlock);
            finalizarProcesoPorDeadlock(procesoASacarPorDeadlock);
        }
        
        list_destroy(listaFinalADesalojar);

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

int procesoConMayorPID(proceso_kernel* p1, proceso_kernel* p2){
    return p1->pid > p2->pid;
}

int indiceDondeProcesoEstaEnLaLista(int pid, t_list* lista){
   int noEsta = -1;
   int indice = 0;


   int esElProcesoBuscado(proceso_kernel* procesoBuscado){
       if(procesoBuscado->pid == pid){
           return 1;
       }else{
           indice++;
           return 0;
       }
   }

    proceso_kernel* procesoBuscado = list_find(lista, esElProcesoBuscado);
    if(procesoBuscado != NULL){
        return indice;
    }else{
        return noEsta;
    }
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

int procesoReteniendoYEsperando(proceso_kernel* proceso){
    return (!list_is_empty(proceso->listaRecursosRetenidos)) && (!list_is_empty(proceso->listaRecursosSolicitados));
}

int procesoReteniendo(proceso_kernel* proceso){
    return (!list_is_empty(proceso->listaRecursosRetenidos));
}

t_list* procesosQueEstanReteniendoYEsperando(t_log* loggerActual){

    t_list* listaFiltrada = list_create();
    t_list* procesosQuePuedenEstarOcupandoRecursos = list_create();
    t_list* listaFiltradaFinal = list_create();

    //siempre vamos a considerar los que estan reteniendo, xq si no esta reteniendo no es causante del deadlock

    //primero filtramos los que estan bloqueados, los que tienen cosas asignadas y estan reteniendo
    list_add_all(listaFiltrada, procesosBlocked);
    list_add_all(listaFiltrada, procesosSuspendedBlock);


    if(!list_is_empty(listaFiltrada)){
        listaFiltradaFinal = list_filter(listaFiltrada, procesoReteniendoYEsperando);
    }

    int sizeBloqueados = list_size(listaFiltradaFinal);
    log_info(loggerActual,"La cantidad de procesos agarrados para el deadlock que se encuentran bloqueados, reteniendo y esperando son: %d", sizeBloqueados);



    //ahora filtramos los que puede ser que esten reteniendo y no pidiendo nada, ya que estos me van a liberar recursos
    
    list_add_all(procesosQuePuedenEstarOcupandoRecursos, procesosExec);
    list_add_all(procesosQuePuedenEstarOcupandoRecursos, procesosReady);
    list_add_all(procesosQuePuedenEstarOcupandoRecursos, procesosSuspendedReady);
    
    //int sizeNoBloqueados = list_size(procesosQuePuedenEstarOcupandoRecursos);
    //log_info(loggerActual,"La cantidad de procesos agarrados para el deadlock que no se encuentran bloqueados son: %d", sizeNoBloqueados);

    if(!list_is_empty(procesosQuePuedenEstarOcupandoRecursos)){
        t_list* listaFiltradaFinal2 = list_filter(procesosQuePuedenEstarOcupandoRecursos, procesoReteniendo);

    int sizeReteniendo = list_size(procesosQuePuedenEstarOcupandoRecursos);
    log_info(loggerActual,"La cantidad de procesos agarrados para el deadlock que no se encuentran bloqueados, pero estan reteniendo son: %d", sizeReteniendo);
        
        if(!list_is_empty(listaFiltradaFinal2)){
            list_add_all(listaFiltradaFinal, listaFiltradaFinal2);
        }
        list_destroy(listaFiltradaFinal2);
    }

    list_destroy(procesosQuePuedenEstarOcupandoRecursos);

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


void finalizarProcesoPorDeadlock(proceso_kernel* procesoASacarPorDeadlock){
    //el 0 era que se realizo el wait
    //el 1 que no se pudo realizar xq no existia el semaforo
    //el 2 que se pudo realizar aunque no se bloqueo
    //vamos a enviarle un codigo 3 de respuesta a la matelib, y la matelib va a cerrar todo debido a eso
    avisarWaitDeSemaforo(procesoASacarPorDeadlock->conexion, 3);
    //ya anteriormente lo liberamos de todos lados

    //liberamos los recursos del proceso
    liberarProceso(procesoASacarPorDeadlock);
}


