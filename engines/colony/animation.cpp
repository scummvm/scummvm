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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "colony/colony.h"
#include "colony/renderer.h"
#include "colony/sound.h"

namespace Colony {

void responsiveAnimationDelay(OSystem *system, uint32 delayMs) {
	if (!system || delayMs == 0)
		return;

	Common::Array<Common::Event> deferredEvents;
	uint mouseMoveCount = 0;
	const uint32 start = system->getMillis();
	uint32 elapsed = 0;
	while (elapsed < delayMs) {
		Common::Event event;
		while (system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (event.typ
e == Common::EVENT_MOUSEMOVE) {
				mouseMoveCount++;
			} else {
				deferredEvents.push_back(event);
			}
		}

		system->updateScreen();
		const uint32 slice = MIN<uint32>(2, delayMs - elapsed);
		system->delayMillis(slice);
		elapsed = system->getMillis() - start;
	}

	for (uint i = 0; i < deferredEvents.size(); ++i)
		system->getEventManager()->pushEvent(deferredEvents[i]);

	if (mouseMoveCount || !deferredEvents.empty()) {
		debugC(5, kColonyDebugAnimation,
			"responsiveAnimationDelay(%u): mouseMoves=%u deferred=%u",
			delayMs, mouseMoveCount, (uint)deferredEvents.size());
	}
}

bool isBackdoorCode111111(const uint8 display[6]) {
	for (int i = 0; i < 6; i++) {
		if (display[i] != 3)
			return false;
	}

	return true;
}

// Mac color indices from colordef.h enum (cColor[] table in Color256).
enum {
	kMcDwall = 6, kMcLwall = 7,
	kMcChar0 = 8,    // char0..char6 = 8..14
	kMcBulkhead = 15, kMcDoor = 16,
	kMcDesk = 58, kMcDesktop = 59, kMcScreen = 62,
	kMcProj = 72, kMcConsole = 79, kMcPowerbase = 81,
	kMcBox1 = 84, kMcForklift = 86, kMcFlglass = 87,
	kMcCryo = 90, kMcCcore = 111,
	kMcTeleport = 93, kMcTeledoor = 94,
	kMcVanity = 96, kMcMirror = 103,
	kMcAirlock = 25, kMcElevator = 23
};

// Mac Toolbox BackColor() constants.
enum {
	kMacWhite = 30, kMacBlack = 33,
	kMacYellow = 69, kMacMagenta = 137,
	kMacRed = 205, kMacCyan = 273,
	kMacGreen = 341, kMacBlue = 409
};

// BMColor arrays from ganimate.c  per-animation color maps.
// Index 0 = background top, 1 = background image, 2+ = per-sprite fill.
// Positive = cColor[] index, negative = -MacSystemColor, 0 = level-based.
const int16 kBMC_Desk[] = {
	0, kMcDesktop,
	-kMacRed, -kMacCyan, -kMacCyan, -kMacCyan, -kMacCyan,
	-kMacWhite, -kMacWhite, -kMacMagenta, -kMacYellow, kMcDesk,
	kMcDesk, kMcDesk, kMcDesk, kMcDesk, kMcDesk,
	-kMacWhite, kMcScreen, -kMacMagenta, -kMacCyan, -kMacCyan,
	-kMacBlue, -kMacWhite, -kMacRed, -kMacWhite, -kMacYellow
};
const int16 kBMC_Vanity[] = {
	0, kMcVanity,
	kMcMirror, -kMacRed, -kM
acCyan, -kMacWhite, -kMacYellow,
	-kMacGreen, -kMacBlue, -kMacRed, -kMacMagenta, -kMacRed,
	kMcVanity, -kMacWhite, -kMacYellow, kMcMirror
};
const int16 kBMC_Reactor[] = {
	0, kMcConsole,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacCyan,
	-kMacMagenta, -kMacWhite
};
const int16 kBMC_Security[] = {
	0, kMcConsole,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacWhite,
	-kMacRed, -kMacCyan, -kMacCyan, -kMacCyan, -kMacCyan
};
const int16 kBMC_Teleport[] = {
	0, kMcTeleport, 0, kMcTeledoor
};
const int16 kBMC_Creatures[] = {
	-kMacWhite, 0, -kMacWhite, -kMacCyan, kMcProj,
	-kMacBlue, -kMacMagenta, -kMacMagenta
};
const int16 kBMC_Controls[] = {
	0, kMcConsole,
	-kMacRed, -kMacYellow, -kMacYellow, -kMacBlue, -kMacYellow, -kMacGreen, kMcScreen
};
const int16 kBMC_Lift[] = {
	0, kMcFlglass,
	kMcTeleport, kMcBox1, kMcCryo, kMcCcore, 0,
	-kMacRed, -kMacRed, -kMacCyan, -kMacCyan
};
const int16 kBMC_Powersuit[] = {
	0, kMcPowerbase,
	-kMacMagenta, -kMacMagenta, -kMacYellow, -kMacYellow, kMcPowerbase, -kMacWhite
};
const int16 kBMC_Forklift[] = {
	0, kMcForklift, kMcForklift, kMcForklift
};
const int16 kBMC_Door[] = {
	0, kMcBulkhead, 0, kMcDoor, -kMacYellow
};
const int16 kBMC_Bulkhead[] = {
	0, kMcBulkhead, 0, kMcBulkhead, -kMacYellow
};
const int16 kBMC_Airlock[] = {
	0, kMcBulkhead, kMcBulkhead, -kMacRed, kMcAirlock
};
const int16 kBMC_Elevator[] = {
	0, kMcBulkhead, 0, kMcElevator, kMcElevator, -kMacYellow
};
const int16 kBMC_Elevator2[] = {
	0, kMcBulkhead, 0, -kMacMagenta, kMcElevator, kMcElevator,
	
-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow
};

struct AnimColorEntry {
	const char *name;
	const int16 *colors;
	int count;
};

const AnimColorEntry kAnimColors[] = {
	{ "desk",        kBMC_Desk,       ARRAYSIZE(kBMC_Desk) },
	{ "vanity",      kBMC_Vanity,     ARRAYSIZE(kBMC_Vanity) },
	{ "reactor",     kBMC_Reactor,    ARRAYSIZE(kBMC_Reactor) },
	{ "security",    kBMC_Security,   ARRAYSIZE(kBMC_Security) },
	{ "tele",        kBMC_Teleport,   ARRAYSIZE(kBMC_Teleport) },
	{ "teleporter",  kBMC_Teleport,   ARRAYSIZE(kBMC_Teleport) },
	{ "teleporter2", kBMC_Teleport,   ARRAYSIZE(kBMC_Teleport) },
	{ "slides",      kBMC_Creatures,  ARRAYSIZE(kBMC_Creatures) },
	{ "slideshow",   kBMC_Creatures,  ARRAYSIZE(kBMC_Creatures) },
	{ "teleshow",    kBMC_Creatures,  ARRAYSIZE(kBMC_Creatures) },
	{ "controls",    kBMC_Controls,   ARRAYSIZE(kBMC_Controls) },
	{ "lift",        kBMC_Lift,       ARRAYSIZE(kBMC_Lift) },
	{ "lifter",      kBMC_Lift,       ARRAYSIZE(kBMC_Lift) },
	{ "suit",        kBMC_Powersuit,  ARRAYSIZE(kBMC_Powersuit) },
	{ "spacesuit",   kBMC_Powersuit,  ARRAYSIZE(kBMC_Powersuit) },
	{ "forklift",    kBMC_Forklift,   ARRAYSIZE(kBMC_Forklift) },
	{ "door",        kBMC_Door,       ARRAYSIZE(kBMC_Door) },
	{ "bulkhead",    kBMC_Bulkhead,   ARRAYSIZE(kBMC_Bulkhead) },
	{ "airlock",     kBMC_Airlock,    ARRAYSIZE(kBMC_Airlock) },
	{ "elev",        kBMC_Elevator,   ARRAYSIZE(kBMC_Elevator) },
	{ "elevator",    kBMC_Elevator,   ARRAYSIZE(kBMC_Elevator) },
	{ "elevator2",   kBMC_Elevator2,  ARRAYSIZE(kBMC_Elevator2) },
	{ nullptr, nullptr, 0 }
};

// Desk animation sprite indices (from desk.pic ComplexSprite layout).
enum {
	kDeskTeeth = 6,
	kDeskLetterEnv = 7,    // letter envelope
	kDeskLetterOpen = 8,   // letter opened
	kDeskClipboard = 9,
	kDeskScreen = 17,
	kDeskJackFirst = 18,   // jack-in-the-box sprites 18..21
	kDeskJackLast = 21,
	kDeskBook = 22,
	kDeskCigarette1 = 23,
	kDeskCigarette2 = 24,
	kDeskPostIt = 25
};

// Convert Mac Toolbox BackC
olor constant to ARGB.
uint32 macSysColorToARGB(int sysColor) {
	switch (sysColor) {
	case kMacWhite:   return 0xFFFFFFFF;
	case kMacBlack:   return 0xFF000000;
	case kMacYellow:  return 0xFFFFFF00;
	case kMacMagenta: return 0xFFFF00FF;
	case kMacRed:     return 0xFFFF0000;
	case kMacCyan:    return 0xFF00FFFF;
	case kMacGreen:   return 0xFF00FF00;
	case kMacBlue:    return 0xFF0000FF;
	default:          return 0xFFFFFFFF;
	}
}

int getAnimationStateCount(const Common::Array<ComplexSprite *> &sprites, int num) {
	num--;
	if (num >= 0 && num < (int)sprites.size()) {
		int count = (int)sprites[num]->objects.size();
		return (count > 0) ? count : 1;
	}

	return 1;
}

bool ColonyEngine::loadAnimation(const Common::String &name) {
	_animationName = name;
	for (int i = 0; i < 6; i++)
		_animDisplay[i] = 1;

	// Look up per-animation BMColor map (from ganimate.c).
	_animBMColors.clear();
	Common::String nameLower = name;
	nameLower.toLowercase();
	for (const AnimColorEntry *e = kAnimColors; e->name; e++) {
		if (nameLower == e->name) {
			_animBMColors.resize(e->count);
			for (int i = 0; i < e->count; i++)
				_animBMColors[i] = e->colors[i];
			break;
		}
	}

	// DOS uses short names with .pic extension (suit.pic, elev.pic, etc.);
	// Mac uses full names without extensions (spacesuit, elevator, etc.)
	// stored in a CData folder (added to SearchMan at engine start).
	const struct { const char *dosName; const char *macName; } nameMap[] = {
		{ "suit",   "spacesuit" },
		{ "elev",   "elevator" },
		{ "slides", "slideshow" },
		{ "lift",   "lifter" },
		{ "tele",   "teleporter" },
		{ nullptr,  nullptr }
	};

	Common::String fileName = name + ".pic";
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
	if (!file) {
		// Try without extension (Mac resource fork — SearchMan is case-agnostic)
		file = Common::MacResManager::openFileOrDataFork(Common::Path(name));
	}
	if (!file) {
		// Try Mac long name mapping (e.g. "suit" -> 
"spacesuit")
		for (int i = 0; nameMap[i].dosName; i++) {
			if (nameLower == nameMap[i].dosName) {
				file = Common::MacResManager::openFileOrDataFork(Common::Path(nameMap[i].macName));
				break;
			}
		}
	}
	if (!file) {
		warning("Could not open animation file %s", name.c_str());
		return false;
	}

	bool isBE = (getPlatform() == Common::kPlatformMacintosh);
	Common::SeekableReadStreamEndianWrapper stream(file, isBE, DisposeAfterUse::YES);

	deleteAnimation();

	// Read background data
	stream.read(_topBG, 8);
	stream.read(_bottomBG, 8);
	_divideBG = stream.readSint16();
	_backgroundActive = stream.readSint16() != 0;
	if (_backgroundActive) {
		_backgroundClip = readRect(stream);
		_backgroundLocate = readRect(stream);
		_backgroundMask = loadImage(stream);
		_backgroundFG = loadImage(stream);
	}

	// Read sprite data
	int16 maxsprite = stream.readSint16();
	stream.readSint16(); // locSprite
	for (int i = 0; i < maxsprite; i++) {
		Sprite *s = new Sprite();
		s->fg = loadImage(stream);
		s->mask = loadImage(stream);
		s->used = stream.readSint16() != 0;
		s->clip = readRect(stream);
		s->locate = readRect(stream);
		_cSprites.push_back(s);
	}

	// Read complex sprite data
	int16 maxLSprite = stream.readSint16();
	stream.readSint16(); // anum
	for (int i = 0; i < maxLSprite; i++) {
		ComplexSprite *ls = new ComplexSprite();
		int16 size = stream.readSint16();
		for (int j = 0; j < size; j++) {
			ComplexSprite::SubObject sub;
			sub.spritenum = stream.readSint16();
			sub.xloc = stream.readSint16();
			sub.yloc = stream.readSint16();
			ls->objects.push_back(sub);
		}
		ls->bounds = readRect(stream);
		ls->visible = stream.readSint16() != 0;
		ls->current = stream.readSint16();
		ls->xloc = stream.readSint16();
		ls->yloc = stream.readSint16();
		ls->acurrent = stream.readSint16();
		ls->axloc = stream.readSint16();
		ls->ayloc = stream.readSint16();
		ls->type = stream.readByte();
		ls->frozen = stream.readByte();
		ls->locked = stream.readByte();
		ls->link = 
stream.readSint16();
		ls->key = stream.readSint16();
		ls->lock = stream.readSint16();
		ls->onoff = true;
		_lSprites.push_back(ls);
	}

	return true;
}

bool ColonyEngine::loadLiftAnimation(int objectType) {
	switch (objectType) {
	case kObjTeleport:
		_liftObject = 1;
		break;
	case kObjBox1:
	case kObjBox2:
		_liftObject = 2;
		break;
	case kObjCryo:
		_liftObject = 3;
		break;
	case kObjReactor:
		_liftObject = 4;
		break;
	default:
		return false;
	}

	return loadAnimation("lift");
}

void ColonyEngine::deleteAnimation() {
	delete _backgroundMask;
	_backgroundMask = nullptr;
	delete _backgroundFG;
	_backgroundFG = nullptr;
	if (_backgroundBaked) {
		_backgroundBaked->free();
		delete _backgroundBaked;
		_backgroundBaked = nullptr;
	}
	_backgroundBakedKey = 0;
	for (uint i = 0; i < _cSprites.size(); i++)
		delete _cSprites[i];
	_cSprites.clear();
	for (uint i = 0; i < _lSprites.size(); i++)
		delete _lSprites[i];
	_lSprites.clear();
}

void ColonyEngine::playAnimation() {
	// Clear movement flags so held keys don't re-trigger on return
	_moveForward = false;
	_moveBackward = false;
	_strafeLeft = false;
	_strafeRight = false;
	_rotateLeft = false;
	_rotateRight = false;

	_animationRunning = true;
	_animExitPressed = _animExitInside = false;
	_animExitStrip = _animExitButton = Common::Rect();
	_coderCursor = 0;
	for (int i = 0; i < 4; i++)
		_coderPick[i] = 0;
	_coderWin = Common::Rect();
	_coderPressed = -1;
	_coderPressInside = false;
	const bool useSquarePixelViewport = !isMacRenderMode();
	if (useSquarePixelViewport)
		_gfx->setSquarePixelViewport(true);
	_system->lockMouse(false);
	warpMouseLogical(_centerX, _centerY);
	const char *cursorName = "default arrow cursor";
	if (isMacRenderMode() && _macArrowCursor) {
		cursorName = "Mac arrow cursor";
		CursorMan.replaceCursor(_macArrowCursor);
	} else {
		CursorMan.setDefaultArrowCursor(true);
	}
	CursorMan.showMouse(true);
	_system->updateScreen();
	warning(
		"Colony animation cursor: %s uses %s at center=(
%d,%d)",
		_animationName.c_str(), cursorName, _centerX, _centerY);

	if (_animationName == "reactor" || _animationName == "security") {
		for (int i = 0; i < 6; i++) {
			_animDisplay[i] = 1;
			setObjectOnOff(14 + i * 2, false);
			setObjectOnOff(13 + i * 2, true);
			setObjectState(13 + i * 2, 1);
		}
	}

	if (_animationName == "security") {
		for (int i = 0; i < 4; i++)
			setObjectState(27 + i, _unlocked ? _decode1[i] : 1);

		if (!_unlocked) {
			for (int i = 0; i < 4; i++) {
				_decode1[i] = (uint8)(2 + _randomSource.getRandomNumber(3));
				setObjectState(27 + i, _decode1[i]);
			}
		}
	} else if (_animationName == "controls") {
		switch (_corePower[_coreIndex]) {
		case 0: setObjectState(2, 1); setObjectState(5, 1); break;
		case 1: setObjectState(2, 1); setObjectState(5, 2); break;
		case 2: setObjectState(2, 2); setObjectState(5, 1); break;
		}
	} else if (_animationName == "desk") {
		if (!(_action0 == 11 || _action0 == 18)) {
			for (int i = 1; i <= 5; i++)
				setObjectOnOff(i, false);
		} else {
			uint8 *decode = (_action0 == 11) ? _decode2 : _decode3;
			for (int i = 0; i < 4; i++) {
				if (decode[i])
					setObjectState(i + 2, decode[i]);
				else
					setObjectState(i + 2, 1);
			}
		}

		if (_action0 != 10) {
			setObjectOnOff(kDeskCigarette1, false);
			setObjectOnOff(kDeskCigarette2, false);
		}
		if (_action0 != 30)
			setObjectOnOff(kDeskTeeth, false);
		if (_action0 != 33) {
			for (int i = kDeskJackFirst; i <= kDeskJackLast; i++)
				setObjectOnOff(i, false);
		}

		int ntype = _action1 / 10;
		switch (ntype) {
		case 0:
		case 1:
		case 2:
		case 3:
			setObjectOnOff(kDeskLetterEnv, false);
			setObjectOnOff(kDeskLetterOpen, false);
			setObjectOnOff(kDeskClipboard, false);
			setObjectOnOff(kDeskBook, false);
			setObjectOnOff(kDeskPostIt, false);
			break;
		case 4: // letters
			setObjectOnOff(kDeskBook, false);
			setObjectOnOff(kDeskClipboard, false);
			setObjectOnOff(kDeskPostIt, false);
			break;
		case 5: // book
			setObjectOnO
ff(kDeskLetterEnv, false);
			setObjectOnOff(kDeskLetterOpen, false);
			setObjectOnOff(kDeskClipboard, false);
			setObjectOnOff(kDeskPostIt, false);
			break;
		case 6: // clipboard
			setObjectOnOff(kDeskBook, false);
			setObjectOnOff(kDeskLetterEnv, false);
			setObjectOnOff(kDeskLetterOpen, false);
			setObjectOnOff(kDeskPostIt, false);
			break;
		case 7: // postit
			setObjectOnOff(kDeskBook, false);
			setObjectOnOff(kDeskLetterEnv, false);
			setObjectOnOff(kDeskLetterOpen, false);
			setObjectOnOff(kDeskClipboard, false);
			break;
		}
	} else if (_animationName == "vanity") {
		debugC(1, kColonyDebugAnimation, "Vanity init: action0=%d action1=%d level=%d weapons=%d armor=%d", _action0, _action1, _level, _weapons, _armor);
		for (int i = 0; i < (int)_lSprites.size(); i++) {
			ComplexSprite *ls = _lSprites[i];
			debugC(1, kColonyDebugAnimation, "  Vanity sprite %d: type=%d frozen=%d locked=%d current=%d onoff=%d key=%d lock=%d frames=%d",
				i + 1, ls->type, ls->frozen, ls->locked, ls->current, (int)ls->onoff, ls->key, ls->lock, (int)ls->objects.size());
		}
		// DOS DoVanity: set suit state on mirror display (object 1)
		if (_weapons && _armor)
			setObjectState(1, 3);
		else if (_weapons)
			setObjectState(1, 2);
		else if (_armor)
			setObjectState(1, 1);
		else
			setObjectState(1, 4);
		// Badge only visible on level 1
		if (_level != 1)
			setObjectOnOff(14, false);
		// Hide items based on action0 (num parameter in DOS)
		if (_action0 < 90) { // coffee cup only
			setObjectOnOff(4, false);
			setObjectOnOff(7, false);
			setObjectOnOff(13, false);
		} else if (_action0 < 100) { // paper
			setObjectOnOff(12, false);
			setObjectOnOff(4, false);
			setObjectOnOff(7, false);
		} else if (_action0 < 110) { // diary
			setObjectOnOff(12, false);
			setObjectOnOff(13, false);
			setObjectOnOff(7, false);
		} else if (_action0 < 120) { // book
			setObjectOnOff(12, false);
			setObjectOnOff(13, false);
			setObjectOnOff(4, false);
		}
	} else if (_anim
ationName == "lift") {
		if (getPlatform() == Common::kPlatformMacintosh)
			_sound->stop();

		_liftUp = (_fl == 2);
		// Hide all object sprites except the active one
		for (int i = 1; i <= 4; i++) {
			if (i != _liftObject)
				setObjectOnOff(i, false);
		}
		if (_liftUp) {
			setObjectState(9, 2); // up arrow OFF
			setObjectState(8, 1); // down arrow ON
			setObjectState(_liftObject, 5); // object at top
		} else {
			setObjectState(9, 1); // up arrow ON
			setObjectState(8, 2); // down arrow OFF
			setObjectState(_liftObject, 1); // object at bottom
		}
		drawAnimation();
		_gfx->copyToScreen();
	}

	// drawAnimation is heavy (per-pixel setPixel calls for the 416x264 pattern)
	// so cap it to the animation update cadence (50ms = 20fps). Between draws,
	// only call updateScreen so the OpenGL backend re-composites with the
	// current cursor position — keeps the pointer smooth without re-rendering.
	uint32 lastDrawTime = 0;
	bool needsDraw = true;
	while (_animationRunning && !shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_animationRunning = false;
				break;
			} else if (event.type == Common::EVENT_SCREEN_CHANGED) {
				_gfx->computeScreenViewport();
				needsDraw = true;
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				const Common::Point pt = eventMouseToLogical(event.mouse);
				_messageSourceRect = Common::Rect();
				if (handleColonyCoderClick(pt)) {
					needsDraw = true;
				} else if (!_animExitStrip.isEmpty() && _animExitStrip.contains(pt)) {
					// gamesprt.c TestButton(): strip clicks never reach sprites
					if (_animExitButton.contains(pt)) {
						_animExitPressed = _animExitInside = true;
						needsDraw = true;
					}
				} else {
					int item = whichSprite(pt);
					if (item > 0) {
						handleAnimationClick(item);
						_messageSourceRect = Common::Rect();
						needsDraw = true;
			
		}
				}
			} else if (event.type == Common::EVENT_LBUTTONUP) {
				if (_coderPressed >= 0) {
					if (_coderPressInside) {
						if (_coderCursor >= 4) {
							for (int j = 0; j < 4; j++)
								_coderPick[j] = 0;
							_coderCursor = 0;
						}
						_coderPick[_coderCursor++] = _coderPressed + 1;
						_sound->play(Sound::kDit);
					}
					_coderPressed = -1;
					_coderPressInside = false;
					needsDraw = true;
				}
				if (_animExitPressed) {
					if (_animExitInside) {
						debugC(1, kColonyDebugAnimation, "Animation: EXIT button");
						_animationRunning = false;
					}
					_animExitPressed = _animExitInside = false;
					needsDraw = true;
				}
			} else if (event.type == Common::EVENT_RBUTTONDOWN) {
				// DOS: right-click exits animation (AnimControl returns FALSE on button-up)
				const Common::Point logical = eventMouseToLogical(event.mouse);
				debugC(1, kColonyDebugAnimation, "Animation: RBUTTONDOWN exit at pos=%d,%d", logical.x, logical.y);
				_animationRunning = false;
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				const Common::Point logical = eventMouseToLogical(event.mouse);
				if (_coderPressed >= 0) {
					const bool inside = _coderIconRects[_coderPressed].contains(logical);
					if (inside != _coderPressInside) {
						_coderPressInside = inside;
						needsDraw = true;
					}
				}
				if (_animExitPressed) {
					const bool inside = _animExitButton.contains(logical);
					if (inside != _animExitInside) {
						_animExitInside = inside;
						needsDraw = true;
					}
				}
				debugC(5, kColonyDebugAnimation, "Animation Mouse: %d, %d", logical.x, logical.y);
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (event.customType == kActionEscape) {
					// Exit the animation like right-click/Return. Opening the
					// main menu here instead would be a dead end: save/load
					// are disabled while _animationRunning is set.
					debugC(1, kColonyDebugAnimation, "Animation: ESC exit");
					_ani
mationRunning = false;
				}
			} else if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_RETURN) {
					// Original Mac AnimControl(): Return closes the animation window.
					_animationRunning = false;
					continue;
				}

				int item = 0;
				if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_0);
				} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_KP0);
				} else if (event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					item = 12; // Enter
				} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE || event.kbd.keycode == Common::KEYCODE_DELETE) {
					item = 11; // Clear
				}

				if (item > 0) {
					_messageSourceRect = Common::Rect();
					handleAnimationClick(item);
					needsDraw = true;
				}
			}
		}
		if (!_animationRunning || shouldQuit())
			break;

		// updateAnimation has its own 50ms throttle; only redraw when we know
		// the visible state changed (click feedback) or the cadence is due.
		const uint32 prevAnimUpdate = _lastAnimUpdate;
		updateAnimation();
		if (_lastAnimUpdate != prevAnimUpdate)
			needsDraw = true;

		const uint32 now = _system->getMillis();
		if (needsDraw || now - lastDrawTime >= 50) {
			drawAnimation();
			_gfx->copyToScreen();
			lastDrawTime = now;
			needsDraw = false;
		} else {
			_system->updateScreen();
		}
		_system->delayMillis(2);
	}

	if (_animationName == "lift" && getPlatform() == Common::kPlatformMacintosh) {
		// Mac KillTSound waits for the lift sample before returning.
		while (_sound->isPlaying() && !shouldQuit())
			responsiveAnimationDelay(_system, 10);
		_sound->stop();
	}

	if (useSquarePixelViewport)
		_gfx->setSquarePixelViewport(false);
	_system->lockMouse(true);
	CursorMan.showMouse(false);
	CursorMan.popAllCursors();

	// Purge pending mouse/keyboard events AFTER re-l
ocking the mouse,
	// so that any synthetic events from the lock transition are also
	// drained and don't trigger actions (e.g. cShoot) in the main loop.
	_system->getEventManager()->purgeMouseEvents();
	_system->getEventManager()->purgeKeyboardEvents();

	// Suppress collision sound on the first wall hit after animation exit.
	// The player is at a door/wall boundary and held movement keys will
	// immediately trigger a collision sound. The flag auto-clears in cCommand().
	_suppressCollisionSound = true;

	deleteAnimation();
}

void ColonyEngine::updateAnimation() {
	uint32 now = _system->getMillis();
	if (now - _lastAnimUpdate < 50) // Reduced to 50ms (20 fps) to make it "move"
		return;
	_lastAnimUpdate = now;

	for (uint i = 0; i < _lSprites.size(); i++) {
		ComplexSprite *ls = _lSprites[i];
		// type 0 are displays that auto-animate
		// Original NoShowIt ONLY checked !ls->locked
		if (ls->onoff && ls->type == 0 && !ls->locked && ls->objects.size() > 1) {
			ls->current++;
			if (ls->current >= (int)ls->objects.size())
				ls->current = 0;
		}
	}
}

// Resolve a BMColor entry to an ARGB color.
// bmEntry > 0: cColor index -> use _macColors[idx].bg
// bmEntry < 0: negated Mac system color constant
// bmEntry == 0: level-based character color (depends on corepower)
uint32 ColonyEngine::resolveAnimColor(int16 bmEntry) const {
	if (bmEntry < 0) {
		return macSysColorToARGB(-bmEntry);
	} else if (bmEntry > 0) {
		if (bmEntry < 145)
			return packMacColor(_macColors[bmEntry].bg);
		return 0xFFFFFFFF;
	} else {
		// Zero = level-based (original gamesprt.c DrawlSprite/DrawBackGround):
		//   if(corepower[coreindex]) RGBBackColor(&cColor[c_char0+level-1].f);
		//   else RGBBackColor(&cColor[c_dwall].b);
		if (_corePower[_coreIndex] > 0 && _level >= 1 && _level <= 7)
			return packMacColor(_macColors[kMcChar0 + _level - 1].fg);
		return packMacColor(_macColors[kMcDwall].bg);
	}
}

void ColonyEngine::drawAnimation() {
	// Full clear required: OpenGL framebuffer is undefi
ned between frames.
	_gfx->clear(0);

	// Center 416x264 animation area on screen (from original InitDejaVu)
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;

	const bool useColor = (isMacColorMode()
		&& !_animBMColors.empty());

	// Fill background patterns (416x264 area).
	// Color mode: QuickDraw pattern bit 1 -> ForeColor (black), bit 0 -> BackColor.
	// Original DrawBackGround():
	//   Top: BMColor[0]<0 -> system color; ==0 -> powered:c_char0+level-1.f, else:c_dwall.b
	//   Bottom: powered -> c_lwall.f; unpowered -> inherits top BackColor
	// B&W/DOS: preserve existing palette-index behavior (bit 1 -> 15, bit 0 -> 0).
	//
	// We render the pattern into a cached RGBA surface and blit it via
	// drawSurface (one texture upload per change). The previous implementation
	// did 416*264 = 109,824 individual setPixel calls per drawAnimation, each
	// issuing its own glBegin/glEnd; on the OpenGL backend that took tens of
	// milliseconds per frame and starved the cursor of refreshes.
	const int patternMode = useColor ? 2 : (isMacRenderMode() ? 1 : 0);
	uint32 topColor = 0, botColor = 0;
	if (useColor) {
		const bool powered = (_corePower[_coreIndex] > 0);
		topColor = resolveAnimColor(_animBMColors[0]);
		botColor = powered ? packMacColor(_macColors[kMcLwall].fg) : topColor;
	}

	const bool keyChanged = !_animPatternValid
		|| _animPatternKeyMode != patternMode
		|| _animPatternKeyDivide != _divideBG
		|| _animPatternKeyTopColor != topColor
		|| _animPatternKeyBotColor != botColor
		|| memcmp(_animPatternKeyTopBG, _topBG, 8) != 0
		|| memcmp(_animPatternKeyBottomBG, _bottomBG, 8) != 0;

	if (!_animPatternSurface) {
		_animPatternSurface = new Graphics::Surface();
		_animPatternSurface->create(416, 264, _gfx->getPixelFormat());
	}

	if (keyChanged) {
		const Graphics::PixelFormat &fmt = _animPatternSurface->format;
		uint32 fgPixel, topBgPixel, botBgPixel;
		switch (patternMode) {
		case
 2: // Mac color: bit=1 → black, bit=0 → topColor/botColor
			fgPixel = fmt.ARGBToColor(255, 0, 0, 0);
			topBgPixel = fmt.ARGBToColor(255,
				(topColor >> 16) & 0xFF, (topColor >> 8) & 0xFF, topColor & 0xFF);
			botBgPixel = fmt.ARGBToColor(255,
				(botColor >> 16) & 0xFF, (botColor >> 8) & 0xFF, botColor & 0xFF);
			break;
		case 1: // Mac B&W: bit=1 → black, bit=0 → white
			fgPixel = fmt.ARGBToColor(255, 0, 0, 0);
			topBgPixel = botBgPixel = fmt.ARGBToColor(255, 255, 255, 255);
			break;
		default: // DOS: bit=1 → white, bit=0 → black
			fgPixel = fmt.ARGBToColor(255, 255, 255, 255);
			topBgPixel = botBgPixel = fmt.ARGBToColor(255, 0, 0, 0);
			break;
		}

		uint32 *pixels = (uint32 *)_animPatternSurface->getPixels();
		for (int y = 0; y < 264; y++) {
			const byte row = ((y < _divideBG) ? _topBG : _bottomBG)[y % 8];
			const uint32 bgPixel = (y < _divideBG) ? topBgPixel : botBgPixel;
			uint32 *dst = pixels + y * 416;
			for (int x = 0; x < 416; x++) {
				const bool set = (row & (0x80 >> (x % 8))) != 0;
				dst[x] = set ? fgPixel : bgPixel;
			}
		}

		memcpy(_animPatternKeyTopBG, _topBG, 8);
		memcpy(_animPatternKeyBottomBG, _bottomBG, 8);
		_animPatternKeyDivide = _divideBG;
		_animPatternKeyTopColor = topColor;
		_animPatternKeyBotColor = botColor;
		_animPatternKeyMode = patternMode;
		_animPatternValid = true;
	}

	_gfx->drawSurface(_animPatternSurface, ox, oy);

	// Draw background image if active.
	// Original: BMColor[1] only applied when corepower[coreindex] > 0.
	if (_backgroundActive && _backgroundFG) {
		uint32 bgFill = 0xFFFFFFFF; // B&W default
		if (useColor && _animBMColors.size() > 1) {
			if (_corePower[_coreIndex] > 0)
				bgFill = resolveAnimColor(_animBMColors[1]);
			else
				bgFill = resolveAnimColor(_animBMColors[0]); // unpowered: inherits top
		}
		drawAnimationImage(_backgroundFG, _backgroundMask,
			ox + _backgroundLocate.left, oy + _backgroundLocate.top,
			bgFill, _backgroundBaked, _backgroundBakedKey);
	}

	// Draw complex sp
rites
	for (uint i = 0; i < _lSprites.size(); i++) {
		if (_lSprites[i]->onoff)
			drawComplexSprite(i, ox, oy);
	}

	drawAnimationExitButton(ox, oy);
	drawColonyCoder(ox, oy);
}

// gamesprt.c DrawButton(): 30px strip below the 416x264 scene with a framed
// "EXIT" default button; TestButton() inverts it while pressed inside.
void ColonyEngine::drawAnimationExitButton(int ox, int oy) {
	if (!isMacRenderMode())
		return;

	Graphics::MacFont systemFont(Graphics::kMacFontSystem, 12);
	const Graphics::Font *font = (_wm && _wm->_fontMan) ? _wm->_fontMan->getFont(systemFont) : nullptr;
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return;

	Graphics::ManagedSurface strip;
	strip.create(416, 30, _gfx->getPixelFormat());
	const uint32 white = strip.format.ARGBToColor(255, 255, 255, 255);
	const uint32 black = strip.format.ARGBToColor(255, 0, 0, 0);
	strip.fillRect(Common::Rect(0, 0, 416, 30), white);
	strip.hLine(0, 0, 415, black);

	const int wd = font->getStringWidth("EXIT") / 2;
	const Common::Rect button(208 - (wd + 15), 6, 208 + (wd + 15), 24);
	const bool invert = _animExitPressed && _animExitInside;
	strip.drawRoundRect(button, 8, black, invert);
	Common::Rect ring(button.left - 3, button.top - 3, button.right + 3, button.bottom + 3);
	strip.drawRoundRect(ring, 11, black, false);
	ring.grow(-1);
	strip.drawRoundRect(ring, 10, black, false);

	const int textY = button.top + (button.height() - font->getFontHeight()) / 2 + 1;
	font->drawString(&strip, "EXIT", button.left, textY, button.width(),
		invert ? white : black, Graphics::kTextAlignCenter);

	_gfx->drawSurface(&strip.rawSurface(), ox, oy + 264);
	strip.free();

	_animExitStrip = Common::Rect(ox, oy + 264, ox + 416, oy + 294);
	_animExitButton = button;
	_animExitButton.translate(ox, oy + 264);
}

void ColonyEngine::loadCoderTiles() {
	_coderTilesLoaded = true;

	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(Common::Path("security"));
	
if (!file)
		return;
	Common::SeekableReadStreamEndianWrapper stream(file, true, DisposeAfterUse::YES);

	byte bg[8];
	stream.read(bg, 8);
	stream.read(bg, 8);
	stream.readSint16();
	if (stream.readSint16() != 0) {
		readRect(stream);
		readRect(stream);
		delete loadImage(stream);
		delete loadImage(stream);
	}

	Common::Array<Image *> imgs;
	const int16 maxsprite = stream.readSint16();
	stream.readSint16();
	for (int i = 0; i < maxsprite && !stream.err(); i++) {
		imgs.push_back(loadImage(stream));
		delete loadImage(stream); // mask
		stream.readSint16();
		readRect(stream);
		readRect(stream);
	}

	// Object 2 (a digit key) provides the button faces (states 1..2),
	// object 27 the four symbol tiles (states 2..5).
	const int16 maxl = stre

... [Content truncated]