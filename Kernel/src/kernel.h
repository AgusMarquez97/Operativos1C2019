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
#include <semaphore.h>
#include <pthread.h>

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

//sem_t s_newq;
sem_t s_hay_request;
sem_t s_hay_new;
sem_t s_exec_request_inicial;
sem_t s_dispatcher;
//sem_init(&s_hay_request,0,0);
pthread_mutex_t s_newq;// = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t s_readyq;
pthread_mutex_t s_requestq;
int CANT_THREADS_EXEC = 5; //TODO: Debe salir del archivo de config
int QUANTUM_SIZE = 3; //TODO: Debe salir del archivo de config

#endif /*KERNEL_H_*/
