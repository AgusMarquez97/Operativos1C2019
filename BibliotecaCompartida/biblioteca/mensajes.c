#include "mensajes.h"

//ENVIAR DATOS PRIMITIVOS

void enviarInt(int socketReceptor, int32_t entero){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(int32_t));

	serializarInt(buffer, entero, &desplazamiento);
	enviar(socketReceptor, buffer, sizeof(int32_t));

	free(buffer);

}

void enviarChar(int socketReceptor, char caracter){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(char));

	serializarChar(buffer, caracter, &desplazamiento);
	enviar(socketReceptor, buffer, sizeof(char));

	free(buffer);

}

void enviarString(int socketReceptor, char* cadena){

	int desplazamiento = 0;
	int32_t tamanioBuffer = sizeof(int32_t) + strlen(cadena) + 1;
	void* buffer = malloc(tamanioBuffer);

	serializarString(buffer, cadena, &desplazamiento);
	enviar(socketReceptor, buffer, tamanioBuffer);

	free(buffer);

}

// ENVIAR QUERYS

void enviarSelect(int socketReceptor, char* tabla, int32_t key) {

	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1 + sizeof(int32_t);

	enviarInt(socketReceptor, tamanioMensaje); //Ver de omitir

	char* buffer = malloc(tamanioMensaje);

	serializarSelect(buffer, tabla, key, &desplazamiento);

	//enviar select

	free(buffer);

}

void enviarInsert(int socketReceptor, char* tabla, int32_t key, char* value, int64_t timestamp){

	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1 + sizeof(int32_t)*2 + strlen(value) + 1 + sizeof(int64_t);

	enviarInt(socketReceptor, tamanioMensaje);

	char* buffer = malloc(tamanioMensaje);

	serializarInsert(buffer, tabla, key, value, timestamp, &desplazamiento);

	enviar(socketReceptor, buffer, tamanioMensaje);//Ver de omitir

	free(buffer);

}

void enviarQuery(int socketReceptor, query* myQuery) {

	switch(myQuery->requestType) {
		case(SELECT):
			enviarSelect(socketReceptor, myQuery->tabla, myQuery->key);
			break;
		case(INSERT):
			enviarInsert(socketReceptor, myQuery->tabla, myQuery->key, myQuery->value, myQuery->timestamp);
			break;
		default:
			break;
	}
}


//RECIBIR DATOS PRIMITIVOS
//revisar uso
void recibirInt(int socketEmisor, int32_t* entero){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(int32_t));

	recibir(socketEmisor, buffer, sizeof(int32_t)); //Crear / acomodar esta funcion
	deserializarInt(buffer, entero, &desplazamiento);

	free(buffer);
}

void recibirChar(int socketEmisor, char* caracter){
	int desplazamiento = 0;
	void* buffer = malloc(sizeof(char));
	recibir(socketEmisor, buffer, sizeof(char));
	deserializarChar(buffer, caracter, &desplazamiento);
	free(buffer);
}

void recibirString(int socketEmisor, char* cadena){
	/*
    VALIDAR TAM A RECIBIR -> ES VARIABLE recibir(socketEmisor, buffer,tam); -> que poner en tam
	int desplazamiento = 0;
	void* buffer = malloc(tam);

	deserializarString(buffer, cadena, &desplazamiento);
	free(buffer);
	 */
}


//RECIBIR QUERY

void recibirQuery(int socketEmisor, query* myQuery, int* desplazamiento) {

	int32_t tamanioQuery = 0;
	int32_t tipoQuery;
	recibirInt(socketEmisor, &tamanioQuery);

	char* buffer = (char *)malloc(tamanioQuery);

	recibir(socketEmisor,buffer, tamanioQuery);

	deserializarInt(buffer,&tipoQuery,desplazamiento);

	switch(tipoQuery) {
		case(SELECT):
		deserializarSelectSinHeader(myQuery->tabla, &myQuery->key, buffer, desplazamiento);
			break;
		case(INSERT):
		deserializarInsertSinHeader(myQuery->tabla, &myQuery->key, myQuery->value, &myQuery->timestamp, buffer, desplazamiento);
			break;
	}
}




