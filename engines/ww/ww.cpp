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

#include "ww/ww.h"
#include "ww/gamelogic.h"
#include "ww/graphics.h"
#include "ww/gxlarchive.h"
#include "ww/objectids.h"

#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace WW {

const char *savegameStr = "SCUMMVM_WAYNES";

WWEngine::WWEngine(OSystem *syst, const WWGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {

	_random = new Common::RandomSource("ww");
	_isSaveAllowed = false;

	Engine::syncSoundSettings();
}

WWEngine::~WWEngine() {
	delete _random;
}

Common::Error WWEngine::run() {
	_isSaveAllowed = false;

	initGraphics(320, 200);
	initMouseCursor();
	_screen = new Screen();
	_backgroundSurface = new WWSurface(320, 150);
	_inventorySprite = new WWSurface(312, 52);
	_walkMap = new byte[kWalkMapSize];
	_backgroundScrollSurface = nullptr;
	_sound = new SoundManager(this, _mixer);
	_midi = new MusicManager(this);

	syncSoundSettings();

	_useOriginalSaveLoad = ConfMan.getBool("originalsaveload");
	if (ConfMan.hasKey("save_slot")) {
		const int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 99)
			_loadSaveSlot = saveSlot;
	}

	if (_loadSaveSlot < 0) {
		if (getGameId() == GType_Waynesworld) {
			if (getFeatures() & ADGF_DEMO)
				intro = new WWIntro_demo1(this);
			else
				intro = new WWIntro_full(this);
		} else {
			intro = new DHIntro(this);
		}
		intro->runIntro();
		delete intro;
		intro = nullptr;

		if (_gameDescription->desc.flags & ADGF_DEMO)
			return Common::kNoError;
	}

	_introOngoing = false;
	_fontWW = new GFTFont();
	_fontWWInv = new GFTFont();
	_fontBit5x7 = new GFTFont();
	_fontWW->loadFromFile("ww.gft");
	_fontWWInv->loadFromFile("wwinv.gft");
	_fontBit5x7->loadFromFile("bit5x7.gft");

	_m00Gxl = new GxlArchive("m00");
	_m01Gxl = new GxlArchive("m01");
	_m02Gxl = new GxlArchive("m02");
	_m03Gxl = new GxlArchive("m03");
	_m05Gxl = new GxlArchive("m05");

	_r10Gxl = new GxlArchive("r10");

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
	_musicIndex = 0;

	loadMainActorSprites();

	initRoomObjects();
	initStaticRoomObjects();
	memset(_wayneInventory, 0, sizeof(_wayneInventory));
	memset(_garthInventory, 0, sizeof(_garthInventory));

	setWayneInventoryItemQuantity(kObjectIdInventoryDollar, 5);
	setGarthInventoryItemQuantity(kObjectIdInventoryDrumstick, 1);

	CursorMan.showMouse(true);

	drawInterface(2);

	_gameState = 0; // DEBUG Initial _gameState 0 is set by room event in room 0
	_currentRoomNumber = 0;
	if (_loadSaveSlot >= 0) {
		_logic->loadSavegame(_loadSaveSlot);
	}
	changeRoom(_currentRoomNumber);
	changeMusic();
	_isSaveAllowed = true;

	while (!shouldQuit() && !(_logic->_r8_flags & 0x20)) {
		handleMapPalette();
		
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
	}

	if (_logic->_r8_flags & 0x20) {
		_logic->showEnding();	
	}
	
	unloadStaticRoomObjects();
	unloadMainActorSprites();

	delete _midi;
	delete _sound;
	delete _logic;

	delete _r10Gxl;

	delete _m05Gxl;
	delete _m03Gxl;
	delete _m02Gxl;
	delete _m01Gxl;
	delete _m00Gxl;

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

bool WWEngine::hasFeature(EngineFeature f) const {
	return false;
	/*
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
	*/
}

bool WWEngine::isPointAtWayne(int x, int y) {
	const int x1 = _wayneSpriteX - (_wayneActorScale * 13) / 100;
	const int x2 = _wayneSpriteX + (_wayneActorScale * 13) / 100;
	const int y1 = _wayneSpriteY - (_wayneActorScale * 48) / 100;
	const int y2 = _wayneSpriteY;
	return x >= x1 && y >= y1 && x <= x2 && y <= y2;
}

bool WWEngine::isPointAtGarth(int x, int y) {
	const int x1 = _garthSpriteX - (_garthActorScale * 13) / 100;
	const int x2 = _garthSpriteX + (_garthActorScale * 13) / 100;
	const int y1 = _garthSpriteY - (_garthActorScale * 48) / 100;
	const int y2 = _garthSpriteY;
	return x >= x1 && y >= y1 && x <= x2 && y <= y2;
}

void WWEngine::updateMouseMove() {
	// Game Menu
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
		const int inventorySlotIndex = _mouseY / 20 * 12 + _mouseX / 26;
		if (inventorySlotIndex < _inventoryItemsCount) {
			const int inventoryObjectId = _inventoryItemsObjectMap[inventorySlotIndex];
			int inventoryItemQuantity;
			if (isActorWayne()) {
				inventoryItemQuantity = getWayneInventoryItemQuantity(inventoryObjectId);
			} else {
				inventoryItemQuantity = getGarthInventoryItemQuantity(inventoryObjectId);
			}
			const char *roomObjectName = getRoomObjectName(inventoryObjectId);
			Common::String objectName;
			if (inventoryItemQuantity == 1) {
				objectName = roomObjectName;
			} else {
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
		const int objectIdAtPoint = findRoomObjectIdAtPoint(_mouseX, _mouseY);
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

void WWEngine::handleMouseLeftClick() {
	switch (_gameState) {
	case 0: // Normal Game mode
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
	case 2: // Dialog Mode
		handleDialogMouseClick();
		break;
	case 3:
		unusedTicketHandleMouseClick();
		break;
	case 4:
		_logic->handleGameMenu();
		break;
	case 5:
		extremeCloseUpHandleMouseClick();
		break;

	default:
		warning("handleMouseLeftClick - Unhandled game state %d", _gameState);
		break;
	}
}

void WWEngine::handleMouseRightClick() {
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
		_logic->handleGameMenu();
		break;
	case 5:
		extremeCloseUpHandleMouseClick();
		break;

	default:
		warning("handleMouseRightClick - Unhandled game state %d", _gameState);
		break;
	}
}

Common::String WWEngine::loadString(const char *filename, int index, int flag) {
	const uint kMaxStringLen = 60;
	char textBuffer[kMaxStringLen];
	Common::File fd;
	const Common::Path tempFilename(Common::String::format("%s.txt", filename));
	if (!fd.open(tempFilename))
		error("WWEngine::loadString() Could not open %s", tempFilename.baseName().c_str());
	fd.seek(index * kMaxStringLen);
	fd.read(textBuffer, kMaxStringLen);
	// Decrypt the string
	uint i = 0;
	for (; i < kMaxStringLen; i++) {
		textBuffer[i] ^= 0x80;
		if (textBuffer[i] == 0x2b)
			break;
	}

	// Safeguard in case the string takes 60 characters, overwrite the last character instead of writing out of boundaries
	if (i >= kMaxStringLen)
		i = kMaxStringLen - 1;
	textBuffer[i] = 0;
	return Common::String(textBuffer);
}

void WWEngine::drawCurrentText(int x, int y, WWSurface *destSurface) {
	int textCenterX, textY = 0, textColor, actorY = 0;
	const int textWidth = _fontWWInv->getTextWidth(_currentText.c_str());
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
	int textX = textCenterX - 75;
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

void WWEngine::drawInterface(int verbNum) {
	if (isActorWayne()) {
		drawImageToScreen(_m00Gxl, "winter.pcx", 0, 151);
	} else {
		drawImageToScreen(_m00Gxl, "ginter.pcx", 0, 151);
	}
	selectVerbNumber2(_verbNumber2 * 24 + 3);
	_verbNumber = verbNum;
	selectVerbNumber(_verbNumber * 24 + 3);
}

void WWEngine::selectVerbNumber2(int x) {
	// sysMouseDriver(2);
	_screen->frameRect(_verbNumber2 * 24 + 3, 165, _verbNumber2 * 24 + 3 + 23, 198, 0);
	_verbNumber2 = (x - 3) / 24;
	_screen->frameRect(_verbNumber2 * 24 + 3, 165, _verbNumber2 * 24 + 3 + 23, 198, 10);
	// sysMouseDriver(1);
}

void WWEngine::selectVerbNumber(int x) {
	const int selectedButtonIndex = (x - 3) / 24;
	_firstObjectNumber = -1;
	if (selectedButtonIndex > 10) {
		if (_mouseClickY > 182) {
			if (x < 290) {
				drawInventory();
				refreshActors();
			} else {
				_logic->r0_handleRoomEvent1();
			}
		}
	} else if (selectedButtonIndex == 0) {
		changeActor();
	} else {
		_verbNumber = selectedButtonIndex;
		drawVerbLine(_verbNumber, -4, nullptr);
		if (_verbNumber == 8 && _inventoryItemsCount == 0) {
			drawInventory();
			refreshActors();
		}
	}
}

void WWEngine::changeActor() {
	if (_currentRoomNumber == 31)
		return;
	toggleActor();
	drawInterface(_verbNumber);
	_isTextVisible = false;
	refreshInventory(true);
}

void WWEngine::drawVerbLine(int verbNumber, int objectNumber, const char *objectName) {
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

void WWEngine::rememberFirstObjectName(int objectId) {
	if (objectId == -2) {
		_firstObjectName = "Wayne";
	} else if (objectId == -3) {
		_firstObjectName = "Garth";
	} else if (objectId <= kLastInventoryObjectId && objectId >= kFirstInventoryObjectId) {
		int inventoryItemQuantity;
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

void WWEngine::drawInventory() {
	if (_inventoryItemsCount != 0) {
		_inventoryItemsCount = 0;
		return;
	}
	int iconX = 0;
	int iconY = 0;
	_inventorySprite->clear(0);
	for (int inventoryItemIndex = 0; inventoryItemIndex < kInventorySize; inventoryItemIndex++) {
		const int objectRoomNumber = getObjectRoom(inventoryItemIndex + kFirstInventoryObjectId);
		if ((isActorWayne() && objectRoomNumber == 99 && _wayneInventory[inventoryItemIndex] > 0) ||
			(isActorGarth() && objectRoomNumber == 99 && _garthInventory[inventoryItemIndex] > 0)) {
			Common::String filename = Common::String::format("icon%02d.pcx", inventoryItemIndex + 1);
			debug("filename: [%s]", filename.c_str());
			drawImageToSurface(_m03Gxl, filename.c_str(), _inventorySprite, iconX, iconY);
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

void WWEngine::setWayneInventoryItemQuantity(int objectId, int quantity) {
	_wayneInventory[objectId - kFirstInventoryObjectId] = quantity;
}

void WWEngine::setGarthInventoryItemQuantity(int objectId, int quantity) {
	_garthInventory[objectId - kFirstInventoryObjectId] = quantity;
}

int WWEngine::getWayneInventoryItemQuantity(int objectId) {
	return _wayneInventory[objectId - kFirstInventoryObjectId];
}

int WWEngine::getGarthInventoryItemQuantity(int objectId) {
	return _garthInventory[objectId - kFirstInventoryObjectId];
}

void WWEngine::loadMainActorSprites() {
	// _inventorySprite = new WWSurface(312, 52);
	_wayneReachRightSprite = _m01Gxl->loadSurface("wreachr.pcx");
	_wayneReachLeftSprite = _m01Gxl->loadSurface("wreachl.pcx");
	_garthReachRightSprite = _m01Gxl->loadSurface("greachr.pcx");
	_garthReachLeftSprite = _m01Gxl->loadSurface("greachl.pcx");
	for (int direction = 0; direction < 8; direction++) {
		_wayneSprites[direction] = _m01Gxl->loadSurface(Common::String::format("wstand%d.pcx", direction).c_str());
		_garthSprites[direction] = _m01Gxl->loadSurface(Common::String::format("gstand%d.pcx", direction).c_str());
		for (int frameNum = 0; frameNum < 4; frameNum++) {
			_wayneWalkSprites[direction][frameNum] = _m01Gxl->loadSurface(Common::String::format("wwalk%d%d.pcx", direction, frameNum).c_str());
			_garthWalkSprites[direction][frameNum] = _m01Gxl->loadSurface(Common::String::format("gwalk%d%d.pcx", direction, frameNum).c_str());
		}
	}
}

void WWEngine::unloadMainActorSprites() {
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

void WWEngine::drawActorReachObject(int objectId, int spriteIndex) {
	const int direction = getObjectDirection(objectId);
	if (isActorWayne()) {
		drawActors(direction, 2, 1, spriteIndex, _wayneSpriteX, _wayneSpriteY, _garthSpriteX, _garthSpriteY);
	} else {
		drawActors(direction, 1, 2, spriteIndex, _wayneSpriteX, _wayneSpriteY, _garthSpriteX, _garthSpriteY);
	}
}

int WWEngine::drawActors(int direction, int wayneKind, int garthKind, int spriteIndex, int wayneX, int wayneY, int garthX, int garthY) {
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
		const int scaledWayneWidth = _wayneActorScale * 27 / 100;
		const int scaledWayneHeight = _wayneActorScale * 48 / 100;
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
		garthHeight = (100 - _garthActorScale) * 48 / 90;
		garthWidth = _garthActorScale * 13 / 100;
		const int scaledGarthWidth = _garthActorScale * 27 / 100;
		const int scaledGarthHeight = _garthActorScale * 48 / 100;
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

#ifdef DEBUG_PATHFIND // DEBUG Draw room mask to background
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
		warning("CHECKME - drawActors - drawStaticRoomObjects2(tempBackground);");
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

void WWEngine::refreshActors() {
	drawActors(_actorSpriteValue, _wayneKind, _garthKind, _actorSpriteIndex, _wayneSpriteX, _wayneSpriteY, _garthSpriteX, _garthSpriteY);
}

void WWEngine::pickupObject(int objectId, byte &flags, byte flagsSet, int inventoryObjectId) {
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

void WWEngine::setWaynePosition(int x, int y) {
	_wayneSpriteX = x;
	_wayneSpriteY = y;
}

void WWEngine::setGarthPosition(int x, int y) {
	_garthSpriteX = x;
	_garthSpriteY = y;
}

bool WWEngine::isActorWayne() {
	return _currentActorNum != 0;
}

bool WWEngine::isActorGarth() {
	return _currentActorNum == 0;
}

void WWEngine::selectActorWayne() {
	_currentActorNum = 1;
}

void WWEngine::selectActorGarth() {
	_currentActorNum = 0;
}

void WWEngine::toggleActor() {
	if (isActorWayne()) {
		selectActorGarth();
	} else {
		selectActorWayne();
	}
}

void WWEngine::openRoomLibrary(int roomNum) {
	_roomName = Common::String::format("r%02d", roomNum);
	delete _roomGxl;

	_roomGxl = new GxlArchive(_roomName);
}

void WWEngine::changeRoomScrolling() {
	int roomNumber = -1;
	_scrollPosition = 0;
	_scrollWidth = 0;
	_scrollRemaining = 0;
	switch (_currentRoomNumber) {
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
	case 14:
		roomNumber = 19;
		break;
	case 19:
		roomNumber = 14;
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
	case 32:
		roomNumber = 33;
		break;
	case 33:
		roomNumber = 32;
		break;

	default:
		break;
	}
	unloadStaticRoomObjects();
	openRoomLibrary(roomNumber);
	loadRoomMask(roomNumber);
	loadStaticRoomObjects(roomNumber);
	_currentRoomNumber = roomNumber;
}

void WWEngine::loadScrollSprite() {
	if (_currentRoomNumber == 14 && _hoverObjectNumber == kObjectIdLoadingDock) {
		_backgroundScrollSurface = new WWSurface(112, 150);
		drawRoomImageToSurface("scroll.pcx", _backgroundScrollSurface, 0, 0);
		if ((_logic->_r1_flags1 & 0x10) && !(_logic->_pizzathonListFlags1 & 0x04)) {
			drawRoomImageToSurface("gill0.pcx", _backgroundScrollSurface, 65, 84);
		}
		_scrollRemaining = 112;
		_scrollWidth = 112;
		_doScrollRight = true;
	} else if (_currentRoomNumber == 19 && _hoverObjectNumber == kObjectIdStore) {
		_backgroundScrollSurface = new WWSurface(112, 150);
		GxlArchive *r14Gxl = new GxlArchive("r14");
		drawImageToSurface(r14Gxl, "backg.pcx", _backgroundScrollSurface, 0, 0);
		delete r14Gxl;
		_scrollRemaining = 112;
		_scrollWidth = 112;
		_doScrollRight = false;
	} else if (_currentRoomNumber == 8 && _hoverObjectNumber == kObjectIdHallway8) {
		stopRoomAnimations();
		_backgroundScrollSurface = new WWSurface(320, 150);
		GxlArchive *r21Gxl = new GxlArchive("r21");
		drawImageToSurface(r21Gxl, "backg.pcx", _backgroundScrollSurface, 0, 0);
		delete r21Gxl;
		_scrollRemaining = 320;
		_scrollWidth = 320;
		_doScrollRight = false;
	} else if (_currentRoomNumber == 21 && _hoverObjectNumber == kObjectIdFoyer) {
		_backgroundScrollSurface = new WWSurface(320, 150);
		GxlArchive *r08Gxl = new GxlArchive("r08");
		drawImageToSurface(r08Gxl, "backg.pcx", _backgroundScrollSurface, 0, 0);
		delete r08Gxl;
		_scrollRemaining = 320;
		_scrollWidth = 320;
		_doScrollRight = true;
	} else if (_currentRoomNumber == 21 && _hoverObjectNumber == kObjectIdOffice21) {
		_backgroundScrollSurface = new WWSurface(320, 150);
		GxlArchive *r22Gxl = new GxlArchive("r22");
		drawImageToSurface(r22Gxl, "backg.pcx", _backgroundScrollSurface, 0, 0);
		delete r22Gxl;
		_scrollRemaining = 320;
		_scrollWidth = 320;
		_doScrollRight = false;
	} else if (_currentRoomNumber == 22 && _hoverObjectNumber == kObjectIdHallway22) {
		_backgroundScrollSurface = new WWSurface(320, 150);
		GxlArchive *r21Gxl = new GxlArchive("r21");
		drawImageToSurface(r21Gxl, "backg.pcx", _backgroundScrollSurface, 0, 0);
		delete r21Gxl;
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
			GxlArchive *r33Gxl = new GxlArchive("r33");
			drawImageToSurface(r33Gxl, "backg.pcx", _backgroundScrollSurface, 0, 0);
			delete r33Gxl;
			_scrollRemaining = 168;
			_scrollWidth = 168;
			_doScrollRight = false;
		}
	} else if (_currentRoomNumber == 33 && _hoverObjectNumber == kObjectIdHallway33) {
		_backgroundScrollSurface = new WWSurface(168, 150);
		drawRoomImageToSurface("scroll.pcx", _backgroundScrollSurface, 0, 0);
		if (_logic->_r32_flags & 0x02) {
			drawRoomImageToSurface("noplunge.pcx", _backgroundScrollSurface, 141, 94);
		}
		_scrollRemaining = 168;
		_scrollWidth = 168;
		_doScrollRight = true;
	} else {
		_scrollPosition = 0;
	}
}

void WWEngine::scrollRoom() {
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

void WWEngine::loadRoomMask(int roomNum) {
	const Common::Path filename(Common::String::format("r%02d.msk", roomNum));
	Common::File fd;
	if (!fd.open(filename))
		error("WWEngine::loadRoomMask() Could not open %s", filename.baseName().c_str());
	if (fd.size() != kWalkMapSize)
		error("WWEngine::loadRoomMask() Wrong file size in %s", filename.baseName().c_str());
	fd.read(_walkMap, kWalkMapSize);
}

void WWEngine::moveObjectToRoom(int objectId, int roomNum) {
	_roomObjects[objectId].roomNumber = roomNum;
	if (objectId <= kLastInventoryObjectId && objectId >= kFirstInventoryObjectId) {
		const int inventoryIndex = objectId - kFirstInventoryObjectId;
		if (isActorWayne()) {
			_wayneInventory[inventoryIndex]++;
		} else {
			_garthInventory[inventoryIndex]++;
		}
	}
}

void WWEngine::moveObjectToNowhere(int objectId) {
	if (objectId <= kLastInventoryObjectId && objectId >= kFirstInventoryObjectId) {
		const int inventoryIndex = objectId - kFirstInventoryObjectId;
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

void WWEngine::walkToObject() {
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

void WWEngine::startDialog() {
	redrawInventory();
	_gameState = 2;
	drawDialogChoices(-9);
}

void WWEngine::setDialogChoices(int choice1, int choice2, int choice3, int choice4, int choice5) {
	_dialogChoices[0] = choice1;
	_dialogChoices[1] = choice2;
	_dialogChoices[2] = choice3;
	_dialogChoices[3] = choice4;
	_dialogChoices[4] = choice5;
}

void WWEngine::drawDialogChoices(int choiceIndex) {
	// sysMouseDriver(2);
	if (choiceIndex == -9) {
		// Redraw all and clear the background
		_screen->fillRect(0, 151, 319, 199, 0);
	}
	byte choiceTextColor = isActorWayne() ? 11 : 10;
	byte selectedTextColor = 13;
	for (int index = 0; index < 5 && _dialogChoices[index] != -1; index++) {
		const byte textColor = index + choiceIndex + 11 == 0 ? selectedTextColor : choiceTextColor;
		Common::String dialogText = loadString("c04", _dialogChoices[index], 0);
		_screen->drawText(_fontBit5x7, dialogText.c_str(), 3, 152 + index * 9, textColor);
	}
	// sysMouseDriver(1);
	_selectedDialogChoice = choiceIndex;
}

void WWEngine::handleDialogMouseClick() {
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

	const int continueDialog = _logic->handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);

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
	} else if (continueDialog != 0 || replyTextIndex1 != -1) {
		drawDialogChoices(-9);
	}
}

void WWEngine::handleVerb(int verbFlag) {
	const int verbNum = verbFlag == 1 ? _verbNumber : _verbNumber2;
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
	default:
		break;
	}

	if (_scrollWidth != 0) {
		changeRoomScrolling();
	}

}

void WWEngine::handleVerbPickUp() {
	_isTextVisible = false;
	if (_objectNumber <= kLastInventoryObjectId) {
		refreshActors();
	}

	int actionTextIndex = _logic->handleVerbPickUp();

	if (_objectNumber == -3) {
		actionTextIndex = 29;
	} else if (_objectNumber == -2) {
		actionTextIndex = 30;
	}

	if (actionTextIndex != -1) {
		displayText("c01", actionTextIndex, 0, -1, -1, 0);
	}

	switch (actionTextIndex) {
	case 0:
	case 10:
	case 44:
	case 54:
		playSound("sv32.snd", true);
		break;
	case 7:
		playSound("sv18.snd", true);
		playSound("sv28.snd", true);
		break;
	case 13:
		playSound("sv19.snd", true);
		break;
	case 50:
		playSound("sv33.snd", true);
		break;
	case 51:
	case 59:
		playSound("sv31.snd", true);
		break;
	case 53:
		playSound("ss07.snd", true);
		break;
	case 55:
		playSound("sv29.snd", true);
		break;
	case 56:
		playSound("sv38.snd", true);
		break;
	default:
		break;
	}

}

void WWEngine::handleVerbLookAt() {
	_isTextVisible = false;
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

void WWEngine::handleVerbUse() {
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

	int actionTextIndex = _logic->handleVerbUse();

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
		playSound("sv18.snd", true);
		break;
	case 67:
		playSound("ss07.snd", true);
		break;
	case 68:
		playSound("sv32.snd", true);
		break;
	case 69:
		playSound("sv47.snd", true);
		break;
	case 70:
		playSound("sv39.snd", true);
		break;
	case 71:
		playSound("sv38.snd", true);
		break;
	default:
		break;
	}
}

void WWEngine::handleVerbTalkTo() {
	_dialogChoices[0] = -1;

	if (_objectNumber == -3 || _objectNumber == -2) {
		const bool room1Special = !(_logic->_r1_flags1 & 0x10) && isActorWayne() && _currentRoomNumber == 1 && (_logic->_r1_flags2 & 0x01);
		int actionTextIndex;
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

void WWEngine::handleVerbPush() {
	_isTextVisible = false;
	if (_objectNumber <= kLastInventoryObjectId) {
		refreshActors();
	}

	const int actionTextIndex = _logic->handleVerbPush();

	if (actionTextIndex != -1) {
		displayText("c05", actionTextIndex, 0, -1, -1, 0);
	}

	switch (actionTextIndex) {
	case 0:
		playSound("sv02.snd", true);
		break;
	case 4:
		playSound("sv47.snd", true);
		break;
	default:
		break;
	}

}

void WWEngine::handleVerbPull() {
	_isTextVisible = false;
	if (_objectNumber <= kLastInventoryObjectId) {
		refreshActors();
	}

	const int actionTextIndex = _logic->handleVerbPull();

	if (actionTextIndex != -1) {
		displayText("c06", actionTextIndex, 0, -1, -1, 0);
	}

	if (actionTextIndex == 0)
		playSound("sv31.snd", true);

}

void WWEngine::handleVerbExtremeCloseupOf() {
	_logic->handleVerbExtremeCloseupOf();
}

void WWEngine::handleVerbGive() {
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
			const int inventoryIndex = _firstObjectNumber - kFirstInventoryObjectId;
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

	const int actionTextIndex = _logic->handleVerbGive();

	_firstObjectNumber = -1;

	if (actionTextIndex != -1) {
		displayText("c08", actionTextIndex, 0, -1, -1, 0);
	}

}

void WWEngine::handleVerbOpen() {
	_isTextVisible = false;
	if (_objectNumber <= kLastInventoryObjectId) {
		refreshActors();
	}

	const int actionTextIndex = _logic->handleVerbOpen();

	if (actionTextIndex != -1) {
		displayText("c09", actionTextIndex, 0, -1, -1, 0);
	}

	switch (actionTextIndex) {
	case 0:
		playSound("sv28.snd", true);
		break;
	case 1:
		playSound("sv38.snd", true);
		break;
	case 5:
		playSound("sv31.snd", true);
		break;
	case 7:
		playSound("ss07.snd", true);
		break;
	case 8:
		playSound("sv47.snd", true);
		break;
	case 10:
		playSound("sv28.snd", true);
		break;
	case 11:
		playSound("sv21.snd", true);
		break;
	default:
		break;
	}
}

void WWEngine::handleVerbClose() {
	_isTextVisible = false;
	if (_objectNumber <= kLastInventoryObjectId) {
		refreshActors();
	}

	const int actionTextIndex = _logic->handleVerbClose();

	if (actionTextIndex != -1) {
		displayText("c10", actionTextIndex, 0, -1, -1, 0);
	}

	switch (actionTextIndex) {
	case 0:
		playSound("sv47.snd", true);
		break;
	case 1:
		playSound("sv21.snd", true);
		break;
	default:
		break;
	}

}

void WWEngine::lookAtUnusedTicket() {
	// Move the mouse cursor out of the ticket so it's not directly turned into a used ticket
	_mouseX = _mouseY = 10;
	g_system->warpMouse(_mouseX, _mouseY);

	// sysMouseDriver(2);
	_gameState = 3;
	_logic->_didScratchTicket = false;
	stopRoomAnimations();
	if (!(_logic->_r10_flags & 0x80)) {
		_roomAnimations[19] = _r10Gxl->loadSurface("win.pcx");
	} else {
		_roomAnimations[19] = _r10Gxl->loadSurface("nowin.pcx");
	}
	paletteFadeOut(0, 256, 64);
	_screen->clear(0);
	playSound("sv14.snd", false);
	drawImageToScreen(_r10Gxl, "ticket.pcx", 0, 13);
	paletteFadeIn(0, 256, 64);
	// sysMouseDriver(1);
}

void WWEngine::unusedTicketHandleMouseMove() {
	if (_mouseX > 157 && _mouseY > 38 && _mouseX < 297 && _mouseY < 129) {
		_logic->_didScratchTicket = true;
		// Reveal partial image
		const int scratchX = _mouseX - 158;
		const int scratchY = _mouseY - 39;
		Graphics::Surface scratchSurface = _roomAnimations[19]->getSubArea(Common::Rect(scratchX, scratchY, scratchX + 4, scratchY + 4));
		_screen->drawSurface(&scratchSurface, _mouseX - 2, _mouseY - 2);
	}
}

void WWEngine::unusedTicketHandleMouseClick() {
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

void WWEngine::extremeCloseUpHandleMouseClick() {
	_gameState = 0;
	// sysMouseDriver(2);
	paletteFadeOut(0, 256, 16);
	_screen->clear(0);
	paletteFadeIn(0, 256, 16);
	drawInterface(_verbNumber);
	loadRoomBackground();
	// sysMouseDriver(1);
}

} // End of namespace WW
