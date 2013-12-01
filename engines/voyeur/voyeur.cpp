/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/voyeur.h"
#include "voyeur/animation.h"
#include "voyeur/graphics.h"
#include "voyeur/utils.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

namespace Voyeur {

VoyeurEngine *g_vm;

VoyeurEngine::VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Voyeur"),
		_defaultFontInfo(3, 0xff, 0xff, 0, 0, ALIGN_LEFT, 0, Common::Point(), 1, 1, 
			Common::Point(1, 1), 1, 0, 0) {
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	_bVoy = NULL;

	initialiseManagers();
}

VoyeurEngine::~VoyeurEngine() {
	delete _bVoy;
}

Common::String VoyeurEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool VoyeurEngine::canLoadGameStateCurrently() {
	return true;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool VoyeurEngine::canSaveGameStateCurrently() {
	return true;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error VoyeurEngine::loadGameState(int slot) {
	return Common::kNoError;
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error VoyeurEngine::saveGameState(int slot, const Common::String &desc) {
	//TODO
	return Common::kNoError;
}

Common::Error VoyeurEngine::run() {
	ESP_Init();
	globalInitBolt();

	_eventsManager.resetMouse();
	doHeadTitle();

	//doHeadTitle();

	return Common::kNoError;
}


int VoyeurEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void VoyeurEngine::initialiseManagers() {
	_debugger.setVm(this);
	_eventsManager.setVm(this);
	_filesManager.setVm(this);
	_game.setVm(this);
	_graphicsManager.setVm(this);
	_soundManager.setVm(this);

}

void VoyeurEngine::ESP_Init() {
}

void VoyeurEngine::globalInitBolt() {
	initBolt();

	_filesManager.openBoltLib("bvoy.blt", _bVoy);
	_bVoy->getBoltGroup(0x10000);
	_bVoy->getBoltGroup(0x10100);

	_graphicsManager._fontPtr = &_defaultFontInfo;
	_graphicsManager._fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;
	assert(_graphicsManager._fontPtr->_curFont);

	// Setup default flags
	Common::fill((byte *)&_voy, (byte *)&_voy + sizeof(SVoy), 0);
	_voy._eCursorOff[0x74 / 2] = 1;
	_voy._eCursorOff[0x68 / 2] = 0;
	_voy._eventTable[998]._data = NULL; // Original set 63h:63h
	_voy._evidence[19] = 0;
	_voy._evidence[17] = 0;
	_voy._evidence[18] = 9999;
	
	_voy._curICF0 = _graphicsManager._palFlag ? 0xFFFFA5E0 : 0x5F90; 
	_eventsManager.addFadeInt();
}

void VoyeurEngine::initBolt() {
	vInitInterrupts();
	_graphicsManager.sInitGraphics();
	_eventsManager.vInitColor();
	initInput();
}

void VoyeurEngine::vInitInterrupts() {
	_eventsManager._intPtr._palette = &_graphicsManager._VGAColors[0];
}

void VoyeurEngine::initInput() {
}

bool VoyeurEngine::doHeadTitle() {
//	char dest[144];

	_eventsManager.startMainClockInt();

	// Show starting screen
	if (_bVoy->getBoltGroup(0x10500))
		showConversionScreen();
	if (shouldQuit())
		return false;

	if (ConfMan.getBool("copy_protection")) {
		bool result = doLock();
		if (!result || shouldQuit())
			return false;
	}

	showTitleScreen();

	// Opening
	if (!_voy._incriminate) {
		doOpening();
		_game.doTransitionCard("Saturday Afternoon", "Player's Apartment");
		_eventsManager.delay(90);
	} else {
		_voy._incriminate = false;
	}

	if (_voy._eCursorOff[58] & 0x80) {
		if (_voy._evidence[19] == 0) {
			// TODO
		} else {
			// TODO
		}
	}

	_voy._eCursorOff[55] = 140;
	return true;
}

void VoyeurEngine::showConversionScreen() {
	_graphicsManager._backgroundPage = _bVoy->boltEntry(0x502)._picResource;
	(*_graphicsManager._vPort)->setupViewPort();
	(*_graphicsManager._vPort)->_flags |= DISPFLAG_8;

	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	// Immediate palette load to show the initial screen
	CMapResource *cMap = _bVoy->getCMapResource(0x503);
	assert(cMap);
	cMap->_steps = 0;
	cMap->startFade();

	// Wait briefly
	_eventsManager.delay(150);
	if (shouldQuit())
		return;

	// Fade out the screen
	cMap = _bVoy->getCMapResource(0x5040000);
	cMap->_steps = 30;
	cMap->startFade();
	if (shouldQuit())
		return;

	(*_graphicsManager._vPort)->_flags |= DISPFLAG_8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	while (!shouldQuit() && (_eventsManager._fadeStatus & 1))
		_eventsManager.delay(1);

	_graphicsManager.screenReset();
	_bVoy->freeBoltGroup(0x10500);


}

bool VoyeurEngine::doLock() {
	bool result = true;
	bool flag = false;
	int buttonVocSize, wrongVocSize;
	byte *buttonVoc = _filesManager.fload("button.voc", &buttonVocSize);
	byte *wrongVoc = _filesManager.fload("wrong.voc", &wrongVocSize);
	LockClass lock;
	PictureResource *cursorPic;
	byte *keyData;
	int keyCount;
	int key;

	if (_bVoy->getBoltGroup(0x10700)) {
		lock.getSysDate();
		lock.getThePassword();
		
		_voy._eventTable[999]._type = lock.fieldC;
		_voy._eventTable[999]._data = _bVoy->memberAddr(0x704);

		Common::String password = lock._password;
		cursorPic = _bVoy->getPictureResource(0x702);
		assert(cursorPic);

		// Get the mappings of keys on the keypad
		keyData = _bVoy->memberAddr(0x705);
		keyCount = READ_LE_UINT16(keyData);

		_graphicsManager._backColors = _bVoy->getCMapResource(0x7010000);
		_graphicsManager._backgroundPage = _bVoy->getPictureResource(0x700);
		(*_graphicsManager._vPort)->setupViewPort();

		_graphicsManager._backColors->startFade();
		(*_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();

		while (!shouldQuit() && (_eventsManager._fadeStatus & 1))
			_eventsManager.delay(1);

		_eventsManager.setCursorColor(127, 0);
		_graphicsManager.setColor(1, 64, 64, 64);
		_graphicsManager.setColor(2, 96, 96, 96);
		_graphicsManager.setColor(3, 160, 160, 160);
		_graphicsManager.setColor(4, 224, 224, 224);
		
		// Set up the cursor
		_eventsManager.setCursor(cursorPic);
		_eventsManager.mouseOn();

		_eventsManager._intPtr. field38 = 1;
		_eventsManager._intPtr._hasPalette = true;

		_graphicsManager._fontPtr->_curFont = _bVoy->boltEntry(0x708)._fontResource;
		_graphicsManager._fontPtr->_fontSaveBack = 0;
		_graphicsManager._fontPtr->_fontFlags = 0;

		Common::String dateString = lock.getDateString();
 		Common::String displayString = Common::String::format("Last Play %s", dateString.c_str());

		bool firstLoop = true;
		bool breakFlag = false;
		while (!breakFlag && !shouldQuit()) {
			(*_graphicsManager._vPort)->setupViewPort();
			(*_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
			_graphicsManager.flipPage();
			_eventsManager.sWaitFlip();

			// Display the last play time
			_graphicsManager._fontPtr->_pos = Common::Point(0, 97);
			_graphicsManager._fontPtr->_justify = ALIGN_CENTRE;
			_graphicsManager._fontPtr->_justifyWidth = 384;
			_graphicsManager._fontPtr->_justifyHeight = 97;

			(*_graphicsManager._vPort)->drawText(displayString);
			(*_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
			_graphicsManager.flipPage();
			_eventsManager.sWaitFlip();

			if (firstLoop) {
				firstLoop = false;
				displayString = "";
			}

			// Loop for getting key presses
			do {
				do {
					// Scan through the list of key rects to check if a keypad key is highlighted
					key = -1;
					Common::Point mousePos = _eventsManager.getMousePos() + 
						Common::Point(30, 20);

					for (int keyIndex = 0; keyIndex < keyCount; ++keyIndex) { 
						int x1 = READ_LE_UINT16(keyData + (((keyIndex << 2) + 1) << 1));
						int x2 = READ_LE_UINT16(keyData + (((keyIndex << 2) + 3) << 1));
						int y1 = READ_LE_UINT16(keyData + (((keyIndex << 2) + 2) << 1));
						int y2 = READ_LE_UINT16(keyData + (((keyIndex << 2) + 4) << 1));

						if (mousePos.x >= x1 && mousePos.x <= x2 && mousePos.y >= y1 && mousePos.y <= y2) {
							key = keyIndex;
						}
					}

					_eventsManager.setCursorColor(127, (key == -1) ? 0 : 1);
					_eventsManager._intPtr.field38 = 1;
					_eventsManager._intPtr._hasPalette = true;

					_eventsManager.delay(1);
				} while (!shouldQuit() && !_voy._incriminate);
				_voy._incriminate = false;
			} while (!shouldQuit() && key == -1);

			_soundManager.abortVOCMap();
			_soundManager.playVOCMap(buttonVoc, buttonVocSize);

			while (_soundManager.vocMapStatus()) {
				if (shouldQuit())
					break;

				_soundManager.continueVocMap();
				_eventsManager.delay(1);
			}

			// Process the key
			if (key < 10) {
				// Numeric key
				if (displayString.size() < 10) {
					displayString += '0' + key;
					continue;
				}
			} else if (key == 10) {
				// Accept key
				if (!flag) {
					if ((password.empty() && displayString.empty()) || (password == displayString)) {
						breakFlag = true;
						result = true;
						break;
					}
				} else {
					if (displayString.size() > 0) {
						result = true;
						breakFlag = true;
						break;
					} 
				}
			} else if (key == 11) {
				// New code
				if ((password.empty() && displayString.empty()) || (password != displayString)) {
					(*_graphicsManager._vPort)->setupViewPort();
					flag = true;
					displayString = "";
					continue;
				}
			} else if (key == 12) {
				// Exit keyword
				breakFlag = true;
				result = false;
				break;
			} else {
				continue;
			}

			_soundManager.playVOCMap(wrongVoc, wrongVocSize);
		}

		_graphicsManager.fillPic(*_graphicsManager._vPort);
		(*_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();
		_graphicsManager.resetPalette();

		if (flag && result)
			lock._password = displayString;
		lock.saveThePassword();

		_voy._eventTable[999]._data = NULL;
		_bVoy->freeBoltGroup(0x10700);
	}

	_eventsManager.mouseOff();

	delete[] buttonVoc;
	delete[] wrongVoc;

	return result;
}

void VoyeurEngine::showTitleScreen() {
	if (_bVoy->getBoltGroup(0x10500)) {
		_graphicsManager._backgroundPage = _bVoy->getPictureResource(0x500);

		(*_graphicsManager._vPort)->setupViewPort();
		(*_graphicsManager._vPort)->_flags |= DISPFLAG_8;
		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();

		// Immediate palette load to show the initial screen
		CMapResource *cMap = _bVoy->getCMapResource(0x501);
		assert(cMap);
		cMap->_steps = 60;
		cMap->startFade();

		// Wait briefly
		_eventsManager.delay(200);
		if (shouldQuit())
			return;

		// Fade out the screen
		cMap = _bVoy->getCMapResource(0x504);
		cMap->_steps = 30;
		cMap->startFade();

		(*_graphicsManager._vPort)->_flags |= DISPFLAG_8;
		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();

		while (!shouldQuit() && (_eventsManager._fadeStatus & 1))
			_eventsManager.delay(1);
		if (shouldQuit())
			return;

		_graphicsManager.screenReset();
		_eventsManager.delay(200);

		// Voyeur title
		playRL2Video("a1100100.rl2");
		_graphicsManager.screenReset();

		_bVoy->freeBoltGroup(0x10500);
	}
}

void VoyeurEngine::doOpening() {
	_graphicsManager.screenReset();

	if (!_bVoy->getBoltGroup(0x10200))
		return;

	byte *frameTable = _bVoy->memberAddr(0x215);
	byte *xyTable = _bVoy->memberAddr(0x216);
	byte *whTable = _bVoy->memberAddr(0x217);
	int frmaeIndex = 0;
	int creditShow = 1;
	_game._v2A098 = 1;
	_voy._eCursorOff[52] = 0;
	_voy._RTVNum = 0;
	_voy._eCursorOff[50] = _voy._RTVNum;
	_game._v2A0A6 = 4;
	_game._v2A0A4 = 0;
	_voy._eCursorOff[58] = 16;
	_game._v2A09A = -1;
	_game.addVideoEventStart();
	_voy._eCursorOff[58] &= ~1;

	for (int i = 0; i < 256; ++i)
		_graphicsManager.setColor(i, 8, 8, 8);

	_eventsManager._intPtr.field38 = 1;
	_eventsManager._intPtr._hasPalette = true;
	(*_graphicsManager._vPort)->setupViewPort();
	(*_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	::Video::RL2Decoder decoder;
	decoder.loadFile("a2300100.rl2");
	decoder.start();

	while (!shouldQuit() && !decoder.endOfVideo() && !_voy._incriminate) {
		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_graphicsManager.setPalette(palette, 0, 256);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();

			Common::copy((byte *)frame->getPixels(), (byte *)frame->getPixels() + 320 * 200,
				(byte *)_graphicsManager._screenSurface.getPixels());
		}

		_eventsManager.pollEvents();
		g_system->delayMillis(10);
	}
	
}

void VoyeurEngine::playRL2Video(const Common::String &filename) {
	::Video::RL2Decoder decoder;
	decoder.loadFile(filename);
	decoder.start();

	while (!shouldQuit() && !decoder.endOfVideo() && !_voy._incriminate) {
		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_graphicsManager.setPalette(palette, 0, 256);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();

			Common::copy((byte *)frame->getPixels(), (byte *)frame->getPixels() + 320 * 200,
				(byte *)_graphicsManager._screenSurface.getPixels());
		}

		_eventsManager.pollEvents();
		g_system->delayMillis(10);
	}
}

} // End of namespace Voyeur
