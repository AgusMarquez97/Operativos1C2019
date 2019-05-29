#include "Lissandra.h"

void iniciarLFS()
{
	iniciarLog("FileSystem");
	inicializarSemaforos();

	/*
	 * 3 Hilos iniciales para LFS:
	 * 1° -> Valida y gestiona la consola (Puede recibir Querys)
	 * 2° -> Gestiona el servidor (Puede recibir Querys)
	 * 3° -> Gestiona la MemTable -> Inserta cada registro que le envia el servidor o la consola
	 * Por ahora no se crean hilos on demand x cada request
	 */

	hiloConsola = crearHilo(consola,NULL);
	hiloServidor = crearHilo(iniciarServidor,NULL);
	//hiloMemTable = crearHiloDetachable(procesarQuery,NULL);

	esperarHilo(hiloConsola);
	esperarHilo(hiloServidor);

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

	crearConfig("../configuraciones/configuracion.txt");

	char * ip = malloc(50);
	strcpy(ip,"IP del servidor: ");
	strcat(ip,obtenerString("DIRECCION_IP"));

	char * puerto = malloc(50);
	strcpy(puerto,"Puerto de escucha: ");
	strcat(puerto,obtenerString("PUERTO_ESCUCHA"));



	loggearInfo(ip);
	loggearInfo(puerto);

	/*
	 * Minimamente dos hilos:
	 * 1 hilo -> Controla y gestiona el Servidor
	 * 1 hilo -> Controla y gestiona la Memtable
	 * los demas Hilos / Procesos podrian gestionar el pasaje de LFS a FS (dumping) y los cambios del compactador
	 */

	//levantarServidor(ip,puerto);

	/*
	 * ¿IniciarHilos, semaforos y procesos?
	 */

	free(ip);
	free(puerto);

}

void inicializarSemaforos()
{
	pthread_mutex_init(&mutex_Mem_Table,NULL);
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
		loggearWarning("Comando aun no disponible");
	}
}

void procesarInsert(query * unaQuery)
{
	if(existeTabla(unaQuery->tabla) == 0)
	{
		loggearError("Error: Tabla no existente en el FileSystem");
		exit(-1);
	}

	/*
	 * FE DE ERRATA DEL TP:
	 * Al pedo obtener la metadata de la tabla -> La memtable no tiene particiones
	 */
	if(unaQuery->timestamp == NULL)
	{
		unaQuery->timestamp = (double)time(NULL);//ObtenerTimeStamp();
	}
	agregarUnRegistroMemTable(unaQuery->tabla,unaQuery->key,unaQuery->value,unaQuery->timestamp);
}


void agregarUnRegistroMemTable(char * nombreTabla,int key, char * value, double timestamp)
{
	pthread_mutex_lock(&mutex_Mem_Table);
	//Ver como agregar el registro -> Usar funcion de las commons
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

/*
 * Luego cambiar esta logica
 */
bool existeTabla(char * nombre)
{
	return true;//list_find()
}
