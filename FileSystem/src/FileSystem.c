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
	hiloDump = crearHilo(ejecutarDumping,NULL);
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
	cambiarValorConfig("BLOCK_SIZE","16");
	cambiarValorConfig("BLOCKS","1024");
	cambiarValorConfig("MAGIC_NUMBER","LISSANDRA");
	cantidadBloques = obtenerInt("BLOCKS");
	eliminarEstructuraConfig();

	//Para el bitmap ver: https://github.com/sisoputnfrba/foro/issues/1337
	bitmapBin = malloc(strlen(carpetaMetadata) + strlen("Bitmap.bin") + 1);
	strcpy(bitmapBin,carpetaMetadata);
	strcat(bitmapBin,"Bitmap.bin");

		int fd = open(bitmapBin,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); //se abre si existe, sino se crea
	    char* bitmap;
	    ftruncate(fd,cantidadBloques/8);
	    bitmap= mmap(0,cantidadBloques/8,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0); //se baja el archivo a memoria

		unBitarray = bitarray_create_with_mode(bitmap, cantidadBloques/8, LSB_FIRST);

		for(int i = 0; i<cantidadBloques;i++){
			bitarray_clean_bit(unBitarray,i); //seteo todos los bits en 0 (al principio todos los bloques están libres)
		}
		//munmap(bitmap,cantidadBloques/8);

	//FILE * bitmap =  txt_open_for_append(bitmapBin);
	//txt_close_file(bitmap);
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

	int i = 0;
	char *s;
	char * blocks;

	crearConfig(metadataBin);
	cantidadBloques = obtenerInt("BLOCKS");
	eliminarEstructuraConfig();

	while(i < cantidadBloques){
		s= string_itoa(i);
		strcat(s,".bin");
		blocks = malloc(strlen(carpetaBloques)+strlen(s)+1);
		strcpy(blocks,carpetaBloques);
		strcat(blocks,s);
		FILE * block =  txt_open_for_append(blocks);
		txt_close_file(block);
		i++;
	}
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
	pthread_mutex_init(&mutex_bloques, NULL);
	pthread_mutex_init(&mutex_bitarray, NULL);
}

void ejecutarDumping()
{
	while(1)
	{
	usleep(dumping*1000);

	int tamNecesario = 0;
	void dumpearTabla(char * tabla, t_list * listaRegistros)
	{
		//Luego crear un hilo detacheable por cada una de estas

		tamNecesario = obtenerTamanioRegistrosDeUnaTabla(tabla);
		asignarBloques(tamNecesario,tabla,listaRegistros); // Libera la memoria

		/*
		 *
		typedef struct
		{
			int tamanio;
			char * nombreTabla;
			t_list * listaRegistros
		}params;

		params hilo;

		hilo->tamanio = tamNecesario;
		hilo->nombreTabla = strdup(tabla);
		hilo->listaRegistros = listaRegistros;

		pthread_t hiloTabla = crearHilo((void*)asignarBloques,params);
		Nota: Habria que cambiar asignarBloques para que reciba la estructura params
		*/

	}
	dictionary_iterator(memTable,(void*)dumpearTabla);
	dictionary_destroy_and_destroy_elements(memTable,free);
	memTable = dictionary_create();
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

			char * aux = malloc(sizeof(int)*2);
			sprintf(aux,"%d",tamanioBloque);

			crearConfig(nroParticion);
			cambiarValorConfig("SIZE",aux);
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

	int aux = buscarPrimerBloqueLibre();
	char* bloque= malloc(10);
	strcpy(bloque,"[");
	strcat(bloque,string_itoa(aux));
	strcat(bloque,"]");
	free(bloque);
	return bloque;
}


int obtenerCantidadBloques(int tamanio)
{
	float tam_nec = (float)tamanio/tamanioBloque;
	if(tam_nec == (int)tam_nec)
	return (int)tam_nec;
	return (int)round(tam_nec + 0.5);
}

//Este libera los registros!
char * castearRegistrosChar(int tamanioNecesario,t_list * listaRegistros)
{
	char * registros = malloc(tamanioNecesario);
	strcpy(registros,"");

	char * aux = malloc(100);

	void agregarRegistro(registro * unRegistro)
	{
		//key;value;timestamp\n

		sprintf(aux,"%d",unRegistro->key);

		strcat(registros,aux);
		strcat(registros,";");

		strcat(registros,unRegistro->value);
		strcat(registros,";");

		sprintf(aux,"%lli",unRegistro->timestamp);

		strcat(registros,aux);

		strcat(registros,"\n");

		free(unRegistro->value);
	}

	list_iterate(listaRegistros,(void*)agregarRegistro);

	free(aux);
	return registros;
}

void escribirBloques(int cantidadFinal,int cantidadDeBloques,int listaBloques[], char * listaRegistros)
{
	/*
	 * Aca hay que escribir en el bloque con el formato estipulado
	 * Una posible forma:
	 * Meter toda la lista de registros en un char * con el formato
	 * key1;value1;timestamp1\nkey2;value2;timestamp2\n ...etc
	 * Luego del char *, usar un offset para ir escribiendo cada bloque
	 * hasta el tam del bloque.
	 */

	FILE * bloque;
	int aux = 0;
	char * aux2 = malloc(100);
	sprintf(aux2,"%d",cantidadDeBloques);
	char * nombreBloque = malloc(strlen(carpetaBloques) + strlen(aux2) + 5 + 2 + strlen(".bin"));
	while(listaBloques[aux] != -1)
	{

		strcpy(nombreBloque,carpetaBloques);
		sprintf(aux2,"%d",listaBloques[aux]);
		strcat(nombreBloque,aux2);
		strcat(nombreBloque,".bin");

		bloque =  txt_open_for_append(nombreBloque);

		if(aux == cantidadDeBloques)
		{
			//escribir lo que entre en el ultimo bloque
			fwrite(string_substring(listaRegistros,strlen(listaRegistros) - cantidadFinal,cantidadFinal),cantidadFinal,1,bloque);
			txt_close_file(bloque);
			break;
		}
		fwrite(string_substring(listaRegistros,tamanioBloque*aux,tamanioBloque),tamanioBloque,1,bloque);
		txt_close_file(bloque);
		aux++;
	}
	free(aux2);
	free(nombreBloque);
}

char * asignarBloques(int tamanioNecesario, char * nombreTabla,t_list * listaRegistros) //retorna el nombre del .tmp
{

	int cantBloques = obtenerCantidadBloques(tamanioNecesario);

	int listaBloques[cantBloques];

	for(int i = 0; i < cantBloques; i++)
	{
		//pthread_mutex_lock(&mutex_bloques);
		listaBloques[i] = buscarPrimerBloqueLibre();
		//pthread_mutex_unlock(&mutex_bloques);
	}

	char * aux_tmp = malloc(1000);
	listaBloques[cantBloques] = -1; //Valor centinela de corte

	char * bloques = strdup(castearBloquesChar(listaBloques));

	char * nombreTemp = malloc(strlen(carpetaTables) + strlen(nombreTabla) + strlen("9999.tmp") + 2 + 20); // Hasta 9999 posibles .tmp

	strcpy(nombreTemp,carpetaTables);
	strcat(nombreTemp,nombreTabla);
	strcat(nombreTemp,"/");
	strcat(nombreTemp,"1.tmp");//obtenerSiguienteTmp(nombreTemp)); //Hacer esta funcion!

	//printf("%s",nombreTemp);

	FILE * temp =  txt_open_for_append(nombreTemp);
	txt_close_file(temp);


	sprintf(aux_tmp,"%d",tamanioNecesario);

	crearConfig(nombreTemp);
	cambiarValorConfig("SIZE",aux_tmp);
	cambiarValorConfig("BLOCKS",bloques);
	eliminarEstructuraConfig();

	free(aux_tmp);
	free(bloques);

	escribirBloques(tamanioNecesario - (cantBloques-1)*tamanioBloque,cantBloques,listaBloques,castearRegistrosChar(tamanioNecesario,listaRegistros));

	return nombreTemp;

}

char * castearBloquesChar(int lista_bloques[])
{

	if(lista_bloques == NULL)
		return NULL;

	int indice = 0;

	while(lista_bloques[indice] != -1)
		indice++;

	if(indice == 0)
		return NULL;

	char * lista_final = malloc(indice*2 + 10);
	char * aux = malloc(indice + 10);
	//La correcta deberia ser cantidad_elementos + (cantidad_elementos - 1) + 2 + 1 pero para asegurar se da mas

	lista_final[0] = '[';

	sprintf(aux,"%d",lista_bloques[0]);
	strcat(lista_final,aux);

	if(indice == 1)
	{
		strcat(lista_final,"]");
		return lista_final;
	}

	for(int i = 1;lista_bloques[i] != -1; i++)
	{
		strcat(lista_final,",");
		sprintf(aux,"%d",lista_bloques[i]);
		strcat(lista_final,aux);
	}

	free(aux);

	strcat(lista_final,"]");

	return lista_final; // Grabarla en el archivo y liberar la memoria

}

char * obtenerSiguienteTmp(char * nombreTabla)
{
	/*
	 * Tengo que recorrer todos los .tmp de la carpeta, y retornar la cantidad leida + 1 concantenando .tmp
	 *
	*/
	char * rutaFinal = malloc(strlen(nombreTabla) + strlen("9999.tmp") + 2);
	char * archivo_temporal = malloc(strlen(nombreTabla) + strlen("9999.tmp") + 2);
	strcpy(rutaFinal,nombreTabla);
	char * auxiliar = malloc(100);
	int contador = 0;

	do{
		sprintf(auxiliar,"%d",contador);
		strcpy(archivo_temporal,auxiliar);
		strcat(archivo_temporal,".tmp");
		strcat(rutaFinal,archivo_temporal); //rutaCarpeta/nro.tmp
		contador++;
	}while(stat(rutaFinal, &estado) != -1);

	free(auxiliar);
	free(rutaFinal);

	return archivo_temporal;
}


//buscarPrimerBloqueLibre y si no encuentra retorna -1
int  buscarPrimerBloqueLibre()
{
	int aux = 0;


	while(bitarray_test_bit(unBitarray,aux))
	{

		aux++;

	if(aux > cantidadBloques)
		{
		return -1;

		}
	}
	bitarray_set_bit(unBitarray,aux);


	return aux;

}

void levantarMetadata()
{

	crearConfig(metadataBin);

	tamanioBloque = obtenerInt("BLOCK_SIZE");
	cantidadBloques = obtenerInt("BLOCKS");

	eliminarEstructuraConfig();

}
void liberarLogs()
{
	log_destroy(logger);
	//log_destroy(logMemTable);
	//log_destroy(fileSystemLog);
}

void liberarHilos()
{
	list_iterate(hilos,(void*)terminarHilo);
}
void terminarAplicacion(int pid)
{
	liberarHilos();
	liberarNombres();
	liberarLogs();
	exit(1);
}

