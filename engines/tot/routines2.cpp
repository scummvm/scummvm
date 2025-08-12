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
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "image/png.h"

#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void loadScreenMemory() {
	screenSize = 65520;
	sceneBackground = (byte *)malloc(screenSize);
	backgroundCopy = (byte *)malloc(screenSize);
}

void loadAnimationForDirection(Common::SeekableReadStream *stream, int direction) {
	for (int j = 0; j < secondaryAnimationFrameCount; j++) {
		loadAnimationIntoBuffer(stream, secondaryAnimation.bitmap[direction][j], secondaryAnimFrameSize);
	}
}

void loadAnimation(Common::String animationName) {
	debug("Loading animation!");
	Common::File animFile;

	if (animationName == "PETER")
		isPeterCoughing = true;
	else
		isPeterCoughing = false;

	isSecondaryAnimationEnabled = true;
	if (!animFile.open(Common::Path(animationName + ".DAT"))) {
		showError(265);
	}

	secondaryAnimFrameSize = animFile.readUint16LE();
	secondaryAnimationFrameCount = animFile.readByte();
	secondaryAnimDirCount = animFile.readByte();
	curSecondaryAnimationFrame = (byte *)malloc(secondaryAnimFrameSize);
	if (secondaryAnimDirCount != 0) {

		secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&animFile, i);
		}
	} else {
		loadAnimationForDirection(&animFile, 0);
	}

	animFile.close();
	debug("Read all frames! longtray2=%d", currentRoomData->secondaryTrajectoryLength);
	secondaryAnimWidth = READ_LE_UINT16(secondaryAnimation.bitmap[0][1]) + 1;
	secondaryAnimHeight = READ_LE_UINT16(secondaryAnimation.bitmap[0][1] + 2) + 1;

	setRoomTrajectories(secondaryAnimHeight, secondaryAnimWidth, SET_WITH_ANIM, false);

	readItemRegister(currentRoomData->secondaryAnimDirections[299]);
	maxXGrid = (regobj.xgrid2 - regobj.xgrid1 + 1);
	maxYGrid = (regobj.ygrid2 - regobj.ygrid1 + 1);
	oldposx = regobj.xgrid1 + 1;
	oldposy = regobj.ygrid1 + 1;

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++) {
			maskGridSecondaryAnim[i][j] = 0;
			maskMouseSecondaryAnim[i][j] = 0;
			movementGridForSecondaryAnim[i][j] = 0;
			mouseGridForSecondaryAnim[i][j] = 0;
		}

	for (int i = 0; i < maxXGrid; i++)
		for (int j = 0; j < maxYGrid; j++) {
			maskGridSecondaryAnim[i][j] = regobj.walkAreasPatch[i][j];
			maskMouseSecondaryAnim[i][j] = regobj.mouseGridPatch[i][j];
			movementGridForSecondaryAnim[i][j] = currentRoomData->walkAreasGrid[oldposx + i][oldposy + j];
			mouseGridForSecondaryAnim[i][j] = currentRoomData->mouseGrid[oldposx + i][oldposy + j];
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

	byte currentScreen = currentRoomData->code;

	setRoomTrajectories(secondaryAnimHeight, secondaryAnimWidth, RESTORE);

	// Save current room
	saveRoomData(currentRoomData, rooms);

	// Load other screen
	rooms->seek(otherScreenNumber * roomRegSize, SEEK_SET);
	currentRoomData = readScreenDataFile(rooms);

	switch (otherScreenNumber) {
	case 20: {
		switch (niche[0][niche[0][3]]) {
		case 0: {
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
			currentRoomData->screenLayers[1].bitmapPointer = 1190768;
		} break;
		case 561: {
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
			currentRoomData->screenLayers[1].bitmapPointer = 1182652;
		} break;
		case 563: {
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
			currentRoomData->screenLayers[1].bitmapPointer = 1186044;
		} break;
		case 615: {
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
			currentRoomData->screenLayers[1].bitmapPointer = 1181760;
		} break;
		}
		currentRoomData->screenLayers[1].bitmapSize = 892;
		currentRoomData->screenLayers[1].coordx = 66;
		currentRoomData->screenLayers[1].coordy = 35;
		currentRoomData->screenLayers[1].depth = 1;
	} break;
	case 24: {
		switch (niche[1][niche[1][3]]) {
		case 0: {
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
			currentRoomData->screenLayers[0].bitmapPointer = 1399610;
		} break;
		case 561: {
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
			currentRoomData->screenLayers[0].bitmapPointer = 1381982;
		} break;
		case 615: {
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
			currentRoomData->screenLayers[0].bitmapPointer = 1381090;
		} break;
		case 622: {
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
			currentRoomData->screenLayers[0].bitmapPointer = 1400502;
		} break;
		case 623: {
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
			currentRoomData->screenLayers[0].bitmapPointer = 1398718;
		} break;
		}
		currentRoomData->screenLayers[0].bitmapSize = 892;
		currentRoomData->screenLayers[0].coordx = 217;
		currentRoomData->screenLayers[0].coordy = 48;
		currentRoomData->screenLayers[0].depth = 1;
	} break;
	case 31: {
		for (i11 = 23; i11 <= 25; i11++)
			for (i22 = 4; i22 <= 9; i22++)
				currentRoomData->mouseGrid[i11][i22] = 4;
		for (i11 = 23; i11 <= 25; i11++)
			for (i22 = 10; i22 <= 11; i22++)
				currentRoomData->mouseGrid[i11][i22] = 3;

		currentRoomData->screenLayers[0].bitmapSize = 0;
		currentRoomData->screenLayers[0].bitmapPointer = 0;
		currentRoomData->screenLayers[0].coordx = 0;
		currentRoomData->screenLayers[0].coordy = 0;
		currentRoomData->screenLayers[0].depth = 0;
	} break;
	}

	// Save other screen
	saveRoomData(currentRoomData, rooms);

	// Restore current room again
	rooms->seek(currentScreen * roomRegSize, SEEK_SET);
	currentRoomData = readScreenDataFile(rooms);

	setRoomTrajectories(secondaryAnimHeight, secondaryAnimWidth, SET_WITH_ANIM);
}

void verifyCopyProtection() {
	//TODO: Copy protection
}

void loadTV() {

	Common::File fichct;
	if (!fichct.open("PALETAS.DAT")) {
		showError(310);
	}
	fichct.seek(currentRoomData->palettePointer + 603);
	fichct.read(palAnimSlice, 144);
	for (int i = 0; i <= 48; i++) {
		palAnimSlice[i * 3 + 0] = palAnimSlice[i * 3 + 0] << 2;
		palAnimSlice[i * 3 + 1] = palAnimSlice[i * 3 + 1] << 2;
		palAnimSlice[i * 3 + 2] = palAnimSlice[i * 3 + 2] << 2;
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
	Common::File paletteFile;
	palette palcp;

	screenSize = currentRoomData->roomImageSize;
	readBitmap(currentRoomData->roomImagePointer, sceneBackground, screenSize, 316);
	Common::copy(sceneBackground, sceneBackground + screenSize, backgroundCopy);
	switch (gamePart) {
	case 1: {
		if (!paletteFile.open("PALETAS.DAT")) {
			showError(310);
		}
		paletteFile.seek(currentRoomData->palettePointer);
		paletteFile.read(palcp, 603);
		if (currentRoomData->paletteAnimationFlag) {
			paletteFile.read(palAnimSlice, 144);
			for (int i = 0; i <= 48; i++) {
				palAnimSlice[i * 3 + 0] = palAnimSlice[i * 3 + 0] << 2;
				palAnimSlice[i * 3 + 1] = palAnimSlice[i * 3 + 1] << 2;
				palAnimSlice[i * 3 + 2] = palAnimSlice[i * 3 + 2] << 2;
			}
		}
		paletteFile.close();
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
	cpCounter = cpCounter2;
	if (!characterFile.open("PERSONAJ.SPT"))
		showError(265);

	mainCharFrameSize = characterFile.readUint16LE();

	verifyCopyProtection();

	for (int i = 0; i <= 3; i++)
		for (int j = 0; j < walkFrameCount; j++) {
			mainCharAnimation.bitmap[i][j] = (byte *)malloc(mainCharFrameSize);
			characterFile.read(mainCharAnimation.bitmap[i][j], mainCharFrameSize);
		}
	for (int i = 0; i < 4; i++)
		for (int j = walkFrameCount; j < (walkFrameCount + 10 * 3); j++) {
			mainCharAnimation.bitmap[i][j] = (byte *)malloc(mainCharFrameSize);
			characterFile.read(mainCharAnimation.bitmap[i][j], mainCharFrameSize);
		}
	characterFile.close();
}

void freeScreenObjects() {
	for (int i = 0; i < numScreenOverlays; i++) {
		if (screenLayers[i] != NULL)
			free(screenLayers[i]);
		screenLayers[i] = NULL;
	}
}

void freeAnimation() {
	if (isSecondaryAnimationEnabled) {
		isSecondaryAnimationEnabled = false;
		if(curSecondaryAnimationFrame)
			free(curSecondaryAnimationFrame);
		curSecondaryAnimationFrame = NULL;
	}
}

void freeInventory() {
	for (int i = 0; i < inventoryIconCount; i++) {
		free(inventoryIconBitmaps[i]);
	}
}

void verifyCopyProtection2() {
	// TODO:
}
void loadItemWithFixedDepth(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth) {
	screenLayers[depth] = (byte *)malloc(bitmapSize);
	readBitmap(bitmapIndex, screenLayers[depth], bitmapSize, 319);

	uint16 w, h;
	w = READ_LE_UINT16(screenLayers[depth]);
	h = READ_LE_UINT16(screenLayers[depth] + 2);
	depthMap[depth].posx = coordx;
	depthMap[depth].posy = coordy;
	depthMap[depth].posx2 = coordx + w + 1;
	depthMap[depth].posy2 = coordy + h + 1;
}

void loadItem(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth) {
	loadItemWithFixedDepth(coordx, coordy, bitmapSize, bitmapIndex, depth - 1);
}

void updateInventory(byte index) {
	for (int i = index; i < (inventoryIconCount - 1); i++) {
		inventory[i].bitmapIndex = inventory[i + 1].bitmapIndex;
		inventory[i].code = inventory[i + 1].code;
		inventory[i].objectName = inventory[i + 1].objectName;
	}
	// verifyCopyProtection();
}

void readBitmap(int32 bitmapPosition, byte *buf, uint bitmapSize, uint error) {
	Common::File bitmapFile;
	if (!bitmapFile.open("BITMAPS.DAT")) {
		showError(error);
	}
	bitmapFile.seek(bitmapPosition);
	bitmapFile.read(buf, bitmapSize);

	bitmapFile.close();
}

void updateItem(uint itemPosition) {
	regobj.used[0] = 9;
	invItemData->seek(itemPosition);
	saveItemRegister(regobj, invItemData);
}

void readItemRegister(Common::SeekableReadStream *stream, uint itemPos, ScreenObject &thisRegObj) {
	stream->seek(itemPos * itemRegSize);
	clearObj();
	thisRegObj.code = stream->readUint16LE();
	thisRegObj.height = stream->readByte();

	thisRegObj.name = stream->readPascalString();

	stream->skip(objectNameLength - thisRegObj.name.size());

	thisRegObj.lookAtTextRef = stream->readUint16LE();
	thisRegObj.beforeUseTextRef = stream->readUint16LE();
	thisRegObj.afterUseTextRef = stream->readUint16LE();
	thisRegObj.pickTextRef = stream->readUint16LE();
	thisRegObj.useTextRef = stream->readUint16LE();
	thisRegObj.speaking = stream->readByte();
	thisRegObj.openable = stream->readByte();
	thisRegObj.closeable = stream->readByte();

	stream->read(thisRegObj.used, 8);

	thisRegObj.pickupable = stream->readByte();
	thisRegObj.useWith = stream->readUint16LE();
	thisRegObj.replaceWith = stream->readUint16LE();
	thisRegObj.depth = stream->readByte();
	thisRegObj.bitmapPointer = stream->readUint32LE();
	thisRegObj.bitmapSize = stream->readUint16LE();
	thisRegObj.rotatingObjectAnimation = stream->readUint32LE();
	thisRegObj.rotatingObjectPalette = stream->readUint16LE();
	thisRegObj.dropOverlayX = stream->readUint16LE();
	thisRegObj.dropOverlayY = stream->readUint16LE();
	thisRegObj.dropOverlay = stream->readUint32LE();
	thisRegObj.dropOverlaySize = stream->readUint16LE();
	thisRegObj.objectIconBitmap = stream->readUint16LE();
	thisRegObj.xgrid1 = stream->readByte();
	thisRegObj.ygrid1 = stream->readByte();
	thisRegObj.xgrid2 = stream->readByte();
	thisRegObj.ygrid2 = stream->readByte();
	stream->read(thisRegObj.walkAreasPatch, 100);
	stream->read(thisRegObj.mouseGridPatch, 100);
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

void putIcon(uint iconPosX, uint iconPosY, uint iconNum) {
	// substract 1 to account for 1-based indices
	g_engine->_graphics->putImg(iconPosX, iconPosY, inventoryIconBitmaps[inventory[iconNum].bitmapIndex - 1]);
}

void drawBackpack() {
	putIcon(34, 169, inventoryPosition);
	putIcon(77, 169, inventoryPosition + 1);
	putIcon(120, 169, inventoryPosition + 2);
	putIcon(163, 169, inventoryPosition + 3);
	putIcon(206, 169, inventoryPosition + 4);
	putIcon(249, 169, inventoryPosition + 5);
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

void drawInventory(byte dir, byte max) {
	switch (dir) {
	case 0:
		if (inventoryPosition > 0) {
			inventoryPosition -= 1;
			drawBackpack();
		}
		break;
	case 1:
		if (inventoryPosition < (max - 6)) {
			inventoryPosition += 1;
			drawBackpack();
		}
		break;
	}
	g_engine->_mouseManager->hide();
	if (inventoryPosition > 0)
		lightUpLeft();
	else
		turnOffLeft();
	if (inventory[inventoryPosition + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
	g_engine->_mouseManager->show();
	if (cpCounter > 145)
		showError(274);
}

void mask() {

	buttonBorder(0, 140, 319, 149, 0, 0, 0, 0, 0);
	for (int i = 1; i <= 25; i++)
		buttonBorder(0, (175 - i), 319, (174 + i), 251, 251, 251, 251, 0);
	drawMenu(1);
	// verifyCopyProtection();
	if (inventoryPosition > 1)
		lightUpLeft();
	else
		turnOffLeft();
	if (inventory[inventoryPosition + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
}

void drawMenu(byte menuNumber) {
	byte *bitmap;
	uint menuSize;
	byte xmenu, ymenu;
	long menuOffset;

	Common::File menuFile;
	if (!menuFile.open("MENUS.DAT")) {
		showError(258);
	}

	menuOffset = g_engine->_lang == Common::ES_ESP ? menuOffsets_ES[menuNumber - 1][0] : menuOffsets_EN[menuNumber - 1][0];
	menuSize = g_engine->_lang == Common::ES_ESP ? menuOffsets_ES[menuNumber - 1][1] : menuOffsets_EN[menuNumber - 1][1];

	switch (menuNumber) {
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
		if (cpCounter2 > 20)
			showError(274);
		xmenu = 50;
		ymenu = 10;
	} break;
	case 5: {
		if (cpCounter > 23)
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

	bitmap = (byte *)malloc(menuSize);
	menuFile.seek(menuOffset);
	menuFile.read(bitmap, menuSize);
	g_engine->_graphics->putImg(xmenu, ymenu, bitmap);
	free(bitmap);
	menuFile.close();
}

void generateDiploma(Common::String &photoName);

static void loadDiploma(Common::String &photoName, Common::String &key) {
	palette auxPal;
	byte *screen;
	uint size;
	byte *stamp;

	Common::File dipFile;
	if (!dipFile.open("DIPLOMA.PAN")) {
		showError(318);
	}

	dipFile.read(auxPal, 768);

	screen = (byte *)malloc(64000);
	dipFile.read(screen, 64000);
	dipFile.close();

	if (!dipFile.open(Common::Path("DIPLOMA/SELLO.BMP")))
		showError(271);
	stamp = (byte *)malloc(2054);
	dipFile.read(stamp, 2054);
	dipFile.close();
	drawFullScreen(screen);

	free(screen);

	if (dipFile.open(Common::Path("DIPLOMA/" + photoName + ".FOT"))) {
		size = dipFile.size() - 768;
		screen = (byte *)malloc(size);
		dipFile.read(pal, 768);
		dipFile.read(screen, size);
		dipFile.close();
		g_engine->_graphics->putShape(10, 20, screen);
		free(screen);
	}
	for (int i = 16; i <= 255; i++) {
		auxPal[i * 3 + 0] = pal[i * 3 + 0];
		auxPal[i * 3 + 1] = pal[i * 3 + 1];
		auxPal[i * 3 + 2] = pal[i * 3 + 2];
	}

	copyPalette(auxPal, pal);
	g_engine->_graphics->fixPalette(pal, 768);
	g_engine->_graphics->setPalette(pal);
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();

	char *passArray = (char *)malloc(10);
	for (int i = 0; i < 10; i++)
		passArray[i] = (char)(Random(10) + 48);

	key.append(passArray, passArray + 10);

	const char *const *messages = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];
	biosText(91, 16, messages[49] + key, 255);
	biosText(90, 15,  messages[49] + key, 13);

	biosText(81, 61,  messages[50], 0);
	biosText(61, 81,  messages[51], 0);
	biosText(31, 101, messages[52] + characterName, 0);
	biosText(31, 121, messages[53], 0);
	biosText(31, 141, messages[54], 0);
	biosText(31, 161, messages[55], 0);

	biosText(80, 60,  messages[50], 15);
	biosText(60, 80,  messages[51], 15);
	biosText(30, 100, messages[52], 15);

	biosText(150, 100, characterName, 13);

	biosText(30, 120, messages[53], 15);
	biosText(30, 140, messages[54], 15);
	biosText(30, 160, messages[55], 15);
	delay(1500);
	g_engine->_sound->playVoc("PORTAZO", 434988, 932);
	g_engine->_graphics->putShape(270, 159, stamp);
	free(stamp);
}

static void saveDiploma(Common::String &photoName, Common::String &key) {
	Common::String name;
	if (photoName != "")
		name = "tot-diploma-" + photoName + ".png";
	else
		"tot-diploma-default.png";
	Common::OutSaveFile *thumbnail = g_engine->getSaveFileManager()->openForSaving(name);
	Graphics::Surface *surface = g_system->lockScreen();
	assert(surface);
	Image::writePNG(*thumbnail, *surface, g_engine->_graphics->getPalette());
	g_system->unlockScreen();
	thumbnail->finalize();
	delete thumbnail;
}

void generateDiploma(Common::String &photoName) {
	Common::String key;
	g_engine->_mouseManager->hide();
	photoName.toUppercase();
	totalFadeOut(0);
	loadDiploma(photoName, key);

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
	saveDiploma(photoName, key);
	g_engine->_mouseManager->show();
}

void checkMouseGrid() {
	uint xGrid, yGrid;
	Common::String invObject;
	if (cpCounter2 > 120)
		showError(274);
	if (mouseY >= 0 && mouseY <= 131) {
		xGrid = (mouseX + 7) / xGridCount;
		yGrid = (mouseY + 7) / yGridCount;
		if (currentRoomData->mouseGrid[xGrid][yGrid] != currentRoomData->mouseGrid[oldGridX][oldGridY] || oldInventoryObjectName != "") {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (actionCode) {
			case 0:
				actionLine = getActionLineText(0) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 1:
				actionLine = getActionLineText(1) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 2:
				actionLine = getActionLineText(2) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 3:
				actionLine = getActionLineText(3) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 4:
				if (inventoryObjectName != "")
					actionLine = getActionLineText(4) + inventoryObjectName + getActionLineText(7) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				else
					actionLine = getActionLineText(4) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 5:
				actionLine = getActionLineText(5) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 6:
				actionLine = getActionLineText(6) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			default:
				actionLine = getActionLineText(0) + currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
			}
			actionLineText(actionLine);
			g_engine->_mouseManager->show();
			oldGridX = xGrid;
			oldGridY = yGrid;
		}
		oldActionCode = 253;
		oldInventoryObjectName = "";
	} else if (mouseY >= 132 && mouseY <= 165) {
		if (actionCode != oldActionCode) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (actionCode) {
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
				if (inventoryObjectName != "")
					actionLine = getActionLineText(4) + inventoryObjectName + getActionLineText(7);
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
			oldActionCode = actionCode;
			oldInventoryObjectName = "";
			oldGridX = 0;
			oldGridY = 0;
		}
	} else if (mouseY >= 166 && mouseY <= 199) {
		if (mouseX >= 26 && mouseX <= 65) {
			invObject = inventory[inventoryPosition].objectName;
		} else if (mouseX >= 70 && mouseX <= 108) {
			invObject = inventory[inventoryPosition + 1].objectName;
		} else if (mouseX >= 113 && mouseX <= 151) {
			invObject = inventory[inventoryPosition + 2].objectName;
		} else if (mouseX >= 156 && mouseX <= 194) {
			invObject = inventory[inventoryPosition + 3].objectName;
		} else if (mouseX >= 199 && mouseX <= 237) {
			invObject = inventory[inventoryPosition + 4].objectName;
		} else if (mouseX >= 242 && mouseX <= 280) {
			invObject = inventory[inventoryPosition + 5].objectName;
		} else {
			invObject = ' ';
		}

		if (invObject != oldInventoryObjectName || oldGridX != 0) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (actionCode) {
			case 1:
				actionLine = getActionLineText(1) + invObject;
				break;
			case 2:
				actionLine = getActionLineText(2) + invObject;
				break;
			case 3:
				actionLine = getActionLineText(3) + invObject;
				break;
			case 4:
				if (inventoryObjectName == "")
					actionLine = getActionLineText(4) + invObject;
				else
					actionLine = getActionLineText(4) + inventoryObjectName + getActionLineText(7) + invObject;
				break;
			case 5:
				actionLine = getActionLineText(5) + invObject;
				break;
			case 6:
				actionLine = getActionLineText(6) + invObject;
				break;
			default:
				euroText(160, 144, invObject, 255, Graphics::kTextAlignCenter);
			}
			actionLineText(actionLine);
			g_engine->_mouseManager->show();
			oldInventoryObjectName = invObject;
		}
		oldActionCode = 255;
		oldGridX = 0;
		oldGridY = 0;
	}
}

void readAlphaGraph(Common::String &output, int length, int posx, int posy, byte barColor) {
	int pun = 1;
	bar(posx, posy - 2, posx + length * 8, posy + 8, barColor);

	biosText(posx, posy, "_", 0);

	Common::Event e;
	bool done = false;
	while (!done && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {

			if (e.type == Common::EVENT_KEYDOWN) {
				int keycode = e.kbd.keycode;
				int asciiCode = e.kbd.ascii;
				// ENTER key
				if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_KP_ENTER) {
					if (output.size() > 0) {
						done = true;
						continue;
					}
				}
				// Max 8 chars
				if (pun > length && asciiCode != 8) {
					g_engine->_sound->beep(750, 60);
					bar((posx + (output.size()) * 8), (posy - 2), (posx + (output.size() + 1) * 8), (posy + 8), 0);
				} else if (asciiCode == 8 && pun > 1) { // delete
					output = output.substr(0, output.size() - 1);
					bar(posx, (posy - 2), (posx + length * 8), (posy + 8), barColor);
					biosText(posx, posy, output.c_str(), 0);
					biosText((posx + (output.size()) * 8), posy, "_", 0);
					pun -= 1;
				} else if (
					(asciiCode < 97 || asciiCode > 122) &&
					(asciiCode < 65 || asciiCode > 90) &&
					(asciiCode < 32 || asciiCode > 57) &&
					(asciiCode < 164 || asciiCode > 165)) {
					g_engine->_sound->beep(1200, 60);
				} else {
					pun += 1;
					output = output + (char)e.kbd.ascii;
					bar(posx, (posy - 2), (posx + length * 8), (posy + 8), barColor);
					biosText(posx, posy, output.c_str(), 0);
					biosText((posx + (output.size()) * 8), posy, "_", 0);
				}
			}
		}
		g_system->delayMillis(10);
		g_engine->_screen->update();
	}
}

void readAlphaGraphSmall(Common::String &output, int length, int posx, int posy, byte barColor,
						 byte textColor) {
	int pun = 1;
	bool borracursor;
	bar(posx, posy + 2, posx + length * 6, posy + 9, barColor);

	biosText(posx, posy, "-", textColor);
	Common::Event e;
	bool done = false;

	while (!done && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				int keycode = e.kbd.keycode;
				int asciiCode = e.kbd.ascii;
				// ENTER key
				if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_KP_ENTER) {
					if (output.size() > 0) {
						done = true;
						continue;
					}
				}

				if (pun > length && asciiCode != 8) {
					g_engine->_sound->beep(750, 60);
					bar((posx + (output.size()) * 6), (posy + 2), (posx + (output.size() + 1) * 6), (posy + 9), barColor);
				} else if (asciiCode == 8 && pun > 1) {
					output = output.substr(0, output.size() - 1);
					bar(posx, (posy + 2), (posx + length * 6), (posy + 9), barColor);
					biosText(posx, posy, output, textColor);
					biosText((posx + (output.size()) * 6), posy, "-", textColor);
					pun -= 1;
					borracursor = true;
				} else if ((asciiCode < '\40') || (asciiCode > '\373')) {
					g_engine->_sound->beep(1200, 60);
					borracursor = false;
				} else {
					pun += 1;
					output = output + (char)e.kbd.ascii;
					bar(posx, (posy + 2), (posx + length * 6), (posy + 9), barColor);
					littText(posx, posy, output, textColor);
					littText((posx + (output.size()) * 6), posy, "-", textColor);
					borracursor = true;
				}
			}
		}

		g_system->delayMillis(10);
		g_engine->_screen->update();
	}

	if (borracursor)
		bar(posx + (output.size()) * 6, posy + 2, (posx + (output.size()) * 6) + 6, posy + 9, barColor);
}

void hyperText(
	Common::String textString,
	uint xpos,
	uint ypos,
	byte maxWidth,
	byte textColor,
	byte shadowColor) {

	byte ihc, lineCounter;
	byte newLineMatrix[10];

	if (textString.size() < maxWidth) {
		euroText((xpos + 1), (ypos + 1), textString, shadowColor);
		g_engine->_screen->update();
		delay(kEnforcedTextAnimDelay);
		euroText(xpos, ypos, textString, textColor);
		g_engine->_screen->update();
		delay(kEnforcedTextAnimDelay);
	} else {
		ihc = 0;
		lineCounter = 0;
		newLineMatrix[0] = 0;
		do {
			ihc += maxWidth + 1;
			lineCounter += 1;
			do {
				ihc -= 1;
			} while (textString[ihc] != ' ');
			newLineMatrix[lineCounter] = ihc + 1;
		} while (ihc + 1 <= textString.size() - maxWidth);

		lineCounter += 1;
		newLineMatrix[lineCounter] = textString.size();

		for (byte line = 1; line <= lineCounter; line++) {
			Common::String lineString = textString.substr(newLineMatrix[line - 1], newLineMatrix[line] - newLineMatrix[line - 1]);
			euroText((xpos + 1), (ypos + ((line - 1) * 11) + 1), lineString, shadowColor);
			g_engine->_screen->update();
			delay(kEnforcedTextAnimDelay);
			euroText(xpos, (ypos + ((line - 1) * 11)), lineString, textColor);
			g_engine->_screen->update();
			delay(kEnforcedTextAnimDelay);
		}
	}
}

void buttonBorder(uint x1, uint y1, uint x2, uint y2,
				  byte color1, byte color2, byte color3, byte color4, byte color5) {

	bar(x1, y1, x2, y2, color4);
	line(x1, y1, x1, y2, color1);
	line(x1, y1, x2, y1, color1);

	line(x2, y1, x2, y2, color2);
	line(x2, y2, x1, y2, color2);

	putpixel(x2, y1, color3);
	putpixel(x1, y2, color3);

	g_engine->_screen->addDirtyRect(Common::Rect(
		x1, y1, x2, y2));
	g_engine->_screen->update();
}

void copyProtection() {
	//todo
}

void credits();

void drawCreditsScreen(byte *&backgroundPointer, uint &sizeAuxBG, byte *&auxBG) {
	palette intermediatePalette, darkPalette;

	Common::File ppFile;

	if (!ppFile.open("DIPLOMA.PAN")) {
		showError(315);
	}
	backgroundPointer = (byte *)malloc(64000);
	ppFile.read(intermediatePalette, 768);
	ppFile.read(backgroundPointer, 64000);
	ppFile.close();

	drawFullScreen(backgroundPointer);

	sizeAuxBG = imagesize(0, 0, 319, 59);

	// Screen is now backgroundPointer so auxBG contains a 320x60 crop of backgroundPointer
	auxBG = (byte *)malloc(sizeAuxBG);
	g_engine->_graphics->getImg(0, 0, 319, 59, auxBG);

	for (int i = 0; i <= 255; i++) {
		darkPalette[i * 3 + 0] = 0;
		darkPalette[i * 3 + 1] = 0;
		darkPalette[i * 3 + 2] = 0;
		// Adjust for 6-bit DAC color
		intermediatePalette[i * 3 + 0] = intermediatePalette[i * 3 + 0] << 2;
		intermediatePalette[i * 3 + 1] = intermediatePalette[i * 3 + 1] << 2;
		intermediatePalette[i * 3 + 2] = intermediatePalette[i * 3 + 2] << 2;
	}

	changePalette(darkPalette, intermediatePalette);
	copyPalette(intermediatePalette, pal);
	if (cpCounter2 > 9)
		showError(274);
}

void putCreditsImg(uint x, uint y, byte *img1, byte *img2, bool direct) {

	uint16 wImg1, hImg1;
	uint horizontalAux;
	uint inc, inc2;
	byte *step;

	wImg1 = READ_LE_UINT16(img1);
	hImg1 = READ_LE_UINT16(img1 + 2);

	step = (byte *)malloc((wImg1 + 1) * (hImg1 + 1) + 4);

	horizontalAux = wImg1 + 1;
	foo = hImg1 + y;

	// makes sure that if the image is at the bottom of the screen we chop the bottom part
	for (int i = foo; i >= 200; i--)
		hImg1 -= 1;

	hImg1++;

	// Copies the crop in the background corresponding to the current credit window in img1
	for (int i = 0; i < hImg1; i++) {
		byte *src = img2 + (320 * (y + i)) + x;
		byte *dst = step + 4 + (horizontalAux * i);
		Common::copy(src, src + horizontalAux, dst);
	}

	for (int kk = 0; kk < hImg1; kk++) {
		inc2 = (kk * wImg1) + 4;
		foo = kk + y;
		for (int jj = 0; jj <= wImg1; jj++) {
			inc = inc2 + jj;
			if ((direct && img1[inc] > 0) || (img1[inc] > 16 && foo >= 66 && foo <= 192)) {
				step[inc] = img1[inc];
			} else if (img1[inc] > 16) {
				switch (foo) {
				case 59:
				case 199:
					step[inc] = img1[inc] + 210;
					break;
				case 60:
				case 198:
					step[inc] = img1[inc] + 180;
					break;
				case 61:
				case 197:
					step[inc] = img1[inc] + 150;
					break;
				case 62:
				case 196:
					step[inc] = img1[inc] + 120;
					break;
				case 63:
				case 195:
					step[inc] = img1[inc] + 90;
					break;
				case 64:
				case 194:
					step[inc] = img1[inc] + 60;
					break;
				case 65:
				case 193:
					step[inc] = img1[inc] + 30;
					break;
				}
			}
		}
	}
	// Wait until render tick
	do {
		g_engine->_chrono->updateChrono();
		g_system->delayMillis(10);
	} while (!gameTick && !g_engine->shouldQuit());
	gameTick = false;

	// Copies the credit window directly to the screen
	for (int i = 0; i < hImg1; i++) {
		byte *src = step + 4 + (horizontalAux * i);
		byte *dst = ((byte *)g_engine->_screen->getPixels()) + (320 * (y + i)) + x;
		Common::copy(src, src + horizontalAux, dst);
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x, y, x + wImg1 + 1, y + hImg1 + 1));
	free(step);
}

void scrollCredit(
	int32 position,
	uint size,
	palette &pal2,
	byte *&background,
	bool &exit,
	int minHeight,
	bool withFade,
	bool refresh) {
	Common::File creditFile;
	if (!creditFile.open("CREDITOS.DAT")) {
		showError(270);
	}
	creditFile.seek(position);
	creditFile.read(sceneBackground, size);
	creditFile.read(pal2, 768);
	creditFile.close();

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
		putCreditsImg(85, i, sceneBackground, background, !withFade);
		if (keyPressed) {
			exit = true;
			break;
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
		if (g_engine->shouldQuit())
			break;
	}
	if (refresh) {
		copyFromScreen(background);
	}
}

void scrollSingleCredit(
	int32 pos,
	uint size,
	palette &pal2,
	byte *&background,
	bool &exit) {
	scrollCredit(
		pos,
		size,
		pal2,
		background,
		exit,
		8,
		true,
		false);
}

void removeTitle(byte *&background2) {
	uint i2, j2;
	Common::Event e;
	for (int i1 = 1; i1 <= 15000; i1++) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		i2 = Random(318);
		j2 = Random(58);
		byte *src = background2 + 4 + (j2 * 320) + i2;
		byte *dest = ((byte *)g_engine->_screen->getPixels()) + (j2 * 320) + i2;
		Common::copy(src, src + 2, dest);

		byte *src2 = background2 + 4 + ((j2 + 1) * 320) + i2;
		byte *dest2 = ((byte *)g_engine->_screen->getPixels()) + ((j2 + 1) * 320) + i2;

		Common::copy(src2, src2 + 2, dest2);

		i2 = Random(320);
		j2 = Random(60);

		byte *src3 = background2 + 4 + (j2 * 320) + i2;
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
	Common::String n = "Gabriel";
	generateDiploma(n);
	saveAllowed = true;
	debug("Credits");
	palette pal2;
	byte *background;
	byte *background2;
	uint sizeBg2;
	bool exit;

	g_engine->_mouseManager->hide();
	totalFadeOut(0);
	g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
	g_engine->_screen->clear();
	g_engine->_sound->playMidi("CREDITOS", true);
	g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
	drawCreditsScreen(background, sizeBg2, background2);

	exit = false;

	if (keyPressed() || exit)
		goto Lexit;
	scrollCredit(0, 8004, pal2, background, exit, 10, false, true);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(8772, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(17544, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(26316, 7504, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(34588, 7504, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(42860, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(51632, 7504, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	removeTitle(background2);
	if (keyPressed() || exit)
		goto Lexit;
	g_engine->_graphics->putImg(0, 0, background2);
	if (keyPressed() || exit)
		goto Lexit;
	copyFromScreen(background);
	if (keyPressed() || exit)
		goto Lexit;
	scrollCredit(59904, 8004, pal2, background, exit, 10, false, true);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(68676, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(77448, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(86220, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(94992, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(103764, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	scrollSingleCredit(112536, 8004, pal2, background, exit);
	if (keyPressed() || exit)
		goto Lexit;
	removeTitle(background2);
	if (keyPressed() || exit)
		goto Lexit;
	g_engine->_graphics->putImg(0, 0, background2);
	if (keyPressed() || exit)
		goto Lexit;
	copyFromScreen(background);
	if (keyPressed() || exit)
		goto Lexit;
	scrollCredit(121308, 8004, pal2, background, exit, 80, false, true);
Lexit:
	delay(1000);
	totalFadeOut(0);
	g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
	g_engine->_screen->clear();
	g_engine->_sound->playMidi("INTRODUC", true);
	g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
	g_engine->_mouseManager->show();
	free(background);
	free(background2);
}

void introduction() {
	saveAllowed = false;
	g_engine->_mouseManager->hide();
	bool exitPressed;
	uint loopCount;
	bool isSpanish = g_engine->_lang == Common::ES_ESP;
	const char *const *messages = (isSpanish) ? fullScreenMessages[0] : fullScreenMessages[1];
	const long *offsets = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0] : flcOffsets[1];
	exitPressed = false;
	totalFadeOut(0);

	if (cpCounter > 6)
		showError(270);
	g_engine->_screen->clear();
	drawFlc(136, 53, offsets[2], 136, 9, 1, true, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(135, 54, offsets[3], 0, 9, 2, true, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	totalFadeOut(0);
	g_engine->_screen->clear();
	if(isSpanish) {
		littText(25,  20, messages[0], 253);
		littText(25,  35, messages[1], 253);
		littText(25,  50, messages[2], 253);
		littText(25,  65, messages[3], 253);
		littText(25,  80, messages[4], 253);
		littText(25,  95, messages[5], 253);
		littText(25, 120, messages[6], 253);
		littText(25, 140, messages[7], 253);
		littText(25, 155, messages[8], 253);
	}
	else {
		littText(25,  35, messages[0], 253);
		littText(25,  55, messages[1], 253);
		littText(25,  75, messages[2], 253);
		littText(25,  95, messages[3], 253);
		littText(25, 115, messages[4], 253);
		littText(25, 135, messages[5], 253);
	}
	if(g_engine->shouldQuit()){
		return;
	}
	totalFadeIn(0, "DEFAULT");
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
	loopCount = 0;

	do {
		g_engine->_chrono->updateChrono();
		Common::Event e;
		g_system->getEventManager()->pollEvent(e);
		if (e.type == Common::EVENT_KEYDOWN || e.type == Common::EVENT_LBUTTONUP) {
			goto Lsalirpres;
		}

		if (gameTick) {
			loopCount += 1;
		}
		g_system->delayMillis(10);
	} while (loopCount < 180 && !g_engine->shouldQuit());

	totalFadeOut(0);
	g_engine->_screen->clear();
	drawFlc(0, 0, offsets[4], 0, 9, 3, true, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[5], 2, 9, 4, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[6], 3, 9, 5, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[7], 0, 9, 0, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[8], isSpanish? 4:8, 9, 6, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[9], 3, 9, 7, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[8], isSpanish? 3:8, 9, 8, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(110, 30, offsets[9], 2, 9, 9, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(0, 0, offsets[10], 0, 9, 0, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(235, 100, offsets[11], 3, 9, 10, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[12], 0, 9, 11, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(235, 100, offsets[11], 3, 9, 12, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[12], isSpanish? 0:2, 9, 13, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(235, 100, offsets[11], isSpanish? 3:8, 9, 14, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[12], 0, 9, 15, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(173, 98, offsets[13], 0, 9, 0, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(224, 100, offsets[14], 2, 9, 16, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(0, 0, offsets[15], 0, 18, 17, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	drawFlc(150, 40, offsets[16], 0, 9, 18, false, true, false, exitPressed);
	if (exitPressed)
		goto Lsalirpres;
	delay(1000);
Lsalirpres:
	debug("Exiting intro!");
	totalFadeOut(0);
	g_engine->_screen->clear();
	g_engine->_mouseManager->show();
}

void firstIntroduction() {
	if (!firstTimeDone && !isIntroSeen) {
		introduction();
		firstTimeDone = true;
		ConfMan.setBool("introSeen", true);
		ConfMan.flushToDisk();
	}
}

void initialLogo() {
	bool foobar = false;
	long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][0] : flcOffsets[1][0];
	drawFlc(0, 0, offset, 0, 18, 25, false, false, false, foobar);
	delay(1000);
}

void initialMenu(bool fade) {
	bool bar = false;
	bool validOption = false;
	g_engine->_sound->stopVoc();

	long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][1] : flcOffsets[1][1];

	if (fade)
		drawFlc(0, 0, offset, 0, 9, 0, true, false, false, bar);
	else
		drawFlc(0, 0, offset, 0, 9, 0, false, false, false, bar);
	if (cpCounter2 > 10)
		showError(274);
	mouseX = 160;
	mouseY = 95;
	mouseMaskIndex = 1;
	g_engine->_mouseManager->setMousePos(mouseMaskIndex, mouseX, mouseY);
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
						startNewGame = true;
						continueGame = false;
						validOption = true;
					} else if (x > 173 && x < 267) {
						credits();
						drawFlc(0, 0, offset, 0, 9, 0, true, false, false, bar);
					}
				} else if (y > 140 && y < 155) {
					if (x > 173 && x < 292) {
						totalFadeOut(0);
						g_engine->_screen->clear();
						introduction();
						drawFlc(0, 0, offset, 0, 9, 0, true, false, false, bar);
					} else if (x >= 18 && x <= 145) {
						debug("Load");
						startNewGame = false;
						continueGame = false;
						validOption = true;
					}
				} else if (y > 174 && y < 190) {
					if (x > 20 && x < 145) {
						startNewGame = false;
						validOption = true;
						continueGame = true;
					} else if (x > 173 && y < 288) {
						exitToDOS();
					}
				}
			}
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
	} while (!validOption && !g_engine->shouldQuit());
}

void exitGame() {
	g_engine->_graphics->clear();
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
	uint oldMousePosX, oldMousePosY, dialogSize;
	byte oldMouseMask;
	char exitChar;

	oldMousePosX = mouseX;
	oldMousePosY = mouseY;
	oldMouseMask = mouseMaskIndex;
	g_engine->_mouseManager->hide();
	dialogSize = imagesize(58, 48, 262, 120);
	byte *dialogBackground = (byte *)malloc(dialogSize);
	g_engine->_graphics->getImg(58, 48, 262, 120, dialogBackground);

	drawMenu(7);
	mouseX = 160;
	mouseY = 90;
	mouseMaskIndex = 1;

	g_engine->_mouseManager->setMouseArea(Common::Rect(115, 80, 190, 100));
	g_engine->_mouseManager->setMousePos(mouseMaskIndex, mouseX, mouseY);
	Common::Event e;
	const char hotKeyYes = hotKeyFor(YES);
	const char hotKeyNo = hotKeyFor(NO);
	exitChar = '@';
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();

		while (g_system->getEventManager()->pollEvent(e)) {
			if (isMouseEvent(e)) {
				g_engine->_mouseManager->setMousePos(e.mouse);
			}
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
					exitChar = '\33';
				} else if (e.kbd.keycode == hotKeyYes) {
					debug("would exit game now");
					free(dialogBackground);
					exitGame();
				} else if (e.kbd.keycode == hotKeyNo) {
					exitChar = '\33';
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				uint x = e.mouse.x;
				if (x < 145) {
					free(dialogBackground);
					g_system->quit();
				} else if (x > 160) {
					exitChar = '\33';
				}
			}
		}
		g_engine->_screen->update();
	} while (exitChar != '\33' && !g_engine->shouldQuit());
	debug("finished exitToDos");
	g_engine->_graphics->putImg(58, 48, dialogBackground);
	mouseX = oldMousePosX;
	mouseY = oldMousePosY;
	mouseMaskIndex = oldMouseMask;
	g_engine->_mouseManager->show();
	free(dialogBackground);
	g_engine->_mouseManager->setMouseArea(Common::Rect(0, 0, 305, 185));
}

void soundControls() {
	uint oldMouseX,
		oldMouseY,
		soundControlsSize,
		sliderSize,
		sliderBgSize,
		sfxVol,
		musicVol,
		xfade,
		oldxfade;

	byte ytext, oldMouseMask;
	bool exitSoundControls;

	exitSoundControls = false;
	oldMouseX = mouseX;
	oldMouseY = mouseY;
	oldMouseMask = mouseMaskIndex;
	g_engine->_mouseManager->hide();

	soundControlsSize = imagesize(50, 10, 270, 120);
	//What was on the screen before blitting sound controls
	byte *soundControlsBackground = (byte *)malloc(soundControlsSize);
	g_engine->_graphics->getImg(50, 10, 270, 120, soundControlsBackground);

	mouseX = 150;
	mouseY = 60;
	mouseMaskIndex = 1;

	g_engine->_mouseManager->setMouseArea(Common::Rect(55, 13, 250, 105));

	for (ytext = 1; ytext <= 6; ytext++)
		buttonBorder(120 - (ytext * 10), 80 - (ytext * 10), 200 + (ytext * 10), 60 + (ytext * 10), 251, 251, 251, 251, 0);

	buttonBorder(86, 31, 94, 44, 0, 0, 0, 0, 0);

	line(90, 31, 90, 44, 255);

	sliderSize = imagesize(86, 31, 94, 44);
	byte *slider = (byte *)malloc(sliderSize);
	g_engine->_graphics->getImg(86, 31, 94, 44, slider);

	drawMenu(3);
	sliderBgSize = imagesize(86, 31, 234, 44);

	byte *sliderBackground1 = (byte *)malloc(sliderBgSize);
	byte *sliderBackground2 = (byte *)malloc(sliderBgSize);
	g_engine->_graphics->getImg(86, 31, 234, 44, sliderBackground1);
	g_engine->_graphics->getImg(86, 76, 234, 89, sliderBackground2);

	sfxVol = round(((rightSfxVol + leftSfxVol) / 2) * 20);
	musicVol = round(((musicVolRight + musicVolLeft) / 2) * 20);
	g_engine->_graphics->putImg(sfxVol + 86, 31, slider);
	g_engine->_graphics->putImg(musicVol + 86, 76, slider);

	g_engine->_mouseManager->setMousePos(1, mouseX, mouseY);
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
					mouseClickX = e.mouse.x;
					mouseClickY = e.mouse.y;
				}
			}
			g_engine->_screen->update();
		} while ((!keyPressed && !mouseClicked) && !g_engine->shouldQuit());

		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();
		if (keyPressed) {
			exitSoundControls = true;
		}
		if (mouseClicked) {
			if (mouseClickY >= 22 && mouseClickY <= 37) {
				g_engine->_mouseManager->hide();
				xfade = 86 + sfxVol;
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
						g_engine->_graphics->putImg(86, 31, sliderBackground1);
						g_engine->_graphics->putImg(xfade, 31, slider);
						// This yields a volume between 0 and 140
						sfxVol = xfade - 86;

						debug("volumefx=%d", sfxVol);
						rightSfxVol = round((float)sfxVol / 20);
						leftSfxVol = round((float)sfxVol / 20);
						g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
					}
					g_engine->_screen->update();
				} while (!mouseReleased);

				g_engine->_mouseManager->show();
			} else if (mouseClickY >= 67 && mouseClickY <= 82) {
				g_engine->_mouseManager->hide();
				xfade = 86 + musicVol;
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
						g_engine->_graphics->putImg(86, 76, sliderBackground2);
						g_engine->_graphics->putImg(xfade, 76, slider);
						musicVol = xfade - 86;
						musicVolRight = round((float)(musicVol) / 20);
						musicVolLeft = round((float)(musicVol) / 20);
						g_engine->_sound->setMidiVolume(musicVolLeft, musicVolRight);
					}
					g_engine->_screen->update();
				} while (!mouseReleased);

				g_engine->_mouseManager->show();
			} else if (mouseClickY >= 97 && mouseClickY <= 107) {
				exitSoundControls = true;
			}
			mouseClicked = false;
		}
		g_system->delayMillis(10);
		g_engine->_screen->update();
	} while (!exitSoundControls && !g_engine->shouldQuit());

	g_engine->_graphics->putImg(50, 10, soundControlsBackground);
	mouseX = oldMouseX;
	mouseY = oldMouseY;
	mouseMaskIndex = oldMouseMask;
	g_engine->_mouseManager->setMousePos(mouseMaskIndex, mouseX, mouseY);
	free(soundControlsBackground);
	free(slider);
	free(sliderBackground1);
	free(sliderBackground2);

	if (cpCounter > 7)
		showError(274);

	g_engine->_mouseManager->setMouseArea(Common::Rect(0, 0, 305, 185));
}

void sacrificeScene() {
	saveAllowed = false;
	palette palaux;

	Common::File file;
	bool isSpanish = (g_engine->_lang == Common::ES_ESP);
	const char *const *messages = (isSpanish) ? fullScreenMessages[0] : fullScreenMessages[1];

	const long *offsets = (isSpanish) ? flcOffsets[0] : flcOffsets[1];

	g_engine->_sound->stopVoc();
	bool exitPressed = currentRoomData->paletteAnimationFlag;
	currentRoomData->paletteAnimationFlag = false;

	bar(0, 139, 319, 149, 0);
	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		littText(10, 10, messages[9], 200);
		littText(10, 30, messages[10], 200);
		littText(10, 50, messages[11], 200);
		littText(10, 70, messages[12], 200);
		littText(10, 90, messages[13], 200);
	} else {
		littText(10, 20, messages[9], 200);
		littText(10, 40, messages[10], 200);
		littText(10, 60, messages[11], 200);
		littText(10, 80, messages[12], 200);
		littText(10, 100, messages[13], 200);
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
		littText(10, 10, messages[14], 200);
		littText(10, 30, messages[15], 200);
		littText(10, 50, messages[16], 200);
		littText(10, 70, messages[17], 200);
		littText(10, 90, messages[18], 200);
	} else {
		littText(10, 20, messages[14], 200);
		littText(10, 40, messages[15], 200);
		littText(10, 60, messages[16], 200);
		littText(10, 80, messages[17], 200);
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
		littText(10, 10, messages[19], 200);
		littText(10, 50, messages[20], 200);
		littText(10, 70, messages[21], 200);
		littText(10, 90, messages[22], 200);
	} else {
		littText(10, 30, messages[19], 200);
		littText(10, 60, messages[20], 200);
		littText(10, 80, messages[21], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	delay(10000);
	if (g_engine->shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	bar(10, 10, 310, 120, 0);

	if (!file.open("SALONREC.PAN")) {
		showError(318);
	}
	file.read(palaux, 768);
	file.read(sceneBackground, 44800);
	file.close();

	pal[0] = 0;
	pal[1] = 0;
	pal[2] = 0;
	for (int i = 1; i <= 234; i++) {
		pal[i * 3 + 1] = palaux[i * 3 + 1] << 2;
		pal[i * 3 + 2] = palaux[i * 3 + 2] << 2;
		pal[i * 3 + 3] = palaux[i * 3 + 3] << 2;
	}

	// We dont have the width and height here in the byte buffer
	drawScreen(sceneBackground, false);
	partialFadeIn(234);
	g_engine->_sound->stopVoc();

	if (g_engine->shouldQuit())
		return;

	drawFlc(0, 0, offsets[17], 0, 9, 19, false, false, true, exitPressed);
	totalFadeOut(128);
	g_engine->_sound->stopVoc();
	delay(1000);
	if (g_engine->shouldQuit())
		return;

	g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
	g_engine->_sound->playMidi("SACRIFIC", true);
	g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
	g_engine->_graphics->clear();

	littText(10, 31, messages[23], 254);
	littText(10, 29, messages[23], 254);
	littText(11, 30, messages[23], 254);
	littText(9, 30, messages[23], 254);

	littText(10, 51, messages[24], 254);
	littText(10, 49, messages[24], 254);
	littText(11, 50, messages[24], 254);
	littText(9, 50, messages[24], 254);

	littText(10, 71, messages[25], 254);
	littText(10, 69, messages[25], 254);
	littText(11, 70, messages[25], 254);
	littText(9, 70, messages[25], 254);

	littText(10, 30, messages[23], 255);
	littText(10, 50, messages[24], 255);
	littText(10, 70, messages[25], 255);

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

	if (!file.open("SACRIFIC.PAN")) {
		showError(318);
	}
	file.read(palaux, 768);

	for (int i = 0; i < 256; i++) {
		palaux[i * 3 + 0] = palaux[i * 3 + 0] << 2;
		palaux[i * 3 + 1] = palaux[i * 3 + 1] << 2;
		palaux[i * 3 + 2] = palaux[i * 3 + 2] << 2;
	}

	file.read(sceneBackground, 64000);
	file.close();
	drawFullScreen(sceneBackground);

	palaux[0] = 0;
	palaux[1] = 0;
	palaux[2] = 0;

	redFadeIn(palaux);

	drawFlc(112, 57, offsets[18], 33, 9, 20, true, false, true, exitPressed);
	g_engine->_sound->autoPlayVoc("REZOS", 0, 0);
	if (g_engine->shouldQuit())
		return;

	drawFlc(42, 30, offsets[19], 0, 9, 27, false, false, false, exitPressed);

	if (g_engine->shouldQuit())
		return;

	totalFadeOut(128);
	g_engine->_sound->stopVoc();
	g_engine->_graphics->clear();

	littText(10, 21, messages[26], 254);
	littText(10, 19, messages[26], 254);
	littText(11, 20, messages[26], 254);
	littText(9, 20,  messages[26], 254);

	littText(10, 41, messages[27], 254);
	littText(10, 39, messages[27], 254);
	littText(11, 40, messages[27], 254);
	littText(9, 40,  messages[27], 254);

	littText(10, 61, messages[28], 254);
	littText(10, 59, messages[28], 254);
	littText(11, 60, messages[28], 254);
	littText(9, 60,  messages[28], 254);

	littText(10, 81, messages[29], 254);
	littText(10, 79, messages[29], 254);
	littText(11, 80, messages[29], 254);
	littText(9, 80,  messages[29], 254);

	littText(10, 101, messages[30], 254);
	littText(10, 99,  messages[30], 254);
	littText(11, 100, messages[30], 254);
	littText(9, 100,  messages[30], 254);

	littText(10, 121, messages[31], 254);
	littText(10, 119, messages[31], 254);
	littText(11, 120, messages[31], 254);
	littText(9, 120,  messages[31], 254);

	littText(10, 141, messages[32], 254);
	littText(10, 139, messages[32], 254);
	littText(11, 140, messages[32], 254);
	littText(9, 140,  messages[32], 254);

	if (!isSpanish) {
		littText(10, 161, messages[56], 254);
		littText(10, 159, messages[56], 254);
		littText(11, 160, messages[56], 254);
		littText(9, 160,  messages[56], 254);

		littText(10, 181, messages[57], 254);
		littText(10, 179, messages[57], 254);
		littText(11, 180, messages[57], 254);
		littText(9,  180,  messages[57], 254);
	}

	littText(10, 20, messages[26], 255);
	littText(10, 40, messages[27], 255);
	littText(10, 60, messages[28], 255);
	littText(10, 80, messages[29], 255);
	littText(10, 100, messages[30], 255);
	littText(10, 120, messages[31], 255);
	littText(10, 140, messages[32], 255);

	if (!isSpanish) {
		littText(10, 160, messages[56], 255);
		littText(10, 180, messages[57], 255);
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

	g_engine->_graphics->clear();

	littText(10, 21, messages[33], 254);
	littText(10, 19, messages[33], 254);
	littText(11, 20, messages[33], 254);
	littText(9, 20, messages[33], 254);

	littText(10, 41, messages[34], 254);
	littText(10, 39, messages[34], 254);
	littText(11, 40, messages[34], 254);
	littText(9, 40, messages[34], 254);

	littText(10, 61, messages[35], 254);
	littText(10, 59, messages[35], 254);
	littText(11, 60, messages[35], 254);
	littText(9, 60, messages[35], 254);
	if (isSpanish) {
		littText(10, 81, messages[36], 254);
		littText(10, 79, messages[36], 254);
		littText(11, 80, messages[36], 254);
		littText(9, 80, messages[36], 254);

		littText(10, 101, messages[37], 254);
		littText(10, 99, messages[37], 254);
		littText(11, 100, messages[37], 254);
		littText(9, 100, messages[37], 254);

		littText(10, 121, messages[38], 254);
		littText(10, 119, messages[38], 254);
		littText(11, 120, messages[38], 254);
		littText(9, 120, messages[38], 254);

		littText(10, 141, messages[39], 254);
		littText(10, 139, messages[39], 254);
		littText(11, 140, messages[39], 254);
		littText(9, 140, messages[39], 254);
	}

	littText(10, 20, messages[33], 255);
	littText(10, 40, messages[34], 255);
	littText(10, 60, messages[35], 255);
	if (isSpanish) {
		littText(10, 80, messages[36], 255);
		littText(10, 100, messages[37], 255);
		littText(10, 120, messages[38], 255);
		littText(10, 140, messages[39], 255);
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
	g_engine->_graphics->clear();

	littText(10, 61, messages[40], 254);
	littText(10, 59, messages[40], 254);
	littText(11, 60, messages[40], 254);
	littText(9, 60, messages[40], 254);

	littText(10, 81, messages[41], 254);
	littText(10, 79, messages[41], 254);
	littText(11, 80, messages[41], 254);
	littText(9, 80, messages[41], 254);

	littText(10, 101, messages[42], 254);
	littText(10, 99, messages[42], 254);
	littText(11, 100, messages[42], 254);
	littText(9, 100, messages[42], 254);

	littText(10, 60, messages[40], 255);
	littText(10, 80, messages[41], 255);
	littText(10, 100, messages[42], 255);

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
	currentRoomData->paletteAnimationFlag = exitPressed;
	saveAllowed = true;
}

void ending() {
	saveAllowed = false;
	bool exitRequested;

	const char *const *messages = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];
	const long *offsets = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0] : flcOffsets[1];

	littText(10, 41, messages[43], 249);
	littText(10, 39, messages[43], 249);
	littText(11, 40, messages[43], 249);
	littText(9, 40,  messages[43], 249);

	littText(10, 61, messages[44], 249);
	littText(10, 59, messages[44], 249);
	littText(11, 60, messages[44], 249);
	littText(9, 60,  messages[44], 249);

	littText(10, 40, messages[43], 253);
	littText(10, 60, messages[44], 253);
	if(g_engine->shouldQuit()) {
		return;
	}
	delay(4000);
	totalFadeOut(0);
	g_engine->_graphics->clear();
	if(g_engine->shouldQuit()) {
		return;
	}
	g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
	g_engine->_sound->playMidi("SACRIFIC", true);
	g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
	drawFlc(0, 0, offsets[30], 12, 9, 26, true, false, false, exitRequested);
	if(exitRequested){
		return;
	}
	drawFlc(0, 0, offsets[31], 0, 9, 0, false, false, false, exitRequested);
	if(exitRequested){
		return;
	}
	delay(1000);
	g_engine->_sound->playVoc("NOOO", 0, 0);
	delay(3000);
	saveAllowed = true;
}

void loadBat() {
	Common::File animFile;

	isSecondaryAnimationEnabled = true;
	if (!animFile.open("MURCIE.DAT")) {
		showError(265);
	}
	secondaryAnimFrameSize = animFile.readUint16LE();
	secondaryAnimationFrameCount = animFile.readByte();
	secondaryAnimDirCount = animFile.readByte();
	curSecondaryAnimationFrame = (byte *)malloc(secondaryAnimFrameSize);
	loadAnimationForDirection(&animFile, 0);
	animFile.close();
}

void loadDevil() {
	Common::File animFile;

	isSecondaryAnimationEnabled = true;
	if (!animFile.open("ROJOMOV.DAT")) {
		showError(265);
	}
	secondaryAnimFrameSize = animFile.readUint16LE();
	secondaryAnimationFrameCount = animFile.readByte();
	secondaryAnimDirCount = animFile.readByte();
	curSecondaryAnimationFrame = (byte *)malloc(secondaryAnimFrameSize);
	if (secondaryAnimDirCount != 0) {
		secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&animFile, i);
		}
	}
	animFile.close();
}

void assembleCompleteBackground(byte *image, uint coordx, uint coordy) {
	uint16 w, h;
	uint16 wFondo;
	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	wFondo = READ_LE_UINT16(sceneBackground);

	wFondo++;
	w++;
	h++;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int color = image[4 + j * w + i];
			if (color != 0) {
				sceneBackground[4 + (coordy + j) * wFondo + (coordx + i)] = color;
			}
		}
	}
}

/**
 * Assemble the screen for scroll assembles only the objects because scrolling screens
 * never have secondary animations and character animation is assembled elsewhere.
 */
void assembleScreen(bool scroll) {

	for (int indice = 0; indice < depthLevelCount; indice++) {
		if (screenLayers[indice] != NULL) {
			assembleCompleteBackground(screenLayers[indice], depthMap[indice].posx, depthMap[indice].posy);
		}
		if (!scroll && mainCharAnimation.depth == indice) {
			assembleCompleteBackground(mainCharAnimation.bitmap[charFacingDirection][iframe], characterPosX, characterPosY);
		}
		if (!scroll && currentRoomData->animationFlag && secondaryAnimation.depth == indice) {
			assembleCompleteBackground(curSecondaryAnimationFrame, secondaryAnimation.posx, secondaryAnimation.posy);
		}
	}
}

void disableSecondAnimation() {
	setRoomTrajectories(secondaryAnimHeight, secondaryAnimWidth, RESTORE);
	currentRoomData->animationFlag = false;
	freeAnimation();
	restoreBackground();
	assembleScreen();
}

// Debug
void drawMouseGrid(RoomFileRegister *screen) {
	for (int i = 0; i < 39; i++) {
		for (int j = 0; j < 27; j++) {
			int color = screen->mouseGrid[i][j];
			if (color != 0) {
				int startX = i * xGridCount + 7;
				int startY = j * yGridCount;
				for (int i2 = 0; i2 < xGridCount; i2 += 2) {
					for (int j2 = 0; j2 < yGridCount; j2++) {
						int absPixel = startY + j2;
						int offsetX = (absPixel % 2 == 0) ? 1 : 0;
						if (offsetX < startX + xGridCount && color != 0) {
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
			int color = screen->walkAreasGrid[i][j];
			if (color != 0) {
				int startX = i * xGridCount + 7;
				int startY = j * yGridCount;
				for (int i2 = 0; i2 < xGridCount; i2 += 2) {
					for (int j2 = 0; j2 < yGridCount; j2++) {
						int absPixel = startY + j2;
						int offsetX = (absPixel % 2 == 0) ? 1 : 0;
						if (offsetX < startX + xGridCount && color != 0) {
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
	drawX(characterPosX + characterCorrectionX, characterPosY + characerCorrectionY, 218);
}

void drawRect(byte color, int x, int y, int x2, int y2) {
	rectangle(x, y, x2, y2, color);
}

void printPos(int x, int y, int screenPosX, int screenPosY, const char *label) {
	g_engine->_graphics->restoreBackgroundArea(screenPosX, screenPosY, screenPosX + 100, screenPosY + 10);
	g_engine->_graphics->euroText(Common::String::format("%s: %d, %d", label, x, y), screenPosX, screenPosY, Graphics::kTextAlignLeft);
}

void drawGrid() {
	int horizontal = 320 / xGridCount;
	int vertical = 140 / yGridCount;
	for (int i = 0; i < horizontal; i++) {
		int startX = i * xGridCount;
		g_engine->_screen->drawLine(startX, 0, startX, 140, 200);
	}

	for (int j = 0; j < vertical; j++) {
		int startY = j * yGridCount;
		g_engine->_screen->drawLine(0, startY, 320, startY, 200);
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
}

void setRoomTrajectories(int animationHeight, int animationWidth, TRAJECTORIES_OP op, bool fixGrids) {
	// add to restore the room, subtract to adjust before loading the screen

	if (currentRoomData->animationFlag && currentRoomData->animationName != "QQQQQQQQ") {
		for (int i = 0; i < currentRoomData->secondaryTrajectoryLength; i++) {
			if (op == RESTORE) {
				currentRoomData->secondaryAnimTrajectory[i].x = currentRoomData->secondaryAnimTrajectory[i].x + (animationWidth >> 1);
				currentRoomData->secondaryAnimTrajectory[i].y = currentRoomData->secondaryAnimTrajectory[i].y + animationHeight;
			} else {
				currentRoomData->secondaryAnimTrajectory[i].x = currentRoomData->secondaryAnimTrajectory[i].x - (animationWidth >> 1);
				currentRoomData->secondaryAnimTrajectory[i].y = currentRoomData->secondaryAnimTrajectory[i].y - animationHeight;
			}
		}
		if (fixGrids) {
			for (int i = 0; i < maxXGrid; i++) {
				for (int j = 0; j < maxYGrid; j++) {
					if (op == RESTORE) {
						currentRoomData->walkAreasGrid[oldposx + i][oldposy + j] = movementGridForSecondaryAnim[i][j];
						currentRoomData->mouseGrid[oldposx + i][oldposy + j] = mouseGridForSecondaryAnim[i][j];
					} else {
						if (maskGridSecondaryAnim[i][j] > 0)
							currentRoomData->walkAreasGrid[oldposx + i][oldposy + j] = maskGridSecondaryAnim[i][j];
						if (maskMouseSecondaryAnim[i][j] > 0)
							currentRoomData->mouseGrid[oldposx + i][oldposy + j] = maskMouseSecondaryAnim[i][j];
					}
				}
			}
		}
	}
}

} // End of namespace Tot
