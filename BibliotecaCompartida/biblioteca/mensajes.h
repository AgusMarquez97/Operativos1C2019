#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "enumsAndStructs.h"
#include "serializacion.h"
#include "sockets.h"

//Enviar y recibir datos primitivos:

void enviarInt(int socketReceptor, int32_t entero);
void enviarChar(int socketReceptor, char caracter);
void enviarString(int socketReceptor, char* cadena);

void recibirInt(int socketEmisor, int32_t* entero);
void recibirChar(int socketEmisor, char* caracter);
void recibirString(int socketEmisor, char* cadena);

void enviarQuery(int socketReceptor, query* myQuery);
void enviarSelect(int socketReceptor, char* tabla, int32_t key);
void enviarInsert(int socketReceptor, char* tabla, int32_t key, char* value, int64_t timestamp);

void recibirQuery(int socketEmisor, query* myQuery, int* desplazamiento);




#endif /* MENSAJES_H_ */
