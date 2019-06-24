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
	//enviar(socketReceptor, buffer, sizeof(char));

	free(buffer);

}

void enviarString(int socketReceptor, char* cadena){

	int desplazamiento = 0;

	int32_t tamanioCadena = sizeof(int32_t) + strlen(cadena) + 1;
	int32_t tamanioBuffer = sizeof(int32_t) + tamanioCadena;

	void* buffer = malloc(tamanioBuffer);

	serializarInt(buffer,tamanioCadena,&desplazamiento);

	serializarString(buffer, cadena, &desplazamiento);

	enviar(socketReceptor, buffer, tamanioBuffer);

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

	enviar(socketReceptor,buffer,tamanioTotal);

	free(buffer);

}

void enviarInsert(int socketReceptor, char* tabla, int32_t key, char* value, int64_t timestamp) {

	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1 + sizeof(int32_t)*2 + strlen(value) + 1 + sizeof(int64_t);
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer

	serializarInsert(buffer, tabla, key, value, timestamp, &desplazamiento);

	enviar(socketReceptor, buffer, tamanioTotal);

	free(buffer);

}

void enviarCreate(int socketReceptor,char* tabla,int32_t consistencyType,int32_t cantParticiones,int64_t compactationTime)
{
	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1 + sizeof(int32_t)*2 + sizeof(int64_t);
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer

	serializarCreate(buffer,tabla,consistencyType,cantParticiones,compactationTime,&desplazamiento);

	enviar(socketReceptor, buffer, tamanioTotal);

	free(buffer);

}

void enviarDescribe(int socketReceptor,char* tabla)
{
	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1;
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer
	serializarDescribe(buffer,tabla,&desplazamiento);
	enviar(socketReceptor, buffer, tamanioTotal);
	free(buffer);
}

void enviarDrop(int socketReceptor,char* tabla)
{
	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t)*2 + strlen(tabla) + 1;
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer
	serializarDrop(buffer,tabla,&desplazamiento);

	enviar(socketReceptor, buffer, tamanioTotal);
	free(buffer);
}

void enviarRequest(int socketReceptor, int32_t request)
{
	int desplazamiento = 0;
	int32_t tamanioMensaje = sizeof(int32_t);
	int32_t tamanioTotal = tamanioMensaje + sizeof(int32_t);
	void* buffer = malloc(tamanioTotal);

	serializarInt(buffer,tamanioMensaje,&desplazamiento);//Serializo el tamanio total del buffer
	serializarInt(buffer,request,&desplazamiento);

	enviar(socketReceptor, buffer, tamanioTotal);

	free(buffer);
}

void enviarQuery(int socketReceptor, query* myQuery) {

	switch(myQuery->requestType) {
		case SELECT:
			enviarSelect(socketReceptor, myQuery->tabla, myQuery->key);
			break;
		case INSERT:
			enviarInsert(socketReceptor, myQuery->tabla, myQuery->key, myQuery->value, myQuery->timestamp);
			break;
		case CREATE:
			enviarCreate(socketReceptor, myQuery->tabla, myQuery->consistencyType, myQuery->cantParticiones, myQuery->compactationTime);
			break;
		case DESCRIBE:
			if(myQuery->tabla) // !=  NULL
			{
			enviarDescribe(socketReceptor,myQuery->tabla);
			}
			else
			{
			enviarRequest(socketReceptor,DESCRIBE);
			}
			break;
		case DROP:
			enviarDrop(socketReceptor,myQuery->tabla);
			break;
		case JOURNAL:
			enviarRequest(socketReceptor,JOURNAL);
			break;
		case HANDSHAKE:
			enviarRequest(socketReceptor,HANDSHAKE);
			break;
		default:
			loggearError("Request no valido");
			break;
	}
}


//RECIBIR DATOS PRIMITIVOS
//revisar uso
int recibirInt(int socketEmisor, int32_t* entero){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(int32_t));

	int cantidadRecibida = recibir(socketEmisor, buffer, sizeof(int32_t)); //Crear / acomodar esta funcion
	deserializarInt(buffer, entero, &desplazamiento);

	free(buffer);

	return cantidadRecibida;
}

int recibirChar(int socketEmisor, char* caracter){

	int desplazamiento = 0;
	void* buffer = malloc(sizeof(char));

	int cantidadRecibida = recibir(socketEmisor, buffer, sizeof(char));
	deserializarChar(buffer, caracter, &desplazamiento);

	free(buffer);
	return cantidadRecibida;
}
int recibirString(int socketEmisor, char** cadena)
{
		int cantidadRecibida;
		int desplazamiento = 0;
		int tamBuffer = 0;

		void * buffer = malloc(sizeof(int32_t)); //Primero se reserva el tamanio de lo que se va a recibir

		cantidadRecibida = recibir(socketEmisor,buffer,sizeof(int32_t));

		deserializarInt(buffer,&tamBuffer,&desplazamiento); //Se recibe el tam del buffer!!

		buffer = realloc(buffer,tamBuffer);

		cantidadRecibida += recibir(socketEmisor,buffer,tamBuffer); //Se recibe el buffer

		desplazamiento = 0; //Me paro en el inicio del nuevo buffer

		deserializarString(buffer, cadena, &desplazamiento);

		char * log = malloc(100);
		strcpy(log,"Se recibio {");
		strcat(log,*cadena);
		strcat(log,"}");
		loggearInfo(log);

		free(*cadena);
		free(log);
		free(buffer);

		return cantidadRecibida;
}


//RECIBIR QUERY
int recibirQuery(int socketEmisor, query ** myQuery) {

	int cantidadRecibida;
	int desplazamiento = 0;

	int32_t tamanioQuery = 0;
	int32_t tipoQuery;
	int32_t tamanioBuffer = 0;

	cantidadRecibida = recibirInt(socketEmisor, &tamanioQuery); //Se recibe el tam del buffer

	if(cantidadRecibida == 0)
	{
		return 0;
	}
	cantidadRecibida += recibirInt(socketEmisor, &tipoQuery);

	tamanioBuffer = tamanioQuery - sizeof(int32_t);
	void * buffer = malloc(tamanioBuffer); //Se le resta el tamanio del tipo de query

	cantidadRecibida += recibir(socketEmisor,buffer,tamanioBuffer);

	*myQuery = realloc(*myQuery,tamanioQuery);

	switch(tipoQuery) {
		case SELECT:
		((*myQuery))->requestType = SELECT;
		deserializarSelect(&(*myQuery)->tabla, &(*myQuery)->key, buffer, &desplazamiento);
		loggearSelect((*myQuery)->tabla,(*myQuery)->key);
		break;
		case INSERT:
		(*myQuery)->requestType = INSERT;
		deserializarInsert(&(*myQuery)->tabla, &(*myQuery)->key, &(*myQuery)->value, &(*myQuery)->timestamp, buffer, &desplazamiento);
		loggearInsert((*myQuery)->tabla,(*myQuery)->key,(*myQuery)->value,(*myQuery)->timestamp);
		break;
		case CREATE:
		((*myQuery))->requestType = CREATE;
		deserializarCreate(&(*myQuery)->tabla, &(*myQuery)->consistencyType, &(*myQuery)->cantParticiones, &(*myQuery)->compactationTime, buffer, &desplazamiento);
		loggearCreate((*myQuery)->tabla,(*myQuery)->consistencyType, (*myQuery)->cantParticiones, (*myQuery)->compactationTime);
		break;
		case DESCRIBE:
		((*myQuery))->requestType = DESCRIBE;
		if(cantidadRecibida > sizeof(int32_t))
		{
		deserializarDescribe(&(*myQuery)->tabla, buffer, &desplazamiento);
		loggearInfo3Mensajes("Se recibio la siguiente query: {DESCRIBE ",(*myQuery)->tabla,"}");
		}else
		{
			loggearInfo("Se recibio la siguiente query: {DESCRIBE}");
		}
			break;
		case DROP:
		((*myQuery))->requestType = DROP;
		deserializarDrop(&(*myQuery)->tabla, buffer, &desplazamiento);
		loggearInfo3Mensajes("Se recibio la siguiente query: {DROP ",(*myQuery)->tabla,"}");
			break;
		case JOURNAL:
		((*myQuery))->requestType = JOURNAL;
		loggearInfo("Se recibio un request Journal");
			break;
		case HANDSHAKE:
		((*myQuery))->requestType = HANDSHAKE;
		loggearInfo("Se realiza un Handshake");
			break;
		default:
			loggearInfo("Request no valida");
	}
	//free(*myQuery);
	return cantidadRecibida;
}


void loggearSelect(char * tabla, int32_t key)
{
	char * aux = malloc(100);
	char * log = malloc(strlen("Se recibio la siguiente query: {SELECT ") + strlen(tabla) + 8 + 30);

	strcpy(log,"Se recibio la siguiente query: {SELECT ");
	strcat(log,tabla);
	strcat(log," ");
	sprintf(aux,"%d",key);
	strcat(log,aux);
	strcat(log,"}");

	loggearInfo(log);

	free(aux);
	free(log);

}
void loggearInsert(char * tabla, int32_t key, char * value, int64_t timestamp)
{
	char * aux = malloc(200);
	char * log = malloc(strlen("Se recibio la siguiente query: {INSERT ") + strlen(tabla) + strlen(value) + 12);

	strcpy(log,"Se recibio la siguiente query: {INSERT ");
	strcat(log,tabla);
	strcat(log," ");
	sprintf(aux,"%d",key);
	strcat(log,aux);
	strcat(log," '");
	strcat(log,value);
	strcat(log,"' ");
	sprintf(aux,"%lli",timestamp);
	strcat(log,aux);
	strcat(log,"}");

	loggearInfo(log);

	free(aux);
	free(log);
}


void loggearCreate(char *tabla,int32_t consistencyType, int32_t cantParticiones, int64_t compactationTime){

	char * log = malloc(strlen("Se recibio la siguiente query: {CREATE         ") + strlen(tabla) + 1 + sizeof(int32_t)*2 + sizeof(int64_t));
	char * aux = malloc(100);

	strcpy(log,"Se recibio la siguiente query: {CREATE ");
	strcat(log,tabla);
	strcat(log," ");
	strcat(log,consistencia_a_string(consistencyType));
	strcat(log," ");
	sprintf(aux,"%d",cantParticiones);
	strcat(log,aux);
	strcat(log," ");
	sprintf(aux,"%lli",compactationTime);
	strcat(log,aux);
	strcat(log,"}");

	loggearInfo(log);

	free(aux);
	free(log);
}



