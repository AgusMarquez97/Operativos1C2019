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
#include <readline/readline.h>
#include <readline/history.h>
#include <biblioteca/levantarConfig.h>
#include <biblioteca/logs.h>

#define PATH "configuraciones/memoria.cfg"


int main(void) {

		/*
	int32_t nroEnviar = 13;
	int clienteObj;
	void * buffer2 = malloc(sizeof(int));
	//clienteObj = levantarCliente("127.0.0.7","8080");

	printf("%d\n",*(int*)buffer2);

	*/
	leerArchivoConfiguracion();
	pthread_t hilo_consola;
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);
	pthread_join(hilo_consola,NULL);

	return 1;

}
//Esta funcion limpia la consola y lee una linea y si no es exit, la imprime por pantalla
void consola(){
	printf("Aca va la consola =)");
	system("clear");
	puts("Bienvenido a la consola de la memoria");
	char* comando;
	while(1){
		comando = readline("$");
		if(comando == "exit"){
		return;
		}else{
			printf("El comando ingresado fue %s\n",comando);
		}
	}
	free(comando);

	return;

}
//Leemos arcchivo configuracion
void leerArchivoConfiguracion(){
	t_config* config;
	config = crearConfig(PATH);
	iniciarLogConPath("Log memorias.log","Log Pool Memorias");

	int puertoEscucha = obtenerInt("PUERTO");
	char* ip_FS = obtenerString("IP_FS");
	int puerto_FS = obtenerInt("PUERTO_FS");
	char ** ip_Seeds = obtenerArray("IP_SEEDS");
	char ** puerto_Seeds = obtenerArray("PUERTO_SEEDS");
	int retardo_mem = obtenerInt("RETARDO_MEM");
	int retardo_FS = obtenerInt("RETARDO_FS");
	int tam_mem = obtenerInt("TAM_MEM");
	int retardo_journal = obtenerInt("RETARDO_JOURNAL");
	int retardo_gossiping = obtenerInt("RETARDO_GOSSIPING");
	int mem_number= obtenerInt("MEMORY_NUMBER");

	printf("el puerto es:%d?n",puertoEscucha);

}



