#include "Compactador.h"


void compactar(query * queryCreate){

	int tiempoCompactacion = queryCreate->compactationTime;
	int cantidadParticiones = queryCreate->cantParticiones;
	char * nombreTabla = strdup(queryCreate->tabla);

	//liberarQueryCreateLevantada(queryCreate);

	char * directorioTabla = malloc(strlen(carpetaTables) + strlen(queryCreate->tabla) + 2);
	strcpy(directorioTabla,carpetaTables);
	strcat(directorioTabla,nombreTabla);
	strcat(directorioTabla,"/");

	while(1)
		{
		t_list * registrosBinTmpc;

		usleep(tiempoCompactacion*1000);

		renombrarArchivosTemporales(directorioTabla);

		registrosBinTmpc = obtenerRegistrosBinTmpc(directorioTabla);

		t_list * registrosCompactados = obtenerListaCompactada(registrosBinTmpc);

		compactarBinarios(directorioTabla,registrosCompactados,cantidadParticiones); // Vuelve a crear los bin



		/*
		 queda para hacer:

		obtenerRegistrosActualizados() que compara por los tmpc contra las particiones
		generarNuevasParticiones() hay que liberar los bloques de los tmpc y los bin viejos, actualizar bitmap,
									borrar esos archivos y escribir los registrosActualizados en los bloques, considerando
									segun la key a que particion se escribe cada registro(usar el resto de dividir por cantParticiones)

		*/

		list_destroy(registrosBinTmpc);
		list_destroy(registrosCompactados);
		}
}

void renombrarArchivosTemporales(char* rutaDirectorioTabla){

		int i = 0;
		int j = 0;
		char * old= malloc(strlen(rutaDirectorioTabla)+strlen("999999.tmp")+1);
		char * new= malloc(strlen(rutaDirectorioTabla)+strlen("999999.tmpc")+1);


		while(j!=-1)
		{
			strcpy(old,rutaDirectorioTabla);
			strcat(old,string_itoa(i));
			strcat(old,".tmp");
			strcpy(new,rutaDirectorioTabla);
			strcat(new,string_itoa(i));
			strcat(new,".tmpc");
			j=rename(old,new);
			i++;
		}
		free(old);
		free(new);

}

t_list * obtenerRegistrosBinTmpc(char * tabla) //Patea .tmpc & .bin
{
			DIR * dir = opendir(tabla);
	        struct dirent *estructuraDir; //Para recorrer y obtener info del directorio
	        t_list* listaRegistros = list_create();
	        char * registros;

	        if(dir != NULL)
	        {
	            while ((estructuraDir=readdir(dir)))
	            {
	            	char * rutaAbs = malloc(strlen(carpetaTables)+strlen(tabla)+strlen(estructuraDir->d_name)+1);
	            	strcpy(rutaAbs,tabla);
	            	strcat(rutaAbs,estructuraDir->d_name);
	            	char ** auxTmp = string_split(estructuraDir->d_name,".");
	            	if(!strncmp(estructuraDir->d_name,"Metadata",8) || !strncmp(estructuraDir->d_name,".",1) || !strcmp(auxTmp[1],"tmp"))
	            	{

	            	}
	            	else
	            	{
	            	registros = (char *)obtenerRegistrosArchivo((char *)rutaAbs);
	            	t_list * registrosAux = obtenerRegistrosCompactacion(registros);
	            	list_add_all(listaRegistros,registrosAux);
	            	list_destroy(registrosAux); //Validar!
	            	free(registros);
	            	}
	            	liberarCadenaSplit(auxTmp);
	            	free(rutaAbs);

	            }
	            closedir(dir);
	        }
	        return listaRegistros;
}


t_list* obtenerRegistrosCompactacion(char * registros)
{
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
		registroAux->value=strdup(registro[1]);
		registroAux->timestamp = (int64_t) atoll(registro[2]);

		list_add(listaRegistros,registroAux);

		liberarCadenaSplit(registro);
			}
			free(aux[i]);
			i++;
		}

	free(aux);
	return listaRegistros;
}

t_list * obtenerListaCompactada(t_list * listaInicial)
{
			t_list * registrosCompactados = list_create();
			t_list * listaFiltrada;
			registro * registroFinal,* registroDummy;

			void compactarRegistros(registro * unRegistro)
			{
				bool keyYaExistente(registro * registroCompactado)
				{
					return registroCompactado->key == unRegistro->key;
				}

				if(!list_any_satisfy(registrosCompactados,(void*)keyYaExistente))
				{
					bool condicionKeyDuplicada(registro * registroComparar)
							{
								return unRegistro->key == registroComparar->key;
							}
				listaFiltrada = list_filter(listaInicial,(void*)condicionKeyDuplicada); //Devuelve en el peor caso 1 lista de 1 elemento

					if(list_size(listaFiltrada)>1)
					{

						registroDummy = unRegistro;
						void mayorTimestamp(registro * otroRegistro)
								{
									if(otroRegistro->timestamp > registroDummy->timestamp)
									{
										registroDummy = otroRegistro;
									}
								}

								list_iterate(listaFiltrada,(void*)mayorTimestamp);
								registroFinal = malloc(sizeof(registro) + strlen(registroDummy->value));
								registroFinal->key = registroDummy->key;
								registroFinal->value = strdup(registroDummy->value);
								registroFinal->timestamp = registroDummy->timestamp;
								list_add(registrosCompactados,registroFinal);
						}

					else
					{
						list_add(registrosCompactados,unRegistro);
					}
				list_destroy(listaFiltrada); //Validar!
				}
			}

			list_iterate(listaInicial,(void*)compactarRegistros);
			return registrosCompactados;
}

void limpiarBloquesBitmap(char * directorio)
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
	                    	char ** auxTmp = string_split(estructuraDir->d_name,".");
	                    	if(!strncmp(estructuraDir->d_name,"Metadata",8) || !strncmp(estructuraDir->d_name,".",1) || !strcmp(auxTmp[1],"tmp"))
	                    	{

	                    	}
	                    	else
	                    	{
	                    		borrarBloques(rutaAbs);
	                    	}
	                    	liberarCadenaSplit(auxTmp);
	                    	free(rutaAbs);

	                    }
	                    closedir(dir);
	                }
}

// (directorioTabla,registrosCompactados,registros,cantidadParticiones)
void compactarBinarios(char * tabla, t_list * listaRegistros, int cantidadParticiones)
{
	int tamNecesario;
	char * registros;
	char * rutaBinario;
	t_list * listaBinario;
	FILE * temp;

	limpiarBloquesBitmap(tabla); // Limpia los archivos .bin y .tmpc => Libera sus bloques y borra los archivos

	for(int i = 0;i < cantidadParticiones;i++)
	{
		rutaBinario = malloc(strlen(tabla) + strlen("999999999.bin") + 1);
		strcpy(rutaBinario,tabla);
		strcat(rutaBinario,string_itoa(i));
		strcat(rutaBinario,".bin");
		temp =  txt_open_for_append(rutaBinario);
		txt_close_file(temp);
	}

	for(int i = 0;i < cantidadParticiones;i++)
	{
		rutaBinario = malloc(strlen(tabla) + strlen("999999999.bin") + 1);
		strcpy(rutaBinario,tabla);
		strcat(rutaBinario,string_itoa(i));
		strcat(rutaBinario,".bin");
		bool condicionBinario(registro * unRegistro)
		{
			if(unRegistro->key % cantidadParticiones == i)
				return true;
			return false;
		}
		listaBinario = list_filter(listaRegistros,(void*)condicionBinario);
		tamNecesario = obtenerTamanioRegistrosDeUnaLista(listaBinario);
		if(tamNecesario!=0)
		{
		registros = castearRegistrosChar(tamNecesario,listaBinario);
		asignarBloquesABinarios(tamNecesario,rutaBinario,registros);
		list_destroy(listaBinario);
		free(registros);
		}else {
			char * aux = malloc(sizeof(int)*2);
			char * bloque = asignarUnBloqueBin();
			sprintf(aux,"%d",0);
			crearConfig(rutaBinario);
			cambiarValorConfig("SIZE",aux);
			cambiarValorConfig("BLOCKS",bloque);
			eliminarEstructuraConfig();
			free(aux);
			free(bloque);
		}

	}
}


void asignarBloquesABinarios(int tamanioNecesario, char * binario,char * listaRegistros) //retorna el nombre del .tmp
{

	int cantBloques = obtenerCantidadBloques(tamanioNecesario);

	int listaBloques[cantBloques];

	for(int i = 0; i < cantBloques; i++)
	{
		listaBloques[i] = buscarPrimerBloqueLibre();
	}

	char * aux_tmp = malloc(1000);
	listaBloques[cantBloques] = -1; //Valor centinela de corte

	char * bloques = castearBloquesChar(listaBloques);

	sprintf(aux_tmp,"%d",tamanioNecesario);

	crearConfig(binario);
	cambiarValorConfig("SIZE",aux_tmp);
	cambiarValorConfig("BLOCKS",bloques);
	eliminarEstructuraConfig();

	free(aux_tmp);
	free(bloques);

	escribirBloques(tamanioNecesario - (cantBloques-1)*tamanioBloque,cantBloques,listaBloques,listaRegistros);
}

void liberarQueryCreateLevantada(query * queryCreate)
{
	if(queryCreate)
	{
		if(queryCreate->tabla)
			free(queryCreate->tabla);
		free(queryCreate);
	}
}
