/*
 * sockets.h
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <curses.h>
#include <sys/select.h>
#include <sys/time.h>

#include "logs.h"

//#include "utils.h"

//#define puerto "8080"
#define tam_Max_Transmision 30
#define backlog 15

typedef struct addrinfo estructuraConexion;
typedef struct sockaddr_storage estructuraConexionEntrante; //Guarda datos de conexiones entrantes
typedef struct timeval tiempoEspera;


int32_t levantarCliente(char* servidorIP,char* servidorPuerto);
void levantarServidor(char * servidorIP, char* servidorPuerto);

                        //Estructuras
void limpiarEstructuraInicial(estructuraConexion* estructuraInicial);
void instanciarConexion(char * direcIP,char * puertoDesc,estructuraConexion** estructuraFinal);

                        //Sockets
int levantarSocketGenerico(char* servidorIP,char* servidorPuerto,estructuraConexion** servidor);
int crearSocket(estructuraConexion* estructura);

                        //Conexiones
int conectarConServidor(int* socketCliente,estructuraConexion* estructuraServidor);


                        //Envio y recepcion de datos


int enviar(int socketConexion, void* datosAEnviar, int32_t tamanioAEnviar);
int recibir(int socketConexion, void* buffer,int32_t tamanioARecibir);

                        //Servidor
void asociarPuerto(int *socketServidor,estructuraConexion* estructuraServidor);
void escuchar(int * socketServidor);
int aceptarConexion(int socketServidor);

						//Select:
void ejecutarSelect(int maxSocket,fd_set *clientes, tiempoEspera* tiempo);

//Manejo con sets:
int estaEnSet(int socketNuevo,fd_set* setSockets);
void agregarASet(int socketNuevo,fd_set* setSockets);
void buscarEnSet(int socketNuevo,fd_set* setSockets);
void EliminarDeSet(int socketNuevo,fd_set* setSockets);
void LimpiarSet(fd_set* setSockets);


                        //Impresion de datos
void imprimirDatosServidor(estructuraConexion* estructuraObjetivo);
void imprimirDatosCliente(estructuraConexionEntrante estructuraObjetivo,socklen_t direc_tam);


#endif /* SOCKETS_H_ */
