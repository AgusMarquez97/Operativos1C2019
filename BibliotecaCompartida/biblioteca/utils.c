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



/*
void loggearTrace(char* mensaje) {

	log_trace(logger, mensaje);

}
*/

