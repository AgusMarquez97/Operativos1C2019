/*
 ============================================================================
 Name        : PoolMemorias.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "PoolMemorias.h"
#include <biblioteca/mensajes.h>

int main(void) {

	int32_t nroEnviar = 13;
	int clienteObj;
	void * buffer2 = malloc(sizeof(int));
	//clienteObj = levantarCliente("127.0.0.7","8080");

	printf("%d\n",*(int*)buffer2);

	return 1;
}
