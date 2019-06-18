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


t_log * logMemTable;

/*
 * En LFS se piensa en 2 hilos permanentes (server + consola) + Los hilos on demand que se vayan creando para atender las requests
 * Se recibirán requests por consola y por el servidor
 */

pthread_t hiloConsola,hiloServidor;

pthread_t fileSystem;


/*
 * El diccionario tendra como key al nombre de la tabla
 * Cada nombre de diccionario tendra una lista de registros definidos abajo
 * Una sola MemTable que tendrá los registros y una segunda para cuando la primera esta en dumping!
 */

t_dictionary * memTable;

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
void procesarSelect(query* unaQuery, int flagConsola);

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
void loggearRegistroEncontrado(char * value, int flagConsola);
void loggearRegistroNoEncontrado(char * tabla, int flagConsola);
void loggearNuevaConexion(int socket);
void loggearTablaCreadaOK(t_log * loggeador,query * unaQuery,int flagConsola, int flagFS);
void loggearErrorTablaExistente(query * unaQuery,int flagConsola);


void gestionarFileSystem();

int rutinaFileSystemSelect(registro * maximoRegMemTable,argumentosQuery * args);
int rutinaFileSystemCreate(argumentosQuery * args);
int rutinaFileSystemDrop(argumentosQuery * args);
int rutinaFileSystemDescribe(argumentosQuery * args);





#endif /* LISSANDRA_H_ */
