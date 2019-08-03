#ifndef KERNEL_H_
#define KERNEL_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/inotify.h>


#include <semaphore.h>
#include <pthread.h>


#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/dictionary.h>

#include <biblioteca/levantarConfig.h>
#include <biblioteca/parser.h>

#include <biblioteca/enumsAndStructs.h>

#include <biblioteca/logs.h>

//#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/src/kernel_utils.c>

#include <readline/readline.h>
#include <readline/history.h>
#include <sys/inotify.h>


#include <biblioteca/sockets.h>
#include <biblioteca/serializacion.h>
#include <biblioteca/mensajes.h>
#include <biblioteca/logs.h>
#include <biblioteca/hilos.h>
#include <biblioteca/levantarConfig.h>
#include <biblioteca/utils.h>
#include <biblioteca/parser.h>

#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

#include <time.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/inotify.h>


#include <signal.h>

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>







#define EVENT_SIZE (sizeof(struct inotify_event)+24)
#define BUF_LEN (1024*EVENT_SIZE)

typedef struct request_element{
		t_queue* request_queue;
		int request_cant;
		int request_left;
		//struct link_element *next;
	} t_request_struct;


//t_queue* new_queue = NULL;
//t_queue* ready_queue = NULL;


sem_t s_listensocket;

//t_queue* new_queue = queue_create();
t_queue* new_queue = NULL;
t_queue* ready_queue = NULL; //queue_create();
t_queue* exit_queue = NULL;
t_list* lista_estadisticas_selects = NULL;
t_list* lista_estadisticas_inserts = NULL;

//sem_t s_newq;
sem_t s_hay_request;
sem_t s_hay_new;
sem_t s_exec_request_inicial;
sem_t s_dispatcher;
sem_t s_hay_request_estado_exit;
//sem_init(&s_hay_request,0,0);
pthread_mutex_t s_newq;// = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t s_readyq;
pthread_mutex_t s_requestq;
pthread_mutex_t s_exitq;
pthread_mutex_t s_lista_selects;
pthread_mutex_t s_lista_inserts;
int CANT_THREADS_EXEC;
int QUANTUM_SIZE;
float RETARDO_EJECUCION,sleep_ejecucion;
long REFRESH_METADATA;

char * sc_memory = "IP:PUERTO";
char * IP_MEMORIA_INICIAL;
char * PUERTO_MEMORIA_INICIAL;
t_queue* memorias_ec = NULL;

t_log * kernel_log = NULL;

t_dictionary * diccionario_selects = NULL; //Esto seguramente hay que eliminarlo

typedef struct {
	unsigned timestamp_operacion;
	double duracion_operacion;
} t_lista_metricas;



/*
 * 	KERNEL API
 */


void journal_request();
int ejecutar_select(query * query_struct);
int ejecutar_drop(query * query_struct);
int ejecutar_run(query * query_struct);
int derivar_request(query * query_struct);
int ejecutar_add(query * query_struct);
int ejecutar_insert(query * query_struct);
int ejecutar_metrics();


void journal_request(query * query_struct)
{
	printf("Journaling solicitado con exito");

	char * IP = strdup("127.0.0.1");
	char * Puerto = strdup("1366");
	int socket_memoria = levantarCliente(IP,Puerto);

	enviarQuery(socket_memoria,query_struct);

	free(IP);
	free(Puerto);
}



int tipo_consistencia_tabla(char * nombre_tabla)
{
 // Necesito las estructuras
return 1;
}


bool kernel_conoce_tabla(char * nombre_tabla)
{
//Me fijo en las estructuras del kernel a ver si tengo conocimiento de la tabla
}


char * obtener_proxima_memoria(int tipo_consistencia)
{
	//Sin implementar
}



enviar_query_a_memoria(query * query_struct)
{
	int tipo_consistencia,socket_memoria;
	char * direccion_memoria,IP,PUERTO;

	tipo_consistencia = tipo_consistencia_tabla(query_struct->tabla);
	direccion_memoria = obtener_proxima_memoria(tipo_consistencia);

	//Aca se obtienen los datos de memoria
	//IP = direccion_memoria_split[0];
	//PUERTO = direccion_memoria_split[1];

	socket_memoria = levantarCliente(IP,PUERTO);

	enviarQuery(socket_memoria,query_struct);

	free(IP);
	free(PUERTO);
}



int ejecutar_select(query * query_struct)
{
	double *tiempo_ejecucion = malloc(sizeof(double));
	clock_t inicio,fin;
	inicio = clock();

	if (kernel_conoce_tabla(query_struct->tabla)) {
	  enviar_query_a_memoria(query_struct);
	} else
	{
		printf("No existe la tabla\n");
		return -1;
	}

	fin = clock();

	*tiempo_ejecucion = ((double) (fin - inicio)) / CLOCKS_PER_SEC; // En segundos

	t_lista_metricas *struct_operacion = malloc(sizeof(t_lista_metricas));

	time_t lahora = time(NULL);
	(*struct_operacion).timestamp_operacion = (unsigned) lahora;
	(*struct_operacion).duracion_operacion = *tiempo_ejecucion;


	pthread_mutex_lock(&s_lista_selects);
	list_add(lista_estadisticas_selects,struct_operacion);
	pthread_mutex_unlock(&s_lista_selects);

	return 0;

}


int ejecutar_drop(query * query_struct)
{

	if (kernel_conoce_tabla(query_struct->tabla)) {
	  enviar_query_a_memoria(query_struct);
	} else
	{
		printf("No existe la tabla\n");
		return -1;
	}

}


int ejecutar_create(query * query_struct)
{
  enviar_query_a_memoria(query_struct);
}


int ejecutar_add(query * query_struct /*char * string_memoria*/)
{
	  //char ** memoria = string_split(string_memoria,":");

	  if (query_struct->consistencyType == SC/*!strcasecmp(memoria[2],"SC")*/) {
		printf("El criterio de la memoria es SC.\n");

	  char * IP = strdup("127.0.0.1");
	  char * Puerto = strdup("1366");

	  //loggearInfoCliente(IP,Puerto);

	  int socket_memoria = levantarCliente(IP,Puerto);

	  enviarRequest(socket_memoria,RUN);

	  free(IP);
	  free(Puerto);

	  } else if (query_struct->consistencyType == SHC/*!strcasecmp(memoria[2],"SHC")*/) {
		printf("El criterio de la memoria es SHC.\n");
	  }

}


int ejecutar_insert(query * query_struct)
{
	char * nombre_tabla = query_struct->tabla;
	double *tiempo_ejecucion = malloc(sizeof(double));
	clock_t inicio,fin;
	inicio = clock();
	printf("Se ejecuta el siguiente request (Por ahora solo se muestra esto por pantalla): \"INSERT %s %d\"\n",nombre_tabla,query_struct->key);

	if (kernel_conoce_tabla(query_struct->tabla)) {
	  enviar_query_a_memoria(query_struct);
	} else
	{
		printf("No existe la tabla\n");
		return -1;
	}

	fin = clock();

	*tiempo_ejecucion = ((double) (fin - inicio)) / CLOCKS_PER_SEC; // En segundos

	t_lista_metricas *struct_operacion = malloc(sizeof(t_lista_metricas));

	time_t lahora = time(NULL);
	(*struct_operacion).timestamp_operacion = (unsigned) lahora;
	(*struct_operacion).duracion_operacion = *tiempo_ejecucion;


	pthread_mutex_lock(&s_lista_inserts);
	list_add(lista_estadisticas_inserts,struct_operacion);
	pthread_mutex_unlock(&s_lista_inserts);


}



int ejecutar_metrics()
{
  //printf("\n\n\n\n\n\n ********* Se solicitan las metricas por consola ********* \n\n\n\n\n\n");
  //log_info(kernel_log,"********* Se solicitan las metricas por consola ********* \n\n\n\n");

//  pthread_mutex_lock(&s_lista_selects);
  metricas_reads(/*lista_estadisticas_selects*/1);
//  pthread_mutex_unlock(&s_lista_selects);

//  pthread_mutex_lock(&s_lista_inserts);
  metricas_writes(/*lista_estadisticas_inserts*/1);
//  pthread_mutex_unlock(&s_lista_inserts);

  // Fata el memory load ya lo seeeeeeeeeeee

  printf("\n\n ********* Metricas ejecutadas ********* \n\n\n\n\n\n");
  //log_info(kernel_log,"********* Metricas ejecutadas ********* \n\n\n\n");
}









/*
 * KERNEL UTILS
 */


void * monitorear_config();


void * monitorear_config()
{

	const char * config_file = "/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/config/kernel_config.cfg";

	int bytes_leidos;
	struct inotify_event * evento = malloc(sizeof(*evento));


    int archivo_monitor = inotify_init();

	if(archivo_monitor == -1)
	{
	 perror("No se pudo crear el archivo monitor");
	}

	    int monitor = inotify_add_watch(archivo_monitor,config_file,IN_MODIFY);//Ver mask

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
	            perror("Error al leer el archivo monitor.");
	            break;
	        }

	        if(evento->mask == IN_MODIFY)
	        {
		  QUANTUM_SIZE = atoi(strdup(obtenerString("QUANTUM")));
		  REFRESH_METADATA = atoi(strdup(obtenerString("REFRESH_METADATA")));
		  RETARDO_EJECUCION = atof(strdup(obtenerString("RETARDO_EJECUCION")));
	        }


	    }

}



void * refrescar_memorias()
{
	while (1)
	{
		int socket_memoria_inicial = levantarCliente(IP_MEMORIA_INICIAL,PUERTO_MEMORIA_INICIAL);
		enviarRequest(socket_memoria,/*algo*/);

		//Cargo lo que me llega

		sleep(1);
	}
}







#endif /*KERNEL_H_*/
