#ifndef HILOS_H_
#define HILOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_t crearHilo(void* funcion, void* param);
void crearHiloDetachable(void* funcion, void* param);
int esperarHilo(pthread_t hilo);
pthread_t makeDetachableThread(void* funcion, void* param);

#endif /* HILOS_H_ */
