#include "Lissandra.h"


void iniciarLFS()
{

	remove("Memtable.log");
	remove("Lissandra.log");

	system("clear");

	memTable = dictionary_create();

	rutaConfig = "/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones/configuracion.txt";

	iniciarLog("FileSystem");

	levantarConfig();

	//handshake();

	gestionarFileSystem();

	levantarMemTable();

	hiloDump = makeDetachableThread(ejecutarDumping,NULL);

	hiloConsola = crearHilo(consola,NULL);
	hiloServidor = makeDetachableThread(iniciarServidor,NULL);

	esperarHilo(hiloConsola);

}

/*
 * LEVANTAR FS
 */


void levantarConfig()
{
	crearConfig(rutaConfig);

	maxValue =  obtenerInt("TAMAÑO_VALUE");
	retardo = obtenerInt("RETARDO"); //en milisegundos  -> puede modificarse (ver cuando y de que forma)
	dumping = obtenerInt("TIEMPO_DUMP"); //en milisegundos  -> puede modificarse (ver cuando y de que forma)

	if(obtenerString("DIRECCION_IP"))
		{
				IP = strdup(obtenerString("DIRECCION_IP"));
		}else
		{
				IP = strdup("127.0.0.1");
		}

		Puerto = strdup(obtenerString("PUERTO_ESCUCHA"));

	loggearInfoServidor(IP,Puerto);

	puntoMontaje = eliminarComillas(obtenerString("PUNTO_MONTAJE"));
	eliminarEstructuraConfig();
	hiloMonitor = makeDetachableThread(monitorearConfig,NULL); //Va a ser hilo detacheable
}

/*
void handshake()
{
	int socketRespuesta;
    int socketServidor = levantarServidor(IP,Puerto);

    int nroRecibido;

    if((socketRespuesta = aceptarConexion(socketServidor)) != -1)
	{
    		loggearInfo("Realizando el handshake");
    		recibirRequest(socketRespuesta,&nroRecibido);

    		if(nroRecibido == HANDSHAKE)
    		{
    			enviarInt(socketRespuesta,maxValue);
    			close(socketRespuesta);
    		}
    		else
    		{
    		loggearError("No se pudo realizar el handshake");
    		}

	}
    close(socketServidor);
}
*/

/*
 * LEVANTAR SERVIDOR
 */

void iniciarServidor()
{
	levantarServidorLFS(IP,Puerto);
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

	        		if((hiloAtendedor = makeDetachableThread(rutinaServidor,(void*)socketRespuesta)) != 0)
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

		args->unaQuery = myQuery;
		args->socketCliente = socketCliente;
		args->flag = 0;

		procesarQuery(args); // Se procesa la query sin necesidad de notificar el resultado -> JOURNALING !
			}

		}
	}

	close(socketCliente);

}




/*
 * LEVANTAR CONSOLA
 */


void consola()
{
	 int aux;
	 char * linea;
	 query * myQuery = NULL;
	 argumentosQuery * args;
	 system("clear");
	 puts("Bienvenido a la consola del Filesystem...");
	  while(1)
	  {
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
				args->socketCliente = -1;
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

				free(linea);
			}
	  }
}


/*
 * PROCESAR QUERY
 */

void procesarQuery(argumentosQuery * args)
{
	int flagConsola = args->flag;

	switch(args->unaQuery->requestType)
	{
	case SELECT:
		procesarSelect(args->unaQuery,flagConsola,args->socketCliente);
		free(args->unaQuery->tabla);
		break;
	case INSERT:
		pthread_mutex_lock(&mutex_memTable);
		procesarInsert(args->unaQuery,flagConsola);
		free(args->unaQuery->value);
		free(args->unaQuery->tabla);
		pthread_mutex_unlock(&mutex_memTable);
		break;
		break;
	case CREATE:
		procesarCreate(args->unaQuery,flagConsola,args->socketCliente);
		free(args->unaQuery->tabla);
		break;
	case DESCRIBE:
		pthread_mutex_lock(&mutex_describe_drop);
		procesarDescribe(args->unaQuery,flagConsola,args->socketCliente);
		//if(args->unaQuery->tabla)
		//	free(args->unaQuery->tabla);
		pthread_mutex_unlock(&mutex_describe_drop);
		break;
	case DROP:
		pthread_mutex_lock(&mutex_describe_drop);
		procesarDrop(args->unaQuery,flagConsola,args->socketCliente);
		free(args->unaQuery->tabla);
		pthread_mutex_unlock(&mutex_describe_drop);
		break;
	case HANDSHAKE:
		loggearInfo("Realizando el handshake");
		enviarInt(args->socketCliente,maxValue);
		break;
	default:
		loggearWarning("Se produjo un error inesperado");
	}

	free(args->unaQuery);
	free(args);
}



void procesarSelect(query * unaQuery, int flagConsola, int socketCliente)
{
			query * insert = malloc(sizeof(query));

			insert->requestType = INSERT;
			insert->tabla = strdup(unaQuery->tabla);
			insert->key = unaQuery->key;

			registro* selectAuxFS, * selectAuxMT;
			selectAuxMT = procesarSelectMemTable(unaQuery);
			selectAuxFS = rutinaFileSystemSelect(unaQuery->tabla, unaQuery->key);



			if(selectAuxFS != NULL)
			{
				if(selectAuxMT != NULL)
				{
					if(selectAuxMT->timestamp > selectAuxFS->timestamp)
					{
						insert->value = strdup(selectAuxMT->value);
						insert->timestamp = selectAuxMT->timestamp;
						loggearRegistroEncontrado(unaQuery->key,selectAuxMT->value,flagConsola);
					}else
					{
						insert->value = strdup(selectAuxFS->value);
						insert->timestamp = selectAuxFS->timestamp;
						loggearRegistroEncontrado(unaQuery->key,selectAuxFS->value,flagConsola);
					}
					free(selectAuxFS->value);
					free(selectAuxFS);
				}
				else
				{
					insert->value = strdup(selectAuxFS->value);
					insert->timestamp = selectAuxFS->timestamp;
					loggearRegistroEncontrado(unaQuery->key,selectAuxFS->value,flagConsola);
					free(selectAuxFS->value);
					free(selectAuxFS);
				}
			}else if(selectAuxMT != NULL)
			{
				insert->value = strdup(selectAuxMT->value);
				insert->timestamp = selectAuxMT->timestamp;
			loggearRegistroEncontrado(unaQuery->key,selectAuxMT->value,flagConsola);
			}else
			{
				insert->value = strdup("VOID");
				insert->timestamp = -1;
				loggearRegistroNoEncontrado(unaQuery->key,flagConsola);
			}
			if(socketCliente != -1)
			enviarQuery(socketCliente,insert);

			free(insert->tabla);
			free(insert->value);
			free(insert);
}



void procesarInsert(query * unaQuery, int flagConsola)
{

	if(strlen(unaQuery->value) <= maxValue)
		{
		agregarUnRegistroMemTable(unaQuery,flagConsola);
		}
	else
		{
		loggearWarning("Error: Valor maximo del value superado");
		if(flagConsola)
			printf("Error: Valor maximo del value superado\n");
		}
}


void procesarCreate(query * unaQuery, int flagConsola, int socketCliente)
{
			int retornoCreate;
			retornoCreate = rutinaFileSystemCreate(unaQuery,flagConsola);
			if(retornoCreate != -1)
			{
				dictionary_put(memTable,unaQuery->tabla,NULL);
				loggearTablaCreadaOK(unaQuery->tabla,flagConsola);
				if(socketCliente != -1)
					enviarInt(socketCliente,1);
			}
			else
			{
				loggearErrorTablaExistente(unaQuery,flagConsola);
				if(socketCliente != -1)
					enviarInt(socketCliente,0);
			}

}


void procesarDescribe(query * unaQuery, int flagConsola, int socketCliente)
{
		char * mensajeErrorTabla;
		query * cadenaDescribe = malloc(sizeof(query));
		cadenaDescribe->requestType = DESCRIBE;

		if(unaQuery->tabla)
		{
		cadenaDescribe->tabla = rutinaFileSystemDescribe(unaQuery->tabla);
		}
		else
		{
			cadenaDescribe->tabla = rutinaFileSystemDescribe(NULL);
		}

		if(cadenaDescribe->tabla != NULL)
		{
		loggearInfo(cadenaDescribe->tabla);
		if(flagConsola)
			printf("%s",cadenaDescribe->tabla);
		}else
		{
			if(unaQuery->tabla == NULL)
			{
		mensajeErrorTabla = strdup("Error, no hay tablas en el sistema");
		loggearInfo(mensajeErrorTabla);
		if(flagConsola)
			printf("%s\n",mensajeErrorTabla);
			}
			else
			{
		mensajeErrorTabla = strdup("Error, Tabla no existente en el sistema");
		loggearInfo(mensajeErrorTabla);
			if(flagConsola)
				printf("%s\n",mensajeErrorTabla);
			}
			cadenaDescribe->tabla = strdup("VOID");
		}

		if(socketCliente != -1)
			enviarQuery(socketCliente,cadenaDescribe);

		free(cadenaDescribe->tabla);
		free(cadenaDescribe);
}

void procesarDrop(query * unaQuery, int flagConsola, int socketCliente)
{
		if(rutinaFileSystemDrop(unaQuery->tabla) == 1)
			{
				loggearTablaDropeadaOK(unaQuery->tabla,flagConsola);
				dictionary_remove(memTable,unaQuery->tabla);
				if(socketCliente != -1)
					enviarInt(socketCliente,1);
			}
			else{
			loggearErrorDrop(unaQuery->tabla,flagConsola);
			if(socketCliente != -1)
					enviarInt(socketCliente,0);
			}
}



/*
 * FUNCIONES AUXILIARES
 */


registro * procesarSelectMemTable(query* unaQuery)
{

	t_list * temp;
	t_list * listaRegistros;
	registro * registroFinal = NULL;

	if(dictionary_has_key(memTable,unaQuery->tabla))
	{
		pthread_mutex_lock(&mutex_memTable);

		temp = (t_list *) dictionary_get(memTable,unaQuery->tabla);

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
							if(!list_is_empty(listaRegistros))
								{
									registroFinal = list_get(listaRegistros,0);

									if(list_size(listaRegistros) > 1)
									{
									void mayorTimestamp(registro * unRegistro)
											{
										if(unRegistro->timestamp > registroFinal->timestamp)
												{
											registroFinal = unRegistro;
												}
											}
									list_iterate(listaRegistros,(void*)mayorTimestamp);
									}
								}
							list_destroy(listaRegistros);
						}
						//list_destroy(temp); // VALIDAR
				}

		}
		pthread_mutex_unlock(&mutex_memTable);
	}
	return registroFinal;

}



void agregarUnRegistroMemTable(query * unaQuery, int flagConsola)
{
	char * nombreTabla = strdup(unaQuery->tabla);
	t_list * temp;
	registro * reg = castearRegistroQuery(unaQuery);

	if(dictionary_has_key(memTable,nombreTabla))
	{
		temp = dictionary_get(memTable,nombreTabla);
		if(temp != NULL)
		{
			if(!list_is_empty(temp))
			{
			temp = (t_list *)dictionary_remove(memTable,nombreTabla);
			}
		}else{
			dictionary_remove(memTable,nombreTabla);
			temp = list_create();
		}
		agregarRegistro(temp,reg);
		dictionary_remove(memTable,nombreTabla);
		dictionary_put(memTable,nombreTabla,temp);

		loggearInfo("Se inserto un registro correctamente");
		if(flagConsola)
			printf("Se inserto un registro correctamente\n");
	}
	else
	{
		loggearTablaNoEncontrada(nombreTabla,flagConsola);
	}

}


void agregarRegistro(t_list * unaLista, registro* unRegistro)
{
	list_add(unaLista,(void*)unRegistro);
}



registro * obtenerRegistro(t_list * lista, int posicionLista)
{
	return (registro*)list_get(lista,posicionLista);
}


registro * crearRegistro(int32_t key,char * value,int64_t timestamp)
{

	registro * unRegistro = malloc(sizeof(registro) + strlen(value) + 1);

	unRegistro->key = key;
	unRegistro->value = strdup(value);
	unRegistro->timestamp = timestamp;

	return unRegistro;
}





/*
 * LIBERAR MEMORIA
 */

void liberarRegistro(registro * unRegistro)
{
	if(unRegistro)
	{
		if(unRegistro->value)
			free(unRegistro->value);
	free(unRegistro);
	}
}

void liberarInsert(query * unQuery)
{
	free(unQuery->tabla);
	free(unQuery->value);
	free(unQuery);
}

void liberarMemTable(t_dictionary ** memTable)
{
	dictionary_clean_and_destroy_elements(*memTable,free);
}





/*
 * 	FUNCIONES AUXILIARES FS
 */


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


/*
 * FUNCIONES DE CASTEO
 */


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
















/*
 * FUNCIONES DE LOGGEAR
 */




//LOGGEAR SELECT

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

void loggearTablaNoEncontrada(char * tabla, int flagConsola)
{
	char * aux = malloc(strlen("No existe la tabla ''") + strlen(tabla) + 5);
	strcpy(aux,"No existe la tabla '");
	strcat(aux,tabla);
	strcat(aux,"'");
	loggearInfo(aux);
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


// LOGGEAR CREATE

void loggearTablaCreadaOK(char * tabla,int flagConsola){

		char * aux = malloc(strlen("Se creo correctamente la tabla  en el FileSystem ''") + strlen(tabla) + 35);
		strcpy(aux,"Se creo correctamente la tabla '");
		strcat(aux,tabla);
		strcat(aux,"'");
		strcat(aux," en el FileSystem");

		loggearInfo(aux);

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
	loggearInfo(aux);
	if(flagConsola)
	printf("%s\n",aux);
	free(aux);
}


// DROP

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







/*
 * LEVANTAR HILO MONITOR
 */
void monitorearConfig()
{

		const char * pathConfig = "/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones";
		char buffer[BUF_LEN];
		while(1)
		{
		int length = 0;
		int offset = 0;
		//struct inotify_event * evento = malloc(sizeof(*evento));

	    int file_descriptor = inotify_init();

		if(file_descriptor == -1)
		{
		 perror("No se pudo crear el archivo monitor");
		 exit(-1);
		}

		    int watch_descriptor = inotify_add_watch(file_descriptor,pathConfig,IN_MODIFY|IN_CREATE|IN_DELETE|IN_CLOSE_WRITE);//Ver mask

		    if(watch_descriptor == -1)
		    {
		        perror("No se pudo crear el monitor");
			exit(-1);
		    }

		    length = read(file_descriptor,buffer,BUF_LEN);
		    while(offset < length){
			struct inotify_event * evento = (struct inotify_event *) &buffer[offset];

		        if(length <= 0)
		        {
		            perror("Error al leer el archivo monitor.");exit(-1);
		            break;
		        }

		    if(evento->len) {
				 if ((evento->mask & IN_CLOSE_WRITE) && !strcasecmp(evento->name,"configuracion.txt"))
			    {
					 actualizarConfig();
					 loggearInfo("Variables de configuracion actualizadas");
			    }
		    }
			offset += sizeof (struct inotify_event) + evento->len;
		    }
		}
}



void actualizarConfig()
{

	t_config * configLocal = config_create(rutaConfig);

	retardo = config_get_int_value(configLocal,"RETARDO");
	dumping = config_get_int_value(configLocal,"TIEMPO_DUMP");

	config_destroy(configLocal);
}



















/*
 * FUNCIONES INNECESARIAS:
 */

void temp()
{
	void imprimirTabla(char * key, void * asd)
			{
				printf("%s\n",key);
			}
			dictionary_iterator(memTable,imprimirTabla);
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


void terminarHilo(pthread_t * unHilo)
{
	//pthread_cancel(unHilo);
	pthread_kill(*unHilo,SIGTERM);
}



void loggearMemTable(t_dictionary * memTable)
{
	loggearInfo("Comenzando la lectura de la MemTable...");

	void loggearTabla(char * nombreTabla, void * registros)
	{
		char * nomT = strdup(nombreTabla);
		char * aux = malloc(strlen("Logs de tabla: '") + strlen(nomT) + 2);

		strcpy(aux,"Logs de tabla: '");
		strcat(aux,nomT);
		strcat(aux,"'");
		loggearInfo(aux);

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

	loggearInfo(mensajeALoggear);

	free(aux);
	free(mensajeALoggear);
	}
		list_iterate(unaLista,(void*)loggearRegistro);

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

void agregarListaRegistros(t_list * lista,t_list * listaAgregar)
{
	list_add_all(lista,listaAgregar);
}


