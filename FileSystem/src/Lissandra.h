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

#include <sys/types.h>
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



char * carpetaMetadata;
char * carpetaTables;
char * carpetaBloques;
char * metadataBin;
char * bitmapBin;

pthread_t hiloCompactador;



int tamanioBloque;
int cantidadBloques;

t_bitarray* unBitarray;

pthread_mutex_t mutex_bloques; //= PTHREAD_MUTEX_INITIALIZER
pthread_mutex_t mutex_bitarray;


t_log * logMemTable;

/*
 * En LFS se piensa en 2 hilos permanentes (server + consola) + Los hilos on demand que se vayan creando para atender las requests
 * Se recibirán requests por consola y por el servidor
 */

pthread_t hiloConsola, hiloServidor, hiloMonintor; // Todos estos pertenecen a la memTable

pthread_t fileSystem;


/*
 * El diccionario tendra como key al nombre de la tabla
 * Cada nombre de diccionario tendra una lista de registros definidos abajo
 * Una sola MemTable que tendrá los registros y una segunda para cuando la primera esta en dumping!
 */

t_dictionary * memTable;

struct inotify_event * evento;
t_list * hilos;

/*
 * Se define el registro de la siguiente forma
 */
typedef struct {
	int32_t key;
	char * value;
	int64_t timestamp;
}registro;

typedef struct{
	query * unaQuery;
	int flag;
}argumentosQuery;



int maxValue;
int retardo; //en milisegundos  -> puede modificarse (ver cuando y de que forma)
int dumping; //en milisegundos  -> puede modificarse (ver cuando y de que forma)
char * puntoMontaje;

void levantarConfig();

void iniciarLFS();

/*
 * Funciones que tendrán los hilos de consola y server.
 */
void consola();
void iniciarServidor();
void loggearInfoServidor(char * IP, char * Puerto);


/*
 * Funcion que se encargará de procesar la query mediante el hilo creado on demand
 */

void procesarQuery(argumentosQuery * args);

/*
 * Garantiza la mutua exclusion
 * Las validaciones sobre si existe o no la tabla se hacen a la hora de agregarMemTable
 */

void agregarAMemTable(t_dictionary * memTable, query * unaQuery, int flagConsola);

/*
 * NO garantiza mutua exclusion. Permite lecturas sucias
 */
registro * procesarSelectMemTable(query* unaQuery);

void procesarInsert(query * unaQuery, int flagConsola);

void procesarCreate(query * unaQuery, int flagConsola);

/*
 * Por desarrollar
 */
void procesarDescribe(query * unaQuery);
void procesarDrop(query * unaQuery);

//Para pruebas
/*
 * Valido para SELECT e INSERT por ahora
 */
query * crearQuery(int32_t tipoRequest, char * nombreTabla, int32_t key, char * value, int64_t timestamp);
void liberarInsert(query * unQuery);

/*
 * Para controlar la MemTable:
 */
registro * crearRegistro(int32_t key,char * value,int64_t timestamp);
void liberarRegistro(registro * unRegistro);
void imprimirListaRegistros(t_list * unaLista);
void agregarRegistro(t_list * unaLista, registro* unRegistro);
void agregarListaRegistros(t_list * lista,t_list * listaAgregar);
registro * obtenerRegistro(t_list * lista, int posicionLista);
void agregarUnRegistroMemTable(query * unaQuery, int flagConsola);
void liberarMemTable(t_dictionary ** memTable);
int obtenerTamanioRegistrosDeUnaLista(t_list * registros);
/*
 * Para casteos query -> registro
 */

registro * castearRegistroQuery(query * unaQuery);
char * castearRegistroString(registro * unRegistro);

/*
 * Para logs y prints
 */

void loggearListaRegistros(t_list * unaLista);
void errorTablaNoCreada(char * tabla);
void imprimirMemTable(t_dictionary * memTable);
void loggearMemTable(t_dictionary * memTable);
void loggearSelectMemT(query* unaQuery);
void loggearRegistroEncontrado(int key, char * value, int flagConsola);
void loggearRegistroNoEncontrado(int key, int flagConsola);
void loggearTablaNoEncontrado(char * tabla, int flagConsola);
void loggearNuevaConexion(int socket);
void loggearTablaCreadaOK(t_log * loggeador,query * unaQuery,int flagConsola, int flagFS);
void loggearErrorTablaExistente(query * unaQuery,int flagConsola);

void loggearTablaDropeadaOK(char * tabla, int flagConsola);
void loggearErrorDrop(char * tabla, int flagConsola);

void gestionarFileSystem();

registro * rutinaFileSystemSelect(char * tabla, int32_t key);
int rutinaFileSystemCreate(argumentosQuery * args);
int rutinaFileSystemDrop(char * tabla);
char * rutinaFileSystemDescribe(char * tabla);
char * obtenerNombre(char * ruta);

void terminarAplicacion();
/*
 * Se utiliza inotify para notificar frente a eventos de
 * modificacion en el archivo de configuracion
 */

void monitorearConfig();

/*
 * Actualizar el config implica:
 *		1) Notificar al resto -> enviando los nuevos valores
 *		2) Refrescar variables globales locales
 */
void actualizarConfig();
void actualizarVariablesGlobales();
void reenviarConfig();
/*
 	 	 Estructura del evento:
   	   	   struct inotify_event {
               int      wd;  -> Monitor
               uint32_t mask; -> Evento
               uint32_t cookie; -> Para conectar multiples eventos
               uint32_t len;  -> Longitud del nombre del archivo que genero el evento
               char     name[]; -> Nombre del archivo que genero el evento
           };
 */

void terminarHilo(pthread_t * unHilo);

void levantarMemTable();
char * obtenerRegistrosArchivo(char * ruta);
void borrarBloques(char * rutaArchivo);
char * castearRegistrosChar(int tamanioNecesario,t_list * listaRegistros);
int obtenerCantidadBloques(int tamanio);
int  buscarPrimerBloqueLibre();
char * castearBloquesChar(int lista_bloques[]);
void escribirBloques(int cantidadFinal,int cantidadDeBloques,int listaBloques[], char * listaRegistros);



#endif /* LISSANDRA_H_ */
