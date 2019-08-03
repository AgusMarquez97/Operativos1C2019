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
	tamanioValue = 20;

	listaSegmentos = list_create();
	historialPaginas = list_create();
	tablaSegmentos = dictionary_create();

	remove("Lissandra.log");
	iniciarLog("Memoria");
	leerArchivoConfiguracion();
	//handshake();
	reservarMemoriaPrincipal();
	inicializarSemaforos();
	loggearInfoFileSystem(configuracionMemoria->IP_FS,configuracionMemoria->PUERTO_FS);

	hiloJournal = makeDetachableThread(ejecutarRutinaJournal,NULL);

	pthread_t hilo_consola;//, hilo_conexionKernel;


	//pthread_create(&hilo_conexionKernel, NULL, (void*) conexionKernel, NULL);
	pthread_create(&hilo_consola,NULL,(void *) consola,NULL);

	//hiloServidor = crearHilo(levantarServidorMemoria,NULL);


	//pthread_join(hilo_conexionKernel,NULL);
	pthread_join(hilo_consola,NULL);

	//esperarHilo(hiloServidor);

	free(configuracionMemoria);
	return 0;

}


/*
 * LEVANTAR MEMORIA
 */

void leerArchivoConfiguracion(){

		crearConfig(PATHCONFIG);
		configuracionMemoria = (configuracion*) malloc(sizeof(configuracion));

		configuracionMemoria->IP_FS = getCleanString(obtenerString("IP_FS"));
		configuracionMemoria->IP_SEEDS = obtenerArray("IP_SEEDS");
		configuracionMemoria->PUERTO_FS = obtenerString("PUERTO_FS");
		configuracionMemoria->PUERTO_SEEDS = obtenerArray("PUERTO_SEEDS");
		configuracionMemoria->PUERTO = obtenerString("PUERTO");
		configuracionMemoria->MEMORY_NUMBER = obtenerInt("MEMORY_NUMBER");
		configuracionMemoria->RETARDO_FS = obtenerInt("RETARDO_FS");
		configuracionMemoria->RETARDO_GOSSIPING = obtenerDouble("RETARDO_GOSSIPING");
		configuracionMemoria->RETARDO_JOURNAL = obtenerDouble("RETARDO_JOURNAL");
		configuracionMemoria->RETARDO_MEM = obtenerInt("RETARDO_MEM");
		configuracionMemoria->TAM_MEM = obtenerInt("TAM_MEM");

		hiloMonitor = makeDetachableThread(monitorearConfig,NULL); //Va a ser hilo detacheable

		//eliminarEstructuraConfig();

		//loggearArchivoDeConfiguracion();
}

void levantarMemoriaComoServidor()
{
	tamanioValue = 52;

	listaSegmentos = list_create();
	historialPaginas = list_create();
	tablaSegmentos = dictionary_create();

	//iniciarLog("Memoria"); -> PENSAR LOG

	leerArchivoConfiguracion();
	reservarMemoriaPrincipal();
	inicializarSemaforos();

	hiloJournal = makeDetachableThread(ejecutarRutinaJournal,NULL);

	levantarServidorMemoria();
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


void reservarMemoriaPrincipal(){
	// Reservamos la memoria
		tamanioPag=0;
	    tamanioMemoria = configuracionMemoria->TAM_MEM;
		memoriaPrincipal = (char*) malloc(tamanioMemoria);
		tamanioPag = sizeof(int32_t) + sizeof(int64_t) + tamanioValue;
		cantidadMarcos = obtenerCantidadMarcos(tamanioPag,tamanioMemoria);

		levantarMarcos(cantidadMarcos);
		char * tamanioPagChar = string_itoa(tamanioPag);

	// Rellenamos con ceros.
		memset(memoriaPrincipal, 0, tamanioMemoria * sizeof(char));

		char* tammem= malloc(7*sizeof(char));
		sprintf(tammem,"%d",tamanioMemoria);
		loggearInfoConcatenandoDosMensajes("MEMORIA RESERVADA: ", tammem);
		loggearInfoConcatenandoDosMensajes("TAMANIO PAG: ", tamanioPagChar);
		free(tamanioPagChar);
		free(tammem);

}


int obtenerCantidadMarcos(int tamanioPagina, int tamanioMemoria)
{
	float tam_nec = (float)tamanioMemoria/tamanioPagina;
	if(tam_nec == (int)tam_nec)
	return (int)tam_nec;
	return (int)round(tam_nec + 0.5);
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



void handshake()
{
	handshakeFS();
	handshakeKernel();
}

void handshakeFS()
{
	int datosRecibidos;
		int socketCliente = levantarCliente(configuracionMemoria->IP_FS,configuracionMemoria->PUERTO_FS);
		tamanioValue = -1;

		enviarInt(socketCliente,HANDSHAKE);

		loggearInfo("Iniciando HANDSHAKE");

		datosRecibidos = recibirInt(socketCliente,&tamanioValue);

		if(datosRecibidos > 0 && tamanioValue != -1)
		{
			loggearInfo("Handshake con FS exitoso");
		}
		else
		{
			loggearError("Error al realizar el handshake con FS");
		}

		close(socketCliente);
}


void handshakeKernel()
{

}




//-----------------------------------------------------------------------
//----------------------------CONEXIONES---------------------------------
//-----------------------------------------------------------------------

int conexionFS(){

		char* IP = strdup(configuracionMemoria->IP_FS);
		char * Puerto = strdup(configuracionMemoria->PUERTO_FS);

		int clienteFS = levantarCliente(IP,Puerto);

		free(IP);
		free(Puerto);

		return clienteFS;
}


void enviarConsistencia()
{
	char* IP = strdup(configuracionMemoria->IP_FS);
	char * Puerto = strdup(configuracionMemoria->PUERTO_FS);

	int clienteMemoria = levantarCliente(IP,Puerto);

	enviarRequest(clienteMemoria,RUN);

}


void levantarServidorMemoria()
{
			IPMemoria = eliminarComillasMemoria(configuracionMemoria->IP_SEEDS[configuracionMemoria->MEMORY_NUMBER]);
			puertoMemoria = configuracionMemoria->PUERTO;

			loggearInfoServidor(IPMemoria,puertoMemoria);

			int socketRespuesta;
			pthread_t hiloAtendedor = 0;
			int socketServidor = levantarServidor(IPMemoria,puertoMemoria);

			argumentosQuery * args = malloc(sizeof(argumentosQuery));
			args->unaQuery = NULL;
			args->flagConsola = 0;

			while(1)
			{
				if((socketRespuesta = aceptarConexion(socketServidor)) != -1)
				{
					args->socketCliente = socketRespuesta;
					loggearNuevaConexion(socketRespuesta);
					if(recibirQuery(socketRespuesta,&args->unaQuery) != -1 && recibirQuery(socketRespuesta,&args->unaQuery) != 0)
					{
						if((hiloAtendedor = makeDetachableThread(procesarQuery,(void*)args)) != 0)
						{

						}
						else
						{
						loggearError("Error al crear un hilo");
						}
					}
					else
					{
						loggearError("Query no valida recibida");
					}
				}
			}
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

	socketFS = conexionFS();

	if(socketFS != -1)
	{

		void enviarUnaQuery(query * unaQuery)
		{
			enviarQuery(socketFS,unaQuery);
			free(unaQuery->value);
		}

		list_iterate(querys,(void *)enviarUnaQuery);
		close(socketFS);
	}
	else
	{
			loggearInfo("No se pudo conectar con FS para realizar el JOURNAL");
	}
	list_destroy_and_destroy_elements(registros,free);
	list_destroy_and_destroy_elements(querys,free);

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
				pthread_mutex_lock(&mutex_journal);
				procesarSelect(args->unaQuery,flagConsola);
				pthread_mutex_unlock(&mutex_journal);
				free(args->unaQuery->tabla);
				break;
			case INSERT:
				pthread_mutex_lock(&mutex_journal);
				procesarInsert(args->unaQuery,flagConsola);
				pthread_mutex_unlock(&mutex_journal);
				free(args->unaQuery->tabla);
				free(args->unaQuery->value);
				break;
			case CREATE:
				procesarCreate(args->unaQuery,flagConsola);
				free(args->unaQuery->tabla);
				break;
			case DESCRIBE:
				procesarDescribe(args->unaQuery,flagConsola);
				if(args->unaQuery->tabla)
					free(args->unaQuery->tabla);
				break;
			case DROP:
				pthread_mutex_lock(&mutex_journal);
				procesarDrop(args->unaQuery,flagConsola);
				pthread_mutex_unlock(&mutex_journal);
				free(args->unaQuery->tabla);
				break;
			case JOURNAL:
				procesarJournal(args->unaQuery,flagConsola);
				break;
			case RUN:
				cambiarConsistencia();
				break;
			case HANDSHAKE:
				//enviarPool(args->socketCliente);
				break;
	default:
		loggearInfo("Request Aun No Disponible");
	}
	free(args->unaQuery);
	free(args);
}


void procesarSelect(query* selectQuery, int flagConsola)
{
	query * queryInsertFS;
	int key = selectQuery->key;
	int socketFS;
	int datosRecibidos;
	registro * registroFinalMemoria = NULL;

	if(criterioConsistencia == -1)
		printf("TABLA NO EXISTE EN FS");

		if(existeEnMemoria(selectQuery->tabla))
		{
			registroFinalMemoria = obtenerRegistro(selectQuery->tabla,key);
			if(registroFinalMemoria == NULL) // SI NO ESTA EN MP
			{
				buscarRegistroFS(selectQuery,flagConsola);
			}else if(criterioConsistencia != EC) // SI ESTA Y ES SC - SHC
			{
				socketFS = conexionFS();
				if(socketFS != -1)
				{
					enviarQuery(socketFS,selectQuery);
					datosRecibidos = recibirQuery(socketFS,&queryInsertFS);
					close(socketFS);
					if(datosRecibidos > 0)
					{
							if(registroFinalMemoria->timestamp >= queryInsertFS->timestamp) // MAX MP
							{
								loggearRegistroEncontrado(key,registroFinalMemoria->value,flagConsola);
								free(registroFinalMemoria->value);
								free(registroFinalMemoria);

								free(queryInsertFS->tabla);
								free(queryInsertFS->value);
								free(queryInsertFS);
							}
							else // MAX FS
							{
								buscarRegistroFS(selectQuery,flagConsola);
								free(registroFinalMemoria->value);
								free(registroFinalMemoria);
							}


					}
					else // MAX MP
							{
								loggearRegistroEncontrado(key,registroFinalMemoria->value,flagConsola);
								free(registroFinalMemoria->value);
								free(registroFinalMemoria);
							}
				}
				else // MAX MP
				{
					loggearRegistroEncontrado(key,registroFinalMemoria->value,flagConsola);
					free(registroFinalMemoria->value);
					free(registroFinalMemoria);
				}
			}
			else // SI ESTA Y ES EC
			{
				loggearRegistroEncontrado(key,registroFinalMemoria->value,flagConsola);
				free(registroFinalMemoria->value);
				free(registroFinalMemoria);
			}
		}
		else
		{
			buscarRegistroFS(selectQuery,flagConsola);
		}

}

void cambiarConsistencia()
{
	criterioConsistencia = SC;
}


void procesarInsert(query* queryInsert, int flagConsola)
{
	if(strlen(queryInsert->value) <= tamanioValue)
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
						agregarAHistorialPags(pagina);
						if(registroRetorno->key==key)
						{
						registroFinal = registroRetorno;
						nroMarco = pagina->nroMarco;
						}else{
						free(registroRetorno->value);
						free(registroRetorno);
						}

				}
				list_iterate(listaPaginas,(void*)buscarKey);
				if(registroFinal != NULL && nroMarco != -1)
				{
				memset(memoriaPrincipal + nroMarco*tamanioPag,0,tamanioPag);
				escribirMarco(nroMarco,registroFinal);
				free(registroFinal->value);
				free(registroFinal);
				}
				else
				{
				agregarPagina(queryInsert->tabla,queryInsert->key,queryInsert->value,queryInsert->timestamp,1);
				}

		} else
		{
			char * nombreTabla = strdup(queryInsert->tabla);
			dictionary_put(tablaSegmentos,nombreTabla,NULL);
			list_add(listaSegmentos,nombreTabla);
			agregarPagina(queryInsert->tabla,queryInsert->key,queryInsert->value,queryInsert->timestamp,1);
		}
		loggearInfo("Se inserto un registro correctamente");
			if(flagConsola)
			printf("Se inserto un registro correctamente\n");
	}
	else
	{
		loggearError("Error: Valor maximo del value superado");
		if(flagConsola)
				printf("Error: Valor maximo del value superado\n");
	}
}
void procesarCreate(query* queryCreate, int flagConsola)
{
	int estadoResultado;
	int socketFS;
	int datosRecibidos;
	socketFS = conexionFS();
	if(socketFS != -1)
	{
		enviarQuery(socketFS,queryCreate);
		datosRecibidos = recibirInt(socketFS,&estadoResultado);
		close(socketFS);
		if(datosRecibidos>0 && estadoResultado == 1)
		{
			loggearTablaCreadaOK(queryCreate,flagConsola);
		}else
		{
			loggearErrorTablaExistente(queryCreate,flagConsola);
		}
	}else{
		if(flagConsola)
					printf("No se pudo conectar con FS\n");
	loggearInfo("No se pudo conectar con FS");
	}


}

void procesarDescribe(query* queryDescribe, int flagConsola)
{
	int socketFS;
	int datosRecibidos;
	query * describe;

	socketFS = conexionFS();
	if(socketFS != -1)
	{
			enviarQuery(socketFS,queryDescribe);
			datosRecibidos = recibirQuery(socketFS,&describe);
			close(socketFS);
			if(datosRecibidos > 0 && strncmp(describe->tabla,"VOID",4))
			{
				loggearInfo(describe->tabla);
				if(flagConsola)
					printf("%s",describe->tabla);
				free(describe->tabla);
				free(describe);
			}else
			{
				if(queryDescribe->tabla)
				{
				loggearInfo("Tabla no existente en el sistema");
					if(flagConsola)
						printf("Tabla no existente en el sistema\n");
				}
				else
				{
				loggearInfo("No existen tablas en el FS");
					if(flagConsola)
						printf("No existen tablas en el FS\n");
				}
				if(datosRecibidos > 0)
				{
					free(describe->tabla);
					free(describe);
				}
			}
	}
	else{
			if(flagConsola)
						printf("No se pudo conectar con FS\n");
	loggearInfo("No se pudo conectar con FS");
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
			eliminarDeHistorial(unaPagina);
			liberarMarco(unaPagina->nroMarco);
		}

		list_iterate(paginas,(void*)liberarPagina);
		list_destroy_and_destroy_elements(paginas,free);

		bool condicionSegmentoExistente(char * segmento)
		{
			return !(strcmp(segmento,queryDrop->tabla));
		}

		list_remove_and_destroy_by_condition(listaSegmentos,(void*)condicionSegmentoExistente,free);


		loggearTablaDropeadaOK(queryDrop->tabla,flagConsola);
	}else
	{
		loggearErrorDrop(queryDrop->tabla,flagConsola);
	}

	int estadoResultado;
	int socketFS;
	int datosRecibidos;
	socketFS = conexionFS();

	if(socketFS != -1)
		{
			enviarQuery(socketFS,queryDrop);
			datosRecibidos = recibirInt(socketFS,&estadoResultado);
			close(socketFS);
			if(datosRecibidos>0 && estadoResultado == 1)
			{
				loggearInfo("Tabla Dropeada en FS");
				if(flagConsola)
					printf("%s\n","Tabla Dropeada en FS");
			}else
			{
				loggearInfo("Tabla no existente en FS");
				if(flagConsola)
					printf("%s\n","Tabla no existente en FS");
			}

		}
	else{
			if(flagConsola)
						printf("No se pudo conectar con FS\n");
	loggearInfo("No se pudo conectar con FS");
	}
}

void procesarJournal(query* queryJournal, int flagConsola)
{
	ejecutarJournal(flagConsola);
}


/*
 * JOURNAL
 */



void ejecutarJournal(int flagConsola)
{
	//Cuando este ocurriendo un journal no debe accederse a la memoria principal!
	pthread_mutex_lock(&mutex_journal);
	int resultadoJournal = journal();
	pthread_mutex_unlock(&mutex_journal);
	if(resultadoJournal == 1)
	{
	loggearInfo("Journal exitoso");
	if(flagConsola)
		printf("Journal exitoso\n");
	}else
	{
		loggearInfo("Error en Journal");
		if(flagConsola)
			printf("Error en Journal\n");
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
	t_list * registros;
	t_list * paginas;
	char * tabla;

	if(dictionary_size(tablaSegmentos) > 0)
	{
		tabla = list_get(listaSegmentos,indice);

		while(dictionary_has_key(tablaSegmentos,tabla))
		{
			registros = list_create();
			paginas = (t_list *) dictionary_get(tablaSegmentos,list_get(listaSegmentos,indice));

			void agregarRegistro(pagina * unaPagina)
			{
				list_add(registros,leerMarco(unaPagina->nroMarco));
			}
			list_iterate(paginas,(void*)agregarRegistro);

			enviarQuerysFS(list_get(listaSegmentos,indice),registros);

			indice++;

			tabla = list_get(listaSegmentos,indice);

			if(!tabla)
				break;
		}

		void limpiarPaginas(t_list * paginas)
		{
			list_destroy_and_destroy_elements(paginas,free);
		}
		dictionary_clean_and_destroy_elements(tablaSegmentos,(void*)limpiarPaginas);
		list_clean_and_destroy_elements(listaSegmentos,free);

	}

	list_clean(historialPaginas);
	memset(memoriaPrincipal,0,tamanioMemoria);

	return 1;
}



//-----------------------------------------------------------------------
//--------------------------------FUNCIONES AUXILIARES-------------------
//-----------------------------------------------------------------------

int obtenerCriterioConsistencia(char * tabla)
{
	query * describe;
	char ** consistencia, ** aux;
	int tipoConsistencia, cantidadRecibida = -1;

	int socket = conexionFS();

	query * queryDescribe = malloc(sizeof(query));
	queryDescribe->requestType = DESCRIBE;
	queryDescribe->tabla = strdup(tabla);

	enviarQuery(socket,queryDescribe);

	free(queryDescribe->tabla);
	free(queryDescribe);


	cantidadRecibida = recibirQuery(socket,&describe);

	close(socket);

	if(cantidadRecibida <= 0 || !strcmp(describe->tabla,"VOID"))
		return -1;

	consistencia = string_split(describe->tabla,";");
	aux = string_split(consistencia[1],"=");

	tipoConsistencia = string_a_consistencia(aux[1]);

	liberarCadenaSplit(consistencia);
	liberarCadenaSplit(aux);

		free(describe->tabla);
		free(describe);


	return tipoConsistencia;

}

void liberarMarco(int nroMarco)
{
	int offset = nroMarco*tamanioPag;
	memset(memoriaPrincipal + offset,0,tamanioPag); // dudosa
	pthread_mutex_lock(&mutex_marcos_libres);
	bitarray_clean_bit(marcosMemoria,nroMarco); // Se libera el marco para poder ser usado
	pthread_mutex_unlock(&mutex_marcos_libres);

}

void agregarPagina(char * tabla, int32_t key,char * value, int64_t timestamp, int flagModificado)
{
	t_list * paginas;

	paginas = (t_list*) dictionary_remove(tablaSegmentos,tabla);

	if(paginas == NULL)
	{
		paginas = list_create();

	}

	pagina * paginaActual = malloc(sizeof(pagina));

	paginaActual->nroMarco = obtenerMarcoLibre();
	paginaActual->nroPagina = list_size(paginas);
	paginaActual->flagModificado = flagModificado;

	registro * regAgregar = malloc(sizeof(registro));
	regAgregar->key = key;
	regAgregar->value = strdup(value);
	regAgregar->timestamp = timestamp;

	escribirMarco(paginaActual->nroMarco,regAgregar);

	agregarAHistorialPags(paginaActual);

	free(regAgregar->value); // VER SI ROMPE ESTA LINEA!
	free(regAgregar);

	list_add(paginas,paginaActual);

	dictionary_put(tablaSegmentos,tabla,paginas);
}

void agregarAHistorialPags(pagina * unaPagina)
{
	bool yaExiste(pagina * paginaCargada)
			{
				return paginaCargada->nroMarco == unaPagina->nroMarco;

			}
	list_remove_by_condition(historialPaginas,(void *)yaExiste);
	list_add(historialPaginas,unaPagina);
}

void eliminarDeHistorial(pagina * unaPagina)
{
		bool yaExiste(pagina * paginaCargada)
			{
				return paginaCargada->nroMarco == unaPagina->nroMarco;

			}
	list_remove_by_condition(historialPaginas,(void *)yaExiste);
}

int obtenerMarcoLibre()
{
	int sacame = 0;
	for(int i = 0; i < cantidadMarcos;i++)
	{
		if(estaLibre(i))
		{
		pthread_mutex_lock(&mutex_marcos_libres);
		bitarray_set_bit(marcosMemoria,i);
		pthread_mutex_unlock(&mutex_marcos_libres);
		return i;
		}
	}
	loggearInfo("Marcos completos: se inicia el LRU");
	int LRU = ejecutarLRU();
	if(LRU==-1)
		return obtenerMarcoLibre();
	else
		return LRU;
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
	return bitarray_test_bit(marcosMemoria,nroMarco)==0;
}

int ejecutarLRU()
{
	pagina * paginaAReemplazar;

	for(int i = 0; i< list_size(historialPaginas);i++)
	{
		paginaAReemplazar = list_get(historialPaginas,i);
		if(paginaAReemplazar->flagModificado == 0)
		{
			list_remove(historialPaginas,i);
			return paginaAReemplazar->nroMarco;
		}

	}
	loggearInfo("MEMORIA FULL -> SE PROCEDE A REALIZAR UN JOURNAL");

	journal(); // SI NO HAY PAGINAS DISPONIBLES => SE LIBERA TODA LA MEMORIA

	return -1;
}






bool existeEnMemoria(char * tabla)
{
	return dictionary_has_key(tablaSegmentos,tabla);
}

void buscarRegistroFS(query * selectQuery, int flagConsola)
{
	char * nombreTabla;
	query * queryInsertFS;
	int socketFS;
	int datosRecibidos;
	socketFS = conexionFS();

	if(socketFS != -1)
	{
		enviarQuery(socketFS,selectQuery);
		datosRecibidos = recibirQuery(socketFS,&queryInsertFS);
		close(socketFS);
		if(datosRecibidos > 0 && strcmp(queryInsertFS->value,"VOID"))
		{
			loggearRegistroEncontrado(queryInsertFS->key,queryInsertFS->value,flagConsola);
			dictionary_put(tablaSegmentos,queryInsertFS->tabla,NULL);
			nombreTabla = strdup(queryInsertFS->tabla);

			list_add(listaSegmentos,nombreTabla);
			agregarPagina(queryInsertFS->tabla,queryInsertFS->key,queryInsertFS->value,queryInsertFS->timestamp,0);

			free(queryInsertFS->tabla);
			free(queryInsertFS->value);
			free(queryInsertFS);
		}

		else
		{
			loggearRegistroNoEncontrado(selectQuery->key,flagConsola);
		}
	}else
	{
		if(flagConsola)
			printf("No se pudo conectar con FS\n");
		loggearInfo("No se pudo conectar con FS");
	}
}

registro * obtenerRegistro(char * tabla, int key)
{
		t_list * listaPaginas;
		registro * registroRetorno = NULL,* registroFinal = NULL;
				listaPaginas = (t_list *) dictionary_get(tablaSegmentos,tabla);
				void buscarKey(pagina * pagina)
				{
					registroRetorno = leerMarco(pagina->nroMarco);
					agregarAHistorialPags(pagina);
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
						return registroFinal;
			return NULL;
}







//-----------------------------------------------------------------------
//--------------------------------LOGS-----------------------------------
//-----------------------------------------------------------------------


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

void loggearInfoFileSystem(char * IP, char * Puerto)
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
	char * aux = malloc(strlen("Error la tabla  '' ya existente en el FS   ") + strlen(unaQuery->tabla) + 10);
	strcpy(aux,"Error la tabla '");
	strcat(aux,unaQuery->tabla);
	strcat(aux,"' ya existe en el FS");
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
	char * aux = malloc(strlen("Tabla '' eliminada Correctamente   en Memoria ") + strlen(tabla) + 10);
	strcpy(aux,"Tabla '");
	strcat(aux,tabla);
	strcat(aux,"' eliminada Correctamente en Memoria");
	loggearInfo(aux);
	if(flagConsola)
	printf("%s\n",aux);
	free(aux);
}
void loggearErrorDrop(char * tabla, int flagConsola)
{
	char * aux = malloc(strlen("La tabla '' no existe en Memoria... Enviando drop al FS") + strlen(tabla) + 10);
	strcpy(aux,"Tabla '");
	strcat(aux,tabla);
	strcat(aux,"' no existe en Memoria... Enviando drop al FS");
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
pthread_mutex_init(&mutex_marcos_libres, NULL);
pthread_mutex_init(&mutex_journal, NULL);

}


//-----------------------------------------------------------------------
//--------------------------------HELPERS--------------------------------
//-----------------------------------------------------------------------

char* getCleanString(char* dirtyString)
{
		int i,len=strlen(dirtyString);
		for(i=1;i<len-1;i++)
		{
			dirtyString[i-1]=dirtyString[i];
		}
		dirtyString[i-1]='\0';
		return dirtyString;
}

/*
 * HILO MONITOR
 */


void monitorearConfig()
{

		const char * pathConfig = "/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/PoolMemorias/configuraciones";
		char buffer[BUF_LEN];
		while(1)
				{
				int length = 0;
				int offset = 0;

			    int file_descriptor = inotify_init();

				if(file_descriptor == -1)
				{
				 perror("No se pudo crear el archivo monitor");
				 exit(-1);
				}

				    int watch_descriptor = inotify_add_watch(file_descriptor,pathConfig,IN_MODIFY|IN_CLOSE_WRITE);//Ver mask

				    if(watch_descriptor == -1)
				    {
				        perror("No se pudo crear el monitor");
				    }

				    length = read(file_descriptor,buffer,BUF_LEN);

				    while(offset < length)
				    {


						struct inotify_event * evento = (struct inotify_event *) &buffer;

						 if(length <= 0)
						 {
							   perror("Error al leer el archivo monitor.");
							   break;
						 }

						if(evento->len)
						{
							 if ((evento->mask & IN_CLOSE_WRITE) && !strcasecmp(evento->name,"memoria.cfg"))
							{

								 actualizarConfig();

							}
						}
						offset += sizeof (struct inotify_event) + evento->len;
				    }
				}
}


void actualizarConfig()
{
	crearConfig(PATHCONFIG);

	configuracionMemoria->RETARDO_FS = obtenerInt("RETARDO_FS");
	configuracionMemoria->RETARDO_GOSSIPING = obtenerDouble("RETARDO_GOSSIPING");
	configuracionMemoria->RETARDO_JOURNAL = obtenerDouble("RETARDO_JOURNAL");
	configuracionMemoria->RETARDO_MEM = obtenerInt("RETARDO_MEM");

	eliminarEstructuraConfig();
}

/*
 * GOSSIPING
 */

// -> VA A NACER UN HILO AL INICIAR LA MEMORIA QUE LLAME A EJECUTAR_GOSSIPING CADA X CANTIDAD DE TIEMPO

void ejecutarGossping()
{
	/*while(1)
	{


		usleep(configuracionMemoria->RETARDO_GOSSIPING);


		bool conectarConMemoria(char * ipPuerto)
		{
			return conectarConMemoria();
		}

		t_list * listaMemoriasActivas = filter(listaMemoriasConfig,conectarConMemoria);



	}*/

}

void sumarMemoriaAlPool()
{
	/*
	 * Actualizar config
	 */
	int nroMemoria = configuracionMemoria->MEMORY_NUMBER + 1;
	char * nroMemoriaChar = string_itoa(nroMemoria);
	crearConfig(PATHCONFIG);

	cambiarValorConfig("MEMORY_NUMBER",nroMemoriaChar);

	eliminarEstructuraConfig();
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

//Hacer el handshake con FS para poder calcular el tamanio de marco por el max tam del value => OK

//Pensar algoritmo de reemplazo de paginas -> EN TESTING
//LRU => https://github.com/sisoputnfrba/foro/issues/1174 & https://github.com/sisoputnfrba/foro/issues/1171

//Terminar API => DIO OK EN MEMORIA | EN TESTING CON SERVIDOR

//Definir bien el JOURNALING => DIO OK EN MEMORIA | EN TESTING CON SERVIDOR

//Ver criterios de consistencia => EN TESTING

//Levantar Pool Memorias => PENDIENTE

//Ver gossiping => PENDIENTE

//Levantar hilo monitor => OK

//Hacer pruebas hasta romper el mundo => WIP





