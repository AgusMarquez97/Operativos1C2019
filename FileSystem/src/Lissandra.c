#include "Lissandra.h"


void iniciarLFS()
{
	remove("Memtable.log");
	remove("Lissandra.log");

	iniciarLog("FileSystem");

	levantarConfig();

	memTable = dictionary_create();


	/*
	 * 2 Hilos iniciales para LFS:
	 * 1° -> Valida y gestiona la consola (Puede recibir Querys)
	 * 2° -> Gestiona el servidor (Puede recibir Querys)
	 * Luego se crean hilos on demand para gestionar requests
	 */

	fileSystem = crearHilo(gestionarFileSystem,NULL);
	esperarHilo(fileSystem); // Crea las estructuras administrativas

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

	puntoMontaje = strdup(eliminarComillas(obtenerString("PUNTO_MONTAJE")));

	hiloMonintor = crearHilo(monitorearConfig,NULL); //Va a ser hilo detacheable
	//Aca voy a querer agregar el hilo a la lista

	eliminarEstructuraConfig();
}

void actualizarConfig()
{
	actualizarVariablesGlobales();
	reenviarConfig();
}

void actualizarVariablesGlobales()
{
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

	const char * pathConfig = "/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/configuraciones/configuracion.txt";

	int bytes_leidos;
    struct inotify_event * evento = malloc(sizeof(*evento));


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

	    while(1)
	    {

	        bytes_leidos = read(archivo_monitor,evento,sizeof(struct inotify_event)*10);
	        //Bloquea al proceso/hilo hasta que ocurra el evento declarado en el monitor

	        if(bytes_leidos <= 0)
	        {
	            perror("Error al leer el archivo monitor..");
	            break;
	        }

	        if(evento->mask == IN_MODIFY)
	        {
	        	actualizarConfig();
	        }


	    }

}


void levantarServidorLFS(char * servidorIP, char* servidorPuerto)
{
			argumentosQuery * args = NULL;
			query * myQuery = NULL;
	  	  	int socketRespuesta, maximoSocket;
	  	  	int datosRecibidos = -1;
			fd_set sockets, clientes;

	        int socketServidor = levantarServidor(servidorIP,servidorPuerto);

	        LimpiarSet(&sockets);
	        LimpiarSet(&clientes);

	        agregarASet(socketServidor,&sockets);
	        maximoSocket = socketServidor;

	        tiempoEspera esperaMaxima;
	        definirEsperaServidor(&esperaMaxima,30);

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
	                    		loggearMemTable(memTable);
	                    	}else  {

	                    		//crearHiloDetachable(procesarQuery,myQuery);
	                    		args = malloc(sizeof(argumentosQuery));
	                    		args->unaQuery = malloc(sizeof(query)); // Ver de eliminar
	                    		args->unaQuery = myQuery;
	                    		args->flag = 0;
	                    		pthread_t h1 = crearHilo(procesarQuery,(void*)args);
	                    		esperarHilo(h1);
	                    		free(args);
	                    		//free(myQuery)-> rompe como arbolito de navidad
	                    		//loggearDatosRecibidos(i,datosRecibidos);
	                    	}


	                    }
	        		}
	        	}
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
	      break;
	    }

	    add_history(linea);

	    if(!strncasecmp(linea,"imprimir",5))
	    {
	    	imprimirMemTable(memTable);
	    }else if (!strncasecmp(linea,"clear",5)) {
	      system("clear");
	    }else{
	    aux = parsear(linea,&myQuery);
	    args = malloc(sizeof(argumentosQuery));
		args->unaQuery = malloc(sizeof(query));
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

void procesarQuery(argumentosQuery * args)
{
	int flagConsola = args->flag;

	logMemTable = retornarLogConPath("Memtable.log","Memtable");

	switch(args->unaQuery->requestType)
	{
	case SELECT:
		procesarSelect(args->unaQuery,flagConsola);
		break;
	case INSERT:
		procesarInsert(args->unaQuery,flagConsola);
		break;
	case CREATE:
		procesarCreate(args->unaQuery,flagConsola);
		pthread_t  fsCreate = crearHilo(rutinaFileSystemCreate,args);
		esperarHilo(fsCreate);
		break;
	default:
		loggearWarningEnLog(logMemTable,"Request aun no disponible");
	}
}

void procesarInsert(query * unaQuery, int flagConsola)
{

	if(unaQuery->timestamp == -1)
	{
		unaQuery->timestamp = (int64_t) (time(NULL))/1000;//Finalmente se agrega el epoch de linux
	}

	agregarUnRegistroMemTable(unaQuery,flagConsola);
}


void agregarUnRegistroMemTable(query * unaQuery, int flagConsola)
{

	agregarAMemTable(memTable,unaQuery,flagConsola);

}

void procesarSelect(query* unaQuery, int flagConsola)
{
	loggearSelectMemT(unaQuery);
	t_list * temp;
	void * listaRegistros;
	registro * registroFinal = malloc(sizeof(registro));

	if(dictionary_has_key(memTable,unaQuery->tabla))
	{
		temp = (t_list *)dictionary_get(memTable,unaQuery->tabla);

		bool condicion(void * elementoLista)
		{
			return ((registro*)elementoLista)->key == unaQuery->key;
		}

		listaRegistros = (t_list*)list_filter(temp,(void*)condicion);


		if(listaRegistros)
		{
		registroFinal->timestamp = 0;

		void mayorTimestamp(registro * unRegistro)
		{
			if(unRegistro->timestamp > registroFinal->timestamp)
			{
				registroFinal = unRegistro;
			}
		}

		list_iterate(listaRegistros,(void*)mayorTimestamp);

		if(registroFinal)
		loggearRegistroEncontrado(((registro*)registroFinal)->value,flagConsola);

		}else{
		loggearInfoEnLog(logMemTable,"No se encontro la key buscada");
		if(flagConsola)
		printf("No se encontro la key\n");
		}
	}	else
	{
		loggearRegistroNoEncontrado(unaQuery->tabla,flagConsola);
	}
	free(registroFinal);
}
void agregarAMemTable(t_dictionary * memTable, query * unaQuery, int flagConsola)
{
	t_list * temp;
	registro * reg = castearRegistroQuery(unaQuery);

	if(dictionary_has_key(memTable,unaQuery->tabla))
	{
		if(dictionary_get(memTable,unaQuery->tabla) != NULL)
		{
			temp = (t_list *)dictionary_remove(memTable,unaQuery->tabla);
		}else{
			dictionary_remove(memTable,unaQuery->tabla);
			temp = list_create();
		}
		agregarRegistro(temp,reg);
		dictionary_put(memTable,unaQuery->tabla,temp);
		loggearInfoEnLog(logMemTable,"Se inserto un registro correctamente");
		if(flagConsola)
			printf("Se inserto un registro correctamente\n");
	}
	else
	{
		/*
		 *
		 temp = list_create();
		agregarRegistro(temp,reg);
		dictionary_put(memTable,unaQuery->tabla,temp);
		loggearInfoEnLog(logMemTable,"Se inserto un registro correctamente");
		 */
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
	if(dictionary_has_key(memTable,unaQuery->tabla))
		{
		loggearErrorTablaExistente(unaQuery,flagConsola);
		}
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

void liberarRegistro(registro * unRegistro)
{
	free(unRegistro->value);
	free(unRegistro);
}


void imprimirListaRegistros(t_list * unaLista)
{
	void imprimirRegistro(void * valor)
		{

			printf("-> {%d %s %lli}\n",((registro*)valor)->key,((registro*)valor)->value,((registro*)valor)->timestamp);
		}
	list_iterate(unaLista,(void*)imprimirRegistro);
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

void loggearRegistroEncontrado(char * value, int flagConsola)
{
	char * aux = malloc(strlen("Se encontro el registro buscado: ''") + strlen(value) + 2);

	strcpy(aux,"Se encontro el valor buscado: '");
	strcat(aux,value);
	strcat(aux,"'");

	loggearInfoEnLog(logMemTable,aux);

	if(flagConsola)
	printf("%s\n",aux);

	free(aux);
}

void loggearRegistroNoEncontrado(char * tabla, int flagConsola)
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
int obtenerTamanioRegistrosDeUnaTabla(t_dictionary * memTable, char * tabla){
	int tamanio = 0;
	t_list * registros =(t_list *) dictionary_get(memTable, tabla);
	void tamanioRegistro(registro *unRegistro){
		tamanio += strlen(unRegistro->value) + 1 + sizeof(int32_t) + sizeof(int64_t) + strlen(";;\n");
		//15;//tam registro = value + 4 de key + 8 de timestamp + 3 de ; ; \n
	}
	list_iterate(registros,(void*)tamanioRegistro);
	return tamanio;
}

void terminarHilo(pthread_t * unHilo)
{
	//pthread_cancel(unHilo);
	pthread_kill(*unHilo,SIGTERM);
}

