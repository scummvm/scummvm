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

#include "tot/anims.h"
#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void TotEngine::initScreenPointers() {
	_screenSize = 65520;
	_sceneBackground = (byte *)malloc(_screenSize);
	_backgroundCopy = (byte *)malloc(_screenSize);
}

void TotEngine::loadAnimationForDirection(Common::SeekableReadStream *stream, int direction) {
	for (int j = 0; j < _secondaryAnimationFrameCount; j++) {
		_graphics->loadAnimationIntoBuffer(stream, _secondaryAnimation.bitmap[direction][j], _secondaryAnimFrameSize);
	}
}

void TotEngine::loadAnimation(Common::String animationName) {
	Common::File animFile;

	if (animationName == "PETER")
		_isPeterCoughing = true;
	else
		_isPeterCoughing = false;

	_isSecondaryAnimationEnabled = true;
	if (!animFile.open(Common::Path(animationName + ".DAT"))) {
		showError(265);
	}

	_secondaryAnimFrameSize = animFile.readUint16LE();
	_secondaryAnimationFrameCount = animFile.readByte();
	_secondaryAnimDirCount = animFile.readByte();
	_curSecondaryAnimationFrame = (byte *)malloc(_secondaryAnimFrameSize);
	if (_secondaryAnimDirCount != 0) {

		_secondaryAnimationFrameCount = _secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&animFile, i);
		}
	} else {
		loadAnimationForDirection(&animFile, 0);
	}

	animFile.close();
	debug("Read all frames! longtray2=%d", _currentRoomData->secondaryTrajectoryLength);
	_secondaryAnimWidth = READ_LE_UINT16(_secondaryAnimation.bitmap[0][1]) + 1;
	_secondaryAnimHeight = READ_LE_UINT16(_secondaryAnimation.bitmap[0][1] + 2) + 1;

	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, SET_WITH_ANIM, false);

	readObject(_currentRoomData->secondaryAnimDirections[299]);
	_maxXGrid = (_curObject.xgrid2 - _curObject.xgrid1 + 1);
	_maxYGrid = (_curObject.ygrid2 - _curObject.ygrid1 + 1);
	_oldposx = _curObject.xgrid1 + 1;
	_oldposy = _curObject.ygrid1 + 1;

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++) {
			_maskGridSecondaryAnim[i][j] = 0;
			_maskMouseSecondaryAnim[i][j] = 0;
			_movementGridForSecondaryAnim[i][j] = 0;
			_mouseGridForSecondaryAnim[i][j] = 0;
		}

	for (int i = 0; i < _maxXGrid; i++)
		for (int j = 0; j < _maxYGrid; j++) {
			_maskGridSecondaryAnim[i][j] = _curObject.walkAreasPatch[i][j];
			_maskMouseSecondaryAnim[i][j] = _curObject.mouseGridPatch[i][j];
			_movementGridForSecondaryAnim[i][j] = _currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j];
			_mouseGridForSecondaryAnim[i][j] = _currentRoomData->mouseGrid[_oldposx + i][_oldposy + j];
		}
	_iframe2 = 0;
}

void TotEngine::updateAltScreen(byte otherScreenNumber) {
	uint i22;
	uint i11;

	byte currentScreen = _currentRoomData->code;

	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, RESTORE);

	// Save current room
	saveRoomData(_currentRoomData, _rooms);

	// Load other screen
	_rooms->seek(otherScreenNumber * kRoomRegSize, SEEK_SET);
	_currentRoomData = readScreenDataFile(_rooms);

	switch (otherScreenNumber) {
	case 20: {
		switch (_niche[0][_niche[0][3]]) {
		case 0: {
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
			_currentRoomData->screenLayers[1].bitmapPointer = 1190768;
		} break;
		case 561: {
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
			_currentRoomData->screenLayers[1].bitmapPointer = 1182652;
		} break;
		case 563: {
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
			_currentRoomData->screenLayers[1].bitmapPointer = 1186044;
		} break;
		case 615: {
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
			_currentRoomData->screenLayers[1].bitmapPointer = 1181760;
		} break;
		}
		_currentRoomData->screenLayers[1].bitmapSize = 892;
		_currentRoomData->screenLayers[1].coordx = 66;
		_currentRoomData->screenLayers[1].coordy = 35;
		_currentRoomData->screenLayers[1].depth = 1;
	} break;
	case 24: {
		switch (_niche[1][_niche[1][3]]) {
		case 0: {
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
			_currentRoomData->screenLayers[0].bitmapPointer = 1399610;
		} break;
		case 561: {
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
			_currentRoomData->screenLayers[0].bitmapPointer = 1381982;
		} break;
		case 615: {
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
			_currentRoomData->screenLayers[0].bitmapPointer = 1381090;
		} break;
		case 622: {
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
			_currentRoomData->screenLayers[0].bitmapPointer = 1400502;
		} break;
		case 623: {
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
			_currentRoomData->screenLayers[0].bitmapPointer = 1398718;
		} break;
		}
		_currentRoomData->screenLayers[0].bitmapSize = 892;
		_currentRoomData->screenLayers[0].coordx = 217;
		_currentRoomData->screenLayers[0].coordy = 48;
		_currentRoomData->screenLayers[0].depth = 1;
	} break;
	case 31: {
		for (i11 = 23; i11 <= 25; i11++)
			for (i22 = 4; i22 <= 9; i22++)
				_currentRoomData->mouseGrid[i11][i22] = 4;
		for (i11 = 23; i11 <= 25; i11++)
			for (i22 = 10; i22 <= 11; i22++)
				_currentRoomData->mouseGrid[i11][i22] = 3;

		_currentRoomData->screenLayers[0].bitmapSize = 0;
		_currentRoomData->screenLayers[0].bitmapPointer = 0;
		_currentRoomData->screenLayers[0].coordx = 0;
		_currentRoomData->screenLayers[0].coordy = 0;
		_currentRoomData->screenLayers[0].depth = 0;
	} break;
	}

	// Save other screen
	saveRoomData(_currentRoomData, _rooms);

	// Restore current room again
	_rooms->seek(currentScreen * kRoomRegSize, SEEK_SET);
	_currentRoomData = readScreenDataFile(_rooms);

	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, SET_WITH_ANIM);
}

void TotEngine::verifyCopyProtection() {
	//TODO: Copy protection
}

void TotEngine::loadTV() {

	Common::File fichct;
	if (!fichct.open("PALETAS.DAT")) {
		showError(310);
	}
	fichct.seek(_currentRoomData->palettePointer + 603);
	fichct.read(g_engine->_graphics->_palAnimSlice, 144);
	for (int i = 0; i <= 48; i++) {
		g_engine->_graphics->_palAnimSlice[i * 3 + 0] <<= 2;
		g_engine->_graphics->_palAnimSlice[i * 3 + 1] <<= 2;
		g_engine->_graphics->_palAnimSlice[i * 3 + 2] <<= 2;
	}

	fichct.close();
	for (int ix = 195; ix <= 200; ix++) {
		g_engine->_graphics->_pal[ix * 3 + 0] = 2 << 2;
		g_engine->_graphics->_pal[ix * 3 + 1] = 2 << 2;
		g_engine->_graphics->_pal[ix * 3 + 2] = 2 << 2;
		setRGBPalette(ix, 2, 2, 2);
	}
}

void TotEngine::loadScreen() {
	Common::File paletteFile;
	palette palcp;

	_screenSize = _currentRoomData->roomImageSize;
	readBitmap(_currentRoomData->roomImagePointer, _sceneBackground, _screenSize, 316);
	Common::copy(_sceneBackground, _sceneBackground + _screenSize, _backgroundCopy);
	switch (_gamePart) {
	case 1: {
		if (!paletteFile.open("PALETAS.DAT")) {
			showError(310);
		}
		paletteFile.seek(_currentRoomData->palettePointer);
		paletteFile.read(palcp, 603);
		if (_currentRoomData->paletteAnimationFlag) {
			paletteFile.read(g_engine->_graphics->_palAnimSlice, 144);
			for (int i = 0; i <= 48; i++) {
				g_engine->_graphics->_palAnimSlice[i * 3 + 0] <<= 2;
				g_engine->_graphics->_palAnimSlice[i * 3 + 1] <<= 2;
				g_engine->_graphics->_palAnimSlice[i * 3 + 2] <<= 2;
			}
		}
		paletteFile.close();
		for (int i = 1; i <= 200; i++) {
			g_engine->_graphics->_pal[i * 3 + 0] = palcp[i * 3 + 0] << 2;
			g_engine->_graphics->_pal[i * 3 + 1] = palcp[i * 3 + 1] << 2;
			g_engine->_graphics->_pal[i * 3 + 2] = palcp[i * 3 + 2] << 2;
		}
		g_system->getPaletteManager()->setPalette(g_engine->_graphics->_pal, 0, 201);
	} break;
	case 2: {
		_graphics->loadPaletteFromFile("SEGUNDA");
		_currentRoomData->paletteAnimationFlag = true;
	} break;
	}
}

void TotEngine::clearScreenLayers() {
	for (int i = 0; i < kNumScreenOverlays; i++) {
		if (_screenLayers[i] != NULL)
			free(_screenLayers[i]);
		_screenLayers[i] = NULL;
	}
}

void TotEngine::clearAnimation() {
	if (_isSecondaryAnimationEnabled) {
		_isSecondaryAnimationEnabled = false;
		_curSecondaryAnimationFrame = NULL;
		for(int j = 0; j < _secondaryAnimDirCount; j++){
			for(int i = 0; i < _secondaryAnimationFrameCount; i++){
				if(_secondaryAnimation.bitmap[j][i] != NULL && _secondaryAnimation.bitmap[j][i] != _curSecondaryAnimationFrame) {
					free(_secondaryAnimation.bitmap[j][i]);
				}
				_secondaryAnimation.bitmap[j][i] = NULL;
			}
		}
	}
}

void TotEngine::freeInventory() {
	for (int i = 0; i < kInventoryIconCount; i++) {
		free(_inventoryIconBitmaps[i]);
	}
}

void TotEngine::verifyCopyProtection2() {
	// TODO:
}

void TotEngine::loadScreenLayerWithDepth(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth) {
	_screenLayers[depth] = (byte *)malloc(bitmapSize);
	readBitmap(bitmapIndex, _screenLayers[depth], bitmapSize, 319);

	uint16 w, h;
	w = READ_LE_UINT16(_screenLayers[depth]);
	h = READ_LE_UINT16(_screenLayers[depth] + 2);
	_depthMap[depth].posx = coordx;
	_depthMap[depth].posy = coordy;
	_depthMap[depth].posx2 = coordx + w + 1;
	_depthMap[depth].posy2 = coordy + h + 1;
}

void TotEngine::loadScreenLayer(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth) {
	loadScreenLayerWithDepth(coordx, coordy, bitmapSize, bitmapIndex, depth - 1);
}

void TotEngine::updateInventory(byte index) {
	for (int i = index; i < (kInventoryIconCount - 1); i++) {
		_inventory[i].bitmapIndex = _inventory[i + 1].bitmapIndex;
		_inventory[i].code = _inventory[i + 1].code;
		_inventory[i].objectName = _inventory[i + 1].objectName;
	}
	// verifyCopyProtection();
}

void TotEngine::drawLookAtItem(RoomObjectListEntry obj) {
	_mouse->hide();
	bar(0, 140, 319, 149, 0);
	actionLineText(getActionLineText(3) + obj.objectName);
	_mouse->show();
}

void TotEngine::putIcon(uint iconPosX, uint iconPosY, uint iconNum) {
	// substract 1 to account for 1-based indices
	_graphics->putImg(iconPosX, iconPosY, _inventoryIconBitmaps[_inventory[iconNum].bitmapIndex - 1]);
}

void TotEngine::drawInventory() {
	putIcon(34, 169, _inventoryPosition);
	putIcon(77, 169, _inventoryPosition + 1);
	putIcon(120, 169, _inventoryPosition + 2);
	putIcon(163, 169, _inventoryPosition + 3);
	putIcon(206, 169, _inventoryPosition + 4);
	putIcon(249, 169, _inventoryPosition + 5);
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

void TotEngine::drawInventory(byte dir, byte max) {
	switch (dir) {
	case 0:
		if (_inventoryPosition > 0) {
			_inventoryPosition -= 1;
			drawInventory();
		}
		break;
	case 1:
		if (_inventoryPosition < (max - 6)) {
			_inventoryPosition += 1;
			drawInventory();
		}
		break;
	}
	_mouse->hide();
	if (_inventoryPosition > 0)
		lightUpLeft();
	else
		turnOffLeft();
	if (_inventory[_inventoryPosition + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
	_mouse->show();
	if (_cpCounter > 145)
		showError(274);
}

void TotEngine::drawInventoryMask() {

	buttonBorder(0, 140, 319, 149, 0, 0, 0, 0, 0);
	for (int i = 1; i <= 25; i++)
		buttonBorder(0, (175 - i), 319, (174 + i), 251, 251, 251, 251, 0);
	drawMenu(1);
	// verifyCopyProtection();
	if (_inventoryPosition > 1)
		lightUpLeft();
	else
		turnOffLeft();
	if (_inventory[_inventoryPosition + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
}

void TotEngine::drawMenu(byte menuNumber) {
	byte *bitmap;
	uint menuSize;
	byte xmenu, ymenu;
	long menuOffset;

	Common::File menuFile;
	if (!menuFile.open("MENUS.DAT")) {
		showError(258);
	}

	menuOffset = _lang == Common::ES_ESP ? menuOffsets_ES[menuNumber - 1][0] : menuOffsets_EN[menuNumber - 1][0];
	menuSize = _lang == Common::ES_ESP ? menuOffsets_ES[menuNumber - 1][1] : menuOffsets_EN[menuNumber - 1][1];

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
		if (_cpCounter2 > 20)
			showError(274);
		xmenu = 50;
		ymenu = 10;
	} break;
	case 5: {
		if (_cpCounter > 23)
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
	_graphics->putImg(xmenu, ymenu, bitmap);
	free(bitmap);
	menuFile.close();
}

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
	g_engine->_graphics->drawFullScreen(screen);

	free(screen);

	if (dipFile.open(Common::Path("DIPLOMA/" + photoName + ".FOT"))) {
		size = dipFile.size() - 768;
		screen = (byte *)malloc(size);
		dipFile.read(g_engine->_graphics->_pal, 768);
		dipFile.read(screen, size);
		dipFile.close();
		g_engine->_graphics->putShape(10, 20, screen);
		free(screen);
	}
	for (int i = 16; i <= 255; i++) {
		auxPal[i * 3 + 0] = g_engine->_graphics->_pal[i * 3 + 0];
		auxPal[i * 3 + 1] = g_engine->_graphics->_pal[i * 3 + 1];
		auxPal[i * 3 + 2] = g_engine->_graphics->_pal[i * 3 + 2];
	}

	g_engine->_graphics->copyPalette(auxPal, g_engine->_graphics->_pal);
	g_engine->_graphics->fixPalette(g_engine->_graphics->_pal, 768);
	g_engine->_graphics->setPalette(g_engine->_graphics->_pal);
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
	biosText(31, 101, messages[52] + g_engine->_characterName, 0);
	biosText(31, 121, messages[53], 0);
	biosText(31, 141, messages[54], 0);
	biosText(31, 161, messages[55], 0);

	biosText(80, 60,  messages[50], 15);
	biosText(60, 80,  messages[51], 15);
	biosText(30, 100, messages[52], 15);

	biosText(150, 100, g_engine->_characterName, 13);

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
		name = "tot-diploma-default.png";
	Common::OutSaveFile *thumbnail = g_engine->getSaveFileManager()->openForSaving(name);
	Graphics::Surface *surface = g_system->lockScreen();
	assert(surface);
	Image::writePNG(*thumbnail, *surface, g_engine->_graphics->getPalette());
	g_system->unlockScreen();
	thumbnail->finalize();
	delete thumbnail;
}

void TotEngine::generateDiploma(Common::String &photoName) {
	Common::String key;
	_mouse->hide();
	photoName.toUppercase();
	_graphics->totalFadeOut(0);
	loadDiploma(photoName, key);

	Common::Event e;
	bool keyPressed = false;
	do {
		_screen->update();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYUP) {
				keyPressed = true;
			}
		}
		g_system->delayMillis(10);
	} while (!keyPressed && !shouldQuit());
	saveDiploma(photoName, key);
	_mouse->show();
}

void TotEngine::checkMouseGrid() {
	uint xGrid, yGrid;
	Common::String invObject;
	if (_cpCounter2 > 120)
		showError(274);
	if (_mouse->mouseY >= 0 && _mouse->mouseY <= 131) {
		xGrid = _mouse->getMouseCoordsWithinGrid().x;
		yGrid = _mouse->getMouseCoordsWithinGrid().y;
		if (_currentRoomData->mouseGrid[xGrid][yGrid] != _currentRoomData->mouseGrid[_oldGridX][_oldGridY] || _oldInventoryObjectName != "") {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (_actionCode) {
			case 0:
				actionLine = getActionLineText(0) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 1:
				actionLine = getActionLineText(1) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 2:
				actionLine = getActionLineText(2) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 3:
				actionLine = getActionLineText(3) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 4:
				if (_inventoryObjectName != "")
					actionLine = getActionLineText(4) + _inventoryObjectName + getActionLineText(7) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				else
					actionLine = getActionLineText(4) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 5:
				actionLine = getActionLineText(5) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 6:
				actionLine = getActionLineText(6) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			default:
				actionLine = getActionLineText(0) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
			}
			actionLineText(actionLine);
			_mouse->show();
			_oldGridX = xGrid;
			_oldGridY = yGrid;
		}
		_oldActionCode = 253;
		_oldInventoryObjectName = "";
	} else if (_mouse->mouseY >= 132 && _mouse->mouseY <= 165) {
		if (_actionCode != _oldActionCode) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (_actionCode) {
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
				if (_inventoryObjectName != "")
					actionLine = getActionLineText(4) + _inventoryObjectName + getActionLineText(7);
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
			_mouse->show();
			_oldActionCode = _actionCode;
			_oldInventoryObjectName = "";
			_oldGridX = 0;
			_oldGridY = 0;
		}
	} else if (_mouse->mouseY >= 166 && _mouse->mouseY <= 199) {
		if (_mouse->mouseX >= 26 && _mouse->mouseX <= 65) {
			invObject = _inventory[_inventoryPosition].objectName;
		} else if (_mouse->mouseX >= 70 && _mouse->mouseX <= 108) {
			invObject = _inventory[_inventoryPosition + 1].objectName;
		} else if (_mouse->mouseX >= 113 && _mouse->mouseX <= 151) {
			invObject = _inventory[_inventoryPosition + 2].objectName;
		} else if (_mouse->mouseX >= 156 && _mouse->mouseX <= 194) {
			invObject = _inventory[_inventoryPosition + 3].objectName;
		} else if (_mouse->mouseX >= 199 && _mouse->mouseX <= 237) {
			invObject = _inventory[_inventoryPosition + 4].objectName;
		} else if (_mouse->mouseX >= 242 && _mouse->mouseX <= 280) {
			invObject = _inventory[_inventoryPosition + 5].objectName;
		} else {
			invObject = ' ';
		}

		if (invObject != _oldInventoryObjectName || _oldGridX != 0) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (_actionCode) {
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
				if (_inventoryObjectName == "")
					actionLine = getActionLineText(4) + invObject;
				else
					actionLine = getActionLineText(4) + _inventoryObjectName + getActionLineText(7) + invObject;
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
			_mouse->show();
			_oldInventoryObjectName = invObject;
		}
		_oldActionCode = 255;
		_oldGridX = 0;
		_oldGridY = 0;
	}
}

void TotEngine::readAlphaGraph(Common::String &output, int length, int posx, int posy, byte barColor) {
	int pun = 1;
	bar(posx, posy - 2, posx + length * 8, posy + 8, barColor);

	biosText(posx, posy, "_", 0);

	Common::Event e;
	bool done = false;
	while (!done && !shouldQuit()) {
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
					_sound->beep(750, 60);
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
					_sound->beep(1200, 60);
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
		_screen->update();
	}
}

void TotEngine::readAlphaGraphSmall(Common::String &output, int length, int posx, int posy, byte barColor,
						 byte textColor) {
	int pun = 1;
	bool removeCaret;
	bar(posx, posy + 2, posx + length * 6, posy + 9, barColor);

	biosText(posx, posy, "-", textColor);
	Common::Event e;
	bool done = false;

	while (!done && !shouldQuit()) {
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
					_sound->beep(750, 60);
					bar((posx + (output.size()) * 6), (posy + 2), (posx + (output.size() + 1) * 6), (posy + 9), barColor);
				} else if (asciiCode == 8 && pun > 1) {
					output = output.substr(0, output.size() - 1);
					bar(posx, (posy + 2), (posx + length * 6), (posy + 9), barColor);
					biosText(posx, posy, output, textColor);
					biosText((posx + (output.size()) * 6), posy, "-", textColor);
					pun -= 1;
					removeCaret = true;
				} else if ((asciiCode < '\40') || (asciiCode > '\373')) {
					_sound->beep(1200, 60);
					removeCaret = false;
				} else {
					pun += 1;
					output = output + (char)e.kbd.ascii;
					bar(posx, (posy + 2), (posx + length * 6), (posy + 9), barColor);
					littText(posx, posy, output, textColor);
					littText((posx + (output.size()) * 6), posy, "-", textColor);
					removeCaret = true;
				}
			}
		}

		g_system->delayMillis(10);
		_screen->update();
	}

	if (removeCaret)
		bar(posx + (output.size()) * 6, posy + 2, (posx + (output.size()) * 6) + 6, posy + 9, barColor);
}

void TotEngine::displayObjectDescription(Common::String textString) {
	uint xpos = 60;
	uint ypos = 15;
	byte maxWidth = 33;
	byte textColor = 255;
	byte shadowColor = 0;
	byte ihc, lineCounter;
	byte newLineMatrix[10];

	if (textString.size() < maxWidth) {
		euroText((xpos + 1), (ypos + 1), textString, shadowColor);
		_screen->update();
		delay(kEnforcedTextAnimDelay);
		euroText(xpos, ypos, textString, textColor);
		_screen->update();
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
			_screen->update();
			delay(kEnforcedTextAnimDelay);
			euroText(xpos, (ypos + ((line - 1) * 11)), lineString, textColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
		}
	}
}

void TotEngine::buttonBorder(uint x1, uint y1, uint x2, uint y2,
				  byte color1, byte color2, byte color3, byte color4, byte color5) {

	bar(x1, y1, x2, y2, color4);
	line(x1, y1, x1, y2, color1);
	line(x1, y1, x2, y1, color1);

	line(x2, y1, x2, y2, color2);
	line(x2, y2, x1, y2, color2);

	putpixel(x2, y1, color3);
	putpixel(x1, y2, color3);

	_screen->addDirtyRect(Common::Rect(
		x1, y1, x2, y2));
	_screen->update();
}

void TotEngine::copyProtection() {
	//todo
}

void TotEngine::soundControls() {
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
	oldMouseX = _mouse->mouseX;
	oldMouseY = _mouse->mouseY;
	oldMouseMask = _mouse->mouseMaskIndex;
	_mouse->hide();

	soundControlsSize = imagesize(50, 10, 270, 120);
	//What was on the screen before blitting sound controls
	byte *soundControlsBackground = (byte *)malloc(soundControlsSize);
	_graphics->getImg(50, 10, 270, 120, soundControlsBackground);

	_mouse->mouseX = 150;
	_mouse->mouseY = 60;
	_mouse->mouseMaskIndex = 1;

	_mouse->setMouseArea(Common::Rect(55, 13, 250, 105));

	for (ytext = 1; ytext <= 6; ytext++)
		buttonBorder(120 - (ytext * 10), 80 - (ytext * 10), 200 + (ytext * 10), 60 + (ytext * 10), 251, 251, 251, 251, 0);

	buttonBorder(86, 31, 94, 44, 0, 0, 0, 0, 0);

	line(90, 31, 90, 44, 255);

	sliderSize = imagesize(86, 31, 94, 44);
	byte *slider = (byte *)malloc(sliderSize);
	_graphics->getImg(86, 31, 94, 44, slider);

	drawMenu(3);
	sliderBgSize = imagesize(86, 31, 234, 44);

	byte *sliderBackground1 = (byte *)malloc(sliderBgSize);
	byte *sliderBackground2 = (byte *)malloc(sliderBgSize);
	_graphics->getImg(86, 31, 234, 44, sliderBackground1);
	_graphics->getImg(86, 76, 234, 89, sliderBackground2);

	sfxVol = round(((_sound->_rightSfxVol + _sound->_leftSfxVol) / 2) * 20);
	musicVol = round(((_sound->_musicVolRight + _sound->_musicVolLeft) / 2) * 20);
	_graphics->putImg(sfxVol + 86, 31, slider);
	_graphics->putImg(musicVol + 86, 76, slider);

	_mouse->warpMouse(1, _mouse->mouseX, _mouse->mouseY);
	bool keyPressed = false;
	bool mouseClicked = false;
	Common::Event e;
	do {
		_chrono->updateChrono();
		do {
			_chrono->updateChrono();
			_mouse->animateMouseIfNeeded();
			while (g_system->getEventManager()->pollEvent(e)) {
				if (e.type == Common::EVENT_KEYUP) {
					keyPressed = true;
				}
				if (e.type == Common::EVENT_LBUTTONDOWN) {
					mouseClicked = true;
					_mouse->mouseClickX = e.mouse.x;
					_mouse->mouseClickY = e.mouse.y;
				}
			}
			_screen->update();
		} while ((!keyPressed && !mouseClicked) && !shouldQuit());

		_chrono->updateChrono();
		_mouse->animateMouseIfNeeded();
		if (keyPressed) {
			exitSoundControls = true;
		}
		if (mouseClicked) {
			if (_mouse->mouseClickY >= 22 && _mouse->mouseClickY <= 37) {
				_mouse->hide();
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
						_graphics->putImg(86, 31, sliderBackground1);
						_graphics->putImg(xfade, 31, slider);
						// This yields a volume between 0 and 140
						sfxVol = xfade - 86;

						debug("volumefx=%d", sfxVol);
						_sound->_rightSfxVol = round((float)sfxVol / 20);
						_sound->_leftSfxVol = round((float)sfxVol / 20);
						_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
					}
					_screen->update();
				} while (!mouseReleased);

				_mouse->show();
			} else if (_mouse->mouseClickY >= 67 && _mouse->mouseClickY <= 82) {
				_mouse->hide();
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
						_graphics->putImg(86, 76, sliderBackground2);
						_graphics->putImg(xfade, 76, slider);
						musicVol = xfade - 86;
						debug("musicvol=%d", musicVol);
						_sound->_musicVolRight = round((float)(musicVol) / 20);
						_sound->_musicVolLeft = round((float)(musicVol) / 20);
						_sound->setMidiVolume(_sound->_musicVolLeft, _sound->_musicVolRight);
					}
					_screen->update();
				} while (!mouseReleased);

				_mouse->show();
			} else if (_mouse->mouseClickY >= 97 && _mouse->mouseClickY <= 107) {
				exitSoundControls = true;
			}
			mouseClicked = false;
		}
		g_system->delayMillis(10);
		_screen->update();
	} while (!exitSoundControls && !shouldQuit());

	_graphics->putImg(50, 10, soundControlsBackground);
	_mouse->mouseX = oldMouseX;
	_mouse->mouseY = oldMouseY;
	_mouse->mouseMaskIndex = oldMouseMask;
	_mouse->warpMouse(_mouse->mouseMaskIndex, _mouse->mouseX, _mouse->mouseY);
	free(soundControlsBackground);
	free(slider);
	free(sliderBackground1);
	free(sliderBackground2);

	if (_cpCounter > 7)
		showError(274);

	_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
}

void TotEngine::loadBat() {
	Common::File animFile;

	_isSecondaryAnimationEnabled = true;
	if (!animFile.open("MURCIE.DAT")) {
		showError(265);
	}
	_secondaryAnimFrameSize = animFile.readUint16LE();
	_secondaryAnimationFrameCount = animFile.readByte();
	_secondaryAnimDirCount = animFile.readByte();
	_curSecondaryAnimationFrame = (byte *)malloc(_secondaryAnimFrameSize);
	loadAnimationForDirection(&animFile, 0);
	animFile.close();
}

void TotEngine::loadDevil() {
	Common::File animFile;

	_isSecondaryAnimationEnabled = true;
	if (!animFile.open("ROJOMOV.DAT")) {
		showError(265);
	}
	_secondaryAnimFrameSize = animFile.readUint16LE();
	_secondaryAnimationFrameCount = animFile.readByte();
	_secondaryAnimDirCount = animFile.readByte();
	_curSecondaryAnimationFrame = (byte *)malloc(_secondaryAnimFrameSize);
	if (_secondaryAnimDirCount != 0) {
		_secondaryAnimationFrameCount = _secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&animFile, i);
		}
	}
	animFile.close();
}

void TotEngine::assembleCompleteBackground(byte *image, uint coordx, uint coordy) {
	uint16 w, h;
	uint16 wFondo;
	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	wFondo = READ_LE_UINT16(_sceneBackground);

	wFondo++;
	w++;
	h++;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int color = image[4 + j * w + i];
			if (color != 0) {
				_sceneBackground[4 + (coordy + j) * wFondo + (coordx + i)] = color;
			}
		}
	}
}

/**
 * Assemble the screen for scroll assembles only the objects because scrolling screens
 * never have secondary animations and character animation is assembled elsewhere.
 */
void TotEngine::assembleScreen(bool scroll) {

	for (int indice = 0; indice < kDepthLevelCount; indice++) {
		if (_screenLayers[indice] != NULL) {
			assembleCompleteBackground(_screenLayers[indice], _depthMap[indice].posx, _depthMap[indice].posy);
		}
		if (!scroll && _mainCharAnimation.depth == indice) {
			assembleCompleteBackground(_mainCharAnimation.bitmap[_charFacingDirection][_iframe], _characterPosX, _characterPosY);
		}
		if (!scroll && _currentRoomData->animationFlag && _secondaryAnimation.depth == indice) {
			assembleCompleteBackground(_curSecondaryAnimationFrame, _secondaryAnimation.posx, _secondaryAnimation.posy);
		}
	}
}

void TotEngine::disableSecondAnimation() {
	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, RESTORE);
	_currentRoomData->animationFlag = false;
	clearAnimation();
	_graphics->restoreBackground();
	assembleScreen();
}

void TotEngine::setRoomTrajectories(int animationHeight, int animationWidth, TRAJECTORIES_OP op, bool fixGrids) {
	// add to restore the room, subtract to adjust before loading the screen

	if (_currentRoomData->animationFlag && _currentRoomData->animationName != "QQQQQQQQ") {
		for (int i = 0; i < _currentRoomData->secondaryTrajectoryLength; i++) {
			if (op == RESTORE) {
				_currentRoomData->secondaryAnimTrajectory[i].x = _currentRoomData->secondaryAnimTrajectory[i].x + (animationWidth >> 1);
				_currentRoomData->secondaryAnimTrajectory[i].y = _currentRoomData->secondaryAnimTrajectory[i].y + animationHeight;
			} else {
				_currentRoomData->secondaryAnimTrajectory[i].x = _currentRoomData->secondaryAnimTrajectory[i].x - (animationWidth >> 1);
				_currentRoomData->secondaryAnimTrajectory[i].y = _currentRoomData->secondaryAnimTrajectory[i].y - animationHeight;
			}
		}
		if (fixGrids) {
			for (int i = 0; i < _maxXGrid; i++) {
				for (int j = 0; j < _maxYGrid; j++) {
					if (op == RESTORE) {
						_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _movementGridForSecondaryAnim[i][j];
						_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _mouseGridForSecondaryAnim[i][j];
					} else {
						if (_maskGridSecondaryAnim[i][j] > 0)
							_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _maskGridSecondaryAnim[i][j];
						if (_maskMouseSecondaryAnim[i][j] > 0)
							_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _maskMouseSecondaryAnim[i][j];
					}
				}
			}
		}
	}
}

} // End of namespace Tot
