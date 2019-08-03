// Imports y todo eso
#include <sys/inotify.h>
#include "kernel.h"







void *monitorear_config()
{

	const char * config_file = "/home/utnso/Documentos/operativos/lissandra/tp-2019-1c-Segmentation-Fault/Kernel/config/kernel_config.cfg";

	int bytes_leidos;
	struct inotify_event * evento = malloc(sizeof(*evento));


    int archivo_monitor = inotify_init();

	if(archivo_monitor == -1)
	{
	 perror("No se pudo crear el archivo monitor");
	}

	    int monitor = inotify_add_watch(archivo_monitor,config_file,IN_MODIFY);//Ver mask

	    if(monitor == -1)
	    {
	        perror("No se pudo crear el monitor");
	    }

	    while(1)
	    {

	        bytes_leidos = read(archivo_monitor,evento,sizeof(struct inotify_event)*10);
	        //Bloquea al proceso/hilo hasta que ocurra el evento declarado en el monitor

	        if(bytes_leidos <= 0)
	        {
	            perror("Error al leer el archivo monitor.");
	            break;
	        }

	        if(evento->mask == IN_MODIFY)
	        {
		  QUANTUM_SIZE = atoi(strdup(obtenerString("QUANTUM")));
		  REFRESH_METADATA = atoi(strdup(obtenerString("REFRESH_METADATA")));
		  RETARDO_EJECUCION = atof(strdup(obtenerString("RETARDO_EJECUCION")));
	        }


	    }

}
