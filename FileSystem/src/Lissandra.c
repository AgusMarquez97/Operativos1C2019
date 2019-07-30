#include "Lissandra.h"


void iniciarLFS()
{

	remove("Memtable.log");
	remove("Lissandra.log");

	iniciarLog("FileSystem");

	levantarConfig();

	memTable = dictionary_create();

	//levantarMemTable();



	//fileSystem = crearHilo(gestionarFileSystem,NULL);
	//esperarHilo(fileSystem); // Crea las estructuras administrativas
	//levantarMemTable();
	hiloConsola = crearHilo(consola,NULL);
	//hiloServidor = crearHilo(iniciarServidor,NULL);

	esperarHilo(hiloConsola);
	//esperarHilo(hiloServidor);
}


void levantarConfig()
{
	crearConfig("/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones/configuracion.txt");

	maxValue =  obtenerInt("TAMAÑO_VALUE");
	retardo = obtenerInt("RETARDO"); //en milisegundos  -> puede modificarse (ver cuando y de que forma)
	dumping = obtenerInt("TIEMPO_DUMP"); //en milisegundos  -> puede modificarse (ver cuando y de que forma)

	puntoMontaje = eliminarComillas(obtenerString("PUNTO_MONTAJE"));
	eliminarEstructuraConfig();
	hiloMonintor = crearHilo(monitorearConfig,NULL); //Va a ser hilo detacheable
}

void actualizarConfig()
{
	//actualizarVariablesGlobales();
	//reenviarConfig();
}

void actualizarVariablesGlobales()
{
	loggearInfo("UN CAMBIO JE");

	crearConfig("/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones/configuracion.txt");

	retardo = obtenerInt("RETARDO");
	dumping = obtenerInt("TIEMPO_DUMP");

	eliminarEstructuraConfig();

}

void reenviarConfig()
{
	/*
	 * Hay que crear un cliente que le envie a memoria los nuevos datos del config
	 */
	return;
}

void monitorearConfig()
{

	const char * pathConfig = "/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones";

	int bytes_leidos;
	struct inotify_event evento;


    int archivo_monitor = inotify_init();

	if(archivo_monitor == -1)
	{
	 perror("No se pudo crear el archivo monitor");
	}

	    int monitor = inotify_add_watch(archivo_monitor,pathConfig,IN_MODIFY);//Ver mask

	    if(monitor == -1)
	    {
	        perror("No se pudo crear el monitor");
	    }
	    int lecturas = 0;
	    while(1)
	    {

	    	if(lecturas == 2) lecturas = 0;
	        bytes_leidos = read(archivo_monitor,&evento,sizeof(struct inotify_event)*20);
	        //Bloquea al proceso/hilo hasta que ocurra el evento declarado en el monitor
	        if(bytes_leidos <= 0)
	        {
	            perror("Error al leer el archivo monitor..");
	            break;
	        }else if(evento.mask == IN_MODIFY)
	        {
	        	actualizarConfig();
	        }



	    }

}

void levantarServidorLFS(char * servidorIP, char* servidorPuerto)
{
	  	  	int socketRespuesta;
	  	  	pthread_t hiloAtendedor = 0;

	        int socketServidor = levantarServidor(servidorIP,servidorPuerto);

	        while(1)
	        {
	        	if((socketRespuesta = aceptarConexion(socketServidor)) != -1)
	        	{
	        		loggearNuevaConexion(socketRespuesta);

	        		if((hiloAtendedor = crearHilo(rutinaServidor,(void*)socketRespuesta)) != 0)
	        		{

	        		}
	        		else
	        		{
	        		loggearError("Error al crear un hilo");
	        		}
	        	}
	        }
}

void rutinaServidor(int socketCliente)
{
	argumentosQuery * args = NULL;
	query * myQuery = NULL;
	int datosRecibidos = -1;

	while(datosRecibidos!=0)
	{
		datosRecibidos = recibirQuery(socketCliente,&myQuery);

		if(datosRecibidos>0)
		{
			if(myQuery)
			{
		args = malloc(sizeof(argumentosQuery));

		args->unaQuery = malloc(sizeof(query)); // Ver de eliminar
		args->unaQuery = myQuery;
		args->flag = 0;
		if(args->unaQuery->requestType == INSERT)
		{
			procesarQuery(args); // Se procesa la query sin necesidad de notificar el resultado -> JOURNALING !
		}
		else
		{
			responderQuery(socketCliente,args); //Se envia una respuesta a memoria
		}

		free(args);
		free(args->unaQuery);
			}
		}

	}

	close(socketCliente);

}


void responderQuery(int socketCliente, argumentosQuery * args)
{
	query * queryDescribe;
	query * querySelect;
	registro * selectAuxMT;
	registro * selectAuxFS;
	int retornoCreate;
	char * cadenaDescribe;
	switch(args->unaQuery->requestType)
				{
				case DESCRIBE:
						cadenaDescribe = rutinaFileSystemDescribe(args->unaQuery->tabla);
						if(cadenaDescribe != NULL)
						{
						loggearInfo(cadenaDescribe);
						}else
						{
							if(args->unaQuery->tabla == NULL)
							{
						char * mensajeErrorTabla = strdup("Error, no hay tablas en el sistema");
						loggearInfo(mensajeErrorTabla);
							}
							else
							{
						char * mensajeErrorTablas = strdup("Error, Tabla no existente en el sistema");
						loggearInfo(mensajeErrorTablas);
							}
						}
						queryDescribe = malloc(sizeof(query));
						queryDescribe->requestType = DESCRIBE;
						queryDescribe->tabla = strdup(cadenaDescribe);
						free(cadenaDescribe);
						enviarQuery(socketCliente,queryDescribe);
						break;
					case SELECT:
						selectAuxMT = procesarSelectMemTable(args->unaQuery);
						selectAuxFS = rutinaFileSystemSelect(args->unaQuery->tabla, args->unaQuery->key);

						querySelect = malloc(sizeof(query));
						querySelect->requestType = SELECT;
						querySelect->key = args->unaQuery->key;
						querySelect->timestamp = -1;

						if(selectAuxFS != NULL)
						{
							if(selectAuxMT != NULL)
							{
								if(selectAuxMT->timestamp > selectAuxFS->timestamp)
								{
									querySelect->value = strdup(selectAuxMT->value);
									loggearRegistroEncontrado(args->unaQuery->key,selectAuxMT->value,0);
								}else
								{
									querySelect->value = strdup(selectAuxFS->value);
								loggearRegistroEncontrado(args->unaQuery->key,selectAuxFS->value,0);
								free(selectAuxFS);
								}
							}
							else
							{
								querySelect->value = strdup(selectAuxFS->value);
								loggearRegistroEncontrado(args->unaQuery->key,selectAuxFS->value,0);
								free(selectAuxFS);
							}
						}else if(selectAuxMT != NULL)
						{
							querySelect->value = strdup(selectAuxMT->value);
							loggearRegistroEncontrado(args->unaQuery->key,selectAuxMT->value,0);
						}else
						{
							querySelect->value = NULL;
							loggearRegistroNoEncontrado(args->unaQuery->key,0);
						}
						enviarQuery(socketCliente,querySelect);
						break;

					case CREATE:
						retornoCreate = rutinaFileSystemCreate(args);
						if(retornoCreate != -1)
						{
						procesarCreate(args->unaQuery,0);
						enviarRequest(socketCliente,1);
						}
						else
						{
							loggearErrorTablaExistente(args->unaQuery,0);
							enviarRequest(socketCliente,0);
						}
						break;
					case DROP:
							if(rutinaFileSystemDrop(args->unaQuery->tabla) == 1)
							{
								loggearTablaDropeadaOK(args->unaQuery->tabla,0);
								dictionary_remove(memTable,args->unaQuery->tabla);
								enviarRequest(socketCliente,1);
							}
							else{
							loggearErrorDrop(args->unaQuery->tabla,0);
							enviarRequest(socketCliente,0);
							}
							break;
					default:
						break;
				}
}

void consola()
{
	 int aux;
	 char * linea;
	 query * myQuery = malloc(100);
	 argumentosQuery * args;
	 logMemTable = retornarLogConPath("Memtable.log","Memtable");
	 system("clear");
	  while(1) {
	    linea = readline(">");
	    if (!linea)
	      break;

	    if (!strncasecmp(linea,"exit",4)) {
	      free(linea);
	      break;
	    }

	    add_history(linea);

	    if(!strncasecmp(linea,"imprimir",5))
	    {
	    	free(linea);
	    	imprimirMemTable(memTable);
	    }else if (!strncasecmp(linea,"clear",5)) {
	      free(linea);
	      system("clear");
	    }else{
	    aux = parsear(linea,&myQuery);
	    args = malloc(sizeof(argumentosQuery));
		args->unaQuery = myQuery;
		args->flag = 1;

		if(aux != -1)
		{
				procesarQuery(args);
		}
		else
		{
	    	printf("Request no valida\n");
	    }
	    free(args);
	    free(linea);
	    //free(myQuery);

	  }
	  }
	  exit(1);
}

void iniciarServidor()
{
	crearConfig("/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones/configuracion.txt");

	char * IP;

	if(obtenerString("DIRECCION_IP"))
	{
			IP = strdup(obtenerString("DIRECCION_IP"));
	}else
	{
			IP = strdup("127.0.0.1");
	}

	char * Puerto = strdup(obtenerString("PUERTO_ESCUCHA"));

	loggearInfoServidor(IP,Puerto);

	levantarServidorLFS(IP,Puerto);

	free(IP);
	free(Puerto);

	eliminarEstructuraConfig();
}

void temp()
{
	void imprimirTabla(char * key, void * asd)
			{
				printf("%s\n",key);
			}
			dictionary_iterator(memTable,imprimirTabla);
}

void procesarQuery(argumentosQuery * args)
{
	int flagConsola = args->flag;
	int retornoCreate;
	char * cadenaDescribe;
	logMemTable = retornarLogConPath("Memtable.log","Memtable");
	registro* selectAuxFS, * selectAuxMT;
	switch(args->unaQuery->requestType)
	{
	case DESCRIBE:
		cadenaDescribe = rutinaFileSystemDescribe(args->unaQuery->tabla);
		if(cadenaDescribe != NULL)
		{
		loggearInfo(cadenaDescribe);
		if(flagConsola)
			printf("%s",cadenaDescribe);
		free(cadenaDescribe);
		}else
		{
			if(args->unaQuery->tabla == NULL)
			{
		loggearInfo("Error, no hay tablas en el sistema");
		if(flagConsola)
			printf("Error, no hay tablas en el sistema\n");
			}
			else
			{
		loggearInfo("Error, Tabla no existente en el sistema");
		if(flagConsola)
			printf("Error, Tabla no existente en el sistema \n");
			}
		}
		break;
	case SELECT:
		selectAuxMT = procesarSelectMemTable(args->unaQuery);
		selectAuxFS = rutinaFileSystemSelect(args->unaQuery->tabla, args->unaQuery->key);
		if(selectAuxFS != NULL)
		{
			if(selectAuxMT != NULL)
			{
				if(selectAuxMT->timestamp > selectAuxFS->timestamp)
				{
					loggearRegistroEncontrado(args->unaQuery->key,selectAuxMT->value,flagConsola);
				}else
				{
				loggearRegistroEncontrado(args->unaQuery->key,selectAuxFS->value,flagConsola);
				free(selectAuxFS);
				}
			}
			else
			{
				loggearRegistroEncontrado(args->unaQuery->key,selectAuxFS->value,flagConsola);
				free(selectAuxFS);
			}
		}else if(selectAuxMT != NULL)
		{
		loggearRegistroEncontrado(args->unaQuery->key,selectAuxMT->value,flagConsola);
		}else
		{
			loggearRegistroNoEncontrado(args->unaQuery->key,flagConsola);
		}
		free(args->unaQuery->tabla);
		break;
	case INSERT:
		if(strlen(args->unaQuery->value) <= maxValue)
		{
		procesarInsert(args->unaQuery,flagConsola);
		}
		else
		{
			loggearWarning("Error: Valor maximo del value superado");
			if(flagConsola)
		printf("Error: Valor maximo del value superado\n");
		}
		break;
	case CREATE:
		retornoCreate = rutinaFileSystemCreate(args);
		if(retornoCreate != -1)
		{
		procesarCreate(args->unaQuery,flagConsola);
		}
		else
		{
			loggearErrorTablaExistente(args->unaQuery,flagConsola);
		}
		break;
	case DROP:
		if(rutinaFileSystemDrop(args->unaQuery->tabla) == 1)
		{
			loggearTablaDropeadaOK(args->unaQuery->tabla,args->flag);
			dictionary_remove(memTable,args->unaQuery->tabla);
		}
		else{
		loggearErrorDrop(args->unaQuery->tabla,args->flag);
		}
		free(args->unaQuery->tabla);
		break;
	default:
		loggearWarningEnLog(logMemTable,"Request aun no disponible");
	}
}

void procesarInsert(query * unaQuery, int flagConsola)
{

	agregarUnRegistroMemTable(unaQuery,flagConsola);
}


void agregarUnRegistroMemTable(query * unaQuery, int flagConsola)
{

	agregarAMemTable(memTable,unaQuery,flagConsola);

}

registro * procesarSelectMemTable(query* unaQuery)
{

	t_list * temp;
	void * listaRegistros;
	registro * registroFinal;

	if(dictionary_has_key(memTable,unaQuery->tabla))
	{
		temp = (t_list *)dictionary_get(memTable,unaQuery->tabla);

		if(temp != NULL)
		{
			if(!list_is_empty(temp))
			{

		bool condicion(void * elementoLista)
		{
			return ((registro*)elementoLista)->key == unaQuery->key;
		}

		listaRegistros = (t_list*)list_filter(temp,(void*)condicion);


		if(listaRegistros)
			{
			registroFinal = list_get(listaRegistros,0);

			if(list_size(listaRegistros) == 1)
				{
					return registroFinal;
				}

		void mayorTimestamp(registro * unRegistro)
				{
			if(unRegistro->timestamp > registroFinal->timestamp)
					{
				registroFinal = unRegistro;
					}
				}

		list_iterate(listaRegistros,(void*)mayorTimestamp);

				}
		list_destroy(listaRegistros);
		return registroFinal;
			}
		}
	}
	return NULL;

}
void agregarAMemTable(t_dictionary * memTable, query * unaQuery, int flagConsola)
{
	t_list * temp;
	registro * reg = castearRegistroQuery(unaQuery);

	if(dictionary_has_key(memTable,unaQuery->tabla))
	{
		temp = dictionary_get(memTable,unaQuery->tabla);
		if(temp != NULL)
		{
			if(!list_is_empty(temp))
			{
			temp = (t_list *)dictionary_remove(memTable,unaQuery->tabla);
			}
		}else{
			dictionary_remove(memTable,unaQuery->tabla);
			temp = list_create();
		}
		agregarRegistro(temp,reg);
		dictionary_remove(memTable,unaQuery->tabla);
		dictionary_put(memTable,unaQuery->tabla,temp);
		loggearInfoEnLog(logMemTable,"Se inserto un registro correctamente");
		if(flagConsola)
			printf("Se inserto un registro correctamente\n");
	}
	else
	{
		errorTablaNoCreada(unaQuery->tabla);
		if(flagConsola)
		{
			printf("Error: Tabla no creada previamente\n");
			//printf("Se inserto un registro correctamente\n");
		}
	}

}

void procesarCreate(query * unaQuery, int flagConsola)
{
	dictionary_put(memTable,unaQuery->tabla,NULL);
	loggearTablaCreadaOK(logMemTable,unaQuery,flagConsola,0);
}

void liberarMemTable(t_dictionary ** memTable)
{
	dictionary_clean_and_destroy_elements(*memTable,free);
}

registro * crearRegistro(int32_t key,char * value,int64_t timestamp)
{

	registro * unRegistro = malloc(sizeof(registro) + strlen(value) + 1);

	unRegistro->key = key;
	unRegistro->value = strdup(value);
	unRegistro->timestamp = timestamp;

	return unRegistro;
}


void imprimirListaRegistros(t_list * unaLista)
{
	if(unaLista)
	{
		if(!list_is_empty(unaLista))
		{
	void imprimirRegistro(void * valor)
		{

			printf("-> {%d %s %lli}\n",((registro*)valor)->key,((registro*)valor)->value,((registro*)valor)->timestamp);
		}
	list_iterate(unaLista,(void*)imprimirRegistro);
		}
	}
}

void agregarRegistro(t_list * unaLista, registro* unRegistro)
{
	list_add(unaLista,(void*)unRegistro);
}

void agregarListaRegistros(t_list * lista,t_list * listaAgregar)
{
	list_add_all(lista,listaAgregar);
}

registro * obtenerRegistro(t_list * lista, int posicionLista)
{
	return (registro*)list_get(lista,posicionLista);
}


registro * castearRegistroQuery(query * unaQuery)
{
	return crearRegistro(unaQuery->key,unaQuery->value,unaQuery->timestamp);

}

char * castearRegistroString(registro * unRegistro)
{
	int longitud = strlen((char*)unRegistro->value) + 1;

	char * registroFinal = malloc(40 + 40 + longitud + 2);

	char * auxK = malloc(40);
	char * auxV = malloc(longitud);
	char * auxD = malloc(40);

	sprintf(auxK,"%d",unRegistro->key);
	sprintf(auxD,"%lli",unRegistro->timestamp);
	strcpy(auxV,unRegistro->value);

	strcpy(registroFinal,auxK);
	strcat(registroFinal," ");
	strcat(registroFinal,auxV);
	strcat(registroFinal," ");
	strcat(registroFinal,auxD);

	free(auxK);
	free(auxV);
	free(auxD);

	return registroFinal;
}


void errorTablaNoCreada(char * tabla)
{
		char * error = malloc(strlen(tabla) + 1 + 100);

		strcpy(error,"Tabla: '");
		strcat(error,tabla);
		strcat(error,"' no creada previamente");

		loggearErrorEnLog(logMemTable,error);
		free(error);
}

query * crearQuery(int32_t tipoRequest, char * nombreTabla, int32_t key, char * value, int64_t timestamp)
{
	query * unaQuery = malloc(sizeof(query));

	unaQuery->requestType = tipoRequest;
	unaQuery->tabla = strdup(nombreTabla);
	unaQuery->key = key;

	if(tipoRequest == INSERT)
	{
	unaQuery->value = strdup(value);
	unaQuery->timestamp = timestamp;
	}

	return unaQuery;
}

void imprimirMemTable(t_dictionary * memTable)
{
	void imprimirLista(char * nombreTabla, void * registros)
	{
		printf("Tabla: %s \n", nombreTabla);
		imprimirListaRegistros((t_list*)registros);
	}
	dictionary_iterator(memTable,(void*)imprimirLista);
}

void loggearMemTable(t_dictionary * memTable)
{
	loggearInfoEnLog(logMemTable,"Comenzando la lectura de la MemTable...");

	void loggearTabla(char * nombreTabla, void * registros)
	{
		char * nomT = strdup(nombreTabla);
		char * aux = malloc(strlen("Logs de tabla: '") + strlen(nomT) + 2);
		strcpy(aux,"Logs de tabla: '");
		strcat(aux,nomT);
		strcat(aux,"'");
		loggearInfoEnLog(logMemTable,aux);

		loggearListaRegistros((t_list*)registros);

		free(nomT);
		free(aux);
	}
	dictionary_iterator(memTable,(void*)loggearTabla);
}



void loggearListaRegistros(t_list * unaLista)
{

	void loggearRegistro(void * valor)
	{

	char * aux = strdup(castearRegistroString((registro*)valor));
	char * mensajeALoggear = malloc(strlen(aux) + strlen("Se logea el registro: {") + 2);

	strcpy(mensajeALoggear,"Se logea el registro: {");
	strcat(mensajeALoggear,aux);
	strcat(mensajeALoggear,"}");

	loggearInfoEnLog(logMemTable,mensajeALoggear);

	free(aux);
	free(mensajeALoggear);
	}
		list_iterate(unaLista,(void*)loggearRegistro);

}

void loggearSelectMemT(query* unaQuery)
{
		char * key = malloc(50);
		char * req = malloc(strlen("Se recibio: {SELECT } ") + strlen(unaQuery->tabla) + 2);

		strcpy(req,"Se recibio: {SELECT ");
		strcat(req,unaQuery->tabla);
		strcat(req," ");
		sprintf(key,"%d",unaQuery->key);
		strcat(req,key);
		strcat(req,"}");

		loggearInfoEnLog(logMemTable,req);

		free(key);
		free(req);
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

void loggearTablaNoEncontrado(char * tabla, int flagConsola)
{
	char * aux = malloc(strlen("No existe la tabla ''") + strlen(tabla) + 5);
	strcpy(aux,"No existe la tabla '");
	strcat(aux,tabla);
	strcat(aux,"'");
	loggearInfoEnLog(logMemTable,aux);
	if(flagConsola)
	printf("%s\n",aux);
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


void loggearNuevaConexion(int socket)
{
	  char * info = malloc(strlen("Nueva conexion asignada al socket: ") + 10 + 4);
	  char * aux = malloc(10);

	  strcpy(info,"Nueva conexion asignada al socket: ");
	  sprintf(aux,"%d",socket);
	  strcat(info,aux);
	  strcat(info,"\n");

	  loggearInfo(info);

	  free(info);
      free(aux);
}
void loggearDatosRecibidos(int socket, int datosRecibidos)
{
		  char * info = malloc(strlen("Se recibieron  bytes del socket  ") + 30 + 5);
		  char * aux = malloc(30);

		  strcpy(info,"Se recibieron  ");
		  sprintf(aux,"%d",datosRecibidos);
		  strcat(info," bytes del socket ");
		  sprintf(aux,"%d",socket);
		  strcat(info,aux);
		  strcat(info,"\n");

		  loggearInfo(info);

		  free(info);
	      free(aux);
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


void liberarInsert(query * unQuery)
{
	free(unQuery->tabla);
	free(unQuery->value);
	free(unQuery);
}

void loggearTablaCreadaOK(t_log * loggeador,query * unaQuery,int flagConsola, int flagFS){

		char * aux = malloc(strlen("Se creo correctamente la tabla  ''") + strlen(unaQuery->tabla) + 30);
		strcpy(aux,"Se creo correctamente la tabla '");
		strcat(aux,unaQuery->tabla);
		strcat(aux,"'");
		if(flagFS)
			strcat(aux," en el FileSystem");
		else
			strcat(aux," en la MemTable");

		loggearInfoEnLog(loggeador,aux);
		if(flagConsola)
		printf("%s\n",aux);
		free(aux);

}
void loggearErrorTablaExistente(query * unaQuery,int flagConsola)
{
	char * aux = malloc(strlen("Error la tabla  '' ya existe en el sistema") + strlen(unaQuery->tabla) + 10);
	strcpy(aux,"Error la tabla '");
	strcat(aux,unaQuery->tabla);
	strcat(aux,"' ya existe en el sistema");
	loggearInfoEnLog(logMemTable,aux);
	if(flagConsola)
	printf("%s\n",aux);
	free(aux);
}

//me falta testear esto:
int obtenerTamanioRegistrosDeUnaLista(t_list * registros){

	int tamanio = 0;
	char * key = malloc(500), * tmp = malloc(500);

	void tamanioRegistro(registro *unRegistro)
	{
		sprintf(key,"%d",unRegistro->key);
		sprintf(tmp,"%lli",unRegistro->timestamp);

		tamanio += strlen(unRegistro->value) + strlen(key) + strlen(tmp) + strlen(";;\n");
		//15;//tam registro = value + 4 de key + 8 de timestamp + 3 de ; ; \n
	}
	list_iterate(registros,(void*)tamanioRegistro);
	free(key);
	free(tmp);
	return tamanio;
}

void terminarHilo(pthread_t * unHilo)
{
	//pthread_cancel(unHilo);
	pthread_kill(*unHilo,SIGTERM);
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
