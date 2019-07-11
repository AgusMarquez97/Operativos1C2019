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
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include <pthread.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/levantarConfig.c>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/parser.c>
#include "/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/enumsAndStructs.h"
#include "/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/logs.c"
#include <readline/readline.h>
#include <readline/history.h>

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
//sem_init(&s_hay_request,0,0);
pthread_mutex_t s_newq;// = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t s_readyq;
pthread_mutex_t s_requestq;
pthread_mutex_t s_exitq;
pthread_mutex_t s_lista_selects;
pthread_mutex_t s_lista_inserts;
int CANT_THREADS_EXEC;// = 5; //TODO: Debe salir del archivo de config
int QUANTUM_SIZE;// = 2; //TODO: Debe salir del archivo de config

char * sc_memory = "IP:PUERTO";
t_queue* memorias_ec = NULL;

t_log * kernel_log = NULL;

t_dictionary * diccionario_selects = NULL; //Esto seguramente hay que eliminarlo

typedef struct {
	unsigned timestamp_operacion;
	double duracion_operacion;
} t_lista_metricas;


#endif /*KERNEL_H_*/
