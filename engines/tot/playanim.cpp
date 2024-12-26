/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/textconsole.h"

#include "tot/chrono.h"
#include "tot/playanim.h"

namespace Tot {

int doserror = 0;
int exitcode = 0;


Common::MemorySeekableReadWriteStream *conversationData;
Common::MemorySeekableReadWriteStream *rooms;
Common::MemorySeekableReadWriteStream *invItemData;

byte iraton;

uint xraton, yraton;

uint pulsax, pulsay;

uint npraton2, npraton;

uint oldxrejilla, oldyrejilla;

regispartida regpartida;

boolean sello_quitado;

boolean cambiopantalla;
boolean teleencendida,
	vasijapuesta,
	guadagna,
	tridente,
	torno,
	barro,
	diablillo_verde,
	rojo_capturado,
	manual_torno,
	alacena_abierta,
	baul_abierto,
	trampa_puesta,
	peteractivo;

boolean dentro_del_juego;

boolean hechaprimeravez;

boolean salirdeljuego;

boolean partidanueva;

boolean continuarpartida;

boolean desactivagrabar;

boolean pintaractivo;

boolean animacion2;

palette movimientopal;

palette pal;

reginventario mobj[inventoryIconCount];

inventoryBitmaps mochilaxms;

byte saltospal;

byte posicioninv;

byte numeroaccion;

byte oldnumeroacc;

byte pasos;

byte indicepuertas;

byte movidapaleta;

byte rejillaquetoca;

byte parte_del_juego;
byte encripcod1;

byte secondaryAnimationFrameCount;

byte numerodir;

byte contadorpc, contadorpc2;

byte indaux1, indaux2;

byte destinox_paso, destinoy_paso;

byte direccionmovimiento;

uint anchoanimado, altoanimado;

uint tiempo;

uint codigoobjmochila;

uint kaka;

uint oldposx, oldposy;
uint volumenfxderecho, volumenfxizquierdo;
uint segpasoicono;
uint ofspasoicono;
uint volumenmelodiaderecho, volumenmelodiaizquierdo;


int elemento1, elemento2;

int characterPosX, characterPosY;

int xframe2, yframe2;

Common::File verb;

Common::String oldobjmochila, objetomochila;

Common::String nombreficherofoto;

Common::String nombrepersonaje;

char *encriptado;
uint hornacina[2][4];

RoomFileRegister *roomData;

InvItemRegister regobj;

route mainRoute;

Common::Point trayec[300];

uint longtray;

uint indicetray;

uint indicetray2;

byte zonaactual, zonadestino, oldzonadestino;

byte maxrejax, maxrejay;

byte rejafondomovto[10][10];

byte rejafondoraton[10][10];

byte rejamascaramovto[10][10];

byte rejamascararaton[10][10];

boolean completadalista1, completadalista2,
	lista1, lista2;


boolean  primera[maxpersonajes],
				lprimera[maxpersonajes],
				cprimera[maxpersonajes],
				libro[maxpersonajes],
				caramelos[maxpersonajes];

boolean  cavernas[5];

boolean  firstList[5], secondList[5];

regsecuencia secuencia;
reganimado animado;
uint sizeframe,
	segpasoframe,
	ofspasoframe,
	sizeanimado,
	segpasoanimado,
	ofspasoanimado,
	_handpantalla,
	segfondo,
	offfondo;

byte fotogramamax2;

byte tipoefectofundido;

byte iframe, iframe2;

long sizepantalla;

datosobj profundidad[numobjetosconv];

byte *objetos[numobjetosconv];

byte *pasoframe;

byte *pasoanimado;

byte *fondo;

byte *characterDirtyRect;

byte *handpantalla;

uint roomNumber;

boolean isLoadingFromLauncher;


void clearObj() {
	byte indpasolimpiador1, indpasolimpiador2;

	regobj.code = 0;
	regobj.altura = 0;
	regobj.name = "";
	regobj.lookAtTextRef = 0;
	regobj.beforeUseTextRef = 0;
	regobj.afterUseTextRef = 0;
	regobj.pickTextRef = 0;
	regobj.useTextRef = 0;
	regobj.habla = 0;
	regobj.abrir = false;
	regobj.cerrar = false;
	for (indpasolimpiador1 = 0; indpasolimpiador1 <= 7; indpasolimpiador1++)
		regobj.usar[indpasolimpiador1] = 0;
	regobj.coger = false;
	regobj.usarcon = 0;
	regobj.reemplazarpor = 0;
	regobj.profundidad = 0;
	regobj.punterobitmap = 0;
	regobj.tambitmap = 0;
	regobj.punteroframesgiro = 0;
	regobj.punteropaletagiro = 0;
	regobj.xparche = 0;
	regobj.yparche = 0;
	regobj.puntparche = 0;
	regobj.tamparche = 0;
	regobj.objectIconBitmap = 0;
	regobj.xrej1 = 0;
	regobj.yrej1 = 0;
	regobj.xrej2 = 0;
	regobj.yrej2 = 0;
	for (indpasolimpiador1 = 0; indpasolimpiador1 < 10; indpasolimpiador1++) {
		for (indpasolimpiador2 = 0; indpasolimpiador2 < 10; indpasolimpiador2++) {
			regobj.parcherejapantalla[indpasolimpiador1][indpasolimpiador2] = 0;
			regobj.parcherejaraton[indpasolimpiador1][indpasolimpiador2] = 0;
		}
	}
	contadorpc2 = contadorpc;
}

void clearScreenData() {
	// debug("Clearing screen data! Shouldnt be necessary now!");
	// byte ilocal1, ilocal2, ilocal3;
	// uint ilocal4;

	// contadorpc2 = contadorpc;
	// roomData->codigo = 0;
	// roomData->puntimagenpantalla = 0;
	// roomData->tamimagenpantalla = 0;
	// for (ilocal1 = 0; ilocal1 <= 39; ilocal1++)
	// 	for (ilocal2 = 0; ilocal2 <= 27; ilocal2++) {
	// 		roomData->rejapantalla[ilocal1][ilocal2] = 0;
	// 		roomData->mouseGrid[ilocal1][ilocal2] = 0;
	// 	}
	// for (ilocal1 = 1; ilocal1 <= 9; ilocal1++)
	// 	for (ilocal2 = 1; ilocal2 <= 30; ilocal2++)
	// 		for (ilocal3 = 1; ilocal3 <= 5; ilocal3++) {
	// 			roomData->trayectories[ilocal1][ilocal2][ilocal3].x = 9999;
	// 			roomData->trayectories[ilocal1][ilocal2][ilocal3].y = 9999;
	// 		}
	// for (ilocal1 = 1; ilocal1 <= 5; ilocal1++) {
	// 	roomData->doors[ilocal1].pantallaquecarga = 0;
	// 	roomData->doors[ilocal1].posxsalida = 0;
	// 	roomData->doors[ilocal1].posysalida = 0;
	// 	roomData->doors[ilocal1].abiertacerrada = 0;
	// 	roomData->doors[ilocal1].codigopuerta = 0;
	// }
	// for (ilocal1 = 1; ilocal1 <= 15; ilocal1++) {
	// 	roomData->bitmapasociados[ilocal1].puntbitmap = 0;
	// 	roomData->bitmapasociados[ilocal1].tambitmap = 0;
	// 	roomData->bitmapasociados[ilocal1].coordx = 0;
	// 	roomData->bitmapasociados[ilocal1].coordy = 0;
	// 	roomData->bitmapasociados[ilocal1].profund = 0;
	// }
	// for (ilocal1 = 0; ilocal1 <= 50; ilocal1++) {
	// 	// if(roomData->indexadoobjetos[ilocal1] != NULL) {
	// 	//  	roomData->indexadoobjetos[ilocal1]->indicefichero = 0;
	// 	//  	roomData->indexadoobjetos[ilocal1]->objectName = "";
	// 	//  }
	// }
	// roomData->banderamovimiento = false;
	// roomData->nombremovto = "QQQQQQQQ";
	// roomData->movementNameSize = 8;
	// roomData->banderapaleta = false;
	// roomData->puntpaleta = 0;
	// for (ilocal4 = 1; ilocal4 <= 300; ilocal4++) {
	// 	roomData->tray2[ilocal4].x = 0;
	// 	roomData->tray2[ilocal4].y = 0;
	// 	roomData->dir2[ilocal4] = 0;
	// }
	// roomData->longtray2 = 0;
}

/**
 * Originally the Room file contains 8 copies of each room, one for every save plus the baseline (which is 0).
 * To put this into memory we need to get the baseline of each room and then put them continuously in a byte stream.addr
 * Whenever the game access a room instead of accessing the room for the autosave or the current save,
 * we assume only one room register is there.
 *
 * To save a game we merely copy the entire stream into the save.
*/
void initializeScreenFile() {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("PANTALLA.DAT"))) {
		error("Error opening room files: ioresult (320)");
	}
	int roomNumber = 0;
	int64 fileSize = roomFile.size();
	delete (rooms);


	byte *roomData = (byte *)malloc(roomRegSize * 32);
	int roomCount = 0;

	while (!roomFile.eos()) {
		if (fileSize - roomFile.pos() >= roomRegSize) {
			roomFile.read(roomData + roomRegSize * roomCount, roomRegSize);
			// This one doesnt work for some reason:
			// rooms->writeStream(roomFile.readStream(roomRegSize), roomRegSize);
			roomFile.skip(roomRegSize * 7);
			roomCount++;
		} else {
			break;
		}
	}
	rooms = new Common::MemorySeekableReadWriteStream(roomData, roomRegSize * roomCount, DisposeAfterUse::NO);
	roomFile.close();
}

void final_varanima() {
	// switch (exitcode) {
	// case 0: {
	// 	output << "Gracias por jugar con otro producto ACA Soft." << NL;
	// 	freemem(pasoicono, sizeicono);
	// } break;
	// case 1:
	// 	output << "Error 1: Funci�n no existe." << NL;
	// 	break;
	// case 2:
	// 	output << "Error 2: Fichero no encontrado." << NL;
	// 	break;
	// case 3:
	// 	output << "Error 3: Camino no encontrado." << NL;
	// 	break;
	// case 4:
	// 	output << "Error 4: Demasiados ficheros abiertos." << NL;
	// 	break;
	// case 5:
	// 	output << "Error 5: Acceso denegado." << NL;
	// 	break;
	// case 6:
	// 	output << "Error 6: Asignacion de fichero incorrecta." << NL;
	// 	break;
	// case RANGE_5(7, 11):
	// 	output << "Error " << exitcode << string(": Espera... La placa madre est�") + " teniendo gemelos." << NL;
	// 	break;
	// case 12:
	// 	output << "Error 12: C�digo de acceso a fichero incorrecto." << NL;
	// 	break;
	// case 13:
	// case 14:
	// 	output << "Error " << exitcode << string(": �Alguien ha visto por ah� un") + " bol�grafo?." << NL;
	// 	break;
	// case 15:
	// 	output << "Error 15: Unidad incorrecta." << NL;
	// 	break;
	// case 16:
	// 	output << "Error 16: No se puede borrar el directorio." << NL;
	// 	break;
	// case 17:
	// 	output << "Error 17: No se puede renombrar en distintas unidades." << NL;
	// 	break;
	// case 18:
	// 	output << "Error 18: No hay m�s ficheros." << NL;
	// 	break;
	// case 19 ... 99:
	// 	output << "Error " << exitcode << ": Mi, mi, mi, mi, mi....  etc." << NL;
	// 	break;
	// case 100:
	// 	output << "Error 100: No se pudo leer del disco." << NL;
	// 	break;
	// case 101:
	// 	output << "Error 101: No se pudo escribir en el disco." << NL;
	// 	break;
	// case 102:
	// 	output << "Error 102: Fichero sin asignar." << NL;
	// 	break;
	// case 103:
	// 	output << "Error 103: Fichero sin abrir." << NL;
	// 	break;
	// case 104:
	// 	output << "Error 104: Fichero abierto para salida." << NL;
	// 	break;
	// case 105:
	// 	output << "Error 105: Fichero abierto para entrada." << NL;
	// 	break;
	// case 106:
	// 	output << "Error 106: Formato num�rico no v�lido." << NL;
	// 	break;
	// case 107 ... 149:
	// 	output << "Error " << exitcode << ": Tr�fico intenso en el bus de datos." << NL;
	// 	break;
	// case 150:
	// 	output << "Error 150: Disco protegido contra escritura." << NL;
	// 	break;
	// case 151:
	// 	output << string("Error 151: El driver del disco devuelve una estructura") +
	// 				  " incorrecta."
	// 		   << NL;
	// 	break;
	// case 152:
	// 	output << "Error 152: Unidad no preparada." << NL;
	// 	break;
	// case 153:
	// 	output << "Error " << exitcode << ": JOZU!! que Barbari�." << NL;
	// 	break;
	// case 154:
	// 	output << string("Error 154: Comprobaci�n err�nea en el control de redundancia") +
	// 				  " c�clica."
	// 		   << NL;
	// 	break;
	// case 155:
	// 	output << "Error " << exitcode << ": Cerrado por vacaciones." << NL;
	// 	break;
	// case 156:
	// 	output << "Error 156: Posicionamiento err�neo en el fichero." << NL;
	// 	break;
	// case 157:
	// 	output << "Error 157: Disco de formato desconocido." << NL;
	// 	break;
	// case 158:
	// 	output << "Error 158: Sector no encontrado en el disco." << NL;
	// 	break;
	// case 159:
	// 	output << "Error 159: Impresora sin papel." << NL;
	// 	break;
	// case 160:
	// 	output << "Error 160: No se pudo escribir en el dispositivo." << NL;
	// 	break;
	// case 161:
	// 	output << "Error 161: No se pudo leer del dispositivo." << NL;
	// 	break;
	// case 162:
	// 	output << "Error 162: Problema con el dispositivo." << NL;
	// 	break;
	// case 163 ... 199:
	// 	output << "Error " << exitcode << ": No sabe, no contesta." << NL;
	// 	break;
	// case 200:
	// 	output << "Error 200: Division por cero." << NL;
	// 	break;
	// case 201:
	// 	output << "Error 201: Variable fuera de rango." << NL;
	// 	break;
	// case 202:
	// 	output << "Error 202: Desbordamiento en la pila." << NL;
	// 	break;
	// case 203:
	// 	output << "Error 203: Desbordamiento en la memoria din�mica." << NL;
	// 	break;
	// case 204:
	// 	output << "Error 204: Operaci�n err�nea con un puntero." << NL;
	// 	break;
	// case 205:
	// 	output << "Error 205: Desbordamiento en coma flotante." << NL;
	// 	break;
	// case 206:
	// 	output << "Error 206: Desbordamiento negativo en coma flotante." << NL;
	// 	break;
	// case 207:
	// 	output << "Error 207: Operaci�n err�nea de coma flotante." << NL;
	// 	break;
	// case 208:
	// 	output << "Error 208: Gestor de solapamiento sin instalar." << NL;
	// 	break;
	// case 209:
	// 	output << "Error 209: Lectura err�nea del fichero de solapamiento." << NL;
	// 	break;
	// case 210:
	// 	output << "Error 210: Objeto sin inicializar." << NL;
	// 	break;
	// case 211:
	// 	output << "Error 211: Llamada a un metodo abstracto." << NL;
	// 	break;
	// case 212:
	// 	output << "Error 212: Cadena de asignaci�n del objeto err�nea." << NL;
	// 	break;
	// case 213:
	// 	output << "Error 213: Indice para colecci�n fuera de rango." << NL;
	// 	break;
	// case 214:
	// 	output << "Error 214: Desbordamiento en la colecci�n." << NL;
	// 	break;
	// case 215:
	// 	output << "Error 215: Desbordamiento en operaci�n aritm�tica." << NL;
	// 	break;
	// case 216:
	// 	output << "Error 216: Acceso a memoria sin asignar." << NL;
	// 	break;
	// case 217 ... 249:
	// 	output << "Error " << exitcode << ": Dios mio... estoy lleno de chips." << NL;
	// 	break;
	// case 250:
	// 	output << "Error 250: El driver de AdLib no est� instalado." << NL;
	// 	break;
	// case 251:
	// 	output << "Error 251: Libre para AdLib." << NL;
	// 	break;
	// case 252:
	// 	output << string("Error 252: No se encontr� el fichero de efectos") + " para SBlaster." << NL;
	// 	break;
	// case 253:
	// 	output << string("Error 253: No se encontr� el fichero de m�sica ") + "para SBlaster." << NL;
	// 	break;
	// case 254:
	// 	output << string("Error 254: Libre para el fichero de configuraci�n del") +
	// 				  " sonido."
	// 		   << NL;
	// 	break;
	// case 255:
	// 	output << "Error 255: Detectado Ctrl-Break." << NL;
	// 	break;
	// case 256:
	// 	output << string("Error 256: La tarjeta de video no soporta el modo gr�fico ") + "requerido." << NL;
	// 	break;
	// case 257:
	// 	output << "Error 257: El tipo de pantalla no es correcto." << NL;
	// 	break;
	// case 258:
	// 	output << "Error 258: No se encontr� el fichero con la imagen del menu." << NL;
	// 	break;
	// case 259:
	// 	output << "Error 259: Clave de protecci�n erronea." << NL;
	// 	break;
	// case 260:
	// 	output << "Error 260: No se encontr� el driver del modo X de v�deo." << NL;
	// 	break;
	// case 261:
	// 	output << string("Error 261: No se encontr� el fichero de datos de los ") + "objetos." << NL;
	// 	break;
	// case 262:
	// 	output << "Error 262: No se encontr� el fichero de los BITMAP." << NL;
	// 	break;
	// case 263:
	// 	output << string("Error 263: No se encontr� el fichero de paleta para la ") + "imagen girando." << NL;
	// 	break;
	// case 264:
	// 	output << "Error 264: Se desbord� la pila de montaje de bitmaps" << NL;
	// 	break;
	// case 265:
	// 	output << "Error 265: No se encontr� el fichero de la animaci�n." << NL;
	// 	break;
	// case 266:
	// 	output << "Error 266: No se encontr� el fichero del efecto de sonido." << NL;
	// 	break;
	// case 267:
	// 	output << "Error 267: No se encontr� el fichero de la melodia." << NL;
	// 	break;
	// case 268:
	// 	output << "Error 268: No se encontr� el driver de sonido MIDI." << NL;
	// 	break;
	// case 269:
	// 	output << "Error 269: No se pudo capturar un vector de interrupci�n." << NL;
	// 	break;
	// case 270:
	// 	output << "Error 270: No se encontr� el fichero de los cr�ditos." << NL;
	// 	break;
	// case 271:
	// 	output << "Error 271: No se encontr� el fichero del BitMap del sello." << NL;
	// 	break;
	// case 272:
	// 	output << "Error 272: No se encontr� el fichero de la animaci�n." << NL;
	// 	break;
	// case 273:
	// 	output << string("Error 273: No se encontraron los �ndices de los modos X de") +
	// 				  " acceso a v�deo."
	// 		   << NL;
	// 	break;
	// case 274:
	// 	output << string("Error 274: Fichero de datos corrupto, instale de nuevo el") +
	// 				  " programa."
	// 		   << NL;
	// 	break;
	// case 275:
	// 	output << "Error 275: Posici�n incorrecta del Sprite." << NL;
	// 	break;
	// case 276:
	// 	output << "Error 276: Alineaci�n incorrecta del driver de sonido." << NL;
	// 	break;
	// case 277:
	// 	output << "Error 277: No se encontr� el fichero de la animaci�n." << NL;
	// 	break;
	// case 278:
	// 	output << "Error 278: No se encontr� el fichero para la actualizaci�n." << NL;
	// 	break;
	// case 279:
	// 	output << "Error 279: No se actualiz� el puntero del rat�n." << NL;
	// 	break;
	// case 280:
	// 	output << "Error 280: No se encontr� el puntero de video." << NL;
	// 	break;
	// case 281:
	// 	output << "Error 281: No se actualiz� el puntero de video." << NL;
	// 	break;
	// case 282:
	// 	output << "Error 282: No se actualiz� el flag del trace." << NL;
	// 	break;
	// case RANGE_16(283, 298):
	// 	output << "Error " << exitcode << ": Coffe not found. Operator Halted." << NL;
	// 	break;
	// case 299:
	// 	output << "Error 299: No se pudo reubicar los datos en la memoria XMS." << NL;
	// 	break;
	// case 300:
	// 	output << "Error 300: No hay memoria XMS suficiente." << NL;
	// 	break;
	// case 301:
	// 	output << "Error 301: No se pudo reservar memoria XMS suficiente." << NL;
	// 	break;
	// case 302:
	// 	output << "Error 302: No se pudo liberar la memoria XMS." << NL;
	// 	break;
	// case 303:
	// 	output << "Error 303: No se encuentra disponible un handle para XMS." << NL;
	// 	break;
	// case 304:
	// 	output << "Error 304: No se encontr� el fichero de gr�ficos." << NL;
	// 	break;
	// case 305:
	// 	output << string("Error 305: Necesita una versi�n 6.0 o posterior del driver") + " del rat�n." << NL;
	// 	break;
	// case 306:
	// 	output << string("Error 306: No se pudo inicializar correctamente el driver ") + "del rat�n." << NL;
	// 	break;
	// case 307:
	// 	output << "Error 307: No hay memoria baja suficiente. " << memavail() / 1024 << NL;
	// 	break;
	// case 308:
	// 	output << string("Error 308: No se pudo inicializar correctamente el modo") + " gr�fico." << NL;
	// 	break;
	// case 309:
	// 	output << string("Error 309: No se pudieron inicializar correctamente los ") + "tipos de letra." << NL;
	// 	break;
	// case 310:
	// 	output << "Error 310: No se encontr� el fichero de paleta." << NL;
	// 	break;
	// case 311:
	// 	output << "Error 311: No se encontr� el fichero de paleta para fundido." << NL;
	// 	break;
	// case 312:
	// 	output << string("Error 312: No se encontr� el fichero de los BITMAP del ") + "inventario." << NL;
	// 	break;
	// case 313:
	// 	output << "Error 313: No se encontr� el fichero de hipertexto." << NL;
	// 	break;
	// case 314:
	// 	output << string("Error 314: No se encontr� el fichero del arbol de ") + "conversaciones." << NL;
	// 	break;
	// case 315:
	// 	output << string("Error 315: No se encontr� el fichero de la pantalla a ") + "pintar." << NL;
	// 	break;
	// case 316:
	// 	output << string("Error 316: No se encontr� el fichero de la pantalla a") + " cargar." << NL;
	// 	break;
	// case 317:
	// 	output << string("Error 317: No se encontr� el fichero de las mascaras del") + " rat�n." << NL;
	// 	break;
	// case 318:
	// 	output << "Error 318: No se encontr� el fichero del diploma." << NL;
	// 	break;
	// case 319:
	// 	output << string("Error 319: No se encontr� el fichero de los objetos en") + " pantalla." << NL;
	// 	break;
	// case 320:
	// 	output << string("Error 320: No se encontr� el fichero de la rejilla de") + " pantalla." << NL;
	// 	break;
	// case 321:
	// 	output << "Error 321: No se encontr� el fichero del objeto rotando." << NL;
	// 	break;
	// case 322:
	// 	output << "Error 322: No se encontr� el fichero de la partida salvada." << NL;
	// 	break;
	// case 323:
	// 	output << "Error 323: No se encontr� el fichero de configuraci�n." << NL;
	// 	break;
	// default:
	// 	output << "Error " << exitcode << ": La parab�lica est� mal orientada." << NL;
	// }
	if (exitcode != 0) {
		// output << "Restaurando ficheros...." << NL;
		// initializeScreenFile();
		// // initializeObjectFile();
		// assign(kkfile, "GAME07.SAV");

		// if (ioresult == 0) {
		// 	close(kkfile);
		// 	erase(kkfile);
		// }
		// // movefile(diractual, diractual, "CONVERSA.TRE", "007");
		// output << "La partida temporal se ha eliminado." << NL;
	}
}

void resetGameState() {

	characterPosX = 160;
	characterPosY = 80;
	iframe = 0;
	trayec[0].x = characterPosX;
	trayec[0].y = characterPosY;
	xframe2 = 0;
	yframe2 = 1;
	zonaactual = 1;
	zonadestino = 1;
	oldzonadestino = 0;
	direccionmovimiento = 1;
	primera[0] = true;
	primera[1] = true;
	primera[2] = true;
	primera[3] = true;
	primera[4] = true;
	primera[5] = true;
	primera[6] = true;
	primera[7] = true;
	primera[8] = true;
	lprimera[0] = false;
	lprimera[1] = false;
	lprimera[2] = false;
	lprimera[3] = false;
	lprimera[4] = false;
	lprimera[5] = false;
	lprimera[6] = false;
	lprimera[7] = false;
	lprimera[8] = false;

	cprimera[0] = false;
	cprimera[1] = false;
	cprimera[2] = false;
	cprimera[3] = false;
	cprimera[4] = false;
	cprimera[5] = false;
	cprimera[6] = false;
	cprimera[7] = false;
	cprimera[8] = false;

	libro[0] = false;
	libro[1] = false;
	libro[2] = false;
	libro[3] = false;
	libro[4] = false;
	libro[5] = false;
	libro[6] = false;
	libro[7] = false;
	libro[8] = false;

	caramelos[0] = false;
	caramelos[1] = false;
	caramelos[2] = false;
	caramelos[3] = false;
	caramelos[4] = false;
	caramelos[5] = false;
	caramelos[6] = false;
	caramelos[7] = false;
	caramelos[8] = false;

	cavernas[0] = false;
	cavernas[1] = false;
	cavernas[2] = false;
	cavernas[3] = false;
	cavernas[4] = false;

	animacion2 = false;
	secuencia.profundidad = 0;
	volumenfxderecho = 6;
	volumenfxizquierdo = 6;
	volumenmelodiaderecho = 3;
	volumenmelodiaizquierdo = 3;

	pintaractivo = true;
	desactivagrabar = false;
	partidanueva = false;
	salirdeljuego = false;
	lista1 = false;
	lista2 = false;

	completadalista1 = false;
	completadalista2 = false;

	movidapaleta = 0;
	parte_del_juego = 1;

	vasijapuesta = false;
	guadagna = false;
	tridente = false;
	torno = false;
	barro = false;
	sello_quitado = false;

	diablillo_verde = false;
	rojo_capturado = false;
	alacena_abierta = false;
	baul_abierto = false;

	teleencendida = false;
	trampa_puesta = false;
	saltospal = 0;

	hornacina[0][0] = 563;
	hornacina[0][1] = 561;
	hornacina[0][2] = 0;
	hornacina[0][3] = 2;

	hornacina[1][0] = 615;
	hornacina[1][1] = 622;
	hornacina[1][2] = 623;
	hornacina[1][3] = 0;

	indicetray = 0;
	posicioninv = 0;
}

void initPlayAnim() {
	debug("initplayanim!");
	isLoadingFromLauncher = false;
	encriptado = "23313212133122121312132132312312122132322131221322222112121"
				 "32121121212112111212112333131232323213222132123211213221231"
				 "32132213232333333213132132132322113212132121322123121232332"
				 "23123221322213233221112312231221233232122332211112233122321"
				 "222312211322312223";

	rooms = nullptr;
	conversationData = nullptr;
	invItemData = nullptr;
	// encriptado[0]  = encripcod1;
    // encriptado[1]  = '\63';
    // encriptado[2]  = '\63';
    // encriptado[3]  = encripcod1 - 1;
    // encriptado[4]  = '\63';
    // encriptado[5]  = encripcod1;
    // encriptado[6]  = encripcod1 - 1;
    // encriptado[7]  = encripcod1;
    // encriptado[8]  = encripcod1 - 1;
    // encriptado[9]  = '\63';
    // encriptado[10] = '\63';
    // encriptado[11] = encripcod1 - 1;
    // encriptado[12] = encripcod1;
    // encriptado[13] = encripcod1;
    // encriptado[14] = encripcod1 - 1;
    // encriptado[15] = encripcod1;
    // encriptado[16] = encripcod1 - 1;
    // encriptado[17] = '\63';
    // encriptado[18] = encripcod1 - 1;
    // encriptado[19] = encripcod1;
    // encriptado[20] = encripcod1 - 1;
    // encriptado[21] = '\63';
    // encriptado[22] = encripcod1;
    // encriptado[23] = encripcod1 - 1;
    // encriptado[24] = '\63';
    // encriptado[25] = encripcod1;
    // encriptado[26] = '\63';
    // encriptado[27] = encripcod1 - 1;
    // encriptado[28] = encripcod1;
    // encriptado[29] = '\63';
    // encriptado[30] = encripcod1 - 1;
    // encriptado[31] = encripcod1;
    // encriptado[32] = encripcod1 - 1;
    // encriptado[33] = encripcod1;
    // encriptado[34] = encripcod1;
    // encriptado[35] = encripcod1 - 1;
    // encriptado[36] = '\63';
    // encriptado[37] = encripcod1;
    // encriptado[38] = '\63';
    // encriptado[39] = encripcod1;
    // encriptado[40] = encripcod1;
    // encriptado[41] = encripcod1 - 1;
    // encriptado[42] = '\63';
    // encriptado[43] = encripcod1 - 1;
    // encriptado[44] = encripcod1;
    // encriptado[45] = encripcod1;
    // encriptado[46] = encripcod1 - 1;
    // encriptado[47] = '\63';
    // encriptado[48] = encripcod1;
    // encriptado[49] = encripcod1;
    // encriptado[50] = encripcod1;
    // encriptado[51] = encripcod1;
    // encriptado[52] = encripcod1;
    // encriptado[53] = encripcod1 - 1;
    // encriptado[54] = encripcod1 - 1;
    // encriptado[55] = encripcod1;
    // encriptado[56] = encripcod1 - 1;
    // encriptado[57] = encripcod1;
    // encriptado[58] = encripcod1 - 1;
    // encriptado[59] = '\63';
    // encriptado[60] = encripcod1;
    // encriptado[61] = encripcod1 - 1;
    // encriptado[62] = encripcod1;
    // encriptado[63] = encripcod1 - 1;
    // encriptado[64] = encripcod1 - 1;
    // encriptado[65] = encripcod1;
    // encriptado[66] = encripcod1 - 1;
    // encriptado[67] = encripcod1;
    // encriptado[68] = encripcod1 - 1;
    // encriptado[69] = encripcod1;
    // encriptado[70] = encripcod1 - 1;
    // encriptado[71] = encripcod1 - 1;
    // encriptado[72] = encripcod1;
    // encriptado[73] = encripcod1 - 1;
    // encriptado[74] = encripcod1 - 1;
    // encriptado[75] = encripcod1 - 1;
    // encriptado[76] = encripcod1;
    // encriptado[77] = encripcod1 - 1;
    // encriptado[78] = encripcod1;
    // encriptado[79] = encripcod1 - 1;
    // encriptado[80] = encripcod1 - 1;
    // encriptado[81] = encripcod1;
    // encriptado[82] = '\63';
    // encriptado[83] = '\63';
    // encriptado[84] = '\63';
    // encriptado[85] = encripcod1 - 1;
    // encriptado[86] = '\63';
    // encriptado[87] = encripcod1 - 1;
    // encriptado[88] = encripcod1;
    // encriptado[89] = '\63';
    // encriptado[90] = encripcod1;
    // encriptado[91] = '\63';
    // encriptado[92] = encripcod1;
    // encriptado[93] = '\63';
    // encriptado[94] = encripcod1;
    // encriptado[95] = encripcod1 - 1;
    // encriptado[96] = '\63';
    // encriptado[97] = encripcod1;
    // encriptado[98] = encripcod1;
    // encriptado[99] = encripcod1;
    // encriptado[100] = encripcod1 - 1;
    // encriptado[101] = '\63';
    // encriptado[102] = encripcod1;
    // encriptado[103] = encripcod1 - 1;
    // encriptado[104] = encripcod1;
    // encriptado[105] = '\63';
    // encriptado[106] = encripcod1;
    // encriptado[107] = encripcod1 - 1;
    // encriptado[108] = encripcod1 - 1;
    // encriptado[109] = encripcod1;
    // encriptado[110] = encripcod1 - 1;
    // encriptado[111] = '\63';
    // encriptado[112] = encripcod1;
    // encriptado[113] = encripcod1;
    // encriptado[114] = encripcod1 - 1;
    // encriptado[115] = encripcod1;
    // encriptado[116] = '\63';
    // encriptado[117] = encripcod1 - 1;
    // encriptado[118] = '\63';
    // encriptado[119] = encripcod1;
    // encriptado[120] = encripcod1 - 1;
    // encriptado[121] = '\63';
    // encriptado[122] = encripcod1;
    // encriptado[123] = encripcod1;
    // encriptado[124] = encripcod1 - 1;
    // encriptado[125] = '\63';
    // encriptado[126] = encripcod1;
    // encriptado[127] = '\63';
    // encriptado[128] = encripcod1;
    // encriptado[129] = '\63';
    // encriptado[130] = '\63';
    // encriptado[131] = '\63';
    // encriptado[132] = '\63';
    // encriptado[133] = '\63';
    // encriptado[134] = '\63';
    // encriptado[135] = encripcod1;
    // encriptado[136] = encripcod1 - 1;
    // encriptado[137] = '\63';
    // encriptado[138] = encripcod1 - 1;
    // encriptado[139] = '\63';
    // encriptado[140] = encripcod1;
    // encriptado[141] = encripcod1 - 1;
    // encriptado[142] = '\63';
    // encriptado[143] = encripcod1;
    // encriptado[144] = encripcod1 - 1;
    // encriptado[145] = '\63';
    // encriptado[146] = encripcod1;
    // encriptado[147] = '\63';
    // encriptado[148] = encripcod1;
    // encriptado[149] = encripcod1;
    // encriptado[150] = encripcod1 - 1;
    // encriptado[151] = encripcod1 - 1;
    // encriptado[152] = '\63';
    // encriptado[153] = encripcod1;
    // encriptado[154] = encripcod1 - 1;
    // encriptado[155] = encripcod1;
    // encriptado[156] = encripcod1 - 1;
    // encriptado[157] = '\63';
    // encriptado[158] = encripcod1;
    // encriptado[159] = encripcod1 - 1;
    // encriptado[160] = encripcod1;
    // encriptado[161] = encripcod1 - 1;
    // encriptado[162] = '\63';
    // encriptado[163] = encripcod1;
    // encriptado[164] = encripcod1;
    // encriptado[165] = encripcod1 - 1;
    // encriptado[166] = encripcod1;
    // encriptado[167] = '\63';
    // encriptado[168] = encripcod1 - 1;
    // encriptado[169] = encripcod1;
    // encriptado[170] = encripcod1 - 1;
    // encriptado[171] = encripcod1;
    // encriptado[172] = '\63';
    // encriptado[173] = encripcod1;
    // encriptado[174] = '\63';
    // encriptado[175] = '\63';
    // encriptado[176] = encripcod1;
    // encriptado[177] = encripcod1;
    // encriptado[178] = '\63';
    // encriptado[179] = encripcod1 - 1;
    // encriptado[180] = encripcod1;
    // encriptado[181] = '\63';
    // encriptado[182] = encripcod1;
    // encriptado[183] = encripcod1;
    // encriptado[184] = encripcod1 - 1;
    // encriptado[185] = '\63';
    // encriptado[186] = encripcod1;
    // encriptado[187] = encripcod1;
    // encriptado[188] = encripcod1;
    // encriptado[189] = encripcod1 - 1;
    // encriptado[190] = '\63';
    // encriptado[191] = encripcod1;
    // encriptado[192] = '\63';
    // encriptado[193] = '\63';
    // encriptado[194] = encripcod1;
    // encriptado[195] = encripcod1;
    // encriptado[196] = encripcod1 - 1;
    // encriptado[197] = encripcod1 - 1;
    // encriptado[198] = encripcod1 - 1;
    // encriptado[199] = encripcod1;
    // encriptado[200] = '\63';
    // encriptado[201] = encripcod1 - 1;
    // encriptado[202] = encripcod1;
    // encriptado[203] = encripcod1;
    // encriptado[204] = '\63';
    // encriptado[205] = encripcod1 - 1;
    // encriptado[206] = encripcod1;
    // encriptado[207] = encripcod1;
    // encriptado[208] = encripcod1 - 1;
    // encriptado[209] = encripcod1;
    // encriptado[210] = '\63';
    // encriptado[211] = '\63';
    // encriptado[212] = encripcod1;
    // encriptado[213] = '\63';
    // encriptado[214] = encripcod1;
    // encriptado[215] = encripcod1 - 1;
    // encriptado[216] = encripcod1;
    // encriptado[217] = encripcod1;
    // encriptado[218] = '\63';
    // encriptado[219] = '\63';
    // encriptado[220] = encripcod1;
    // encriptado[221] = encripcod1;
    // encriptado[222] = encripcod1 - 1;
    // encriptado[223] = encripcod1 - 1;
    // encriptado[224] = encripcod1 - 1;
    // encriptado[225] = encripcod1 - 1;
    // encriptado[226] = encripcod1;
    // encriptado[227] = encripcod1;
    // encriptado[228] = '\63';
    // encriptado[229] = '\63';
    // encriptado[230] = encripcod1 - 1;
    // encriptado[231] = encripcod1;
    // encriptado[232] = encripcod1;
    // encriptado[233] = '\63';
    // encriptado[234] = encripcod1;
    // encriptado[235] = encripcod1 - 1;
    // encriptado[236] = encripcod1;
    // encriptado[237] = encripcod1;
    // encriptado[238] = encripcod1;
    // encriptado[239] = '\63';
    // encriptado[240] = encripcod1 - 1;
    // encriptado[241] = encripcod1;
    // encriptado[242] = encripcod1;
    // encriptado[243] = encripcod1 - 1;
    // encriptado[244] = encripcod1 - 1;
    // encriptado[245] = '\63';
    // encriptado[246] = encripcod1;
    // encriptado[247] = encripcod1;
    // encriptado[248] = '\63';
    // encriptado[249] = encripcod1 - 1;
    // encriptado[250] = encripcod1;
    // encriptado[251] = encripcod1;
    // encriptado[252] = encripcod1;
    // encriptado[253] = '\63';
	tocapintar = false;
	for (int i = 0; i < numobjetosconv; i++) {
		objetos[i] = NULL;
	}
	xraton = 160;
	yraton = 100;
	pulsax = xraton;
	pulsay = yraton;
	iraton = 1;

	resetGameState();
	firstList[0] = 222;
	firstList[1] = 295;
	firstList[2] = 402;
	firstList[3] = 223;
	firstList[4] = 521;
	secondList[0] = 221;
	secondList[1] = 423;
	secondList[2] = 308;
	secondList[3] = 362;
	secondList[4] = 537;
	contadorpc = 0;
	contadorpc2 = 0;
	continuarpartida = true;
	hechaprimeravez = false;
	dentro_del_juego = false;
}

} // End of namespace Tot
