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

	crearConfig(PATHCONFIG);
	configuracionMemoria = (configuracion*) malloc(sizeof(configuracion));
	leerArchivoConfiguracion();
	reservarMemoriaPrincipal();

	pthread_t hilo_consola, hilo_conexionKernel, hilo_conexionFS;

	pthread_create(&hilo_conexionFS, NULL, (void*) conexionFS, NULL);
	pthread_create(&hilo_conexionKernel, NULL, (void*) conexionKernel, NULL);
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);

	pthread_join(hilo_conexionFS,NULL);
	pthread_join(hilo_conexionKernel,NULL);
	pthread_join(hilo_consola,NULL);

	free(configuracionMemoria);

	return 1;

}
//Esta funcion limpia la consola y lee una linea y si no es exit, la imprime por pantalla
void consola(){
	printf("Inicializando consola...");
	sleep(2);
	system("clear");
	puts("Bienvenido a la consola de la memoria...");
	query* query= malloc(sizeof(query));
	while(1){
		char* comando = malloc(50);
		comando = readline(">");
		if(strncmp(comando,"exit",4) == 0){
			break;
		}
		add_history(comando);

		if (strncmp(comando,"clear",5) == 0) {
			system("clear");
		}else{
			int tipoDeComando = parsear(comando,query);
			switch(tipoDeComando){
			case 1:
				;
				char* comandoEnChar = malloc(sizeof(char));
				sprintf(comandoEnChar,"%d",tipoDeComando);
				loggearInfoConcatenandoDosMensajes("El numero comando ingresado fue: ", comandoEnChar);
				loggearInfo("El comando ingresado fue SELECT");
				loggearInfoConcatenandoDosMensajes("La tabla del select fue:", query->tabla);
				//procesarSelect(query);
				free(comandoEnChar);
				break;
			case 2:
				;
				char* comandoEnChar2 = malloc(sizeof(char));
				sprintf(comandoEnChar,"%d",tipoDeComando);
				loggearInfoConcatenandoDosMensajes("El numero comando ingresado fue: ", comandoEnChar);
				loggearInfo("El comando ingresado fue INSERT");
				loggearInfoConcatenandoDosMensajes("La tabla al que se realizara el insert es:", query->tabla);
				//procesarInsert(query);
				free(comandoEnChar2);
			break;
			default:
				break;
			}
			system("clear");

		}
		free(comando);
	}


	return;

}
//Leemos arcchivo configuracion
void leerArchivoConfiguracion(){



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
				/*if(i ==0){
					strcat(aux2,(char*) (*(aux+i)));
				}
				else{
					strcat(aux2, ",");
					strcat(aux2,(char*) (*(aux+i)));
				}*/
				strcat(aux2,(char*) (*(aux+i)));
			 i++;
			}
			loggearInfo(aux2);
			free(aux2);

					int j =0;
					char* aux4;
					int* aux3=configuracionMemoria->PUERTO_SEEDS;
					int cantidad_puertos= sizeof(configuracionMemoria->PUERTO_SEEDS);
					aux4= (char*) malloc(25+(cantidad_puertos*sizeof(int)));
					strcpy(aux4,"Puertos de Seeds:");
					while((*(aux3+j)) != NULL){
						if(j ==0){
							strcat(aux4,(char*) (*(aux3+j)));
							//strcat(aux4, ", ");
						}
						else{
							strcat(aux4, ",");
					 strcat(aux4,(char*) (*(aux3+j)));
						}
					 j++;
					}
					log_info(logger,aux4);
					free(aux4);

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
void reservarMemoriaPrincipal(){
	// Reservamos la memoria
		g_TamanioMemoria = configuracionMemoria->TAM_MEM;
		g_BaseMemoria = (char*) malloc(g_TamanioMemoria);
	// Rellenamos con ceros.
		memset(g_BaseMemoria, '0', g_TamanioMemoria * sizeof(char));

	// si no podemos salimos y cerramos el programa.
		if (g_BaseMemoria == NULL )
		{
			loggearInfo("No se pudo reservar la memoria.");
		}
		else
		{

			char* tammem= malloc(7*sizeof(char));
			sprintf(tammem,"%d",g_TamanioMemoria);
			loggearInfoConcatenandoDosMensajes("MEMORIA RESERVADA. Tamaño de la memoria ", tammem);
			free(tammem);
		}
}

/*
char* procesarSelect(query selectQuery){
	return
}*/

/*
char* procesarInsert(query insertQuery){
	return
}*/

void conexionFS(){
	printf("Conectandose al FS");
}

void conexionKernel(){
	printf("Esperando conexion del Kernel");
}



