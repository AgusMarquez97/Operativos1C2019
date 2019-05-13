#include <mensajes.h>

//ENVIAR

void enviarInt(int socketReceptor, int entero){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(int));

	serializarInt(buffer, entero, &desplazamiento);
	enviar(socketReceptor, buffer, sizeof(int));

	free(buffer);

}

void enviarChar(int socketReceptor, char caracter){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(char));

	serializarInt(buffer, caracter, &desplazamiento);
	enviar(socketReceptor, buffer, sizeof(char));

	free(buffer);

}

void enviarString(int socketReceptor, char* cadena){

	int desplazamiento = 0;
	int tamanioBuffer = sizeof(int) + strlen(cadena) + 1;
	void* buffer = malloc(tamanioBuffer);

	serializarString(buffer, cadena, &desplazamiento);
	enviar(socketReceptor, buffer, tamanioBuffer);

	free(buffer);

}

void enviarQuery(int socketReceptor, query myQuery) {

	switch(myQuery->tipo) {

		case(SELECT):
			enviarSelect(socketReceptor, myQuery->tabla, myQuery->key);
			break;
		case(INSERT):
			enviarSelect(socketReceptor, myQuery->tabla, myQuery->key, myQuery->value, myQuery->timestamp);
			break;
	}
}

void enviarSelect(int socketReceptor, char* tabla, int key) {

	int desplazamiento = 0;
	int tamanioMensaje = sizeof(int)*2 + strlen(tabla) + 1 + sizeof(int);

	enviarInt(socketReceptor, tamanioMensaje);

	char* buffer = malloc(tamanioMensaje);

	serializarSelect(buffer, tabla, key, desplazamiento);

	//enviar select

	free(buffer);

}

void enviarInsert(int socketReceptor, char* tabla, int key, char* value, int timestamp) {

	int desplazamiento = 0;
	int tamanioMensaje = sizeof(int)*2 + strlen(tabla) + 1 + sizeof(int)*2 + strlen(value) + 1 + sizeof(int);

	enviarInt(socketReceptor, tamanioMensaje);

	char* buffer = malloc(tamanioMensaje);

	serializarInsert(buffer, tabla, key, value, timestamp, desplazamiento);

	enviar(socketReceptor, buffer, tamanioMensaje);

	free(buffer);

}

//RECIBIR

void recibirInt(int socketEmisor, int* entero) {

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(int));

	recibir(socketEmisor, buffer, sizeof(int));
	deserializarInt(buffer, entero, &desplazamiento);

	free(buffer);
}

void recibirQuery(int socketEmisor, query* myQuery, int* desplazamiento) {

	int tamanioQuery = 0;
	queryType tipoQuery;

	recibirInt(socketEmisor, &tamanioQuery);

	char* buffer = (char *)malloc(tamanioQuery);

	recibir(socketEmisor, buffer, tamanioQuery);

	deserializarInt(&tipoQuery, *buffer, desplazamiento);

	switch(tipoQuery) {
		case(SELECT):
			deserializarSelect(myQuery->tabla, myQuery->key, buffer, desplazamiento);
			break;
		case(INSERT):
			deserializarSelect(myQuery->tabla, myQuery->key, myQuery->value, myQuery->timestamp, buffer, desplazamiento);
			break;
	}
}




