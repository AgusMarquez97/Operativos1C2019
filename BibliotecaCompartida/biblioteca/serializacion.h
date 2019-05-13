#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils.h>
#include <enums.h>

#include <commons/collections/list.h>

void serializarInt(void* buffer, int entero, int* desplazamiento);
void serializarChar(void* buffer, char caracter, int* desplazamiento);
void serializarString(void* buffer, char* cadena, int* desplazamiento);
void serializarListaInt(void* buffer, t_list* listaEnteros, int* desplazamiento);
void serializarListaString(void* buffer, t_list* listaCadenas, int* desplazamiento);
void serializarSelect(void* buffer, char* tabla, int key, int* desplazamiento);
void serializarInsert(void* buffer, char* tabla, int key, char* value, int timestamp, int* desplazamiento);

void deserializarInt(int* entero, void* buffer, int* desplazamiento);
void deserializarChar(char* caracter, void* buffer, int* desplazamiento);
void deserializarString(char** cadena, void* buffer, int* desplazamiento, int tamanioCadena);
void deserializarListaString(t_list* listaString, void* buffer, int* desplazamiento);
void deserializarListaString(t_list* listaEnteros, void* buffer, int* desplazamiento);
void deserializarSelect(char** tabla, int* key, void* buffer, int* desplazamiento);
void deserializarInsert(char** tabla, int* key, char** value, double* timestamp, void* buffer, int* desplazamiento);

#endif /* SERIALIZACION_H_ */
