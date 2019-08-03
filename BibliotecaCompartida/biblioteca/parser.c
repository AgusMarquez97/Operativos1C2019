#include "parser.h"

int parsear(char * string_query,query **struct_query)
{
	int ERROR = -1, cantidadStrings=0;
	//char * MENSAJE_ERROR="Sintaxis incorrecta o comando desconocido.";

	//string_append_with_format(&MENSAJE_ERROR,": %s",string_query);
	char ** aux_split, ** auxEspacios;
	char ** query_split = string_split(string_query, " ");

	*struct_query = malloc(sizeof(query));
	int query_cant_palabras = string_size(query_split);
//	printf("Llegaron al parser %d palabras\n",query_cant_palabras);
//	printf("Request a parsear: %s\n",string_query);

	if ( !query_cant_palabras)
	{
		liberarCadenaSplit(query_split);
		return ERROR;
	}

	if (!strcasecmp(query_split[0],"select")) {

	  if ( (query_cant_palabras != 3) )
	  {
		  liberarCadenaSplit(query_split);
		  return ERROR;
	  }

	  if ( (nombre_tabla_valido(query_split[1])) && (valor_solo_numerico(query_split[2])) )
	  {
		string_to_upper(query_split[1]);
		((*struct_query))->requestType = SELECT;
		((*struct_query))->tabla = strdup(query_split[1]);
		((*struct_query))->key = atoi(query_split[2]);
		((*struct_query))->value = NULL;
		((*struct_query))->timestamp = -1;

		liberarCadenaSplit(query_split);
		return SELECT;

	  } else { 
		   //printf("%s\n\n",MENSAJE_ERROR);
		   //printf("El select es INcorrecto\n");
		   liberarCadenaSplit(query_split);
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"insert"))
	{

			  aux_split = string_split(string_query,"\"");
			  cantidadStrings = cantidadSubStrings(aux_split);

			  switch (cantidadStrings) {
						case (2): ((*struct_query))->timestamp = (int64_t) (time(NULL))*1000;
							  break;

						case (3):
								string_trim(&aux_split[2]);
								if (!valor_solo_numerico(aux_split[2])) {
								//printf("%s\n\n",MENSAJE_ERROR);
								//printf("El insert es INcorrecto\n");
								liberarCadenaSplit(aux_split);
								liberarCadenaSplit(query_split);
								return ERROR;
								break;}
							  else {
								((*struct_query))->timestamp = (int64_t) atoll(aux_split[2]);
								break;
							  }
				default:
					liberarCadenaSplit(aux_split);
					liberarCadenaSplit(query_split);
					 return ERROR;
					 break;
			}

	    auxEspacios = string_split(aux_split[0]," ");
	    if (nombre_tabla_valido(auxEspacios[1]) && valor_solo_numerico(auxEspacios[2]) && cantidadSubStrings(auxEspacios)==3)
	    {

		//printf("El insert es correcto\n");
		string_to_upper(auxEspacios[1]);
		((*struct_query))->requestType = INSERT;
		((*struct_query))->tabla = strdup(auxEspacios[1]);
		((*struct_query))->key = atoi(auxEspacios[2]);
		((*struct_query))->value = strdup(aux_split[1]);

		liberarCadenaSplit(auxEspacios);
		liberarCadenaSplit(aux_split);
		liberarCadenaSplit(query_split);
		return INSERT;
	    } else {
	       liberarCadenaSplit(auxEspacios);
	       liberarCadenaSplit(aux_split);
	       liberarCadenaSplit(query_split);
		   return ERROR;
	  	}
		

	}


	if (!strcasecmp(query_split[0],"describe")) {

	  
      ((*struct_query))->requestType = DESCRIBE;
	  ((*struct_query))->key = -1;
	  ((*struct_query))->value = NULL;
	  ((*struct_query))->timestamp = -1;

	  if ( (query_cant_palabras == 1) )
	  {
		//printf("El describe es correcto\n");
		((*struct_query))->tabla = NULL;
		liberarCadenaSplit(query_split);
		return DESCRIBE;
	  }

	  if (	(query_cant_palabras == 2) && (nombre_tabla_valido(query_split[1]))
	     )
	  {
		//printf("El describe es correcto\n");
		string_to_upper(query_split[1]);
		((*struct_query))->tabla = strdup(query_split[1]);
		liberarCadenaSplit(query_split);
		return DESCRIBE;

	  } else { 
		  liberarCadenaSplit(query_split);
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"create")) {

	  if ( (query_cant_palabras != 5) )
	  {
		liberarCadenaSplit(query_split);
		return ERROR;
	  }

	  if (	(nombre_tabla_valido(query_split[1])) 	   &&
		(tipo_consistencia_valido(query_split[2])) &&
		(valor_solo_numerico(query_split[3]))	   &&
		(valor_solo_numerico(query_split[4]))
	     )
	  {
		  if(atoi(query_split[3]) > 0)
		  {
		//printf("El create es correcto\n");
		string_to_upper(query_split[1]);
		((*struct_query))->requestType = CREATE;
		((*struct_query))->tabla = strdup(query_split[1]);
		((*struct_query))->consistencyType = string_a_consistencia(query_split[2]);
		((*struct_query))->cantParticiones = atoi(query_split[3]);
		((*struct_query))->compactationTime = atoi(query_split[4]); //Luego sacar esto!
		((*struct_query))->timestamp = -1;

		liberarCadenaSplit(query_split);
		return CREATE;
		  }
		  else
		  {
			  liberarCadenaSplit(query_split);
			  return ERROR;
		  }

	  } else {
		   liberarCadenaSplit(query_split);
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"drop")) {

	  if ( (query_cant_palabras != 2) )
	  {
	    liberarCadenaSplit(query_split);
		return ERROR;
	  }

	  if (	(nombre_tabla_valido(query_split[1]))
	     )
	  {
		string_to_upper(query_split[1]);
		((*struct_query))->requestType = DROP;
		((*struct_query))->tabla = strdup(query_split[1]);
		((*struct_query))->key = -1;
		((*struct_query))->value = NULL;
		((*struct_query))->timestamp = -1;
		liberarCadenaSplit(query_split);
		return DROP;

	  } else {
		  liberarCadenaSplit(query_split);
		   return ERROR;
		 }
		

	}



	if (!strcasecmp(query_split[0],"journal")) {

	  if ( (query_cant_palabras != 1) )
	  {
		liberarCadenaSplit(query_split);
		return ERROR;
	  } else {
			((*struct_query))->requestType = JOURNAL;
			((*struct_query))->tabla = NULL;
			((*struct_query))->key = -1;
			((*struct_query))->value = NULL;
			((*struct_query))->timestamp = -1;
			liberarCadenaSplit(query_split);
			return JOURNAL;
		}
		
	}


	if (!strcasecmp(query_split[0],"metrics")) {

	  if ( (query_cant_palabras != 1) )
	  {
		liberarCadenaSplit(query_split);
		return ERROR;
	  } else {
			((*struct_query))->requestType = METRICS;
			((*struct_query))->tabla = NULL;
			((*struct_query))->key = -1;
			((*struct_query))->value = NULL;
			((*struct_query))->timestamp = -1;
			liberarCadenaSplit(query_split);
			return METRICS;
		}
		

	}



	if (!strcasecmp(query_split[0],"run")) {

	  if ( (query_cant_palabras != 2) )
	  {
		  liberarCadenaSplit(query_split);
		return ERROR;
	  } else {
			((*struct_query))->requestType = RUN;
			((*struct_query))->script = strdup(query_split[1]);
			liberarCadenaSplit(query_split);
			return RUN;
		 }
	}
	liberarCadenaSplit(query_split);
	return ERROR;

}



int string_size(char ** text)
{
	int i= 0;

	while ((text[i]) != '\0')
	{
	  i = i+1;
	}

	return i;
}


int nombre_tabla_valido(char * text)
{

	string_to_upper(text);

	regex_t regex;
	int reti;

	reti = regcomp(&regex,"^[A-Z0-9]*$",0);

	if (reti) {
		return 0;
	}

	reti = regexec(&regex,text,0,NULL,0);

	if (!reti) {
	   regfree(&regex);
	   return 1;
	}
	else if (reti == REG_NOMATCH) {
		regfree(&regex);
		return 0;
	}
	else {
		regfree(&regex);
		return 0;
	}
	
}



int valor_solo_numerico(char * text)
{

	regex_t regex;
	int reti;
//	printf("%s\n",text);

	reti = regcomp(&regex,"^[0-9]*$",0);

	if (reti) {
	  //fprintf(stderr,"Error al armar regex\n");
	  //exit (1);
		return 0;
	}

	reti = regexec(&regex,text,0,NULL,0);

	if (!reti) {
	   regfree(&regex);
	   return 1;
	}
	else if (reti == REG_NOMATCH) {
		regfree(&regex);
		return 0;
	}
	else {
		regfree(&regex);
		return 0;
	}
	
}


int tipo_consistencia_valido(char * texto)
{

	if ( (!strcasecmp(texto,"sc")) || (!strcasecmp(texto,"ec")) || (!strcasecmp(texto,"shc")) )
	{
		return 1;
	} else
	{
		return 0;
	}
}


char * consistencia_a_string(int32_t consistencia)
{
	switch(consistencia)
	{
	case SC:
	return "SC";
	break;
	case SHC:
	return "SHC";
	break;
	case EC:
	return "EC";
	break;
	default:
	return "";
	}
}

int string_a_consistencia(char * consistencia)
{
	if(!strncasecmp(consistencia,"SC",2))
	return SC;
	if(!strncasecmp(consistencia,"SHC",3))
	return SHC;
	if(!strncasecmp(consistencia,"EC",2))
	return EC;
	return -1;
}

int cantidadSubStrings(char ** string)
{
	int i = 0, j= 0;
	if(string)
	{
		while(string[i]!=NULL)
		{
			if(strlen(string[i]) > 0)
				j++;
			i++;
		}
	}
	return j;
}

void liberarCadenaSplit(char ** cadena)
{
	if(cadena)
	{
		int i = 0;
		while(cadena[i] != NULL)
		{
			free(cadena[i]);
			i++;
		}
		free(cadena);
	}
}


