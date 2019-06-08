#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "utils.h"
#include "enumsAndStructs.h"
#include "serializacion.h"
#include "sockets.h"

//Enviar y recibir datos primitivos:

void loggearSelect(char * tabla, int32_t key);
void loggearInsert(char * tabla, int32_t key, char * value, int64_t timestamp);

void enviarInt(int socketReceptor, int32_t entero);
void enviarChar(int socketReceptor, char caracter);
void enviarString(int socketReceptor, char* cadena);

int recibirInt(int socketEmisor, int32_t* entero);
int recibirChar(int socketEmisor, char* caracter);
int recibirString(int socketEmisor, char** cadena);

/*
 * Funcion utilizada para enviar querys a traves de un socket ya conectado
 */
void enviarQuery(int socketReceptor, query* myQuery);

/*
 * enviarSelect:
 * DIVISION DE TAMANIOS DEL BUFFER:
 * 1° POSICION: 4 BYTES -> TAMANIO_BUFFER (int32_t)
 * 2° POSICION: 4 BYTES -> TIPO_QUERY (int32_t)
 * 3° POSICION: 4 BYTES -> TAMANIO_NOMBRE_TABLA (int32_t)
 * 4° POSICION: N BYTES (DEFINIDOS EN 3°) -> NOMBRE_TABLA (CHAR *)
 * 5° POSICION: 4 BYTES -> KEY_TABLA (int32_t)
 *
 * EN TOTAL: SIZEOF(int32_t)*4 + N BYTES DE NOMBRE_TABLA (STRLEN(NOMBRE_TABLA) + 1)
 */
void enviarSelect(int socketReceptor, char* tabla, int32_t key);

/*
 * enviarInsert:
 * DIVISION DE TAMANIOS DEL BUFFER:
 * 1° POSICION: 4 BYTES -> TAMANIO_BUFFER (int32_t)
 * 2° POSICION: 4 BYTES -> TIPO_QUERY (int32_t)
 * 3° POSICION: 4 BYTES -> TAMANIO_NOMBRE_TABLA (int32_t)
 * 4° POSICION: N BYTES (DEFINIDOS EN 3°) -> NOMBRE_TABLA (CHAR *)
 * 5° POSICION: 4 BYTES -> KEY_TABLA (int32_t)
 * 6° POSICION: 4 BYTES -> TAMANIO_VALUE (int32_t)
 * 7° POSICION: N BYTES -> VALUE (CHAR *)
 * 8° POSICION: 8 BYTES -> TIMESTAMP (int64_t) -> VER DE CAMBIAR A CHAR *
 * EN TOTAL: SIZEOF(int32_t)*5 + SIZEOF((int64_t) + N BYTES DE STRING (STRLEN(NOMBRE_TABLA) + 1) + N BYTES DE VALUE (STRLEN(VALUE) + 1)
 */
void enviarInsert(int socketReceptor, char* tabla, int32_t key, char* value, int64_t timestamp);

/*
 * RECIBIRQUERY:
 * ORDEN OBLIGATORIO:
 * 1° -> 4 BYTES -> TAM BUFFER
 * 2° -> 4 BYTES -> TIPO DE OPERACION
 * 3° -> N BYTES -> BUFFER
 */

int recibirQuery(int socketEmisor, query* myQuery);




#endif /* MENSAJES_H_ */
