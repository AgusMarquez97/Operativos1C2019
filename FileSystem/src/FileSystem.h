/*
 * FileSystem.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "Lissandra.h"



void levantarServidor(char * servidorIP, char* servidorPuerto)
{
			query* myQuery = malloc(sizeof(query));
			//void * buffer;
	  	  	int socketServidor, socketRespuesta, maximoSocket;
	  	  	int datosRecibidos = -1;
			estructuraConexion* servidorObjetivo;
	        fd_set sockets, clientes;

	        char * info;
	        char * aux;

			socketServidor = levantarSocketGenerico(servidorIP,servidorPuerto,&servidorObjetivo);
			asociarPuerto(&socketServidor,servidorObjetivo);

	        free(servidorObjetivo);

	        //system("clear");
			escuchar(&socketServidor);

	        loggearInfo("Escuchando...\n");

	        //Manejo sets
	        LimpiarSet(&sockets);
	        LimpiarSet(&clientes);

	        agregarASet(socketServidor,&sockets);
	        maximoSocket = socketServidor;

		while(1) // Loop para escuchar conexiones entrantes
			{
	        clientes = sockets;

	        // EJ si se quiere definir un tiempo maximo
	            tiempoEspera esperaMaxima;
	            esperaMaxima.tv_sec = 500;
	            esperaMaxima.tv_usec = 0; //-> Espera como maximo 30 segundos hasta que pase algo


	        ejecutarSelect(maximoSocket,&clientes,&esperaMaxima); //pasarle null si no importa

	        for(int i = 0;i<=maximoSocket;i++) //Itero hasta el ultimo socket
	        	{
	        		if (estaEnSet(i, &clientes)) //1 - Tengo un cliente nuevo que quiere leer!
	        		{
	                    if (i == socketServidor) //1-1 - Ese Cliente es el mismo servidor -> Acepto nuevas conexiones
	                    {

	                    socketRespuesta = aceptarConexion(socketServidor);

	                    agregarASet(socketRespuesta,&sockets); //añado a sockets al nuevo cliente

	                        if (socketRespuesta > maximoSocket) // Trackeo el maximo socket
	                        {
	                            maximoSocket = socketRespuesta;
	                        }

	                    info = malloc(200);
	                    aux = malloc(10);
	                    strcpy(info,"Nueva conexion asignada al socket: ");
						snprintf(aux,10,"%d",socketRespuesta);
	                    strcat(info,aux);
	                    strcat(info,"\n");
	                    //printf("Nueva conexion asignada al socket: %d\n\n",socketRespuesta);// Guardo la conexion
	                    loggearInfo(info);
	                    free(info);
	                    free(aux);
	                    }
	                    else
	                    {

	                    	aux = malloc(10);
	                    	info = malloc(200);

	                    	datosRecibidos = recibirQuery(i,myQuery);


	                    	if(datosRecibidos==0)
	                    	{
	                    		close(i);
	                    		EliminarDeSet(i,&sockets);
	                    	}else  {
	                    		strcpy(info,"Se recibieron en total: ");
	                    		snprintf(aux,10,"%d",datosRecibidos);
	                    		strcat(info,aux);
	                    		strcat(info," bytes\n");
	                    		loggearInfo(aux);

	                    		free(aux);
	                    		free(info);
	                    	}

	                    }
	                	/*else   //1-2 - Tengo un cliente que quiere leer y es distinto del server
	                	{
	                    recibirDatos(i); // Recibo de ese cliente
	                    close(i);
	                    EliminarDeSet(i,&sockets);
	                	}*/
	        		}
	        	}
			}
}

#endif /* FILESYSTEM_H_ */
