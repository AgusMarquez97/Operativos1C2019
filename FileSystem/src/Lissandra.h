/*
 * Lissandra.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef LISSANDRA_H_
#define LISSANDRA_H_


#include <biblioteca/sockets.h>
#include <biblioteca/serializacion.h>
#include <biblioteca/mensajes.h>
#include <biblioteca/enumsAndStructs.h>
#include <biblioteca/logs.h>
#include <biblioteca/hilos.h>
#include <biblioteca/levantarConfig.h>
#include <biblioteca/utils.h>

#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

#include <time.h>


pthread_mutex_t mutex_Mem_Table;

//Formato en que se guardarán los registros: NO SE HACE REFERENCIA CRUZADA CON EL NOMBRE DE LA TABLA
typedef struct {
	int key;
	char * value;
	double timestamp;
}registro;

//Apunta a una Key de tabla + una diccionario. Habrá muchas tablas y cada tabla tendra una lista de registros

typedef struct {
	char ** nombresTablas;
	registro * registros;
	int indice;
	//size_t tam;
}baseTemporal;

//Habrá una sola memTable + 1 copia para evitar bloqueos(al recibir request) cuando se esta haciendo dumping
baseTemporal memTable;
baseTemporal memTableSecundaria;

//En principio se piensa en 3 hilos
pthread_t hiloConsola,hiloMemTable,hiloServidor;


void iniciarLFS();

void consola();
void iniciarServidor();

void inicializarSemaforos();

void procesarQuery(query * unaQuery);
void procesarInsert(query * unaQuery);
void agregarUnRegistroMemTable(char * nombreTabla,int key, char * value, double timestamp);
void procesarSelect(query * unaQuery);

//Por ahora recibe de una query a la vez!

bool existeTabla(char * nombreTabla);

size_t tamanioRegistro(registro * nombreTabla);





#endif /* LISSANDRA_H_ */
