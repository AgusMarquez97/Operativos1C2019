/*
 * Compactador.h
 *
 *  Created on: 12 abr. 2019
 *      Author: utnso
 */

#ifndef COMPACTADOR_H_
#define COMPACTADOR_H_

#include "Lissandra.h"


void compactar(query * queryCreate);
void renombrarArchivosTemporales(char* rutaDirectorioTabla);
t_list * obtenerRegistrosBinTmpc(char * tabla);
t_list* obtenerRegistrosCompactacion(char * registros);
t_list * obtenerListaCompactada(t_list * listaInicial);
void limpiarBloquesBitmap(char * directorio);
void compactarBinarios(char * tabla, t_list * listaRegistros, int cantidadParticiones);

void asignarBloquesABinarios(int tamanioNecesario, char * binario,char * listaRegistros);
void liberarQueryCreateLevantada(query * queryCreate);

char * asignarUnBloqueBin();

#endif /* COMPACTADOR_H_ */
