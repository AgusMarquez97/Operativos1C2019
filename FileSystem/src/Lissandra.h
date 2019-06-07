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

#include <readline/readline.h>
#include <readline/history.h>

/*
 * En LFS se piensa en 2 hilos permanentes (server + consola) + Los hilos on demand que se vayan creando para atender las requests
 * Se recibirán requests por consola y por el servidor
 */

pthread_t hiloConsola,hiloServidor;
pthread_mutex_t mutex_Mem_Table;

/*
 * El diccionario tendra como key al nombre de la tabla
 * Cada nombre de diccionario tendra una lista de registros definidos abajo
 * Una sola MemTable que tendrá los registros y una segunda para cuando la primera esta en dumping!
 */

t_dictionary * memTable;
t_dictionary * memTableBackUp;

/*
 * Se define el registro de la siguiente forma
 */
typedef struct {
	int32_t key;
	char * value;
	int64_t timestamp;
}registro;

void iniciarLFS();
void inicializarSemaforos();

/*
 * Funciones que tendrán los hilos de consola y server.
 */
void consola();
void iniciarServidor();
void loggearInfoServidor(char * IP, char * Puerto);


/*
 * Funcion que se encargará de procesar la query mediante el hilo creado on demand
 */

void procesarQuery(query * unaQuery);

/*
 * Garantiza la mutua exclusion
 * Las validaciones sobre si existe o no la tabla se hacen a la hora de agregarMemTable
 */

void procesarInsert(query * unaQuery);

/*
 * NO garantiza mutua exclusion. Permite lecturas sucias
 */
void procesarSelect(query * unaQuery);


/*
 * Por desarrollar
 */
void procesarCreate(query * unaQuery);
void procesarDescribe(query * unaQuery);
void procesarDrop(query * unaQuery);

//Para pruebas
query * crearInsert(char * nombreTabla,int32_t key,char * value,int64_t timestamp);
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
void agregarUnRegistroMemTable(query * unaQuery);
void agregarAMemTable(t_dictionary * memTable, query * unaQuery);
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
void warningTablaNoCreada(char * tabla);
void imprimirMemTable(t_dictionary * memTable);
void loggearMemTable(t_dictionary * memTable);




#endif /* LISSANDRA_H_ */
