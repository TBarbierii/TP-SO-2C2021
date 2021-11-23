#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <commons/log.h>
#include <matelib.h>
#include <string.h>

#define SEMAFORO_SALUDO "SEM_HELLO"

int main(int argc, char *argv[]) {

    if(argc < 2){
        printf("No se ingresó archivo de configuración");
        exit(EXIT_FAILURE);
    }

    char* config = argv[1];

	printf("MAIN - Utilizando el archivo de config: %s\n", config);

	mate_instance instance;

	mate_init(&instance, (char*)config);

    char* saludo = "¡Hola mundo!\n";

    mate_pointer saludoRef = mate_memalloc(&instance, strlen(saludo));

    mate_memwrite(&instance, saludo, saludoRef, strlen(saludo));

	char* contenidoAmostrar = malloc(strlen(saludo)+1);

    mate_memread(&instance, saludoRef, saludo , strlen(saludo));

	printf(contenidoAmostrar);
	free(contenidoAmostrar);

	printf("\n%s",saludo);

    mate_sem_post(&instance, SEMAFORO_SALUDO);

    mate_close(&instance);

	return EXIT_SUCCESS;
}

//EJEMPLO NUESTRO
/* int main(){

    mate_instance* referencia = malloc(sizeof(mate_instance)); //porque rompe si hacemos el malloc en el mate_init?

    mate_init(referencia, "/home/utnso/tp-2021-2c-UCM-20-SO/MateLib/cfg/configProcesos.config");
    
    //mate_sem_init(referencia,"SEM1",1);
    //mate_sem_post(referencia,"SEM1");
    //mate_sem_wait(referencia,"SEM1");
    //mate_sem_wait(referencia,"SEM1");
    //mate_call_io(referencia,"laguna","asd");
    mate_pointer mate = mate_memalloc(referencia, 45);
    //mate_memfree(referencia, mate);
    //mate_close(referencia);
    //free(referencia);
    
    void* lectura = malloc(45);
    printf("\nDireccion %i\n", mate);

    if(mate != 0){

    mate_memwrite(referencia, "----------------------------------------1-45", mate, 45);
    mate_memread(referencia, mate, lectura,45);
    log_info(referencia->group_info->loggerProceso, "\n LLego: %s", (char*)lectura);
    }

	free(lectura);
	
	mate_close(referencia);
    free(referencia);
    
    //pthread_t h1, h2, h3;

    //pthread_create(&h1, NULL, (void*)hilo1,NULL);  
    //pthread_create(&h2, NULL, (void*)hilo2,NULL);
    //pthread_create(&h3, NULL, (void*)hilo3,NULL);  

    //pthread_join(h1, NULL);
    //pthread_join(h2, NULL);
    //pthread_join(h3, NULL);

    
    return 0;
} */


//DEADLOCK


/*
void* carpincho1_func(void* config){

    mate_instance instance;

    mate_init(&instance, config);

    printf("C1 - Toma SEM1\n");
    mate_sem_wait(&instance, "SEM1");
    sleep(3);
    printf("C1 - Toma SEM2\n");
    mate_sem_wait(&instance, "SEM2");
    sleep(3);

    printf("C1 - libera SEM1\n");
    mate_sem_post(&instance, "SEM1");
    printf("C1 - libera SEM2\n");
    mate_sem_post(&instance, "SEM2");

    printf("C2 - Se retira a descansar\n");
    mate_close(&instance);
    return 0;
}

void* carpincho2_func(void* config){

    mate_instance instance;

    mate_init(&instance, config);

    printf("C2 - toma SEM2\n");
    mate_sem_wait(&instance, "SEM2");
    sleep(3);
    printf("C2 - toma SEM3\n");
    mate_sem_wait(&instance, "SEM3");
    sleep(3);

    printf("C2 - libera SEM2\n");
    mate_sem_post(&instance, "SEM2");
    printf("C2 - libera SEM3\n");
    mate_sem_post(&instance, "SEM3");

	printf("C2 - Se retira a descansar\n");
	mate_close(&instance);
	return 0;
}

void* carpincho3_func(void* config){

    mate_instance instance;

    mate_init(&instance, config);

    printf("C3 - toma SEM3\n");
    mate_sem_wait(&instance, "SEM3");
    sleep(3);
    printf("C3 - toma SEM4\n");
    mate_sem_wait(&instance, "SEM4");
    sleep(3);

    printf("C3 - libera SEM3\n");
    mate_sem_post(&instance, "SEM3");
    printf("C3 - libera SEM4\n");
    mate_sem_post(&instance, "SEM4");

	printf("C3 - Se retira a descansar\n");
	mate_close(&instance);
	return 0;
}

void* carpincho4_func(void* config){

    mate_instance instance;

    mate_init(&instance, config);

    printf("C4 - toma SEM4\n");
    mate_sem_wait(&instance, "SEM4");
    sleep(3);
    printf("C4 - toma SEM1\n");
    mate_sem_wait(&instance, "SEM1");
    sleep(3);

    printf("C4 - libera SEM1\n");
    mate_sem_post(&instance, "SEM1");
    printf("C4 - libera SEM4\n");
    mate_sem_post(&instance, "SEM4");

	printf("C4 - Se retira a descansar\n");
	mate_close(&instance);
	return 0;
}

void* carpincho5_func(void* config){

    mate_instance instance;

    mate_init(&instance, config);

    printf("C5 - toma SEM5\n");
    mate_sem_wait(&instance, "SEM5");
    sleep(3);
    printf("C5 - toma SEM6\n");
    mate_sem_wait(&instance, "SEM6");
    sleep(3);

    printf("C5 - toma SEM5\n");
    mate_sem_post(&instance, "SEM5");
    printf("C5 - toma SEM6\n");
    mate_sem_post(&instance, "SEM6");

	printf("C5 - Se retira a descansar\n");
	mate_close(&instance);
	return 0;
}

void* carpincho6_func(void* config){

    mate_instance instance;

    mate_init(&instance, config);

    printf("C6 - toma SEM6\n");
    mate_sem_wait(&instance, "SEM6");
    sleep(3);
    printf("C6 - toma SEM1\n");
    mate_sem_wait(&instance, "SEM1");
    sleep(3);
    printf("C6 - Libera SEM1\n");
    mate_sem_post(&instance, "SEM1");

    printf("C6 - toma SEM5\n");
    mate_sem_wait(&instance, "SEM5");
    sleep(3);

    mate_sem_post(&instance, "SEM5");
    mate_sem_post(&instance, "SEM6");

	printf("C6 - Se retira a descansar\n");
	mate_close(&instance);
	return 0;
}


int main(int argc, char *argv[]) {

    mate_instance instance;

    mate_init(&instance, argv[1]);

  // Creamos los semaforos que van a usar los carpinchos
    mate_sem_init(&instance, "SEM1", 1);
    mate_sem_init(&instance, "SEM2", 1);
    mate_sem_init(&instance, "SEM3", 1);
    mate_sem_init(&instance, "SEM4", 1);
    mate_sem_init(&instance, "SEM5", 1);
    mate_sem_init(&instance, "SEM6", 1);

    mate_close(&instance);

  // Deadlock entre estos 4
	pthread_t carpincho1;
	pthread_t carpincho2;
	pthread_t carpincho3;
	pthread_t carpincho4;

  // Deadlock entre estos 2 con uno pendiente del anterior
	pthread_t carpincho5;
	pthread_t carpincho6;


	printf("MAIN - Utilizando el archivo de config: %s\n", argv[1]);

	pthread_create(&carpincho1, NULL, carpincho1_func, argv[1]);
    sleep(1);
	pthread_create(&carpincho2, NULL, carpincho2_func, argv[1]);
    sleep(1);
	pthread_create(&carpincho3, NULL, carpincho3_func, argv[1]);
    sleep(1);
	pthread_create(&carpincho4, NULL, carpincho4_func, argv[1]);
    sleep(1);
	pthread_create(&carpincho5, NULL, carpincho5_func, argv[1]);
    sleep(1);
	pthread_create(&carpincho6, NULL, carpincho6_func, argv[1]);
    sleep(1);

    

	pthread_join(carpincho6, NULL);
	pthread_join(carpincho5, NULL);
	pthread_join(carpincho4, NULL);
	pthread_join(carpincho3, NULL);
	pthread_join(carpincho2, NULL);
	pthread_join(carpincho1, NULL);
  
	printf("MAIN - Como no sabemos a quienes va a matar el algoritmo, entonces hacemos el free de los semáforos acá");
	mate_init(&instance, argv[1]);
    mate_sem_destroy(&instance, "SEM1");
    mate_sem_destroy(&instance, "SEM2");
    mate_sem_destroy(&instance, "SEM3");
    mate_sem_destroy(&instance, "SEM4");
    mate_sem_destroy(&instance, "SEM5");
    mate_sem_destroy(&instance, "SEM6");
	mate_close(&instance);

	printf("MAIN - Retirados los carpinchos de la pelea, hora de analizar los hechos\n");

	return EXIT_SUCCESS;
} */





//SJF Y HRRN

/* 
char *LOG_PATH = "./planificacion.log";
char *PROGRAM_NAME = "planificacion";
sem_t *va_el_2;
sem_t *va_el_3;
t_log *logger;

void imprimir_carpincho_n_hace_algo(int numero_de_carpincho)
{
    log_info(logger, "EJECUTANDO Carpincho %d", numero_de_carpincho);
    sleep(3);
}

void exec_carpincho_1(char *config)
{
    mate_instance self;
    mate_init(&self, config);
    for (int i = 0; i < 3; i++)
    {
        imprimir_carpincho_n_hace_algo(1);
        mate_call_io(&self, (mate_io_resource) "pelopincho", "Carpincho 1 se va a IO");
    }
    imprimir_carpincho_n_hace_algo(1);
    mate_close(&self);
}

void exec_carpincho_2(char *config)
{
    
    mate_instance self;
    mate_init(&self, config);
    sem_post(va_el_3); //Creo que esta demas, es para que el 3 entre dsp del 2
    for (int i = 0; i < 3; i++)
    {
        imprimir_carpincho_n_hace_algo(2);
        mate_call_io(&self, (mate_io_resource) "pelopincho", "Carpincho 2 se va a IO");
    }
    imprimir_carpincho_n_hace_algo(2);
    mate_close(&self);
}

void exec_carpincho_3(char *config)
{   
    
    mate_instance self;
    sem_wait(va_el_3);
    mate_init(&self, config);
    for (int i = 0; i < 3; i++)
    {
        imprimir_carpincho_n_hace_algo(3);
        imprimir_carpincho_n_hace_algo(3);
        imprimir_carpincho_n_hace_algo(3);
        imprimir_carpincho_n_hace_algo(3);
        imprimir_carpincho_n_hace_algo(3);
        mate_call_io(&self, (mate_io_resource) "pelopincho", "Carpincho 3 se va a IO");
    }
    imprimir_carpincho_n_hace_algo(3);
    imprimir_carpincho_n_hace_algo(3);
    imprimir_carpincho_n_hace_algo(3);
    imprimir_carpincho_n_hace_algo(3);
    imprimir_carpincho_n_hace_algo(3);
    mate_close(&self);
}

void free_all()
{
    sem_destroy(va_el_3);
    free(va_el_3);
    sem_destroy(va_el_2);
    free(va_el_2);

    log_destroy(logger);
}

void init_sems()
{
    va_el_2 = malloc(sizeof(sem_t));
    sem_init(va_el_2, 1, 0);
    va_el_3 = malloc(sizeof(sem_t));
    sem_init(va_el_3, 1, 0);
}

int main(int argc, char *argv[])
{
    logger = log_create(LOG_PATH, PROGRAM_NAME, true, LOG_LEVEL_DEBUG);
    pthread_t carpincho1_thread;
    pthread_t carpincho2_thread;
    pthread_t carpincho3_thread;

    init_sems();

    pthread_create(&carpincho1_thread, NULL, (void *)exec_carpincho_1, argv[1]);
    pthread_create(&carpincho2_thread, NULL, (void *)exec_carpincho_2, argv[1]);
    pthread_create(&carpincho3_thread, NULL, (void *)exec_carpincho_3, argv[1]);
    pthread_join(carpincho1_thread, NULL);
    pthread_join(carpincho2_thread, NULL);
    pthread_join(carpincho3_thread, NULL);
    free_all();
    puts("Termine!");
}
 */






//SUSPENSION
/*
void* carpincho1_func(void* config){

	mate_instance instance;

	printf("C1 - Llamo a mate_init\n");
	mate_init(&instance, (char*)config);

	printf("C1 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C1 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C1 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

    mate_close(&instance);

	return 0;
}

void* carpincho2_func(void* config){

	mate_instance instance;

	printf("C2 - Llamo a mate_init\n");
	mate_init(&instance, (char*)config);

	printf("C2 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C2 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C2 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

    mate_close(&instance);

	return 0;
}

void* carpincho3_func(void* config){

	mate_instance instance;

	printf("C3 - Llamo a mate_init\n");
	mate_init(&instance, (char*)config);

	printf("C3 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C3 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C3 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

    mate_close(&instance);

	return 0;
}

void* carpincho4_func(void* config){

	mate_instance instance;

	printf("C4 - Llamo a mate_init\n");
	mate_init(&instance, (char*)config);

	printf("C4 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C4 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

	printf("C4 - Hace una llamada a IO\n");
	mate_call_io(&instance, "PILETA", "Vamos a usar la pileta...");

    mate_close(&instance);
	return 0;
}



int main(int argc, char *argv[]) {

	pthread_t carpincho1;
	pthread_t carpincho2;
	pthread_t carpincho3;
	pthread_t carpincho4;

	printf("MAIN - Utilizando el archivo de config: %s\n", argv[1]);

	pthread_create(&carpincho1, NULL, carpincho1_func, argv[1]);
	sleep(1);
	pthread_create(&carpincho2, NULL, carpincho2_func, argv[1]);
	sleep(1);
	pthread_create(&carpincho3, NULL, carpincho3_func, argv[1]);
	sleep(1);
	pthread_create(&carpincho3, NULL, carpincho4_func, argv[1]);

	pthread_join(carpincho4, NULL);
	pthread_join(carpincho3, NULL);
	pthread_join(carpincho2, NULL);
	pthread_join(carpincho1, NULL);

	printf("MAIN - Retirados los carpinchos de la pelea, hora de analizar los hechos\n");

	return 0;
} */

//REEMPLAZO MMU

/* 
sem_t semCarpincho1;
sem_t semCarpincho2;
sem_t semCarpincho3;

void* carpincho1_func(void* config){

	sem_wait(&semCarpincho1);
	mate_instance instanceC1;

	printf("C1 - Llamo a mate_init\n");
	mate_init(&instanceC1, (char*)config);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc0 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc1 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc2 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 119 bytes\n");
	mate_pointer alloc3 = mate_memalloc(&instanceC1, 119);

	printf("C1 - Escribo en la página 0\n");
	mate_memwrite(&instanceC1, "Hola", alloc0, 5);

	printf("C1 - Escribo en la página 1\n");
	mate_memwrite(&instanceC1, "Hola", alloc1, 5);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Escribo en la página 2\n");
	mate_memwrite(&instanceC1, "Chau", alloc2, 5);

	printf("C1 - Escribo en las páginas 3, 4, 5 y 6\n");
	mate_memwrite(&instanceC1, "Animalia Chordata Mammalia Rodentia Hystricognathi Caviidae Hydrochoerus hydrochaeris, aka Carpincho", alloc3, 100);

	printf("C1 - Reservo un alloc de 53 bytes\n");
	mate_pointer alloc4 = mate_memalloc(&instanceC1, 53);

	printf("C1 - Libero al C2 para que finalice\n");
	sem_post(&semCarpincho2);
	
	printf("C1 - Libero al C3 para que finalice\n");
	sem_post(&semCarpincho3);

	printf("C1 - Se retira a descansar\n");
	mate_close(&instanceC1);

	return 0;
}

void* carpincho2_func(void* config){

	sem_wait(&semCarpincho2);
	mate_instance instanceC2;

	printf("C2 - Llamo a mate_init\n");
	mate_init(&instanceC2, (char*)config);

	printf("C2 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc0 = mate_memalloc(&instanceC2, 23);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc1 = mate_memalloc(&instanceC2, 23);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc2 = mate_memalloc(&instanceC2, 23);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	void* localMalloc = malloc(5);

	printf("C2 - LEO de la página 0\n");
	mate_memread(&instanceC2, alloc0, localMalloc , 5);

	printf("C2 - LEO de la página 1\n");
	mate_memread(&instanceC2, alloc1, localMalloc, 5);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Se retira a descansar\n");
	mate_close(&instanceC2);

	return 0;
}

void* carpincho3_func(void* config){

	sem_wait(&semCarpincho3);
	mate_instance instanceC3;

	printf("C3 - Llamo a mate_init\n");
	mate_init(&instanceC3, (char*)config);

	printf("C3 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc0 = mate_memalloc(&instanceC3, 23);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc1 = mate_memalloc(&instanceC3, 23);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc2 = mate_memalloc(&instanceC3, 23);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	void* localMalloc = malloc(5);

	printf("C3 - LEO de la página 0\n");
	mate_memread(&instanceC3, alloc0, localMalloc , 5);

	printf("C3 - LEO de la página 1\n");
	mate_memread(&instanceC3, alloc1, localMalloc, 5);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Se retira a descansar\n");
	mate_close(&instanceC3);

	return 0;
}



int main(int argc, char *argv[]) {

	pthread_t carpincho1;
	pthread_t carpincho2;
	pthread_t carpincho3;

	sem_init(&semCarpincho1, 0, 1);
	sem_init(&semCarpincho2, 0, 0);
	sem_init(&semCarpincho3, 0, 0);

	printf("MAIN - Utilizando el archivo de config: %s\n", argv[1]);

	pthread_create(&carpincho1, NULL, carpincho1_func, argv[1]);
	pthread_create(&carpincho2, NULL, carpincho2_func, argv[1]);
	pthread_create(&carpincho3, NULL, carpincho3_func, argv[1]);

	pthread_join(carpincho3, NULL);
	pthread_join(carpincho2, NULL);
	pthread_join(carpincho1, NULL);

	printf("MAIN - Retirados los carpinchos de la pelea, hora de analizar los hechos\n");

	return EXIT_SUCCESS;
}*/

//PRUEBA ASIGNACION

/* sem_t semCarpincho1;
sem_t semCarpincho2;
sem_t semCarpincho3;

void* carpincho1_func(void* config){

	sem_wait(&semCarpincho1);
	mate_instance instanceC1;

	printf("C1 - Llamo a mate_init\n");
	mate_init(&instanceC1, (char*)config);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc0 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc1 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc2 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 10 bytes\n");
	mate_pointer alloc3 = mate_memalloc(&instanceC1, 10);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Escribo en la página 3\n");
	mate_memwrite(&instanceC1, "Hola", alloc3, 5);

	printf("C1 - Escribo en la página 2\n");
	mate_memwrite(&instanceC1, "Hola", alloc2, 5);

	printf("C1 - Escribo en la página 1\n");
	mate_memwrite(&instanceC1, "Hola", alloc1, 5);

	printf("C1 - Escribo en la página 0\n");
	mate_memwrite(&instanceC1, "Hola", alloc0, 5);

	printf("C1 - Libero al C2\n");
	sem_post(&semCarpincho2);

	printf("C1 - Freno a C1\n");
	sem_wait(&semCarpincho1);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc4 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc5 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc6 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc7 = mate_memalloc(&instanceC1, 23);

	printf("C1 - Libero al C2 para que finalice\n");
	sem_post(&semCarpincho2);

	printf("C1 - Se retira a descansar\n");
	mate_close(&instanceC1);

	return 0;
}

void* carpincho2_func(void* config){

	sem_wait(&semCarpincho2);
	mate_instance instanceC2;

	printf("C2 - Llamo a mate_init\n");
	mate_init(&instanceC2, (char*)config);

	printf("C2 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc0 = mate_memalloc(&instanceC2, 23);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc1 = mate_memalloc(&instanceC2, 23);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc2 = mate_memalloc(&instanceC2, 23);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Reservo un alloc de 10 bytes\n");
	mate_pointer alloc3 = mate_memalloc(&instanceC2, 10);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Escribo en la página 0\n");
	mate_memwrite(&instanceC2, "Hola", alloc0, 5);

	printf("C2 - Libero al C3\n");
	sem_post(&semCarpincho3);

	printf("C2 - Freno a C2\n");
	sem_wait(&semCarpincho2);

	printf("C2 - Libero al C3 para que finalice\n");
	sem_post(&semCarpincho3);

	printf("C2 - Se retira a descansar\n");
	mate_close(&instanceC2);

	return 0;
}

void* carpincho3_func(void* config){

	sem_wait(&semCarpincho3);
	mate_instance instanceC3;

	printf("C3 - Llamo a mate_init\n");
	mate_init(&instanceC3, (char*)config);

	printf("C3 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc0 = mate_memalloc(&instanceC3, 23);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc1 = mate_memalloc(&instanceC3, 23);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Reservo un alloc de 23 bytes\n");
	mate_pointer alloc2 = mate_memalloc(&instanceC3, 23);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Reservo un alloc de 10 bytes\n");
	mate_pointer alloc3 = mate_memalloc(&instanceC3, 10);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Escribo en la página 0\n");
	mate_memwrite(&instanceC3, "Hola", alloc0, 5);

	printf("C3 - Libero al C1\n");
	sem_post(&semCarpincho1);

	printf("C3 - Freno a C3\n");
	sem_wait(&semCarpincho3);

	printf("C3 - Se retira a descansar\n");
	mate_close(&instanceC3);

	return 0;
}



int main(int argc, char *argv[]) {

	pthread_t carpincho1;
	pthread_t carpincho2;
	pthread_t carpincho3;

	sem_init(&semCarpincho1, 0, 1);
	sem_init(&semCarpincho2, 0, 0);
	sem_init(&semCarpincho3, 0, 0);

	printf("MAIN - Utilizando el archivo de config: %s\n", argv[1]);

	pthread_create(&carpincho1, NULL, carpincho1_func, argv[1]);
	pthread_create(&carpincho2, NULL, carpincho2_func, argv[1]);
	pthread_create(&carpincho3, NULL, carpincho3_func, argv[1]);

	pthread_join(carpincho3, NULL);
	pthread_join(carpincho2, NULL);
	pthread_join(carpincho1, NULL);

	printf("MAIN - Retirados los carpinchos de la pelea, hora de analizar los hechos\n");

	return EXIT_SUCCESS;
}
 */