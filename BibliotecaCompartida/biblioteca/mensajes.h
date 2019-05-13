#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils.h>

void enviarSelect(int socketReceptor, int operacion, char* tabla, int key);
void enviarInsert(int socketReceptor, int operacion, char* tabla, int key, char* value, int timestamp);

char recibirQuery(int socketEmisor, char** buffer, int* desplazamiento);

#endif /* MENSAJES_H_ */
