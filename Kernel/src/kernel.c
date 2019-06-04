/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Proceso kernel
 ============================================================================
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/src/kernel_api.c>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/src/kernel.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>	
#include <semaphore.h>
#include <pthread.h>

#define MAXDATASIZE 1000

void *get_in_addr(struct sockaddr *sa) {

	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);

}

int solicitar_memorias() {
	/*
	 * Se comunica con la memoria que encuentra en el archivo
	 * 	de configuracion y solicita pool de memorias.
	 *
	 * Sin implementar todavia
	 */

	printf("Solicitando pool de memorias...\n");
	return 0;
}

int agregar_memorias_a_criterios() {
	/*
	 * Agrega las memorias del pool que conoce a los distintos criterios
	 *
	 * Sin implementar todavia
	 */

	printf("Agregando memorias a los distintos criterios...\n");
	return 0;
}
/*
int parsear(char * query) {
	printf("Esto por ahora no hace nada, pero se viene se viene...\n");
	return 0;
}

int ejecutar_request(char * query) {
	printf("Query ejecutado (?)\n");
	return 0;
}
*/

int ejecutar_request(/*t_queue */void * request)
{
	query query_struct;// = malloc(sizeof(query));
	request = (char *) request;
	int resultado_ejecucion_request = 0;
	printf("%s",request);
	request = string_substring(request, 0, string_length(request) - 2);
	int codigo_request = parsear(request,&query_struct);
	if ( !codigo_request )
	{
	  return -1;
	}

	switch (codigo_request) {

	  case (SELECT): resultado_ejecucion_request = ejecutar_select(query_struct);
			 break;

	  default: printf("No es un select, se deriva el request...");
		   break;

	}

	return resultado_ejecucion_request;

}


t_request_struct *crear_estructura_para_planificar(t_queue* requests) {
	t_request_struct *request_struct = malloc(sizeof(t_request_struct));
	request_struct->request_queue = requests;
	request_struct->request_cant = 1; //Posiblemente no se use
	request_struct->request_left = 1; //Posiblemente no se use
	return request_struct;

}

void agregar_request_ready(t_request_struct * request)
{

		pthread_mutex_lock(&s_readyq);

		queue_push(ready_queue, request);

		pthread_mutex_unlock(&s_readyq);

		sem_post(&s_exec_request_inicial);

}


void *exec(void * numero_exec) {

	int thread_n = (int) numero_exec;

	printf("**** El hilo de ejecucion %d (thread_id %d) esta arriba ****\n",
			thread_n, pthread_self());

	while (1) {
		sem_wait(&s_exec_request_inicial);

		pthread_mutex_lock(&s_readyq);
		t_request_struct * next_request = (t_request_struct *) queue_pop(ready_queue);
		pthread_mutex_unlock(&s_readyq);

		t_queue* requests = next_request->request_queue;
		int quantum_utilizado = 1;
		int fallo = 0;

		if ((int) queue_size(requests) > 1) { sleep(7);} //Retrasa la ejecucion y me da tiempo de meter otro request en el medio

		while ( (quantum_utilizado <= QUANTUM_SIZE) && ((int) queue_size(requests) > 0) && (fallo == 0) ){

			printf(
					"**** El hilo de ejecucion %d va a ejecutar el siguiente request ****\n",
					thread_n);

			quantum_utilizado++;

			int codigo_ejecucion = ejecutar_request(queue_pop(requests));

			if (codigo_ejecucion < 0) {

			  printf("Fallo request, abortando ejecucion de script\n");
			  //agregar_a_estado_exit(next_request); En realidad se debe hacer esto
			  return;

			}

			printf("El codigo de request es: %d\n",codigo_ejecucion);

		  }

		if ((int) queue_size(requests) == 0) {  

			queue_destroy(requests);
			//agregar_a_estado_exit(next_request); En realidad se debe hacer esto
			printf("Fin de proceso.\n\n\n\n");
			printf("*********************************************\n\n\n\n");

		} else {

			printf("\n\n\n\n**** Se acabo el quantum, se vuelve a la ready queue ****\n\n\n\n");
			agregar_request_ready(next_request);
		}
	
	}

}




void *agregar_a_ready() {

	while (1) {

		sem_wait(&s_hay_request);

		pthread_mutex_lock(&s_newq);

		void *requests = queue_pop(new_queue);

		pthread_mutex_unlock(&s_newq);

		t_request_struct *requests_struct = crear_estructura_para_planificar(
				(t_queue*) requests);

		agregar_request_ready(requests_struct);

	}

}

void agregar_a_new(t_queue* requests) {
	pthread_mutex_lock(&s_newq);
	queue_push(new_queue,requests);
	pthread_mutex_unlock(&s_newq);

	sem_post(&s_hay_request);

	return;

}

t_queue* procesar_script(char * script, t_queue* request_queue) {
	FILE * fid;
	size_t len = 0;
	ssize_t read;
	char * line = NULL;

	if ((fid = fopen(script, "r+")) == NULL) {
		printf("Error al abrir el script: %s", script);
		printf("Longitud del parametro ruta del archivo: %d",
				string_length(script));
		exit(-1);
	}

	pthread_mutex_lock(&s_requestq);

	while ((read = getline(&line, &len, fid)) != -1) {
		char * line2 = string_duplicate(line);
		queue_push(request_queue, line2);
	}

	pthread_mutex_unlock(&s_requestq);

	return request_queue;
}


t_queue* armar_request_queue(char * input) {
	t_queue* request_queue = queue_create();

	if (string_starts_with(input, "@")) {
		procesar_script(string_substring(input, 1, string_length(input) - 3),
				request_queue);
		return request_queue;
	}

	queue_push(request_queue, (void *) input);

	return request_queue;

}

void * atender_conexion(void * new_fd) {
	char buf[MAXDATASIZE];
	int numbytes;
	int listening_socket = (int) new_fd;

	char *msj = "*** Conectado a Lissandra ***\n";
	char * input;
	int len, bytes_sent;

	len = strlen(msj);
	bytes_sent = send(listening_socket, msj, len, 0);

	if ((numbytes = recv(listening_socket, buf, MAXDATASIZE - 1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	printf("client: received %s", buf);
	buf[numbytes] = '\0';

	while (strncmp("SALIR", buf, strlen("SALIR")) != 0) {

		t_queue* request_queue = armar_request_queue(buf);

		agregar_a_new(request_queue);

		if ((numbytes = recv(listening_socket, buf, MAXDATASIZE - 1, 0))
				== -1) {
			perror("recv");
			exit(1);
		}

		printf("client: received %s", buf);
		buf[numbytes] = '\0';

	};

	close(listening_socket);
	pthread_exit(NULL);

}

int procesar() {
	pthread_t thread_id;
	struct addrinfo hints, *res, *p;
	int status, sckt, new_fd; //, numbytes;
	char ipstr[INET6_ADDRSTRLEN];
	socklen_t addr_size;
	int yes = 1;
	struct sockaddr_storage their_addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((status = getaddrinfo(NULL, "20000", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {

		if ((sckt = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sckt, p->ai_addr, p->ai_addrlen) == -1) {
			close(sckt);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sckt, 0) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Planificador iniciado. Escuchando...\n");

	while (1) {
		addr_size = sizeof their_addr;
		new_fd = accept(sckt, (struct sockaddr *) &their_addr, &addr_size);

		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr), ipstr,
				sizeof ipstr);
		printf("...nueva conexion desde: %s\n", ipstr);
		pthread_create(&thread_id, NULL, atender_conexion, (void *) new_fd);
		pthread_detach(thread_id);
	}
}

void inicializar_semaforos() {
	sem_init(&s_hay_request, 0, 0);
	sem_init(&s_hay_new, 0, 0);
	sem_init(&s_exec_request_inicial, 0, 0);
	sem_init(&s_dispatcher, 0, 1);
	pthread_mutex_init(&s_newq, NULL);
	pthread_mutex_init(&s_readyq, NULL);
}

void inicializar_threads() {
	pthread_t readythread_id;
	pthread_t thread_id;

	pthread_create(&readythread_id, NULL, agregar_a_ready, NULL);
	pthread_detach(readythread_id);

	for (int thread = 1; thread <= CANT_THREADS_EXEC; thread++) {
		pthread_create(&thread_id, NULL, exec, (void *) thread);
		pthread_detach(thread_id);
	}
}

void inicializar_colas() {
	new_queue = queue_create();
	ready_queue = queue_create();
}

void iniciar_estructuras() {

	inicializar_semaforos();
	inicializar_threads();
	inicializar_colas();

}

int main(int argc, char *argv[]) {

	iniciar_estructuras();
	solicitar_memorias();
	agregar_memorias_a_criterios();
	procesar();
	printf("Saliendo...");
	return 0;
}
