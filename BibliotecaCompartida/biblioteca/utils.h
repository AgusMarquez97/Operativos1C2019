#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <commons/log.h>

t_log * logger;

void iniciarLog(char* nombre);
void loggearTrace(char* mensaje);
void loggearInfo(char* mensaje);
void loggearWarning(char* mensaje);
void loggearError(char* mensaje);
void salirConError(char* mensaje, int socket);
void exitGracefully(int valorReturn);

#endif /* UTILS_H_ */
