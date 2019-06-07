/*
 * kernel_api.c - API del kernel
 */


#include<stdio.h>
#include </home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/biblioteca/enumsAndStructs.h>
/*
void main(int argc, char *argv[])
{
}
*/

void journal_request()
{
	printf("Journaling solicitado con exito");
}


int ejecutar_select(query query_struct)
{

	printf("Se ejecuta el request desde la API. La tabla es: %s\n",query_struct.tabla);
	return 0;

}


int ejecutar_run(query * query_struct)
{

	printf("Se ejecuta el request desde la API. El script a ejecutar es: %s\n",query_struct->script);
	return -1;

}
