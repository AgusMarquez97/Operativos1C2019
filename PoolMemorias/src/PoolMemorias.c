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
	remove("Lissandra.log");
	crearConfig(PATHCONFIG);
	configuracionMemoria = (configuracion*) malloc(sizeof(configuracion));
	leerArchivoConfiguracion();
	reservarMemoriaPrincipal();

	pthread_t hilo_consola, hilo_conexionKernel, hilo_conexionFS;

	//pthread_create(&hilo_conexionFS, NULL, (void*) conexionFS, NULL);
	pthread_create(&hilo_conexionKernel, NULL, (void*) conexionKernel, NULL);
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);

	pthread_join(hilo_conexionFS,NULL);
	pthread_join(hilo_conexionKernel,NULL);
	pthread_join(hilo_consola,NULL);

	free(configuracionMemoria);

	return 1;

}

void consola(){
	printf("Inicializando consola...");
	sleep(2);
	system("clear");
	puts("Bienvenido a la consola de la memoria...");
	query* query;
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
			int aux = parsear(comando,&query);
			if(aux != -1)
			{
				loggearInfo("Query enviada por consola!!");
				procesarQuery(query);
			}
			else
			{
		    	printf("Request no valida\n");
		    }
			system("clear");
		}
		free(comando);
	}


	return;

}

void leerArchivoConfiguracion(){

		configuracionMemoria->IP_FS = getCleanString(obtenerString("IP_FS"));
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

		loggearArchivoDeConfiguracion();

		memoriaPrincipal = malloc(configuracionMemoria->TAM_MEM);




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

//-----------------------------------------------------------------------
//----------------------------CONEXIONES---------------------------------
//-----------------------------------------------------------------------

void conexionFS(){

		char* IP = strdup("127.0.0.1");
		//char * IP = strdup(obtenerString("IP_FS"));
		char * Puerto = strdup(obtenerString("PUERTO_FS"));
		loggearInfoCliente(IP,Puerto);
		int cliente = levantarCliente(IP,Puerto);

		free(IP);
		free(Puerto);
}


void conexionKernel(){
	char * IP;

	if(obtenerString("IP"))
	{
			IP = strdup(obtenerString("DIRECCION_IP"));
	}else
	{
			IP = strdup("127.0.0.1");
	}
		char * Puerto = strdup(obtenerString("PUERTO"));
		loggearInfoServidor(IP,Puerto);

		levantarServidorMemoria(IP,Puerto);

		free(IP);
		free(Puerto);
}



void levantarServidorMemoria(char * servidorIP, char* servidorPuerto)
{
			query * myQuery;
	  	  	int socketRespuesta, maximoSocket;
	  	  	int datosRecibidos = -1;
			fd_set sockets, clientes;

	        int socketServidor = levantarServidor(servidorIP,servidorPuerto);

	        LimpiarSet(&sockets);
	        LimpiarSet(&clientes);

	        agregarASet(socketServidor,&sockets);
	        maximoSocket = socketServidor;

	        tiempoEspera esperaMaxima;
	        definirEsperaServidor(&esperaMaxima,300);

	        while(1) // Loop para escuchar conexiones entrantes
			{
	        clientes = sockets;
	        ejecutarSelect(maximoSocket,&clientes,&esperaMaxima); //pasarle null si no importa

	        for(int i = 0;i<=maximoSocket;i++) //Itero hasta el ultimo socket
	        	{
	        		if (estaEnSet(i, &clientes)) //1 - Tengo un cliente nuevo que quiere leer!
	        		{
	                    if (i == socketServidor) //1-1 - Ese Cliente es el mismo servidor -> Acepto nuevas conexiones
	                    {

	                    socketRespuesta = aceptarConexion(socketServidor);

	                    agregarASet(socketRespuesta,&sockets); //añado a set de sockets al nuevo cliente

	                        if (socketRespuesta > maximoSocket) // Trackeo el maximo socket
	                        {
	                            maximoSocket = socketRespuesta;
	                        }

	                    loggearNuevaConexion(socketRespuesta);

	                    }
	                    else
	                    {
	                    	datosRecibidos = recibirQuery(i,&myQuery);
	                    	if(datosRecibidos==0)
	                    	{
	                    		close(i);
	                    		EliminarDeSet(i,&sockets);
	                    	}else  {
	                    		loggearDatosRecibidos(i,datosRecibidos);
	                    		pthread_t procesarQ = crearHilo(procesarQuery,(void*)myQuery);
	                    		esperarHilo(procesarQ);
	                    	}

	                    }
	        		}
	        	}
			}
}


//-----------------------------------------------------------------------
//-------------------PROCESAMIENTO DE QUERYS-----------------------------
//-----------------------------------------------------------------------

void procesarQuery(query* query)
{
	switch(query->requestType)
	{
	case SELECT:
		loggearSelect(query->tabla,query->key);
		loggearInfo("El comando ingresado fue SELECT");
		loggearInfoConcatenandoDosMensajes("La tabla del select fue:", query->tabla);
		procesarSelect(query);
		break;
	case INSERT:
		loggearInsert(query->tabla,query->key,query->value,query->timestamp);
		loggearInfo("El comando ingresado fue INSERT");
		loggearInfoConcatenandoDosMensajes("La tabla al que se realizara el insert es:", query->tabla);
		procesarInsert(query);
		break;
	case CREATE:
		procesarCreate(query);
		break;
	case DESCRIBE:
		procesarDescribe(query);
		break;
	case DROP:
		procesarDrop(query);
		break;
	case JOURNAL:
		procesarJournal(query);
		break;
	default:
		loggearInfo("Request Aun No Disponible");
	}
}


void procesarSelect(query* selectQuery){

}

void procesarInsert(query* insertQuery){

}

void procesarCreate(query* createQuery){

}

void procesarDescribe(query* describeQuery){

}

void procesarDrop(query* dropQuery){

}

void procesarJournal(query* journalQuery){

}

//-----------------------------------------------------------------------
//--------------------------------LOGS-----------------------------------
//-----------------------------------------------------------------------

void loggearArchivoDeConfiguracion(){
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

void loggearInfoServidor(char * IP, char * Puerto)
{

	char * ipServidor = malloc(strlen("IP del servidor: ") + strlen(IP) + 1);
	strcpy(ipServidor,"IP del servidor: ");
	strcat(ipServidor,IP);

	char * puertoServidor = malloc(strlen("Puerto de escucha: ") + strlen(Puerto) + 1);
	strcpy(puertoServidor,"Puerto de escucha: ");
	strcat(puertoServidor,Puerto);

	loggearInfo(ipServidor);
	loggearInfo(puertoServidor);

	free(ipServidor);
	free(puertoServidor);
}

void loggearInfoCliente(char * IP, char * Puerto)
{

	char * ipServidor = malloc(strlen("IP del Servidor de FS es: ") + strlen(IP) + 1);
	strcpy(ipServidor,"IP del Servidor de FS es: ");
	strcat(ipServidor,IP);

	char * puertoServidor = malloc(strlen("Puerto del FS es: ") + strlen(Puerto) + 1);
	strcpy(puertoServidor,"Puerto del FS es: ");
	strcat(puertoServidor,Puerto);

	loggearInfo(ipServidor);
	loggearInfo(puertoServidor);

	free(ipServidor);
	free(puertoServidor);
}

void loggearNuevaConexion(int socket)
{
	  char * info = malloc(strlen("Nueva conexion asignada al socket: ") + 10 + 4);
	  char * aux = malloc(10);

	  strcpy(info,"Nueva conexion asignada al socket: ");
	  snprintf(aux,10,"%d",socket);
	  strcat(info,aux);
	  strcat(info,"\n");

	  loggearInfo(info);

	  free(info);
      free(aux);
}


void loggearDatosRecibidos(int socket, int datosRecibidos)
{
		  char * info = malloc(strlen("Se recibieron bytes del socket  ") + 30 + 5);
		  char * aux = malloc(30);

		  strcpy(info,"Se recibieron:  ");
		  snprintf(aux,30,"%d",datosRecibidos);
		  strcat(info," bytes del socket ");
		  snprintf(aux,30,"%d",socket);
		  strcat(info,aux);
		  strcat(info,"\n");

		  loggearInfo(info);

		  free(info);
	      free(aux);
}


//-----------------------------------------------------------------------
//--------------------------------HELPERS--------------------------------
//-----------------------------------------------------------------------

char* getCleanString(char* dirtyString){
		int i,len=strlen(dirtyString);
		for(i=1;i<len-1;i++)
		{
			dirtyString[i-1]=dirtyString[i];
		}
		dirtyString[i-1]='\0';
		return dirtyString;
}







/*
 * Posibles querys:
 *
 * SELECT -> BUSCA EN LA TABLA DE SEGMENTOS -> SI NO LA TIENE VA DERECHO A FS Y ESPERA RTA
 * -> POR ULTIMO RETORNA Y ALMACENA DICHA RTA
 *
 * INSERT -> VALIDA SOLO MEMORIA -> FS NO HACE NADA
 * CREATE -> DERECHO AL FS -> NO HACE NADA EN MEMORIA -> PASAMANOS
 * DROP -> LIBERA EL SEGMENTO + TABLA DE PAG ASOCIADAS -> ENVIA A FS PARA QUE REPLIQUE
 * DESCRIBE -> PIDE A FS Y MUESTRA POR PANTALLA -> PASAMANOS
 * JOURNAL -> VACIA LA MEMORIA PRINCIPAL
 *
 * CONSIDERACIONES
 *
 * LA INFORMACION DE LOS REGISTROS EN SI NO DEBERIA ESTAR EN EL DICCIONARIO DE SEGMENTOS NI EN UN LISTA DE PAGINAS
 * ASI COMO LA INFO DE FS ESTA EN BLOQUES, AQUI LA INFO ESTA CONTENIDA EN LA MEMORIA PRINCIPAL. DICHA MEMORIA DEBERIA SER
 * UN CHAR[TAM_MEMORIA] O ALGUNA VARIANTE QUE RESPETE QUE LA INFO ESTE CONTIGUA. ESTE CHAR HAY QUE DIVIDIRLO EN PAGINAS
 * DE ALGUNA FORMA. CADA PAGINA DEBE SER IDENTIFICADA
 *
 * -> CUANDO UNA MEMORIA SE LLENA HAY QUE EJECUTAR EL JOURNALING
 * EL JOURNALING CONSISTE EN ITERAR EL DICCIONARIO BUSCANDO PAGINAS MODIFICADAS PARA ENVIAR A FS.
 * UNA VEZ ENVIADAS SE LIBERA LA MEMORIA DE MEMORIA
 *
 * PENSARLO EN PRINCIPIO COMO 1 SOLA MEMORIA => ES IMPORTANTE DEFINIR QUE UNA NUEVA MEMORIA ES UN NUEVO PROCESO, DICHO
 * PROCESO NACE CON FORK(). LAS MEMORIAS CON GOSSIPING NO ESTARAN COMUNICADAS COMO HILOS NI COMPARTIRAN MEMORIA. ESTAS SE
 * COMUNICARAN MEDIANTE SOCKETS. => VER ISSUE https://github.com/sisoputnfrba/foro/issues/1311
 */

/*-------------------------------- IMPORTANTE DEF DE MEMORIA -------------------------------------
 *
 * LA MEMORIA TENDRA UN TAMANIO FIJO DEFINIDO EN EL ARCHIVO DE CONFIG. DICHA MEMORIA SERA UN GRAN CHAR *
 * QUE TENDRA UN MALLOC PARA RESERVAR TODA SU MEMORIA. EN DICHA MEMORIA ESTARAN LOS REGISTROS PER SE.
 * LOS REGISTROS SE COPIARAN A MEMORIA CON LA FUNCION MEMCPY. LA MEMORIA PRINCIPAL ESTARA DIVIDIDA EN
 * MARCOS
 *
 * PROBLEMA PRINCIPAL -> TAMAÑO DE MARCO:
 *
 * ISSUE: https://github.com/sisoputnfrba/foro/issues/1447
 * EL TAMANIO DEL MARCO = TAM_KEY + TAM_TIMESTAMP + MAX_TAM_VALUE (FS) => PENSAR A LA MEMORIA YA DIVIDIDA POR ESTOS MARCOS
 *  *
 * ENTONCES: LOS MARCOS TENDRAN FRAGMENTACION INTERNA. HAY QUE VALIDAR LA FORMA EN ESCRIBIR EN DICHOS MARCOS,
 * LO IDEAL Y BUSCADO ES QUE LA FRAGMENTACION ESTE SOLO EN EL ULTIMO MARCO.
 */


/*
 * CASO EN QUE EL BIT DE MODIFICADO SERA 0:
 * " Si yo te hago select desde el kernel o la memoria, y la memoria no tiene la clave que le pedí,
 * la va ir a buscar, asignarla en donde tenga que hacerlo y va a tener el bit 0 porque no hubo modificación "
 *
 */



/*
 * LA MAYOR COMPLEJIDAD PARECERIA ESTAR EN LOS CRITERIOS DE CONSISTENCIA, EL GOSSIPING Y PODER MANTENER A LAS MEMORIAS
 * AUN CUANDO 1 MUERA
 */




// POR HACER:

//Antes que nada => releer el drive parte memoria & leer issues como un campeon

//Terminar bien servidor

//Validar estructuras adm

//Ver como seria el offset de memoria => https://github.com/sisoputnfrba/foro/issues/1246

//Levantar hilo monitor

//Hacer el handshake con FS para poder calcular el tamanio de marco por el max tam del value

//Pensar algoritmo de reemplazo de paginas ->
//LRU => https://github.com/sisoputnfrba/foro/issues/1174 & https://github.com/sisoputnfrba/foro/issues/1171

//Terminar API

//Definir bien el JOURNALING

//Ver criterios de consistencia

//Hacer pruebas hasta romper el mundo


void agregarSegmentoYPaginas(query * queryInsert, int flagConsola); // ver logica del drive

void buscarEnMemoria(query * querySelect, int flagConsola); // tiene que patear la tabla y buscar en mem

void ejecutarJournaling(); //Usar variable global & analizar todos los posibles casos cuando ocurra







