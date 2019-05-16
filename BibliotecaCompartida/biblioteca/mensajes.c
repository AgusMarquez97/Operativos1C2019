#include "mensajes.h"

//ENVIAR DATOS PRIMITIVOS

void enviarInt(int socketReceptor, int32_t entero){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(int32_t));

	serializarInt(buffer, entero, &desplazamiento);
	//enviar(socketReceptor, buffer, sizeof(int32_t));

	free(buffer);

}

void enviarChar(int socketReceptor, char caracter){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(char));

	serializarChar(buffer, caracter, &desplazamiento);
	//enviar(socketReceptor, buffer, sizeof(char));

	free(buffer);

}

void enviarString(int socketReceptor, char* cadena){

	int desplazamiento = 0;
	int32_t tamanioBuffer = sizeof(int32_t) + strlen(cadena) + 1;
	void* buffer = malloc(tamanioBuffer);

	serializarInt(buffer,tamanioBuffer,&desplazamiento);
	serializarString(buffer, cadena, &desplazamiento);
	//enviar(socketReceptor, buffer, tamanioBuffer);

	free(buffer);

}

// ENVIAR QUERYS

void enviarSelect(int socketReceptor, char* tabla, int32_t key) {

	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1 + sizeof(int32_t);
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer
	serializarSelect(buffer, tabla, key, &desplazamiento);//Serializo la query

	//enviar(socketReceptor,buffer,tamanioTotal);

	//enviar select

	free(buffer);

}

void enviarInsert(int socketReceptor, char* tabla, int32_t key, char* value, int64_t timestamp){

	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1 + sizeof(int32_t)*2 + strlen(value) + 1 + sizeof(int64_t);
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer
	serializarInsert(buffer, tabla, key, value, timestamp, &desplazamiento);

	//enviar(socketReceptor, buffer, tamanioTotal);//Ver de omitir

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
		int32_t tam_cadena;
		int desplazamiento = 0;

		recibir(socketEmisor, &tam_cadena, sizeof(int32_t)); //Se recibe el tam de la cadena

		void * buffer = malloc(sizeof(tam_cadena));

		recibir(socketEmisor, buffer, tam_cadena); //Se recibe el buffer
		deserializarString(buffer, cadena, &desplazamiento);

		free(buffer);
}


//RECIBIR QUERY

void recibirQuery(int socketEmisor, query* myQuery) {

	int desplazamiento = 0;
	int32_t tamanioQuery = 0;
	int32_t tipoQuery;

	recibirInt(socketEmisor, &tamanioQuery); //Se recibe el tam del buffer

	void* buffer = malloc(tamanioQuery);

	recibir(socketEmisor,buffer,tamanioQuery); //Se recibe el buffer

	deserializarInt(buffer,&tipoQuery,&desplazamiento);

	switch(tipoQuery) {
		case(SELECT):
		deserializarSelect(myQuery->tabla, &myQuery->key, buffer, desplazamiento);
			break;
		case(INSERT):
		deserializarInsert(myQuery->tabla, &myQuery->key, myQuery->value, &myQuery->timestamp, buffer, desplazamiento);
			break;
	}
}




