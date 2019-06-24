#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

#include <commons/string.h>

//#include "utils.h"
#include "enumsAndStructs.h"
//#include "serializacion.h"
//#include "sockets.h"


int parsear(char * string_query,query **struct_query);

int string_size(char ** text);

int nombre_tabla_valido(char * text);

int valor_solo_numerico(char * text);

int tipo_consistencia_valido(char * texto);


char * consistencia_a_string(int32_t consistencia);
int string_a_consistencia(char * consistencia);


#endif /* PARSER_H_ */
