#include "Lissandra.h"

void iniciarLFS()
{
	remove("Lissandra.log");
	iniciarLog("FileSystem");
	inicializarSemaforos();

	memTable = dictionary_create();
	memTableBackUp = dictionary_create();

	/*
	 * 2 Hilos iniciales para LFS:
	 * 1° -> Valida y gestiona la consola (Puede recibir Querys)
	 * 2° -> Gestiona el servidor (Puede recibir Querys)
	 * Luego se crean hilos on demand para gestionar requests
	 */

	//hiloConsola = crearHilo(consola,NULL);
	hiloServidor = crearHilo(iniciarServidor,NULL);

	//esperarHilo(hiloConsola);
	esperarHilo(hiloServidor);

}


void inicializarSemaforos()
{
	pthread_mutex_init(&mutex_Mem_Table,NULL);
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
		  char * info = malloc(strlen("Se recibieron  bytes del socket  ") + 30 + 5);
		  char * aux = malloc(30);

		  strcpy(info,"Se recibieron  ");
		  snprintf(aux,30,"%d",datosRecibidos);
		  strcat(info," bytes del socket ");
		  snprintf(aux,30,"%d",socket);
		  strcat(info,aux);
		  strcat(info,"\n");

		  loggearInfo(info);

		  free(info);
	      free(aux);
}

void levantarServidorLFS(char * servidorIP, char* servidorPuerto)
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
	                    	datosRecibidos = recibirQuery(i,myQuery);
	                    	if(datosRecibidos==0)
	                    	{
	                    		close(i);
	                    		EliminarDeSet(i,&sockets);
	                    	}else  {
	                    		loggearDatosRecibidos(i,datosRecibidos);
	                    	}

	                    }
	        		}
	        	}
			}
}


void consola()
{
	 char * linea;
	  while(1) {
	    linea = readline(">");
	    if (!linea)
	      break;

	    if (strncmp(linea,"exit",4) == 0) {
	      free(linea);
	      break;
	    }

	    add_history(linea);

	    if (strncmp(linea,"clear",5) == 0) {
	      system("clear");
	    }
	    else{
	    printf("%s\n", linea);
	    }

	    free(linea);
	  }
	  exit(1);
}

void iniciarServidor()
{

	crearConfig("../configuraciones/configuracion.txt"); //Para correrlo en eclipse: "configuraciones/configuracion.txt"

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


void procesarQuery(query * unaQuery)
{
	switch(unaQuery->requestType)
	{
	case SELECT:
		procesarSelect(unaQuery);
		break;
	case INSERT:
		procesarInsert(unaQuery);
		break;
	default:
		loggearWarning("Request aun no disponible");
	}
}

void procesarInsert(query * unaQuery)
{

	if(unaQuery->timestamp == -1)
	{
		unaQuery->timestamp = (double)time(NULL);//ObtenerTimeStamp(); -> VALIDAR ESTO!
	}

	agregarUnRegistroMemTable(unaQuery);
}


void agregarUnRegistroMemTable(query * unaQuery)
{
	pthread_mutex_lock(&mutex_Mem_Table);
	agregarAMemTable(memTable,unaQuery);
	pthread_mutex_unlock(&mutex_Mem_Table);
}


void procesarSelect(query* unaQuery)
{

	/*
	 * Para buscar:
	 * 1° -> Buscar nombre Tabla
	 * 2° -> Buscar Key
	 *
	 * Aca hay que buscar en todos los lugares donde puede estar:
	 *
	 * 1° -> La memtable
	 * 2° -> los .bin
	 * 3° -> los .tmp
	 * 4° -> los .tmpc
	 *
	 * --> Quedarse con el registro que posea el timestamp mayor
	 */
}
void agregarAMemTable(t_dictionary * memTable, query * unaQuery)
{
	t_list * temp;
	registro * reg = castearRegistroQuery(unaQuery);

	if(dictionary_has_key(memTable,unaQuery->tabla))
	{
		temp = (t_list *)dictionary_remove(memTable,unaQuery->tabla);

		if(list_size(temp) == 0 || temp == NULL)
		{
			temp = list_create();
		}
		agregarRegistro(temp,reg);
		dictionary_put(memTable,unaQuery->tabla,temp);
		loggearInfo("Registro insertado correctamente");

	}
	else
	{
		temp = list_create();
		agregarRegistro(temp,reg);
		dictionary_put(memTable,unaQuery->tabla,temp);

		warningTablaNoCreada(unaQuery->tabla);
		loggearInfo("Registro insertado correctamente");

	}

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
			printf("{%d %s %lli}\n",((registro*)valor)->key,((registro*)valor)->value,((registro*)valor)->timestamp);
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

	snprintf(auxK,40,"%d",unRegistro->key);
	snprintf(auxD,40,"%lli",unRegistro->timestamp);
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

void warningTablaNoCreada(char * tabla)
{
		char * wrg = malloc(strlen(tabla) + 1 + 100);

		strcpy(wrg,"Tabla: ");
		strcat(wrg,"'");
		strcat(wrg,tabla);
		strcat(wrg,"'");
		strcat(wrg," no creada previamente");

		loggearWarning(wrg);
		free(wrg);
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
	loggearInfo("Comenzando el log de la MemTable...");

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


query * crearInsert(char * nombreTabla,int32_t key,char * value,int64_t timestamp)
{
	char * aux = malloc(strlen(value) + 3);

	strcpy(aux,"'");
	strcat(aux,value);
	strcat(aux,"'");

	query * unaQuery = malloc(sizeof(query) + strlen(aux) + strlen(nombreTabla));

	unaQuery->tabla = strdup(nombreTabla);
	unaQuery->key = key;
	unaQuery->value = strdup(aux);
	unaQuery->timestamp = timestamp;

	return unaQuery;
}

void liberarInsert(query * unQuery)
{
	free(unQuery->tabla);
	free(unQuery->value);
	free(unQuery);
}



