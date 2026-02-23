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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "waynesworld/waynesworld.h"
#include "waynesworld/gamelogic.h"
#include "waynesworld/graphics.h"
#include "waynesworld/objectids.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/paletteman.h"

namespace WaynesWorld {

WaynesWorldEngine::WaynesWorldEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {

	_random = new Common::RandomSource("waynesworld");
	_isSaveAllowed = false;

	Engine::syncSoundSettings();

#ifdef USE_TRANSLATION
	_oldGUILanguage	= TransMan.getCurrentLanguage();

	if (gd->flags & GF_GUILANGSWITCH)
		TransMan.setLanguage(getLanguageLocale(gd->language));
#endif
}

WaynesWorldEngine::~WaynesWorldEngine() {
#ifdef USE_TRANSLATION
	if (TransMan.getCurrentLanguage() != _oldGUILanguage)
		TransMan.setLanguage(_oldGUILanguage);
#endif

	delete _random;

}

Common::Error WaynesWorldEngine::run() {

	_isSaveAllowed = false;

	for (uint i = 0; i < kRoomAnimationsCount; i++)
		_roomAnimations[i] = nullptr;

	for (uint i = 0; i < kStaticRoomObjectSpritesCount; i++)
		_staticRoomObjectSprites[i] = nullptr;

	initGraphics(320, 200);
	initMouseCursor();
	_screen = new Screen();
	_backgroundSurface = new WWSurface(320, 150);
	_inventorySprite = new WWSurface(312, 52);
	_walkMap = new byte[kWalkMapSize];
	_backgroundScrollSurface = nullptr;

	_fontWW = new GFTFont();
	_fontWWInv = new GFTFont();
	_fontBit5x7 = new GFTFont();
	_fontWW->loadFromFile("ww.gft");
	_fontWWInv->loadFromFile("wwinv.gft");
	_fontBit5x7->loadFromFile("bit5x7.gft");

	_logic = new GameLogic(this);

	_wayneSpriteX = 94;
	_wayneSpriteY = 112;
	_garthSpriteX = 147;
	_garthSpriteY = 111;
	_wayneActorScale = 100;
	_garthActorScale = 100;
	_actorSpriteValue = 0;
	_wayneKind = 1;
	_garthKind = 1;
	_actorSpriteIndex = 0;
	_currentActorNum = 1;
	_inventoryItemsCount = 0;
	_currentRoomNumber = -1;
	_hoverObjectNumber = -1;
	_objectNumber = -1;
	_verbNumber = -1;
	_verbNumber2 = 2;
	_firstObjectNumber = -1;
	_isTextVisible = false;
	_currentTextX = -1;
	_currentTextY = -1;
	_scrollPosition = 0;
	_selectedDialogChoice = 0;
	_gameState = 7;
	_currentMapItemIndex = -1;

	loadMainActorSprites();

	initRoomObjects();
	initStaticRoomObjects();
	memset(_wayneInventory, 0, sizeof(_wayneInventory));
	memset(_garthInventory, 0, sizeof(_garthInventory));

	setWayneInventoryItemQuantity(kObjectIdInventoryDollar, 5);
	setGarthInventoryItemQuantity(kObjectIdInventoryDrumstick, 1);

#if 0
	while (!shouldQuit()) {
		updateEvents();
	}
#endif

#if 1
//	loadPalette("m01/wstand0");
//	g_system->getPaletteManager()->setPalette(_palette2, 0, 256);

	CursorMan.showMouse(true);

//	drawImageToScreen("r00/backg", 0, 0);

	drawInterface(2);
	changeRoom(0);
	// _wayneSpriteX = -1; _garthSpriteX = -1;
	// changeRoom(31); // DEBUG
	// _logic->r31_displayCategories();
	// _logic->_r1_eventFlag = 1;
	// _logic->_r1_eventCtr = 3;

	_gameState = 0; // DEBUG Initial _gameState 0 is set by room event in room 0
	// _gameState = 1; // DEBUG Open map

	// lookAtUnusedTicket();

	while (!shouldQuit()) {
		_mouseClickButtons = 0;
		// _keyInput = 0;
		updateEvents();
		updateMouseMove();
		if (_mouseClickButtons != 0) {
			handleMouseClick();
		}
		if (_roomEventNum != 0) {
			handleRoomEvent();
		}
		if (_gameState == 1 && _currentRoomNumber < 100) {
			gameMapOpen();
		}
		updateRoomAnimations();
		g_system->updateScreen();
		// g_system->delayMillis(20);
	}

#endif

	unloadStaticRoomObjects();
	unloadMainActorSprites();

	delete _logic;

	delete _fontWW;
	delete _fontWWInv;
	delete _fontBit5x7;

	delete _backgroundScrollSurface;
	delete[] _walkMap;
	delete _inventorySprite;
	delete _backgroundSurface;
	delete _screen;

	return Common::kNoError;
}

bool WaynesWorldEngine::hasFeature(EngineFeature f) const {
	return false;
	/*
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
	*/
}

void WaynesWorldEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyCode = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			_keyCode = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
  			_mouseX = event.mouse.x;
  			_mouseY = event.mouse.y;
  			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseClickButtons |= kLeftButtonClicked;
			_mouseClickX = event.mouse.x;
			_mouseClickY = event.mouse.y;
			//_mouseClickButtons |= kLeftButtonDown;
  			break;
		case Common::EVENT_LBUTTONUP:
			//_mouseClickButtons &= ~kLeftButtonDown;
  			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseClickButtons |= kRightButtonClicked;
			_mouseClickX = event.mouse.x;
			_mouseClickY = event.mouse.y;
			//_mouseClickButtons |= kRightButtonDown;
  			break;
		case Common::EVENT_RBUTTONUP:
			//_mouseClickButtons &= ~kRightButtonDown;
  			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		default:
			break;
		}
	}
}

int WaynesWorldEngine::getRandom(int max) {
	return max == 0 ? 0 : _random->getRandomNumber(max - 1);
}

void WaynesWorldEngine::waitMillis(uint millis) {
	uint32 waitTime = _system->getMillis() + millis;
	while (_system->getMillis() < waitTime && !shouldQuit()) {
		updateEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

void WaynesWorldEngine::waitSeconds(uint seconds) {
	// TODO Replace calls with waitMillis
	waitMillis(seconds * 1000);
}

void WaynesWorldEngine::initMouseCursor() {
	const uint kCursorWidth = 9;
	const uint kCursorHeight = 9;
	const uint kCursorHotspotX = 4;
	const uint kCursorHotspotY = 4;
	static const byte kCursorData[kCursorWidth * kCursorHeight] = {
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00
	};
	CursorMan.replaceCursor(kCursorData, kCursorWidth, kCursorHeight, kCursorHotspotX, kCursorHotspotY, 0);
}

bool WaynesWorldEngine::isPointAtWayne(int x, int y) {
    int x1 = _wayneSpriteX - (_wayneActorScale * 13) / 100;
    int x2 = _wayneSpriteX + (_wayneActorScale * 13) / 100;
    int y1 = _wayneSpriteY - (_wayneActorScale * 48) / 100;
    int y2 = _wayneSpriteY;
    return x >= x1 && y >= y1 && x <= x2 && y <= y2;
}

bool WaynesWorldEngine::isPointAtGarth(int x, int y) {
    int x1 = _garthSpriteX - (_garthActorScale * 13) / 100;
    int x2 = _garthSpriteX + (_garthActorScale * 13) / 100;
    int y1 = _garthSpriteY - (_garthActorScale * 48) / 100;
    int y2 = _garthSpriteY;
    return x >= x1 && y >= y1 && x <= x2 && y <= y2;
}

void WaynesWorldEngine::updateMouseMove() {

    // Yet unknown
    if (_gameState == 4)
        return;

    // Dialog
    if (_gameState == 2) {
        int newDialogChoiceIndex = -10;
        if (_mouseY > 186) {
            newDialogChoiceIndex = -15;
        } else if (_mouseY > 177) {
            newDialogChoiceIndex = -14;
        } else if (_mouseY > 168) {
            newDialogChoiceIndex = -13;
        } else if (_mouseY > 159) {
            newDialogChoiceIndex = -12;
        } else if (_mouseY > 150) {
            newDialogChoiceIndex = -11;
        }
        if (_selectedDialogChoice != newDialogChoiceIndex) {
            drawDialogChoices(newDialogChoiceIndex);
        }
        return;
    }

    // Unused ticket
    if (_gameState == 3) {
        unusedTicketHandleMouseMove();
        return;
    }

    // Inventory
    if (_gameState == 0 && _inventoryItemsCount > 0 && _mouseX < 312) {
        int inventorySlotIndex = _mouseY / 20 * 12 + _mouseX / 26;
        if (inventorySlotIndex < _inventoryItemsCount) {
            int inventoryObjectId = _inventoryItemsObjectMap[inventorySlotIndex];
            int inventoryItemQuantity;
            if (isActorWayne()) {
                inventoryItemQuantity = getWayneInventoryItemQuantity(inventoryObjectId);
            } else {
                inventoryItemQuantity = getGarthInventoryItemQuantity(inventoryObjectId);
            }
            const char *roomObjectName = getRoomObjectName(inventoryObjectId);
			Common::String objectName;
            if (inventoryItemQuantity == 1) {
//                sprintf(objectName, "%s", roomObjectName);
				objectName = roomObjectName;
            } else {
//                sprintf(objectName, "%d %ss", inventoryItemQuantity, roomObjectName);
				objectName = Common::String::format("%d %ss", inventoryItemQuantity, roomObjectName);
            }
            drawVerbLine(_verbNumber, inventoryObjectId, objectName.c_str());
            return;
        }
    }

    // Wayne and Garth
    if (_gameState == 0 && isActorWayne() && isPointAtGarth(_mouseX, _mouseY)) {
        drawVerbLine(_verbNumber, -3, "Garth");
    } else if (_gameState == 0 && isActorGarth() && isPointAtWayne(_mouseX, _mouseY)) {
        drawVerbLine(_verbNumber, -2, "Wayne");
    } else {
        // Room objects
        int objectIdAtPoint = findRoomObjectIdAtPoint(_mouseX, _mouseY);
        if (_gameState == 0) {
            if (objectIdAtPoint != -1) {
                drawVerbLine(_verbNumber, objectIdAtPoint, getRoomObjectName(objectIdAtPoint));
            } else {
                drawVerbLine(_verbNumber, -1, 0);
            }
        } else if (_gameState == 1) {
            gameMapHandleMouseMove(objectIdAtPoint);
        }
    }

}

void WaynesWorldEngine::handleMouseClick() {
    if (_mouseClickButtons & kLeftButtonClicked) {
        handleMouseLeftClick();
    }
    if (_mouseClickButtons & kRightButtonClicked) {
        handleMouseRightClick();
    }
    if (_mouseClickButtons & kKeyPressed) {
        // TODO handleKeyInput();
    }
}

void WaynesWorldEngine::handleMouseLeftClick() {
    switch (_gameState) {
    case 0:
        if (_mouseClickY < 150) {
            _objectNumber = _hoverObjectNumber;
            if (_objectNumber != -1) {
                walkToObject();
                handleVerb(1);
            } else {
                walkTo(_mouseClickX, _mouseClickY, -1, -1, -1);
            }
        } else if (_mouseClickX > 2 && _mouseClickX < 315 && _mouseClickY > 164) {
            selectVerbNumber(_mouseClickX);
        }
        break;
    case 1:
        gameMapHandleMouseClick();
        break;
    case 2:
        handleDialogMouseClick();
        break;
    case 3:
        unusedTicketHandleMouseClick();
        break;
    case 4:
        // TODO handleMouseClickState4();
        break;
    case 5:
        extremeCloseUpHandleMouseClick();
        break;
    }
}

void WaynesWorldEngine::handleMouseRightClick() {
    switch (_gameState) {
    case 0:
        if (_mouseClickX > 2 && _mouseClickX < 268 && _mouseClickY > 164) {
            selectVerbNumber2(_mouseClickX);
        } else if (_hoverObjectNumber != -1) {
            _objectNumber = _hoverObjectNumber;
            walkToObject();
            handleVerb(2);
        }
        break;
    case 1:
        gameMapHandleMouseClick();
        break;
    case 2:
        handleDialogMouseClick();
        break;
    case 3:
        unusedTicketHandleMouseClick();
        break;
    case 4:
        // TODO handleMouseClickState4();
        break;
    case 5:
        extremeCloseUpHandleMouseClick();
        break;
    }
}

Image::PCXDecoder *WaynesWorldEngine::loadImage(const char *filename, bool appendRoomName) {
//	Common::String tempFilename = appendRoomName
//		? Common::String::format("%s/%s.pcx", _roomName.c_str(), filename)
//		: Common::String::format("%s.pcx", filename);

	Common::Path tempFilename(appendRoomName
		? Common::String::format("%s/%s.pcx", _roomName.c_str(), filename)
		: Common::String::format("%s.pcx", filename));

	Common::File pcxFile;
	if (!pcxFile.open(tempFilename)) {
		warning("loadImage() Could not open '%s'", tempFilename.baseName().c_str());
		return nullptr;
	}

	Image::PCXDecoder *pcx = new Image::PCXDecoder();
	if (!pcx->loadStream(pcxFile)) {
		warning("loadImage() Could not process '%s'", tempFilename.baseName().c_str());
		delete pcx;
		return nullptr;
	}

	return pcx;
}

WWSurface *WaynesWorldEngine::loadSurfaceIntern(const char *filename, bool appendRoomName) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, appendRoomName);
	WWSurface *surface = new WWSurface(imageDecoder->getSurface());
    delete imageDecoder;
    return surface;
}

WWSurface *WaynesWorldEngine::loadSurface(const char *filename) {
	return loadSurfaceIntern(filename, false);
}

WWSurface *WaynesWorldEngine::loadRoomSurface(const char *filename) {
	return loadSurfaceIntern(filename, true);
}

void WaynesWorldEngine::loadPalette(const char *filename) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, false);
	if (imageDecoder->getPalette().empty()) {
		warning("loadPalette() Could not load palette from '%s'", filename);
	} else {
		memcpy(_palette2, imageDecoder->getPalette().data(), Graphics::PALETTE_SIZE);
		g_system->getPaletteManager()->setPalette(_palette2, 0, 256);
	}
    delete imageDecoder;
}

void WaynesWorldEngine::paletteFadeIn(int index, int count, int stepsSize) {
	byte fadePalette[768];
	int fadeStartIndex = index * 3;
	int fadeEndIndex = (index + count) * 3;
	bool isDone = false;
	// CHECKME Assume we're always fading in from black for now
	memset(fadePalette, 0, 768);
	while (!isDone) {
		isDone = true;
		for (int fadeIndex = fadeStartIndex; fadeIndex < fadeEndIndex; fadeIndex++) {
			byte destValue = _palette2[fadeIndex];
			if (fadePalette[fadeIndex] < destValue) {
				fadePalette[fadeIndex] = MIN<int>(fadePalette[fadeIndex] + stepsSize, destValue);
				isDone = false;
			}
		}
		g_system->getPaletteManager()->setPalette(fadePalette, 0, 256);
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
}

void WaynesWorldEngine::paletteFadeOut(int index, int count, int stepsSize) {
	byte fadePalette[768];
	int fadeStartIndex = index * 3;
	int fadeEndIndex = (index + count) * 3;
	bool isDone = false;
	memcpy(fadePalette, _palette2, 768);
	while (!isDone) {
		isDone = true;
		for (int fadeIndex = fadeStartIndex; fadeIndex < fadeEndIndex; fadeIndex++) {
			if (fadePalette[fadeIndex] > 0) {
				fadePalette[fadeIndex] = MAX<int>(fadePalette[fadeIndex] - stepsSize, 0);
				isDone = false;
			}
		}
		g_system->getPaletteManager()->setPalette(fadePalette, 0, 256);
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
}

void WaynesWorldEngine::drawImageToSurfaceIntern(const char *filename, WWSurface *destSurface, int x, int y, bool transparent, bool appendRoomName) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, appendRoomName);
	if (transparent) {
		destSurface->drawSurfaceTransparent(imageDecoder->getSurface(), x, y);
	} else {
		destSurface->drawSurface(imageDecoder->getSurface(), x, y);
	}
    delete imageDecoder;
}

void WaynesWorldEngine::drawImageToScreenIntern(const char *filename, int x, int y, bool transparent, bool appendRoomName) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, appendRoomName);
	if (transparent) {
		_screen->drawSurfaceTransparent(imageDecoder->getSurface(), x, y);
	} else {
		_screen->drawSurface(imageDecoder->getSurface(), x, y);
	}
    delete imageDecoder;
}

void WaynesWorldEngine::drawImageToBackground(const char *filename, int x, int y) {
    drawImageToSurfaceIntern(filename, _backgroundSurface, x, y, false, false);
}

void WaynesWorldEngine::drawImageToBackgroundTransparent(const char *filename, int x, int y) {
    drawImageToSurfaceIntern(filename, _backgroundSurface, x, y, true, false);
}

void WaynesWorldEngine::drawImageToScreen(const char *filename, int x, int y) {
    drawImageToScreenIntern(filename, x, y, false, false);
}

void WaynesWorldEngine::drawImageToSurface(const char *filename, WWSurface *destSurface, int x, int y) {
    drawImageToSurfaceIntern(filename, destSurface, x, y, false, false);
}

void WaynesWorldEngine::drawRoomImageToBackground(const char *filename, int x, int y) {
    drawImageToSurfaceIntern(filename, _backgroundSurface, x, y, false, true);
}

void WaynesWorldEngine::drawRoomImageToBackgroundTransparent(const char *filename, int x, int y) {
    drawImageToSurfaceIntern(filename, _backgroundSurface, x, y, true, true);
}

void WaynesWorldEngine::drawRoomImageToScreen(const char *filename, int x, int y) {
    drawImageToScreenIntern(filename, x, y, false, true);
}

void WaynesWorldEngine::drawRoomImageToSurface(const char *filename, WWSurface *destSurface, int x, int y) {
    drawImageToSurfaceIntern(filename, destSurface, x, y, false, true);
}

void WaynesWorldEngine::drawSpiralEffect(Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight) {
	ScreenEffect screenEffect(this, surface, x, y, grainWidth, grainHeight);
	screenEffect.drawSpiralEffect();
}

void WaynesWorldEngine::drawRandomEffect(Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight) {
	ScreenEffect screenEffect(this, surface, x, y, grainWidth, grainHeight);
	screenEffect.drawRandomEffect();
}

Common::String WaynesWorldEngine::loadString(const char *filename, int index, int flag) {
	const uint kMaxStringLen = 60;
	char textBuffer[kMaxStringLen];
	Common::File fd;
	Common::Path tempFilename(Common::String::format("%s.txt", filename));
	if (!fd.open(tempFilename))
		error("WaynesWorldEngine::loadString() Could not open %s", tempFilename.baseName().c_str());
	fd.seek(index * kMaxStringLen);
	fd.read(textBuffer, kMaxStringLen);
	// Decrypt the string
	uint i = 0;
	for (; i < kMaxStringLen; i++) {
		textBuffer[i] += 0x80;
		if (textBuffer[i] == 0x2b)
			break;
	}
	textBuffer[i] = 0;
	return Common::String(textBuffer);
}

void WaynesWorldEngine::drawCurrentTextToSurface(WWSurface *destSurface, int x, int y) {
    drawCurrentText(x, y, destSurface);
}

void WaynesWorldEngine::drawCurrentText(int x, int y, WWSurface *destSurface) {
    int textCenterX, textX, textY = 0, textColor, actorY = 0;
    int textWidth = _fontWWInv->getTextWidth(_currentText.c_str());
    if (x != -1) {
        textCenterX = x;
        textY = y;
        textColor = 15;
    } else if (isActorWayne()) {
        textCenterX = _wayneSpriteX;
        actorY = _wayneSpriteY;
        textColor = 4;
    } else {
        textCenterX = _garthSpriteX;
        actorY = _garthSpriteY;
        textColor = 119;
    }
    textX = textCenterX - 75;
    if (textX < 0) {
        textX = 5;
    } else if (textX + 150 > 319) {
        textX -= textX - 165;
    }
    if (x == -1) {
        textY = (actorY - 48) - (textWidth / 150 + 1) * 15;
        if (textY < 0) {
            textY = actorY + 15;
        }
    }
	if (destSurface) {
		_fontWWInv->drawWrappedText(destSurface, _currentText.c_str(), textX, textY, 150, 0);
		_fontWW->drawWrappedText(destSurface, _currentText.c_str(), textX, textY, 150, textColor);
	} else {
		_screen->drawWrappedText(_fontWWInv, _currentText.c_str(), textX, textY, 150, 0);
		_screen->drawWrappedText(_fontWW, _currentText.c_str(), textX, textY, 150, textColor);
	}
    _isTextVisible = true;
    _currentTextX = x;
    _currentTextY = y;
}

void WaynesWorldEngine::displayText(const char *filename, int index, int flag, int x, int y, int drawToVirtual) {
    _currentText = loadString(filename, index, flag);
    drawCurrentText(x, y, nullptr);
}

void WaynesWorldEngine::displayTextLines(const char *filename, int baseIndex, int x, int y, int count) {
    int ticks = 3000;
    if (count < 0) {
        ticks = -count;
        count = 1;
    }
    for (int textIndex = 0; textIndex < count; textIndex++) {
        displayText(filename, baseIndex + textIndex, 0, x, y, 0);
        _isTextVisible = false;
        waitMillis(ticks);
        refreshActors();
    }
}

void WaynesWorldEngine::playSound(const char *filename, int flag) {
	// TODO
}

void WaynesWorldEngine::changeMusic() {
	// TODO
}

void WaynesWorldEngine::drawInterface(int verbNum) {
    if (isActorWayne()) {
        drawImageToScreen("m00/winter", 0, 151);
    } else {
        drawImageToScreen("m00/ginter", 0, 151);
    }
    selectVerbNumber2(_verbNumber2 * 24 + 3);
    _verbNumber = verbNum;
    selectVerbNumber(_verbNumber * 24 + 3);
}

void WaynesWorldEngine::selectVerbNumber2(int x) {
    // sysMouseDriver(2);
    _screen->frameRect(_verbNumber2 * 24 + 3, 165, _verbNumber2 * 24 + 3 + 23, 198, 0);
    _verbNumber2 = (x - 3) / 24;
    _screen->frameRect(_verbNumber2 * 24 + 3, 165, _verbNumber2 * 24 + 3 + 23, 198, 10);
    // sysMouseDriver(1);
}

void WaynesWorldEngine::selectVerbNumber(int x) {
    int selectedButtonIndex = (x - 3) / 24;
    _firstObjectNumber = -1;
    if (selectedButtonIndex > 10) {
        if (_mouseClickY > 182) {
            if (x < 290) {
                drawInventory();
                refreshActors();
            } else {
                // TODO r0_handleRoomEvent1();
            }
        }
    } else if (selectedButtonIndex == 0) {
        changeActor();
    } else {
        _verbNumber = selectedButtonIndex;
        drawVerbLine(_verbNumber, -4, 0);
        if (_verbNumber == 8 && _inventoryItemsCount == 0) {
            drawInventory();
            refreshActors();
        }
    }
}

void WaynesWorldEngine::changeActor() {
	if (_currentRoomNumber == 31)
		return;
	toggleActor();
    drawInterface(_verbNumber);
    _isTextVisible = false;
    refreshInventory(true);
}

void WaynesWorldEngine::drawVerbLine(int verbNumber, int objectNumber, const char *objectName) {

	// TODO Move to StaticData class/file
	static const char *kVerbStrings[] = {
		"",
		"pick up",
		"look at",
		"use",
		"talk to",
		"push",
		"pull",
		"extreme closeup of",
		"give",
		"open",
		"close"
	};

	if (_hoverObjectNumber == objectNumber || verbNumber == -1)
		return;

	_hoverObjectNumber = objectNumber;

	Common::String verbLine;

	if (objectName) {
		if (verbNumber == 8) {
			if (_firstObjectNumber == -1) {
				verbLine = Common::String::format("give %s to", objectName);
			} else {
				verbLine = Common::String::format("give %s to %s", _firstObjectName.c_str(), objectName);
			}
		} else if (verbNumber == 3) {
			if (_firstObjectNumber == -1) {
				verbLine = Common::String::format("use %s", objectName);
			} else {
				verbLine = Common::String::format("use %s on %s", _firstObjectName.c_str(), objectName);
			}
		} else {
			verbLine = Common::String::format("%s %s", kVerbStrings[verbNumber], objectName);
		}
	} else {
		if (_firstObjectNumber == -1) {
			verbLine = Common::String::format("%s", kVerbStrings[verbNumber]);
		} else if (verbNumber == 8) {
			verbLine = Common::String::format("give %s to", _firstObjectName.c_str());
		} else if (verbNumber == 3) {
			verbLine = Common::String::format("use %s", _firstObjectName.c_str());
		}
	}

	_screen->fillRect(0, 153, 316, 162, 0);
	_screen->drawText(_fontBit5x7, verbLine.c_str(), 5, 154, 6);
}

void WaynesWorldEngine::rememberFirstObjectName(int objectId) {
    if (objectId == -2) {
        _firstObjectName = "Wayne";
    } else if (objectId == -3) {
        _firstObjectName = "Garth";
    } else if (objectId <= kLastInventoryObjectId) {
        int inventoryItemQuantity = 0;
        if (isActorWayne()) {
            inventoryItemQuantity = getWayneInventoryItemQuantity(objectId);
        } else {
            inventoryItemQuantity = getGarthInventoryItemQuantity(objectId);
        }
        if (inventoryItemQuantity == 1) {
            _firstObjectName = getRoomObjectName(objectId);
        } else {
            _firstObjectName = Common::String::format("%d %s", inventoryItemQuantity, getRoomObjectName(objectId));
        }
    } else {
        _firstObjectName = getRoomObjectName(objectId);
    }
}

void WaynesWorldEngine::redrawInventory() {
    // NOTE This seems to hide the inventory
    _inventoryItemsCount = 1;
    drawInventory();
    refreshActors();
}

void WaynesWorldEngine::refreshInventory(bool doRefresh) {
    // NOTE This seems to show the inventory
    if (_inventoryItemsCount != 0) {
        drawInventory();
        drawInventory();
    }
    if (doRefresh) {
        refreshActors();
    }
}

void WaynesWorldEngine::drawInventory() {
    if (_inventoryItemsCount != 0) {
        _inventoryItemsCount = 0;
        return;
    }
    int iconX = 0;
    int iconY = 0;
    _inventorySprite->clear(0);
    for (int inventoryItemIndex = 0; inventoryItemIndex < kInventorySize; inventoryItemIndex++) {
        int objectRoomNumber = getObjectRoom(inventoryItemIndex + kFirstInventoryObjectId);
        if ((isActorWayne() && objectRoomNumber == 99 && _wayneInventory[inventoryItemIndex] > 0) ||
            (isActorGarth() && objectRoomNumber == 99 && _garthInventory[inventoryItemIndex] > 0)) {
            Common::String filename = Common::String::format("m03/icon%02d", inventoryItemIndex + 1);
			debug("filename: [%s]", filename.c_str());
            drawImageToSurface(filename.c_str(), _inventorySprite, iconX, iconY);
            iconX += 26;
            if (iconX > 300) {
                iconX = 0;
                iconY += 20;
            }
            _inventoryItemsObjectMap[_inventoryItemsCount] = inventoryItemIndex + kFirstInventoryObjectId;
            _inventoryItemsCount++;
        }
    }
}

void WaynesWorldEngine::setWayneInventoryItemQuantity(int objectId, int quantity) {
	_wayneInventory[objectId - kFirstInventoryObjectId] = quantity;
}

void WaynesWorldEngine::setGarthInventoryItemQuantity(int objectId, int quantity) {
	_garthInventory[objectId - kFirstInventoryObjectId] = quantity;
}

int WaynesWorldEngine::getWayneInventoryItemQuantity(int objectId) {
	return _wayneInventory[objectId - kFirstInventoryObjectId];
}

int WaynesWorldEngine::getGarthInventoryItemQuantity(int objectId) {
	return _garthInventory[objectId - kFirstInventoryObjectId];
}

void WaynesWorldEngine::loadMainActorSprites() {
    // _inventorySprite = new WWSurface(312, 52);
	_wayneReachRightSprite = loadSurface("m01/wreachr");
	_wayneReachLeftSprite = loadSurface("m01/wreachl");
	_garthReachRightSprite = loadSurface("m01/greachr");
	_garthReachLeftSprite = loadSurface("m01/greachl");
    for (int direction = 0; direction < 8; direction++) {
		_wayneSprites[direction] = loadSurface(Common::String::format("m01/wstand%d", direction).c_str());
		_garthSprites[direction] = loadSurface(Common::String::format("m01/gstand%d", direction).c_str());
		for (int frameNum = 0; frameNum < 4; frameNum++) {			
			_wayneWalkSprites[direction][frameNum] = loadSurface(Common::String::format("m01/wwalk%d%d", direction, frameNum).c_str());
			_garthWalkSprites[direction][frameNum] = loadSurface(Common::String::format("m01/gwalk%d%d", direction, frameNum).c_str());
		}
    }
}

void WaynesWorldEngine::unloadMainActorSprites() {
	delete _wayneReachRightSprite;
	delete _wayneReachLeftSprite;
	delete _garthReachRightSprite;
	delete _garthReachLeftSprite;
    for (int direction = 0; direction < 8; direction++) {
		delete _wayneSprites[direction];
		delete _garthSprites[direction];
		for (int frameNum = 0; frameNum < 4; frameNum++) {
			delete _wayneWalkSprites[direction][frameNum];
			delete _garthWalkSprites[direction][frameNum];
		}
    }
}

int WaynesWorldEngine::getActorScaleFromY(int actorY) {
	return _logic->getActorScaleFromY(actorY);
}

void WaynesWorldEngine::drawActorReachObject(int objectId, int spriteIndex) {
    int direction = getObjectDirection(objectId);
    if (isActorWayne()) {
        drawActors(direction, 2, 1, spriteIndex, _wayneSpriteX, _wayneSpriteY, _garthSpriteX, _garthSpriteY);
    } else {
        drawActors(direction, 1, 2, spriteIndex, _wayneSpriteX, _wayneSpriteY, _garthSpriteX, _garthSpriteY);
    }
}

int WaynesWorldEngine::drawActors(int direction, int wayneKind, int garthKind, int spriteIndex, int wayneX, int wayneY, int garthX, int garthY) {
    updateRoomAnimations();

    if (_wayneSpriteX == -1 && _garthSpriteX == -1) {
        WWSurface *tempBackground = new WWSurface(320, 150);
        tempBackground->drawSurface(_backgroundSurface, 0, 0);
        if (_inventoryItemsCount > 0) {
            tempBackground->drawSurfaceTransparent(_inventorySprite, 0, 0);
        }
        _screen->drawSurface(tempBackground, 0, 0);
        delete tempBackground;
        return 100;
    }

    int wayneHeight = 0, wayneWidth = 0;
    int garthHeight = 0, garthWidth = 0;
    WWSurface *wayneSprite = nullptr;
    WWSurface *garthSprite = nullptr;

    if (_wayneSpriteX != -1) {
        _wayneActorScale = getActorScaleFromY(wayneY);
        wayneHeight = (100 - _wayneActorScale) * 48 / 90;
        wayneWidth = _wayneActorScale * 13 / 100;
        int scaledWayneWidth = _wayneActorScale * 27 / 100;
        int scaledWayneHeight = _wayneActorScale * 48 / 100;
        wayneSprite = new WWSurface(scaledWayneWidth, scaledWayneHeight);
        if (wayneKind == 0) {
            wayneSprite->scaleSurface(_wayneWalkSprites[direction][spriteIndex]);
        } else if (wayneKind == 1) {
            wayneSprite->scaleSurface(_wayneSprites[direction]);
        } else if (wayneKind == 2) {
            if (direction < 5) {
                wayneSprite->scaleSurface(_wayneReachRightSprite);
            } else {
                wayneSprite->scaleSurface(_wayneReachLeftSprite);
            }
        }
        if (_scrollPosition == 0) {
            drawStaticRoomObjects(_currentRoomNumber, wayneX, wayneY, wayneHeight, wayneWidth, wayneSprite);
        }
    }

    if (_garthSpriteX != -1) {
        _garthActorScale = getActorScaleFromY(garthY);
        garthHeight = (100 - _garthActorScale) * 48 / 100;
        garthWidth = _garthActorScale * 13 / 100;
        int scaledGarthWidth = _garthActorScale * 27 / 100;
        int scaledGarthHeight = _garthActorScale * 48 / 100;
        garthSprite = new WWSurface(scaledGarthWidth, scaledGarthHeight);
        if (garthKind == 0) {
            garthSprite->scaleSurface(_garthWalkSprites[direction][spriteIndex]);
        } else if (garthKind == 1) {
            garthSprite->scaleSurface(_garthSprites[direction]);
        } else if (garthKind == 2) {
            if (direction < 5) {
                garthSprite->scaleSurface(_garthReachRightSprite);
            } else {
                garthSprite->scaleSurface(_garthReachLeftSprite);
            }
        }
        if (_scrollPosition == 0) {
            drawStaticRoomObjects(_currentRoomNumber, garthX, garthY, garthHeight, garthWidth, garthSprite);
        }
    }

    WWSurface *tempBackground = new WWSurface(320, 150);
    tempBackground->drawSurface(_backgroundSurface, 0, 0);

#if 0 // DEBUG Draw room mask to background
	for (int yc = 0; yc < 150; yc++) {
		for (int xc = 0; xc < 320; xc++) {
			 *(byte*)tempBackground->getBasePtr(xc, yc) = walkIsPixelWalkable(xc, yc) ? 15 : 0;
		}
	}
#endif

    if (wayneY <= garthY) {
        if (_wayneSpriteX != -1) {
            tempBackground->drawSurfaceTransparent(wayneSprite, wayneX - wayneWidth, wayneY + wayneHeight - 48);
        }
        if (_garthSpriteX != -1) {
            tempBackground->drawSurfaceTransparent(garthSprite, garthX - garthWidth, garthY + garthHeight - 48);
        }
    } else {
        if (_garthSpriteX != -1) {
            tempBackground->drawSurfaceTransparent(garthSprite, garthX - garthWidth, garthY + garthHeight - 48);
        }
        if (_wayneSpriteX != -1) {
            tempBackground->drawSurfaceTransparent(wayneSprite, wayneX - wayneWidth, wayneY + wayneHeight - 48);
        }
    }

    if (_scrollPosition != 0) {
        // drawStaticRoomObjects2(tempBackground);
    }

    if (_inventoryItemsCount > 0) {
        tempBackground->drawSurfaceTransparent(_inventorySprite, 0, 0);
    }

    if (_isTextVisible) {
        drawCurrentTextToSurface(tempBackground, _currentTextX, _currentTextY);
    }

    _screen->drawSurface(tempBackground, 0, 0);

    _wayneSpriteX = wayneX;
    _wayneSpriteY = wayneY;
    _garthSpriteX = garthX;
    _garthSpriteY = garthY;
    _actorSpriteValue = direction;
    _wayneKind = wayneKind;
    _garthKind = garthKind;
    _actorSpriteIndex = spriteIndex;

    if (_wayneSpriteX != -1) {
        delete wayneSprite;
    }

    if (_garthSpriteX != -1) {
        delete garthSprite;
    }

    delete tempBackground;

    return _wayneActorScale;
}

void WaynesWorldEngine::refreshActors() {
	drawActors(_actorSpriteValue, _wayneKind, _garthKind, _actorSpriteIndex, _wayneSpriteX, _wayneSpriteY, _garthSpriteX, _garthSpriteY);
}

void WaynesWorldEngine::pickupObject(int objectId, byte &flags, byte flagsSet, int inventoryObjectId) {
    drawActorReachObject(objectId, 0);
    for (int index = 0; index < 20; index++) {
        waitMillis(50);
        updateRoomAnimations();
    }
    _garthKind = 1;
    _wayneKind = 1;
    moveObjectToNowhere(objectId);
    flags |= flagsSet;
    moveObjectToRoom(inventoryObjectId, 99);
    refreshInventory(false);
    loadRoomBackground();
}

void WaynesWorldEngine::playAnimation(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks) {
//    char filename[32];
	Common::String filename;
    // sysMouseDriver(2);
	if (count > 0) {
		for (int index = startIndex; index < startIndex + count; index++) {
			updateRoomAnimations();
			filename = Common::String::format("%s%d", prefix, index);
			drawRoomImageToScreen(filename.c_str(), x, y);
			drawRoomImageToBackground(filename.c_str(), x, y);
			waitMillis(ticks);
		}
	} else {
		for (int index = startIndex; index > startIndex + count; index--) {
			updateRoomAnimations();
			filename = Common::String::format("%s%d", prefix, index);
			drawRoomImageToScreen(filename.c_str(), x, y);
			drawRoomImageToBackground(filename.c_str(), x, y);
			waitMillis(ticks);
		}
	}
    if (flag) {
		filename = Common::String::format("%s%d", prefix, startIndex);
        drawRoomImageToScreen(filename.c_str(), x, y);
        drawRoomImageToBackground(filename.c_str(), x, y);
    }
    // sysMouseDriver(1)
}

void WaynesWorldEngine::playAnimationLoops(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks, int loopCount) {
	for (int loop = 0; loop < loopCount; loop++) {
		playAnimation(prefix, startIndex, count, x, y, flag, ticks);
	}
}

void WaynesWorldEngine::setWaynePosition(int x, int y) {
    _wayneSpriteX = x;
    _wayneSpriteY = y;
}

void WaynesWorldEngine::setGarthPosition(int x, int y) {
    _garthSpriteX = x;
    _garthSpriteY = y;
}

bool WaynesWorldEngine::isActorWayne() {
	return _currentActorNum != 0;
}

bool WaynesWorldEngine::isActorGarth() {
	return _currentActorNum == 0;
}

void WaynesWorldEngine::selectActorWayne() {
	_currentActorNum = 1;
}

void WaynesWorldEngine::selectActorGarth() {
	_currentActorNum = 0;
}

void WaynesWorldEngine::toggleActor() {
	if (isActorWayne()) {
		selectActorGarth();
	} else {
		selectActorWayne();
	}
}

void WaynesWorldEngine::openRoomLibrary(int roomNum) {
    _roomName = Common::String::format("r%02d", roomNum);
}

void WaynesWorldEngine::loadRoomBackground() {
	loadPalette(Common::String(_roomName + "/backg").c_str());
	g_system->getPaletteManager()->setPalette(_palette2, 0, 256);
	
    drawRoomImageToSurface("backg", _backgroundSurface, 0, 0);
    refreshRoomBackground(_currentRoomNumber);
    refreshActors();
}

void WaynesWorldEngine::changeRoom(int roomNum) {
    if (_currentRoomNumber != -1) {
        // NOTE Not needed gxCloseLib(roomLib);
        unloadStaticRoomObjects();
    }
    openRoomLibrary(roomNum);
    if (_wayneSpriteX != -1) {
        loadRoomMask(roomNum);
        loadStaticRoomObjects(roomNum);
    }
    _actorSpriteValue = 0;
    _currentRoomNumber = roomNum;
    _roomChangeCtr++;
    loadRoomBackground();
}

void WaynesWorldEngine::refreshRoomBackground(int roomNum) {
	_logic->refreshRoomBackground(roomNum);
}

void WaynesWorldEngine::handleRoomEvent() {
    if (_roomEventNum != 0) {
		int eventNum = _roomEventNum;
		_roomEventNum = 0;
		_logic->handleRoomEvent(eventNum);
	}
}

void WaynesWorldEngine::changeRoomScrolling() {
    int roomNumber = -1;
    _scrollPosition = 0;
    _scrollWidth = 0;
    _scrollRemaining = 0;
    switch (_currentRoomNumber) {
    case 14:
        roomNumber = 19;
        break;
    case 19:
        roomNumber = 14;
        break;
    case 8:
        if (isActorWayne()) {
            _garthSpriteX = 319;
            _garthSpriteY = 131;
        } else {
            _wayneSpriteX = 319;
            _wayneSpriteY = 131;
        }
        roomNumber = 21;
        break;
    case 22:
        if (isActorWayne()) {
            _garthSpriteX = 2;
            _garthSpriteY = 131;
        } else {
            _wayneSpriteX = 2;
            _wayneSpriteY = 131;
        }
        roomNumber = 21;
        break;
    case 21:
        if (_hoverObjectNumber == kObjectIdFoyer) {
            if (isActorWayne()) {
                _garthSpriteX = 5;
                _garthSpriteY = 130;
            } else {
                _wayneSpriteX = 5;
                _wayneSpriteY = 130;
            }
            roomNumber = 8;
        } else {
            if (isActorWayne()) {
                _garthSpriteX = 319;
                _garthSpriteY = 131;
            } else {
                _wayneSpriteX = 319;
                _wayneSpriteY = 131;
            }
            roomNumber = 22;
        }
        break;
    case 32:
        roomNumber = 33;
        break;
    case 33:
        roomNumber = 32;
        break;
    }
    unloadStaticRoomObjects();
    openRoomLibrary(roomNumber);
    loadRoomMask(roomNumber);
    loadStaticRoomObjects(roomNumber);
    _currentRoomNumber = roomNumber;
}

void WaynesWorldEngine::loadScrollSprite() {
    if (_currentRoomNumber == 14 && _hoverObjectNumber == kObjectIdLoadingDock) {
        _backgroundScrollSurface = new WWSurface(112, 150);
        drawRoomImageToSurface("scroll", _backgroundScrollSurface, 0, 0);
        if ((_logic->_r1_flags1 & 0x10) && !(_logic->_pizzathonListFlags1 & 0x04)) {
            drawRoomImageToSurface("gill0", _backgroundScrollSurface, 65, 84);
        }
        _scrollRemaining = 112;
        _scrollWidth = 112;
        _doScrollRight = true;
    } else if (_currentRoomNumber == 19 && _hoverObjectNumber == kObjectIdStore) {
        _backgroundScrollSurface = new WWSurface(112, 150);
        drawImageToSurface("r14/backg", _backgroundScrollSurface, 0, 0);
        _scrollRemaining = 112;
        _scrollWidth = 112;
        _doScrollRight = false;
    } else if (_currentRoomNumber == 8 && _hoverObjectNumber == kObjectIdHallway8) {
        stopRoomAnimations();
        _backgroundScrollSurface = new WWSurface(320, 150);
        drawImageToSurface("r21/backg", _backgroundScrollSurface, 0, 0);
        _scrollRemaining = 320;
        _scrollWidth = 320;
        _doScrollRight = false;
    } else if (_currentRoomNumber == 21 && _hoverObjectNumber == kObjectIdFoyer) {
        _backgroundScrollSurface = new WWSurface(320, 150);
        drawImageToSurface("r08/backg", _backgroundScrollSurface, 0, 0);
        _scrollRemaining = 320;
        _scrollWidth = 320;
        _doScrollRight = true;
    } else if (_currentRoomNumber == 21 && _hoverObjectNumber == kObjectIdOffice21) {
        _backgroundScrollSurface = new WWSurface(320, 150);
        drawImageToSurface("r22/backg", _backgroundScrollSurface, 0, 0);
        _scrollRemaining = 320;
        _scrollWidth = 320;
        _doScrollRight = false;
    } else if (_currentRoomNumber == 22 && _hoverObjectNumber == kObjectIdHallway22) {
        _backgroundScrollSurface = new WWSurface(320, 150);
        drawImageToSurface("r21/backg", _backgroundScrollSurface, 0, 0);
        _scrollRemaining = 320;
        _scrollWidth = 320;
        _doScrollRight = true;
    } else if (_currentRoomNumber == 32 && _hoverObjectNumber == kObjectIdOffice) {
        if (!(_logic->_r32_flags & 0x04)) {
            displayTextLines("c04r", 452, 300, 20, 1);
            _scrollPosition = 0;
        } else {
            walkTo(150, 140, 6, 160, 140);
            stopRoomAnimations();
            _backgroundScrollSurface = new WWSurface(168, 150);
            drawImageToSurface("r33/backg", _backgroundScrollSurface, 0, 0);
            _scrollRemaining = 168;
            _scrollWidth = 168;
            _doScrollRight = false;
        }
    } else if (_currentRoomNumber == 33 && _hoverObjectNumber == kObjectIdHallway33) {
        _backgroundScrollSurface = new WWSurface(168, 150);
        drawRoomImageToSurface("scroll", _backgroundScrollSurface, 0, 0);
        if (_logic->_r32_flags & 0x02) {
            drawRoomImageToSurface("noplunge", _backgroundScrollSurface, 141, 94);
        }
        _scrollRemaining = 168;
        _scrollWidth = 168;
        _doScrollRight = true;
    } else {
        _scrollPosition = 0;
    }
}

void WaynesWorldEngine::scrollRoom() {
	const int kScrollStripWidth = 8;
	int stripSourceX, stripDestX;
	if (_doScrollRight) {
		_scrollPosition = _scrollWidth - _scrollRemaining;
		_backgroundSurface->move(-kScrollStripWidth, 0, _backgroundSurface->h);
		stripSourceX = _scrollPosition;
		stripDestX = 320 - kScrollStripWidth;
		_wayneSpriteX -= kScrollStripWidth;
		_garthSpriteX -= kScrollStripWidth;
	} else {
		_scrollPosition = _scrollRemaining - _scrollWidth;
		_backgroundSurface->move(kScrollStripWidth, 0, _backgroundSurface->h);
		stripSourceX = _scrollWidth + _scrollPosition - kScrollStripWidth;
		stripDestX = 0;
		_wayneSpriteX += kScrollStripWidth;
		_garthSpriteX += kScrollStripWidth;
	}
	Graphics::Surface scrollStripSurface = _backgroundScrollSurface->getSubArea(Common::Rect(stripSourceX, 0, stripSourceX + kScrollStripWidth, 150));
	_backgroundSurface->drawSurface(&scrollStripSurface, stripDestX, 0);
	_scrollRemaining -= kScrollStripWidth;
	if (_scrollRemaining <= 0) {
		delete _backgroundScrollSurface;
		_backgroundScrollSurface = nullptr;
	}
}

void WaynesWorldEngine::loadRoomMask(int roomNum) {
	Common::Path filename(Common::String::format("r%02d.msk", roomNum));
	Common::File fd;
	if (!fd.open(filename))
		error("WaynesWorldEngine::loadRoomMask() Could not open %s", filename.baseName().c_str());
	if (fd.size() != kWalkMapSize)
		error("WaynesWorldEngine::loadRoomMask() Wrong file size in %s", filename.baseName().c_str());
	fd.read(_walkMap, kWalkMapSize);
}

void WaynesWorldEngine::fillRoomMaskArea(int x1, int y1, int x2, int y2, bool blocked) {
	for (int y = y1; y <= y2; y++) {
		for (int x = x1; x <= x2; x++) {
			int offset = (y * 40) + (x / 8);
			byte value = 0x80 >> (x % 8);
			if (blocked) {
				_walkMap[offset] &= ~value;
			} else {
				_walkMap[offset] |= value;
			}
		}
	}
}

void WaynesWorldEngine::loadAnimationSpriteRange(int baseIndex, const char *filename, int count) {
	for (int index = 0; index < count; index++) {
		Common::String tempFilename = Common::String::format("%s%d", filename, index);
		loadAnimationSprite(baseIndex + index, tempFilename.c_str());
	}
}

void WaynesWorldEngine::loadAnimationSprite(int index, const char *filename) {
	delete _roomAnimations[index];
	_roomAnimations[index] = loadRoomSurface(filename);
}

void WaynesWorldEngine::drawAnimationSprite(int index, int x, int y) {
	_animationsRedrawBackground = true;
	_backgroundSurface->drawSurface(_roomAnimations[index], x, y);
}

void WaynesWorldEngine::drawAnimationSpriteTransparent(int index, int x, int y) {
	_animationsRedrawBackground = true;
	_backgroundSurface->drawSurfaceTransparent(_roomAnimations[index], x, y);
}

void WaynesWorldEngine::updateRoomAnimations() {
	if (_hasRoomAnimationCallback) {
		_animationsRedrawBackground = false;
		updateAnimationTimers();
		_logic->updateRoomAnimations();
		if (_animationsRedrawBackground) {
			_hasRoomAnimationCallback = false;
			refreshActors();
			_hasRoomAnimationCallback = true;
		}
	}
}

void WaynesWorldEngine::startRoomAnimations() {
	_animationsCtr = 0;
	_hasRoomAnimationCallback = true;
}

void WaynesWorldEngine::stopRoomAnimations() {
	for (uint i = 0; i < kRoomAnimationsCount; i++) {
		delete _roomAnimations[i];
		_roomAnimations[i] = nullptr;
	}
	_hasRoomAnimationCallback = false;
}

void WaynesWorldEngine::updateAnimationTimers() {
	uint32 currentTicks = _system->getMillis();
	for (uint i = 0; i < kAnimationTimersCount; i++) {
		AnimationTimer &animationTimer = _animationTimers[i];
		animationTimer.expired = animationTimer.nextUpdateTicks == 0 || currentTicks >= animationTimer.nextUpdateTicks;
		if (animationTimer.expired) {
			if (animationTimer.nextUpdateTicks > 0) {
				++animationTimer.counter;
			}
			animationTimer.nextUpdateTicks = currentTicks + animationTimer.delay;
		}
	}
}

void WaynesWorldEngine::setAnimationTimer(uint index, uint32 delay, int initialCounter) {
	AnimationTimer &animationTimer = _animationTimers[index];
	animationTimer.nextUpdateTicks = 0;
	animationTimer.delay = delay;
	animationTimer.counter = initialCounter;
	animationTimer.expired = false;
}

bool WaynesWorldEngine::isAnimationTimerExpired(uint index) {
	return _animationTimers[index].expired;
}

int WaynesWorldEngine::getAnimationTimerCounter(uint index) {
	return _animationTimers[index].counter;
}

void WaynesWorldEngine::initStaticRoomObjects() {
	for  (uint i = 0; i < kStaticRoomObjectsCount; i++)
		_staticRoomObjects[i] = kStaticRoomObjects[i];
}

void WaynesWorldEngine::loadStaticRoomObjects(int roomNum) {
	int startIndex = kStaticRoomObjectsMap[roomNum].index;
	int count = kStaticRoomObjectsMap[roomNum].count;
	for (int index = 0; index < count; index++) {
		const StaticRoomObject &roomObject = _staticRoomObjects[startIndex + index];
		if (roomObject.x1 != -1) {
			debug("%s", roomObject.name);
			_staticRoomObjectSprites[index] = loadRoomSurface(roomObject.name);
			_backgroundSurface->drawSurfaceTransparent(_staticRoomObjectSprites[index], roomObject.x1, roomObject.y1);
		}
	}
}

void WaynesWorldEngine::unloadStaticRoomObjects() {
	for (uint i = 0; i < kStaticRoomObjectSpritesCount; i++) {
		delete _staticRoomObjectSprites[i];
		_staticRoomObjectSprites[i] = nullptr;
	}
}

void WaynesWorldEngine::setStaticRoomObjectPosition(int roomNum, int fromIndex, int toIndex, int x, int y) {
	int startIndex = kStaticRoomObjectsMap[roomNum].index + fromIndex;
	int endIndex = kStaticRoomObjectsMap[roomNum].index + toIndex;
	unloadStaticRoomObjects();
	for (int index = startIndex; index <= endIndex; index++) {
		StaticRoomObject &roomObject = _staticRoomObjects[index];
		roomObject.x1 = x;
		roomObject.y1 = y;
	}
	loadStaticRoomObjects(roomNum);
}

void WaynesWorldEngine::drawStaticRoomObjects(int roomNum, int x, int y, int actorHeight, int actorWidth, WWSurface *surface) {
	int x1 = x - actorWidth;
	int x2 = x + actorWidth;
	int y2 = y + actorHeight - 48;
	int startIndex = kStaticRoomObjectsMap[roomNum].index;
	int count = kStaticRoomObjectsMap[roomNum].count;
	for (int index = 0; index < count; index++) {
		const StaticRoomObject &roomObject = _staticRoomObjects[startIndex + index];
		if (roomObject.x1 != -1 && roomObject.y2 > y &&
			((roomObject.x1 <= x1 && roomObject.x2 >= x1) || (roomObject.x1 <= x2 && roomObject.x2 >= x2) || (roomObject.x1 >= x1 && roomObject.x2 <= x2))) {
			surface->drawSurfaceTransparent(_staticRoomObjectSprites[index], roomObject.x1 - x1, roomObject.y1 - y2);
		}
	}
}

void WaynesWorldEngine::initRoomObjects() {
	for  (uint i = 0; i < kRoomObjectsCount; i++)
		_roomObjects[i] = kRoomObjects[i];
}

void WaynesWorldEngine::moveObjectToRoom(int objectId, int roomNum) {
    _roomObjects[objectId].roomNumber = roomNum;
    if (objectId <= kLastInventoryObjectId) {
        int inventoryIndex = objectId - kFirstInventoryObjectId;
        if (isActorWayne()) {
            _wayneInventory[inventoryIndex]++;
        } else {
            _garthInventory[inventoryIndex]++;
        }
    }
}

void WaynesWorldEngine::moveObjectToNowhere(int objectId) {
    if (objectId <= kLastInventoryObjectId) {
        int inventoryIndex = objectId - kFirstInventoryObjectId;
        if (isActorWayne()) {
            _wayneInventory[inventoryIndex]--;
        } else {
            _garthInventory[inventoryIndex]--;
        }
        if (_wayneInventory[inventoryIndex] == 0 && _garthInventory[inventoryIndex] == 0) {
            _roomObjects[objectId].roomNumber = -2;
        }
    } else {
        _roomObjects[objectId].roomNumber = -2;
    }
}

const RoomObject *WaynesWorldEngine::getRoomObject(int objectId) {
    return &_roomObjects[objectId];
}

const char *WaynesWorldEngine::getRoomObjectName(int objectId) {
    return _roomObjects[objectId].name;
}

int WaynesWorldEngine::getObjectRoom(int objectId) {
    return _roomObjects[objectId].roomNumber;
}

int WaynesWorldEngine::getObjectDirection(int objectId) {
    return _roomObjects[objectId].direction;
}

int WaynesWorldEngine::findRoomObjectIdAtPoint(int x, int y) {
    for (uint index = 0; index < kRoomObjectsCount; index++) {
        const RoomObject *roomObject = getRoomObject(index);
        if (roomObject->roomNumber == _currentRoomNumber &&
            x >= roomObject->x1 && x <= roomObject->x2 &&
            y >= roomObject->y1 && y <= roomObject->y2) {
            return (int)index;
        }
    }
    return -1;
}

void WaynesWorldEngine::walkToObject() {
    loadScrollSprite();
    if (_hoverObjectNumber == -2) {
        walkTo(_garthSpriteX, _garthSpriteY, _actorSpriteValue, -1, -1);
    } else if (_hoverObjectNumber == -3) {
        walkTo(_wayneSpriteX, _wayneSpriteY, _actorSpriteValue, -1, -1);
    } else if (_hoverObjectNumber > kLastInventoryObjectId) {
        const RoomObject *roomObject = getRoomObject(_hoverObjectNumber);
        walkTo(roomObject->walkX, roomObject->walkY, roomObject->direction, -1, -1);
    } else {
        refreshActors();
    }
}

void WaynesWorldEngine::startDialog() {
    redrawInventory();
    _gameState = 2;
    drawDialogChoices(-9);
}

void WaynesWorldEngine::setDialogChoices(int choice1, int choice2, int choice3, int choice4, int choice5) {
    _dialogChoices[0] = choice1;
    _dialogChoices[1] = choice2;
    _dialogChoices[2] = choice3;
    _dialogChoices[3] = choice4;
    _dialogChoices[4] = choice5;
}

void WaynesWorldEngine::drawDialogChoices(int choiceIndex) {
    // sysMouseDriver(2);
    if (choiceIndex == -9) {
        // Redraw all and clear the background
        _screen->fillRect(0, 151, 319, 199, 0);
    }
    byte choiceTextColor = isActorWayne() ? 11 : 10;
    byte selectedTextColor = 13;
    for (int index = 0; index < 5 && _dialogChoices[index] != -1; index++) {
        byte textColor = index + choiceIndex + 11 == 0 ? selectedTextColor : choiceTextColor;
        Common::String dialogText = loadString("c04", _dialogChoices[index], 0);
		_screen->drawText(_fontBit5x7, dialogText.c_str(), 3, 152 + index * 9, textColor);
    }
    // sysMouseDriver(1);
    _selectedDialogChoice = choiceIndex;
}

void WaynesWorldEngine::handleDialogMouseClick() {
    int continueDialog = 0;
    int replyTextIndex1, replyTextIndex2, replyTextIndex3;
    int replyTextX, replyTextY;

    if (_selectedDialogChoice == -10)
        return;

    // _selectedDialogChoice goes from -11 (first entry) to -15 (fifth entry)
    _selectedDialogChoice = _dialogChoices[-_selectedDialogChoice - 11];
    if (_selectedDialogChoice == -1)
        return;

    _isTextVisible = false;
    refreshActors();

    continueDialog = _logic->handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);

    if (replyTextIndex1 != -1) {
        _logic->handleDialogReply(replyTextIndex1, replyTextX, replyTextY);
        if (replyTextIndex2 != -1) {
            _logic->handleDialogReply(replyTextIndex2, replyTextX, replyTextY);
            if (replyTextIndex3 != -1) {
                _logic->handleDialogReply(replyTextIndex3, replyTextX, replyTextY);
            }
        }
    }

    if (_gameState == 0) {
        drawInterface(_verbNumber);
    } else if (continueDialog != 0) {
        drawDialogChoices(-9);
    }
}

void WaynesWorldEngine::handleVerb(int verbFlag) {
    int verbNum = verbFlag == 1 ? _verbNumber : _verbNumber2;
    switch (verbNum) {
    case 1:
        handleVerbPickUp();
        break;
    case 2:
        handleVerbLookAt();
        break;
    case 3:
        handleVerbUse();
        break;
    case 4:
        handleVerbTalkTo();
        break;
    case 5:
        handleVerbPush();
        break;
    case 6:
        handleVerbPull();
        break;
    case 7:
        handleVerbExtremeCloseupOf();
        break;
    case 8:
        handleVerbGive();
        break;
    case 9:
        handleVerbOpen();
        break;
    case 10:
        handleVerbClose();
        break;
    }

    if (_scrollWidth != 0) {
        changeRoomScrolling();
    }

}

void WaynesWorldEngine::handleVerbPickUp() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }

    actionTextIndex = _logic->handleVerbPickUp();

    if (_objectNumber == -3) {
        actionTextIndex = 29;
    } else if (_objectNumber == -2) {
        actionTextIndex = 30;
    }

    if (actionTextIndex != -1) {
        displayText("c01", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 7:
        playSound("sv18", 1);
        playSound("sv28", 1);
        break;
    case 0:
    case 10:
    case 44:
    case 54:
        playSound("sv32", 1);
        break;
    case 13:
        playSound("sv19", 1);
        break;
    case 50:
        playSound("sv33", 1);
        break;
    case 51:
    case 59:
        playSound("sv31", 1);
        break;
    case 53:
        playSound("ss07", 1);
        break;
    case 55:
        playSound("sv29", 1);
        break;
    case 56:
        playSound("sv38", 1);
        break;
    }

}

void WaynesWorldEngine::handleVerbLookAt() {
    _isTextVisible = 0;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }
    if (_objectNumber == kObjectIdComputer) {
        moveObjectToRoom(kObjectIdCindi, 103);
    }
    if (_objectNumber == kObjectIdInventoryUnusedTicket) {
        lookAtUnusedTicket();
    } else {
		int textIndex;
		if (_objectNumber == -2 || _objectNumber == -3) {
			textIndex = 0;
		} else {
			textIndex = _objectNumber - 27;
		}
        if (isActorWayne()) {
            displayText("c02w", textIndex, 1, -1, -1, 1);
        } else {
            displayText("c02g", textIndex, 1, -1, -1, 1);
        }
    }
}

void WaynesWorldEngine::handleVerbUse() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }

    // See if the object has to be used with another object
    if (_firstObjectNumber == -1 && (
        _objectNumber == kObjectIdInventoryPlungers ||
        _objectNumber == kObjectIdInventoryExtensionCord ||
        _objectNumber == kObjectIdInventorySuckCut ||
        _objectNumber == kObjectIdSuckCut ||
        _objectNumber == kObjectIdInventoryRope ||
        _objectNumber == kObjectIdInventoryHockeyStick ||
        _objectNumber == kObjectIdInventoryDrumstick ||
        _objectNumber == kObjectIdInventoryCheesePizza ||
        _objectNumber == kObjectIdInventoryChain ||
        _objectNumber == kObjectIdChain_1 ||
        _objectNumber == kObjectIdInventoryTube ||
        _objectNumber == kObjectIdInventoryLunchBox ||
        _objectNumber == kObjectIdInventoryPassCard ||
        _objectNumber == kObjectIdInventorySetOfKeys ||
        _objectNumber == kObjectIdInventorySquirtGun ||
        _objectNumber == kObjectIdInventoryBeakerOfAcid ||
        _objectNumber == kObjectIdInventoryFreshDonut ||
        _objectNumber == kObjectIdInventoryDayOldDonut)) {
        _firstObjectNumber = _objectNumber;
        rememberFirstObjectName(_objectNumber);
        return;
    }

    if (_firstObjectNumber == kObjectIdInventoryBeakerOfAcid && _objectNumber == kObjectIdInventorySquirtGun) {
        _logic->_r39_flags |= 0x01;
        moveObjectToNowhere(kObjectIdInventoryBeakerOfAcid);
        refreshInventory(true);
        return;
    }

    actionTextIndex = _logic->handleVerbUse();

    if (_firstObjectNumber == kObjectIdInventoryPlungers && _objectNumber != kObjectIdLampPost) {
        actionTextIndex = 44;
    } else if (_firstObjectNumber == kObjectIdInventoryExtensionCord && _objectNumber != kObjectIdOutlet11) {
        actionTextIndex = 45;
    } else if (_firstObjectNumber == kObjectIdInventorySuckCut && _objectNumber != kObjectIdExtensionCord_1) {
        actionTextIndex = 48;
    } else if (_firstObjectNumber == kObjectIdSuckCut && _objectNumber != kObjectIdFountain11_0 && _objectNumber != kObjectIdFountain11_1) {
        actionTextIndex = 48;
    }

    if (_objectNumber == -3) {
        actionTextIndex = 31;
    } else if (_objectNumber == -2) {
        if (_currentRoomNumber == 38 && (_logic->_r35_flags & 0x01)) {
            _logic->r38_atrap();
            actionTextIndex = -1;
        } else {
            actionTextIndex = 32;
        }
    }

    _firstObjectNumber = -1;

    if (actionTextIndex != -1) {
        displayText("c03", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 0:
        playSound("sv18", 1);
        break;
    case 67:
        playSound("ss07", 1);
        break;
    case 68:
        playSound("sv32", 1);
        break;
    case 69:
        playSound("sv47", 1);
        break;
    case 70:
        playSound("sv39", 1);
        break;
    case 71:
        playSound("sv38", 1);
        break;
    }

}

void WaynesWorldEngine::handleVerbTalkTo() {
    int actionTextIndex = -1;

    _dialogChoices[0] = -1;

    if (_objectNumber == -3 || _objectNumber == -2) {
        bool room1Special = !(_logic->_r1_flags1 & 0x10) && isActorWayne() && _currentRoomNumber == 1 && (_logic->_r1_flags2 & 0x01);
        if (room1Special) {
            actionTextIndex = 14;
        } else {
            actionTextIndex = getRandom(5) + 16;
            if (_objectNumber == -2)
                actionTextIndex += 5;
        }
        displayText("c11", actionTextIndex, 0, -1, -1, 0);
        waitSeconds(2);
		// Not in the original but without it the old text will be drawn as well
		_isTextVisible = false;
        if (room1Special) {
            actionTextIndex = 15;
        } else {
            actionTextIndex += 10;
        }
		toggleActor();
        loadRoomBackground();
        displayText("c11", actionTextIndex, 0, -1, -1, 0);
		toggleActor();
        waitSeconds(2);
        _isTextVisible = false;
        loadRoomBackground();
        if (room1Special) {
            _logic->r1_handleRoomEvent(true);
        }
        return;
    }

	_logic->handleVerbTalkTo();

    if (_dialogChoices[0] != -1) {
        startDialog();
    }

}

void WaynesWorldEngine::handleVerbPush() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }

    actionTextIndex = _logic->handleVerbPush();

    if (actionTextIndex != -1) {
        displayText("c05", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 0:
        playSound("sv02", 1);
        break;
    case 4:
        playSound("sv47", 1);
        break;
    }

}

void WaynesWorldEngine::handleVerbPull() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }

    actionTextIndex = _logic->handleVerbPull();

    if (actionTextIndex != -1) {
        displayText("c06", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 0:
        playSound("sv31", 1);
        break;
    }

}

void WaynesWorldEngine::handleVerbExtremeCloseupOf() {
	_logic->handleVerbExtremeCloseupOf();
}

void WaynesWorldEngine::handleVerbGive() {
    int actionTextIndex = -1;

    if (_firstObjectNumber == -1) {
        _firstObjectNumber = _objectNumber;
        rememberFirstObjectName(_objectNumber);
        return;
    }

    if (_firstObjectNumber >= kFirstInventoryObjectId && _firstObjectNumber <= kLastInventoryObjectId &&
        (_objectNumber == -2 || _objectNumber == -3)) {
        if (_firstObjectNumber == kObjectIdInventoryDrumstick || _firstObjectNumber == kObjectIdInventoryGuitar) {
            displayText("c08", 2, 0, -1, -1, 0);
        } else {
            int inventoryIndex = _firstObjectNumber - kFirstInventoryObjectId;
            if (_objectNumber == -3) {
                _garthInventory[inventoryIndex] += _wayneInventory[inventoryIndex];
                _wayneInventory[inventoryIndex] = 0;
            } else {
                _wayneInventory[inventoryIndex] += _garthInventory[inventoryIndex];
                _garthInventory[inventoryIndex] = 0;
            }
            refreshInventory(true);
            displayText("c08", 1, 0, -1, -1, 0);
        }
        _firstObjectNumber = -1;
        return;
    }

    actionTextIndex = _logic->handleVerbGive();

    _firstObjectNumber = -1;

    if (actionTextIndex != -1) {
        displayText("c08", actionTextIndex, 0, -1, -1, 0);
    }

}

void WaynesWorldEngine::handleVerbOpen() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }

    actionTextIndex = _logic->handleVerbOpen();

    if (actionTextIndex != -1) {
        displayText("c09", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 5:
        playSound("sv31", 1);
        break;
    case 7:
        playSound("ss07", 1);
        break;
    case 8:
        playSound("sv47", 1);
        break;
    case 0:
        playSound("sv28", 1);
        break;
    case 1:
        playSound("sv38", 1);
        break;
    case 10:
        playSound("sv28", 1);
        break;
    case 11:
        playSound("sv21", 1);
        break;
    }

}

void WaynesWorldEngine::handleVerbClose() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= kLastInventoryObjectId) {
        refreshActors();
    }

    actionTextIndex = _logic->handleVerbClose();

    if (actionTextIndex != -1) {
        displayText("c10", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 0:
        playSound("sv47", 1);
        break;
    case 1:
        playSound("sv21", 1);
        break;
    }

}

void WaynesWorldEngine::lookAtUnusedTicket() {
    // sysMouseDriver(2);
    _gameState = 3;
    _logic->_didScratchTicket = false;
    stopRoomAnimations();    
    if (!(_logic->_r10_flags & 0x80)) {
        _roomAnimations[19] = loadSurface("r10/win");
    } else {
        _roomAnimations[19] = loadSurface("r10/nowin");
    }
    paletteFadeOut(0, 256, 64);
    _screen->clear(0);
    playSound("sv14", 0);
    drawImageToScreen("r10/ticket", 0, 13);
    paletteFadeIn(0, 256, 64);
    // sysMouseDriver(1);
}

void WaynesWorldEngine::unusedTicketHandleMouseMove() {
    if (_mouseX > 157 && _mouseY > 38 && _mouseX < 297 && _mouseY < 129) {
        _logic->_didScratchTicket = true;
        // Reveal partial image
		int scratchX = _mouseX - 158;
		int scratchY = _mouseY - 39;
		Graphics::Surface scratchSurface = _roomAnimations[19]->getSubArea(Common::Rect(scratchX, scratchY, scratchX + 4, scratchY + 4));
		_screen->drawSurface(&scratchSurface, _mouseX - 2, _mouseY - 2);
    }
}

void WaynesWorldEngine::unusedTicketHandleMouseClick() {
    int objectId = kObjectIdInventoryLosingTicket;
    int textIndex = 1;
    _gameState = 0;
    delete _roomAnimations[19];
	_roomAnimations[19] = nullptr;
    if (!((_logic->_r10_flags & 0x80))) {
        objectId = kObjectIdInventoryWinningTicket;
        textIndex = 0;
    }
    if (_logic->_didScratchTicket) {
        moveObjectToRoom(objectId, 99);
        moveObjectToNowhere(29);
        refreshInventory(false);
        _logic->_r10_flags |= 0x80;
    } else {
        textIndex = 2;
    }
    changeRoom(_currentRoomNumber);
    drawInterface(_verbNumber);
    displayText("c00", textIndex, 0, -1, -1, 0);
}

void WaynesWorldEngine::extremeCloseUpHandleMouseClick() {
    _gameState = 0;
    // sysMouseDriver(2);
    paletteFadeOut(0, 256, 16);
    _screen->clear(0);
    paletteFadeIn(0, 256, 16);
    drawInterface(_verbNumber);
    loadRoomBackground();
    // sysMouseDriver(1);
}

} // End of namespace WaynesWorld
