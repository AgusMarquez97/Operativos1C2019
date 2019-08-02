#include "hilos.h"

pthread_t crearHilo(void* funcion, void* param){

	pthread_t nuevoHilo;

	pthread_create(&nuevoHilo, NULL, funcion, param);

	return nuevoHilo;

}

void crearHiloDetachable(void* funcion, void* param){

	pthread_t nuevoHilo;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&nuevoHilo, &attr, funcion, param);

}

pthread_t makeDetachableThread(void* funcion, void* param)
{
	pthread_t hiloDetacheable = crearHilo(funcion,param);
	pthread_detach(hiloDetacheable);
	return hiloDetacheable;
}

int esperarHilo(pthread_t hilo){
	return pthread_join(hilo, NULL);
}
