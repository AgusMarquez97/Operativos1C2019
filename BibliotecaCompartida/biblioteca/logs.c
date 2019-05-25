#include "logs.h"

void iniciarLogConPath(char* path,char* nombre) {

	logger = log_create(path, nombre, true, LOG_LEVEL_INFO);

	loggearInfo("Logger iniciado!");

}
void iniciarLog(char* nombre) {

	logger = log_create("Lissandra.log", nombre, true, LOG_LEVEL_INFO);

	loggearInfo("Logger iniciado!");

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
