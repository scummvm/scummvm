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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "tot/offsets.h"
#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/texts.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void loadScreenMemory() {
	sizepantalla = 65520;
	background = (byte *)malloc(sizepantalla);
	handpantalla = (byte *)malloc(sizepantalla);
}

void loadAnimationForDirection(Common::SeekableReadStream *stream, int direction) {
	for (int j = 0; j < secondaryAnimationFrameCount; j++) {
		loadAnimationIntoBuffer(stream, animado.dib[direction][j], sizeanimado);
	}
}

void loadAnimation(Common::String animacion) {
	debug("Loading animation!");
	Common::File fichcani;

	if (animacion == "PETER")
		peteractivo = true;
	else
		peteractivo = false;

	animacion2 = true;
	if (!fichcani.open(Common::Path(animacion + ".DAT"))) {
		showError(265);
	}

	sizeanimado = fichcani.readUint16LE();
	secondaryAnimationFrameCount = fichcani.readByte();
	numerodir = fichcani.readByte();
	pasoanimado = (byte *)malloc(sizeanimado);
	if (numerodir != 0) {

		secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&fichcani, i);
		}
	} else {
		loadAnimationForDirection(&fichcani, 0);
	}

	fichcani.close();
	debug("Read all frames! longtray2=%d", currentRoomData->longtray2);
	anchoanimado = READ_LE_UINT16(animado.dib[0][1]) + 1;
	altoanimado = READ_LE_UINT16(animado.dib[0][1] + 2) + 1;

	setRoomTrajectories(altoanimado, anchoanimado, SET_WITH_ANIM, false);

	readItemRegister(currentRoomData->dir2[299]);
	maxrejax = (regobj.xrej2 - regobj.xrej1 + 1);
	maxrejay = (regobj.yrej2 - regobj.yrej1 + 1);
	oldposx = regobj.xrej1 + 1;
	oldposy = regobj.yrej1 + 1;

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++) {
			rejamascaramovto[i][j] = 0;
			rejamascararaton[i][j] = 0;
			rejafondomovto[i][j] = 0;
			rejafondoraton[i][j] = 0;
		}

	for (int i = 0; i < maxrejax; i++)
		for (int j = 0; j < maxrejay; j++) {
			rejamascaramovto[i][j] = regobj.parcherejapantalla[i][j];
			rejamascararaton[i][j] = regobj.parcherejaraton[i][j];
			rejafondomovto[i][j] = currentRoomData->rejapantalla[oldposx + i][oldposy + j];
			rejafondoraton[i][j] = currentRoomData->mouseGrid[oldposx + i][oldposy + j];
		}
	iframe2 = 0;
	debug("Finished loading animation!");
}

void assignText() {
	if (!verb.open("CONVERSA.TXT")) {
		showError(313);
	}
}

void updateAltScreen(byte otherScreenNumber) {
	uint i22;
	uint i11;

	byte currentScreen = currentRoomData->codigo;

	setRoomTrajectories(altoanimado, anchoanimado, RESTORE);

	// Save current room
	saveRoomData(currentRoomData, rooms);

	// Load other screen
	rooms->seek(otherScreenNumber * roomRegSize, SEEK_SET);
	currentRoomData = readScreenDataFile(rooms);

	switch (otherScreenNumber) {
	case 20: {
		switch (hornacina[0][hornacina[0][3]]) {
		case 0: {
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(4);
			currentRoomData->bitmapasociados[1].puntbitmap = 1190768;
		} break;
		case 561: {
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(5);
			currentRoomData->bitmapasociados[1].puntbitmap = 1182652;
		} break;
		case 563: {
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(6);
			currentRoomData->bitmapasociados[1].puntbitmap = 1186044;
		} break;
		case 615: {
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(7);
			currentRoomData->bitmapasociados[1].puntbitmap = 1181760;
		} break;
		}
		currentRoomData->bitmapasociados[1].tambitmap = 892;
		currentRoomData->bitmapasociados[1].coordx = 66;
		currentRoomData->bitmapasociados[1].coordy = 35;
		currentRoomData->bitmapasociados[1].profund = 1;
	} break;
	case 24: {
		switch (hornacina[1][hornacina[1][3]]) {
		case 0: {
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(4);
			currentRoomData->bitmapasociados[0].puntbitmap = 1399610;
		} break;
		case 561: {
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(5);
			currentRoomData->bitmapasociados[0].puntbitmap = 1381982;
		} break;
		case 615: {
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(7);
			currentRoomData->bitmapasociados[0].puntbitmap = 1381090;
		} break;
		case 622: {
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(8);
			currentRoomData->bitmapasociados[0].puntbitmap = 1400502;
		} break;
		case 623: {
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(9);
			currentRoomData->bitmapasociados[0].puntbitmap = 1398718;
		} break;
		}
		currentRoomData->bitmapasociados[0].tambitmap = 892;
		currentRoomData->bitmapasociados[0].coordx = 217;
		currentRoomData->bitmapasociados[0].coordy = 48;
		currentRoomData->bitmapasociados[0].profund = 1;
	} break;
	case 31: {
		for (i11 = 23; i11 <= 25; i11++)
			for (i22 = 4; i22 <= 9; i22++)
				currentRoomData->mouseGrid[i11][i22] = 4;
		for (i11 = 23; i11 <= 25; i11++)
			for (i22 = 10; i22 <= 11; i22++)
				currentRoomData->mouseGrid[i11][i22] = 3;

		currentRoomData->bitmapasociados[0].tambitmap = 0;
		currentRoomData->bitmapasociados[0].puntbitmap = 0;
		currentRoomData->bitmapasociados[0].coordx = 0;
		currentRoomData->bitmapasociados[0].coordy = 0;
		currentRoomData->bitmapasociados[0].profund = 0;
	} break;
	}

	// Save other screen
	saveRoomData(currentRoomData, rooms);

	// Restore current room again
	rooms->seek(currentScreen * roomRegSize, SEEK_SET);
	currentRoomData = readScreenDataFile(rooms);

	setRoomTrajectories(altoanimado, anchoanimado, SET_WITH_ANIM);
}

struct regzxc {
	byte cabecera[258];
	char cadena[8];
	byte relleno[327];
	byte masrelleno[5268];
};

void verifyCopyProtection() {
	// regzxc regiszxc;
	// varying_string<8> rombioszxc, cadenaauxzxc, cadenarefzxc;
	// byte indicezxc;

	// Common::File fichzxc;
	// if(!fichzxc.open("MCGA.TWK")){
	// 	error("verifyCopyProtection(): ioresult (273)");
	// }

	// fichzxc.read(regiszxc.cabecera, 258);
	// fichzxc.read(regiszxc.cadena, 8);
	// fichzxc.read(regiszxc.relleno, 327);
	// fichzxc.read(regiszxc.masrelleno, 5268);

	// fichzxc.close();
	// rombioszxc = "";
	// rombioszxc[0] = '\0';
	// cadenaauxzxc[0] = '\0';
	// for (indicezxc = 0; indicezxc <= 7; indicezxc ++)
	// 	rombioszxc = rombioszxc + chr(mem[0xf000 * 1337 + (0xfff5+indicezxc)]);
	// cadenaauxzxc = regiszxc.cadena;
	// for (indicezxc = 1; indicezxc <= 8; indicezxc ++)
	// 	cadenaauxzxc[indicezxc] = chr((ord(regiszxc.cadena[indicezxc])
	// 	                               ^ ord(rombioszxc[indicezxc])));
	// indicezxc = 17 * 10;
	// cadenarefzxc = string(chr(indicezxc + 10)) + chr(indicezxc + 11) + chr(indicezxc + 12)
	//                + chr(indicezxc + 13) + chr(indicezxc + 14) + chr(indicezxc + 15)
	//                + chr(indicezxc + 16) + chr(indicezxc + 17);
	// cadenarefzxc[0] = '\10';
	// if (cadenaauxzxc != cadenarefzxc) {
	// 	contadorpc += 1;
	// 	contadorpc2 += 1;
	// }
}

void cargatele() {

	Common::File fichct;
	if (!fichct.open("PALETAS.DAT")) {
		showError(310);
	}
	fichct.seek(currentRoomData->puntpaleta + 603);
	fichct.read(movimientopal, 144);
	for (int i = 0; i <= 48; i++) {
		movimientopal[i * 3 + 0] = movimientopal[i * 3 + 0] << 2;
		movimientopal[i * 3 + 1] = movimientopal[i * 3 + 1] << 2;
		movimientopal[i * 3 + 2] = movimientopal[i * 3 + 2] << 2;
	}

	fichct.close();
	for (int ix = 195; ix <= 200; ix++) {
		pal[ix * 3 + 0] = 2 << 2;
		pal[ix * 3 + 1] = 2 << 2;
		pal[ix * 3 + 2] = 2 << 2;
		setRGBPalette(ix, 2, 2, 2);
	}
}

void loadScreen() {
	Common::File fichcp;
	palette palcp;

	sizepantalla = currentRoomData->tamimagenpantalla;
	readBitmap(currentRoomData->puntimagenpantalla, background, sizepantalla, 316);
	Common::copy(background, background + sizepantalla, handpantalla);
	switch (gamePart) {
	case 1: {
		if (!fichcp.open("PALETAS.DAT")) {
			showError(310);
		}
		fichcp.seek(currentRoomData->puntpaleta);
		fichcp.read(palcp, 603);
		if (currentRoomData->paletteAnimationFlag) {
			fichcp.read(movimientopal, 144);
			for (int i = 0; i <= 48; i++) {
				movimientopal[i * 3 + 0] = movimientopal[i * 3 + 0] << 2;
				movimientopal[i * 3 + 1] = movimientopal[i * 3 + 1] << 2;
				movimientopal[i * 3 + 2] = movimientopal[i * 3 + 2] << 2;
			}
		}
		fichcp.close();
		for (int i = 1; i <= 200; i++) {
			pal[i * 3 + 0] = palcp[i * 3 + 0] << 2;
			pal[i * 3 + 1] = palcp[i * 3 + 1] << 2;
			pal[i * 3 + 2] = palcp[i * 3 + 2] << 2;
		}
		g_system->getPaletteManager()->setPalette(pal, 0, 201);
	} break;
	case 2: {
		loadPalette("SEGUNDA");
		currentRoomData->paletteAnimationFlag = true;
	} break;
	}
}

void loadCharAnimation() {
	Common::File characterFile;
	contadorpc = contadorpc2;
	if (!characterFile.open("PERSONAJ.SPT"))
		showError(265);

	sizeframe = characterFile.readUint16LE();

	verifyCopyProtection();

	for (int i = 0; i <= 3; i++)
		for (int j = 0; j < walkFrameCount; j++) {
			secuencia.bitmap[i][j] = (byte *)malloc(sizeframe);
			characterFile.read(secuencia.bitmap[i][j], sizeframe);
		}
	for (int i = 0; i < 4; i++)
		for (int j = walkFrameCount; j < (walkFrameCount + 10 * 3); j++) {
			secuencia.bitmap[i][j] = (byte *)malloc(sizeframe);
			characterFile.read(secuencia.bitmap[i][j], sizeframe);
		}
	characterFile.close();
}

void freeScreenObjects() {
	uint indicecarga;

	for (indicecarga = 0; indicecarga < numobjetosconv; indicecarga++) {
		if (screenObjects[indicecarga] != NULL)
			free(screenObjects[indicecarga]);
		screenObjects[indicecarga] = NULL;
	}

	// if (liberadormem != nil) {
	// release(liberadormem);
	// liberadormem = nil;
	// }
	// verifyCopyProtection2();
}

void freeAnimation() {
	if (animacion2) {
		animacion2 = false;
		free(pasoanimado);
		pasoanimado = NULL;
	}
}

void freeInventory() {
	for (int i = 0; i < inventoryIconCount; i++) {
		free(mochilaxms[i]);
	}
}

// struct regas {
// 	array<1, 258, byte> cabecera;
// 	varying_string<8> cadena;
// 	array<1, 327, byte> relleno;
// 	array<1, 5268, byte> masrelleno;
// };

void verifyCopyProtection2() {
	// file<regas> fichas;
	// regas regisas;
	// varying_string<8> rombiosas, cadenaauxas, cadenarefas;
	// byte indiceas;

	// rombiosas = "";
	// rombiosas[0] = '\0';
	// reset(fichas);
	// if (ioresult != 0)
	// 	error("verifyCopyProtection2(): ioresult (273)");
	// fichas >> regisas;
	// close(fichas);
	// cadenaauxas[0] = '\0';
	// cadenaauxas = regisas.cadena;
	// for (indiceas = 0; indiceas <= 7; indiceas++)
	// 	rombiosas = rombiosas + chr(mem[0xf000 * 1337 + (0xfff5 + indiceas)]);
	// for (indiceas = 1; indiceas <= 8; indiceas++)
	// 	cadenaauxas[indiceas] = chr((ord(regisas.cadena[indiceas]) ^ ord(rombiosas[indiceas])));
	// indiceas = 18 * 10;
	// cadenarefas = string(chr(indiceas)) + chr(indiceas + 1) + chr(indiceas + 2) + chr(indiceas + 3) +
	// 			  chr(indiceas + 4) + chr(indiceas + 5) + chr(indiceas + 6) + chr(indiceas + 7);
	// cadenarefas[0] = '\10';
	// if (cadenaauxas != cadenarefas) {
	// 	contadorpc2 += 1;
	// 	contadorpc += 1;
	// }
}
void loadItemWithFixedDepth(uint coordx, uint coordy, uint tamdibujo, int32 dibujo, uint prof) {
	screenObjects[prof] = (byte *)malloc(tamdibujo);
	readBitmap(dibujo, screenObjects[prof], tamdibujo, 319);

	uint16 w, h;
	w = READ_LE_UINT16(screenObjects[prof]);
	h = READ_LE_UINT16(screenObjects[prof] + 2);
	depthMap[prof].posx = coordx;
	depthMap[prof].posy = coordy;
	depthMap[prof].posx2 = coordx + w + 1;
	depthMap[prof].posy2 = coordy + h + 1;
}

void loadItem(uint coordx, uint coordy, uint tamdibujo, int32 dibujo, uint prof) {
	loadItemWithFixedDepth(coordx, coordy, tamdibujo, dibujo, prof - 1);
}

void updateInventory(byte indicador) {
	for (int ind_mo = indicador; ind_mo < (inventoryIconCount - 1); ind_mo++) {
		mobj[ind_mo].bitmapIndex = mobj[ind_mo + 1].bitmapIndex;
		mobj[ind_mo].code = mobj[ind_mo + 1].code;
		mobj[ind_mo].objectName = mobj[ind_mo + 1].objectName;
	}
	// verifyCopyProtection();
}

void readBitmap(int32 posbm, byte *puntbm, uint tambm, uint errorbm) {
	Common::File fichbitmap;
	if (!fichbitmap.open("BITMAPS.DAT")) {
		showError(errorbm);
	}
	fichbitmap.seek(posbm);
	fichbitmap.read(puntbm, tambm);

	fichbitmap.close();
}

void updateItem(uint itemPosition) {
	regobj.usar[0] = 9;
	invItemData->seek(itemPosition);
	saveItemRegister(regobj, invItemData);
}

void readItemRegister(Common::SeekableReadStream *stream, uint itemPos, InvItemRegister &thisRegObj) {
	stream->seek(itemPos * itemRegSize);
	clearObj();
	thisRegObj.code = stream->readUint16LE();
	thisRegObj.altura = stream->readByte();

	thisRegObj.name = stream->readPascalString();

	stream->skip(longitudnombreobjeto - thisRegObj.name.size());

	thisRegObj.lookAtTextRef = stream->readUint16LE();
	thisRegObj.beforeUseTextRef = stream->readUint16LE();
	thisRegObj.afterUseTextRef = stream->readUint16LE();
	thisRegObj.pickTextRef = stream->readUint16LE();
	thisRegObj.useTextRef = stream->readUint16LE();
	thisRegObj.habla = stream->readByte();
	thisRegObj.abrir = stream->readByte();
	thisRegObj.cerrar = stream->readByte();

	stream->read(thisRegObj.usar, 8);

	thisRegObj.coger = stream->readByte();
	thisRegObj.usarcon = stream->readUint16LE();
	thisRegObj.reemplazarpor = stream->readUint16LE();
	thisRegObj.profundidad = stream->readByte();
	thisRegObj.punterobitmap = stream->readUint32LE();
	thisRegObj.tambitmap = stream->readUint16LE();
	thisRegObj.punteroframesgiro = stream->readUint32LE();
	thisRegObj.punteropaletagiro = stream->readUint16LE();
	thisRegObj.xparche = stream->readUint16LE();
	thisRegObj.yparche = stream->readUint16LE();
	thisRegObj.puntparche = stream->readUint32LE();
	thisRegObj.tamparche = stream->readUint16LE();
	thisRegObj.objectIconBitmap = stream->readUint16LE();
	thisRegObj.xrej1 = stream->readByte();
	thisRegObj.yrej1 = stream->readByte();
	thisRegObj.xrej2 = stream->readByte();
	thisRegObj.yrej2 = stream->readByte();
	stream->read(thisRegObj.parcherejapantalla, 100);
	stream->read(thisRegObj.parcherejaraton, 100);
}

void readItemRegister(uint itemPosition) {
	readItemRegister(invItemData, itemPosition, regobj);
}

void drawLookAtItem(RoomObjectListEntry obj) {
	g_engine->_mouseManager->hide();
	bar(0, 140, 319, 149, 0);
	actionLineText(getActionLineText(3) + obj.objectName);
	g_engine->_mouseManager->show();
}

void putIcon(uint posiconx, uint posicony, uint numicon) {
	// substract 1 to account for 1-based indices
	putImg(posiconx, posicony, mochilaxms[mobj[numicon].bitmapIndex - 1]);
}

void drawBackpack() {
	putIcon(34, 169, posicioninv);
	putIcon(77, 169, posicioninv + 1);
	putIcon(120, 169, posicioninv + 2);
	putIcon(163, 169, posicioninv + 3);
	putIcon(206, 169, posicioninv + 4);
	putIcon(249, 169, posicioninv + 5);
}

void lightUpLeft() {
	line(10, 173, 29, 173, 255);
	line(10, 173, 10, 189, 255);
	line(30, 174, 30, 190, 249);
	line(30, 190, 11, 190, 249);
}

void turnOffLeft() {
	line(10, 173, 29, 173, 249);
	line(10, 173, 10, 189, 249);
	line(30, 174, 30, 190, 255);
	line(30, 190, 11, 190, 255);
}

void lightUpRight() {
	line(291, 173, 310, 173, 255);
	line(291, 173, 291, 189, 255);
	line(311, 174, 311, 190, 249);
	line(311, 190, 292, 190, 249);
}

void turnOffRight() {
	line(291, 173, 310, 173, 249);
	line(291, 173, 291, 189, 249);
	line(311, 174, 311, 190, 255);
	line(311, 190, 292, 190, 255);
}

void inventory(byte direccion, byte tope) {
	switch (direccion) {
	case 0:
		if (posicioninv > 0) {
			posicioninv -= 1;
			drawBackpack();
		}
		break;
	case 1:
		if (posicioninv < (tope - 6)) {
			posicioninv += 1;
			drawBackpack();
		}
		break;
	}
	g_engine->_mouseManager->hide();
	if (posicioninv > 0)
		lightUpLeft();
	else
		turnOffLeft();
	if (mobj[posicioninv + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
	g_engine->_mouseManager->show();
	if (contadorpc > 145)
		showError(274);
}

void mask() {
	byte ytextaux;
	buttonBorder(0, 140, 319, 149, 0, 0, 0, 0, 0, 0, "");
	for (ytextaux = 1; ytextaux <= 25; ytextaux++)
		buttonBorder(0, (175 - ytextaux), 319, (174 + ytextaux), 251, 251, 251, 251, 0, 0, "");
	drawMenu(1);
	// verifyCopyProtection();
	if (posicioninv > 1)
		lightUpLeft();
	else
		turnOffLeft();
	if (mobj[posicioninv + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
}

void drawMenu(byte nummenu) {
	byte *punteromenu;
	uint tampunteromenu;
	byte xmenu, ymenu;
	long posicionmenu;

	Common::File menuFile;
	if (!menuFile.open("MENUS.DAT")) {
		showError(258);
	}

	posicionmenu = g_engine->_lang == Common::ES_ESP ? menuOffsets_ES[nummenu - 1][0] : menuOffsets_EN[nummenu - 1][0];
	tampunteromenu = g_engine->_lang == Common::ES_ESP ? menuOffsets_ES[nummenu - 1][1] : menuOffsets_EN[nummenu - 1][1];

	switch (nummenu) {
	case 1: {
		xmenu = 0;
		ymenu = 150;
	} break;
	case 2: {
		xmenu = 50;
		ymenu = 10;
	} break;
	case 3: {
		xmenu = 50;
		ymenu = 10;
	} break;
	case 4: {
		if (contadorpc2 > 20)
			showError(274);
		xmenu = 50;
		ymenu = 10;
	} break;
	case 5: {
		if (contadorpc > 23)
			showError(274);
		xmenu = 0;
		ymenu = 150;
	} break;
	case 6: {
		xmenu = 50;
		ymenu = 10;
	} break;
	case 7: {
		xmenu = 58;
		ymenu = 48;
	} break;
	case 8: {
		xmenu = 84;
		ymenu = 34;
	} break;
	}

	punteromenu = (byte *)malloc(tampunteromenu);
	menuFile.seek(posicionmenu);
	menuFile.read(punteromenu, tampunteromenu);
	putImg(xmenu, ymenu, punteromenu);
	free(punteromenu);
	menuFile.close();
}

void generateDiploma(Common::String &nombrefoto);

static void loadDiploma(Common::String &nombrefoto, Common::String &clave) {
	palette palauxlocal;
	byte *pantalla;
	uint size;
	byte *sello;

	Common::File fich;
	if (!fich.open("DIPLOMA.PAN")) {
		showError(318);
	}

	fich.read(palauxlocal, 768);

	pantalla = (byte *)malloc(64000);
	fich.read(pantalla, 64000);
	fich.close();

	if (!fich.open(Common::Path("DIPLOMA/SELLO.BMP")))
		showError(271);
	sello = (byte *)malloc(2054);
	fich.read(sello, 2054);
	fich.close();
	drawFullScreen(pantalla);

	free(pantalla);

	if (fich.open(Common::Path("DIPLOMA/" + nombrefoto + ".FOT"))) {
		size = fich.size() - 768;
		pantalla = (byte *)malloc(size);
		fich.read(pal, 768);
		fich.read(pantalla, size);
		fich.close();
		putShape(10, 20, pantalla);
		free(pantalla);
	}
	for (int i = 16; i <= 255; i++) {
		palauxlocal[i * 3 + 0] = pal[i * 3 + 0];
		palauxlocal[i * 3 + 1] = pal[i * 3 + 1];
		palauxlocal[i * 3 + 2] = pal[i * 3 + 2];
	}

	copyPalette(palauxlocal, pal);
	g_engine->_graphics->fixPalette(pal, 768);
	g_engine->_graphics->setPalette(pal);
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();

	char *passArray = (char *)malloc(10);
	for (int i = 0; i < 10; i++)
		passArray[i] = (char)(Random(10) + 48);

	clave.append(passArray, passArray + 10);

	const char *const *messages = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];
	outtextxyBios(91, 16, messages[49] + clave, 255);
	outtextxyBios(90, 15,  messages[49] + clave, 13);

	outtextxyBios(81, 61,  messages[50], 0);
	outtextxyBios(61, 81,  messages[51], 0);
	outtextxyBios(31, 101, messages[52] + nombrepersonaje, 0);
	outtextxyBios(31, 121, messages[53], 0);
	outtextxyBios(31, 141, messages[54], 0);
	outtextxyBios(31, 161, messages[55], 0);

	outtextxyBios(80, 60,  messages[50], 15);
	outtextxyBios(60, 80,  messages[51], 15);
	outtextxyBios(30, 100, messages[52], 15);

	outtextxyBios(150, 100, nombrepersonaje, 13);

	outtextxyBios(30, 120, messages[53], 15);
	outtextxyBios(30, 140, messages[54], 15);
	outtextxyBios(30, 160, messages[55], 15);
	delay(1500);
	playVoc("PORTAZO", 434988, 932);
	// putShape(270, 161, (byte *)sello);
	putShape(270, 159, sello);
	free(sello);
}

static void saveDiploma(Common::String &nombrefoto, Common::String &clave) {
	byte *panta;
	Common::DumpFile fich;

	Common::Path path;
	if (nombrefoto != "")
		path = Common::Path("DIPLOMA/" + nombrefoto + ".DIP");
	else
		path = Common::Path("DIPLOMA/DEFAULT.DIP");

	fich.open(path, true);
	debug("Path: %s", path.toString().c_str());
	if (!fich.isOpen()) {
		error("Could not open output file!");
	}

	palette palCopy;
	copyPalette(pal, palCopy);
	byte *palBuf = palCopy;
	for (int i = 0; i < 768; i++) {
		palBuf[i] = palBuf[i] >> 2;
	}
	char fixedClave[10];
	fixedClave[0] = 10;
	for (int i = 1; i < 10; i++) {
		fixedClave[i] = clave[i - 1];
	}
	fich.write(palBuf, 768);
	fich.write(fixedClave, 10);
	panta = (byte *)malloc(64000);
	copyFromScreen(panta);
	fich.write(panta, 64000);
	fich.finalize();
	fich.close();
	free(panta);
}

void generateDiploma(Common::String &nombrefoto) {
	Common::String clave;

	nombrefoto.toUppercase();
	totalFadeOut(0);
	loadDiploma(nombrefoto, clave);

	Common::Event e;
	bool keyPressed = false;
	do {
		g_engine->_screen->update();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYUP) {
				keyPressed = true;
			}
		}
		g_system->delayMillis(10);
	} while (!keyPressed && !g_engine->shouldQuit());
	saveDiploma(nombrefoto, clave);
}

void checkMouseGrid() {
	uint xrejilla, yrejilla;
	Common::String objmochila;
	if (contadorpc2 > 120)
		showError(274);
	if (yraton >= 0 && yraton <= 131) {
		xrejilla = (xraton + 7) / factorx;
		yrejilla = (yraton + 7) / factory;
		if (currentRoomData->mouseGrid[xrejilla][yrejilla] != currentRoomData->mouseGrid[oldxrejilla][oldyrejilla] || oldobjmochila != "") {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (numeroaccion) {
			case 0:
				actionLine = getActionLineText(0) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			case 1:
				actionLine = getActionLineText(1) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			case 2:
				actionLine = getActionLineText(2) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			case 3:
				actionLine = getActionLineText(3) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			case 4:
				if (objetomochila != "")
					actionLine = getActionLineText(4) + objetomochila + getActionLineText(7) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				else
					actionLine = getActionLineText(4) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			case 5:
				actionLine = getActionLineText(5) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			case 6:
				actionLine = getActionLineText(6) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
				break;
			default:
				actionLine = getActionLineText(0) + currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[xrejilla][yrejilla]]->objectName;
			}
			actionLineText(actionLine);
			g_engine->_mouseManager->show();
			oldxrejilla = xrejilla;
			oldyrejilla = yrejilla;
		}
		oldnumeroacc = 253;
		oldobjmochila = "";
	} else if (yraton >= 132 && yraton <= 165) {
		if (numeroaccion != oldnumeroacc) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (numeroaccion) {
			case 0:
				actionLine = getActionLineText(0);
				break;
			case 1:
				actionLine = getActionLineText(1);
				break;
			case 2:
				actionLine = getActionLineText(2);
				break;
			case 3:
				actionLine = getActionLineText(3);
				break;
			case 4:
				if (objetomochila != "")
					actionLine = getActionLineText(4) + objetomochila + getActionLineText(7);
				else
					actionLine = getActionLineText(4);
				break;
			case 5:
				actionLine = getActionLineText(5);
				break;
			case 6:
				actionLine = getActionLineText(6);
				break;
			}
			actionLineText(actionLine);
			g_engine->_mouseManager->show();
			oldnumeroacc = numeroaccion;
			oldobjmochila = "";
			oldxrejilla = 0;
			oldyrejilla = 0;
		}
	} else if (yraton >= 166 && yraton <= 199) {
		if (xraton >= 26 && xraton <= 65) {
			objmochila = mobj[posicioninv].objectName;
		} else if (xraton >= 70 && xraton <= 108) {
			objmochila = mobj[posicioninv + 1].objectName;
		} else if (xraton >= 113 && xraton <= 151) {
			objmochila = mobj[posicioninv + 2].objectName;
		} else if (xraton >= 156 && xraton <= 194) {
			objmochila = mobj[posicioninv + 3].objectName;
		} else if (xraton >= 199 && xraton <= 237) {
			objmochila = mobj[posicioninv + 4].objectName;
		} else if (xraton >= 242 && xraton <= 280) {
			objmochila = mobj[posicioninv + 5].objectName;
		} else {
			objmochila = ' ';
		}

		if (objmochila != oldobjmochila || oldxrejilla != 0) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (numeroaccion) {
			case 1:
				actionLine = getActionLineText(1) + objmochila;
				break;
			case 2:
				actionLine = getActionLineText(2) + objmochila;
				break;
			case 3:
				actionLine = getActionLineText(3) + objmochila;
				break;
			case 4:
				if (objetomochila == "")
					actionLine = getActionLineText(4) + objmochila;
				else
					actionLine = getActionLineText(4) + objetomochila + getActionLineText(7) + objmochila;
				break;
			case 5:
				actionLine = getActionLineText(5) + objmochila;
				break;
			case 6:
				actionLine = getActionLineText(6) + objmochila;
				break;
			default:
				outtextxy(160, 144, objmochila, 255, true, Graphics::kTextAlignCenter);
			}
			actionLineText(actionLine);
			g_engine->_mouseManager->show();
			oldobjmochila = objmochila;
		}
		oldnumeroacc = 255;
		oldxrejilla = 0;
		oldyrejilla = 0;
	}
}

void readAlphaGraph(Common::String &dato, int long_, int posx, int posy, byte colorbarra) {
	int pun = 1;
	bar(posx, posy - 2, posx + long_ * 8, posy + 8, colorbarra);

	outtextxyBios(posx, posy, "_", 0);

	Common::Event e;
	bool done = false;
	while (!done && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {

			if (e.type == Common::EVENT_KEYDOWN) {
				int keycode = e.kbd.keycode;
				int asciiCode = e.kbd.ascii;
				// ENTER key
				if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_KP_ENTER) {
					if (dato.size() > 0) {
						done = true;
						continue;
					}
				}
				// Max 8 chars
				if (pun > long_ && asciiCode != 8) {
					sound(750, 60);
					bar((posx + (dato.size()) * 8), (posy - 2), (posx + (dato.size() + 1) * 8), (posy + 8), 0);
				} else if (asciiCode == 8 && pun > 1) { // delete
					dato = dato.substr(0, dato.size() - 1);
					bar(posx, (posy - 2), (posx + long_ * 8), (posy + 8), colorbarra);
					outtextxyBios(posx, posy, dato.c_str(), 0);
					outtextxyBios((posx + (dato.size()) * 8), posy, "_", 0);
					pun -= 1;
				} else if (
					(asciiCode < 97 || asciiCode > 122) &&
					(asciiCode < 65 || asciiCode > 90) &&
					(asciiCode < 32 || asciiCode > 57) &&
					(asciiCode < 164 || asciiCode > 165)) {
					sound(1200, 60);
				} else {
					pun += 1;
					dato = dato + (char)e.kbd.ascii;
					bar(posx, (posy - 2), (posx + long_ * 8), (posy + 8), colorbarra);
					outtextxyBios(posx, posy, dato.c_str(), 0);
					outtextxyBios((posx + (dato.size()) * 8), posy, "_", 0);
				}
			}
		}
		g_system->delayMillis(10);
		g_engine->_screen->update();
	}
}

void readAlphaGraphSmall(Common::String &dato, int long_, int posx, int posy, byte colorbarra,
						 byte colortexto) {
	int pun = 1;
	bool borracursor;
	bar(posx, posy + 2, posx + long_ * 6, posy + 9, colorbarra);

	outtextxyBios(posx, posy, "-", colortexto);
	Common::Event e;
	bool done = false;

	while (!done && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				int keycode = e.kbd.keycode;
				int asciiCode = e.kbd.ascii;
				// ENTER key
				if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_KP_ENTER) {
					if (dato.size() > 0) {
						done = true;
						continue;
					}
				}

				if (pun > long_ && asciiCode != 8) {
					sound(750, 60);
					bar((posx + (dato.size()) * 6), (posy + 2), (posx + (dato.size() + 1) * 6), (posy + 9), colorbarra);
				} else if (asciiCode == 8 && pun > 1) {
					dato = dato.substr(0, dato.size() - 1);
					bar(posx, (posy + 2), (posx + long_ * 6), (posy + 9), colorbarra);
					outtextxyBios(posx, posy, dato, colortexto);
					outtextxyBios((posx + (dato.size()) * 6), posy, "-", colortexto);
					pun -= 1;
					borracursor = true;
				} else if ((asciiCode < '\40') || (asciiCode > '\373')) {
					sound(1200, 60);
					borracursor = false;
				} else {
					pun += 1;
					dato = dato + (char)e.kbd.ascii;
					bar(posx, (posy + 2), (posx + long_ * 6), (posy + 9), colorbarra);
					outtextxy(posx, posy, dato, colortexto);
					outtextxy((posx + (dato.size()) * 6), posy, "-", colortexto);
					borracursor = true;
				}
				// car = readkey();
				// car = upcase(car);
			}
		}

		g_system->delayMillis(10);
		g_engine->_screen->update();
	}

	if (borracursor)
		bar(posx + (dato.size()) * 6, posy + 2, (posx + (dato.size()) * 6) + 6, posy + 9, colorbarra);
}

void hipercadena(
	Common::String cadenatextnueva,
	uint xhcnueva,
	uint yhcnueva,
	byte anchohc,
	byte colortextohc,
	byte colorsombrahc) {
	Common::String cadenasalhc;
	byte ihc, iteracioneshc, lineahc;
	byte matrizsaltoshc[10];

	if (cadenatextnueva.size() < anchohc) {
		outtextxy((xhcnueva + 1), (yhcnueva + 1), cadenatextnueva, colorsombrahc, true);
		g_engine->_screen->update();
		delay(enforcedTextAnimDelay);
		outtextxy(xhcnueva, yhcnueva, cadenatextnueva, colortextohc, true);
		g_engine->_screen->update();
		delay(enforcedTextAnimDelay);
	} else {
		ihc = 0;
		iteracioneshc = 0;
		matrizsaltoshc[0] = 0;
		do {
			ihc += anchohc + 1;
			iteracioneshc += 1;
			do {
				ihc -= 1;
			} while (cadenatextnueva[ihc] != ' ');
			matrizsaltoshc[iteracioneshc] = ihc + 1;
		} while (ihc + 1 <= cadenatextnueva.size() - anchohc);

		iteracioneshc += 1;
		matrizsaltoshc[iteracioneshc] = cadenatextnueva.size();

		for (lineahc = 1; lineahc <= iteracioneshc; lineahc++) {
			Common::String lineString = cadenatextnueva.substr(matrizsaltoshc[lineahc - 1], matrizsaltoshc[lineahc] - matrizsaltoshc[lineahc - 1]);
			outtextxy((xhcnueva + 1), (yhcnueva + ((lineahc - 1) * 11) + 1), lineString, colorsombrahc, true);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			outtextxy(xhcnueva, (yhcnueva + ((lineahc - 1) * 11)), lineString, colortextohc, true);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
		}
	}
}

void buttonBorder(uint x1, uint y1, uint x2, uint y2,
				  byte color1, byte color2, byte color3, byte color4, byte color5, uint xtexto,
				  Common::String nombrepartidasalida) {

	bar(x1, y1, x2, y2, color4);
	line(x1, y1, x1, y2, color1);
	line(x1, y1, x2, y1, color1);

	line(x2, y1, x2, y2, color2);
	line(x2, y2, x1, y2, color2);

	putpixel(x2, y1, color3);
	putpixel(x1, y2, color3);

	// outtextxy(xtexto, (y1 - 1), nombrepartidasalida, color5);
	g_engine->_screen->addDirtyRect(Common::Rect(
		x1, y1, x2, y2));
	g_engine->_screen->update();
}

void copyProtection();

// static void buttonPress(uint xx1, uint yy1, uint xx2, uint yy2, bool bandera) {
// 	g_engine->_mouseManager->hide();

// 	byte color = bandera ? 249 : 255;

// 	line(xx1, yy1, (xx2 - 1), yy1, color);
// 	line(xx1, yy1, xx1, (yy2 - 1), color);
// 	color = bandera ? 255 : 249;
// 	line((xx1 + 1), yy2, xx2, yy2, color);
// 	line(xx2, (yy1 + 1), xx2, yy2, color);
// 	g_engine->_mouseManager->show();
// }

void copyProtection() {
	// const int retardopitido = 100;
	// byte *puntfondprotec;
	// uint xfade, oldxfade, ypaso, tamfondprotec, claveaccesointroducida,
	// 	palabraclaveacceso, oldxraton, oldyraton;
	// byte filanum, columnanum, posicioncursor, intentos, ytext, oldiraton,
	// 	oldcolorprotec;
	// // textsettingstype oldstyle;
	// bool salirprotec;
	// char chaux;
	// varying_string<5> clavetecleada, filastr, columnastr;
	// int _error;
	// int32 claveaccesoendisco, xorprot1, xorprot2;
	// file<int32> fichclave;

	// assign(fichclave, "MCGA.DRV");
	// /*$I-*/ reset(fichclave); /*$I+*/
	// if (ioresult != 0)
	// 	error("copyProtection(): ioresult (260)");
	// clavetecleada = "      ";
	// filanum = Random(95) + 1;
	// columnanum = Random(38) + 1;
	// clavetecleada[0] = '\0';
	// seek(fichclave, 1);
	// xorprot1 = 6543736;
	// fichclave >> xorprot1;
	// xorprot2 = 9873254;
	// seek(fichclave, (((filanum - 1) * 38) + columnanum + 1));
	// fichclave >> xorprot2;
	// claveaccesointroducida = 0;
	// fichclave >> claveaccesoendisco;
	// palabraclaveacceso = (uint)((claveaccesoendisco ^ xorprot1) - xorprot2);
	// salirprotec = false;
	// oldxraton = xraton;
	// close(fichclave);
	// oldyraton = yraton;
	// oldiraton = iraton;
	// drawMouseBackground(xraton, yraton);
	// // oldcolorprotec = getcolor();
	// // gettextsettings(oldstyle);
	// tamfondprotec = imagesize(50, 10, 270, 120);
	// puntfondprotec = (byte *)malloc(tamfondprotec);
	// getImg(50, 10, 270, 120, puntfondprotec);
	// // settextstyle(peque2, horizdir, 4);
	// xraton = 150;
	// yraton = 60;
	// iraton = 1;
	// setMouseArea(55, 13, 250, 105);
	// clearMouseAndKeyboard(npraton, npraton2);
	// if ((npraton > 0) || (npraton2 > 0))
	// 	error("copyProtection(): npraton (281)");
	// for (ytext = 1; ytext <= 6; ytext++)
	// 	buttonBorder((120 - (ytext * 10)), (80 - (ytext * 10)), (200 + (ytext * 10)),
	// 				 (60 + (ytext * 10)), 251, 251, 251, 251, 0, 0, "");
	// str(filanum, filastr);
	// str(columnanum, columnastr);
	// drawMenu(6);
	// outtextxy(65, 15, string("Mira en la fila ") + filastr + " y columna " + columnastr, 255);
	// posicioncursor = 0;
	// intentos = 0;
	// setMousePos(1, xraton, yraton);
	// do {
	// 	do {
	// 		if (tocapintar) {
	// 			drawMouseBackground(xraton, yraton);
	// 			drawMouseMask(iraton, xraton, yraton);
	// 			if (iraton > 7)
	// 				iraton = 1;
	// 			else
	// 				iraton += 1;
	// 			tocapintar = false;
	// 		}
	// 		getMouseClickCoords(0, npraton, pulsax, pulsay);
	// 	} while (!(npraton > 0));
	// 	pulsax += 7;
	// 	pulsay += 7;
	// 	if ((pulsax > 59 && pulsax < 180) && (pulsay > 28 && pulsay < 119))

	// 		switch (pulsay) {
	// 		case 29 ... 58:
	// 			switch (pulsax) {
	// 			case 60 ... 89:
	// 				if (posicioncursor < 50) {
	// 					sound(200);
	// 					buttonPress(60, 29, 89, 58, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "0", 255);
	// 					clavetecleada = clavetecleada + '0';
	// 					buttonPress(60, 29, 89, 58, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 90 ... 119:
	// 				if (posicioncursor < 50) {
	// 					sound(250);
	// 					buttonPress(90, 29, 119, 58, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "1", 255);
	// 					clavetecleada = clavetecleada + '1';
	// 					buttonPress(90, 29, 119, 58, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 120 ... 149:
	// 				if (posicioncursor < 50) {
	// 					sound(300);
	// 					buttonPress(120, 29, 149, 58, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "2", 255);
	// 					clavetecleada = clavetecleada + '2';
	// 					buttonPress(120, 29, 149, 58, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 150 ... 179:
	// 				if (posicioncursor < 50) {
	// 					sound(350);
	// 					buttonPress(150, 29, 179, 58, true);
	// 					delay(retardopitido);

	// 					outtextxy((205 + posicioncursor), 44, "3", 255);
	// 					clavetecleada = clavetecleada + '3';
	// 					buttonPress(150, 29, 179, 58, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			}
	// 			break;
	// 		case 59 ... 88:
	// 			switch (pulsax) {
	// 			case 60 ... 89:
	// 				if (posicioncursor < 50) {
	// 					sound(400);
	// 					buttonPress(60, 59, 89, 88, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "4", 255);
	// 					clavetecleada = clavetecleada + '4';
	// 					buttonPress(60, 59, 89, 88, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 90 ... 119:
	// 				if (posicioncursor < 50) {
	// 					sound(450);
	// 					buttonPress(90, 59, 119, 88, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "5", 255);
	// 					clavetecleada = clavetecleada + '5';
	// 					buttonPress(90, 59, 119, 88, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 120 ... 149:
	// 				if (posicioncursor < 50) {
	// 					sound(500);
	// 					buttonPress(120, 59, 149, 88, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "6", 255);
	// 					clavetecleada = clavetecleada + '6';
	// 					buttonPress(120, 59, 149, 88, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 150 ... 179:
	// 				if (posicioncursor < 50) {
	// 					sound(550);
	// 					buttonPress(150, 59, 179, 88, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "7", 255);
	// 					clavetecleada = clavetecleada + '7';
	// 					buttonPress(150, 59, 179, 88, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			}
	// 			break;
	// 		case 89 ... 118:
	// 			switch (pulsax) {
	// 			case 60 ... 89:
	// 				if (posicioncursor < 50) {
	// 					sound(600);
	// 					buttonPress(60, 89, 89, 118, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "8", 255);
	// 					clavetecleada = clavetecleada + '8';
	// 					buttonPress(60, 89, 89, 118, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 90 ... 119:
	// 				if (posicioncursor < 50) {
	// 					sound(650);
	// 					buttonPress(90, 89, 119, 118, true);
	// 					delay(retardopitido);
	// 					outtextxy((205 + posicioncursor), 44, "9", 255);
	// 					clavetecleada = clavetecleada + '9';
	// 					buttonPress(90, 89, 119, 118, false);
	// 					posicioncursor += 10;
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 120 ... 149:
	// 				if (posicioncursor > 0) {
	// 					sound(700);
	// 					buttonPress(120, 89, 149, 118, true);
	// 					delay(retardopitido);
	// 					posicioncursor -= 10;
	// 					outtextxy((205 + posicioncursor), 44, "�", 250);
	// 					clavetecleada = copy(clavetecleada, 1,
	// 										 (length(clavetecleada) - 1));
	// 					buttonPress(120, 89, 149, 118, false);
	// 					nosound;
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			case 150 ... 179:
	// 				if (posicioncursor > 39) {
	// 					sound(750);
	// 					buttonPress(150, 89, 179, 118, true);
	// 					delay(retardopitido);
	// 					val(clavetecleada, claveaccesointroducida, _error);
	// 					buttonPress(150, 89, 179, 118, false);
	// 					nosound;
	// 					if ((_error == 0) && (intentos < 3)) {
	// 						if (claveaccesointroducida == palabraclaveacceso)
	// 							salirprotec = true;
	// 						else {
	// 							intentos += 1;
	// 							sound(60);
	// 							delay(700);
	// 							nosound;
	// 						}
	// 					}
	// 					if (intentos >= 3)
	// 						error("copyProtection(): too many attempts! (259)");
	// 				} else {
	// 					sound(70);
	// 					delay(250);
	// 					nosound;
	// 				}
	// 				break;
	// 			}
	// 			break;
	// 		}
	// 	clearMouseAndKeyboard(npraton, npraton2);
	// 	if ((npraton > 0) || (npraton2 > 0))
	// 		error("copyProtection(): npraton! (282)");
	// } while (!salirprotec);
	// putImg(50, 10, puntfondprotec);
	// xraton = oldxraton;
	// yraton = oldyraton;
	// iraton = oldiraton;
	// setMousePos(iraton, xraton, yraton);
	// if (contadorpc > 8)
	// 	error("copyProtection(): contadorpc! (274)");
	// freemem(puntfondprotec, tamfondprotec);
	// setMouseArea(0, 0, 305, 185);
}

void credits();

void drawCreditsScreen(byte *&fondopp, uint &sizefondo2, byte *&fondo2) {
	palette palpaso, palnegro;

	Common::File fichpp;

	if (!fichpp.open("DIPLOMA.PAN")) {
		showError(315);
	}
	fondopp = (byte *)malloc(64000);
	fichpp.read(palpaso, 768);
	fichpp.read(fondopp, 64000);
	fichpp.close();

	drawFullScreen(fondopp);

	sizefondo2 = imagesize(0, 0, 319, 59);

	// Screen is now fondopp so fondo2 contains a 320x60 crop of fondopp
	fondo2 = (byte *)malloc(sizefondo2);
	getImg(0, 0, 319, 59, fondo2);

	for (int i = 0; i <= 255; i++) {
		palnegro[i * 3 + 0] = 0;
		palnegro[i * 3 + 1] = 0;
		palnegro[i * 3 + 2] = 0;
		// Adjust for 6-bit DAC color
		palpaso[i * 3 + 0] = palpaso[i * 3 + 0] << 2;
		palpaso[i * 3 + 1] = palpaso[i * 3 + 1] << 2;
		palpaso[i * 3 + 2] = palpaso[i * 3 + 2] << 2;
	}

	changePalette(palnegro, palpaso);
	copyPalette(palpaso, pal);
	if (contadorpc2 > 9)
		showError(274);
}

void putCreditsImg(uint x, uint y, byte *imagen1, byte *imagen2, bool direct) {

	uint16 wImagen1, hImagen1;
	uint auxhor;
	uint incremento, incremento2;
	byte *paso;

	wImagen1 = READ_LE_UINT16(imagen1);
	hImagen1 = READ_LE_UINT16(imagen1 + 2);

	paso = (byte *)malloc((wImagen1 + 1) * (hImagen1 + 1) + 4);

	auxhor = wImagen1 + 1;
	kaka = hImagen1 + y;

	// makes sure that if the image is at the bottom of the screen we chop the bottom part
	for (int i = kaka; i >= 200; i--)
		hImagen1 -= 1;

	hImagen1++;

	// Copies the crop in the background corresponding to the current credit window in imagen1
	for (int i = 0; i < hImagen1; i++) {
		byte *src = imagen2 + (320 * (y + i)) + x;
		byte *dst = paso + 4 + (auxhor * i);
		Common::copy(src, src + auxhor, dst);
	}

	for (int kk = 0; kk < hImagen1; kk++) {
		incremento2 = (kk * wImagen1) + 4;
		kaka = kk + y;
		for (int jj = 0; jj <= wImagen1; jj++) {
			incremento = incremento2 + jj;
			if ((direct && imagen1[incremento] > 0) || (imagen1[incremento] > 16 && kaka >= 66 && kaka <= 192)) {
				paso[incremento] = imagen1[incremento];
			} else if (imagen1[incremento] > 16) {
				switch (kaka) {
				case 59:
				case 199:
					paso[incremento] = imagen1[incremento] + 210;
					break;
				case 60:
				case 198:
					paso[incremento] = imagen1[incremento] + 180;
					break;
				case 61:
				case 197:
					paso[incremento] = imagen1[incremento] + 150;
					break;
				case 62:
				case 196:
					paso[incremento] = imagen1[incremento] + 120;
					break;
				case 63:
				case 195:
					paso[incremento] = imagen1[incremento] + 90;
					break;
				case 64:
				case 194:
					paso[incremento] = imagen1[incremento] + 60;
					break;
				case 65:
				case 193:
					paso[incremento] = imagen1[incremento] + 30;
					break;
				}
			}
		}
	}
	// Wait until render tick
	do {
		g_engine->_chrono->updateChrono();
		g_system->delayMillis(10);
	} while (!tocapintar && !g_engine->shouldQuit());
	tocapintar = false;

	// Copies the credit window directly to the screen
	for (int i = 0; i < hImagen1; i++) {
		byte *src = paso + 4 + (auxhor * i);
		byte *dst = ((byte *)g_engine->_screen->getPixels()) + (320 * (y + i)) + x;
		Common::copy(src, src + auxhor, dst);
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x, y, x + wImagen1 + 1, y + hImagen1 + 1));
	free(paso);
}

void scrollCredit(
	int32 posicion,
	uint tam,
	palette &pal2,
	byte *&fondopp,
	bool &salirpitando,
	int minHeight,
	bool withFade,
	bool refresh) {
	Common::File fich;
	if (!fich.open("CREDITOS.DAT")) {
		showError(270);
	}
	fich.seek(posicion);
	fich.read(background, tam);
	fich.read(pal2, 768);
	fich.close();

	for (int i = 16; i <= 255; i++) {
		// Adjust for 6-bit DAC
		pal2[i * 3 + 0] = pal2[i * 3 + 0] << 2;
		pal2[i * 3 + 1] = pal2[i * 3 + 1] << 2;
		pal2[i * 3 + 2] = pal2[i * 3 + 2] << 2;

		pal[i * 3 + 0] = pal2[i * 3 + 0];
		pal[i * 3 + 1] = pal2[i * 3 + 1];
		pal[i * 3 + 2] = pal2[i * 3 + 2];
	}

	changeRGBBlock(16, 240, &pal[16 * 3 + 0]);
	Common::Event e;
	bool keyPressed = false;

	// Loops an image from the bottom of the screen to the top
	for (int i = 199; i >= minHeight; i--) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYUP) {
				keyPressed = true;
			}
		}
		putCreditsImg(85, i, background, fondopp, !withFade);
		if (keyPressed) {
			salirpitando = true;
			break;
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
		if (g_engine->shouldQuit())
			break;
	}
	if (refresh) {
		copyFromScreen(fondopp);
	}
}

void scrollSingleCredit(
	int32 posicion,
	uint tam,
	palette &pal2,
	byte *&fondopp,
	bool &salirpitando) {
	scrollCredit(
		posicion,
		tam,
		pal2,
		fondopp,
		salirpitando,
		8,
		true,
		false);
}

void removeTitle(byte *&fondo2) {
	uint i2, j2;
	Common::Event e;
	for (int i1 = 1; i1 <= 15000; i1++) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		i2 = Random(318);
		j2 = Random(58);
		byte *src = fondo2 + 4 + (j2 * 320) + i2;
		byte *dest = ((byte *)g_engine->_screen->getPixels()) + (j2 * 320) + i2;
		Common::copy(src, src + 2, dest);

		byte *src2 = fondo2 + 4 + ((j2 + 1) * 320) + i2;
		byte *dest2 = ((byte *)g_engine->_screen->getPixels()) + ((j2 + 1) * 320) + i2;

		Common::copy(src2, src2 + 2, dest2);

		i2 = Random(320);
		j2 = Random(60);

		byte *src3 = fondo2 + 4 + (j2 * 320) + i2;
		byte *dest3 = ((byte *)g_engine->_screen->getPixels()) + (j2 * 320) + i2;
		Common::copy(src3, src3 + 1, dest3);
		if (i1 % 200 == 0) {
			g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 60));
			g_engine->_screen->update();
		}
		if (g_engine->shouldQuit()) {
			break;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 60));
	g_engine->_screen->update();
}

inline bool keyPressed() {
	Common::Event e;
	g_system->getEventManager()->pollEvent(e);
	return e.type == Common::EVENT_KEYUP;
}

void credits() {
	saveAllowed = true;
	debug("Credits");
	// 	byte ii;
	palette pal2;
	byte *fondopp;
	byte *fondo2;
	uint sizefondo2;
	bool salirpitando;

	g_engine->_mouseManager->hide();
	totalFadeOut(0);
	lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	cleardevice();
	playMidiFile("CREDITOS", true);
	restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	drawCreditsScreen(fondopp, sizefondo2, fondo2);

	salirpitando = false;

	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollCredit(0, 8004, pal2, fondopp, salirpitando, 10, false, true);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(8772, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(17544, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(26316, 7504, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(34588, 7504, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(42860, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(51632, 7504, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	removeTitle(fondo2);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	putImg(0, 0, fondo2);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	copyFromScreen(fondopp);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollCredit(59904, 8004, pal2, fondopp, salirpitando, 10, false, true);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(68676, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(77448, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(86220, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(94992, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(103764, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollSingleCredit(112536, 8004, pal2, fondopp, salirpitando);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	removeTitle(fondo2);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	putImg(0, 0, fondo2);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	copyFromScreen(fondopp);
	if (keyPressed() || salirpitando)
		goto Lsalida;
	scrollCredit(121308, 8004, pal2, fondopp, salirpitando, 80, false, true);
Lsalida:
	delay(1000);
	totalFadeOut(0);
	lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	cleardevice();
	playMidiFile("INTRODUC", true);
	restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	g_engine->_mouseManager->show();
	free(fondopp);
	free(fondo2);
}

void introduction() {
	saveAllowed = false;
	g_engine->_mouseManager->hide();
	bool pulsada_salida;
	uint contadorvueltas;
	bool isSpanish = g_engine->_lang == Common::ES_ESP;
	const char *const *messages = (isSpanish) ? fullScreenMessages[0] : fullScreenMessages[1];
	const long *offsets = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0] : flcOffsets[1];
	pulsada_salida = false;
	totalFadeOut(0);

	if (contadorpc > 6)
		showError(270);
	cleardevice();
	drawFlc(136, 53, offsets[2], 136, 9, 1, true, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(135, 54, offsets[3], 0, 9, 2, true, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	totalFadeOut(0);
	cleardevice();
	if(isSpanish) {
		outtextxy(25,  20, messages[0], 253);
		outtextxy(25,  35, messages[1], 253);
		outtextxy(25,  50, messages[2], 253);
		outtextxy(25,  65, messages[3], 253);
		outtextxy(25,  80, messages[4], 253);
		outtextxy(25,  95, messages[5], 253);
		outtextxy(25, 120, messages[6], 253);
		outtextxy(25, 140, messages[7], 253);
		outtextxy(25, 155, messages[8], 253);
	}
	else {
		outtextxy(25,  35, messages[0], 253);
		outtextxy(25,  55, messages[1], 253);
		outtextxy(25,  75, messages[2], 253);
		outtextxy(25,  95, messages[3], 253);
		outtextxy(25, 115, messages[4], 253);
		outtextxy(25, 135, messages[5], 253);
	}
	if(g_engine->shouldQuit()){
		return;
	}
	totalFadeIn(0, "DEFAULT");
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
	contadorvueltas = 0;

	do {
		g_engine->_chrono->updateChrono();
		Common::Event e;
		g_system->getEventManager()->pollEvent(e);
		if (e.type == Common::EVENT_KEYDOWN || e.type == Common::EVENT_LBUTTONUP) {
			goto Lsalirpres;
		}

		if (tocapintar) {
			contadorvueltas += 1;
		}
		g_system->delayMillis(10);
	} while (contadorvueltas < 180 && !g_engine->shouldQuit());

	totalFadeOut(0);
	cleardevice();
	drawFlc(0, 0, offsets[4], 0, 9, 3, true, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[5], 2, 9, 4, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[6], 3, 9, 5, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[7], 0, 9, 0, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[8], isSpanish? 4:8, 9, 6, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[9], 3, 9, 7, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[8], isSpanish? 3:8, 9, 8, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[9], 2, 9, 9, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(0, 0, offsets[10], 0, 9, 0, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(235, 100, offsets[11], 3, 9, 10, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[12], 0, 9, 11, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(235, 100, offsets[11], 3, 9, 12, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[12], isSpanish? 0:2, 9, 13, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(235, 100, offsets[11], isSpanish? 3:8, 9, 14, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[12], 0, 9, 15, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(173, 98, offsets[13], 0, 9, 0, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(224, 100, offsets[14], 2, 9, 16, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(0, 0, offsets[15], 0, 18, 17, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[16], 0, 9, 18, false, true, false, pulsada_salida);
	if (pulsada_salida)
		goto Lsalirpres;
	delay(1000);
Lsalirpres:
	debug("Exiting intro!");
	totalFadeOut(0);
	cleardevice();
	g_engine->_mouseManager->show();
}

void firstIntroduction() {
	if (!hechaprimeravez && !introSeen) {
		introduction();
		hechaprimeravez = true;
		ConfMan.setBool("introSeen", true);
		ConfMan.flushToDisk();
	}
}

void initialLogo() {
	bool basurillalogica = false;
	long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][0] : flcOffsets[1][0];
	drawFlc(0, 0, offset, 0, 18, 25, false, false, false, basurillalogica);
	delay(1000);
}

void initialMenu(bool fundido) {
	bool kklogica = false;
	bool opcionvalida = false;
	stopVoc();

	long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][1] : flcOffsets[1][1];

	if (fundido)
		drawFlc(0, 0, offset, 0, 9, 0, true, false, false, kklogica);
	else
		drawFlc(0, 0, offset, 0, 9, 0, false, false, false, kklogica);
	if (contadorpc2 > 10)
		showError(274);
	xraton = 160;
	yraton = 95;
	iraton = 1;
	setMousePos(iraton, xraton, yraton);
	Common::Event e;
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (isMouseEvent(e)) {
				g_engine->_mouseManager->setMousePos(e.mouse);
			}
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
					exitToDOS();
				}
			}
			if (e.type == Common::EVENT_LBUTTONUP) {
				uint x = e.mouse.x + 7;
				uint y = e.mouse.y + 7;
				if (y > 105 && y < 120) {
					if (x > 46 && x < 145) {
						partidanueva = true;
						continuarpartida = false;
						opcionvalida = true;
					} else if (x > 173 && x < 267) {
						credits();
						drawFlc(0, 0, offset, 0, 9, 0, true, false, false, kklogica);
					}
				} else if (y > 140 && y < 155) {
					if (x > 173 && x < 292) {
						totalFadeOut(0);
						cleardevice();
						introduction();
						drawFlc(0, 0, offset, 0, 9, 0, true, false, false, kklogica);
					} else if (x >= 18 && x <= 145) {
						debug("Load");
						partidanueva = false;
						continuarpartida = false;
						opcionvalida = true;
					}
				} else if (y > 174 && y < 190) {
					if (x > 20 && x < 145) {
						partidanueva = false;
						opcionvalida = true;
						continuarpartida = true;
					} else if (x > 173 && y < 288) {
						exitToDOS();
					}
				}
			}
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
	} while (!opcionvalida && !g_engine->shouldQuit());
}

void saveTemporaryGame() {
	// uint indiaux;
	// byte zonasitio, i1, i2;

	// assign(partida, "GAME07.SAV");
	// rewrite(partida);
	// regpartida.indicetray = indicetray;
	// zonasitio = roomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
	// if (zonasitio > 9) {
	// 	do {
	// 		indicetray -= 1;
	// 		characterPosX = trayec[indicetray].x;
	// 		characterPosY = trayec[indicetray].y;
	// 		zonasitio = roomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
	// 	} while (!((indicetray == 1) || (zonasitio < 10)));
	// }
	// if (zonasitio < 10) {
	// 	regpartida.xframe = characterPosX;
	// 	regpartida.yframe = characterPosY;
	// } else {
	// 	indicetray = regpartida.indicetray;
	// 	do {
	// 		indicetray += 1;
	// 		characterPosX = trayec[indicetray].x;
	// 		characterPosY = trayec[indicetray].y;
	// 		zonasitio = roomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
	// 	} while (!((zonasitio < 10) || (indicetray == longtray)));
	// 	if (zonasitio < 10) {
	// 		regpartida.xframe = characterPosX;
	// 		regpartida.yframe = characterPosY;
	// 	} else
	// 		error("saveTemporaryGame(): error saving! (275)");
	// }
	// regpartida.numeropantalla = roomData->codigo;
	// regpartida.longtray = longtray;
	// regpartida.indicetray = indicetray;
	// regpartida.codigoobjmochila = codigoobjmochila;
	// regpartida.volumenfxderecho = volumenfxderecho;
	// regpartida.volumenfxizquierdo = volumenfxizquierdo;
	// regpartida.volumenmelodiaderecho = volumenmelodiaderecho;
	// regpartida.volumenmelodiaizquierdo = volumenmelodiaizquierdo;
	// regpartida.oldxrejilla = oldxrejilla;
	// regpartida.oldyrejilla = oldyrejilla;
	// regpartida.animadoprofundidad = animado.profundidad;
	// regpartida.animadodir = animado.dir;
	// regpartida.animadoposx = animado.posx;
	// regpartida.animadoposy = animado.posy;
	// regpartida.animadoiframe2 = iframe2;
	// regpartida.zonaactual = zonaactual;
	// regpartida.zonadestino = zonadestino;
	// regpartida.oldzonadestino = oldzonadestino;
	// regpartida.posicioninv = posicioninv;
	// regpartida.numeroaccion = numeroaccion;
	// regpartida.oldnumeroacc = oldnumeroacc;
	// regpartida.pasos = pasos;
	// regpartida.indicepuertas = indicepuertas;
	// regpartida.direccionmovimiento = direccionmovimiento;
	// regpartida.iframe = iframe;
	// regpartida.parte_del_juego = parte_del_juego;
	// regpartida.manual_torno = manual_torno;
	// regpartida.sello_quitado = sello_quitado;
	// regpartida.lista1 = lista1;
	// regpartida.lista2 = lista2;
	// regpartida.completadalista1 = completadalista1;
	// regpartida.completadalista2 = completadalista2;
	// regpartida.vasijapuesta = vasijapuesta;
	// regpartida.guadagna = guadagna;
	// regpartida.tridente = tridente;
	// regpartida.torno = torno;
	// regpartida.barro = barro;
	// regpartida.diablillo_verde = diablillo_verde;
	// regpartida.rojo_capturado = rojo_capturado;
	// regpartida.alacena_abierta = alacena_abierta;
	// regpartida.baul_abierto = baul_abierto;
	// regpartida.teleencendida = teleencendida;
	// regpartida.trampa_puesta = trampa_puesta;
	// for (indiaux = 0; indiaux < inventoryIconCount; indiaux++) {
	// 	regpartida.mobj[indiaux].bitmapIndex = mobj[indiaux].bitmapIndex;
	// 	regpartida.mobj[indiaux].code = mobj[indiaux].code;
	// 	regpartida.mobj[indiaux].objectName = mobj[indiaux].objectName;
	// }
	// regpartida.elemento1 = elemento1;
	// regpartida.elemento2 = elemento2;
	// regpartida.xframe = characterPosX;
	// regpartida.yframe = characterPosY;
	// regpartida.xframe2 = xframe2;
	// regpartida.yframe2 = yframe2;
	// regpartida.oldobjmochila = oldobjmochila;
	// regpartida.objetomochila = objetomochila;
	// regpartida.nombrepersonaje = nombrepersonaje;
	// for(int i = 0; i < routePointCount; i++) {
	// 	regpartida.mainRoute[i].x = mainRoute[i].x;
	// 	regpartida.mainRoute[i].y = mainRoute[i].y;
	// }
	// for (indiaux = 0; indiaux < 300; indiaux++) {
	// 	regpartida.trayec[indiaux].x = trayec[indiaux].x;
	// 	regpartida.trayec[indiaux].y = trayec[indiaux].y;
	// }
	// for (indiaux = 0; indiaux < maxpersonajes; indiaux++) {
	// 	regpartida.primera[indiaux] = primera[indiaux];
	// 	regpartida.lprimera[indiaux] = lprimera[indiaux];
	// 	regpartida.cprimera[indiaux] = cprimera[indiaux];
	// 	regpartida.libro[indiaux] = libro[indiaux];
	// 	regpartida.caramelos[indiaux] = caramelos[indiaux];
	// }

	// for (indiaux = 0; indiaux < 5; indiaux++) {
	// 	regpartida.cavernas[indiaux] = cavernas[indiaux];
	// 	regpartida.firstList[indiaux] = firstList[indiaux];
	// 	regpartida.secondList[indiaux] = secondList[indiaux];
	// }
	// for (indiaux = 0; indiaux < 4; indiaux++) {
	// 	regpartida.hornacina[0][indiaux] = hornacina[0][indiaux];
	// 	regpartida.hornacina[1][indiaux] = hornacina[1][indiaux];
	// }
	// partida << regpartida;
	// close(partida);
	// setRoomTrajectories(altoanimado, anchoanimado, true);

	// // assign(fichpanta, "PANTALLA.DAT");
	// // /*$I-*/ reset(fichpanta); /*$I+*/
	// // if (ioresult != 0)
	// // 	error("saveTemporaryGame(): error saving ioresult! (320)");
	// // seek(fichpanta, ((datospantalla.codigo * 8) + 7));
	// // fichpanta << datospantalla;
	// // close(fichpanta);
}

void exitGame() {
	clear();
	g_system->quit();
}

void clearGame() {
	debug("releasing game...");
	resetGameState();
	clearAnims();
	clearVars();
}

void exitToDOS() {
	debug("Exit to dos!");
	uint oldxraton, oldyraton, tamfondcontroles;
	byte oldiraton;
	// textsettingstype oldstyle;
	char chpasosalida;

	oldxraton = xraton;
	oldyraton = yraton;
	oldiraton = iraton;
	g_engine->_mouseManager->hide();
	tamfondcontroles = imagesize(58, 48, 262, 120);
	byte *puntfondcontroles = (byte *)malloc(tamfondcontroles);
	getImg(58, 48, 262, 120, puntfondcontroles);

	drawMenu(7);
	xraton = 160;
	yraton = 90;
	iraton = 1;

	setMouseArea(115, 80, 190, 100);
	setMousePos(iraton, xraton, yraton);
	Common::Event e;
	const char hotKeyYes = hotKeyFor(YES);
	const char hotKeyNo = hotKeyFor(NO);
	chpasosalida = '@';
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();

		while (g_system->getEventManager()->pollEvent(e)) {
			if (isMouseEvent(e)) {
				g_engine->_mouseManager->setMousePos(e.mouse);
			}
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
					chpasosalida = '\33';
				} else if (e.kbd.keycode == hotKeyYes) {
					debug("would exit game now");
					free(puntfondcontroles);
					// CLEAR MEMORY!!
					exitGame();
					// exit game
				} else if (e.kbd.keycode == hotKeyNo) {
					chpasosalida = '\33';
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				uint x = e.mouse.x;
				if (x < 145) {
					free(puntfondcontroles);
					// exit game
					g_system->quit();
				} else if (x > 160) {
					chpasosalida = '\33';
				}
			}
		}
		g_engine->_screen->update();
	} while (chpasosalida != '\33' && !g_engine->shouldQuit());
	debug("finished exitToDos");
	putImg(58, 48, puntfondcontroles);
	xraton = oldxraton;
	yraton = oldyraton;
	iraton = oldiraton;
	g_engine->_mouseManager->show();
	free(puntfondcontroles);
	setMouseArea(0, 0, 305, 185);
}

void soundControls() {
	uint oldxraton,
		oldyraton,
		tamfondcontroles,
		sliderSize,
		sliderBgSize,
		volumenfx,
		volumenmelodia,
		xfade,
		oldxfade;

	byte ytext, oldiraton;
	bool salirmenucontroles;

	salirmenucontroles = false;
	oldxraton = xraton;
	oldyraton = yraton;
	oldiraton = iraton;
	g_engine->_mouseManager->hide();

	tamfondcontroles = imagesize(50, 10, 270, 120);
	byte *puntfondcontroles = (byte *)malloc(tamfondcontroles);
	getImg(50, 10, 270, 120, puntfondcontroles);

	xraton = 150;
	yraton = 60;
	iraton = 1;

	setMouseArea(55, 13, 250, 105);

	for (ytext = 1; ytext <= 6; ytext++)
		buttonBorder(120 - (ytext * 10), 80 - (ytext * 10), 200 + (ytext * 10), 60 + (ytext * 10), 251, 251, 251, 251, 0, 0, "");

	buttonBorder(86, 31, 94, 44, 0, 0, 0, 0, 0, 0, "");

	line(90, 31, 90, 44, 255);

	sliderSize = imagesize(86, 31, 94, 44);
	byte *slider = (byte *)malloc(sliderSize);
	getImg(86, 31, 94, 44, slider);

	drawMenu(3);
	sliderBgSize = imagesize(86, 31, 234, 44);

	byte *sliderBackground1 = (byte *)malloc(sliderBgSize);
	byte *sliderBackground2 = (byte *)malloc(sliderBgSize);
	getImg(86, 31, 234, 44, sliderBackground1);
	getImg(86, 76, 234, 89, sliderBackground2);

	volumenfx = round(((volumenfxderecho + volumenfxizquierdo) / 2) * 20);
	volumenmelodia = round(((volumenmelodiaderecho + volumenmelodiaizquierdo) / 2) * 20);
	putImg(volumenfx + 86, 31, slider);
	putImg(volumenmelodia + 86, 76, slider);

	setMousePos(1, xraton, yraton);
	bool keyPressed = false;
	bool mouseClicked = false;
	Common::Event e;
	do {
		g_engine->_chrono->updateChrono();
		do {
			g_engine->_chrono->updateChrono();
			g_engine->_mouseManager->animateMouseIfNeeded();
			while (g_system->getEventManager()->pollEvent(e)) {
				if (e.type == Common::EVENT_KEYUP) {
					keyPressed = true;
				}
				if (e.type == Common::EVENT_LBUTTONDOWN) {
					mouseClicked = true;
					pulsax = e.mouse.x;
					pulsay = e.mouse.y;
				}
			}
			g_engine->_screen->update();
		} while ((!keyPressed && !mouseClicked) && !g_engine->shouldQuit());

		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();
		if (keyPressed) {
			salirmenucontroles = true;
		}
		if (mouseClicked) {
			if (pulsay >= 22 && pulsay <= 37) {
				g_engine->_mouseManager->hide();
				xfade = 86 + volumenfx;
				bool mouseReleased = false;
				do {

					oldxfade = xfade;
					while (g_system->getEventManager()->pollEvent(e)) {
						if (e.type == Common::EVENT_LBUTTONUP) {
							mouseReleased = true;
						} else if (e.type == Common::EVENT_MOUSEMOVE) {
							xfade = e.mouse.x;
						}
					}
					if (xfade < 86) {
						xfade = 86;
					} else if (xfade > 226) {
						xfade = 226;
					}

					if (oldxfade != xfade) {
						putImg(86, 31, sliderBackground1);
						putImg(xfade, 31, slider);
						// This yields a volume between 0 and 140
						volumenfx = xfade - 86;

						debug("volumefx=%d", volumenfx);
						volumenfxderecho = round((float)volumenfx / 20);
						volumenfxizquierdo = round((float)volumenfx / 20);
						setSfxVolume(volumenfxizquierdo, volumenfxderecho);
					}
					g_engine->_screen->update();
				} while (!mouseReleased);

				g_engine->_mouseManager->show();
			} else if (pulsay >= 67 && pulsay <= 82) {
				g_engine->_mouseManager->hide();
				xfade = 86 + volumenmelodia;
				bool mouseReleased = false;
				do {
					while (g_system->getEventManager()->pollEvent(e)) {
						if (e.type == Common::EVENT_LBUTTONUP) {
							mouseReleased = true;
						} else if (e.type == Common::EVENT_MOUSEMOVE) {
							xfade = e.mouse.x;
						}
					}
					if (xfade < 86) {
						xfade = 86;
					} else if (xfade > 226) {
						xfade = 226;
					}

					if (oldxfade != xfade) {
						putImg(86, 76, sliderBackground2);
						putImg(xfade, 76, slider);
						volumenmelodia = xfade - 86;
						volumenmelodiaderecho = round((float)(volumenmelodia) / 20);
						volumenmelodiaizquierdo = round((float)(volumenmelodia) / 20);
						setMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
					}
					g_engine->_screen->update();
				} while (!mouseReleased);

				g_engine->_mouseManager->show();
			} else if (pulsay >= 97 && pulsay <= 107) {
				salirmenucontroles = true;
			}
			mouseClicked = false;
		}
		g_system->delayMillis(10);
		g_engine->_screen->update();
	} while (!salirmenucontroles && !g_engine->shouldQuit());

	putImg(50, 10, puntfondcontroles);
	xraton = oldxraton;
	yraton = oldyraton;
	iraton = oldiraton;
	setMousePos(iraton, xraton, yraton);
	free(puntfondcontroles);
	free(slider);
	free(sliderBackground1);
	free(sliderBackground2);

	if (contadorpc > 7)
		showError(274);

	setMouseArea(0, 0, 305, 185);
}

void sacrificeScene() {
	saveAllowed = false;
	palette palaux;

	Common::File fich;
	bool isSpanish = (g_engine->_lang == Common::ES_ESP);
	const char *const *messages = (isSpanish) ? fullScreenMessages[0] : fullScreenMessages[1];

	const long *offsets = (isSpanish) ? flcOffsets[0] : flcOffsets[1];

	stopVoc();
	bool pulsada_salida = currentRoomData->paletteAnimationFlag;
	currentRoomData->paletteAnimationFlag = false;

	bar(0, 139, 319, 149, 0);
	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		outtextxy(10, 10, messages[9], 200);
		outtextxy(10, 30, messages[10], 200);
		outtextxy(10, 50, messages[11], 200);
		outtextxy(10, 70, messages[12], 200);
		outtextxy(10, 90, messages[13], 200);
	} else {
		outtextxy(10, 20, messages[9], 200);
		outtextxy(10, 40, messages[10], 200);
		outtextxy(10, 60, messages[11], 200);
		outtextxy(10, 80, messages[12], 200);
		outtextxy(10, 100, messages[13], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);

	delay(10000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);

	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		outtextxy(10, 10, messages[14], 200);
		outtextxy(10, 30, messages[15], 200);
		outtextxy(10, 50, messages[16], 200);
		outtextxy(10, 70, messages[17], 200);
		outtextxy(10, 90, messages[18], 200);
	} else {
		outtextxy(10, 20, messages[14], 200);
		outtextxy(10, 40, messages[15], 200);
		outtextxy(10, 60, messages[16], 200);
		outtextxy(10, 80, messages[17], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	delay(10000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);

	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		outtextxy(10, 10, messages[19], 200);
		outtextxy(10, 50, messages[20], 200);
		outtextxy(10, 70, messages[21], 200);
		outtextxy(10, 90, messages[22], 200);
	} else {
		outtextxy(10, 30, messages[19], 200);
		outtextxy(10, 60, messages[20], 200);
		outtextxy(10, 80, messages[21], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	delay(10000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	bar(10, 10, 310, 120, 0);

	if (!fich.open("SALONREC.PAN")) {
		showError(318);
	}
	fich.read(palaux, 768);
	fich.read(background, 44800);
	fich.close();

	pal[0] = 0;
	pal[1] = 0;
	pal[2] = 0;
	for (int i = 1; i <= 234; i++) {
		pal[i * 3 + 1] = palaux[i * 3 + 1] << 2;
		pal[i * 3 + 2] = palaux[i * 3 + 2] << 2;
		pal[i * 3 + 3] = palaux[i * 3 + 3] << 2;
	}

	// We dont have the width and height here in the byte buffer
	drawScreen(background, false);
	partialFadeIn(234);
	stopVoc();

	if (g_engine->shouldQuit())
		return;

	drawFlc(0, 0, offsets[17], 0, 9, 19, false, false, true, pulsada_salida);
	totalFadeOut(128);
	stopVoc();
	delay(1000);
	if (g_engine->shouldQuit())
		return;

	lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	playMidiFile("SACRIFIC", true);
	restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	clear();

	outtextxy(10, 31, messages[23], 254);
	outtextxy(10, 29, messages[23], 254);
	outtextxy(11, 30, messages[23], 254);
	outtextxy(9, 30, messages[23], 254);

	outtextxy(10, 51, messages[24], 254);
	outtextxy(10, 49, messages[24], 254);
	outtextxy(11, 50, messages[24], 254);
	outtextxy(9, 50, messages[24], 254);

	outtextxy(10, 71, messages[25], 254);
	outtextxy(10, 69, messages[25], 254);
	outtextxy(11, 70, messages[25], 254);
	outtextxy(9, 70, messages[25], 254);

	outtextxy(10, 30, messages[23], 255);
	outtextxy(10, 50, messages[24], 255);
	outtextxy(10, 70, messages[25], 255);

	for (int i = 0; i < 32; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}

	delay(10000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 32; i > 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);

	if (!fich.open("SACRIFIC.PAN")) {
		showError(318);
	}
	fich.read(palaux, 768);

	for (int i = 0; i < 256; i++) {
		palaux[i * 3 + 0] = palaux[i * 3 + 0] << 2;
		palaux[i * 3 + 1] = palaux[i * 3 + 1] << 2;
		palaux[i * 3 + 2] = palaux[i * 3 + 2] << 2;
	}

	fich.read(background, 64000);
	fich.close();
	drawFullScreen(background);

	palaux[0] = 0;
	palaux[1] = 0;
	palaux[2] = 0;

	redFadeIn(palaux);

	drawFlc(112, 57, offsets[18], 33, 9, 20, true, false, true, pulsada_salida);
	autoPlayVoc("REZOS", 0, 0);
	if (g_engine->shouldQuit())
		return;

	drawFlc(42, 30, offsets[19], 0, 9, 27, false, false, false, pulsada_salida);

	if (g_engine->shouldQuit())
		return;

	totalFadeOut(128);
	stopVoc();
	clear();

	outtextxy(10, 21, messages[26], 254);
	outtextxy(10, 19, messages[26], 254);
	outtextxy(11, 20, messages[26], 254);
	outtextxy(9, 20,  messages[26], 254);

	outtextxy(10, 41, messages[27], 254);
	outtextxy(10, 39, messages[27], 254);
	outtextxy(11, 40, messages[27], 254);
	outtextxy(9, 40,  messages[27], 254);

	outtextxy(10, 61, messages[28], 254);
	outtextxy(10, 59, messages[28], 254);
	outtextxy(11, 60, messages[28], 254);
	outtextxy(9, 60,  messages[28], 254);

	outtextxy(10, 81, messages[29], 254);
	outtextxy(10, 79, messages[29], 254);
	outtextxy(11, 80, messages[29], 254);
	outtextxy(9, 80,  messages[29], 254);

	outtextxy(10, 101, messages[30], 254);
	outtextxy(10, 99,  messages[30], 254);
	outtextxy(11, 100, messages[30], 254);
	outtextxy(9, 100,  messages[30], 254);

	outtextxy(10, 121, messages[31], 254);
	outtextxy(10, 119, messages[31], 254);
	outtextxy(11, 120, messages[31], 254);
	outtextxy(9, 120,  messages[31], 254);

	outtextxy(10, 141, messages[32], 254);
	outtextxy(10, 139, messages[32], 254);
	outtextxy(11, 140, messages[32], 254);
	outtextxy(9, 140,  messages[32], 254);

	if (!isSpanish) {
		outtextxy(10, 161, messages[56], 254);
		outtextxy(10, 159, messages[56], 254);
		outtextxy(11, 160, messages[56], 254);
		outtextxy(9, 160,  messages[56], 254);

		outtextxy(10, 181, messages[57], 254);
		outtextxy(10, 179, messages[57], 254);
		outtextxy(11, 180, messages[57], 254);
		outtextxy(9,  180,  messages[57], 254);
	}

	outtextxy(10, 20, messages[26], 255);
	outtextxy(10, 40, messages[27], 255);
	outtextxy(10, 60, messages[28], 255);
	outtextxy(10, 80, messages[29], 255);
	outtextxy(10, 100, messages[30], 255);
	outtextxy(10, 120, messages[31], 255);
	outtextxy(10, 140, messages[32], 255);

	if (!isSpanish) {
		outtextxy(10, 160, messages[56], 255);
		outtextxy(10, 180, messages[57], 255);
	}

	for (int i = 0; i <= 31; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(16000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 31; i >= 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);
	if (g_engine->shouldQuit())
		return;

	clear();

	outtextxy(10, 21, messages[33], 254);
	outtextxy(10, 19, messages[33], 254);
	outtextxy(11, 20, messages[33], 254);
	outtextxy(9, 20, messages[33], 254);

	outtextxy(10, 41, messages[34], 254);
	outtextxy(10, 39, messages[34], 254);
	outtextxy(11, 40, messages[34], 254);
	outtextxy(9, 40, messages[34], 254);

	outtextxy(10, 61, messages[35], 254);
	outtextxy(10, 59, messages[35], 254);
	outtextxy(11, 60, messages[35], 254);
	outtextxy(9, 60, messages[35], 254);
	if (isSpanish) {
		outtextxy(10, 81, messages[36], 254);
		outtextxy(10, 79, messages[36], 254);
		outtextxy(11, 80, messages[36], 254);
		outtextxy(9, 80, messages[36], 254);

		outtextxy(10, 101, messages[37], 254);
		outtextxy(10, 99, messages[37], 254);
		outtextxy(11, 100, messages[37], 254);
		outtextxy(9, 100, messages[37], 254);

		outtextxy(10, 121, messages[38], 254);
		outtextxy(10, 119, messages[38], 254);
		outtextxy(11, 120, messages[38], 254);
		outtextxy(9, 120, messages[38], 254);

		outtextxy(10, 141, messages[39], 254);
		outtextxy(10, 139, messages[39], 254);
		outtextxy(11, 140, messages[39], 254);
		outtextxy(9, 140, messages[39], 254);
	}

	outtextxy(10, 20, messages[33], 255);
	outtextxy(10, 40, messages[34], 255);
	outtextxy(10, 60, messages[35], 255);
	if (isSpanish) {
		outtextxy(10, 80, messages[36], 255);
		outtextxy(10, 100, messages[37], 255);
		outtextxy(10, 120, messages[38], 255);
		outtextxy(10, 140, messages[39], 255);
	}

	for (int i = 0; i < 32; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(13000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 32; i > 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);
	if (g_engine->shouldQuit())
		return;
	clear();

	outtextxy(10, 61, messages[40], 254);
	outtextxy(10, 59, messages[40], 254);
	outtextxy(11, 60, messages[40], 254);
	outtextxy(9, 60, messages[40], 254);

	outtextxy(10, 81, messages[41], 254);
	outtextxy(10, 79, messages[41], 254);
	outtextxy(11, 80, messages[41], 254);
	outtextxy(9, 80, messages[41], 254);

	outtextxy(10, 101, messages[42], 254);
	outtextxy(10, 99, messages[42], 254);
	outtextxy(11, 100, messages[42], 254);
	outtextxy(9, 100, messages[42], 254);

	outtextxy(10, 60, messages[40], 255);
	outtextxy(10, 80, messages[41], 255);
	outtextxy(10, 100, messages[42], 255);

	for (int i = 0; i < 32; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(7000);
	if (g_engine->shouldQuit())
		return;
	for (int i = 32; i > 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);
	totalFadeOut(0);
	currentRoomData->paletteAnimationFlag = pulsada_salida;
	saveAllowed = true;
}

void ending() {
	saveAllowed = false;
	bool pulsada_salida;

	const char *const *messages = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];
	const long *offsets = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0] : flcOffsets[1];

	outtextxy(10, 41, messages[43], 249);
	outtextxy(10, 39, messages[43], 249);
	outtextxy(11, 40, messages[43], 249);
	outtextxy(9, 40,  messages[43], 249);

	outtextxy(10, 61, messages[44], 249);
	outtextxy(10, 59, messages[44], 249);
	outtextxy(11, 60, messages[44], 249);
	outtextxy(9, 60,  messages[44], 249);

	outtextxy(10, 40, messages[43], 253);
	outtextxy(10, 60, messages[44], 253);
	if(g_engine->shouldQuit()) {
		return;
	}
	delay(4000);
	totalFadeOut(0);
	clear();
	if(g_engine->shouldQuit()) {
		return;
	}
	lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	playMidiFile("SACRIFIC", true);
	restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	drawFlc(0, 0, offsets[30], 12, 9, 26, true, false, false, pulsada_salida);
	if(pulsada_salida){
		return;
	}
	drawFlc(0, 0, offsets[31], 0, 9, 0, false, false, false, pulsada_salida);
	if(pulsada_salida){
		return;
	}
	delay(1000);
	playVoc("NOOO", 0, 0);
	delay(3000);
	saveAllowed = true;
}

void loadBat() {
	Common::File fichcani;

	animacion2 = true;
	if (!fichcani.open("MURCIE.DAT")) {
		showError(265);
	}
	sizeanimado = fichcani.readUint16LE();
	secondaryAnimationFrameCount = fichcani.readByte();
	numerodir = fichcani.readByte();
	pasoanimado = (byte *)malloc(sizeanimado);
	loadAnimationForDirection(&fichcani, 0);
	fichcani.close();
}

void loadDevil() {
	Common::File fichcani;

	animacion2 = true;
	if (!fichcani.open("ROJOMOV.DAT")) {
		showError(265);
	}
	sizeanimado = fichcani.readUint16LE();
	secondaryAnimationFrameCount = fichcani.readByte();
	numerodir = fichcani.readByte();
	pasoanimado = (byte *)malloc(sizeanimado);
	if (numerodir != 0) {
		secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&fichcani, i);
		}
	}
	fichcani.close();
}

void assembleCompleteBackground(byte *image, uint coordx, uint coordy) {
	uint16 w, h;
	uint16 wFondo;
	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	wFondo = READ_LE_UINT16(background);

	wFondo++;
	w++;
	h++;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int color = image[4 + j * w + i];
			if (color != 0) {
				background[4 + (coordy + j) * wFondo + (coordx + i)] = color;
			}
		}
	}
}

/**
 * Assemble the screen for scroll assembles only the objects because scrolling screens
 * never have secondary animations and character animation is assembled elsewhere.
 */
void assembleScreen(bool scroll) {

	for (int indice = 0; indice < nivelesdeprof; indice++) {
		if (screenObjects[indice] != NULL) {
			assembleCompleteBackground(screenObjects[indice], depthMap[indice].posx, depthMap[indice].posy);
		}
		if (!scroll && secuencia.profundidad == indice) {
			assembleCompleteBackground(secuencia.bitmap[direccionmovimiento][iframe], characterPosX, characterPosY);
		}
		if (!scroll && currentRoomData->animationFlag && animado.profundidad == indice) {
			assembleCompleteBackground(pasoanimado, animado.posx, animado.posy);
		}
	}
}

void disableSecondAnimation() {
	setRoomTrajectories(altoanimado, anchoanimado, RESTORE);
	currentRoomData->animationFlag = false;
	freeAnimation();
	handPantallaToBackground();
	assembleScreen();
}

// Debug
void drawMouseGrid(RoomFileRegister *screen) {
	for (int i = 0; i < 39; i++) {
		for (int j = 0; j < 27; j++) {
			int color = screen->mouseGrid[i][j];
			if (color != 0) {
				int startX = i * factorx + 7;
				int startY = j * factory;
				for (int i2 = 0; i2 < factorx; i2 += 2) {
					for (int j2 = 0; j2 < factory; j2++) {
						int absPixel = startY + j2;
						int offsetX = (absPixel % 2 == 0) ? 1 : 0;
						if (offsetX < startX + factorx && color != 0) {
							*(byte *)g_engine->_screen->getBasePtr(startX + i2 + offsetX, startY + j2) = 255 - color;
						}
					}
				}
			}
		}
	}
	g_engine->_screen->markAllDirty();
}

void drawScreenGrid(RoomFileRegister *screen) {
	for (int i = 0; i < 39; i++) {
		for (int j = 0; j < 27; j++) {
			int color = screen->rejapantalla[i][j];
			if (color != 0) {
				int startX = i * factorx + 7;
				int startY = j * factory;
				for (int i2 = 0; i2 < factorx; i2 += 2) {
					for (int j2 = 0; j2 < factory; j2++) {
						int absPixel = startY + j2;
						int offsetX = (absPixel % 2 == 0) ? 1 : 0;
						if (offsetX < startX + factorx && color != 0) {
							*(byte *)g_engine->_screen->getBasePtr(startX + i2 + offsetX, startY + j2) = 255 - color;
						}
					}
				}
			}
		}
	}
	g_engine->_screen->markAllDirty();
}

void drawPos(uint x, uint y, byte color) {
	if (x < 320 && x > 0 && y > 0 && y < 200)
		*(byte *)g_engine->_screen->getBasePtr(x, y) = color;

	g_engine->_screen->addDirtyRect(Common::Rect(x, y, x + 1, y + 1));
	g_engine->_screen->markAllDirty();
}

void drawLine(int x, int y, int x2, int y2, byte color) {
	g_engine->_screen->drawLine(x, y, x2, y2, color);
}

void drawX(int x, int y, byte color) {
	if (x > 0 && y > 0)
		*(byte *)g_engine->_screen->getBasePtr(x, y) = color;
	if (x - 1 > 0 && y - 1 > 0)
		*(byte *)g_engine->_screen->getBasePtr(x - 1, y - 1) = color;
	if (x - 1 > 0 && y + 1 < 140)
		*(byte *)g_engine->_screen->getBasePtr(x - 1, y + 1) = color;
	if (x + 1 < 320 && y + 1 < 140)
		*(byte *)g_engine->_screen->getBasePtr(x + 1, y + 1) = color;
	if (x + 1 < 320 && y - 1 > 0)
		*(byte *)g_engine->_screen->getBasePtr(x + 1, y - 1) = color;
}

void drawCharacterPosition() {
	drawX(characterPosX, characterPosY, 210);
	drawX(characterPosX + rectificacionx, characterPosY + rectificaciony, 218);
}

void drawRect(byte color, int x, int y, int x2, int y2) {
	rectangle(x, y, x2, y2, color);
}

void printPos(int x, int y, int screenPosX, int screenPosY, const char *label) {
	g_engine->_graphics->restoreBackground(screenPosX, screenPosY, screenPosX + 100, screenPosY + 10);
	g_engine->_graphics->euroText(Common::String::format("%s: %d, %d", label, x, y), screenPosX, screenPosY, Graphics::kTextAlignLeft);
}

void drawGrid() {
	int horizontal = 320 / factorx;
	int vertical = 140 / factory;
	for (int i = 0; i < horizontal; i++) {
		int startX = i * factorx;
		g_engine->_screen->drawLine(startX, 0, startX, 140, 200);
	}

	for (int j = 0; j < vertical; j++) {
		int startY = j * factory;
		g_engine->_screen->drawLine(0, startY, 320, startY, 200);
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
}

void setRoomTrajectories(int animationHeight, int animationWidth, TRAJECTORIES_OP op, bool fixGrids) {
	// add to restore the room, subtract to adjust before loading the screen

	if (currentRoomData->animationFlag && currentRoomData->nombremovto != "QQQQQQQQ") {
		for (int i = 0; i < currentRoomData->longtray2; i++) {
			if (op == RESTORE) {
				currentRoomData->tray2[i].x = currentRoomData->tray2[i].x + (animationWidth >> 1);
				currentRoomData->tray2[i].y = currentRoomData->tray2[i].y + animationHeight;
			} else {
				currentRoomData->tray2[i].x = currentRoomData->tray2[i].x - (animationWidth >> 1);
				currentRoomData->tray2[i].y = currentRoomData->tray2[i].y - animationHeight;
			}
		}
		if (fixGrids) {
			for (int i = 0; i < maxrejax; i++) {
				for (int j = 0; j < maxrejay; j++) {
					if (op == RESTORE) {
						currentRoomData->rejapantalla[oldposx + i][oldposy + j] = rejafondomovto[i][j];
						currentRoomData->mouseGrid[oldposx + i][oldposy + j] = rejafondoraton[i][j];
					} else {
						if (rejamascaramovto[i][j] > 0)
							currentRoomData->rejapantalla[oldposx + i][oldposy + j] = rejamascaramovto[i][j];
						if (rejamascararaton[i][j] > 0)
							currentRoomData->mouseGrid[oldposx + i][oldposy + j] = rejamascararaton[i][j];
					}
				}
			}
		}
	}
}

} // End of namespace Tot
