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

int main(void) {

	iniciarLog("Memoria");
	leerArchivoConfiguracion();
	pthread_t hilo_consola;
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);
	pthread_join(hilo_consola,NULL);

	return 1;

}
//Esta funcion limpia la consola y lee una linea y si no es exit, la imprime por pantalla
void consola(){
	printf("Inicializando consola...");
	sleep(2);
	system("clear");
	puts("Bienvenido a la consola de la memoria...");
	char* comando;
	while(1){
		comando = readline(">");
		if(strncmp(comando,"exit",4) == 0){
			break;
		}
		add_history(comando);

		if (strncmp(comando,"clear",5) == 0) {
			system("clear");
		}else{
			printf("El comando ingresado fue %s\n",comando);
		}
	}
	free(comando);

	return;

}
//Leemos arcchivo configuracion
void leerArchivoConfiguracion(){

		//crearConfig(PATHCONFIG);
		t_config* config;
		config = config_create(PATHCONFIG);
		configuracion *configuracionMemoria = (configuracion*) malloc(sizeof(configuracion));
		configuracionMemoria->IP_FS = (char*) malloc(20);
		char* IP_FS= config_get_string_value(config, "IP_FS");
		//char* IP_FS = obtenerString("IP_FS");
		strcpy(configuracionMemoria->IP_FS, IP_FS);
		printf("%s", configuracionMemoria->IP_FS);

		free(configuracionMemoria);
		/*char* IP_FS;
		strcpy(IP_FS,obtenerString("IP_FS"));
		(*configuracionMemoria).IP_FS = (char*) malloc(sizeof(IP_FS)+1);
		strcpy(configuracionMemoria->IP_FS, IP_FS);
		(*configuracionMemoria).MEMORY_NUMBER = obtenerInt("MEMORY_NUMBER");

		printf("%d, %s, %d", (*configuracionMemoria).PUERTO,(*configuracionMemoria).IP_FS,(*configuracionMemoria).MEMORY_NUMBER);
*/
}



