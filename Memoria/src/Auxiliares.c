#include "Memoria.h"

uint32_t generadorIdsPaginas(){

    return id_pag++;

}

uint32_t dar_vuelta_id(uint32_t num){

	char* id = string_new();

	uint32_t id_retornado;
	id = string_itoa(num);
	id = string_reverse(id);
	uint32_t veces = 3 - string_length(id);

	char* ceros = string_repeat('0', veces);

	char* a = string_new();
	string_append(&a, id);
	string_append(&a, ceros);

	return id_retornado = atoi(a);

}

uint32_t generarDireccionLogica(uint32_t id, uint32_t desplazamiento){

	uint32_t ID;
	ID = dar_vuelta_id(id);

	char* ID_STRING = string_itoa(ID);
	char* DESPLAZAMIENTO =  string_itoa(desplazamiento);

	char* a = string_new();
		string_append(&a, ID_STRING);
		string_append(&a, DESPLAZAMIENTO);

	uint32_t direccionLogica = atoi(a);
	return direccionLogica;
}

uint32_t obtenerId(uint32_t num){


	char* id = string_new();

	uint32_t id_retornado;

	id = string_itoa(num);
	id  = string_substring(id, 0, 3);

	id = string_reverse(id);


	return id_retornado = atoi(id);

}

uint32_t calcular_direccion_fisica(uint32_t carpincho, uint32_t direccionLogica){

	uint32_t direccionFisica;

	uint32_t id = obtenerId(direccionLogica);

	uint32_t desplazamiento = obtenerDesplazamiento(direccionLogica);

		bool buscarCarpincho(t_carpincho* s){
			return s->id_carpincho == id;
		}

		t_carpincho* capybara = list_find(carpinchos,(void*)buscarCarpincho);

		bool buscarpagina(t_pagina* s){
			return s->id_pagina == id;
		}

		t_pagina* pagina = list_find(capybara->tabla_de_paginas,(void*)buscarPagina);

		direccionFisica = pagina->marco.comienzo + desplazamiento; //ver si es . o ->

	return direccionFisica;
}

uint32_t generadorIdsCarpinchos(){

    return id_carpincho++;

}

uint32_t generadorIdsMarcos(){

	return id_marco++;
	
}