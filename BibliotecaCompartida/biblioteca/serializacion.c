#include "serializacion.h"

//SERIALIZAR

void serializarInt(void* buffer, int entero, int* desplazamiento) {

	memcpy(buffer + *desplazamiento, &entero, sizeof(int));
	*desplazamiento += sizeof(int);

}

void serializarDouble(void* buffer, double numero, int* desplazamiento) {

	memcpy(buffer + *desplazamiento, &numero, sizeof(double));
	*desplazamiento += sizeof(double);

}

void serializarChar(void* buffer, char caracter, int* desplazamiento) {

	memcpy(buffer + *desplazamiento, &caracter, sizeof(char));
	*desplazamiento += sizeof(char);

}

void serializarString(void* buffer, char* cadena, int* desplazamiento) {

	serializarInt(buffer, strlen(cadena) + 1, desplazamiento);

	memcpy(buffer + *desplazamiento, &cadena, strlen(cadena) + 1);
	*desplazamiento += strlen(cadena) + 1;

}

void serializarListaInt(void* buffer, t_list* listaEnteros, int* desplazamiento) {

	serializarInt(buffer, listaEnteros->elements_count, desplazamiento);

	for(int i=0; i < listaEnteros->elements_count; i++){
		serializarInt(buffer, list_get(listaEnteros, i), desplazamiento);
	}

}

void serializarListaString(void* buffer, t_list* listaCadenas, int* desplazamiento) {

	serializarInt(buffer, listaCadenas->elements_count, desplazamiento);

	for(int i=0; i < listaCadenas->elements_count; i++){
		serializarString(buffer, list_get(listaCadenas, i), desplazamiento);
	}

}

void serializarSelect(void* buffer, char* tabla, int key, int* desplazamiento) {

	serializarInt(SELECT, buffer, desplazamiento);
	serializarString(tabla, buffer, desplazamiento);
	serializarInt(key, buffer, desplazamiento);

}

void serializarInsert(void* buffer, char* tabla, int key, char* value, double timestamp, int* desplazamiento) {

	serializarInt(INSERT, buffer, desplazamiento);
	serializarString(buffer, tabla, desplazamiento);
	serializarInt(buffer, key, desplazamiento);
	serializarString(buffer, value, desplazamiento);
	serializarDouble(buffer, timestamp, desplazamiento);

}

//SERIALIZAR

//DESERIALIZAR

void deserializarInt(int* entero, void* buffer, int* desplazamiento) {

	memcpy(&(*entero), buffer + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

}

void deserializarChar(char* caracter, void* buffer, int* desplazamiento) {

	memcpy(&(*caracter), buffer + *desplazamiento, sizeof(char));
	*desplazamiento += sizeof(int);

}

void deserializarString(char** cadena, void* buffer, int* desplazamiento) {

	int tamanioCadena = 0;

	deserializarInt(&tamanioCadena, buffer, desplazamiento);

	*cadena = malloc(tamanioCadena);

	memcpy(*cadena, buffer + *desplazamiento, tamanioCadena);
	*desplazamiento += sizeof(int);

}

void deserializarListaEnteros(t_list* listaEnteros, void* buffer, int* desplazamiento) {

	int cantElem = 0;
	deserializarInt(&cantElem, buffer, desplazamiento);

	int entero;

	for(int i=0; i < cantElem; i++) {

		deserializarInt(&entero, buffer, desplazamiento);

		list_add(listaEnteros, (void*)entero);

	}
}

void deserializarListaString(t_list* listaString, void* buffer, int* desplazamiento) {

	int cantElem = 0;
	deserializarInt(&cantElem, buffer, desplazamiento);

	char* cadena;

	for(int i=0; i < cantElem; i++) {

		deserializarString(&cadena, buffer, desplazamiento);

		list_add(listaString, (void*)cadena);

	}
	 int operacion,
	free(cadena);
}

//chequear si funciona asi, o le paso char* tabla y despues hago &tabla cuando deserializo string
void deserializarSelect(char** tabla, int* key, void* buffer, int* desplazamiento) {

	deserializarString(tabla, buffer, desplazamiento);
	deserializarInt(key, buffer, desplazamiento);

}

void deserializarInsert(char** tabla, int* key, char** value, double* timestamp, void* buffer, int* desplazamiento) {

	deserializarString(tabla, buffer, desplazamiento);
	deserializarInt(key, buffer, desplazamiento);
	deserializarString(value, buffer, desplazamiento);
	deserializarInt(timestamp, buffer, desplazamiento);

}

//DESERIALIZAR

