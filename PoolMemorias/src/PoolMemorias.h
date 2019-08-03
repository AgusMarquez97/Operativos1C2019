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
#include <biblioteca/parser.h>

#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

#include <time.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/inotify.h>


#include <signal.h>
#include "commons/bitarray.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

#include "commons/txt.h"
#include "commons/string.h"
#include <signal.h>
#include "commons/collections/list.h"
#include <math.h>
#include <time.h>
#include <ftw.h>
#include <dirent.h>

#include <semaphore.h>


#define PATHCONFIG "/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/PoolMemorias/configuraciones/memoria.cfg"

#define EVENT_SIZE (sizeof(struct inotify_event)+24)
#define BUF_LEN (1024*EVENT_SIZE)


typedef struct{
	int nroMemoria;
	char * NroIPPuerto;
	bool activa;
} estructuraGossip;

estructuraGossip * datosMemoria;
t_list * listaGossip; // cuyos elementos seran de tipo * estructuraGossip


int criterioConsistencia = EC;


char * IPMemoria;
char * puertoMemoria;


pthread_t hiloMonitor, hiloJournal, hiloServidor, hiloGossip;

pthread_mutex_t mutex_marcos_libres;
pthread_mutex_t mutex_journal, mutex_journal_LRU, mutex_gossip;

//	---------- DEFINCION DE ESTRUCTURAS DEL SISTEMA: VER ISSUE https://github.com/sisoputnfrba/foro/issues/1319

typedef struct {
	char * PUERTO;
	char * IP;

	char* IP_FS;
	char * PUERTO_FS;

	char ** IP_SEEDS;
	char ** PUERTO_SEEDS;
	char *  MEMORY_NUMBER;

	int32_t RETARDO_MEM;
	int32_t RETARDO_FS;
	int64_t TAM_MEM;
	int64_t RETARDO_JOURNAL;
	int64_t RETARDO_GOSSIPING;

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
	int socketCliente;
}argumentosQuery;

t_bitarray * marcosMemoria;

t_list * listaSegmentos;
t_list * historialPaginas;
// contiene las paginas cargadas en memoria principal, las ultimas de la lista son las que
// se usaron más recientemente

void consola();
void leerArchivoConfiguracion();
void reservarMemoriaPrincipal();

void conexionKernel();
int conexionFS();
void levantarServidorMemoria();

void procesarQuery(argumentosQuery * args);

/*
 * Nos llega un select (EVENTUAL CONSISTENCY) =>
 * CASO A:
 * 		EL SEGMENTO YA EXISTE EN LA TABLA DE SEGMENTOS
 * 			CASO A1:
 * 				LA KEY SE ENCUENTRA EN MEMORIA => SE RETORNA EL VALUE & SE AGREGAN LAS PAGINAS USADAS
 * 			CASO A2:
 * 				LA KEY NO SE ENCUENTRA EN MEMORIA => SE BUSCA EN FS
 * 					CASO A2.1:
 * 						LA KEY ESTA EN FS => SE RETORNA EL RESULTADO & SE AGREGAN LAS PAGINAS USADAS & SE AGREGA LA PAG A MEM
 * 					CASO A2.2:
 * 						INFORMAR QUE LA KEY NO EXISTE O NO TIENE VALUE ASOCIADO
 * 	CASO B
 * 		SE BUSCA EN FS -> SE RETORNA EL RESULTADO & SE CARGA EN MEMORIA CON FLAG DE MOD = 0
 * 			{Se crea el segmento y se agrega la pagina al seg (Y A MP) + se carga al historial de paginas}
 *
 * 	STRONG CONSISTENCY:
 *	BUSCAR LA KEY EN MP Y EN FS:
 *		CASO 1: EXISTE LA KEY EN AMBOS => RETORNAMOS LA DE MAYOR TIMESTAMP
 *		CASO 2: EXISTE LA KEY EN ALGUNO => RETORNAMOS LA ENCONTRADA
 *		CASO 3: INFORMAMOS EL ERROR
 *
 */

void procesarSelect(query* selectQuery, int flagConsola);

void procesarInsert(query* queryInsert, int flagConsola);

void procesarCreate(query* queryCreate, int flagConsola);

void procesarDescribe(query* queryDescribe, int flagConsola);

void procesarDrop(query* queryDrop, int flagConsola);

void procesarJournal(query* queryJournal, int flagConsola);

void loggearArchivoDeConfiguracion();
void loggearInfoServidor(char * IP, char * Puerto);
void loggearInfoFileSystem(char * IP, char * Puerto);
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
registro * obtenerRegistro(char * tabla, int key);

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

bool existeEnMemoria(char * tabla);
void eliminarDeHistorial(pagina * unaPagina);

int obtenerCriterioConsistencia(char * tabla);

void buscarRegistroFS(query * selectQuery, int flagConsola);

void monitorearConfig();
void actualizarConfig();


void ejecutarGossping();

int obtenerCantidadMarcos(int tamanioPagina, int tamanioMemoria);


void handshakeKernel();
void handshakeFS();

void cambiarConsistencia();

void levantarPoolMemorias();

void agregarPool(char * datosMemorias);
bool existeEnElPool(int nro);
void enviarPoolKernel(int socketKernel);
int obtenerTamanioGossipActual();
void enviarPoolMemoria(int socketMemoria);
bool existeEnElPool(int nro);

void ejecutarLQL(argumentosQuery * args);

bool estaLlena();

void liberarMarcos();


#endif /* POOLMEMORIAS_H_ */
