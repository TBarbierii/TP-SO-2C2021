#include "Conexiones.h"


void crear_marcos(){

    uint32_t cantidad_marcos = tamanio/tamanioPagina;

        for(uint32_t i=0; i<cantidad_marcos; i++){

            t_marco* marco = malloc(sizeof(t_marco));

            marco->id_marco = i;
            marco->proceso_asignado = -1;
            marco->estaLibre = true;
            marco->comienzo = i * tamanioPagina;

            list_add(marcos, marco);

        }

}

uint32_t generadorIdsPaginas(t_carpincho* carp){

	return carp->contadorPag++;
	
}

uint32_t generarDireccionLogica(uint32_t id, uint32_t desplazamiento){


	/*char* primerNumero = "1";

	char* id_char = string_itoa(id);
	uint32_t veces = 3 - string_length(id_char);


	char* ceros = string_repeat('0', veces);

	char* a = string_new();
	string_append(&a, primerNumero);
	string_append(&a, ceros);
	string_append(&a, id_char);


	char* DESPLAZAMIENTO =  string_itoa(desplazamiento);

	char* b = string_new();
		string_append(&b,a );
		string_append(&b, DESPLAZAMIENTO);

	uint32_t direccionLogica = atoi(b);

	free(a);
	free(DESPLAZAMIENTO);
	free(ceros);
	free(b);
	free(id_char);*/
	uint32_t direccionLogica = id * tamanioPagina + desplazamiento;
	return direccionLogica;
}

uint32_t obtenerId(uint32_t num){


	/*uint32_t id_retornado;

	char* DL = string_itoa(num);
	char* substring  = string_substring(DL, 1, 3);

	id_retornado = atoi(substring);

	free(DL);
	free(substring);*/

	uint32_t id_retornado = num/tamanioPagina;

	return id_retornado;

}

uint32_t obtenerDesplazamiento(uint32_t num){

		
		/*uint32_t id_retornado;

		char* DL = string_itoa(num);
		char* substring =  string_substring_from(DL, 4);

		if(strlen(DL)==0){
			return 0;
		}

		 id_retornado = atoi(substring);

		 free(DL);
		 free(substring);*/

		 uint32_t id_retornado = num%tamanioPagina;

		 return id_retornado;

}

uint32_t calcular_direccion_fisica(uint32_t carpincho, uint32_t direccionLogica){

	uint32_t direccionFisica;

	uint32_t id = obtenerId(direccionLogica);

	uint32_t desplazamiento = obtenerDesplazamiento(direccionLogica);

		bool buscarCarpincho(t_carpincho* s){
			return s->id_carpincho == carpincho;
		};
		pthread_mutex_lock(listaCarpinchos);
		t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);
		pthread_mutex_unlock(listaCarpinchos);

		bool buscarPagina(t_pagina* s){
			return s->id_pagina == id;
		};

		t_pagina* pagina = list_find(capybara->tabla_de_paginas,(void*)buscarPagina);

		direccionFisica = pagina->marco->comienzo + desplazamiento; 

	return direccionFisica;
}

uint32_t aumentarIdCarpinchos(){
	pthread_mutex_lock(controladorIds);
    id_carpincho++;
	pthread_mutex_unlock(controladorIds);

}