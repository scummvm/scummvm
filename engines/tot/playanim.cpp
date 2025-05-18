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

bool sello_quitado;

bool cambiopantalla;
bool teleencendida,
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

bool inGame;

bool hechaprimeravez;

bool salirdeljuego;

bool partidanueva;

bool continuarpartida;

bool desactivagrabar;

bool pintaractivo;

bool animacion2;

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

byte gamePart;
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

// Text decryption key
Common::String decryptionKey;

uint hornacina[2][4];

RoomFileRegister *currentRoomData;

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

bool completadalista1, completadalista2,
	lista1, lista2;

bool primera[maxpersonajes],
	lprimera[maxpersonajes],
	cprimera[maxpersonajes],
	libro[maxpersonajes],
	caramelos[maxpersonajes];

bool cavernas[5];

uint16 firstList[5], secondList[5];

regsecuencia secuencia;
reganimado animado;
uint sizeframe,
	segpasoframe,
	ofspasoframe,
	sizeanimado,
	segpasoanimado,
	ofspasoanimado,
	segfondo,
	offfondo;

byte fotogramamax2;

byte tipoefectofundido;

byte iframe, iframe2;

long sizepantalla;

datosobj depthMap[numobjetosconv];

byte *screenObjects[numobjetosconv];

byte *pasoframe;

byte *pasoanimado;

byte *background;

byte *characterDirtyRect;

byte *handpantalla;

uint currentRoomNumber;

bool isLoadingFromLauncher;

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
	// Do nothing
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
		showError(320);
	}

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
	gamePart = 1;

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
	decryptionKey = "23313212133122121312132132312312122132322131221322222112121"
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
		screenObjects[i] = NULL;
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
	inGame = false;
}

} // End of namespace Tot
