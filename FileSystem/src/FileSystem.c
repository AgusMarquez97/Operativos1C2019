/*
 ============================================================================
 Name        : FileSystem.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "FileSystem.h"

void gestionarFileSystem()
{
	remove("Filesystem.log");
	fileSystemLog = retornarLogConPath("Filesystem.log","FileSystem");

	inicializarSemaforos();
	limpiarFileSystem();
	levantarFileSystem();
	levantarMetadata();
	//hiloDump = crearHilo(ejecutarDumping,NULL);
	//liberarNombres(); //->No liberar que luego no se pueden usar
}

void limpiarFileSystem()
{
	/*
	 * Necesito un borrado recursivo!
	 */
	borrarDirectorioVacio(puntoMontaje);
}

void levantarFileSystem()
{
	if (stat(puntoMontaje, &estado) == -1) {
		mkdir(puntoMontaje, 0700); //S_IFDIR
	}
	crearCarpetaMetadata();
	crearCarpetaTables();
	crearCarpetaBloques();
}

void crearCarpetaMetadata()
{
	carpetaMetadata = malloc(strlen(puntoMontaje) + strlen("Metadata/") + 1);
	strcpy(carpetaMetadata,puntoMontaje);
	strcat(carpetaMetadata,"Metadata/");

	if (stat(carpetaMetadata, &estado) == -1) {
		    mkdir(carpetaMetadata, 0700);
	}

	metadataBin = malloc(strlen(carpetaMetadata) + strlen("Metadata.bin") + 1);
	strcpy(metadataBin,carpetaMetadata);
	strcat(metadataBin,"Metadata.bin");

	FILE * metadata =  txt_open_for_append(metadataBin);
	txt_close_file(metadata);

	crearConfig(metadataBin);
	cambiarValorConfig("BLOCK_SIZE","64");
	cambiarValorConfig("BLOCKS","5192");
	cambiarValorConfig("MAGIC_NUMBER","LISSANDRA");
	eliminarEstructuraConfig();

	//Para el bitmap ver: https://github.com/sisoputnfrba/foro/issues/1337
	bitmapBin = malloc(strlen(carpetaMetadata) + strlen("Bitmap.bin") + 1);
	strcpy(bitmapBin,carpetaMetadata);
	strcat(bitmapBin,"Bitmap.bin");

	FILE * bitmap =  txt_open_for_append(bitmapBin);
	txt_close_file(bitmap);
}

void crearCarpetaTables()
{
	carpetaTables = malloc(strlen(puntoMontaje) + strlen("Tables/") + 1);
	strcpy(carpetaTables,puntoMontaje);
	strcat(carpetaTables,"Tables/");

	if (stat(carpetaTables, &estado) == -1) {
			mkdir(carpetaTables, 0700);
	}
}

void crearCarpetaBloques()
{
	carpetaBloques = malloc(strlen(puntoMontaje) + strlen("Bloques/") + 1);
	strcpy(carpetaBloques,puntoMontaje);
	strcat(carpetaBloques,"Bloques/");

	if (stat(carpetaBloques, &estado) == -1) {
			mkdir(carpetaBloques, 0700);
	}

	char * bloques = malloc(strlen(carpetaBloques) + strlen("1.bin") + 1); //Se crea el primer bloque del sistema

	strcpy(bloques,carpetaBloques);
	strcat(bloques,"1.bin");

	FILE * bloque =  txt_open_for_append(bloques);
	txt_close_file(bloque);
}

void borrarDirectorioVacio(char * directorio)
{
	if(rmdir(directorio)!=0)
	{
	//loggearInfoEnLog(fileSystemLog,"No se pudo borrar el directorio");
	}
}

void inicializarSemaforos()
{

}

void ejecutarDumping()
{
	while(1)
	{
	usleep(dumping*1000);

	/*
	 * Ejecutar dumpeo -> PASOS:
	 * 1° Bajar la memTable a las tablas, esto implica:
	 * Crea un archivo .tmp para guardar una lista de bloques que contendra los registros de la tabla
	 * Hay que ver como se van a asignar la cantidad de bloques
	 * Al guardar la info en bloques hay que actualizar el bitmap para indicar que los bloques estan en uso
	 * Por ultimo hay que limpiar la memTable para que la puedan volver a utilizar! -> OJO CON LA MEMORIA - FREE
	 */
	}
}

void liberarNombres()
{
	free(carpetaMetadata);
	free(metadataBin);
	free(bitmapBin);
	free(carpetaTables);
	free(carpetaBloques);
}

int tablaYaExistenteEnFS(char * pathAbsolutoCarpeta)
{
	/*
	 * Validar con mayusculas y minusculas -> el stat() no sirve ya que no diferencia
	 * mayus de minusc
	 */
	return 0;
}

void crearMetadataTabla(char * directorioTabla, query * queryCreate, int flagConsola)
{
			char * metadataTabla = malloc(strlen(directorioTabla) + strlen("Metadata") + 1);
			strcpy(metadataTabla,directorioTabla);
			strcat(metadataTabla,"Metadata");

			FILE * metadata =  txt_open_for_append(metadataTabla);
			txt_close_file(metadata);

			//Tratar de cambiar todo eso a char *

			char * consistencia = malloc(5);

			switch(queryCreate->consistencyType)
			{
			case SC:
				consistencia = strdup("SC");
				break;
			case SHC:
				consistencia = strdup("SHC");
				break;
			case EC:
				consistencia = strdup("EC");
				break;
			default: //Este error nunca deberia pasar
				loggearErrorEnLog(fileSystemLog,"Error de consistencia no detectado");
				if(flagConsola)
					printf("Error de consistencia no detectado\n");
			}

			char * particiones = malloc(30);
			sprintf(particiones,"%d",queryCreate->cantParticiones);
			char * tiempoCompactacion = malloc(30);
			sprintf(tiempoCompactacion,"%lli",queryCreate->compactationTime);

			crearConfig(metadataTabla);
			cambiarValorConfig("CONSISTENCY",consistencia);
			cambiarValorConfig("PARTITIONS",particiones);
			cambiarValorConfig("COMPACTION_TIME",tiempoCompactacion);
			eliminarEstructuraConfig();

			free(consistencia);
			free(particiones);
			free(tiempoCompactacion);
			free(metadataTabla);
			free(tamanioBloque);
}

void crearParticonesTabla(char * directorioTabla, query * queryCreate, int flagConsola)
{
			int i = 0;
			char * nroAux;
			char * nroParticion;
			FILE * particion;
			while(i < queryCreate->cantParticiones)
			{
			nroAux = malloc(100);
			sprintf(nroAux,"%d",i);

			nroParticion = malloc(strlen(directorioTabla) + 7);

			strcpy(nroParticion,directorioTabla);
			strcat(nroParticion,nroAux);
			strcat(nroParticion,".bin");

			particion =  txt_open_for_append(nroParticion);
			txt_close_file(particion);

			crearConfig(nroParticion);
			cambiarValorConfig("SIZE",tamanioBloque);
			cambiarValorConfig("BLOCKS",asignarUnBloqueBin());
			eliminarEstructuraConfig();

			free(nroParticion);
			free(nroAux);

			i++;
			}
}

int crearCarpetaTabla(query * queryCreate, int flagConsola)
{
	char * directorioTabla = malloc(strlen(carpetaTables) + strlen(queryCreate->tabla) + 2);
	strcpy(directorioTabla,carpetaTables);
	strcat(directorioTabla,queryCreate->tabla);
	strcat(directorioTabla,"/");

	if (tablaYaExistenteEnFS(directorioTabla) == 0) {
	mkdir(directorioTabla, 0700);
	}else
	{
		loggearErrorEnLog(fileSystemLog,"Error, tabla ya existe en el FileSystem");
		if(flagConsola)
			printf("Error, tabla ya existe en el FileSystem\n");
		return -1;
	}
		crearMetadataTabla(directorioTabla,queryCreate,flagConsola);
		crearParticonesTabla(directorioTabla,queryCreate,flagConsola);

		//Finalmente -> Crear un hilo detacheable que se encargue de la compactacion de esta tabla cada x tiempo
		loggearTablaCreadaOK(fileSystemLog,queryCreate,flagConsola,1);
		free(directorioTabla);

	return 0;
}

int rutinaFileSystemCreate(argumentosQuery * args)
{
	return crearCarpetaTabla(args->unaQuery,args->flag);
}

char * asignarUnBloqueBin()
{
	/*
	 * Hay que:
	 * 1° -> Obtener un bloque libre del directorio de bloques o crear dicho bloque -> hay que usar bitmap!
	 * 2° -> Habría que actualizar el archivo de metadata del sistema
	 * 3° -> Una vez obtenido el bloque, pasarlo a char * con formato de array (ver de evitar esto) y retornarlo
	 */
	return "[0]";
}

void levantarMetadata()
{

	crearConfig(metadataBin);

	tamanioBloque = strdup(obtenerString("BLOCK_SIZE"));
	cantidadBloques = obtenerInt("BLOCKS");

	eliminarEstructuraConfig();

}

