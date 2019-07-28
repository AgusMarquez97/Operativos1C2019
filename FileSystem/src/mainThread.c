#include "Lissandra.h"
//#include "FileSystem.h"

pthread_t lissandra;

int main() {

	//signal(SIGINT,terminarAplicacion);
	//signal(SIGTERM,terminarAplicacion);


	hilos = list_create();
	lissandra = crearHilo(iniciarLFS,NULL);
	esperarHilo(lissandra);//Luego ver si detachear -> ¿En que situacion NORMAL finalizaria el LFS?->NUNCA

	return EXIT_SUCCESS;

}
