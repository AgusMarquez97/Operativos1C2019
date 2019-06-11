#include "parser.h"


int parsear(char * string_query,query *struct_query)
{

	char ** query_split = string_split(string_query, " ");
/*	printf("La primera palabra es: %s\n",query_split[0]);
	printf("La segunda palabra es: %s\n",query_split[1]);
	printf("La tercera palabra es: %s\n",query_split[2]);
	printf("La cantidad de palabras es: %d\n",string_size(query_split));
*/
	//struct_query = malloc(sizeof(query));
	int query_cant_palabras = string_size(query_split);
//	printf("Llegaron al parser %d palabras\n",query_cant_palabras);
//	printf("Request a parsear: %s\n",string_query);

	if ( !query_cant_palabras)
	{
		return ERROR;
	}

	if (!strcasecmp(query_split[0],"select")) {

	  if ( (query_cant_palabras != 3) )
	  {
		printf("%s\n\n",MENSAJE_ERROR);
		//printf("El select es INcorrecto\n");
		return ERROR;
	  }

	  if ( (nombre_tabla_valido(query_split[1])) && (valor_solo_numerico(query_split[2])) )
	  {
		printf("El select es correcto\n");
		string_to_upper(query_split[1]);
		struct_query->requestType = SELECT;
		struct_query->tabla = query_split[1];
		struct_query->key = atoi(query_split[2]);
		struct_query->value = NULL;
		struct_query->timestamp = NULL;

		return SELECT;

	  } else { 
		   printf("%s\n\n",MENSAJE_ERROR);
		   //printf("El select es INcorrecto\n");
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"insert")) {

	  if (	(nombre_tabla_valido(query_split[1])) &&
		(valor_solo_numerico(query_split[2]))
	     )
	  {

		switch (query_cant_palabras) {
			case (4): struct_query->timestamp = NULL;
				  break;

			case (5): if (!valor_solo_numerico(query_split[4])) {
					printf("%s\n\n",MENSAJE_ERROR);
					//printf("El insert es INcorrecto\n");
					return ERROR;
					break;}
				  else {
					struct_query->timestamp = query_split[4];
					break;
				  }
				  
			default: printf("%s\n\n",MENSAJE_ERROR);
				 //printf("El insert es INcorrectoAAA\n");
				 return ERROR;
				 break;
		}

		printf("El insert es correcto\n");
		string_to_upper(query_split[1]);
		struct_query->requestType = INSERT;
		struct_query->tabla = query_split[1];
		struct_query->key = atoi(query_split[2]);
		struct_query->value = query_split[3];

		return INSERT;

	  } else {
		   printf("%s\n\n",MENSAJE_ERROR);
		   //printf("El insert es INcorrecto\n");
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"describe")) {

	  
          struct_query->requestType = DESCRIBE;
	  struct_query->key = NULL;
	  struct_query->value = NULL;
	  struct_query->timestamp = NULL;

	  if ( (query_cant_palabras == 1) )
	  {
		printf("El describe es correcto\n");
		struct_query->tabla = NULL;

		return DESCRIBE;
	  }

	  if (	(query_cant_palabras == 2) && (nombre_tabla_valido(query_split[1]))
	     )
	  {
		printf("El describe es correcto\n");
		string_to_upper(query_split[1]);
		struct_query->tabla = query_split[1];

		return DESCRIBE;

	  } else { 
		   printf("%s\n\n",MENSAJE_ERROR);
		   //printf("El describe es INcorrecto\n");
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"create")) {

	  if ( (query_cant_palabras != 5) )
	  {
		printf("%s\n\n",MENSAJE_ERROR);
		//printf("El create es INcorrecto\n");
		return ERROR;
	  }

	  if (	(nombre_tabla_valido(query_split[1])) 	   &&
		(tipo_consistencia_valido(query_split[2])) &&
		(valor_solo_numerico(query_split[3]))	   &&
		(valor_solo_numerico(query_split[4]))
	     )
	  {
		printf("El create es correcto\n");
		string_to_upper(query_split[1]);
		struct_query->requestType = CREATE;
		struct_query->tabla = query_split[1];
		struct_query->consistencyType = query_split[2];
		struct_query->key = atoi(query_split[3]);
		struct_query->value = query_split[4];
		struct_query->timestamp = NULL;

		return CREATE;

	  } else {
		   //printf("El create es INcorrecto\n");
		   printf("%s\n\n",MENSAJE_ERROR);
		   return ERROR;
		 }
		

	}


	if (!strcasecmp(query_split[0],"drop")) {

	  if ( (query_cant_palabras != 2) )
	  {
		//printf("El drop es INcorrecto\n");
		printf("%s\n\n",MENSAJE_ERROR);
		return ERROR;
	  }

	  if (	(nombre_tabla_valido(query_split[1]))
	     )
	  {
		//printf("El drop es correcto\n");
		string_to_upper(query_split[1]);
		struct_query->requestType = DROP;
		struct_query->tabla = query_split[1];
		struct_query->key = NULL;
		struct_query->value = NULL;
		struct_query->timestamp = NULL;

		return DROP;

	  } else {
		   //printf("El drop es INcorrecto\n");
		   printf("%s\n\n",MENSAJE_ERROR);
		   return ERROR;
		 }
		

	}



	if (!strcasecmp(query_split[0],"journal")) {

	  if ( (query_cant_palabras != 1) )
	  {
		//printf("El journal es INcorrecto\n");
		printf("%s\n\n",MENSAJE_ERROR);
		return ERROR;
	  } else {
			printf("El journal es correcto\n");
			string_to_upper(query_split[1]);
			struct_query->requestType = JOURNAL;
			struct_query->tabla = NULL;
			struct_query->key = NULL;
			struct_query->value = NULL;
			struct_query->timestamp = NULL;

			return JOURNAL;
		}
		

	}



	if (!strcasecmp(query_split[0],"run")) {

	  if ( (query_cant_palabras != 2) )
	  {
		//printf("El run es INcorrecto\n");
		printf("%s\n\n",MENSAJE_ERROR);
		return ERROR;
	  } else {
			printf("El run es correcto\n");
			struct_query->requestType = RUN;
			struct_query->script = query_split[1];
			return RUN;
		 }
	}

/*
	  if (	(nombre_tabla_valido(query_split[1]))
	     )
	  {
		printf("El run es correcto\n");
		string_to_upper(query_split[1]);
		struct_query->requestType = 8;
		struct_query->tabla = NULL;
		struct_query->key = NULL;
		struct_query->value = NULL;
		struct_query->timestamp = NULL;

		return RUN;

	  } else { printf("El run es INcorrecto\n");
		   return 0;
		 }
*/		

//	}

	//printf("Comando desconocido.\n\n");MENSAJE_ERROR
	printf("%s\n\n",MENSAJE_ERROR);
	return ERROR;

}


int string_size(char ** text)
{
	int i= 0;

//	printf("%s\n",text[i]);

	while ((text[i]) != '\0')
	{
//	printf("%d\n",i);
	  i = i+1;
	}

	return i;
}


int nombre_tabla_valido(char * text)
{

	string_to_upper(text);

	regex_t regex;
	int reti;
//		printf("%s\n",text);

	reti = regcomp(&regex,"^[A-Z0-9]*$",0);

	if (reti) {
	  fprintf(stderr,"Error al armar regex\n");
	  exit (1);
	}

	reti = regexec(&regex,text,0,NULL,0);

	if (!reti) {
	   printf("Nombre de tabla correcto\n");
	   return 1;
	}
	else if (reti == REG_NOMATCH) {
		printf("Nombre de tabla incorrecto\n");
		return 0;
	}
	else {
		fprintf(stderr,"Error en regex\n");
		exit (0);
	}
	
}



int valor_solo_numerico(char * text)
{

	regex_t regex;
	int reti;
//	printf("%s\n",text);

	reti = regcomp(&regex,"^[0-9]*$",0);

	if (reti) {
	  fprintf(stderr,"Error al armar regex\n");
	  exit (1);
	}

	reti = regexec(&regex,text,0,NULL,0);

	if (!reti) {
	   printf("Valor numerico correcto\n");
	   return 1;
	}
	else if (reti == REG_NOMATCH) {
		printf("Valor numerico incorrecto\n");
		return 0;
	}
	else {
		fprintf(stderr,"Error en regex\n");
		exit (0);
	}
	
}


int tipo_consistencia_valido(char * texto)
{

	if ( (!strcasecmp(texto,"sc")) || (!strcasecmp(texto,"ec")) || (!strcasecmp(texto,"shc")) )
	{
		printf("Consistencia valida\n");
		return 1;
	} else
	{
		printf("Consistencia INvalida\n");
		return 0;
	}
}






