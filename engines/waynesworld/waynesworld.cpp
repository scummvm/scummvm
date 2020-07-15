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

	initGraphics(320, 200);
	_screen = new Screen();
	_backgroundSurface = new WWSurface(320, 150);

#if 0
	while (!shouldQuit()) {
		updateEvents();
	}
#endif

#if 1
	loadPalette("m01/wstand0");
	g_system->getPaletteManager()->setPalette(_palette2, 0, 256);

	WWSurface *wayne = loadSurface("m01/wstand0");

	// Image::PCXDecoder *pcx = loadImage("m00/winter", false);
	// g_system->copyRectToScreen(pcx->getSurface()->getPixels(), pcx->getSurface()->pitch, 0, 0, pcx->getSurface()->w, pcx->getSurface()->h);
	// delete pcx;

	// drawImageToScreen("m00/winter", 0, 151);
	drawInterface(4);
	drawImageToScreen("r00/backg", 0, 0);

	while (!shouldQuit()) {
		updateEvents();
		// _screen->clear(0);
		_screen->drawSurfaceTransparent(wayne, _mouseX - 10, _mouseY - 10);
		g_system->updateScreen();
	}

	delete wayne;

#endif

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
			//_mouseButtons |= kLeftButtonClicked;
			//_mouseButtons |= kLeftButtonDown;
  			break;
		case Common::EVENT_LBUTTONUP:
			//_mouseButtons &= ~kLeftButtonDown;
  			break;
		case Common::EVENT_RBUTTONDOWN:
			//_mouseButtons |= kRightButtonClicked;
			//_mouseButtons |= kRightButtonDown;
  			break;
		case Common::EVENT_RBUTTONUP:
			//_mouseButtons &= ~kRightButtonDown;
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
	// TODO
}

void WaynesWorldEngine::waitSeconds(uint seconds) {
	// TODO
	// Replace calls with waitMillis
}

Image::PCXDecoder *WaynesWorldEngine::loadImage(const char *filename, bool appendRoomName) {
	Common::String tempFilename = appendRoomName
		? Common::String::format("%s/%s.pcx", _roomName.c_str(), filename)
		: Common::String::format("%s.pcx", filename);

	Common::File pcxFile;
	if (!pcxFile.open(tempFilename)) {
		warning("loadImage() Could not open '%s'", tempFilename.c_str());
		return nullptr;
	}

	Image::PCXDecoder *pcx = new Image::PCXDecoder();
	if (!pcx->loadStream(pcxFile)) {
		warning("loadImage() Could not process '%s'", tempFilename.c_str());
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
	if (imageDecoder->getPaletteColorCount() != 256) {
		warning("loadPalette() Could not load palette from '%s'", filename);
	} else {
		memcpy(_palette2, imageDecoder->getPalette(), imageDecoder->getPaletteColorCount() * 3);
	}
    delete imageDecoder;
}

void WaynesWorldEngine::drawImageToSurfaceIntern(const char *filename, WWSurface *destSurface, int x, int y, bool transparent, bool appendRoomName) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, appendRoomName);
	if (transparent) {
		destSurface->drawSurface(imageDecoder->getSurface(), x, y);
	} else {
		destSurface->drawSurfaceTransparent(imageDecoder->getSurface(), x, y);
	}
    delete imageDecoder;
}

void WaynesWorldEngine::drawImageToScreenIntern(const char *filename, int x, int y, bool transparent, bool appendRoomName) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, appendRoomName);
	if (transparent) {
		_screen->drawSurface(imageDecoder->getSurface(), x, y);
	} else {
		_screen->drawSurfaceTransparent(imageDecoder->getSurface(), x, y);
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

void WaynesWorldEngine::loadString(const char *filename, int index, int flag) {
	// TODO Load the string
}

void WaynesWorldEngine::drawCurrentTextToSurface(WWSurface *destSurface, int x, int y) {
    drawCurrentText(x, y, destSurface);
}

void WaynesWorldEngine::drawCurrentText(int x, int y, WWSurface *destSurface) {
    int textWidth, textCenterX, textX, textY, textColor, actorY;
    // WWSurface *textSurface = destSurface ? destSurface : _screen;
    // txSetFont(font_wwinv);
    // textWidth = txStrWidth(txtString);
    if (x != -1) {
        textCenterX = x;
        textY = y;
        textColor = 15;
    } else if (_currentActorNum != 0) {
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
	// TODO Draw the text
    // txSetViewPort(textX, 0, textX + 150, 199);
    // tx_sub_2B2D2(32);
    // txSetColor(0, 0);
    // sysMouseDriver(2);
    // txWrapText(txtString, textX, textY);
    // txSetFont(font_ww);
    // tx_sub_2B2D2(32);
    // txSetColor(textColor, 0);
    // txWrapText(txtString, clippedTextXCpy, textYCpy);
    // sysMouseDriver(1);
    // txSetViewPort(0, 0, 319, 199);
    // txSetFont(font_bit5x7);
    _isTextVisible = true;
    _currentTextX = x;
    _currentTextY = y;
}

void WaynesWorldEngine::displayText(const char *filename, int index, int flag, int x, int y, int drawToVirtual) {
    loadString(filename, index, flag);
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

void WaynesWorldEngine::drawInterface(int verbNum) {
    if (_currentActorNum != 0) {
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
    if (_currentActorNum != 0) {
        _currentActorNum = 0;
    } else {
        _currentActorNum = 1;
    }
    drawInterface(_verbNumber);
    _isTextVisible = false;
    refreshInventory(true);
}

void WaynesWorldEngine::drawVerbLine(int verbNumber, int objectNumber, const char *objectName) {

	// TODO Move to StaticData class/file
	static const char *kVerbStrings[] = {
		"",
		"pick up"
		"look at"
		"use"
		"talk to"
		"push"
		"pull"
		"extreme closeup of"
		"give"
		"open"
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

	_screen->fillRect(3, 162, 316, 153, 0);
	// TODO Implement text drawing and draw it
	// txSetColor(0, 6);
	// txOutTextXY(154, 5, verbLine);
	debug("verbLine: [%s]", verbLine.c_str());
}

void WaynesWorldEngine::rememberFirstObjectName(int objectId) {
    if (objectId == -2) {
        _firstObjectName = "Wayne";
    } else if (objectId == -3) {
        _firstObjectName = "Garth";
    } else if (objectId <= 77) {
        int inventoryIndex = objectId - 28;
        int inventoryItemQuantity = 0;
        if (_currentActorNum != 0) {
            inventoryItemQuantity = _wayneInventory[inventoryIndex];
        } else {
            inventoryItemQuantity = _garthInventory[inventoryIndex];
        }
        if (inventoryItemQuantity == 1) {
            _firstObjectName = "TODO";//getRoomObjectName(objectId);
        } else {
            _firstObjectName = "TODO";//Common::String::format("%d %s", inventoryItemQuantity, getRoomObjectName(objectId));
        }
    } else {
        _firstObjectName = "TODO";//getRoomObjectName(objectId);
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
    for (int inventoryItemIndex = 0; inventoryItemIndex < 50; inventoryItemIndex++) {
        int objectRoomNumber = 0; // TODO getObjectRoom(inventoryItemIndex + 28);
        if ((_currentActorNum != 0 && objectRoomNumber == 99 && _wayneInventory[inventoryItemIndex] > 0) ||
            (_currentActorNum == 0 && objectRoomNumber == 99 && _garthInventory[inventoryItemIndex] > 0)) {
            Common::String filename = Common::String::format("m03/icon%02d", inventoryItemIndex + 1);
            drawImageToSurface(filename.c_str(), _inventorySprite, iconX, iconY);
            iconX += 26;
            if (iconX > 300) {
                iconX = 0;
                iconY += 20;
            }
            _inventoryItemsObjectMap[_inventoryItemsCount] = inventoryItemIndex + 28;
            _inventoryItemsCount++;
        }
    }
}

void WaynesWorldEngine::refreshActors() {
	// TODO
}

void WaynesWorldEngine::changeRoomScrolling() {
	// TODO
}

void WaynesWorldEngine::loadRoomBackground(int roomNum) {
	// TODO
}

void WaynesWorldEngine::moveObjectToRoom(int objectId, int roomNum) {
	// TODO
}

void WaynesWorldEngine::moveObjectToNowhere(int objectId) {
	// TODO
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
    byte choiceTextColor = _currentActorNum != 0 ? 11 : 10;
    byte selectedTextColor = 13;
    for (int index = 0; index < 5 && _dialogChoices[index] != -1; index++) {
        byte textColor = index + choiceIndex + 11 == 0 ? selectedTextColor : choiceTextColor;
        loadString("c04", _dialogChoices[index], 0);
        // TODO Draw the text
        // txOutTextXY(txtString, 3, 152 + index * 9);
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

    continueDialog = logic_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);

    if (replyTextIndex1 != -1) {
        logic_handleDialogReply(replyTextIndex1, replyTextX, replyTextY);
        if (replyTextIndex2 != -1) {
            logic_handleDialogReply(replyTextIndex2, replyTextX, replyTextY);
            if (replyTextIndex3 != -1) {
                logic_handleDialogReply(replyTextIndex3, replyTextX, replyTextY);
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
        // TODO handleVerbExtremeCloseupOf();
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

    if (_word_306DB != 0) {
        changeRoomScrolling();
    }

}

void WaynesWorldEngine::handleVerbPickUp() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= 77) {
        refreshActors();
    }

    actionTextIndex = logic_handleVerbPickUp();

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
    if (_objectNumber <= 77) {
        refreshActors();
    }
    if (_objectNumber == kObjectIdComputer) {
        moveObjectToRoom(kObjectIdCindi, 103);
    }
    if (_objectNumber == kObjectIdInventoryUnusedTicket) {
        // TODO lookAtUnusedTicket();
    } else {
        if (_currentActorNum != 0) {
            displayText("c02w", _objectNumber, 1, -1, -1, 1);
        } else {
            displayText("c02g", _objectNumber, 1, -1, -1, 1);
        }
    }
}

void WaynesWorldEngine::handleVerbUse() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= 77) {
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
        // TODO _byte_3070E |= 0x01;
        moveObjectToNowhere(kObjectIdInventoryBeakerOfAcid);
        refreshInventory(true);
        return;
    }

    actionTextIndex = logic_handleVerbUse();

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
		/* TODO
        if (_currentRoomNumber == 38 && (_byte_3070A & 0x01)) {
            r38_atrap();
            actionTextIndex = -1;
        } else {
            actionTextIndex = 32;
        }
		*/
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
        bool room1Special = false; // TODO !(_byte_306FC & 0x10) && _currentActorNum != 0 && _currentRoomNumber == 1 && (_byte_306FD & 0x01);
        if (room1Special) {
            actionTextIndex = 14;
        } else {
            actionTextIndex = getRandom(5) + 16;
            if (_objectNumber == -2)
                actionTextIndex += 5;
        }
        displayText("c11", actionTextIndex, 0, -1, -1, 0);
        waitSeconds(2);
        if (room1Special) {
            actionTextIndex = 15;
        } else {
            actionTextIndex += 10;
        }
        if (_currentActorNum != 0) {
            _currentActorNum = 1;
        } else {
            _currentActorNum = 0;
        }
        loadRoomBackground(_currentRoomNumber);
        displayText("c11", actionTextIndex, 0, -1, -1, 0);
        if (_currentActorNum != 0) {
            _currentActorNum = 1;
        } else {
            _currentActorNum = 0;
        }
        waitSeconds(2);
        _isTextVisible = false;
        loadRoomBackground(_currentRoomNumber);
        if (room1Special) {
            // TODO r1_handleRoomEvent(true);
        }
        return;
    }

	logic_handleVerbTalkTo();

    if (_dialogChoices[0] != -1) {
        startDialog();
    }

}

void WaynesWorldEngine::handleVerbPush() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= 77) {
        refreshActors();
    }

    actionTextIndex = logic_handleVerbPush();

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
    if (_objectNumber <= 77) {
        refreshActors();
    }

    actionTextIndex = logic_handleVerbPull();

    if (actionTextIndex != -1) {
        displayText("c06", actionTextIndex, 0, -1, -1, 0);
    }

    switch (actionTextIndex) {
    case 0:
        playSound("sv31", 1);
        break;
    }

}

void WaynesWorldEngine::handleVerbGive() {
    int actionTextIndex = -1;

    if (_firstObjectNumber == -1) {
        _firstObjectNumber = _objectNumber;
        rememberFirstObjectName(_objectNumber);
        return;
    }

    if (_firstObjectNumber >= 28 && _firstObjectNumber <= 77 &&
        (_objectNumber == -2 || _objectNumber == -3)) {
        if (_firstObjectNumber == kObjectIdInventoryDrumstick || _firstObjectNumber == kObjectIdInventoryGuitar) {
            displayText("c08", 2, 0, -1, -1, 0);
        } else {
            int inventoryIndex = _firstObjectNumber - 28;
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

    actionTextIndex = logic_handleVerbGive();

    _firstObjectNumber = -1;

    if (actionTextIndex != -1) {
        displayText("c08", actionTextIndex, 0, -1, -1, 0);
    }

}

void WaynesWorldEngine::handleVerbOpen() {
    int actionTextIndex = -1;

    _isTextVisible = false;
    if (_objectNumber <= 77) {
        refreshActors();
    }

    actionTextIndex = logic_handleVerbOpen();

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
    if (_objectNumber <= 77) {
        refreshActors();
    }

    actionTextIndex = logic_handleVerbClose();

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

int WaynesWorldEngine::logic_handleVerbPickUp() {
	// TODO
	return 0;
}

int WaynesWorldEngine::logic_handleVerbUse() {
	// TODO
	return 0;
}

void WaynesWorldEngine::logic_handleVerbTalkTo() {
	// TODO
}

int WaynesWorldEngine::logic_handleVerbPush() {
	// TODO
	return 0;
}

int WaynesWorldEngine::logic_handleVerbPull() {
	// TODO
	return 0;
}

int WaynesWorldEngine::logic_handleVerbOpen() {
	// TODO
	return 0;
}

int WaynesWorldEngine::logic_handleVerbClose() {
	// TODO
	return 0;
}

int WaynesWorldEngine::logic_handleVerbGive() {
	// TODO
	return 0;
}

void WaynesWorldEngine::logic_handleDialogReply(int index, int x, int y) {
	// TODO
}

int WaynesWorldEngine::logic_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	// TODO
	return 0;
}

} // End of namespace WaynesWorld
