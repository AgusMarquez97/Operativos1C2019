#include "serializacion.h"


//SERIALIZAR Y DESEREALIZAR DATOS PRIMITIVOS

void serializarInt(void* buffer, int32_t entero, int* desplazamiento) {

	memcpy(buffer + *desplazamiento, &entero, sizeof(int32_t));
	*desplazamiento += sizeof(int32_t);
}

void serializarDouble(void* buffer, int64_t numero, int* desplazamiento) {

	memcpy(buffer + *desplazamiento, &numero, sizeof(int64_t));
	*desplazamiento += sizeof(int64_t);

}

void serializarChar(void* buffer, char caracter, int* desplazamiento) {

	memcpy(buffer + *desplazamiento, &caracter, sizeof(char));
	*desplazamiento += sizeof(char);
}

void serializarString(void* buffer, char* cadena, int* desplazamiento) {


	serializarInt(buffer,strlen(cadena) + 1, desplazamiento);

	memcpy(buffer + *desplazamiento, cadena, strlen(cadena) + 1);
	*desplazamiento += strlen(cadena) + 1;

}

void deserializarInt(void* buffer,int32_t* entero,int* desplazamiento) {

	memcpy(entero, buffer + *desplazamiento, sizeof(int32_t));
	*desplazamiento += sizeof(int32_t);

}

void deserializarDouble(void* buffer,int64_t* entero,int* desplazamiento) {

	memcpy(entero, buffer + *desplazamiento, sizeof(int64_t));
	*desplazamiento += sizeof(int64_t);

}

void deserializarChar(void* buffer,char* caracter,int* desplazamiento){

	memcpy(caracter, buffer + *desplazamiento, sizeof(char));
	*desplazamiento += sizeof(char);

}

void deserializarString(void* buffer,char** cadena,int* desplazamiento){

	int32_t tamanioCadena = 0;

	deserializarInt(buffer,&tamanioCadena, desplazamiento);

	*cadena = malloc(tamanioCadena);

	memcpy(*cadena,buffer + *desplazamiento, tamanioCadena);

	*desplazamiento += strlen(*cadena) + 1;

}

//SERIALIZAR Y DESEREALIZAR DATOS PRIMITIVOS




//SERIALIZAR Y DESEREALIZAR QUERYS

void serializarSelect(void* buffer, char* tabla, int32_t key, int* desplazamiento) {

	serializarInt(buffer,SELECT,desplazamiento);
	serializarString(buffer,tabla,desplazamiento);
	serializarInt(buffer,key,desplazamiento);

}

void serializarInsert(void* buffer, char* tabla, int32_t key, char* value, int64_t timestamp, int* desplazamiento) {

	serializarInt(buffer,INSERT,desplazamiento);
	serializarString(buffer, tabla, desplazamiento);
	serializarInt(buffer, key, desplazamiento);
	serializarString(buffer, value, desplazamiento);
	serializarDouble(buffer, timestamp, desplazamiento);

}

void serializarCreate(void* buffer, char* tabla,int32_t consistencyType,int32_t cantParticiones,int64_t compactationTime,int* desplazamiento){
	serializarInt(buffer,CREATE,desplazamiento);
	serializarString(buffer, tabla, desplazamiento);
	serializarInt(buffer, consistencyType, desplazamiento);
	serializarInt(buffer, cantParticiones, desplazamiento);
	serializarDouble(buffer, compactationTime, desplazamiento);
}

void serializarDescribe(void* buffer, char* tabla, int* desplazamiento){
	serializarInt(buffer,DESCRIBE,desplazamiento);
	serializarString(buffer, tabla, desplazamiento);
}

void serializarDrop(void* buffer, char* tabla,int* desplazamiento){
	serializarInt(buffer,DROP,desplazamiento);
	serializarString(buffer, tabla, desplazamiento);
}


void deserializarSelect(char** tabla, int32_t* key, void* buffer, int* desplazamiento) {

	deserializarString(buffer,tabla,desplazamiento);
	deserializarInt(buffer,key, desplazamiento);

}

void deserializarInsert(char** tabla, int32_t* key, char** value, int64_t* timestamp, void* buffer, int* desplazamiento) {

	deserializarString(buffer,tabla, desplazamiento);
	deserializarInt(buffer,key,desplazamiento);
	deserializarString(buffer,value, desplazamiento);
	deserializarDouble(buffer,timestamp,desplazamiento);

}

void deserializarCreate(char** tabla,int32_t * consistencyType,int32_t * cantParticiones,int64_t * compactationTime, void * buffer, int * desplazamiento){
	deserializarString(buffer,tabla, desplazamiento);
	deserializarInt(buffer,consistencyType,desplazamiento);
	deserializarInt(buffer,cantParticiones, desplazamiento);
	deserializarDouble(buffer,compactationTime,desplazamiento);
}

void deserializarDescribe(char** tabla, void* buffer, int* desplazamiento){
	deserializarString(buffer,tabla, desplazamiento);
}

void deserializarDrop(char** tabla,void* buffer, int* desplazamiento){
	deserializarString(buffer,tabla, desplazamiento);
}

//SERIALIZAR Y DESEREALIZAR QUERYS




//SERIALIZAR Y DESEREALIZAR LISTAS

void serializarListaInt(void* buffer, t_list* listaEnteros, int* desplazamiento) {

	serializarInt(buffer, listaEnteros->elements_count, desplazamiento);

	for(int i=0; i < listaEnteros->elements_count; i++){
		serializarInt(buffer,(int32_t) list_get(listaEnteros, i), desplazamiento);
	}

}

void serializarListaString(void* buffer, t_list* listaCadenas, int* desplazamiento) {

	serializarInt(buffer, listaCadenas->elements_count, desplazamiento);

	for(int i=0; i < listaCadenas->elements_count; i++){
		serializarString(buffer, list_get(listaCadenas, i), desplazamiento);
	}

}

void deserializarListaInts(void* buffer,t_list* listaEnteros, int* desplazamiento) {

	int32_t cantElem = 0;
	deserializarInt(buffer,&cantElem, desplazamiento);

	int entero;

	for(int i=0; i < cantElem; i++) {

		deserializarInt(buffer,&entero, desplazamiento);
		list_add(listaEnteros, (void*)entero);

	}
}

void deserializarListaString( void* buffer, t_list* listaString, int* desplazamiento) {

	int32_t cantElem = 0;
	deserializarInt(buffer, &cantElem, desplazamiento);

	char* cadena;

	for(int i=0; i < cantElem; i++) {
		deserializarString(buffer, &cadena, desplazamiento);
		list_add(listaString, (void*)cadena);

	}

	free(cadena);
}

//SERIALIZAR Y DESEREALIZAR LISTAS



void serializarRequestCorta(int nro, void* buffer, char* tabla, int* desplazamiento){
	serializarInt(buffer,nro,desplazamiento);
	serializarString(buffer, tabla, desplazamiento);
}

void deserializarRequestCorta(char** tabla, void* buffer, int* desplazamiento){
	deserializarString(buffer,tabla, desplazamiento);
}

