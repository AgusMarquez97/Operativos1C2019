#include <mensajes.h>

//ENVIAR

void enviarSelect(int socketReceptor, int operacion, char* tabla, int key) {

	int desplazamiento = 0;
	int tamanioMensaje = sizeof(int) + sizeof(char) + sizeof(int);

	//enviar tamanio

	char* buffer = malloc(tamanioMensaje);

	serializarSelect(buffer, tabla, key, desplazamiento);

	//enviar select

	free(buffer);

}

void enviarInsert(int socketReceptor, int operacion, char* tabla, int key, char* value, int timestamp) {

	int desplazamiento = 0;
	int tamanioMensaje = sizeof(int) + sizeof(char) + sizeof(int) + sizeof(char) + sizeof(int);

	//enviar tamanio

	char* buffer = malloc(tamanioMensaje);

	serializarInsert(buffer, tabla, key, value, timestamp, desplazamiento);

	//enviar insert

	free(buffer);

}

//RECIBIR

//recibe la data de la query y la almacena en el buffer. devuelve que query es para que la puedas deserializar luego
char recibirQuery(int socketEmisor, char** buffer, int* desplazamiento) {

	int tamanioQuery;

	//recibir tamanio

	*buffer = (char *)malloc(tamanioQuery);

	//recibir query

	char tipoQuery;
	deserializarInt(&tipoQuery, *buffer, desplazamiento);

	return tipoQuery;

}




