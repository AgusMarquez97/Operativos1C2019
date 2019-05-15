#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>

#include "utils.h"
#include "enumsAndStructs.h"

//Serializaciones primitivas
void serializarInt(void* buffer, int32_t entero, int* desplazamiento);
void serializarDouble(void* buffer, int64_t numero, int* desplazamiento);
void serializarChar(void* buffer, char caracter, int* desplazamiento);
void serializarString(void* buffer, char* cadena, int* desplazamiento);

void deserializarInt(void* buffer,int32_t* entero,int* desplazamiento);
void deserializarDouble(void* buffer,int64_t* entero,int* desplazamiento);
void deserializarChar(void* buffer,char* caracter,int* desplazamiento);
void deserializarString(void* buffer,char* cadena,int* desplazamiento);

//Querys:
void serializarSelect(void* buffer, char* tabla, int32_t key, int* desplazamiento);
void serializarInsert(void* buffer, char* tabla, int32_t key, char* value, int64_t timestamp, int* desplazamiento);

void deserializarSelectConHeader(char* tabla,int32_t* key, void* buffer, int* desplazamiento);
void deserializarSelectSinHeader(char* tabla, int32_t* key, void* buffer, int* desplazamiento);
void deserializarInsertConHeader(char* tabla, int32_t* key, char* value, int64_t* timestamp, void* buffer, int* desplazamiento);
void deserializarInsertSinHeader(char* tabla, int32_t* key, char* value, int64_t* timestamp, void* buffer, int* desplazamiento);

//Extra: Listas
void serializarListaInt(void* buffer, t_list* listaEnteros, int* desplazamiento);
void serializarListaString(void* buffer, t_list* listaCadenas, int* desplazamiento);

void deserializarListaInts(void* buffer,t_list* listaEnteros, int* desplazamiento);
void deserializarListaString( void* buffer, t_list* listaString, int* desplazamiento);

#endif /* SERIALIZACION_H_ */
