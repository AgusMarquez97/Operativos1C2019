/*
 * kernel_api.c - API del kernel
 */


#include<stdio.h>
#include<time.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/enumsAndStructs.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/src/kernel.h>
/*
void main(int argc, char *argv[])
{
}
*/

void journal_request()
{
	printf("Journaling solicitado con exito");
}


int ejecutar_select(query * query_struct)
{

	char * nombre_tabla = query_struct->tabla;
	//double tiempo_ejecucion;
	double *tiempo_ejecucion = malloc(sizeof(double));
	clock_t inicio,fin;
	inicio = clock();
	printf("Se ejecuta el siguiente request (Por ahora solo se muestra esto por pantalla): \"SELECT %s %d\"\n",nombre_tabla,query_struct->key);
	//En algun lugar de por aca va a estar la conexion a la memoria
	fin = clock();

	*tiempo_ejecucion = ((double) (fin - inicio)) / CLOCKS_PER_SEC; // En segundos

	//printf("El tiempo total de ejecucion fue de %f segundos.\n",*tiempo_ejecucion);

	t_lista_metricas *struct_operacion = malloc(sizeof(t_lista_metricas));

	time_t lahora = time(NULL);
	(*struct_operacion).timestamp_operacion = (unsigned) lahora;
	(*struct_operacion).duracion_operacion = *tiempo_ejecucion;


	pthread_mutex_lock(&s_lista_selects);
	list_add(lista_estadisticas_selects,struct_operacion);
	pthread_mutex_unlock(&s_lista_selects);

	//printf("El timestamp de la operacion fue: %u segundos (sacado del struct). En formato string es: %s\n",
	 //(*struct_operacion).timestamp_operacion,asctime(gmtime(&lahora)));

	//printf("El tiempo total de ejecucion fue de %f segundos (sacado del struct).\n",(*struct_operacion).duracion_operacion);



// Lo mismo pero sacado de la lista
/*
	t_lista_metricas *struct_operacion_2 = malloc(sizeof(t_lista_metricas));

	printf("El tamaño de la lista es: %d\n",list_size(lista_estadisticas_selects));

	*struct_operacion_2 = *((t_lista_metricas *) list_get(lista_estadisticas_selects,0));

	(*struct_operacion).timestamp_operacion++;
	(*struct_operacion).duracion_operacion++;

	printf("El timestamp de la operacion fue: %u segundos (sacado de la lista). En formato string es: %s\n",
	 (*struct_operacion_2).timestamp_operacion,asctime(gmtime(&lahora)));

	printf("El tiempo total de ejecucion fue de %f segundos (sacado de la lista).\n",(*struct_operacion_2).duracion_operacion);
*/
//printf("El tiempo total de ejecucion fue de %f segundos (sacado de la lista).\n",(*struct_operacion).duracion_operacion);


//	char * tipo_consistencia = obtener_consistencia_tabla(nombre_tabla);
//	char * memoria_a_utilizar = obtener_memoria_segun_criterio(tipo_consistencia);
	char * ip_memoria; //= hacer el split de la memoria 
	char * puerto_memoria; //= hacer el split de la memoria 
	//Se supone que deberia consultar en la metadata para conocer la consistencia de la tabla
//	char * proxima_memoria = (char *) queue_pop(memorias_ec); //Aca probablemente deberia ir un mutex
//	printf("La consistencia de la tabla es EC. Se envia a la memoria: %s\n",proxima_memoria);
//	queue_push(memorias_ec,proxima_memoria);
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



unsigned get_timestamp()
{

	time_t ltime;
	ltime = time(NULL);
	//char * retorno = (char *) asctime(localtime(&ltime));
	//return asctime(localtime(&ltime));
	//printf("Timestamp de la operacion: %s\n",asctime(localtime(&ltime)));

	fprintf(stdout,"%un",(unsigned)time(NULL));

	unsigned retorno = time(NULL);
	printf("El retorno essssssssssssSS: %u\n",retorno);

	

	return retorno;

}






