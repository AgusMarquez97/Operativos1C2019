#include "Compactador.h"
#include "FileSystem.h"
#include "Lissandra.h"

pthread_t lissandra;
pthread_t fileSystem;
pthread_t compactador;


int main(void) {
	lissandra = crearHilo(iniciarLFS,NULL);
	esperarHilo(lissandra); //Luego ver si detachear -> ¿En que situacion NORMAL finalizaria el LFS?
	return EXIT_SUCCESS;

}
