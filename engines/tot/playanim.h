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
#ifndef TOT_PLAYANIM_H
#define TOT_PLAYANIM_H

#include "common/file.h"
#include "common/memstream.h"
#include "common/scummsys.h"

#include "tot/util.h"

namespace Tot {

const int verbRegSize = 263;
const int roomRegSize = 10856;
const int itemRegSize = 279;

// Enforces a small delay when text reveal is supposed to happen
const int enforcedTextAnimDelay = 0;

const int indicelista1 = 19;
const int indicelista2 = 20;
const int codigolista1 = 149;
const int codigolista2 = 150;
const int memoriaminima = 130000;
/**
 * Num of icons in the inventory
 */
const int inventoryIconCount = 34;
const int numobjetosconv = 15;
/**
 * Num of depth levels
 */
const int nivelesdeprof = 15;
/**
 * Num of frames of the main character in a single direction
 */
const int walkFrameCount = 16;
/**
 * Num of frames in the secondary animation
 */
const int secAnimationFrameCount = 50;
/**
 * Screen size
 */
const int tampan = 320 * 140;
/**
 * X factor of the screen grid
 * This results in 320/8 = 40 quadrants.
 */
const int factorx = 8;
/**
 * Y factor of the screen grid
 * This results in 140/5 = 28 quadrants.
 */
const int factory = 5;
/**
 * Adjustment of feet on frame 49
 */
const int rectificaciony = 49;
/**
 * Adjustment of feet on frame 28
 */
const int rectificacionx = 14;
const int longitudnombreobjeto = 20;
const int maxpersonajes = 9;
/**
 * Num of reserved colors at the end of the game's palette
 */
const int coloresreservados = 28;
/**
 * Size of an inventory icon (39x26)
 */
const int sizeicono = 1018;
const int tamcarahablando = 7206;
/**
 * Number of points in a
 */
const int routePointCount = 7;
/**
 * Trajectory changes
 */
typedef Common::Point route[routePointCount];

struct datosobj {
	uint16 codigo, posx, posy, posx2, posy2;
};

struct regsecuencia {
	uint16 profundidad;
	byte *bitmap[4][walkFrameCount + 30]; // 30 = 3 actions * 10 frames each
};

struct reganimado {
	uint16 profundidad, dir, posx, posy;
	byte *dib[4][secAnimationFrameCount];
};

struct reginventario {
	uint16 bitmapIndex;
	uint16 code;
	Common::String objectName;
};

struct inventoryBitmaps {
	byte *bitmap[inventoryIconCount];
};

/**
 * Hypertext struct
 */
struct regismht {
	Common::String cadenatext; // string
	boolean encadenado;        // true if the next entry is a continuation of this one
	uint16 respuesta;          // entry number of reply
	int32 punteronil;
};

struct InvItemRegister {
	/**
	 * registry number
	 */
	uint16 code;
	byte altura;             /* 0 top 1 middle 2 bottom*/
	Common::String name;     /*name for mouseover*/
	uint16 lookAtTextRef;    /* Registro al mirar el objeto en pantalla */
	uint16 beforeUseTextRef; /* Registro antes de usar el objeto en mochila */
	uint16 afterUseTextRef;  /* Registro despues de usar el objeto en mochila */
	uint16 pickTextRef;      /* Registro al coger el objeto */
	uint16 useTextRef;       /* Registro al usar el objeto */
	byte habla;              /* number of character to respond */
	boolean abrir;           /* true if it can be opened */
	boolean cerrar;          /* true if it can be closed*/
	byte usar[8];
	boolean coger;
	uint16 usarcon,
		reemplazarpor; /*Numero del objeto por el que se reemplaza en el
			   caso de que se utilize con un objeto de la mochila
			   y haya que cambiarlo por otro.*/
	byte profundidad;
	uint32 punterobitmap; /*Puntero al fichero de bitmap para el
						 objeto que se suelta en la pantalla*/
	uint16 tambitmap;
	uint32 punteroframesgiro; /*Puntero al fichero de
							bitmap para el objeto girando*/
	uint16 punteropaletagiro, /*Puntero al fichero de paletas para
					  la imagen girando*/
		xparche, yparche;
	uint32 puntparche;
	uint16 tamparche, objectIconBitmap;
	byte xrej1, yrej1, xrej2, yrej2;

	byte parcherejapantalla[10][10];
	byte parcherejaraton[10][10];
};

struct DoorRegistry {
	uint16 pantallaquecarga,
		posxsalida, posysalida; /*Coordenadas x e y de salida del
						 personaje en la siguiente pantalla*/
	byte abiertacerrada, codigopuerta;
};

struct RoomObjectListEntry {
	uint16 indicefichero;
	Common::String objectName;
};

struct RoomBitmapRegister {
	int32 puntbitmap;
	uint16 tambitmap;
	uint16 coordx, coordy, profund;
};

struct RoomFileRegister {
	uint16 codigo;
	int32 puntimagenpantalla;
	uint16 tamimagenpantalla;
	byte rejapantalla[40][28]; /* movement grid */
	byte mouseGrid[40][28];    /* mousegrid with index to indexadoObjetos */
	/**
	 * This is a preset matrix of trajectories from different areas of the game.action
	 * Each room is divided into a number of areas according to the screen grid [rejapantalla].action
	 *
	 * A given coordinate in the game (x,y) is mapped into a grid position by dividing the x with factorx (8)
	 * and the y coordinate by factory (5). With each room being 320x140 this results in
	 * 40 horizontal divisions and 28 vertical divisions which matches the [rejapantalla] matrix.action
	 *
	 * When obtaining the grid position a given (x,y) coordinate matches to, [rejapantalla] returns an
	 * area number.action
	 *
	 * trayectorias then has a precalculated route from each possible combination of two areas in the game.
	 * pixel by pixel translation within the area is done by bresenham algorithm in the trajectory function.
	 */
	Common::Point trajectories[9][30][5];
	DoorRegistry doors[5]; /* doors in the room */
	RoomBitmapRegister bitmapasociados[15];
	RoomObjectListEntry *indexadoobjetos[51] = {NULL}; /* includes name of objects for mouseover + index to object file*/
	boolean animationFlag;                             /* true if there is a secondary animation */
	Common::String nombremovto;                        /* name of the secondary animation, 8 chars*/
	boolean paletteAnimationFlag;                      /* true if there exist palette animation */
	uint16 puntpaleta;                                 /* points to the screen palette */
	Common::Point tray2[300];                          /* trajectory of the secondary animation */
	uint16 dir2[300];                                  /* directions of the secondary trajectory. Pos 300 reflects object code. */
	uint16 longtray2;                                  /* longitude of the trajectory of the secondary animation */
};

struct regispartida {
	uint numeropantalla,
		longtray,
		indicetray,
		codigoobjmochila,
		volumenfxderecho,
		volumenfxizquierdo,
		volumenmelodiaderecho,
		volumenmelodiaizquierdo,
		oldxrejilla,
		oldyrejilla,
		animadoprofundidad,
		animadodir,
		animadoposx,
		animadoposy,
		animadoiframe2;
	byte zonaactual,
		zonadestino,
		oldzonadestino,
		posicioninv,
		numeroaccion,
		oldnumeroacc,
		pasos,
		indicepuertas,
		direccionmovimiento,
		iframe,
		parte_del_juego;
	boolean sello_quitado,
		manual_torno,
		lista1,
		lista2,
		completadalista1,
		completadalista2,
		vasijapuesta,
		guadagna,
		tridente,
		torno,
		barro,
		diablillo_verde,
		alacena_abierta,
		baul_abierto,
		teleencendida,
		trampa_puesta,
		rojo_capturado;
	reginventario mobj[inventoryIconCount];
	int elemento1,
		elemento2,
		xframe,
		yframe,
		xframe2,
		yframe2;
	Common::String oldobjmochila,
		objetomochila;
	Common::String nombrepersonaje;
	route mainRoute;
	uint16 firstList[5], secondList[5];
	Common::Point trayec[300];
	boolean primera[maxpersonajes],
		lprimera[maxpersonajes],
		cprimera[maxpersonajes],
		libro[maxpersonajes],
		caramelos[maxpersonajes];

	boolean cavernas[5];
	uint hornacina[2][4];
};

typedef byte palette[768];

extern Common::MemorySeekableReadWriteStream *conversationData;
extern Common::MemorySeekableReadWriteStream *rooms;
extern Common::MemorySeekableReadWriteStream *invItemData;
/**
 * Frame index of the mouse mask
 */
extern byte iraton;
/**
 * Coords of the mouse sprite
 */
extern uint xraton, yraton;
/**
 * Coords of mouse clicks
 */
extern uint pulsax, pulsay;
/**
 * Mouse clicks for both buttons
 */
extern uint npraton2, npraton;

/**
 * Previous positions of the mouse within the screen grid
 */
extern uint oldxrejilla, oldyrejilla;

extern regispartida regpartida;

extern boolean sello_quitado;

/**
 * Flag to enable screen/room change
 */
extern boolean cambiopantalla;
extern boolean teleencendida,
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

/**
 * Flag for temporary savegame
 */
extern boolean inGame;
/**
 * Flag for first time run of the game.
 */
extern boolean hechaprimeravez;
/**
 * Flag to exit program.
 */
extern boolean salirdeljuego;
/**
 * Flag to initialize game
 */
extern boolean partidanueva;
/**
 * Flag to resume game
 */
extern boolean continuarpartida;
/**
 * Flag to load a game upon start.
 */
extern boolean desactivagrabar;
/**
 * true if sprites should be drawn
 */
extern boolean pintaractivo;
/**
 * Flag for secondary animation
 */
extern boolean animacion2;
/**
 * 54 color palette slice.
 */
extern palette movimientopal;
/**
 * General palette
 */
extern palette pal;

/**
 * These are the icons currnetly in the inventory
 */
extern reginventario mobj[inventoryIconCount];
/**
 * Keeps an array of all inventory icons
 */
extern inventoryBitmaps mochilaxms;

/**
 * Delay of palette animation
 */
extern byte saltospal;
/**
 * Position within inventory
 */
extern byte posicioninv;
/**
 * Currently selected action.
 */
extern byte numeroaccion;
/**
 * Previously selected action.
 */
extern byte oldnumeroacc;
/**
 * Number of trajectory changes
 */
extern byte pasos;
/**
 * index of currently selected door.
 */
extern byte indicepuertas;
/**
 * Aux for palette animation
 */
extern byte movidapaleta;
/**
 * Index of patch for grid within XMS
 */
extern byte rejillaquetoca;
/**
 * 1 first part, 2 second part
 */
extern byte gamePart;
extern byte encripcod1;
/**
 * Number of frames of secondary animation
 */
extern byte secondaryAnimationFrameCount;
/**
 * Number of directions of the secondary animation
 */
extern byte numerodir;
/**
 * Data protection control
 */
extern byte contadorpc, contadorpc2;
/**
 * Auxiliary indices
 */
extern byte indaux1, indaux2;
/**
 * Coordinates of target step
 */
extern byte destinox_paso, destinoy_paso;
/**
 * Current character facing direction
 * 0: upwards
 * 1: right
 * 2: downwards
 * 3: left
 */
extern byte direccionmovimiento;

/**
 * Width and height of secondary animation
 */
extern uint anchoanimado, altoanimado;
/**
 * Time window between candle decrease
 */
extern uint tiempo;
/**
 * Code of selected object in the backpack
 */
extern uint codigoobjmochila;
/**
 * Foo
 */
extern uint kaka;
/**
 * Auxiliary vars for grid update
 */
extern uint oldposx, oldposy;
extern uint volumenfxderecho, volumenfxizquierdo;
extern uint segpasoicono;
extern uint ofspasoicono;
extern uint volumenmelodiaderecho, volumenmelodiaizquierdo;

/**
 * Amplitude of movement
 */
extern int elemento1, elemento2;
/**
 * Current position of the main character
 */
extern int characterPosX, characterPosY;
/**
 * Target position of the main character?
 */
extern int xframe2, yframe2;
/**
 * Text map
 */
extern Common::File verb;
/**
 * Auxiliary vars with current inventory object name.
 */
extern Common::String oldobjmochila, objetomochila;

extern Common::String nombreficherofoto;
/**
 * Name of player
 */
extern Common::String nombrepersonaje;

extern Common::String decryptionKey;

extern uint hornacina[2][4];

extern RoomFileRegister *currentRoomData;

extern InvItemRegister regobj;
/**
 * New movement to execute.
 */
extern route mainRoute;
/**
 * Matrix of positions for a trajectory between two points
 */
extern Common::Point trayec[300];

/**
 * Longitude of the trajectory matrix.
 */
extern uint longtray;
/**
 * Position within the trajectory matrix
 */
extern uint indicetray;
/**
 * Position within the trajectory matrix for secondary animation
 */
extern uint indicetray2;
/**
 * Screen areas
 */
extern byte zonaactual, zonadestino, oldzonadestino;
/**
 * Amplitude of grid slices
 */
extern byte maxrejax, maxrejay;

/**
 * capture of movement grid of secondary animation
 */
extern byte rejafondomovto[10][10];
/**
 * capture of mouse grid of secondary animation
 */
extern byte rejafondoraton[10][10];
/**
 * movement mask for grid of secondary animation
 */
extern byte rejamascaramovto[10][10];

/**
 * mouse mask for grid of secondary animation
 */
extern byte rejamascararaton[10][10];

extern boolean completadalista1,
	completadalista2,
	lista1, // whether we've been given list 1
	lista2; // whether we've been given list 2

extern boolean primera[maxpersonajes],
	lprimera[maxpersonajes],
	cprimera[maxpersonajes],
	libro[maxpersonajes],
	caramelos[maxpersonajes];

extern boolean cavernas[5];
/**
 * First and second lists of objects to retrieve in the game
 */
extern uint16 firstList[5],
	secondList[5];
/**
 * Animation sequence
 */
extern regsecuencia secuencia;
extern reganimado animado;
extern uint sizeframe,
	segpasoframe,
	ofspasoframe,
	sizeanimado,
	segpasoanimado,
	ofspasoanimado,
	segfondo,
	offfondo;
/**
 * Flags for animations
 */
// extern boolean tocapintar, tocapintar2;
/**
 * Max num of loaded frames for secondary animation
 */
extern byte fotogramamax2;
/**
 * Index of fade effect for room change
 */
extern byte tipoefectofundido;
/**
 * Frame number for the animations
 */
extern byte iframe, iframe2;

extern long sizepantalla;

/**
 * Depth of screenobjects
 */
extern datosobj depthMap[numobjetosconv];
/**
 * Bitmaps of screenobjects
 */
extern byte *screenObjects[numobjetosconv];
/**
 * Current frame of main character
 */
extern byte *pasoframe;
/**
 * Current frame of secondary animation
 */
extern byte *pasoanimado;

/**
 * Pointer storing the screen as it displays on the game
 */
extern byte *background;

/**
 * Dirty patch of screen to repaint on every frame
 */
extern byte *characterDirtyRect;
/**
 * Stores a copy of the background bitmap
 */
extern byte *handpantalla;

extern uint currentRoomNumber;

extern boolean isLoadingFromLauncher;

void initializeScreenFile();
void clearObj();
void clearScreenData();
void initPlayAnim();
void resetGameState();

} // End of namespace Tot
#endif
