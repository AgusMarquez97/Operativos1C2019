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

//#include "kernel_api.h"
//#include "kernel_utils.h"

#include "kernel.h"

#define MAXDATASIZE 1000

void *get_in_addr(struct sockaddr *sa) {

	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);

}





bool filtro_estadisticas(t_lista_metricas* struct_operacion)
{
	unsigned result = (unsigned) time(NULL) - ((*struct_operacion).timestamp_operacion);
	//printf("La diferencia en segundos es de: %u\n",result);
	return (result <= 30);

}


int metricas_reads(/*t_list* lista_metricas*/int tipo_salida)//, int cant_reads)
{
	double duracion_total = 0;
	float latency = 0;
	int cant_reads = list_size(lista_estadisticas_selects);
	t_lista_metricas *struct_operacion = malloc(sizeof(t_lista_metricas));

	for (int i = 0;i<=cant_reads-1;i++)
	{
	  struct_operacion = list_get(/*lista_metricas*/lista_estadisticas_selects,i);
	  duracion_total = duracion_total + (*struct_operacion).duracion_operacion;
	  latency = duracion_total/cant_reads;
	}

	switch (tipo_salida)
	{
	  case 0: log_info(kernel_log,"Reads: %d",cant_reads);
		  log_info(kernel_log,"Read latency: %f",latency);
		  log_info(kernel_log,"");
		  break;
	  case 1: printf("\nReads: %d\n",cant_reads);
		  printf("Read latency: %f\n",latency);
		  break;
	  default: printf("Tipo incorrecto de salida\n");
		   return -1;
	}


}


void metricas_writes(/*t_list* lista_metricas*/int tipo_salida)//, int cant_writes)
{
	double duracion_total = 0;
	float latency = 0;
	int cant_writes = list_size(lista_estadisticas_inserts);
	t_lista_metricas *struct_operacion = malloc(sizeof(t_lista_metricas));

	for (int i = 0;i<=cant_writes-1;i++)
	{
	  struct_operacion = list_get(/*lista_metricas*/lista_estadisticas_inserts,i);
	  duracion_total = duracion_total + (*struct_operacion).duracion_operacion;
	  latency = duracion_total/cant_writes;
	}

	switch (tipo_salida)
	{
	  case 0: log_info(kernel_log,"Writes: %d",cant_writes);
		  log_info(kernel_log,"Write latency: %f",latency);
		  log_info(kernel_log,"");
		  log_info(kernel_log,"");
		  log_info(kernel_log,"");
		  break;
	  case 1: printf("Writes: %d\n",cant_writes);
		  printf("Write latency: %f\n\n\n",latency);
		  break;
	  default: printf("Tipo incorrecto de salida\n");
		   return;
	}
	
	



}


void *mostrar_y_purgar_metricas() {

	while (1) {
		sleep(30); //Por motivos de prueba se le puso 10, cambiar a 30 al final

		log_info(kernel_log,"**** Comienzo de metricas automaticas ****");
		log_info(kernel_log,"");
		log_info(kernel_log,"");
		log_info(kernel_log,"");

		pthread_mutex_lock(&s_lista_selects);

		//printf("El tamaño de la lista antes de purgar es: %d\n",list_size(lista_estadisticas_selects));
		lista_estadisticas_selects = list_filter(lista_estadisticas_selects,filtro_estadisticas);
		int cant_selects = list_size(lista_estadisticas_selects);
		//printf("El tamaño de la lista despues de purgar es: %d\n",cant_selects);

		metricas_reads(/*lista_estadisticas_selects*/0);//,cant_selects);

		pthread_mutex_unlock(&s_lista_selects);



		pthread_mutex_lock(&s_lista_inserts);

		//printf("El tamaño de la lista antes de purgar es: %d\n",list_size(lista_estadisticas_inserts));
		lista_estadisticas_inserts = list_filter(lista_estadisticas_inserts,filtro_estadisticas);
		int cant_inserts = list_size(lista_estadisticas_inserts);
		//printf("El tamaño de la lista despues de purgar es: %d\n",cant_inserts);

		metricas_writes(/*lista_estadisticas_inserts*/0);//,cant_inserts);

		pthread_mutex_unlock(&s_lista_inserts);
 	
		// Fata el memory load ya lo seeeeeeeeeeee

		log_info(kernel_log,"");
		log_info(kernel_log,"");
		log_info(kernel_log,"");
		log_info(kernel_log,"**** Fin de metricas automaticas ****");
	}

}


int solicitar_memorias(char * ip_memoria, char * puerto_memoria, t_queue * queue_memorias) {
	/*
	 * Se comunica con la memoria que encuentra en el archivo
	 * 	de configuracion y solicita pool de memorias.
	 *
	 * Por ahora se hardcodea la queue
	 */

	printf("Solicitando pool de memorias...\n");

	int socket_memoria_inicial = levantarCliente(IP_MEMORIA_INICIAL,PUERTO_MEMORIA_INICIAL);
	char * lista_memorias = enviarRequest(socket_memoria_inicial,GOSSIP_KERNEL //Esto va a andar cuando se agregeu el enum);
	agregar_memorias_a_criterios(lista_memorias);

	//Cargo lo que me llega

	return 0;
}

int agregar_memorias_a_criterios(char * lista_memorias) {
	/*
	 * Agrega las memorias del pool que conoce a los distintos criterios
	 *
	 * Sin implementar todavia
	 */

	char * ip_memoria_inicial = obtenerString("IP_MEMORIA");
	char * puerto_memoria_inicial = obtenerString("PUERTO_MEMORIA");
	//t_list *lista_memorias;// = malloc(sizeof(t_list));
	t_queue * queue_memorias = queue_create();
	

	/* Aca iria el llamado a la funcion solicitar_memorias
	 *
	 * Hay que ponerse de acuerdo en como implementar esto con los chicos de memoria
	 *
	 * Se supone que luego del llamado tengo una lista de strings <IP>:<PUERTO>:<CRITERIO>
	 *
	 * Por ahora se hardcodea una lista para probar el resto
	 */

	solicitar_memorias(ip_memoria_inicial,puerto_memoria_inicial,queue_memorias);

	if (queue_size(queue_memorias) == 0)
	{
		printf("No se encontraron memorias.\n");
		return -1;
	}

	printf("Agregando memorias a los distintos criterios...\n");

	while (queue_size(queue_memorias) > 0) {

	  char * string_memoria = queue_pop(queue_memorias);
	  printf("Memoria encontrada: %s\n",string_memoria);

	  ejecutar_add(string_memoria);
}
//	char * memoria = "111.222.333.444:1234";
//	queue_push(memorias_ec,memoria);
	return 0;
}


agregar_a_estado_exit(t_request_struct * next_request)
{
	pthread_mutex_lock(&s_exitq);
	queue_push(exit_queue, next_request);
	pthread_mutex_unlock(&s_exitq);
	sem_post(&s_hay_request_estado_exit);
	
}



int ejecutar_request(query * query_struct)
{
	int resultado_ejecucion_request = 0;
	int codigo_request = query_struct->requestType;

	//printf("Nombre de tabla: %s\n",query_struct->tabla);

	if ( codigo_request == 0 )
	{
	  return -1;
	}

	switch (codigo_request) {

	  case (SELECT):
				resultado_ejecucion_request = ejecutar_select(query_struct);
			 	break;

	  case (DROP):
				resultado_ejecucion_request = ejecutar_drop(query_struct);
			 	break;

	  case (INSERT):
				resultado_ejecucion_request = ejecutar_insert(query_struct);
			 	break;

	  case (CREATE):
				resultado_ejecucion_request = ejecutar_create(query_struct);
			 	break;

	  default: 		printf("No es un request conocido - ERROR\n");
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
	query *next_query = malloc(sizeof(query));

	log_info(kernel_log,"**** El hilo de ejecucion %d (thread_id %u) esta arriba ****",thread_n, pthread_self());
	//printf("**** El hilo de ejecucion %d (thread_id %d) esta arriba ****\n",
	//		thread_n, pthread_self());

	while (1) {
		sem_wait(&s_exec_request_inicial);

		pthread_mutex_lock(&s_readyq);

		/*
		 * De la ready queue se extrae la proxima request queue
		*/
		t_request_struct * next_request = (t_request_struct *) queue_pop(ready_queue); // 
		
		pthread_mutex_unlock(&s_readyq);

		t_queue* requests = next_request->request_queue; // Se extrae la lista de requests propiamente dicha
		int quantum_utilizado = 1;


		//if ((int) queue_size(requests) > 1) { sleep(7);} //Retrasa la ejecucion y me da tiempo de meter otro request en el medio

		while ( (quantum_utilizado <= QUANTUM_SIZE) && ((int) queue_size(requests) > 0)){

			next_query = (query *) queue_pop(requests);		

			log_debug(kernel_log,"**** El hilo de ejecucion %d va a ejecutar el siguiente request: %d ****\n",thread_n,next_query->requestType);
			//printf(
			//		"**** El hilo de ejecucion %d va a ejecutar el siguiente request: %d ****\n",
			//		thread_n,next_query->requestType);

			quantum_utilizado++;


			int codigo_ejecucion = ejecutar_request(next_query);

			if (codigo_ejecucion < 0) {

			  log_error(kernel_log,"Error en la ejecucion del request."); //TODO:Ver forma de indicar el request que fallo 
			  printf("Error en la ejecucion del request.\n");
			  printf("Fin de proceso.\n\n\n\n");
			  printf("*********************************************\n\n\n\n");
			  agregar_a_estado_exit(next_request);// En realidad se debe hacer esto
			  log_info(kernel_log,"**** Fin del script por fallo en un request. Se esperan %f segundos\n",sleep_ejecucion);
			  sleep(sleep_ejecucion);
			  log_info(kernel_log,"**** Lista la espera.\n");
			  return 0;

			}

			//printf("El codigo de resultado de ejecucion de request es: %d\n",codigo_ejecucion);
			//free(next_query);

		  }

		if ((int) queue_size(requests) == 0) {  

			//queue_destroy(requests);
			//queue_clean_and_destroy_elements(requests, free);
			agregar_a_estado_exit(next_request);// En realidad se debe hacer esto
			//printf("Fin de proceso.\n\n\n\n");
			//printf("*********************************************\n\n\n\n");
			log_info(kernel_log,"Fin de ejecucion de request unitario o script. Se esperan %f segundos",sleep_ejecucion);
			sleep(sleep_ejecucion);
			log_debug(kernel_log,"**** Lista la espera ****");

		} else {
			log_info(kernel_log,"**** Se acabo el quantum, se vuelve a la ready queue ****");
			printf("\n\n\n\n**** Se acabo el quantum, se vuelve a la ready queue ****\n\n\n\n");
			agregar_request_ready(next_request);
		} //printf("En estado exit hay %d requests (Los scripts se cuentan como un solo request)\n",(int) queue_size(exit_queue));
	
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



void *purgar_estado_exit() {

	while (1) {

		sem_wait(&s_hay_request_estado_exit);
		log_debug(kernel_log,"****Llego un request al estado exit. Se lo elimina de inmediato ****");
		pthread_mutex_lock(&s_exitq);

		queue_pop(exit_queue);

		pthread_mutex_unlock(&s_exitq);

	}

}



void agregar_a_new(t_queue* requests) {
	pthread_mutex_lock(&s_newq);
	queue_push(new_queue,requests);
	pthread_mutex_unlock(&s_newq);

	sem_post(&s_hay_request);

	return;

}


t_queue* procesar_script(char * script) {
	FILE * fid;
	size_t len = 0;
	ssize_t read;
	char * line = NULL;
	char ** line3;
	int linea_script = 1;

	//printf("Se ejecuta el script: %s\n",script);
	log_info(kernel_log,"Se ejecuta el script: %s",script);

	if ((fid = fopen(script, "r+")) == NULL) {
		log_error(kernel_log,"Error al abrir el script: %s",script);
		log_debug(kernel_log,"Longitud del parametro ruta del archivo: %d",string_length(script));
		printf("Error al abrir el script: %s\n", script);
		return NULL;
	}

	t_queue * request_queue = queue_create();

	//pthread_mutex_lock(&s_requestq);

	while ((read = getline(&line, &len, fid)) != -1) {
		//char * line2 = string_duplicate(line);
		line3 = string_split(line,"\n");
		query * query_struct = malloc(sizeof(query));
		if ( parsear(line3[0],&query_struct) > 0 )
		{
		  queue_push(request_queue, query_struct);//No va free(); -> NO LIBERA
		} else
		  {
			log_error(kernel_log,"%s: comando desconocido.",line3[0]);
			log_error(kernel_log,"El request en la linea %d fallo, se cancela la ejecucion del script",linea_script);
			printf("%s: comando desconocido.\n",line3[0]);
			printf("El request en la linea %d fallo, se cancela la ejecucion del script\n",linea_script);
			//printf("Fin de proceso.\n\n\n\n");
			//printf("*********************************************\n\n\n\n");
		  //free(query_struct); //sirve?
			//queue_destroy(request_queue);
			//queue_destroy_and_destroy_elements(request_queue, free);
			return NULL;
		  }
		linea_script++;
	}

	//pthread_mutex_unlock(&s_requestq);
	//queue_destroy_and_destroy_elements(request_queue, free);
	//return NULL;
	return request_queue;
}


/*
 * Nueva version de armar_request_queue() que recibe un struct query en vez de un char *
 *
*/

t_queue* armar_request_queue(query un_query/*_struct*/) { //CUIDADO: esto se modifico para que no reciba el puntero, ver version vieja por las dudas
	query * query_struct = malloc(sizeof(query)); *query_struct = un_query;
	if ( query_struct->requestType == RUN )
	{
	  t_queue* request_queue = procesar_script(query_struct->script);
	  return request_queue;
	}

	t_queue* request_queue = queue_create();
	queue_push(request_queue,query_struct);
	return request_queue;

}



void * atender_conexion(void * new_fd) {
	char buf[MAXDATASIZE];
	int numbytes;
	int listening_socket = (int) new_fd;

	char *msj = "*** Conectado a Lissandra ***\n";
	char * input;
	int len, bytes_sent;
	//query query_struct;
	query * query_struct = malloc(sizeof(query));

	len = strlen(msj);
	bytes_sent = send(listening_socket, msj, len, 0);

	if ((numbytes = recv(listening_socket, buf, MAXDATASIZE - 1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	//printf("client: received %s", buf);
	log_debug(kernel_log,"Recibido el siguiente input via telnet: %s",buf);
	buf[numbytes] = '\0';
	input=string_substring(buf, 0, numbytes-2);
//	memset(&buf, 0, sizeof buf[MAXDATASIZE]);

	while ( strcasecmp(input,"salir") ) {

//		if ( string_is_empty(input) ) { continue; }
		if ( parsear(input,&query_struct) < 0 ) // Se le saca el &, volver a poner si algo male sal
		{
			//log_error(kernel_log,"Error en parseo de query, no se planifica.");
			log_error(kernel_log,"%s: comando desconocido.",input);
			printf("%s: comando desconocido.\n\n",input);
			printf("Fin de proceso.\n\n\n\n");
			printf("*********************************************\n\n\n\n");
			if ((numbytes = recv(listening_socket, buf, MAXDATASIZE - 1, 0))
				== -1) {
			perror("recv");
			exit(1);
			}
			printf("client: received %s", buf);
			buf[numbytes] = '\0';
			input=string_substring(buf, 0, numbytes-2);

			continue;
		}

		switch (query_struct->requestType)
		{
		  case METRICS: ejecutar_request(query_struct);
				break;
		  case JOURNAL: ejecutar_request(query_struct);
				break;
		  case ADD: 	ejecutar_request(query_struct);
				break;
		  default: {
				t_queue* request_queue = armar_request_queue(*query_struct); //CUIDADO: le pongo un *

				if (request_queue != NULL)
				{
		  		  agregar_a_new(request_queue);
				}
			    }
		}
		if ((numbytes = recv(listening_socket, buf, MAXDATASIZE - 1, 0))
				== -1) {
			perror("recv");
			exit(1);
		}

		printf("client: received %s", buf);
		buf[numbytes] = '\0';
		input=string_substring(buf, 0, string_length(buf) - 2); return 0;//}
		//}

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

	//printf("Planificador iniciado. Escuchando...\n");
	log_info(kernel_log,"Planificador iniciado. Escuchando...");

	while (1) {
		addr_size = sizeof their_addr;
		new_fd = accept(sckt, (struct sockaddr *) &their_addr, &addr_size);

		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr), ipstr,
				sizeof ipstr);
		log_info(kernel_log,"Nueva conexion desde: %s\n", ipstr);
		//printf("...nueva conexion desde: %s\n", ipstr);
		pthread_create(&thread_id, NULL, atender_conexion, (void *) new_fd);
		pthread_detach(thread_id);
	}
}


void *consola()
{
	 //int aux;
	 char * linea;
	 query * query_struct = malloc(sizeof(query));
	 //argumentosQuery * args;
	 //logMemTable = retornarLogConPath("Memtable.log","Memtable");
	 //system("clear");
	 printf("\n\n\n*** Conectado a Lissandra ***\n\n");
	  while(1) {
	    linea = readline("kernel> ");
	    if (!linea)
	      break;

	    if (!strcasecmp(linea,"salir")) {
	      break;
	    }

	    if (!strcasecmp(linea,"")) {
	      continue;
	    }

	    add_history(linea);

	    if (!strcasecmp(linea,"clear")) {
	      system("clear");
	    }else{

	    if ( parsear(linea,&query_struct) < 0 ) {
			log_error(kernel_log,"%s: comando desconocido.",linea);
			printf("%s: comando desconocido.\n\n",linea);
			//printf("Fin de proceso.\n\n\n\n");
			//printf("*********************************************\n\n\n\n");

			continue;
	    }

		switch (query_struct->requestType)
		{
		  case METRICS: ejecutar_request(query_struct);
				break;
		  case JOURNAL: ejecutar_request(query_struct);
				break;
		  case ADD: 	ejecutar_add(query_struct);
				break;
		  default: {
				t_queue* request_queue = armar_request_queue(*query_struct); //CUIDADO: le pongo un *

				if (request_queue != NULL)
				{
		  		  agregar_a_new(request_queue);
				}
			    }
		}
/*
		t_queue* request_queue = armar_request_queue(*query_struct); //CUIDADO: le pongo un *

		if (request_queue != NULL)
		{
		  agregar_a_new(request_queue);
		}*/
	    }
	  }
	  exit(1);
}


void *monitorear_config_thread_function()
{
	log_info(kernel_log,"**** Hilo de modificacion de configuracion inicializado ****");
	//const char * config_file = "/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/config/kernel_config.cfg";
	const char * config_file = "/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/config/";

	while(1) {
	char buffer[BUF_LEN];
	int length = 0;
	int offset = 0;
	int quantum = 0;
	long refresh_metadata = 0;
	long retardo_ejecucion = 0;
	//struct inotify_event * evento = malloc(sizeof(*evento));


    int file_descriptor = inotify_init();

	if(file_descriptor == -1)
	{
	 perror("No se pudo crear el archivo monitor");
	 exit(-1);
	}

	    int watch_descriptor = inotify_add_watch(file_descriptor,config_file,IN_MODIFY|IN_CREATE|IN_DELETE|IN_CLOSE_WRITE);//Ver mask

	    if(watch_descriptor == -1)
	    {
	        perror("No se pudo crear el monitor");
		exit(-1);
	    }

//while(1) {
	    length = read(file_descriptor,buffer,BUF_LEN);
	    while(offset < length){
		struct inotify_event * evento = (struct inotify_event *) &buffer[offset];
	        //bytes_leidos = read(archivo_monitor,buffer,BUF_LEN);
	        //Bloquea al proceso/hilo hasta que ocurra el evento declarado en el monitor

	        if(length <= 0)
	        {
	            perror("Error al leer el archivo monitor.");exit(-1);
	            break;
	        }

	    if(evento->len) {
		/*if ((evento->mask & IN_DELETE) && !strcasecmp(evento->name,".kernel_config.cfg.swp")) // Chequear IN_CLOSE_WRITE
		    { log_info(kernel_log,"**** Se elimino el siguiente archivo: %s\n",evento->name);
		    } else*/ if ((evento->mask & IN_CLOSE_WRITE) && !strcasecmp(evento->name,"kernel_config.cfg"))
		    {
			//log_info(kernel_log,"**** Paso algo con el siguiente archivo: %s\n",evento->name);

			crearConfig("/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/config/kernel_config.cfg");

			quantum = obtenerInt("QUANTUM");
			refresh_metadata = obtenerInt("REFRESH_METADATA");
			retardo_ejecucion = obtenerInt("RETARDO_EJECUCION");

			if (quantum != QUANTUM_SIZE) {
			 log_info(kernel_log,"Actualizado el valor del quantum. Viejo valor: %d, nuevo valor: %d",QUANTUM_SIZE,quantum);
			 QUANTUM_SIZE = quantum;
			}

			if (refresh_metadata != REFRESH_METADATA) {
			 log_info(kernel_log,"Actualizado el valor de refresh de metadata. Viejo valor: %d segundos, nuevo valor: %d segundos",
			   REFRESH_METADATA,refresh_metadata);
			 REFRESH_METADATA = refresh_metadata;
			}

			if (retardo_ejecucion != RETARDO_EJECUCION) {
			 log_info(kernel_log,"Actualizado el valor de retardo de ejecucion. Viejo valor: %d ms, nuevo valor: %d ms",
			   RETARDO_EJECUCION,retardo_ejecucion);
			 RETARDO_EJECUCION = retardo_ejecucion;
			 sleep_ejecucion = RETARDO_EJECUCION/1000;
			}
		    }
	    }
		offset += sizeof (struct inotify_event) + evento->len;
	    }
	    }

}


void inicializar_logs() {

	kernel_log = log_create("/home/utnso/kernel.log", "kernel.log", 0, LOG_LEVEL_INFO);

}

void inicializar_semaforos() { //TODO: creo que hay algunos que no se usan
	sem_init(&s_hay_request, 0, 0);
	sem_init(&s_hay_new, 0, 0);
	sem_init(&s_exec_request_inicial, 0, 0);
	sem_init(&s_dispatcher, 0, 1);
	sem_init(&s_hay_request_estado_exit, 0, 0);
	pthread_mutex_init(&s_newq, NULL);
	pthread_mutex_init(&s_readyq, NULL);
	pthread_mutex_init(&s_exitq, NULL);
	pthread_mutex_init(&s_lista_selects, NULL);
	pthread_mutex_init(&s_lista_inserts, NULL);
}


void cargar_configuraciones() {

	crearConfig("/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/config/kernel_config.cfg");

	//if(obtenerString("GRADO_MULTIPROCESAMIENTO"))
	//{
	   CANT_THREADS_EXEC = atoi(strdup(obtenerString("GRADO_MULTIPROCESAMIENTO")));
	//} else { exit(-1); }

	QUANTUM_SIZE = atoi(strdup(obtenerString("QUANTUM")));
	REFRESH_METADATA = atoi(strdup(obtenerString("REFRESH_METADATA")));
	RETARDO_EJECUCION = atoi(strdup(obtenerString("RETARDO_EJECUCION")));
	sleep_ejecucion = RETARDO_EJECUCION/1000;

	log_info(kernel_log,"Se cargaron los siguientes valores de configuracion:");
	log_info(kernel_log,"*	Numero de estados de ejecucion: %d",CANT_THREADS_EXEC);
	log_info(kernel_log,"*	Tamaño del quantum: %d",QUANTUM_SIZE);
	log_info(kernel_log,"*	Tiempo de actualizacion de la metadata de las tablas: %d",REFRESH_METADATA);
	log_info(kernel_log,"*	Retardo al final del ciclo de ejecucion (en milisegundos): %d",RETARDO_EJECUCION);

}


void inicializar_threads() {
	pthread_t readythread_id;
	pthread_t thread_id;
	pthread_t metricsthread_id;
	pthread_t consolathread_id;
	pthread_t monitorconfigthread_id;
	pthread_t purgarexitthread_id;
	pthread_t refrescarmemorias_id;

	pthread_create(&readythread_id, NULL, agregar_a_ready, NULL);
	pthread_detach(readythread_id);

	for (int thread = 1; thread <= CANT_THREADS_EXEC; thread++) {
		pthread_create(&thread_id, NULL, exec, (void *) thread);
		pthread_detach(thread_id);
	}

	pthread_create(&metricsthread_id, NULL, mostrar_y_purgar_metricas, NULL);
	pthread_detach(metricsthread_id);

	pthread_create(&consolathread_id, NULL, consola, NULL);
	//pthread_join(consolathread_id,NULL);
	pthread_detach(consolathread_id);

	pthread_create(&monitorconfigthread_id, NULL, monitorear_config_thread_function, NULL);
	pthread_detach(monitorconfigthread_id);

	pthread_create(&purgarexitthread_id, NULL, purgar_estado_exit, NULL);
	pthread_detach(purgarexitthread_id);

	pthread_create(&refrescarmemorias_id, NULL, refrescar_memorias, NULL);
	pthread_detach(refrescarmemorias_id);
}

void inicializar_colas() {
	new_queue = queue_create();
	ready_queue = queue_create();
	exit_queue = queue_create();
	memorias_ec = queue_create();
}

void iniciar_estructuras() {

	//inicializar_logs();
	inicializar_semaforos();
	inicializar_threads();
	inicializar_colas();

	lista_estadisticas_selects = list_create();
	lista_estadisticas_inserts = list_create();


}


int main(int argc, char *argv[]) {

	inicializar_logs();
	cargar_configuraciones();
	iniciar_estructuras();
	agregar_memorias_a_criterios();
	procesar();
	printf("Saliendo...");
	return 0;
}
