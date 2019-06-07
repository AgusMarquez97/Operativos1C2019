
#include "Compactador.h"
#include "FileSystem.h"
#include "Lissandra.h"

pthread_t lissandra;
pthread_t fileSystem;
pthread_t compactador;


int main(void) {

	 /*
	 query * clienteJose = crearInsert("Clientes",1,"Jose Ernesto",123456);
	 query * clientePepe = crearInsert("Clientes",2,"Pepe asdasdasdasd",64564);
	 query * empresa = crearInsert("Empresas",1,"Ricardo y asociados",123156);

	 agregarAMemTable(memTable,clienteJose);
	 agregarAMemTable(memTable,clientePepe);
	 agregarAMemTable(memTable,empresa);


	 loggearMemTable(memTable);


	 liberarInsert(clienteJose);
	 liberarInsert(clientePepe);
	 liberarInsert(empresa);
	 liberarMemTable(&memTable);
*/

	lissandra = crearHilo(iniciarLFS,NULL);
	esperarHilo(lissandra); //Luego ver si detachear -> ¿En que situacion NORMAL finalizaria el LFS?->NUNCA

	return EXIT_SUCCESS;

}
