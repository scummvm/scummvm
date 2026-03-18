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

#include "colony/colony.h"
#include "colony/renderer.h"
#include "common/system.h"
#include "common/events.h"
#include "common/debug.h"
#include "common/file.h"
#include "graphics/cursorman.h"
#include <math.h>

namespace Colony {

static void responsiveAnimationDelay(OSystem *system, uint32 delayMs) {
	if (!system || delayMs == 0)
		return;

	Common::Array<Common::Event> deferredEvents;
	uint mouseMoveCount = 0;
	const uint32 start = system->getMillis();
	uint32 elapsed = 0;
	while (elapsed < delayMs) {
		Common::Event event;
		while (system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_MOUSEMOVE) {
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

static bool isBackdoorCode111111(const uint8 display[6]) {
	for (int i = 0; i < 6; i++) {
		if (display[i] != 3)
			return false;
	}

	return true;
}

// Mac color indices from colordef.h enum (cColor[] table in Color256).
enum {
	mc_dwall = 6, mc_lwall = 7,
	mc_char0 = 8,    // char0..char6 = 8..14
	mc_bulkhead = 15, mc_door = 16,
	mc_desk = 58, mc_desktop = 59, mc_screen = 62,
	mc_proj = 72, mc_console = 79, mc_powerbase = 81,
	mc_box1 = 84, mc_forklift = 86, mc_flglass = 87,
	mc_cryo = 90, mc_ccore = 111,
	mc_teleport = 93, mc_teledoor = 94,
	mc_vanity = 96, mc_mirror = 103,
	mc_airlock = 25, mc_elevator = 23
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
static const int16 kBMC_Desk[] = {
	0, mc_desktop,
	-kMacRed, -kMacCyan, -kMacCyan, -kMacCyan, -kMacCyan,
	-kMacWhite, -kMacWhite, -kMacMagenta, -kMacYellow, mc_desk,
	mc_desk, mc_desk, mc_desk, mc_desk, mc_desk,
	-kMacWhite, mc_screen, -kMacMagenta, -kMacCyan, -kMacCyan,
	-kMacBlue, -kMacWhite, -kMacRed, -kMacWhite, -kMacYellow
};
static const int16 kBMC_Vanity[] = {
	0, mc_vanity,
	mc_mirror, -kMacRed, -kMacCyan, -kMacWhite, -kMacYellow,
	-kMacGreen, -kMacBlue, -kMacRed, -kMacMagenta, -kMacRed,
	mc_vanity, -kMacWhite, -kMacYellow, mc_mirror
};
static const int16 kBMC_Reactor[] = {
	0, mc_console,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacCyan,
	-kMacMagenta, -kMacWhite
};
static const int16 kBMC_Security[] = {
	0, mc_console,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacWhite,
	-kMacRed, -kMacCyan, -kMacCyan, -kMacCyan, -kMacCyan
};
static const int16 kBMC_Teleport[] = {
	0, mc_teleport, 0, mc_teledoor
};
static const int16 kBMC_Creatures[] = {
	-kMacWhite, 0, -kMacWhite, -kMacCyan, mc_proj,
	-kMacBlue, -kMacMagenta, -kMacMagenta
};
static const int16 kBMC_Controls[] = {
	0, mc_console,
	-kMacRed, -kMacYellow, -kMacYellow, -kMacBlue, -kMacYellow, -kMacGreen, mc_screen
};
static const int16 kBMC_Lift[] = {
	0, mc_flglass,
	mc_teleport, mc_box1, mc_cryo, mc_ccore, 0,
	-kMacRed, -kMacRed, -kMacCyan, -kMacCyan
};
static const int16 kBMC_Powersuit[] = {
	0, mc_powerbase,
	-kMacMagenta, -kMacMagenta, -kMacYellow, -kMacYellow, mc_powerbase, -kMacWhite
};
static const int16 kBMC_Forklift[] = {
	0, mc_forklift, mc_forklift, mc_forklift
};
static const int16 kBMC_Door[] = {
	0, mc_bulkhead, 0, mc_door, -kMacYellow
};
static const int16 kBMC_Bulkhead[] = {
	0, mc_bulkhead, 0, mc_bulkhead, -kMacYellow
};
static const int16 kBMC_Airlock[] = {
	0, mc_bulkhead, mc_bulkhead, -kMacRed, mc_airlock
};
static const int16 kBMC_Elevator[] = {
	0, mc_bulkhead, 0, mc_elevator, mc_elevator, -kMacYellow
};
static const int16 kBMC_Elevator2[] = {
	0, mc_bulkhead, 0, -kMacMagenta, mc_elevator, mc_elevator,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow
};

struct AnimColorEntry {
	const char *name;
	const int16 *colors;
	int count;
};

static const AnimColorEntry kAnimColors[] = {
	{ "desk",        kBMC_Desk,       ARRAYSIZE(kBMC_Desk) },
	{ "vanity",      kBMC_Vanity,     ARRAYSIZE(kBMC_Vanity) },
	{ "reactor",     kBMC_Reactor,    ARRAYSIZE(kBMC_Reactor) },
	{ "security",    kBMC_Security,   ARRAYSIZE(kBMC_Security) },
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

// Convert Mac Toolbox BackColor constant to ARGB.
static uint32 macSysColorToARGB(int sysColor) {
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

static uint32 packMacColorBG(const uint16 rgb[3]) {
	return 0xFF000000 | ((uint32)(rgb[0] >> 8) << 16) |
	       ((uint32)(rgb[1] >> 8) << 8) | (uint32)(rgb[2] >> 8);
}

static int getAnimationStateCount(const Common::Array<ComplexSprite *> &sprites, int num) {
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

	// DOS uses short names (suit.pic, elev.pic, etc.); Mac uses full names
	// (spacesuit, elevator, etc.) without extensions, in a CData folder.
	static const struct { const char *dosName; const char *macName; } nameMap[] = {
		{ "suit",   "spacesuit" },
		{ "elev",   "elevator" },
		{ "slides", "slideshow" },
		{ "lift",   "lifter" },
		{ nullptr,  nullptr }
	};

	Common::String fileName = name + ".pic";
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
	if (!file) {
		// Try lowercase (Mac resource fork)
		fileName = name;
		fileName.toLowercase();
		file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
	}
	if (!file) {
		// Try Mac long name mapping
		Common::String macName;
		for (int i = 0; nameMap[i].dosName; i++) {
			if (nameLower == nameMap[i].dosName) {
				macName = nameMap[i].macName;
				break;
			}
		}
		if (!macName.empty())
			file = Common::MacResManager::openFileOrDataFork(Common::Path(macName));
	}
	if (!file) {
		// Try CData directory with both DOS and Mac names
		fileName = "CData/" + nameLower;
		file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
		if (!file) {
			for (int i = 0; nameMap[i].dosName; i++) {
				if (nameLower == nameMap[i].dosName) {
					fileName = Common::String("CData/") + nameMap[i].macName;
					file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
					break;
				}
			}
		}
	}
	if (!file) {
		warning("Could not open animation file %s", name.c_str());
		return false;
	}

	deleteAnimation();

	// Read background data
	file->read(_topBG, 8);
	file->read(_bottomBG, 8);
	_divideBG = readSint16(*file);
	_backgroundActive = readSint16(*file) != 0;
	if (_backgroundActive) {
		_backgroundClip = readRect(*file);
		_backgroundLocate = readRect(*file);
		_backgroundMask = loadImage(*file);
		_backgroundFG = loadImage(*file);
	}

	// Read sprite data
	int16 maxsprite = readSint16(*file);
	readSint16(*file); // locSprite
	for (int i = 0; i < maxsprite; i++) {
		Sprite *s = new Sprite();
		s->fg = loadImage(*file);
		s->mask = loadImage(*file);
		s->used = readSint16(*file) != 0;
		s->clip = readRect(*file);
		s->locate = readRect(*file);
		_cSprites.push_back(s);
	}

	// Read complex sprite data
	int16 maxLSprite = readSint16(*file);
	readSint16(*file); // anum
	for (int i = 0; i < maxLSprite; i++) {
		ComplexSprite *ls = new ComplexSprite();
		int16 size = readSint16(*file);
		for (int j = 0; j < size; j++) {
			ComplexSprite::SubObject sub;
			sub.spritenum = readSint16(*file);
			sub.xloc = readSint16(*file);
			sub.yloc = readSint16(*file);
			ls->objects.push_back(sub);
		}
		ls->bounds = readRect(*file);
		ls->visible = readSint16(*file) != 0;
		ls->current = readSint16(*file);
		ls->xloc = readSint16(*file);
		ls->yloc = readSint16(*file);
		ls->acurrent = readSint16(*file);
		ls->axloc = readSint16(*file);
		ls->ayloc = readSint16(*file);
		ls->type = file->readByte();
		ls->frozen = file->readByte();
		ls->locked = file->readByte();
		ls->link = readSint16(*file);
		ls->key = readSint16(*file);
		ls->lock = readSint16(*file);
		ls->onoff = true;
		_lSprites.push_back(ls);
	}

	delete file;
	return true;
}

void ColonyEngine::deleteAnimation() {
	delete _backgroundMask;
	_backgroundMask = nullptr;
	delete _backgroundFG;
	_backgroundFG = nullptr;
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
	_system->lockMouse(false);
	_system->showMouse(true);
	_system->warpMouse(_centerX, _centerY);
	const char *cursorName = "default arrow cursor";
	if (_renderMode == Common::kRenderMacintosh && _macArrowCursor) {
		cursorName = "Mac arrow cursor";
		CursorMan.replaceCursor(_macArrowCursor);
	} else {
		CursorMan.setDefaultArrowCursor(true);
	}
	CursorMan.showMouse(true);
	_system->updateScreen();
	warning(
		"Colony animation cursor: %s uses %s at center=(%d,%d)",
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
			setObjectOnOff(23, false);
			setObjectOnOff(24, false);
		}
		if (_action0 != 30)
			setObjectOnOff(6, false); // Teeth
		if (_action0 != 33) { // Jack-in-the-box
			for (int i = 18; i <= 21; i++)
				setObjectOnOff(i, false);
		}

		int ntype = _action1 / 10;
		switch (ntype) {
		case 0:
		case 1:
		case 2:
		case 3:
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(9, false);
			setObjectOnOff(22, false);
			setObjectOnOff(25, false);
			break;
		case 4: // letters
			setObjectOnOff(22, false);
			setObjectOnOff(9, false);
			setObjectOnOff(25, false);
			break;
		case 5: // book
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(9, false);
			setObjectOnOff(25, false);
			break;
		case 6: // clipboard
			setObjectOnOff(22, false);
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(25, false);
			break;
		case 7: // postit
			setObjectOnOff(22, false);
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(9, false);
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
	} else if (_animationName == "lift") {
		// Original DoLift: set up initial state based on forklift mode.
		// _fl==1 → picking up (up=0, object starts at bottom)
		// _fl==2 → putting down (up=1, object starts at top)
		// Object sprite mapping: BOX1/BOX2→2, TELEPORT→1, CRYO→3, REACTOR→4
		_liftUp = (_fl == 2);
		switch (_fl == 2 ? _carryType : 0) {
		case kObjBox1: case kObjBox2: _liftObject = 2; break;
		case kObjTeleport: _liftObject = 1; break;
		case kObjCryo: _liftObject = 3; break;
		case kObjReactor: _liftObject = 4; break;
		default: _liftObject = 2; break; // pickup: we don't know yet, but dolSprite handles it
		}
		// For pickup, determine object from what we're about to pick up
		if (!_liftUp) {
			// The interaction code sets _carryType AFTER the animation,
			// but the object type is in the Thing we're interacting with.
			// We can infer from which sprites are visible.
			for (int i = 1; i <= 4; i++) {
				if (i < (int)_lSprites.size() && _lSprites[i - 1] && _lSprites[i - 1]->onoff)
					_liftObject = i;
			}
		}
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

	while (_animationRunning && !shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN) {
				int item = whichSprite(event.mouse);
				if (item > 0) {
					handleAnimationClick(item);
				}
			} else if (event.type == Common::EVENT_RBUTTONDOWN) {
				// DOS: right-click exits animation (AnimControl returns FALSE on button-up)
				debugC(1, kColonyDebugAnimation, "Animation: RBUTTONDOWN exit at pos=%d,%d", event.mouse.x, event.mouse.y);
				_animationRunning = false;
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				debugC(5, kColonyDebugAnimation, "Animation Mouse: %d, %d", event.mouse.x, event.mouse.y);
			} else if (event.type == Common::EVENT_KEYDOWN) {
				int item = 0;
				if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_0);
				} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_KP0);
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					item = 12; // Enter
				} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE || event.kbd.keycode == Common::KEYCODE_DELETE) {
					item = 11; // Clear
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_animationRunning = false;
				}

				if (item > 0) {
					handleAnimationClick(item);
				}
			}
		}

		updateAnimation();
		drawAnimation();
		_gfx->copyToScreen();
		responsiveAnimationDelay(_system, 4);
	}

	_system->lockMouse(true);
	_system->showMouse(false);
	CursorMan.showMouse(false);
	CursorMan.popAllCursors();

	// Purge pending mouse/keyboard events AFTER re-locking the mouse,
	// so that any synthetic events from the lock transition are also
	// drained and don't trigger actions (e.g. cShoot) in the main loop.
	_system->getEventManager()->purgeMouseEvents();
	_system->getEventManager()->purgeKeyboardEvents();

	// Suppress collision sound on the first few wall hits after animation exit.
	// The player is at a door/wall boundary and held movement keys will
	// immediately trigger checkwall collisions that play kBang — which sounds
	// like a spurious gunshot. The flag auto-clears on the first successful move.
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
			return packMacColorBG(_macColors[bmEntry].bg);
		return 0xFFFFFFFF;
	} else {
		// Zero = level-based (original gamesprt.c DrawlSprite/DrawBackGround):
		//   if(corepower[coreindex]) RGBBackColor(&cColor[c_char0+level-1].f);
		//   else RGBBackColor(&cColor[c_dwall].b);
		if (_corePower[_coreIndex] > 0 && _level >= 1 && _level <= 7)
			return packMacColorBG(_macColors[mc_char0 + _level - 1].fg);
		return packMacColorBG(_macColors[mc_dwall].bg);
	}
}

void ColonyEngine::drawAnimation() {
	_gfx->clear(0);

	// Center 416x264 animation area on screen (from original InitDejaVu)
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;

	const bool useColor = (_hasMacColors && _renderMode == Common::kRenderMacintosh
	                       && !_animBMColors.empty());

	// Fill background patterns (416x264 area).
	// Color mode: QuickDraw pattern bit 1 -> ForeColor (black), bit 0 -> BackColor.
	// Original DrawBackGround():
	//   Top: BMColor[0]<0 -> system color; ==0 -> powered:c_char0+level-1.f, else:c_dwall.b
	//   Bottom: powered -> c_lwall.f; unpowered -> inherits top BackColor
	// B&W/DOS: preserve existing palette-index behavior (bit 1 -> 15, bit 0 -> 0).
	if (useColor) {
		const bool powered = (_corePower[_coreIndex] > 0);
		uint32 topBG = resolveAnimColor(_animBMColors[0]);
		// Bottom: only uses c_lwall.f when powered; unpowered inherits top color
		uint32 botBG = powered ? packMacColorBG(_macColors[mc_lwall].fg) : topBG;
		for (int y = 0; y < 264; y++) {
			byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
			byte row = pat[y % 8];
			uint32 bg = (y < _divideBG) ? topBG : botBG;
			for (int x = 0; x < 416; x++) {
				bool set = (row & (0x80 >> (x % 8))) != 0;
				_gfx->setPixel(ox + x, oy + y, set ? (uint32)0xFF000000 : bg);
			}
		}
	} else if (_renderMode == Common::kRenderMacintosh) {
		// Mac QuickDraw FillRect: pattern bit=1 → ForeColor (black=0),
		// bit=0 → BackColor (white=15).
		for (int y = 0; y < 264; y++) {
			byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
			byte row = pat[y % 8];
			for (int x = 0; x < 416; x++) {
				bool set = (row & (0x80 >> (x % 8))) != 0;
				_gfx->setPixel(ox + x, oy + y, set ? 0 : 15);
			}
		}
	} else {
		// DOS MetaWINDOW: pattern bit=1 → pen color (white=15),
		// bit=0 → background (black=0). Opposite of Mac QuickDraw.
		for (int y = 0; y < 264; y++) {
			byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
			byte row = pat[y % 8];
			for (int x = 0; x < 416; x++) {
				bool set = (row & (0x80 >> (x % 8))) != 0;
				_gfx->setPixel(ox + x, oy + y, set ? 15 : 0);
			}
		}
	}

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
		                   bgFill);
	}

	// Draw complex sprites
	for (uint i = 0; i < _lSprites.size(); i++) {
		if (_lSprites[i]->onoff)
			drawComplexSprite(i, ox, oy);
	}
}

void ColonyEngine::drawComplexSprite(int index, int ox, int oy) {
	ComplexSprite *ls = _lSprites[index];
	if (!ls->onoff)
		return;

	int cnum = ls->current;
	if (cnum < 0 || cnum >= (int)ls->objects.size())
		return;

	int spriteIdx = ls->objects[cnum].spritenum;
	if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
		return;

	Sprite *s = _cSprites[spriteIdx];
	int x = ox + ls->xloc + ls->objects[cnum].xloc + s->clip.left;
	int y = oy + ls->yloc + ls->objects[cnum].yloc + s->clip.top;

	// Resolve fill color from BMColor[index+2] (ganimate.c DrawlSprite).
	uint32 fillColor = 0xFFFFFFFF; // B&W default: white
	const bool useColor = (_hasMacColors && _renderMode == Common::kRenderMacintosh
	                       && !_animBMColors.empty());
	if (useColor) {
		int bmIdx = index + 2;
		if (bmIdx < (int)_animBMColors.size())
			fillColor = resolveAnimColor(_animBMColors[bmIdx]);
		else
			fillColor = resolveAnimColor(0); // fallback to level-based
	}

	drawAnimationImage(s->fg, s->mask, x, y, fillColor);
}

void ColonyEngine::drawAnimationImage(Image *img, Image *mask, int x, int y, uint32 fillColor) {
	if (!img || !img->data)
		return;

	const bool useColor = (_hasMacColors && _renderMode == Common::kRenderMacintosh);
	// Mac QuickDraw srcBic+srcOr rendering:
	//   mask bit=1 -> opaque (part of sprite)
	//   fg bit=1   -> ForeColor (black)
	//   fg bit=0   -> BackColor (fillColor from BMColor)
	// Mac B&W: same — fg bit=1 is black (0), fg bit=0 is white (15).
	// DOS MetaWINDOW: OPPOSITE — fg bit=1 is white (15), fg bit=0 is black (0).
	const bool isMacMode = (_renderMode == Common::kRenderMacintosh);
	uint32 fgColor, bgColor;
	if (useColor) {
		fgColor = (uint32)0xFF000000;
		bgColor = fillColor;
	} else if (isMacMode) {
		fgColor = 0;
		bgColor = 15;
	} else {
		fgColor = 15;
		bgColor = 0;
	}

	for (int iy = 0; iy < img->height; iy++) {
		for (int ix = 0; ix < img->width; ix++) {
			int byteIdx = iy * img->rowBytes + (ix / 8);
			int bitIdx = 7 - (ix % 8);

			bool maskSet = true;
			if (mask && mask->data) {
				int mByteIdx = iy * mask->rowBytes + (ix / 8);
				int mBitIdx = 7 - (ix % 8);
				maskSet = (mask->data[mByteIdx] & (1 << mBitIdx)) != 0;
			}

			if (!maskSet)
				continue;

			bool fgSet = (img->data[byteIdx] & (1 << bitIdx)) != 0;
			uint32 color = fgSet ? fgColor : bgColor;

			_gfx->setPixel(x + ix, y + iy, color);
		}
	}
}

Image *ColonyEngine::loadImage(Common::SeekableReadStream &file) {
	Image *im = new Image();
	if (getPlatform() == Common::kPlatformMacintosh) {
		readUint32(file); // baseAddr placeholder
		im->rowBytes = readSint16(file);
		Common::Rect r = readRect(file);
		im->width = r.width();
		im->height = r.height();
		im->align = 0;
		im->bits = 1;
		im->planes = 1;
	} else {
		im->width = readSint16(file);
		im->height = readSint16(file);
		im->align = readSint16(file);
		im->rowBytes = readSint16(file);
		im->bits = file.readByte();
		im->planes = file.readByte();
	}

	int16 tf = readSint16(file);
	uint32 size;
	if (tf) {
		// Mac original loadbitmap: reads bsize bytes into a buffer, then
		// decompresses from that buffer. We must read exactly bsize bytes
		// from the stream to keep file position aligned.
		uint32 bsize = readUint32(file);
		size = readUint32(file);
		im->data = new byte[size];
		byte *packed = new byte[bsize + 8](); // +8 matches original NewPtr(bsize+8)
		file.read(packed, bsize);
		// Decompress: exact match of Mac UnPackBytes(src, dst, len).
		// Buffer is pairs of (count, value). Count is decremented in-place;
		// when it reaches 0, advance to next pair.
		byte *sp = packed;
		for (uint32 di = 0; di < size; di++) {
			if (*sp) {
				im->data[di] = *(sp + 1);
				(*sp)--;
			} else {
				sp += 2;
				im->data[di] = *(sp + 1);
				(*sp)--;
			}
		}
		delete[] packed;
	} else {
		size = readUint32(file);
		im->data = new byte[size];
		file.read(im->data, size);
	}
	return im;
}

void ColonyEngine::unpackBytes(Common::SeekableReadStream &file, byte *dst, uint32 len) {
	uint32 i = 0;
	while (i < len) {
		byte count = file.readByte();
		byte value = file.readByte();
		for (int j = 0; j < count && i < len; j++) {
			dst[i++] = value;
		}
	}
}

Common::Rect ColonyEngine::readRect(Common::SeekableReadStream &file) {
	int16 top, left, bottom, right;
	if (getPlatform() == Common::kPlatformMacintosh) {
		top = readSint16(file);
		left = readSint16(file);
		bottom = readSint16(file);
		right = readSint16(file);
	} else {
		left = readSint16(file);
		top = readSint16(file);
		right = readSint16(file);
		bottom = readSint16(file);
	}
	// Guard against invalid rects from animation data
	if (left > right || top > bottom)
		return Common::Rect();
	return Common::Rect(left, top, right, bottom);
}

int16 ColonyEngine::readSint16(Common::SeekableReadStream &s) {
	if (getPlatform() == Common::kPlatformMacintosh)
		return s.readSint16BE();
	return s.readSint16LE();
}

uint16 ColonyEngine::readUint16(Common::SeekableReadStream &s) {
	if (getPlatform() == Common::kPlatformMacintosh)
		return s.readUint16BE();
	return s.readUint16LE();
}

uint32 ColonyEngine::readUint32(Common::SeekableReadStream &s) {
	if (getPlatform() == Common::kPlatformMacintosh)
		return s.readUint32BE();
	return s.readUint32LE();
}

int ColonyEngine::whichSprite(const Common::Point &p) {
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;
	Common::Point pt(p.x - ox, p.y - oy);

	debugC(1, kColonyDebugAnimation, "Click at (%d, %d), relative (%d, %d)", p.x, p.y, pt.x, pt.y);

	for (int i = _lSprites.size() - 1; i >= 0; i--) {
		ComplexSprite *ls = _lSprites[i];
		if (!ls->onoff)
			continue;

		int cnum = ls->current;
		if (cnum < 0 || cnum >= (int)ls->objects.size())
			continue;

		int spriteIdx = ls->objects[cnum].spritenum;
		if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
			continue;

		Sprite *s = _cSprites[spriteIdx];
		int xloc = ls->xloc + ls->objects[cnum].xloc;
		int yloc = ls->yloc + ls->objects[cnum].yloc;

		Common::Rect r = s->clip;
		r.translate(xloc, yloc);

		if (!r.contains(pt))
			continue;

		// Pixel-perfect mask test (matches DOS WhichlSprite)
		Image *mask = s->mask;
		if (mask && mask->data) {
			int row = pt.y - r.top;
			int col = pt.x - r.left;
			int bitCol = (col + mask->align) * mask->bits;
			int maskIndex = row * mask->rowBytes + (bitCol / 8);
			int shift = bitCol % 8;

			if (maskIndex >= 0 && maskIndex < mask->rowBytes * mask->height) {
				byte maskByte = mask->data[maskIndex];
				if (mask->planes == 2)
					maskByte |= mask->data[mask->rowBytes * mask->height + maskIndex];
				maskByte = maskByte >> shift;
				if (!(maskByte & ((1 << mask->bits) - 1))) {
					debugC(1, kColonyDebugAnimation, "  Sprite %d (type=%d frz=%d): bbox hit but mask transparent at row=%d col=%d bits=%d align=%d",
						i + 1, ls->type, ls->frozen, row, col, mask->bits, mask->align);
					continue; // Transparent pixel, skip this sprite
				}
			} else {
				debugC(1, kColonyDebugAnimation, "  Sprite %d: mask index %d out of bounds (max %d)", i + 1, maskIndex, mask->rowBytes * mask->height);
			}
		} else {
			debugC(1, kColonyDebugAnimation, "  Sprite %d: no mask data, using bbox", i + 1);
		}

		debugC(1, kColonyDebugAnimation, "Sprite %d HIT. type=%d frozen=%d Frame %d, Sprite %d. Box: (%d,%d,%d,%d)",
			i + 1, ls->type, ls->frozen, cnum, spriteIdx, r.left, r.top, r.right, r.bottom);
		return i + 1;
	}

	// Dump accurately calculated bounds if debug is high enough
	if (gDebugLevel >= 2) {
		for (int i = 0; i < (int)_lSprites.size(); i++) {
			ComplexSprite *ls = _lSprites[i];
			if (ls->onoff) {
				int cnum = ls->current;
				if (cnum < 0 || cnum >= (int)ls->objects.size())
					continue;
				int spriteIdx = ls->objects[cnum].spritenum;
				if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
					continue;
				Sprite *s = _cSprites[spriteIdx];

				int xloc = ls->xloc + ls->objects[cnum].xloc;
				int yloc = ls->yloc + ls->objects[cnum].yloc;
				Common::Rect r = s->clip;
				r.translate(xloc, yloc);

				debugC(2, kColonyDebugAnimation, "  Sprite %d: Frame=%d Box=(%d,%d,%d,%d)", i + 1,
					cnum, r.left, r.top, r.right, r.bottom);
			}
		}
	}

	return 0;
}

void ColonyEngine::handleAnimationClick(int item) {
	uint32 now = _system->getMillis();
	if (now - _lastClickTime < 250) {
		debugC(1, kColonyDebugAnimation, "Ignoring rapid click on item %d", item);
		return;
	}
	_lastClickTime = now;
	debugC(1, kColonyDebugAnimation, "Animation click on item %d in %s", item, _animationName.c_str());

	if (item > 0) {
		dolSprite(item - 1);
	}

	if (_animationName == "desk") {
		handleDeskClick(item);
	} else if (_animationName == "vanity") {
		handleVanityClick(item);
	} else if (_animationName == "slides") {
		handleSlidesClick(item);
	} else if (_animationName == "teleshow") {
		handleTeleshowClick(item);
	} else if (_animationName == "reactor" || _animationName == "security") {
		handleKeypadClick(item);
	} else if (_animationName == "suit") {
		handleSuitClick(item);
	} else if (_animationName == "door" || _animationName == "bulkhead") {
		handleDoorClick(item);
	} else if (_animationName == "airlock") {
		handleAirlockClick(item);
	} else if (_animationName == "elev" || _animationName == "elevator" || _animationName == "elevator2") {
		handleElevatorClick(item);
	} else if (_animationName == "controls") {
		handleControlsClick(item);
	} else if (_animationName == "forklift") {
		// Original DoForkLift: waits for item==1 (entrance click) to confirm entry
		if (item == 1) {
			_animationResult = 1;
			_animationRunning = false;
		}
	} else if (_animationName == "lift") {
		// Original DoLift state machine:
		// item 8 = lower button (active when _liftUp)
		// item 9 = raise button (active when !_liftUp)
		if (item == 8 && _liftUp) {
			// Lower the object: animate states 5→1
			setObjectState(8, 2); // lower arrow OFF
			setObjectState(9, 1); // raise arrow ON
			for (int i = 5; i >= 1; i--) {
				setObjectState(_liftObject, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 50);
			}
			_liftUp = false;
			_animationResult = 1;
		} else if (item == 9 && !_liftUp) {
			// Raise the object: animate states 1→5
			setObjectState(9, 2); // raise arrow OFF
			setObjectState(8, 1); // lower arrow ON
			for (int i = 1; i <= 5; i++) {
				setObjectState(_liftObject, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 50);
			}
			_liftUp = true;
			_animationResult = 1;
		}
	}
}

void ColonyEngine::handleDeskClick(int item) {
	if (item >= 2 && item <= 5) {
		int idx = item - 2;
		uint8 *decode = (_level == 1) ? _decode2 : _decode3;
		if (decode[idx] == 0) {
			decode[idx] = (uint8)(2 + (_randomSource.getRandomNumber(3)));
			_lSprites[item - 1]->current = decode[idx] - 1;
			drawAnimation();
			_gfx->copyToScreen();
		}
	} else if (item == 7) { // Letter
		if (_lSprites[6]->current > 0)
			doText(_action1, 0);
	} else if (item == 9) { // Clipboard
		doText(_action1, 0);
	} else if (item == 17) { // Screen
		doText(_action0, 0);
	} else if (item == 22) { // Book
		doText(_action1, 0);
	} else if (item == 24) { // Cigarette
		doText(55, 0);
		terminateGame(false);
	} else if (item == 25) { // Post-it
		doText(_action1, 0);
	}
}

void ColonyEngine::handleVanityClick(int item) {
	debugC(1, kColonyDebugAnimation, "Vanity item %d clicked. Sprite type=%d frozen=%d locked=%d current=%d onoff=%d key=%d lock=%d size=%d",
		item,
		(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->type : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->frozen : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->locked : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->current : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? (int)_lSprites[item-1]->onoff : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->key : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->lock : -1,
		(item > 0 && item <= (int)_lSprites.size()) ? (int)_lSprites[item-1]->objects.size() : -1);
	if (item == 12) { // Coffee cup - spill animation
		if (!_doorOpen) { // reuse _doorOpen as "spilled" flag
			for (int i = 1; i < 6; i++) {
				setObjectState(12, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 50);
			}
			_doorOpen = true;
		}
	} else if (item == 13) { // Paper
		doText(_action0, 0);
	} else if (item == 14) { // Badge
		doText(80, 0);
	} else if (item == 4) { // Diary
		doText(_action0, 0);
	} else if (item == 7) { // Book
		doText(_action0, 0);
	} else {
		debugC(1, kColonyDebugAnimation, "Vanity: unhandled item %d", item);
	}
}

void ColonyEngine::handleSlidesClick(int item) {
	if (item == 2) { // Speaker — GANIMATE.C DoCreatures: DoTestSound() + DoText
		_sound->play(Sound::kTest);
		doText(261 + _creature, 0);
	} else if (item == 5) { // Prev
		_creature--;
		if (_creature == 0)
			_creature = 8;
		setObjectState(1, _creature);
	} else if (item == 6) { // Next
		_creature++;
		if (_creature == 9)
			_creature = 1;
		setObjectState(1, _creature);
	}
}

void ColonyEngine::handleTeleshowClick(int item) {
	if (item == 2) { // Speaker — GANIMATE.C DoTeleShow: DoTestSound() + DoText
		_sound->play(Sound::kTest);
		doText(269 + _creature, 0);
	} else if (item == 5) { // Prev
		_creature--;
		if (_creature == 0)
			_creature = 7;
		setObjectState(1, _creature);
	} else if (item == 6) { // Next
		_creature++;
		if (_creature == 8)
			_creature = 1;
		setObjectState(1, _creature);
	}
}

void ColonyEngine::handleKeypadClick(int item) {
	if (item >= 1 && item <= 10) {
		for (int i = 5; i >= 1; i--)
			_animDisplay[i] = _animDisplay[i - 1];
		_animDisplay[0] = (uint8)(item + 1);
		refreshAnimationDisplay();
		drawAnimation();
		_gfx->copyToScreen();
		// Don't return, let dolSprite animate the button
	} else if (item == 11) { // Clear
		for (int i = 0; i < 6; i++)
			_animDisplay[i] = 1;
		// Reset keypad buttons to unpressed state
		for (int i = 1; i <= 10; i++)
			setObjectState(i, 1);
		refreshAnimationDisplay();
		drawAnimation();
		_gfx->copyToScreen();
	} else if (item == 12) { // Enter
		uint8 testarray[6];
		if (_animationName == "reactor") {
			uint8 *decode = (_level == 1) ? _decode2 : _decode3;
			crypt(testarray, decode[3] - 2, decode[2] - 2, decode[1] - 2, decode[0] - 2);

			debug("Reactor code check: decode=[%d,%d,%d,%d] expected=[%d,%d,%d,%d,%d,%d] entered=[%d,%d,%d,%d,%d,%d]",
				decode[0], decode[1], decode[2], decode[3],
				testarray[0], testarray[1], testarray[2], testarray[3], testarray[4], testarray[5],
				_animDisplay[5], _animDisplay[4], _animDisplay[3], _animDisplay[2], _animDisplay[1], _animDisplay[0]);

			bool match = true;
			for (int i = 0; i < 6; i++) {
				if (testarray[i] != _animDisplay[5 - i])
					match = false;
			}
			// Debug backdoor: accept 111111 as valid code
			bool debugMatch = true;
			for (int i = 0; i < 6; i++) {
				if (_animDisplay[i] != 3) // digit "1" stored as 3 (button "1" is item 2, +1 = 3)
					debugMatch = false;
			}
			debug("Reactor: match=%d debugMatch=%d coreState=%d coreIndex=%d", match, debugMatch, _coreState[_coreIndex], _coreIndex);
			if (match || debugMatch) {
				if (_coreState[_coreIndex] == 0)
					_coreState[_coreIndex] = 1;
				else if (_coreState[_coreIndex] == 1)
					_coreState[_coreIndex] = 0;
				_gametest = true;
				debug("Reactor: code accepted! New coreState=%d", _coreState[_coreIndex]);
			} else {
				// Original ganimate.c: wrong reactor code → Terminate(TRUE)
				debug("Reactor: WRONG CODE — terminating game");
				_animationRunning = false;
				terminateGame(true);
				return;
			}
			_animationRunning = false;
		} else if (_animationName == "security") {
			crypt(testarray, _decode1[0] - 2, _decode1[1] - 2, _decode1[2] - 2, _decode1[3] - 2);
			bool match = true;
			for (int i = 0; i < 6; i++) {
				if (testarray[i] != _animDisplay[5 - i])
					match = false;
			}
			if (match || isBackdoorCode111111(_animDisplay)) {
				// Original ganimate.c: flash all 10 keypad objects on success
				for (int i = 1; i <= 10; i++)
					setObjectState(i, 2);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 200);
				for (int i = 1; i <= 10; i++)
					setObjectState(i, 1);
				drawAnimation();
				_gfx->copyToScreen();
				_unlocked = true;
				_gametest = true;
			}
			_animationRunning = false;
		}
	}
	if (item <= 12) {
		// setObjectState(item, 1); // Reset to ensure animation runs Off -> On - handled by dolSprite
		if (item > 10) // Clear/Enter should return to Off
			setObjectState(item, 1);
		drawAnimation();
		_gfx->copyToScreen();
	} else if (item == 25 && _animationName == "security") {
		doText(14, 0);
	} else if (item == 27 && _animationName == "reactor") {
		doText(12, 0);
	}
}

void ColonyEngine::handleSuitClick(int item) {
	if (item == 1) { // Armor
		if (_armor == 3) {
			for (int i = 6; i >= 1; i--) {
				setObjectState(1, i);
				setObjectState(3, i / 2 + 1);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 30);
			}
			_armor = 0;
		} else {
			setObjectState(1, (_armor * 2 + 1) + 1); // intermediate/pressed
			drawAnimation();
			_gfx->copyToScreen();
			responsiveAnimationDelay(_system, 50);
			_armor++;
		}
		setObjectState(1, _armor * 2 + 1); // target state
		setObjectState(3, _armor + 1); // display state
		drawAnimation();
		_gfx->copyToScreen();
	} else if (item == 2) { // Weapons
		if (_weapons == 3) {
			for (int i = 6; i >= 1; i--) {
				setObjectState(2, i);
				setObjectState(4, i / 2 + 1);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 30);
			}
			_weapons = 0;
		} else {
			setObjectState(2, (_weapons * 2 + 1) + 1); // intermediate/pressed
			drawAnimation();
			_gfx->copyToScreen();
			responsiveAnimationDelay(_system, 50);
			_weapons++;
		}
		setObjectState(2, _weapons * 2 + 1);
		setObjectState(4, _weapons + 1);
		drawAnimation();
		_gfx->copyToScreen();
	} else if (item == 6) {
		// GANIMATE.C DoPowerSuit() case 6: help/info text
		doText(13, 0);
	}
}

void ColonyEngine::handleDoorClick(int item) {
	const bool isMac = (getPlatform() == Common::kPlatformMacintosh);
	const int doorFrames = getAnimationStateCount(_lSprites, 2);
	int openStart = isMac ? 2 : 1;
	int closeStart = isMac ? doorFrames - 1 : doorFrames;

	if (openStart > doorFrames)
		openStart = doorFrames;
	if (closeStart < 1)
		closeStart = 1;

	// Original DoDoor() differs by platform:
	// DOS uses a 3-state door, Mac uses a 5-state door.
	if (item == 3) {
		_sound->play(Sound::kDoor);
		if (_doorOpen) {
			for (int i = closeStart; i >= 1; i--) {
				setObjectState(2, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 80);
			}
		} else {
			for (int i = openStart; i <= doorFrames; i++) {
				setObjectState(2, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 80);
			}
		}
		_doorOpen = !_doorOpen;
	}
	if ((item == 1 || item == 101) && _doorOpen) {
		_animationResult = 1;
		_animationRunning = false;
	}
}

void ColonyEngine::handleAirlockClick(int item) {
	const bool isMac = (getPlatform() == Common::kPlatformMacintosh);
	const int doorItem = isMac ? 3 : 2;
	const int toggleItem = isMac ? 2 : 1;
	const int exitItem = isMac ? 3 : 2;
	const int doorFrames = getAnimationStateCount(_lSprites, doorItem);
	int openStart = isMac ? 2 : 1;
	int closeStart = isMac ? doorFrames - 1 : doorFrames;
	uint8 *airlock = (_airlockX >= 0 && _airlockY >= 0 && _airlockDirection >= 0 &&
		_airlockX < 31 && _airlockY < 31 && _airlockDirection < 5) ?
		&_mapData[_airlockX][_airlockY][_airlockDirection][0] : nullptr;
	bool locked = airlock ? (airlock[1] != 0) : !_doorOpen;

	if (openStart > doorFrames)
		openStart = doorFrames;
	if (closeStart < 1)
		closeStart = 1;

	debugC(1, kColonyDebugAnimation,
		"Airlock click: item=%d locked=%d unlocked=%d core=%d levelCount=%d wall=(%d,%d,%d) dest=[%d,%d,%d]",
		item, locked ? 1 : 0, _unlocked ? 1 : 0, _corePower[_coreIndex],
		(_level >= 1 && _level <= 8) ? _levelData[_level - 1].count : -1,
		_airlockX, _airlockY, _airlockDirection,
		airlock ? airlock[2] : -1, airlock ? airlock[3] : -1, airlock ? airlock[4] : -1);

	// Original DoAirLock() also differs by platform:
	// DOS uses sprite 2 as the 3-state door, Mac uses sprite 3 as the 5-state door.
	if ((item == exitItem || (!isMac && item == 101)) && !locked) {
		_animationResult = 1;
		_animationRunning = false;
	} else if (item == toggleItem && _corePower[_coreIndex] && _unlocked && airlock) {
		_sound->play(Sound::kAirlock);
		if (!locked) {
			for (int i = closeStart; i >= 1; i--) {
				setObjectState(doorItem, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 80);
			}
			setDoorState(_airlockX, _airlockY, _airlockDirection, 1);
			_doorOpen = false;
			locked = true;
		} else {
			for (int i = openStart; i <= doorFrames; i++) {
				setObjectState(doorItem, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 80);
			}

			setDoorState(_airlockX, _airlockY, _airlockDirection, 0);
			_doorOpen = true;
			locked = false;
			if (_level >= 1 && _level <= 8 && _levelData[_level - 1].count == 2) {
				_airlockTerminate = true;
				_animationResult = 0;
				_animationRunning = false;
				return;
			}
		}
	} else if (item == toggleItem) {
		debugC(1, kColonyDebugAnimation,
			"Airlock toggle ignored: unlocked=%d core=%d airlockPtr=%d",
			_unlocked ? 1 : 0, _corePower[_coreIndex], airlock ? 1 : 0);
	} else if (item == 101) {
		_animationRunning = false;
	}
}

void ColonyEngine::handleElevatorClick(int item) {
	const bool isMac = (getPlatform() == Common::kPlatformMacintosh);

	if (isMac) {
		if (_animationName == "elevator2") {
			const int doorFrames3 = getAnimationStateCount(_lSprites, 3);
			const int doorFrames4 = getAnimationStateCount(_lSprites, 4);
			const int doorFrames = (doorFrames3 < doorFrames4) ? doorFrames3 : doorFrames4;
			const int savedLevel = _level;

			if (item >= 5 && item <= 9) {
				int fl = item - 4;
				if (fl == _elevatorFloor) {
					setObjectState(item, 1);
					drawAnimation();
					_gfx->copyToScreen();
				} else {
					_sound->play(Sound::kElevator);
					for (int i = doorFrames - 1; i >= 1; i--) {
						setObjectState(3, i);
						setObjectState(4, i);
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 80);
					}

					if (fl > _elevatorFloor) {
						for (int i = _elevatorFloor; i <= fl; i++) {
							_level = i + 1;
							if (i >= 1)
								setObjectState(2, i);
							drawAnimation();
							_gfx->copyToScreen();
							responsiveAnimationDelay(_system, 80);
						}
					} else {
						for (int i = _elevatorFloor; i >= fl; i--) {
							_level = i + 1;
							if (i >= 1)
								setObjectState(2, i);
							drawAnimation();
							_gfx->copyToScreen();
							responsiveAnimationDelay(_system, 80);
						}
					}

					_elevatorFloor = fl;
					_level = savedLevel;
					_sound->play(Sound::kElevator);
					for (int i = 2; i <= doorFrames; i++) {
						setObjectState(3, i);
						setObjectState(4, i);
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 80);
					}
					setObjectState(item, 1);
					drawAnimation();
					_gfx->copyToScreen();
				}
			} else if (item == 1 || item == 101) {
				_level = savedLevel;
				_animationRunning = false;
			}
			return;
		}

		const int doorFrames2 = getAnimationStateCount(_lSprites, 2);
		const int doorFrames3 = getAnimationStateCount(_lSprites, 3);
		const int doorFrames = (doorFrames2 < doorFrames3) ? doorFrames2 : doorFrames3;

		if (item == 4) {
			_sound->play(Sound::kElevator);
			if (_doorOpen) {
				for (int i = doorFrames - 1; i >= 1; i--) {
					setObjectState(2, i);
					setObjectState(3, i);
					drawAnimation();
					_gfx->copyToScreen();
					responsiveAnimationDelay(_system, 80);
				}
			} else {
				for (int i = 2; i <= doorFrames; i++) {
					setObjectState(2, i);
					setObjectState(3, i);
					drawAnimation();
					_gfx->copyToScreen();
					responsiveAnimationDelay(_system, 80);
				}
			}
			_doorOpen = !_doorOpen;
		} else if (item == 1 || item == 101) {
			if (_doorOpen) {
				if (!loadAnimation("elevator2")) {
					_animationRunning = false;
					return;
				}

				_animationResult = 2;
				_doorOpen = true;
				if (_elevatorFloor >= 1)
					setObjectState(2, _elevatorFloor);
				setObjectState(3, 5);
				setObjectState(4, 5);
				drawAnimation();
				_gfx->copyToScreen();
			} else {
				_animationResult = 0;
				_animationRunning = false;
			}
		}
		return;
	}

	// DOS DoElevator: two phases
	// _doorOpen=false: Phase 1 (outside) - item==5 toggles doors
	// _doorOpen=true: Phase 2 (inside) - items 6-10 select floor
	// _animationResult tracks: 0=outside, 1=doors open, 2=inside
	if (_animationResult < 2) {
		// Phase 1: outside the elevator
		if (item == 5) {
			_sound->play(Sound::kElevator);
			if (!_doorOpen) {
				for (int i = 1; i < 4; i++) {
					setObjectState(3, i);
					setObjectState(4, i);
					drawAnimation();
					_gfx->copyToScreen();
					responsiveAnimationDelay(_system, 80);
				}
				_doorOpen = true;
			} else {
				for (int i = 3; i >= 1; i--) {
					setObjectState(4, i);
					setObjectState(3, i);
					drawAnimation();
					_gfx->copyToScreen();
					responsiveAnimationDelay(_system, 80);
				}
				_doorOpen = false;
			}
		} else if (item == 2 || (item == 101 && _doorOpen)) {
			// Enter the elevator (transition to phase 2)
			_animationResult = 2;
			setObjectOnOff(6, true);
			setObjectOnOff(7, true);
			setObjectOnOff(8, true);
			setObjectOnOff(9, true);
			setObjectOnOff(10, true);
			setObjectOnOff(2, false);
			setObjectOnOff(5, false);
			drawAnimation();
			_gfx->copyToScreen();
		} else if (item == 101 && !_doorOpen) {
			// Exit without entering
			_animationResult = 0;
			_animationRunning = false;
		}
	} else {
		// Phase 2: inside  floor selection
		if (item >= 6 && item <= 10) {
			int fl = item - 5;
			if (fl == _elevatorFloor) {
				setObjectState(item, 1); // already on this floor
			} else {
				_sound->play(Sound::kElevator);
				for (int i = 3; i >= 1; i--) {
					setObjectState(4, i);
					setObjectState(3, i);
					drawAnimation();
					_gfx->copyToScreen();
					responsiveAnimationDelay(_system, 80);
				}
				_elevatorFloor = fl;
				for (int i = 1; i <= 3; i++) {
					setObjectState(4, i);
					setObjectState(3, i);
					drawAnimation();
					_gfx->copyToScreen();
					responsiveAnimationDelay(_system, 80);
				}
				setObjectState(item, 1);
			}
		} else if (item == 1 || item == 101) {
			// Exit elevator
			_animationRunning = false;
		}
	}
}

void ColonyEngine::handleControlsClick(int item) {
	switch (item) {
	case 4: // Accelerator
		if (_corePower[_coreIndex] < 2 || _coreState[_coreIndex] != 0) {
			// GANIMATE.C: if(corepower<2) DoStopSound(); else if(corestate!=0) DoStopSound();
			_sound->play(Sound::kStop);
			debugC(1, kColonyDebugAnimation, "Accelerator failed: power=%d, state=%d", _corePower[_coreIndex], _coreState[_coreIndex]);
			setObjectState(4, 1);
			for (int i = 6; i > 0; i--) {
				setObjectState(4, i);
				drawAnimation();
				_gfx->copyToScreen();
				responsiveAnimationDelay(_system, 20);
			}
			break;
		}

		_animationRunning = false;
		if (_orbit) {
			gameOver(false);
		} else {
			takeOff();
			_orbit = 1;
		}
		break;
	case 5: // Emergency power
		// setObjectState(5, 1); // Reset to ensure animation runs Off -> On - handled by dolSprite
		// dolSprite(4); // Animate the button press - handled by top dolSprite
		if (_coreState[_coreIndex] < 2) {
			if (_corePower[_coreIndex] == 0)
				_corePower[_coreIndex] = 1;
			else if (_corePower[_coreIndex] == 1)
				_corePower[_coreIndex] = 0;
		}
		// Finalize visual state according to power settings
		switch (_corePower[_coreIndex]) {
		case 0: setObjectState(2, 1); setObjectState(5, 1); break;
		case 1: setObjectState(2, 1); setObjectState(5, 2); break;
		case 2: setObjectState(2, 2); setObjectState(5, 1); break;
		}
		drawAnimation();
		_gfx->copyToScreen();
		break;
	case 6: // Ship weapons
		_animationRunning = false;
		if (!_orbit)
			terminateGame(true);
		else
			gameOver(true);
		break;
	case 7: // Damage report
	{
		// dolSprite(6); // Button animation - handled by top dolSprite
		if (_corePower[_coreIndex] < 2) {
			doText(15, 0); // Critical status
		} else if (!_orbit) {
			doText(49, 0); // Ready for liftoff
		} else {
			doText(66, 0); // Orbital stabilization
		}

		setObjectState(7, 1); // Reset button
		drawAnimation();
		_gfx->copyToScreen();
		break;
	}
		break;
	}
}

void ColonyEngine::moveObject(int index) {
	if (index < 0 || index >= (int)_lSprites.size())
		return;

	ComplexSprite *ls = _lSprites[index];
	const int objectFrames = (int)ls->objects.size();

	// Build link group
	Common::Array<int> linked;
	if (ls->link) {
		for (int i = 0; i < (int)_lSprites.size(); i++)
			if (_lSprites[i]->link == ls->link)
				linked.push_back(i);
	} else {
		linked.push_back(index);
	}

	// Get initial mouse position and animation origin
	Common::Point old = _system->getEventManager()->getMousePos();
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;
	int oldKeyTarget = -1;

	auto findKeyTarget = [&](const Common::Point &screenPt) -> int {
		if (!ls->key)
			return -1;

		const Common::Point pt(screenPt.x - ox, screenPt.y - oy);

		for (int i = (int)_lSprites.size() - 1; i >= 0; --i) {
			if (i == index || !_lSprites[i]->onoff || _lSprites[i]->lock != ls->key)
				continue;

			const ComplexSprite *target = _lSprites[i];
			const int cnum = target->current;
			if (cnum < 0 || cnum >= (int)target->objects.size())
				continue;

			const int spriteIdx = target->objects[cnum].spritenum;
			if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
				continue;

			const Sprite *s = _cSprites[spriteIdx];
			Common::Rect r = s->clip;
			r.translate(target->xloc + target->objects[cnum].xloc,
			            target->yloc + target->objects[cnum].yloc);

			if (pt.x < r.left || pt.x > r.right || pt.y < r.top || pt.y > r.bottom)
				continue;

			const Image *mask = s->mask;
			if (mask && mask->data) {
				const int row = pt.y - r.top;
				const int col = pt.x - r.left;
				const int bitCol = (col + mask->align) * mask->bits;
				const int maskIndex = row * mask->rowBytes + (bitCol / 8);

				if (maskIndex < 0 || maskIndex >= mask->rowBytes * mask->height)
					continue;

				byte maskByte = mask->data[maskIndex];
				if (mask->planes == 2)
					maskByte |= mask->data[mask->rowBytes * mask->height + maskIndex];
				maskByte >>= bitCol % 8;
				if (!(maskByte & ((1 << mask->bits) - 1)))
					continue;
			}

			return i;
		}

		return -1;
	};

	// Drag loop: track mouse while left button held.
	// NOTE: The original DOS hides dragged sprites during drag (setObjectOnOff FALSE)
	// and redraws them separately on top. We improve on this by keeping them visible
	// throughout, and drawing an extra copy on top so they render above drawers.
	while (!shouldQuit()) {
		Common::Event event;
		bool buttonDown = true;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONUP) {
				buttonDown = false;
				break;
			}
		}
		if (!buttonDown)
			break;

		Common::Point cur = _system->getEventManager()->getMousePos();
		int dx = cur.x - old.x;
		int dy = cur.y - old.y;

		if (dx != 0 || dy != 0) {
			// Cycle frame for non-type-2 sprites
			if (ls->type != 2 && (int)ls->objects.size() > 1) {
				ls->current++;
				if (ls->current >= (int)ls->objects.size())
					ls->current = 0;
			}

			// Move all linked sprites
			for (uint i = 0; i < linked.size(); i++) {
				_lSprites[linked[i]]->xloc += dx;
				_lSprites[linked[i]]->yloc += dy;
			}

			// Original MoveObject/KeylSprite: dragging a "key" sprite over a matching
			// lock sprite toggles that target between its first two states.
			const int keyTarget = findKeyTarget(cur);
			if (keyTarget >= 0 && keyTarget != oldKeyTarget) {
				ComplexSprite *target = _lSprites[keyTarget];
				if (target->current == 1) {
					target->current--;
					if (ls->type != 2 && objectFrames > 1) {
						ls->current++;
						if (ls->current >= objectFrames)
							ls->current = 0;
					}
				} else if (target->current == 0) {
					target->current++;
					if (ls->type != 2 && objectFrames > 1) {
						ls->current++;
						if (ls->current >= objectFrames)
							ls->current = 0;
					}
				}
				debugC(1, kColonyDebugAnimation,
					"moveObject: key sprite %d toggled lock sprite %d to state %d",
					index + 1, keyTarget + 1, target->current + 1);
				oldKeyTarget = keyTarget;
			}

			old = cur;
		}

		// Draw all sprites normally, then draw dragged sprites again on top
		// so they appear above drawers and other overlapping sprites
		drawAnimation();
		for (uint i = 0; i < linked.size(); i++)
			drawComplexSprite(linked[i], ox, oy);
		_gfx->copyToScreen();

		responsiveAnimationDelay(_system, 4);
	}

	// Reset frame for non-type-2
	if (ls->type != 2)
		ls->current = 0;

	drawAnimation();
	_gfx->copyToScreen();
}

void ColonyEngine::dolSprite(int index) {
	if (index < 0 || index >= (int)_lSprites.size())
		return;

	ComplexSprite *ls = _lSprites[index];
	int maxFrames = (int)ls->objects.size();

	switch (ls->type) {
	case 0: // Display
		if (!ls->frozen)
			moveObject(index);
		break;
	case 1: // Key and control
		if (ls->frozen) {
			// Container: can open or close if not locked, or if slightly open
			if (!ls->locked || ls->current) {
				if (ls->current > 1) {
					// Close: animate from current down to 0
					while (ls->current > 0) {
						ls->current--;
						// Sync linked sprites via key/lock
						if (ls->key) {
							for (int i = 0; i < (int)_lSprites.size(); i++) {
								if (i != index && _lSprites[i]->lock == ls->key) {
									_lSprites[i]->current = ls->current;
									if (_lSprites[i]->current >= (int)_lSprites[i]->objects.size())
										_lSprites[i]->current = (int)_lSprites[i]->objects.size() - 1;
								}
							}
						}
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 50);
					}
				} else {
					// Open: animate from current up to max
					while (ls->current < maxFrames - 1) {
						ls->current++;
						// Sync linked sprites via key/lock
						if (ls->key) {
							for (int i = 0; i < (int)_lSprites.size(); i++) {
								if (i != index && _lSprites[i]->lock == ls->key) {
									_lSprites[i]->current = ls->current;
									if (_lSprites[i]->current >= (int)_lSprites[i]->objects.size())
										_lSprites[i]->current = (int)_lSprites[i]->objects.size() - 1;
								}
							}
						}
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 50);
					}
				}
			}
		} else {
			moveObject(index);
		}
		break;
	case 2: // Container and object
		if (ls->frozen) {
			if (!ls->locked) {
				// Unlocked container: toggle open/close
				if (ls->current > 0) {
					while (ls->current > 0) {
						ls->current--;
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 50);
					}
				} else {
					while (ls->current < maxFrames - 1) {
						ls->current++;
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 50);
					}
				}
			} else {
				// Locked container: current>1 closes, current==1 opens further
				if (ls->current > 1) {
					while (ls->current > 0) {
						ls->current--;
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 50);
					}
				} else if (ls->current == 1) {
					while (ls->current < maxFrames - 1) {
						ls->current++;
						drawAnimation();
						_gfx->copyToScreen();
						responsiveAnimationDelay(_system, 50);
					}
				}
			}
		} else {
			moveObject(index);
		}
		break;
	default:
		break;
	}
}

void ColonyEngine::setObjectState(int num, int state) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->current = state - 1;
}

int ColonyEngine::objectState(int num) const {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		return _lSprites[num]->current + 1;
	return 0;
}

void ColonyEngine::setObjectOnOff(int num, bool on) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->onoff = on;
}

void ColonyEngine::refreshAnimationDisplay() {
	for (int i = 0; i < 6; i++) {
		if (_animDisplay[i] < 9) {
			setObjectOnOff(13 + i * 2, true);
			setObjectOnOff(14 + i * 2, false);
			setObjectState(13 + i * 2, _animDisplay[i]);
		} else {
			setObjectOnOff(14 + i * 2, true);
			setObjectOnOff(13 + i * 2, false);
			setObjectState(14 + i * 2, _animDisplay[i] - 8);
		}
	}
}

void ColonyEngine::crypt(uint8 sarray[6], int i, int j, int k, int l) {
	int res[6];
	res[0] = ((3 * l) ^ i ^ j ^ k) % 10;
	res[1] = ((i * 3) ^ (j * 7) ^ (k * 11) ^ (l * 13)) % 10;
	res[2] = ((3 + (l * 17)) ^ (j * 19) ^ (k * 23) ^ (i * 29)) % 10;
	res[3] = ((l * 19) ^ (j * 23) ^ (k * 29) ^ (i * 31)) % 10;
	res[4] = ((l * 17) | (j * 19) | (k * 23) | (i * 29)) % 10;
	res[5] = (29 + (l * 17) - (j * 19) - (k * 23) - (i * 29)) % 10;
	for (int m = 0; m < 6; m++) {
		if (res[m] < 0)
			res[m] = -res[m];
		sarray[m] = (uint8)(res[m] + 2);
	}
}

} // End of namespace Colony
