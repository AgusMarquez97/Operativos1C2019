#include "utils.h"


int get_thread_id()
{
	return process_get_thread_id();
}

int get_process_id()
{
	return process_getpid();
}


char * get_Time()
{
	return temporal_get_string_time();
}

char * eliminarComillas(char * cadena)
{
	if(strlen(cadena) > 2)
	return string_substring(cadena,1,strlen(cadena) - 3);
	return "";
}

char * eliminarComillasMemoria(char * cadena)
{
	if(strlen(cadena) > 2)
	return string_substring(cadena,1,strlen(cadena) - 2);
	return "";
}

/*
void loggearTrace(char* mensaje) {

	log_trace(logger, mensaje);

}
*/

