#include "logs.h"

void iniciarLogConPath(char* path,char* nombre) {

	logger = log_create(path, nombre, true, LOG_LEVEL_INFO);

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
