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
	listaSegmentos = list_create();
	remove("Lissandra.log");
	crearConfig(PATHCONFIG);
	configuracionMemoria = (configuracion*) malloc(sizeof(configuracion));
	leerArchivoConfiguracion();
	handshake();
	reservarMemoriaPrincipal();

	pthread_t hilo_consola, hilo_conexionKernel;


	pthread_create(&hilo_conexionKernel, NULL, (void*) conexionKernel, NULL);
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);

	pthread_join(hilo_conexionKernel,NULL);
	pthread_join(hilo_consola,NULL);

	free(configuracionMemoria);



}

void consola(){
	system("clear");
	puts("Bienvenido a la consola de la memoria...");
	argumentosQuery * args;
	query* query = NULL;
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
			int aux = parsear(comando,&query);
			args = malloc(sizeof(argumentosQuery));
			args->unaQuery = query;
			args->flagConsola = 1;
			if(aux != -1)
			{
				procesarQuery(args);
			}
			else
			{
		    	printf("Request no valida\n");
		    }

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
}

int obtenerCantidadMarcos(int tamanioPagina, int tamanioMemoria)
{
	float tam_nec = (float)tamanioMemoria/tamanioPagina;
	if(tam_nec == (int)tam_nec)
	return (int)tam_nec;
	return (int)round(tam_nec + 0.5);
}

void reservarMemoriaPrincipal(){
	// Reservamos la memoria


		tamanioMemoria = configuracionMemoria->TAM_MEM;
		memoriaPrincipal = (char*) malloc(tamanioMemoria);
		tamanioPag = sizeof(int32_t) + sizeof(int64_t) + tamanioValue;
		cantidadMarcos = obtenerCantidadMarcos(tamanioPag,tamanioMemoria);

		levantarMarcos(cantidadMarcos);

	// Rellenamos con ceros.
		memset(memoriaPrincipal, 0, tamanioMemoria * sizeof(char));

		char* tammem= malloc(7*sizeof(char));
		sprintf(tammem,"%d",tamanioMemoria);
		loggearInfoConcatenandoDosMensajes("MEMORIA RESERVADA. Tamaño de la memoria ", tammem);
		free(tammem);

}


void levantarMarcos(int cantidadMarcos)
{
	char * bitmap = calloc(1,cantidadMarcos);
	marcosMemoria = bitarray_create_with_mode(bitmap, cantidadMarcos/8, LSB_FIRST);

	for(int i = 0; i<cantidadMarcos;i++)
	{
	bitarray_clean_bit(marcosMemoria,i); //seteo todos los bits en 0 (al principio todos los bloques están libres)
	}
}


//-----------------------------------------------------------------------
//----------------------------CONEXIONES---------------------------------
//-----------------------------------------------------------------------

int conexionFS(){

		char* IP = strdup(configuracionMemoria->IP_FS);
		//char * IP = strdup(obtenerString("IP_FS"));
		char * Puerto =string_itoa(configuracionMemoria->PUERTO_FS);

		loggearInfoCliente(IP,Puerto);

		int clienteFS = levantarCliente(IP,Puerto);

		free(IP);
		free(Puerto);

		return clienteFS;
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
				int socketRespuesta;
				pthread_t hiloAtendedor = 0;
		        int socketServidor = levantarServidor(servidorIP,servidorPuerto);

		        argumentosQuery * args = malloc(sizeof(argumentosQuery));
		        args->unaQuery = NULL;
		        args->flagConsola = 0;

		        while(1)
		        {
		        	if((socketRespuesta = aceptarConexion(socketServidor)) != -1)
		        	{
		        		loggearNuevaConexion(socketRespuesta);

		        		if((hiloAtendedor = crearHilo(procesarQuery,(void*)args)) != 0)
		        		{

		        		}
		        		else
		        		{
		        		loggearError("Error al crear un hilo");
		        		}
		        	}
		        }
}


//-----------------------------------------------------------------------
//-------------------PROCESAMIENTO DE QUERYS-----------------------------
//-----------------------------------------------------------------------

void procesarQuery(argumentosQuery * args)
{
	int flagConsola = args->flagConsola;
	switch(args->unaQuery->requestType)
	{
			case SELECT:
				loggearSelect(args->unaQuery->tabla,args->unaQuery->key);
				procesarSelect(args->unaQuery,flagConsola);
				break;
			case INSERT:
				loggearInsert(args->unaQuery->tabla,args->unaQuery->key,args->unaQuery->value,args->unaQuery->timestamp);
				procesarInsert(args->unaQuery,flagConsola);
				break;
			case CREATE:
				procesarCreate(args->unaQuery,flagConsola);
				break;
			case DESCRIBE:
				procesarDescribe(args->unaQuery,flagConsola);
				break;
			case DROP:
				procesarDrop(args->unaQuery,flagConsola);
				break;
			case JOURNAL:
				procesarJournal(args->unaQuery,flagConsola);
				break;
	default:
		loggearInfo("Request Aun No Disponible");
	}
	free(args); //Validar
}


int procesarSelect(query* selectQuery, int flagConsola)
{
	char * nombreTabla;
	query * queryInsertFS;
	int key = selectQuery->key;
	int socketFS;
	int datosRecibidos;
	t_list * listaPaginas;
	registro * registroRetorno = NULL,* registroFinal = NULL;
	if(dictionary_has_key(tablaSegmentos,selectQuery->tabla)) //Valida que exista el segmento !
	{
		listaPaginas = (t_list *) dictionary_get(tablaSegmentos,selectQuery->tabla);
		void buscarKey(pagina * pagina)
		{
			registroRetorno = leerMarco(pagina->nroMarco);
			if(registroRetorno->key==key)
			{
				registroFinal = registroRetorno;
			}else{
				free(registroRetorno->value);
				free(registroRetorno);
			}
		}

		list_iterate(listaPaginas,(void*)buscarKey);
		if(registroFinal != NULL)
		{
		loggearRegistroEncontrado(key,registroFinal->value,flagConsola);
		free(registroFinal->value);
		free(registroFinal);
		return 1;
		}
	}
		socketFS = conexionFS();
		enviarQuery(socketFS,selectQuery);
		datosRecibidos = recibirQuery(socketFS,&queryInsertFS);
		if(datosRecibidos > 0 && queryInsertFS->requestType == INSERT && !strcmp(queryInsertFS->tabla,tabla) &&queryInsertFS->key == key)
		{
		loggearRegistroEncontrado(key,queryInsertFS->value,flagConsola);
		t_list * listaNula = list_create();
		dictionary_put(tablaSegmentos,queryInsertFS->tabla,listaNula);
		nombreTabla = strdup(queryInsertFS->tabla);
		list_add(listaSegmentos,nombreTabla);
		agregarPagina(queryInsertFS->tabla,queryInsertFS->key,queryInsertFS->value,queryInsertFS->timestamp,0);
		}else{
		loggearRegistroNoEncontrado(key,flagConsola);}

		return 2;

}

void procesarInsert(query* queryInsert, int flagConsola)
{
	t_list * listaPaginas;
	registro * registroRetorno, *registroFinal = NULL;
	int key = queryInsert->key;
	int nroMarco = -1;

	if(dictionary_has_key(tablaSegmentos,queryInsert->tabla))
	{
			listaPaginas = (t_list*) dictionary_get(tablaSegmentos,queryInsert->tabla);

			void buscarKey(pagina * pagina)
			{
					registroRetorno = leerMarco(pagina->nroMarco);

					if(registroRetorno->key==key)
					{
					registroFinal = registroRetorno;
					nroMarco = pagina->nroMarco;
					}else{
					free(registroRetorno->value);
					free(registroRetorno);
					}

			}
			if(registroFinal != NULL && nroMarco != -1)
			{
			memset(memoriaPrincipal + nroMarco*tamanioPag,0,tamanioPag);
			escribirMarco(nroMarco,registroFinal);
			}
			else
			{
			agregarPagina(queryInsert->tabla,queryInsert->key,queryInsert->value,queryInsert->timestamp,1);
			}
		list_iterate(listaPaginas,(void*)buscarKey);
	} else
	{
		t_list * listaNula = list_create();
		char * nombreTabla = strdup(queryInsert->tabla);
		dictionary_put(tablaSegmentos,tabla,listaNula);
		list_add(listaSegmentos,nombreTabla);
		agregarPagina(queryInsert->tabla,queryInsert->key,queryInsert->value,queryInsert->timestamp,1);
	}
}
void procesarCreate(query* queryCreate, int flagConsola)
{
	int estadoResultado;
	int socketFS;
	int datosRecibidos;
	socketFS = conexionFS();
	enviarQuery(socketFS,queryCreate);
	datosRecibidos = recibirInt(socketFS,&estadoResultado);

	if(datosRecibidos>0 && estadoResultado == 1)
	{
		loggearTablaCreadaOK(queryCreate,flagConsola);
	}else
	{
		loggearErrorTablaExistente(queryCreate,flagConsola);
	}

}

void procesarDescribe(query* queryDescribe, int flagConsola)
{
	char * describe = NULL;
	int socketFS;
	int datosRecibidos;
	socketFS = conexionFS();
	enviarQuery(socketFS,queryDescribe);
	datosRecibidos = recibirString(socketFS,&describe);

	if(datosRecibidos>0 && describe)
	{
		loggearInfo(describe);
		if(flagConsola)
			printf("%s",describe);
	}else
	{
		if(queryDescribe->tabla)
		loggearInfo("Tabla no existente en el sistema");
		else
		loggearInfo("No existen tablas en FS");
	}

}

void procesarDrop(query* queryDrop, int flagConsola)
{
	t_list * paginas;
	if(dictionary_has_key(tablaSegmentos,queryDrop->tabla))
	{
		paginas = (t_list*) dictionary_remove(tablaSegmentos,queryDrop->tabla);

		void liberarPagina(pagina * unaPagina)
		{

			liberarMarco(unaPagina->nroMarco);
		}

		list_iterate(paginas,(void*)liberarPagina);
		list_destroy_and_destroy_elements(paginas,free);
	}
	int estadoResultado;
	int socketFS;
	int datosRecibidos;
	socketFS = conexionFS();
	enviarQuery(socketFS,queryDrop);
	datosRecibidos = recibirInt(socketFS,&estadoResultado);

	if(datosRecibidos>0 && estadoResultado == 1)
	{
		loggearTablaDropeadaOK(queryDrop->tabla,flagConsola);
	}else
	{
		loggearErrorDrop(queryDrop->tabla,flagConsola);
	}
}

void procesarJournal(query* queryJournal, int flagConsola)
{
	ejecutarJournal(flagConsola);
}

void ejecutarJournal(int flagConsola)
{
	//Cuando este ocurriendo un journal no debe accederse a la memoria principal!

	int resultadoJournal = journal();
	if(resultadoJournal == 1)
	{
	loggearInfo("Journal exitoso");
	if(flagConsola)
		printf("Journal exitoso");
	}else
	{
		loggearInfo("Error en Journal");
		if(flagConsola)
			printf("Error en Journal");
	}
}

void ejecutarRutinaJournal()
{
	while(1)
	{
		usleep(configuracionMemoria->RETARDO_JOURNAL*1000);

		ejecutarJournal(0);
	}
}


int journal()
{

	int indice = 0;
	t_list * registros = list_create();
	t_list * paginas;

	while(dictionary_has_key(tablaSegmentos,list_get(listaSegmentos,indice)))
	{
		paginas = (t_list *) dictionary_get(tablaSegmentos,list_get(listaSegmentos,indice));
		void agregarRegistro(pagina * unaPagina)
		{
			list_add(registros,leerMarco(unaPagina->nroMarco));
		}
		list_iterate(paginas,(void*)agregarRegistro);

		enviarQuerysFS(list_get(listaSegmentos,indice),registros);
		indice++;
	}

	dictionary_clean_and_destroy_elements(tablaSegmentos,free);
	free(memoriaPrincipal);
	return 1;
}

void liberarMarco(int nroMarco)
{
	int offset = nroMarco*tamanioPag;
	pthread_mutex_lock(&mutex_journal);
	memset(memoriaPrincipal + offset,0,tamanioPag); // dudosa
	bitarray_clean_bit(marcosMemoria,nroMarco); // Se libera el marco para poder ser usado
	pthread_mutex_unlock(&mutex_journal);
	sem_post(semaforoMemoria);
}

void agregarPagina(char * tabla, int32_t key,char * value, int64_t timestamp, int flagModificado)
{
	t_list * paginas;
	pagina * paginaActual = malloc(sizeof(pagina));

	paginas = (t_list*) dictionary_remove(tablaSegmentos,tabla);

	paginaActual->nroPagina = list_size(paginas);
	paginaActual->nroMarco = obtenerMarcoLibre();
	paginaActual->flagModificado = flagModificado;

	registro * regAgregar = malloc(sizeof(registro));
	regAgregar->key = key;
	regAgregar->value = value;
	regAgregar->timestamp = timestamp;

	escribirMarco(paginaActual->nroMarco,regAgregar);

	free(regAgregar->value); // VER SI ROMPE ESTA LINEA!
	free(regAgregar);

	list_add(paginas,paginaActual);

	dictionary_put(tablaSegmentos,tabla,paginas);
}

int obtenerMarcoLibre()
{
	int marco = -1;
	for(int i = 0; i < cantidadMarcos;i++)
	{
		if(estaLibre(i))
		marco = i;
	}
	if(marco == -1)
	{
		return ejecutarLRU();
	}
	sem_wait(semaforoMemoria);
	bitarray_set_bit(marcosMemoria,marco);
	return marco;
}

registro * leerMarco(int nroMarco)
{
	 int offset = nroMarco * tamanioPag;
	 registro * unReg = malloc(sizeof(registro));

	 deserializarInt(memoriaPrincipal,&unReg->key,&offset);
	 deserializarString(memoriaPrincipal,&unReg->value,&offset);
	 deserializarDouble(memoriaPrincipal,&unReg->timestamp,&offset);

	 return unReg;
}

void escribirMarco(int nroMarco, registro * unReg)
{
	 int offset = nroMarco * tamanioPag;
	 serializarInt(memoriaPrincipal,unReg->key,&offset);
	 serializarString(memoriaPrincipal, unReg->value,&offset);
	 serializarDouble(memoriaPrincipal,unReg->timestamp,&offset);
}

bool estaLibre(int nroMarco)
{
	return bitarray_test_bit(marcosMemoria,nroMarco) == 0;
}

int ejecutarLRU()
{
	return 1;
}


void enviarQuerysFS(char * tabla, t_list * registros)
{
	t_list * querys = list_create();
	query * unaQuery;
	int socketFS;
	void agregarQuery(registro * unRegistro)
	{
		unaQuery = malloc(sizeof(query));

		unaQuery->requestType = INSERT;
		unaQuery->tabla = tabla;
		unaQuery->key = unRegistro->key;
		unaQuery->value = strdup(unRegistro->value);
		unaQuery->timestamp = unRegistro->timestamp;

		list_add(querys,unaQuery);
		free(unRegistro->value); // VALIDAR
	}
	list_iterate(registros,(void *)agregarQuery);

	void enviarUnaQuery(query * unaQuery)
	{
		socketFS = conexionFS();
		enviarQuery(socketFS,unaQuery);
		free(unaQuery->value);

	}

	list_iterate(querys,(void *)enviarUnaQuery);

	list_destroy_and_destroy_elements(registros,free);
	list_destroy_and_destroy_elements(querys,free);

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
					while(((aux3+j)) != NULL){
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



void handshake()
{
	int datosRecibidos;
	char * puerto = malloc(40);
	sprintf(puerto,"%d",configuracionMemoria->PUERTO);
	int socketCliente = levantarCliente(configuracionMemoria->IP_FS,puerto);
	tamanioValue = -1;

	enviarInt(socketCliente,HANDSHAKE);

	datosRecibidos = recibirInt(socketCliente,&tamanioValue);

	if(datosRecibidos > 0 && tamanioValue != -1)
	{
		loggearInfo("Handshake exitoso");
	}
	else
	{
		loggearError("Error al realizar el handshake");
	}

	free(puerto);

}


void loggearRegistroEncontrado(int key, char * value, int flagConsola)
{
	char * nro = malloc(100);
	char * aux = malloc(strlen("Key buscada: s'' value: ''") + strlen(value) + 5);

	sprintf(nro,"%d",key);

	strcpy(aux,"Key buscada: '");
	strcat(aux,nro);
	strcat(aux,"'");
	strcat(aux," value: '");
	strcat(aux,value);
	strcat(aux,"'");

	loggearInfo(aux);

	if(flagConsola)
	printf("%s\n",aux);

	free(nro);
	free(aux);
}

void loggearRegistroNoEncontrado(int key, int flagConsola)
{
	char * nro = malloc(100);
	sprintf(nro,"%d",key);
	char * aux = malloc(strlen("No se encontro value para la siguiente key ''") + 5);
	strcpy(aux,"No se encontro value para la siguiente key '");
	strcat(aux,nro);
	strcat(aux,"'");

	loggearInfo(aux);

	if(flagConsola)
	printf("%s\n",aux);

	free(aux);
	free(nro);
}


void loggearErrorTablaExistente(query * unaQuery,int flagConsola)
{
	char * aux = malloc(strlen("Error la tabla  '' ya existe en el sistema") + strlen(unaQuery->tabla) + 10);
	strcpy(aux,"Error la tabla '");
	strcat(aux,unaQuery->tabla);
	strcat(aux,"' ya existe en el sistema");
	loggearInfo(aux);
	if(flagConsola)
	printf("%s\n",aux);
	free(aux);
}

void loggearTablaCreadaOK(query * unaQuery,int flagConsola)
{
		char * aux = malloc(strlen("Se creo correctamente la tabla  ''") + strlen(unaQuery->tabla) + 30);
		strcpy(aux,"Se creo correctamente la tabla '");
		strcat(aux,unaQuery->tabla);
		strcat(aux,"'");

		loggearInfo(aux);
		if(flagConsola)
		printf("%s\n",aux);
		free(aux);
}

void loggearTablaDropeadaOK(char * tabla, int flagConsola)
{
	char * aux = malloc(strlen("Tabla '' eliminada Correctamente") + strlen(tabla) + 10);
	strcpy(aux,"Tabla '");
	strcat(aux,tabla);
	strcat(aux,"' eliminada Correctamente");
	loggearInfo(aux);
	if(flagConsola)
	printf("%s\n",aux);
	free(aux);
}
void loggearErrorDrop(char * tabla, int flagConsola)
{
	char * aux = malloc(strlen("La tabla '' no existe en el Sistema") + strlen(tabla) + 10);
	strcpy(aux,"Tabla '");
	strcat(aux,tabla);
	strcat(aux,"' no existe en el Sistema");
	loggearInfo(aux);
	if(flagConsola)
	printf("%s\n",aux);
	free(aux);
}

void liberarRegistro(registro * unRegistro)
{
	if(unRegistro)
	{
		if(unRegistro->value)
			free(unRegistro->value);
	free(unRegistro);
	}
}


void inicializarSemaforos()
{
pthread_mutex_init(&mutex_journal, NULL);
sem_init(semaforoMemoria,0,cantidadMarcos);
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

//Antes que nada => releer el drive parte memoria & leer issues como un campeon => OK

//Terminar bien servidor => PONELE

//Validar estructuras adm => OK

//Ver como seria el offset de memoria => https://github.com/sisoputnfrba/foro/issues/1246 => OK

//Levantar hilo monitor => PENDIENTE

//Hacer el handshake con FS para poder calcular el tamanio de marco por el max tam del value => EN TESTING

//Pensar algoritmo de reemplazo de paginas -> PENDIENTE
//LRU => https://github.com/sisoputnfrba/foro/issues/1174 & https://github.com/sisoputnfrba/foro/issues/1171

//Terminar API => EN TESTING

//Definir bien el JOURNALING => EN TESTING

//Ver gossiping => PENDIENTE

//Ver criterios de consistencia => PENDIENTE

//Hacer pruebas hasta romper el mundo => PENDIENTE









