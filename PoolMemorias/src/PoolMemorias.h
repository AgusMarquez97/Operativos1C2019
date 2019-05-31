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

/*
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
*/

typedef struct {
	int PUERTO;
	char* IP_FS;
	int PUERTO_FS;
	char** IP_SEEDS;
	int* PUERTO_SEEDS;
	int RETARDO_MEM;
	int RETARDO_FS;
	double TAM_MEM;
	double RETARDO_JOURNAL;
	double RETARDO_GOSSIPING;
	int MEMORY_NUMBER;
} configuracion;

void consola();
void leerArchivoConfiguracion();

#endif /* POOLMEMORIAS_H_ */
