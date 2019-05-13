#include "utils.h"

void iniciarLog(char* nombre) {

	logger = log_create("Lissandra.log", nombre, true, LOG_LEVEL_TRACE);
	loggear("Logger iniciado!");

}

void loggearTrace(char* mensaje) {

	log_trace(logger, mensaje);

}

void loggearInfo(char* mensaje) {

	log_info(logger, mensaje);

}

void loggearWarning(char* mensaje) {

	log_warning(logger, mensaje);

}

void salirConError(char* mensaje, int socket) {

	log_error(logger, mensaje);
	log_error(logger, "Error de errno: %s", strerror(errno));

	close(socket);

	exitGracefully(1);


}

void exitGracefully(int valorReturn) {
	exit(valorReturn);
}


