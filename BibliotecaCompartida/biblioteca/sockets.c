#include "sockets.h"

int levantarCliente(char* servidorIP,char* servidorPuerto)
{
		int socketCliente;

		estructuraConexion* servidorObjetivo;

		socketCliente = levantarSocketGenerico(servidorIP,servidorPuerto,&servidorObjetivo);

		conectarConServidor(&socketCliente,servidorObjetivo);

      	free(servidorObjetivo); //Ya estoy conectado

        return socketCliente;

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

          //perror("Error, no se pudieron crear las estructuras");
          loggearError("Error, no se pudieron crear las estructuras");
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
	        //perror("No se pudo asignar un socket");
	        loggearError("Error al crear el socket");
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
            //perror("No se pudo realizar la conexion");
            loggearError("No se pudo realizar la conexion");
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

	//printf("Servidor IP: %s\n",serverIP);
	//printf("Servidor Puerto:  %s\n",serverPuerto);

	char * ip = malloc(50);
	strcpy(ip,"IP del servidor: ");
	strcat(ip,serverIP);

	char * puerto = malloc(50);
	strcpy(puerto,"Puerto de escucha: ");
	strcat(puerto,serverPuerto);

	loggearInfo(ip);
	loggearInfo(puerto);

	free(ip);
	free(puerto);

}


void imprimirDatosCliente(estructuraConexionEntrante estructuraObjetivo,socklen_t direc_tam)
{

    char clienteIP[NI_MAXHOST]; 	//IP Cliente
    char clientePuerto[NI_MAXSERV];	//Puerto Cliente

	getnameinfo((struct sockaddr *)&estructuraObjetivo, direc_tam, clienteIP, sizeof(clienteIP), clientePuerto, sizeof(clientePuerto), NI_NUMERICHOST|NI_NUMERICSERV);

	//printf("Cliente IP = %s\n",clienteIP);
	//printf("Cliente Puerto =  %s\n",clientePuerto);


	char * ip = malloc(50);
	strcpy(ip,"IP del cliente objetivo: ");
	strcat(ip,clienteIP);

	char * puerto = malloc(50);
	strcpy(puerto,"Puerto de escucha: ");
	strcat(puerto,clientePuerto);

	free(ip);
	free(puerto);
}

int enviar(int socketConexion, void* datosAEnviar, int32_t tamanioAEnviar){

	int bytesTotales = 0;

	loggearInfo("Enviando...");

	while (tamanioAEnviar - bytesTotales > 0) {

		int bytesEnviados = send(socketConexion, datosAEnviar + bytesTotales, tamanioAEnviar - bytesTotales, 0);

		if(bytesEnviados < 0) {
			//salirConError("No se pudieron enviar los datos al cliente", socketConexion);
			loggearError("No se pudieron enviar los datos al cliente");
		}
		bytesTotales += bytesEnviados;
	}
		//Luego ver de usar itoa y  -> bytesEnviados
	loggearInfo("Envio completo");

	return bytesTotales; //los mando por si alguien lo necesita

}

int recibir(int socketConexion, void* buffer,int32_t tamanioARecibir) {

	loggearInfo("Recibiendo...");
	int bytesTotales = recv(socketConexion, buffer, tamanioARecibir, MSG_WAITALL);

	if(bytesTotales <= 0) {
		if(bytesTotales < 0){
			loggearError("No se pudieron recibir los datos del cliente");
		}
		else {
			loggearWarning("Se cerro la conexion con el cliente");
		}
	}

	if(bytesTotales < tamanioARecibir){
		loggearError("No se puedo recibir todos los datos");
	}

	//loggearInfo("recibido completo");

	return bytesTotales; //los mando por si alguien lo necesita
}


//------------------ SERVIDOR-------------

void levantarServidor(char * servidorIP, char* servidorPuerto)
{
			void * buffer;
	  	  	int socketServidor, socketRespuesta, maximoSocket;
	  	  	int datosRecibidos;
			estructuraConexion* servidorObjetivo;
	        fd_set sockets, clientes;

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
	                    char * info = malloc(70);
	                    strcpy(info,"Nueva conexion asignada al socket: ");
	                    strcat(info,socketRespuesta);
	                    //printf("Nueva conexion asignada al socket: %d\n\n",socketRespuesta);// Guardo la conexion
	                    loggearInfo(info);
	                    }
	                    else
	                    {
	                    	int tam;
	                    	datosRecibidos = recibirInt(i,&tam); // Recibo de ese cliente

	                    	if(datosRecibidos==0)
	                    	{
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
            loggearError("No se pudo asociar el socket al puerto");
            exit(1);
        }
}

void escuchar(int * socketServidor)
{
    if(listen(*socketServidor,backlog) == -1)
    {
    	loggearError("Fallo al escuchar");
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
	    	loggearError("Fallo al aceptar conexion ");
	        exit(1);
	    }

	    loggearInfo("\n-----------------Se acepta al cliente: ------------------- \n\n");
	    imprimirDatosCliente(conexionEntrante,direc_tam);	//printf("Se crea el socket: %d\n",socketCliente);

	    return socketCliente; // Retorna el socket asociado a un cliente que quiere conectarse al servidor
}

//Select:
void ejecutarSelect(int maxSocket,fd_set *clientes, tiempoEspera* tiempo) //Pasar null a tiempo si no importa
{
    int retornoSelect;
        retornoSelect  = select(maxSocket+1,clientes,NULL,NULL,tiempo);

         if (retornoSelect == -1) {
        	loggearError("Error al ejecutar el select: ");
            exit(4);
        }
		 if (retornoSelect == 0) {
			loggearError("Se termino el tiempo de espera del servidor...");
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
