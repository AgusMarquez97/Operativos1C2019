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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(void) {

		/*
	int32_t nroEnviar = 13;
	int clienteObj;
	void * buffer2 = malloc(sizeof(int));
	//clienteObj = levantarCliente("127.0.0.7","8080");

	printf("%d\n",*(int*)buffer2);

	*/
	pthread_t hilo_consola;
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);
	pthread_join(hilo_consola,NULL);

	return 1;

}

//Esta funcion limpia la consola y lee una linea y si no es exit, la imprime por pantalla
void consola(){

	system("clear");
	puts("Bienvenido a la consola de la memoria");
	while(1){
		char* comando = readline(">");
		if(comando == "exit"){
			return;
		}else{
			printf("El comando ingresado fue %s\n",comando);
		}
	}
	return;

}
