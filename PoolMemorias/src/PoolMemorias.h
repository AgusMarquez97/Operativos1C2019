/*
 * PoolMemorias.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef POOLMEMORIAS_H_
#define POOLMEMORIAS_H_

#include <biblioteca/sockets.h>
#include <biblioteca/serializacion.h>
#include <biblioteca/mensajes.h>
#include <biblioteca/enumsAndStructs.h>
#include <biblioteca/logs.h>
#include <biblioteca/hilos.h>
#include <biblioteca/levantarConfig.h>
#include <biblioteca/utils.h>

#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "commons/bitarray.h"

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>


#define PATHCONFIG "configuraciones/memoria.cfg"

pthread_mutex_t mutex_journal;
sem_t *  semaforoMemoria;

//	---------- DEFINCION DE ESTRUCTURAS DEL SISTEMA: VER ISSUE https://github.com/sisoputnfrba/foro/issues/1319

typedef struct {
	int32_t PUERTO;
	char* IP_FS;
	int32_t PUERTO_FS;
	char** IP_SEEDS;
	int32_t * PUERTO_SEEDS;
	int32_t RETARDO_MEM;
	int32_t RETARDO_FS;
	int64_t TAM_MEM;
	int64_t RETARDO_JOURNAL;
	int64_t RETARDO_GOSSIPING;
	int32_t MEMORY_NUMBER;
} configuracion;

// - Puntero para la lectura de los campos del archivo de configuracion
configuracion *configuracionMemoria;

t_dictionary * tablaSegmentos;

// x cada segmento habra una TP:

typedef struct {
	int nroPagina; //Nro que referencia a la pagina per se
	int nroMarco; //Nro de marco que contiene la info en MP
	int flagModificado; // Usado para validar las paginas mas actuales / luego de insert !
} pagina;



//Concepto de pagina! -> dichas paginas residiran en memoria! -> se hara memcpy de esto en memoria
typedef struct{
	int64_t timestamp;
	char * value;
	int32_t key;
} registro;

// - Base de la memoria
char* memoriaPrincipal;

// Tamaño de la memoria & Tamanio de los marcos / paginas
int tamanioMemoria;
int tamanioPag;
int cantidadMarcos;

//Tamanio max del value obtenido en el handshake
int tamanioValue;


typedef struct{
	query * unaQuery;
	int flagConsola;
}argumentosQuery;

t_bitarray * marcosMemoria;

t_list * listaSegmentos;
t_list * historialPaginas;	 // contiene las paginas cargadas en memoria principal, las ultimas de la lista son las que
						//se usaron más recientemente

void consola();
void leerArchivoConfiguracion();
void reservarMemoriaPrincipal();

void conexionKernel();
int conexionFS();
void levantarServidorMemoria(char * servidorIP, char* servidorPuerto);

void procesarQuery(argumentosQuery * args);
int procesarSelect(query* selectQuery, int flagConsola);
void procesarInsert(query* queryInsert, int flagConsola);
void procesarCreate(query* queryCreate, int flagConsola);
void procesarDescribe(query* queryDescribe, int flagConsola);
void procesarDrop(query* queryDrop, int flagConsola);
void procesarJournal(query* queryJournal, int flagConsola);

void loggearArchivoDeConfiguracion();
void loggearInfoServidor(char * IP, char * Puerto);
void loggearInfoCliente(char * IP, char * Puerto);
void loggearNuevaConexion(int socket);
void loggearDatosRecibidos(int socket, int datosRecibidos);

void handshake();

char* getCleanString(char* dirtyString);

void loggearRegistroEncontrado(int key, char * value, int flagConsola);
void loggearRegistroNoEncontrado(int key, int flagConsola);
void loggearErrorTablaExistente(query * unaQuery,int flagConsola);
void loggearTablaCreadaOK(query * unaQuery,int flagConsola);
void loggearTablaDropeadaOK(char * tabla, int flagConsola);
void loggearErrorDrop(char * tabla, int flagConsola);


void refrescarPagina(char * tabla, int key, char * value, int64_t timestamp);

void levantarMarcos(int cantidadMarcos);
registro * obtenerRegistro(int nroMarco);

void agregarPagina(char * tabla, int32_t key,char * value, int64_t timestamp, int flagModificado);

bool estaLibre(int nroMarco);
void escribirMarco(int nroMarco, registro * unReg);
registro * leerMarco(int nroMarco);
void liberarMarco(int nroMarco);
int obtenerMarcoLibre();
int ejecutarLRU();

void inicializarSemaforos();

void ejecutarJournal(int flagConsola);
int journal();
void procesarJournal(query* queryJournal, int flagConsola);
void ejecutarRutinaJournal();

void enviarQuerysFS(char * tabla, t_list * registros);

void agregarAHistorialPags(pagina * unaPagina);


#endif /* POOLMEMORIAS_H_ */
