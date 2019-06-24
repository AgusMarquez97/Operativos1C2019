#include "logs.h"

void iniciarLogConPath(char* path,char* nombre) {

	logger = log_create(path, nombre, 0, LOG_LEVEL_INFO);

}
void iniciarLog(char* nombre) {

	logger = log_create("Lissandra.log", nombre, 0, LOG_LEVEL_INFO);

}


void loggearInfo(char* mensaje) {

	log_info(logger, mensaje);

}

void loggearWarning(char* mensaje) {

	log_warning(logger, mensaje);

}

void loggearError(char* mensaje) {

	log_error(logger, mensaje);
	log_error(logger, "Error de errno: %s", strerror(errno));

}

void loggearInfoConcatenandoDosMensajes(char* mensaje1, char* mensaje2){
	char *result = malloc(strlen(mensaje1) + strlen(mensaje2) + 1);
	strcpy(result, mensaje1);
	strcat(result, mensaje2);
	log_info(logger, result);
	free(result);
}


t_log * retornarLogConPath(char* path,char* nombre)
{
	return log_create(path, nombre, 0, LOG_LEVEL_INFO);
}

void loggearInfoEnLog(t_log * unLog,char* mensaje)
{
	log_info(unLog, mensaje);
}
void loggearWarningEnLog(t_log * unLog,char* mensaje)
{
	log_warning(unLog, mensaje);
}
void loggearErrorEnLog(t_log * unLog,char* mensaje)
{
	log_error(unLog, mensaje);
	log_error(unLog, "Error de errno: %s", strerror(errno));
}


void loggearInfo3Mensajes(char * mensajePrincipal, char * mensajeAConcatenar, char * mensajeFinal)
{
	char * log = malloc(strlen(mensajePrincipal) + strlen(mensajeAConcatenar) + strlen(mensajeFinal) + 1);

	strcpy(log,mensajePrincipal);
	strcat(log,mensajeAConcatenar);
	strcat(log,mensajeFinal);

	loggearInfo(log);

	free(log);
}
