#include "sockets.h"

void levantarCliente(char* servidorIP,char* servidorPuerto, void* datosAEnviar)
{
        int socketCliente;
        /*
    Aca vendria la parte de serializacion y deserializacion
        */
		estructuraConexion* servidorObjetivo;

		socketCliente = levantarSocketGenerico(servidorIP,servidorPuerto,&servidorObjetivo);

		conectarConServidor(&socketCliente,servidorObjetivo);

      	free(servidorObjetivo); //Ya estoy conectado, no necesito guardar mas esto

      		//Si es repetitivo agregar while(1)
        enviarDatos((char*)datosAEnviar,socketCliente);
        recibirDatos(socketCliente);
        close(socketCliente);

}


int levantarSocketGenerico(char* servidorIP,char* servidorPuerto,estructuraConexion** servidor)
{
			instanciarConexion(servidorIP,servidorPuerto,servidor);
			//imprimirDatosServidor(*servidor);
			return crearSocket(*servidor);
}


//Instancia estructuras inicales
void instanciarConexion(char * direcIP,char * puertoDesc,estructuraConexion** estructuraFinal)
{
	estructuraConexion estructuraInicial;
	limpiarEstructuraInicial(&estructuraInicial);

      if(getaddrinfo(direcIP,puertoDesc,&estructuraInicial,estructuraFinal) != 0)
      {
          //fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
          perror("Error, no se pudieron crear las estructuras");
      }

}

void limpiarEstructuraInicial(estructuraConexion* estructuraInicial)
{

    estructuraInicial->ai_family = AF_UNSPEC;
    estructuraInicial->ai_socktype = SOCK_STREAM;
    estructuraInicial->ai_flags = 0;  //Alternativa para servidor AI_PASSIVE
    estructuraInicial->ai_protocol = 0;

}

int crearSocket(estructuraConexion* estructura)
{
		int socketResultado = socket(estructura->ai_family,estructura->ai_socktype,estructura->ai_protocol);
	    if(socketResultado == -1)
	    {
	        perror("No se pudo asignar un socket");
	        exit(1);
	    }
	    return socketResultado;
}


int conectarConServidor(int* socketCliente,estructuraConexion* estructuraServidor)
{
	int conexionAServidor;
	conexionAServidor = connect(*socketCliente, estructuraServidor->ai_addr, estructuraServidor->ai_addrlen);
    if(conexionAServidor==-1)
        {
            perror("No se pudo realizar la conexion");
            //close(socketCliente);
            exit(1);
        }
    return conexionAServidor;

}


//---------------------------------------------EXTRAS---------------------------------------------

void imprimirDatosServidor(estructuraConexion* estructuraObjetivo)
{
    char serverIP[NI_MAXHOST]; 		//IP Server
    char serverPuerto[NI_MAXSERV];  //Puerto Servidor
	getnameinfo(estructuraObjetivo->ai_addr, sizeof(*estructuraObjetivo->ai_addr), serverIP, sizeof(serverIP), serverPuerto, sizeof(serverPuerto), NI_NUMERICHOST|NI_NUMERICSERV);
	printf("Servidor IP: %s\n",serverIP);
	printf("Servidor Puerto:  %s\n",serverPuerto);

}


void imprimirDatosCliente(estructuraConexionEntrante estructuraObjetivo,socklen_t direc_tam)
{

    char clienteIP[NI_MAXHOST]; 	//IP Cliente
    char clientePuerto[NI_MAXSERV];	//Puerto Cliente

	getnameinfo((struct sockaddr *)&estructuraObjetivo, direc_tam, clienteIP, sizeof(clienteIP), clientePuerto, sizeof(clientePuerto), NI_NUMERICHOST|NI_NUMERICSERV);
	printf("Cliente IP = %s\n",clienteIP);
	printf("Cliente Puerto =  %s\n",clientePuerto);

}

void enviarDatos(void* datosAEnviar,int socketConexion)
{

	int cantidadEnviada = -1;

	//Agregar funcionalidad para enviar de a partes
	if(strlen(datosAEnviar) >= tam_Max_Transmision)
	{
	cantidadEnviada = send(socketConexion,datosAEnviar,tam_Max_Transmision,0); //flag= MSG_DONTWAIT no bloqueante
	}
	else
	{
	cantidadEnviada = send(socketConexion,datosAEnviar,strlen(datosAEnviar),0);
	}

	    if(cantidadEnviada==-1)
	    {
	        perror("No se pudieron enviar los datos al cliente");
	        exit(1);
        }
	    //printf("Se enviaron: %d bytes\n",cantidadEnviada);
        printf("Se envio: %s",(char*)datosAEnviar);
}


int recibirDatos(int socketConexion)
{
        void* buffer[tam_Max_Transmision]; //Buffer para recibir
		memset(buffer, '\0', sizeof(buffer));
		int cantidadRecibida;
		cantidadRecibida = recv(socketConexion,buffer,tam_Max_Transmision,0);
	    if(cantidadRecibida==-1)
	    {
	        perror("No se pudieron recibir los datos del cliente");
	        exit(1);
	    }
        /*if(cantidadRecibida==0)
	    {
	        perror("Se cerro la conexion");
	        exit(1);
	    }*/


        //printf("Se recibieron: %d Bytes\n",cantidadRecibida);
	    if(cantidadRecibida>0)
	    {
        printf("Se recibio: %s\n",(char*)buffer);
	    }
	    return cantidadRecibida;
}





//------------------ SERVIDOR-------------

void levantarServidor(char * servidorIP, char* servidorPuerto)
{
	  	  	int socketServidor, socketRespuesta, maximoSocket;
	  	  	int datosRecibidos;
			estructuraConexion* servidorObjetivo;
	        fd_set sockets, clientes;

			socketServidor = levantarSocketGenerico(servidorIP,servidorPuerto,&servidorObjetivo);
			asociarPuerto(&socketServidor,servidorObjetivo);

	        free(servidorObjetivo);

	        //system("clear");
			escuchar(&socketServidor);

	        printf("Escuchando...\n");

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
	                    printf("Nueva conexion asignada al socket: %d\n\n",socketRespuesta);// Guardo la conexion
	                    }
	                    else
	                    {
	                    	datosRecibidos = recibirDatos(i); // Recibo de ese cliente
	                    	if(datosRecibidos==0)
	                    	{
	                    		printf("Se cerro la conexion con el socket: %d\n",i);
	                    		close(i);
	                    		EliminarDeSet(i,&sockets);
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

void asociarPuerto(int *socketServidor,estructuraConexion* estructuraServidor)
{
    int yes = 1;
    setsockopt(*socketServidor, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)); //Valida que se pueda usar el puerto

    if(bind(*socketServidor,estructuraServidor->ai_addr,estructuraServidor->ai_addrlen)==-1)
        {
            perror("No se pudo asociar el socket al puerto");
            exit(1);
        }
}

void escuchar(int * socketServidor)
{
    if(listen(*socketServidor,backlog) == -1)
    {
        perror("Fallo al escuchar");
        exit(1);
    }
}


int aceptarConexion(int socketServidor)
{
		int socketCliente = -1;

        estructuraConexionEntrante conexionEntrante;
        socklen_t direc_tam = sizeof(struct sockaddr_storage);

		socketCliente=accept(socketServidor,(struct sockaddr *)&conexionEntrante,&direc_tam);

	    if(socketCliente == -1)
	    {
	        perror("Fallo al aceptar conexion ");
	        exit(1);
	    }

	    printf("\n-----------------Se acepta al cliente: ------------------- \n\n");
	    imprimirDatosCliente(conexionEntrante,direc_tam);	//printf("Se crea el socket: %d\n",socketCliente);
	    printf("\n");
	    return socketCliente; // Retorna el socket asociado a un cliente que quiere conectarse al servidor
}

//Select:
void ejecutarSelect(int maxSocket,fd_set *clientes, tiempoEspera* tiempo) //Pasar null a tiempo si no importa
{
    int retornoSelect;
        retornoSelect  = select(maxSocket+1,clientes,NULL,NULL,tiempo);

         if (retornoSelect == -1) {
            perror("Error al ejecutar el select: ");
            exit(4);
        }
		 if (retornoSelect == 0) {
            printf("Se termino el tiempo de espera del servidor...");
            exit(4);
        }

}

//Manejo de Sets:

int estaEnSet(int socketNuevo,fd_set* setSockets)
{
    return FD_ISSET(socketNuevo,setSockets);
}
void EliminarDeSet(int socketNuevo,fd_set* setSockets)
{
    FD_CLR(socketNuevo,setSockets);
}
void agregarASet(int socketNuevo,fd_set* setSockets)
{
    FD_SET(socketNuevo,setSockets);
}
void LimpiarSet(fd_set* setSockets)
{
    FD_ZERO(setSockets);
}
