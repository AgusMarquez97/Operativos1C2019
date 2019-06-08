#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

//#include "utils.h"
#include "enumsAndStructs.h"
//#include "serializacion.h"
//#include "sockets.h"

int ERROR = -1;

int parsear(char * string_query,query *struct_query);

int string_size(char ** text);

int nombre_tabla_valido(char * text);

int valor_solo_numerico(char * text);

int tipo_consistencia_valido(char * texto);

#endif /* PARSER_H_ */
