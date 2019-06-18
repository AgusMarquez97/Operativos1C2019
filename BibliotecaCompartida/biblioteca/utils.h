#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <commons/process.h>
#include <commons/temporal.h>
#include <commons/string.h>

/*
 * Para obtener el ID del proceso / hilo actual
 */

int ObtenerHilo();
int ObtenerProceso();


/*
 * Para obtener la hora en formato string
 */
char * ObtenerTimeStamp();

char * eliminarComillas(char * cadena);




#endif /* UTILS_H_ */
