/*
 * FileSystem.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#include "Lissandra.h"


#define CANTIDAD_MAXIMA_BLOQUES 100


struct stat estado = {0};
/*	Para info sobre un archivo -> usar: stat(pathArchivo,&estructuraBuffer)
    struct stat {
               dev_t     st_dev;          ID of device containing file
               ino_t     st_ino;          inode number
               mode_t    st_mode;         protection
               nlink_t   st_nlink;        number of hard links
               uid_t     st_uid;          user ID of owner
               gid_t     st_gid;          group ID of owner
               dev_t     st_rdev;         device ID (if special file)
               off_t     st_size;         total size, in bytes
               blksize_t st_blksize;      blocksize for filesystem I/O
               blkcnt_t  st_blocks;       number of 512B blocks allocated

               Para validaciones con archivos regulares y directorios:

               stat(pathname, &sb);
                if (sb.st_mode == S_IFREG) {
                Handle regular file
           }
           if (sb.st_mode == S_IFDIR) {
                 Handle directory
            }

 */


void gestionarFileSystem();

/*
 * Para el bitmap usar las commons -> <commons/bitarray.h>
 */

/*
 * Crea la carpeta punto de montaje + las carpetas administrativas
 */
void levantarFileSystem();

/*
 *	Crea la carpeta de metadata + archivos metadata.bin y bitmap.bin
 */
void crearCarpetaMetadata();
void crearCarpetaTables();
void crearCarpetaBloques();

void inicializarSemaforos();

/*
 * Crea las estructuras administrativas para la tabla
 */
void crearMetadataTabla(char * directorioTabla, query * queryCreate, int flagConsola);

/*
 * Crea el archivo metadata para la tabla
 */
int crearCarpetaTabla(query * queryCreate, int flagConsola);



/*
 *	Rutina que manda a ejecutar LFS con un hilo frente a CREATE
 */

int rutinaFileSystemCreate(query * unaQuery, int flag);
int rutinaFileSystemDrop(char * tabla);
char * rutinaFileSystemDescribe(char * tabla);

char * asignarUnBloqueBin();

void levantarMetadata();

void terminarAplicacion();
/*
 * Libera todos los logs del modulo
 */

void liberarLogs();

/*
 * Mata a todos los hilos
 */

void liberarHilos();

/*
 * Libera todas las variables globales char *
 */

void liberarNombres();

//toma un bitarray y la cantidad de bloques del file system y retorna el numero del bloque que esta disponible o -1 en caso de no haber bloques disponibles

int buscarPrimerBloqueLibre();
char * obtenerSiguienteTmp(char * nombreTabla);
char * castearBloquesChar(int lista_bloques[]);
int obtenerCantidadBloques(int tamanio);
void escribirBloques(int cantidadFinal,int cantidadDeBloques,int listaBloques[], char * listaRegistros);
char * castearRegistrosChar(int tamanioNecesario,t_list * listaRegistros);

char * asignarBloques(int tamanioNecesario, char * nombreTabla,char * listaRegistros);

void levantarMemTable();
int recorrer_directorio(const char *path, const struct stat *datosArchivo,int flags);
void recorrerDirectorio(char * directorio);

char * leerBloque(char *  numeroBloque,int bytesALeer);
char * obtenerRegistrosArchivo(char * ruta);
registro * rutinaFileSystemSelect(char * tabla, int32_t key);
t_list * leerTabla(char * tabla,int32_t key);
t_list* obtenerRegistros(char * registros,int32_t key);

int eliminarDirectorio(char * directorio);
void borrarBloques(char * rutaArchivo);
int cantidadTablas();
char * obtenerMetadaTabla(char * rutaTabla);
char * obtenerMetadataTablas();
char * obtenerNombre(char * ruta);

void compactar(query * queryCreate);
void renombrarArchivosTemporales(char* rutaDirectorioTabla);


#endif /* FILESYSTEM_H_ */
