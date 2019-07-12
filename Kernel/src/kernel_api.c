/*
 * kernel_api.c - API del kernel
 */


#include<stdio.h>
#include<time.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/enumsAndStructs.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/src/kernel.h>


void journal_request()
{
	printf("Journaling solicitado con exito");
}


int ejecutar_select(query * query_struct)
{

	char * nombre_tabla = query_struct->tabla;
	double *tiempo_ejecucion = malloc(sizeof(double));
	clock_t inicio,fin;
	inicio = clock();
	printf("Se ejecuta el siguiente request (Por ahora solo se muestra esto por pantalla): \"SELECT %s %d\"\n",nombre_tabla,query_struct->key);
	//En algun lugar de por aca va a estar la conexion a la memoria
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

	char * tabla = query_struct->tabla;
	printf("Se ejecuta el siguiente request: \"DROP %s\"\n",tabla);
	//Se supone que deberia consultar en la metadata para conocer la consistencia de la tabla
	char * proxima_memoria = (char *) queue_pop(memorias_ec); //Aca probablemente deberia ir un mutex
	printf("La consistencia de la tabla es EC. Se envia a la memoria: %s\n",proxima_memoria);
	queue_push(memorias_ec,proxima_memoria);
	printf("Por motivos de prueba vamos a hacer que ande el drop\n");
	return 0;

}


int ejecutar_run(query * query_struct)
{

	printf("Se ejecuta el request desde la API. El script a ejecutar es: %s\n",query_struct->script);
	return -1;

}


int derivar_request(query * query_struct)
{

	printf("Esto algun dia hara algo, pero no hoy.\n");//,query_struct->consistencyType);
	return 0;

}


int ejecutar_add(char * string_memoria)
{
	  char ** memoria = string_split(string_memoria,":");

	  if (!strcasecmp(memoria[2],"SC")) {
		printf("El criterio de la memoria es SC.\n"); 
	  } else if (!strcasecmp(memoria[2],"SHC")) {
		printf("El criterio de la memoria es SHC.\n");
	  } else if (!strcasecmp(memoria[2],"EC")) {
		printf("El criterio de la memoria es EC.\n");
	  } else {
		   printf("%s: Criterio inexistente.\n",memoria[2]);
		 }

}


int ejecutar_insert(query * query_struct)
{

	char * nombre_tabla = query_struct->tabla;
	double *tiempo_ejecucion = malloc(sizeof(double));
	clock_t inicio,fin;
	inicio = clock();
	printf("Se ejecuta el siguiente request (Por ahora solo se muestra esto por pantalla): \"INSERT %s %d\"\n",nombre_tabla,query_struct->key);

	//En algun lugar de por aca va a estar la conexion a la memoria
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
  log_info(kernel_log,"********* Se solicitan las metricas por consola ********* \n\n\n\n");

  pthread_mutex_lock(&s_lista_selects);
  metricas_reads(lista_estadisticas_selects);
  pthread_mutex_unlock(&s_lista_selects);

  pthread_mutex_lock(&s_lista_inserts);
  metricas_writes(lista_estadisticas_inserts);
  pthread_mutex_unlock(&s_lista_inserts);

  // Fata el memory load ya lo seeeeeeeeeeee

  //printf("\n\n\n\n\n\n ********* Metricas ejecutadas ********* \n\n\n\n\n\n");
  log_info(kernel_log,"********* Metricas ejecutadas ********* \n\n\n\n");
}






