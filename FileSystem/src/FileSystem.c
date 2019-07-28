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
	int x = 0; //si ya existia
	if (stat(bitmapBin, &estado) == -1) {
				x=-1;//no existia
		}

	int fd = open(bitmapBin,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); //se abre si existe, sino se crea
	char* bitmap;
	ftruncate(fd,cantidadBloques/8);
	bitmap= mmap(0,cantidadBloques/8,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0); //se baja el archivo a memoria

	unBitarray = bitarray_create_with_mode(bitmap, cantidadBloques/8, LSB_FIRST);

	if(x==-1)
		{
		for(int i = 0; i<cantidadBloques;i++)
			{
			bitarray_clean_bit(unBitarray,i); //seteo todos los bits en 0 (al principio todos los bloques están libres)
			}
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
	char *s = malloc(30);
	char * blocks = malloc(strlen(carpetaBloques)+strlen("99999.bin")+2);

	crearConfig(metadataBin);
	cantidadBloques = obtenerInt("BLOCKS");
	eliminarEstructuraConfig();

	while(i < cantidadBloques){
		sprintf(s,"%d",i);
		strcat(s,".bin");
		strcpy(blocks,carpetaBloques);
		strcat(blocks,s);
		//remove(blocks); //DESPUES SACAR ESTO
		FILE * block =  txt_open_for_append(blocks);
		txt_close_file(block);
		i++;
	}
	free(s);
	free(blocks);
}

void borrarDirectorioVacio(char * directorio)
{
	if(rmdir(directorio)!=0)
	{
	//loggearInfoEnLog(fileSystemLog,"No se pudo borrar el directorio");
	}
}


int borrarArchivosDirectorio(const char *path, const struct stat *datosArchivo,int flags)
{
    if(flags == FTW_F)
    {
        //printf("Se elimina el archivo: %s",path);
        remove(path);
    }
    return 0;
}

void limpiarTabla(char * rutaTabla)
{
	ftw(rutaTabla,borrarArchivosDirectorio,15);

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

	void dumpearTabla(char * tabla, t_list * listaRegistros)
	{
		//Luego crear un hilo detacheable por cada una de estas
		if(listaRegistros)
		{
				if(!list_is_empty(listaRegistros))
			{
			int tamNecesario = obtenerTamanioRegistrosDeUnaLista(listaRegistros);
			char * registros = castearRegistrosChar(tamNecesario,listaRegistros);

			list_clean_and_destroy_elements(listaRegistros,free);

			char * aux = asignarBloques(tamNecesario,tabla,registros); // Libera la memoria
			free(aux);
			free(registros);
			}
		}
	}
	dictionary_iterator(memTable,(void*)dumpearTabla);
	loggearInfo("Dumpeo OK");

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


void crearMetadataTabla(char * directorioTabla, query * queryCreate, int flagConsola)
{
			char * metadataTabla = malloc(strlen(directorioTabla) + strlen("Metadata") + 1);
			strcpy(metadataTabla,directorioTabla);
			strcat(metadataTabla,"Metadata");

			FILE * metadata =  txt_open_for_append(metadataTabla);
			txt_close_file(metadata);

			//Tratar de cambiar todo eso a char *

			char * consistencia;

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

			char * bloque = asignarUnBloqueBin();

			sprintf(aux,"%d",0);

			crearConfig(nroParticion);
			cambiarValorConfig("SIZE",aux);
			cambiarValorConfig("BLOCKS",bloque);
			eliminarEstructuraConfig();

			free(nroParticion);
			free(nroAux);
			free(bloque);
			free(aux);

			i++;
			}
}

int crearCarpetaTabla(query * queryCreate, int flagConsola)
{
	char * directorioTabla = malloc(strlen(carpetaTables) + strlen(queryCreate->tabla) + 2);
	strcpy(directorioTabla,carpetaTables);
	strcat(directorioTabla,queryCreate->tabla);
	strcat(directorioTabla,"/");

	if (stat(directorioTabla, &estado) == -1) {
	mkdir(directorioTabla, 0700);
	}else
	{
		return -1;
	}
		limpiarTabla(directorioTabla);
		crearMetadataTabla(directorioTabla,queryCreate,flagConsola);
		crearParticonesTabla(directorioTabla,queryCreate,flagConsola);


		hiloCompactador = crearHilo(compactar,queryCreate);

		//Finalmente -> Crear un hilo detacheable que se encargue de la compactacion de esta tabla cada x tiempo
		loggearTablaCreadaOK(fileSystemLog,queryCreate,flagConsola,1);
		free(directorioTabla);

	return 0;
}

int rutinaFileSystemCreate(argumentosQuery * args)
{
	int retorno = crearCarpetaTabla(args->unaQuery,args->flag);
	return retorno;
}

char * asignarUnBloqueBin()
{

	int aux = buscarPrimerBloqueLibre();
	char* bloque= malloc(50);
	char * aux2 = malloc(50);
	strcpy(bloque,"[");
	sprintf(aux2,"%d",aux);
	strcat(bloque,aux2);
	strcat(bloque,"]");
	free(aux2);
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
	char * registros = malloc(tamanioNecesario + 100);
	strcpy(registros,"");

	char * aux = malloc(300);

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
	char * subCadena;
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
			subCadena = string_substring(listaRegistros,strlen(listaRegistros) - cantidadFinal,cantidadFinal);
			fwrite(subCadena,cantidadFinal,1,bloque);
			txt_close_file(bloque);
			free(subCadena);
			break;
		}else
		{
		subCadena = string_substring(listaRegistros,tamanioBloque*aux,tamanioBloque);
		fwrite(subCadena,tamanioBloque,1,bloque);
		txt_close_file(bloque);
		free(subCadena);
		}
		aux++;
	}
	free(aux2);
	free(nombreBloque);
}

char * asignarBloques(int tamanioNecesario, char * nombreTabla,char * listaRegistros) //retorna el nombre del .tmp
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

	char * bloques = castearBloquesChar(listaBloques);

	char * nombreTemp = malloc(strlen(carpetaTables) + strlen(nombreTabla) + strlen("9999.tmp") + 2 + 20); // Hasta 9999 posibles .tmp

	strcpy(nombreTemp,carpetaTables);
	strcat(nombreTemp,nombreTabla);
	strcat(nombreTemp,"/");
	char * auxTmp = obtenerSiguienteTmp(nombreTemp);
	strcat(nombreTemp,auxTmp);
	free(auxTmp);

	FILE * temp =  txt_open_for_append(nombreTemp);
	txt_close_file(temp);


	sprintf(aux_tmp,"%d",tamanioNecesario);

	crearConfig(nombreTemp);
	cambiarValorConfig("SIZE",aux_tmp);
	cambiarValorConfig("BLOCKS",bloques);
	eliminarEstructuraConfig();

	free(aux_tmp);
	free(bloques);

	escribirBloques(tamanioNecesario - (cantBloques-1)*tamanioBloque,cantBloques,listaBloques,listaRegistros);

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
	char * aux = malloc(indice + 10); //Ver de eliminar y probar
	//La correcta deberia ser cantidad_elementos + (cantidad_elementos - 1) + 2 + 1 pero para asegurar se da mas

	strcpy(lista_final,"[");
	sprintf(aux,"%d",lista_bloques[0]);
	strcat(lista_final,aux);

	if(indice == 1)
	{
		strcat(lista_final,"]");
		free(aux);
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
	struct stat estado = {0};
	char * archivo_temporal = malloc(strlen(nombreTabla) + strlen("9999.tmp") + 2);
	//
	char * auxiliar = malloc(100);
	int contador = 0;

	do{
		strcpy(rutaFinal,nombreTabla);
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

void levantarMemTable()
{
	//ftw(carpetaTables,recorrer_directorio,15);
	recorrerDirectorio(carpetaTables);
}

int recorrer_directorio(const char *path, const struct stat *datosArchivo,int flags)
	{
		char * aux = malloc(strlen(path) + 3);
		strcpy(aux,path);
		strcat(aux,"/");
		char * nombreCarpeta = strdup(string_substring_from(aux,strlen(carpetaTables) - 1));

	    if(flags & FTW_D)
	    	printf("%s",nombreCarpeta);//dictionary_put(memTable,nombreCarpeta,NULL);

	    free(aux);
	    free(nombreCarpeta);
	    return 0;
	}



void recorrerDirectorio(char * directorio)
{
        DIR * dir = opendir(directorio);
        struct dirent *estructuraDir; //Para recorrer y obtener info del directorio
        query * queryCreate;
        char ** metadata;

        if(dir != NULL)
        {
            while ((estructuraDir=readdir(dir)))
            {

                if(estructuraDir->d_type == DT_DIR && strncmp(estructuraDir->d_name,".",1))
                {
                	dictionary_put(memTable,estructuraDir->d_name,NULL);

                	char * rutaAbs = malloc(strlen(directorio) + strlen(estructuraDir->d_name) + strlen("/Metadata") + 1);
                	strcpy(rutaAbs,directorio);
                	strcat(rutaAbs,estructuraDir->d_name);

                	char * describe = obtenerMetadaTabla(rutaAbs);
                	metadata = string_split(describe,";");
                	char ** cantidadParticiones = string_split(metadata[2],"=");
                	char ** tiempoCompactacion = string_split(metadata[3],"=");
                	char * auxTiempoCompactacion = string_substring_until(tiempoCompactacion[1],strlen(tiempoCompactacion[1])-1);
                	queryCreate = malloc(sizeof(query));

                	queryCreate->compactationTime = atoi(auxTiempoCompactacion);
                	queryCreate->cantParticiones = atoi(cantidadParticiones[1]);
                	queryCreate->tabla = strdup(estructuraDir->d_name);

                	hiloCompactador = crearHilo(compactar,queryCreate);

                	free(rutaAbs);

                	liberarCadenaSplit(metadata);
                	liberarCadenaSplit(cantidadParticiones);
                	liberarCadenaSplit(tiempoCompactacion);

                	free(auxTiempoCompactacion);
                	free(describe);

                }
            }
            closedir(dir);
        }

}

char * leerBloque(char * numeroBloque,int bytesALeer) // "5"
{ //Bytes a leer es siempre tamaño del bloque, excepto en el ultimo bloque
	char * auxiliar = malloc(strlen("99999.bin")+strlen(numeroBloque)+1);
	strcpy(auxiliar,numeroBloque);
	strcat(auxiliar,".bin");
	char * bloqueALeer = malloc(strlen(carpetaBloques)+strlen(auxiliar)+100);
	strcpy(bloqueALeer,carpetaBloques);
	strcat(bloqueALeer,auxiliar);
	char * contenido = malloc(bytesALeer+1);
	strcpy(contenido,"");
	FILE * archivo= fopen(bloqueALeer,"r");
//	fseek(archivo, 0, SEEK_SET);
	fflush(archivo);
	rewind(archivo);
	fread(contenido,1,bytesALeer,archivo);
	contenido[bytesALeer] = 0;
	fclose(archivo);
	free(bloqueALeer);
	free(auxiliar);
	return contenido;

}

char * obtenerRegistrosArchivo(char * ruta)
{
	crearConfig(ruta);
	char ** bloques = obtenerArray("BLOCKS"); // devuelve ["5","64","17",NULL]
	int tamanioArchivo = obtenerInt("SIZE");
	eliminarEstructuraConfig();

	char* aux = malloc(tamanioArchivo+10);
	strcpy(aux,"");
	int i = 0;
	char * bloqueLeido;
	while(bloques[i]!=NULL && tamanioArchivo > 0)
	{
		bloqueLeido=NULL;
		if(bloques[i+1]!=NULL)
		{
		bloqueLeido =leerBloque(bloques[i],tamanioBloque);
		strcat(aux,bloqueLeido);
		}
		else
		{
		bloqueLeido =leerBloque(bloques[i],tamanioArchivo-tamanioBloque*i);
		strcat(aux,bloqueLeido);
		}
	free(bloqueLeido);
	i++;
	}

	free(bloques);
	return aux;
}

registro * rutinaFileSystemSelect(char * tabla, int32_t key)
	{
	t_list* registros;
	char * rutaAbs = malloc(strlen(carpetaTables)+strlen(tabla)+strlen("99999.bin")+1);
	strcpy(rutaAbs,carpetaTables);
	strcat(rutaAbs,tabla);
	strcat(rutaAbs,"/");

	registros=leerTabla(rutaAbs,key);
	free(rutaAbs);
	registro * registroMax;
	if(registros!=NULL)
	{
		if(!list_is_empty(registros))
		{
			if(list_size(registros) == 1)
			{
				registroMax = (registro *)list_remove(registros,0);
				list_destroy(registros);
				return registroMax;
			}

	registroMax = (registro *)list_remove(registros,0);

	void mayorTimestamp(registro * unRegistro)
			{
				if(unRegistro->timestamp > registroMax->timestamp)
				{
					free(registroMax->value);
					free(registroMax);
					registroMax = unRegistro;
				}else
				{
					free(unRegistro->value);
					free(unRegistro);
				}
			}

			list_iterate(registros,(void*)mayorTimestamp);
			list_destroy(registros);
			return registroMax;
		}
	}
	list_destroy(registros); ///VALIDAR POR SI ROMPE
	return NULL;

	}


t_list * leerTabla(char * tabla, int32_t key)
{
        DIR * dir = opendir(tabla);
        struct dirent *estructuraDir; //Para recorrer y obtener info del directorio
        t_list* listaRegistros = list_create();

        if(dir != NULL)
        {
            while ((estructuraDir=readdir(dir)))
            {
            	char * rutaAbs = malloc(strlen(carpetaTables)+strlen(tabla)+strlen(estructuraDir->d_name)+1);
            	strcpy(rutaAbs,tabla);
            	strcat(rutaAbs,estructuraDir->d_name);
            	if(!strncmp(estructuraDir->d_name,"Metadata",8) || !strncmp(estructuraDir->d_name,".",1))
            	{

            	}
            	else
            	{

            	char * registros = obtenerRegistrosArchivo(rutaAbs);
            	t_list * registrosAux = obtenerRegistros(registros, key);
            	list_add_all(listaRegistros,registrosAux);
            	list_destroy(registrosAux);
            	free(registros);
            	}
            	free(rutaAbs);

            }
            closedir(dir);
        }
        return listaRegistros;
}

t_list* obtenerRegistros(char * registros, int32_t key){
	t_list * listaRegistros = list_create();
	char ** aux = string_split(registros,"\n");
	int i = 0;
	registro* registroAux;


		while(aux[i]!=NULL)
		{
			if(strcmp(aux[i],"")){


		char ** registro =string_split(aux[i],";");
		registroAux=malloc(sizeof(int32_t)+sizeof(int64_t)+1+strlen(registro[1]));
		registroAux->key=atoi(registro[0]);

			if(registroAux->key == key)
			{
				registroAux->value=strdup(registro[1]);
				registroAux->timestamp = (int64_t) atoll(registro[2]);
				list_add(listaRegistros,registroAux);
			} else{
				free(registroAux);
			}

			liberarCadenaSplit(registro);
			}
			free(aux[i]);
			i++;
		}

	free(aux);
	return listaRegistros;
}


int rutinaFileSystemDrop(char * tabla)
{

	char * rutaTabla = malloc(strlen(carpetaTables)+strlen(tabla) + 5);
	strcpy(rutaTabla,carpetaTables);
	strcat(rutaTabla,tabla);
	strcat(rutaTabla,"/");
	int retorno = eliminarDirectorio(rutaTabla);
	remove(rutaTabla);
	free(rutaTabla);
	return retorno;
}

int eliminarDirectorio(char * directorio)
{
        DIR * dir = opendir(directorio);
        struct dirent *estructuraDir; //Para recorrer y obtener info del directorio

        if(dir != NULL)
                {
                    while ((estructuraDir=readdir(dir)))
                    {
                    	char * rutaAbs = malloc(strlen(directorio)+strlen(estructuraDir->d_name)+6);
                    	strcpy(rutaAbs,directorio);
                    	strcat(rutaAbs,estructuraDir->d_name);
                    	if(!strncmp(estructuraDir->d_name,"Metadata",8) || !strncmp(estructuraDir->d_name,".",1))
                    	{
                    		if(!strncmp(estructuraDir->d_name,"Metadata",8))
                    		remove(rutaAbs);
                    	}
                    	else
                    	{
                    		borrarBloques(rutaAbs);
                    	}
                    	free(rutaAbs);

                    }
                    closedir(dir);
                    return 1;
                }
        return 0;

}

void borrarBloques(char * rutaArchivo)
{
FILE * bloque;
crearConfig(rutaArchivo);
char ** bloques = obtenerArray("BLOCKS"); // devuelve ["5","64","17",NULL]
eliminarEstructuraConfig();

char * rutaBloque = malloc(strlen(carpetaBloques) + CANTIDAD_MAXIMA_BLOQUES + strlen(".bin") + 1);

int i = 0;
char * bloqueLeido = malloc(CANTIDAD_MAXIMA_BLOQUES + strlen(".bin") + 1);

while(bloques[i]!=NULL)
	{
		strcpy(bloqueLeido,bloques[i]);
		strcat(bloqueLeido,".bin");
		strcpy(rutaBloque,carpetaBloques);
		bitarray_clean_bit(unBitarray,atoi(bloques[i]));
		strcat(rutaBloque,bloqueLeido);
		remove(rutaBloque);
		bloque =  txt_open_for_append(rutaBloque);
		txt_close_file(bloque);
		free(bloques[i]);
		i++;
	}
free(rutaBloque);
free(bloqueLeido);
free(bloques);
remove(rutaArchivo);

}


char * rutinaFileSystemDescribe(char * tabla)
{
	char * metadata;
	if(tabla)
	{
	char * rutaTabla = malloc(strlen(carpetaTables)+strlen(tabla)+strlen("999999.bin")+1);
	strcpy(rutaTabla,carpetaTables);
	strcat(rutaTabla,tabla);
	metadata = obtenerMetadaTabla(rutaTabla);
	free(tabla);
	free(rutaTabla);
	}
	else
	{
	metadata = obtenerMetadataTablas();
	}

	return metadata;

}
char * obtenerMetadataTablas()
{
	int cantidadTablasSistema = cantidadTablas();
	if(cantidadTablasSistema>0)
	{
		DIR * dir = opendir(carpetaTables);
		char * tablaActual;
		struct dirent * estructuraDir; //Para recorrer y obtener info del directorio
		char * cadenaRetorno = malloc(strlen("\nTABLE=TABLA_REQUETE_LARGA_LOL;CONSISTENCY=SHC;PARTITIONS=9999999;COMPACTION_TIME=100000000000000;\n")*cantidadTablasSistema + 50);
		strcpy(cadenaRetorno,"");
		if(dir != NULL)
		{
		while ((estructuraDir=readdir(dir)))
			   {
					if(strncmp(estructuraDir->d_name,".",1))
					{
						tablaActual = malloc(strlen(carpetaTables) + strlen(estructuraDir->d_name) + 1);
						strcpy(tablaActual,carpetaTables);
						strcat(tablaActual,estructuraDir->d_name);
						char * metadataTabla = obtenerMetadaTabla(tablaActual);
						strcat(cadenaRetorno,metadataTabla);
						free(metadataTabla);
						free(tablaActual);
					}
			   }
		}
		closedir(dir);
		return cadenaRetorno;
	}
		return NULL;
}
char * obtenerMetadaTabla(char * rutaTabla)
{
	 DIR * dir = opendir(rutaTabla);
	 struct dirent *estructuraDir; //Para recorrer y obtener info del directorio
	 char * cadenaRetorno = NULL;
	 char * nombreTabla;
	 if(dir != NULL)
	     {
	        while ((estructuraDir=readdir(dir)))
	        {
	        	if(!strncmp(estructuraDir->d_name,"Metadata",8))
	        	{
				char * rutaMetadata = malloc(strlen(carpetaTables)+strlen(rutaTabla)+strlen(estructuraDir->d_name)+1);
				strcpy(rutaMetadata,rutaTabla);
				strcat(rutaMetadata,"/");
				strcat(rutaMetadata,"Metadata");

				crearConfig(rutaMetadata);

				nombreTabla = obtenerNombre(rutaTabla);

				cadenaRetorno = malloc(strlen(estructuraDir->d_name) +
				strlen("TABLE=;CONSISTENCY=;PARTITIONS=;COMPACTION_TIME=;COMPACTION_TIME=\n")
				+ strlen(obtenerString("CONSISTENCY")) + strlen(obtenerString("PARTITIONS"))
				+ strlen(obtenerString("COMPACTION_TIME")) + 10);

				strcpy(cadenaRetorno,"TABLE=");
				strcat(cadenaRetorno,nombreTabla);
				strcat(cadenaRetorno,";CONSISTENCY=");
				strcat(cadenaRetorno,obtenerString("CONSISTENCY"));
				strcat(cadenaRetorno,";PARTITIONS=");
				strcat(cadenaRetorno,obtenerString("PARTITIONS"));
				strcat(cadenaRetorno,";COMPACTION_TIME=");
				strcat(cadenaRetorno,obtenerString("COMPACTION_TIME"));
				strcat(cadenaRetorno,"\n");
				eliminarEstructuraConfig();

				free(rutaMetadata);
				free(nombreTabla);
				}
	        }
	        closedir(dir);
	      }

	 return cadenaRetorno;
}

int cantidadTablas()
{
		DIR * dir = opendir(carpetaTables);
		struct dirent *estructuraDir; //Para recorrer y obtener info del directorio
		int cantidad = 0;
		if(dir != NULL)
		     {
		        while ((estructuraDir=readdir(dir)))
		        {
		        	if(strncmp(estructuraDir->d_name,".",1))
		        	{
		        		cantidad++;
					}
		        }
		        closedir(dir);
		      }

		 return cantidad;
}

char * obtenerNombre(char * ruta)
{
	int i = 0;
	int cantidadBarras = 0;
	char * cadenaRetorno = malloc(strlen(ruta)+1);
	while(ruta[i] !=  0)
	{
		if(ruta[i] == '/')
		{
			cantidadBarras++;
		}
		i++;
	}
	int offSet = 0;
	i=0;
	int cant_anterior = 0;
	while(ruta[i] != 0)
	{
		if(ruta[i] == '/')
		{
			if((cantidadBarras - cant_anterior) == 1)
			{
				offSet = i;
				break;
			}
			else
			{
				cant_anterior++;
			}
		}
		i++;
	}
	i=0;
	if(offSet != 0)
	{
		offSet++;
		while(ruta[offSet] != 0)
		{
			cadenaRetorno[i] = ruta[offSet];
			offSet++;
			i++;
		}
		cadenaRetorno[i] = 0;
		return cadenaRetorno;
	}
	return NULL;
}



