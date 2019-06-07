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

		crearConfig(PATHCONFIG);

		configuracionMemoria = (configuracion*) malloc(sizeof(configuracion));

		configuracionMemoria->IP_FS = obtenerString("IP_FS");
		configuracionMemoria->IP_SEEDS = obtenerArray("IP_SEEDS");
		configuracionMemoria->PUERTO_FS = obtenerInt("PUERTO_FS");
		configuracionMemoria->PUERTO_SEEDS = (int*) obtenerArray("PUERTO_SEEDS");
		configuracionMemoria->PUERTO = obtenerInt("PUERTO");
		configuracionMemoria->MEMORY_NUMBER = obtenerInt("MEMORY_NUMBER");
		configuracionMemoria->RETARDO_FS = obtenerInt("RETARDO_FS");
		configuracionMemoria->RETARDO_GOSSIPING = obtenerDouble("RETARDO_GOSSIPING");
		configuracionMemoria->RETARDO_JOURNAL = obtenerDouble("RETARDO_JOURNAL");
		configuracionMemoria->RETARDO_MEM = obtenerInt("RETARDO_MEM");
		configuracionMemoria->TAM_MEM = obtenerInt("TAM_MEM");

		EscribirArchivoLog();
		free(configuracionMemoria);

}
void EscribirArchivoLog(){
			int n = configuracionMemoria->PUERTO_FS;
			char* puertoFS = malloc(4*sizeof(char));
			sprintf(puertoFS,"%d",n);
			iniciarLog("Log Memoria");
			loggearInfoConcatenandoDosMensajes("El IP del FS es: ", (char*) configuracionMemoria->IP_FS);
			loggearInfoConcatenandoDosMensajes("El puerto del FS es: ", puertoFS);
			free(puertoFS);

			char** aux = configuracionMemoria->IP_SEEDS;
			int i =0;
			char* aux2;
			int longitud = 0;
			while((*(aux+i)) != NULL){
			  longitud+=strlen((*(aux+i)))+1;
			  i++;
			 }
			i=0;
			aux2 = malloc(longitud+20);
			strcpy(aux2,"Direcciones de IP de Seeds:");
			while((*(aux+i)) != NULL){
			 strcat(aux2,(*(aux+i)));
			 i++;
			}
			loggearInfo(aux2);

					int j =0;
					char* aux4;
					int* aux3=configuracionMemoria->PUERTO_SEEDS;
					int cantidad_puertos= sizeof(configuracionMemoria->PUERTO_SEEDS);
					aux4= malloc(20+cantidad_puertos*sizeof(int));
					strcpy(aux4,"Puertos de Seeds:");
					while((*(aux3+j)) != NULL){
						if(j ==0){
							strcat(aux4,(char*) (*(aux3+j))),
							strcat(aux4, ", ");
						}
						else{
					 strcat(aux4,(char*) (*(aux3+j)));
						}
					 j++;
					}
					loggearInfo(aux4);

					int nropuerto = configuracionMemoria->PUERTO;
					char* puerto = malloc(4*sizeof(char));
					sprintf(puerto,"%d",nropuerto);
					loggearInfoConcatenandoDosMensajes("El puerto de la memoria es: ", puerto);
					free(puerto);

					int mnum =configuracionMemoria->MEMORY_NUMBER;
					char* memorynumber= malloc(5*sizeof(char));
					sprintf(memorynumber,"%d",mnum);
					loggearInfoConcatenandoDosMensajes("El numero de la memoria es: ", memorynumber);
					free(memorynumber);


					int rfs =configuracionMemoria->RETARDO_FS;
					char* retardofs= malloc(5*sizeof(char));
					sprintf(retardofs,"%d",rfs);
					loggearInfoConcatenandoDosMensajes("El retardo de FS es: ", retardofs);
					free(retardofs);

					int rGos =configuracionMemoria->RETARDO_GOSSIPING;
					char* retardogos= malloc(7*sizeof(char));
					sprintf(retardogos,"%d",rGos);
					loggearInfoConcatenandoDosMensajes("El retardo de Gossiping es: ", retardogos);
					free(retardogos);

					int rjou =configuracionMemoria->RETARDO_JOURNAL;
					char* retardojou= malloc(7*sizeof(char));
					sprintf(retardojou,"%d",rjou);
					loggearInfoConcatenandoDosMensajes("El retardo de Journal es: ", retardojou);
					free(retardojou);

					int rmem =configuracionMemoria->RETARDO_MEM;
					char* retardomem= malloc(7*sizeof(char));
					sprintf(retardomem,"%d",rmem);
					loggearInfoConcatenandoDosMensajes("El retardo de Memoria es: ", retardomem);
					free(retardomem);

					int tam =configuracionMemoria->TAM_MEM;
					char* tammem= malloc(7*sizeof(char));
					sprintf(tammem,"%d",tam);
					loggearInfoConcatenandoDosMensajes("El tamaño de Memoria es: ", tammem);
					free(tammem);
}




