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

uint32_t generadorIdsCarpinchos(){

    return id_carpincho++;

}