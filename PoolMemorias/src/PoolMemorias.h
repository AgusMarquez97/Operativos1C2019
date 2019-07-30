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

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>

#define PATHCONFIG "configuraciones/memoria.cfg"

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
} tablaPaginas;



//Concepto de pagina! -> dichas paginas residiran en memoria! -> se hara memcpy de esto en memoria
typedef struct{
	int64_t timestamp;
	char * value;
	int32_t key;
} pagina;

char * memoriaPrincipal; //Memoria con tamanio fijo


// - Base de la memoria
char* g_BaseMemoria;

// Tamaño de la memoria
int g_TamanioMemoria;

void consola();
void leerArchivoConfiguracion();
void reservarMemoriaPrincipal();

void conexionKernel();
void conexionFS();
void levantarServidorMemoria(char * servidorIP, char* servidorPuerto);

void procesarQuery(query* query);
void procesarSelect(query* query);
void procesarInsert(query* query);
void procesarCreate(query* query);
void procesarDescribe(query* describeQuery);
void procesarDrop(query* dropQuery);
void procesarJournal(query* journalQuery);

void loggearArchivoDeConfiguracion();
void loggearInfoServidor(char * IP, char * Puerto);
void loggearInfoCliente(char * IP, char * Puerto);
void loggearNuevaConexion(int socket);
void loggearDatosRecibidos(int socket, int datosRecibidos);

char* getCleanString(char* dirtyString);

#endif /* POOLMEMORIAS_H_ */
