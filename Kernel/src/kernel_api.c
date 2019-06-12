/*
 * kernel_api.c - API del kernel
 */


#include<stdio.h>
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

	char * tabla = query_struct->tabla;
	printf("Se ejecuta el siguiente request: \"SELECT %s %d\"\n",tabla,query_struct->key);
	//Se supone que deberia consultar en la metadata para conocer la consistencia de la tabla
	char * proxima_memoria = (char *) queue_pop(memorias_ec); //Aca probablemente deberia ir un mutex
	printf("La consistencia de la tabla es EC. Se envia a la memoria: %s\n",proxima_memoria);
	queue_push(memorias_ec,proxima_memoria);
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
