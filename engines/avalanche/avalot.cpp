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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

/* AVALOT		The kernel of the program. */

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"
#include "avalanche/avalot.h"
#include "avalanche/animation.h"
#include "avalanche/dialogs.h"
#include "avalanche/menu.h"
#include "avalanche/pingo.h"
#include "avalanche/timer.h"
#include "avalanche/background.h"
#include "avalanche/closing.h"
#include "avalanche/enums.h"

#include "common/file.h"
#include "common/random.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/config-manager.h"

namespace Avalanche {

const char *Avalot::kVersionNum = "1.30";
const char *Avalot::kCopyright = "1995";

const MouseHotspotType Avalot::kMouseHotSpots[9] = {
	{8,0}, // 1 - up-arrow
	{0,0}, // 2 - screwdriver
	{15,6}, // 3 - right-arrow
	{0,0}, // 4 - fletch
	{8,7}, // 5 - hourglass
	{4,0}, // 6 - TTHand
	{8,5}, // 7- Mark's crosshairs
	{8,7}, // 8- I-beam
	{0,0} // 9 - question mark
};

// Art gallery at 2,1; notice about this at 2,2.
const int32 Avalot::kCatacombMap[8][8] = {
	// Geida's room
	// 1       2	   3     | 4	   5	   6	   7       8
	{0x204,	 0x200,  0xd0f0, 0xf0ff, 0xff,   0xd20f, 0xd200, 0x200},
	{0x50f1, 0x20ff, 0x2ff,  0xff,   0xe0ff, 0x20ff, 0x200f, 0x7210},
	{0xe3f0, 0xe10f, 0x72f0, 0xff,   0xe0ff, 0xff,   0xff,   0x800f},
	{0x2201, 0x2030, 0x800f, 0x220,  0x20f,  0x30,   0xff,   0x23f}, // >> Oubliette
	{0x5024, 0xf3,   0xff,   0x200f, 0x22f0, 0x20f,  0x200,  0x7260},
	{0xf0,   0x2ff,  0xe2ff, 0xff,   0x200f, 0x50f0, 0x72ff, 0x201f},
	{0xf6,   0x220f, 0x22f0, 0x30f,  0xf0,   0x20f,  0x8200, 0x2f0}, // <<< In here
	{0x34,   0x200f, 0x51f0, 0x201f, 0xf1,   0x50ff, 0x902f, 0x2062}
};
// vv Stairs trap.

/* Explanation: $NSEW.
		Nibble N: North.
		0     = no connection,
		2     = (left,) middle(, right) door with left-hand handle,
		5     = (left,) middle(, right) door with right-hand handle,
		7     = arch,
		8     = arch and 1 north of it,
		9     = arch and 2 north of it,
		D     = no connection + WINDOW,
		E     = no connection + TORCH,
		F     = recessed door (to Geida's room.)

		Nibble S: South.
		0     = no connection,
		1,2,3 = left, middle, right door.

		Nibble E: East.
		0     = no connection (wall),
		1     = no connection (wall + window),
		2     = wall with door,
		3     = wall with door and window,
		6     = wall with candles,
		7     = wall with door and candles,
		F     = straight-through corridor.

		Nibble W: West.
		0     = no connection (wall),
		1     = no connection (wall + shield),
		2     = wall with door,
		3     = wall with door and shield,
		4     = no connection (window),
		5     = wall with door and window,
		6     = wall with candles,
		7     = wall with door and candles,
		F     = straight-through corridor. */

const char Avalot::kSpludwicksOrder[3] = {kObjectOnion, kObjectInk, kObjectMushroom};

// A quasiped defines how people who aren't sprites talk. For example, quasiped
// "A" is Dogfood. The rooms aren't stored because I'm leaving that to context.
const QuasipedType Avalot::kQuasipeds[16] = {
//_whichPed, _foregroundColor,   _room,      _backgroundColor,     _who
	{1, kColorLightgray,    kRoomArgentPub,    kColorBrown,    kPeopleDogfood},   // A: Dogfood (screen 19).
	{2, kColorGreen,        kRoomArgentPub,    kColorWhite,    kPeopleIbythneth}, // B: Ibythneth (screen 19).
	{2, kColorWhite,        kRoomYours,        kColorMagenta,  kPeopleArkata},    // C: Arkata (screen 1).
	{2, kColorBlack,        kRoomLustiesRoom,  kColorRed,      kPeopleInvisible}, // D: Hawk (screen 23).
	{2, kColorLightgreen,   kRoomOutsideDucks, kColorBrown,    kPeopleTrader},    // E: Trader (screen 50).
	{5, kColorYellow,       kRoomRobins,       kColorRed,      kPeopleAvalot},    // F: Avvy, tied up (scr.42)
	{1, kColorBlue,         kRoomAylesOffice,  kColorWhite,    kPeopleAyles},     // G: Ayles (screen 16).
	{1, kColorBrown,        kRoomMusicRoom,    kColorWhite,    kPeopleJacques},   // H: Jacques (screen 7).
	{1, kColorLightgreen,   kRoomNottsPub,     kColorGreen,    kPeopleSpurge},    // I: Spurge (screen 47).
	{2, kColorYellow,       kRoomNottsPub,     kColorRed,      kPeopleAvalot},    // J: Avalot (screen 47).
	{1, kColorLightgray,    kRoomLustiesRoom,  kColorBlack,    kPeopleDuLustie},  // K: du Lustie (screen 23).
	{1, kColorYellow,       kRoomOubliette,    kColorRed,      kPeopleAvalot},    // L: Avalot (screen 27).
	{2, kColorWhite,        kRoomOubliette,    kColorRed,      kPeopleInvisible}, // M: Avaroid (screen 27).
	{3, kColorLightgray,    kRoomArgentPub,    kColorDarkgray, kPeopleMalagauche},// N: Malagauche (screen 19).
	{4, kColorLightmagenta, kRoomNottsPub,     kColorRed,      kPeoplePort},      // O: Port (screen 47).
	{1, kColorLightgreen,   kRoomDucks,        kColorDarkgray, kPeopleDrDuck}     // P: Duck (screen 51).
};

const uint16 Avalot::kNotes[12] = {196, 220, 247, 262, 294, 330, 350, 392, 440, 494, 523, 587};
const TuneType Avalot::kTune = {
	kPitchHigher, kPitchHigher, kPitchLower, kPitchSame, kPitchHigher, kPitchHigher, kPitchLower, kPitchHigher, kPitchHigher, kPitchHigher,
	kPitchLower, kPitchHigher, kPitchHigher, kPitchSame, kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchHigher,
	kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchSame, kPitchLower, kPitchHigher, kPitchSame, kPitchLower, kPitchHigher
};

Room Avalot::_whereIs[29] = {
	// The Lads
	kRoomYours, // Avvy
	kRoomSpludwicks, // Spludwick
	kRoomOutsideYours, // Crapulus
	kRoomDucks, // Duck - r__DucksRoom's not defined yet.
	kRoomArgentPub, // Malagauche
	kRoomRobins, // Friar Tuck.
	kRoomDummy, // Robin Hood - can't meet him at the start.
	kRoomBrummieRoad, // Cwytalot
	kRoomLustiesRoom, // Baron du Lustie.
	kRoomOutsideCardiffCastle, // The Duke of Cardiff.
	kRoomArgentPub, // Dogfood
	kRoomOutsideDucks, // Trader
	kRoomArgentPub, // Ibythneth
	kRoomAylesOffice, // Ayles
	kRoomNottsPub, // Port
	kRoomNottsPub, // Spurge
	kRoomMusicRoom, // Jacques
	kRoomNowhere,
	kRoomNowhere,
	kRoomNowhere,
	kRoomNowhere,
	kRoomNowhere,
	kRoomNowhere,
	kRoomNowhere,
	kRoomNowhere,
	// The Lasses
	kRoomYours, // Arkata
	kRoomGeidas, // Geida
	kRoomDummy, // nobody allocated here!
	kRoomWiseWomans  // The Wise Woman.
};

Clock::Clock(AvalancheEngine *vm) {
	_vm = vm;
	_oldHour = _oldHourAngle = _oldMinute = 17717;
}

void Clock::update() { // TODO: Move variables from Gyro to here (or at least somewhere nearby), rename them.
	TimeDate t;
	_vm->_system->getTimeAndDate(t);
	_hour = t.tm_hour;
	_minute = t.tm_min;
	_second = t.tm_sec;

	_hourAngle = (_hour % 12) * 30 + _minute / 2;

	if (_oldHour != _hour)  {
		plotHands();
		chime();
	}

	if (_oldMinute != _minute)
		plotHands();

	if ((_hour == 0) && (_oldHour != 0) && (_oldHour != 17717)) {
		Common::String tmpStr = Common::String::format("Good morning!%c%cYes, it's just past " \
			"midnight. Are you having an all-night Avvy session? Glad you like the game that much!", 
			Dialogs::kControlNewLine, Dialogs::kControlNewLine);
		_vm->_dialogs->displayText(tmpStr);
	}
	_oldHour = _hour;
	_oldHourAngle = _hourAngle;
	_oldMinute = _minute;
}

void Clock::calcHand(uint16 angle, uint16 length, Common::Point &endPoint, byte color) {
	if (angle > 900) {
		endPoint.x = 177;
		return;
	}

	endPoint = _vm->_graphics->drawArc(_vm->_graphics->_surface, kCenterX, kCenterY, 449 - angle, 450 - angle, length, color);
}

void Clock::drawHand(const Common::Point &endPoint, byte color) {
	if (endPoint.x == 177)
		return;

	_vm->_graphics->_surface.drawLine(kCenterX, kCenterY, endPoint.x, endPoint.y, color);
}

void Clock::plotHands() {
	calcHand(_oldHourAngle, 14, _clockHandHour, kColorYellow);
	calcHand(_oldMinute * 6, 17, _clockHandMinute, kColorYellow);
	drawHand(_clockHandHour, kColorBrown);
	drawHand(_clockHandMinute, kColorBrown);

	calcHand(_hourAngle, 14, _clockHandHour, kColorBrown);
	calcHand(_minute * 6, 17, _clockHandMinute, kColorBrown);
	drawHand(_clockHandHour, kColorYellow);
	drawHand(_clockHandMinute, kColorYellow);
}

void Clock::chime() {
	if ((_oldHour == 17717) || (!_vm->_avalot->_soundFx)) // Too high - must be first time around
		return;
	
	byte hour = _hour % 12;
	if (hour == 0)
		hour = 12;

	_vm->_avalot->setMousePointerWait();

	for (int i = 1; i <= hour; i++) {
		for (int j = 1; j <= 3; j++)
			_vm->_sound->playNote((i % 3) * 64 + 140 - j * 30, 50 - j * 12);
		if (i != hour) 
			_vm->_system->delayMillis(100);
	}
}


Avalot::Avalot(AvalancheEngine *vm) : _fxHidden(false), _clock(vm), _interrogation(0) {
	_vm = vm;

	// Needed because of Lucerna::load_also()
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++)
			_also[i][j] = nullptr;
	}

	_totalTime = 0;
}

Avalot::~Avalot() {
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++) {
			if (_also[i][j] != nullptr)  {
				delete _also[i][j];
				_also[i][j] = nullptr;
			}
		}
	}

	for (int i = 0; i < 9; i++) {
		_digits[i].free();
		_directions[i].free();
	}
	_digits[9].free();
}

void Avalot::handleKeyDown(Common::Event &event) {
	_vm->_sound->click();

	if ((Common::KEYCODE_F1 <= event.kbd.keycode) && (event.kbd.keycode <= Common::KEYCODE_F15))
		_vm->_parser->handleFunctionKey(event);
	else if ((32 <= event.kbd.ascii) && (event.kbd.ascii <= 128) && (event.kbd.ascii != 47))
		_vm->_parser->handleInputText(event);
	else
		switch (event.kbd.keycode) { // We can control Avvy with the numpad as well.
		case Common::KEYCODE_KP8:
			event.kbd.keycode = Common::KEYCODE_UP;
			break;
		case Common::KEYCODE_KP2:
			event.kbd.keycode = Common::KEYCODE_DOWN;
			break;
		case Common::KEYCODE_KP6:
			event.kbd.keycode = Common::KEYCODE_RIGHT;
			break;
		case Common::KEYCODE_KP4:
			event.kbd.keycode = Common::KEYCODE_LEFT;
			break;
		case Common::KEYCODE_KP9:
			event.kbd.keycode = Common::KEYCODE_PAGEUP;
			break;
		case Common::KEYCODE_KP3:
			event.kbd.keycode = Common::KEYCODE_PAGEDOWN;
			break;
		case Common::KEYCODE_KP7:
			event.kbd.keycode = Common::KEYCODE_HOME;
			break;
		case Common::KEYCODE_KP1:
			event.kbd.keycode = Common::KEYCODE_END;
			break;
		default:
			break;
	}

	switch (event.kbd.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_END:
	case Common::KEYCODE_KP5:
		if (_alive && _avvyIsAwake) {
			_vm->_animation->handleMoveKey(event); // Fallthroughs are intended.
			drawDirection();
			return;
		}
	case Common::KEYCODE_BACKSPACE:
		_vm->_parser->handleBackspace();
		break;
	case Common::KEYCODE_RETURN:
		_vm->_parser->handleReturn();
		break;
	default:
		break;
	}

	drawDirection();
}

void Avalot::setup() {
	init();

	_vm->_dialogs->reset();
	dusk();
	loadDigits();

	_vm->_parser->_inputTextPos = 0;
	_vm->_parser->_quote = true;

	_vm->_animation->setDirection(kDirStopped);
	_vm->_animation->resetAnims();

	drawToolbar();
	_vm->_dialogs->setReadyLight(2);

	dawn();
	_vm->_parser->_cursorState = false;
	_vm->_parser->cursorOn();
	_vm->_animation->_sprites[0]._speedX = kWalk;
	_vm->_animation->updateSpeed();

	_vm->_menu->init();

	int16 loadSlot = Common::ConfigManager::instance().getInt("save_slot");
	if (loadSlot >= 0) {
		_thinks = 2; // You always have money.
		thinkAbout(kObjectMoney, Avalot::kThing);

		_vm->loadGame(loadSlot);
	} else {
		_isLoaded = false; // Set to true in _vm->loadGame().
		newGame();

		_soundFx = !_soundFx;
		fxToggle();
		thinkAbout(kObjectMoney, Avalot::kThing);

		_vm->_dialogs->displayScrollChain('q', 83); // Info on the game, etc.
	}
}

void Avalot::runAvalot() {
	setup();

	do {
		uint32 beginLoop = _vm->_system->getMillis();

		_vm->updateEvents(); // The event handler.

		_clock.update();
		_vm->_menu->update();
		_vm->_background->update();
		_vm->_animation->animLink();
		checkClick();
		_vm->_timer->updateTimer();

#ifdef DEBUG
		for (int i = 0; i < _lineNum; i++) {
			LineType *curLine = &_lines[i];
			_vm->_graphics->_surface.drawLine(curLine->_x1, curLine->_y1, curLine->_x2, curLine->_y2, curLine->col);
		}

		for (int i = 0; i < _fieldNum; i++) {
			FieldType *curField = &_fields[i];
			if (curField->_x1 < 640)
				_vm->_graphics->_surface.frameRect(Common::Rect(curField->_x1, curField->_y1, curField->_x2, curField->_y2), kColorLightmagenta);
		}
#endif

		_vm->_graphics->refreshScreen();

		uint32 delay = _vm->_system->getMillis() - beginLoop;
		if (delay <= 55)
			_vm->_system->delayMillis(55 - delay); // Replaces slowdown(); 55 comes from 18.2 Hz (B Flight).
	} while (!_letMeOut && !_vm->shouldQuit());

	warning("STUB: Avalot::run()");

	_vm->_closing->exitGame();
}

void Avalot::init() {
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++)
			_also[i][j] = nullptr;
	}

#if 0
	if (_vm->_enhanced->atbios)
		atkey = "f1";
	else
		atkey = "alt-";
#endif

	_mouse = kMouseStateNo;
	_letMeOut = false;
	_holdTheDawn = true;
	_currentMouse = 177;
	_dropsOk = true;
	_mouseText = "";
	_cheat = false;
	_cp = 0;
	_ledStatus = 177;
	_defaultLed = 2;
	_enidFilename = ""; // Undefined.
	for (int i = 0; i < 3; i++)
		_scoreToDisplay[i] = -1; // Impossible digits.
	_holdTheDawn = false;

	setMousePointerWait();
	CursorMan.showMouse(true);
}

/**
 * Call a given Verb
 * @remarks	Originally called 'callverb'
 */
void Avalot::callVerb(VerbCode id) {
	if (id == _vm->_parser->kPardon) {
		Common::String tmpStr = Common::String::format("The f5 key lets you do a particular action in certain " \
			"situations. However, at the moment there is nothing assigned to it. You may press alt-A to see " \
			"what the current setting of this key is.");
		_vm->_dialogs->displayText(tmpStr);
	} else {
		_weirdWord = false;
		_vm->_parser->_polite = true;
		_vm->_parser->_verb = id;
		_vm->_parser->doThat();
	}
}

void Avalot::drawAlsoLines() {
	CursorMan.showMouse(false);

	_vm->_graphics->_magics.fillRect(Common::Rect(0, 0, 640, 200), 0);
	_vm->_graphics->_magics.frameRect(Common::Rect(0, 45, 640, 161), 15);

	for (int i = 0; i < _lineNum; i++) {
		// We had to check if the lines are within the borders of the screen.
		if ((_lines[i]._x1 >= 0) && (_lines[i]._x1 < _vm->_graphics->kScreenWidth) && (_lines[i]._y1 >= 0) && (_lines[i]._y1 < _vm->_graphics->kScreenHeight)
		 && (_lines[i]._x2 >= 0) && (_lines[i]._x2 < _vm->_graphics->kScreenWidth) && (_lines[i]._y2 >= 0) && (_lines[i]._y2 < _vm->_graphics->kScreenHeight))
			_vm->_graphics->_magics.drawLine(_lines[i]._x1, _lines[i]._y1, _lines[i]._x2, _lines[i]._y2, _lines[i]._color);
	}

	CursorMan.showMouse(true);
}

/**
 * Check is it's possible to give something to Spludwick
 * @remarks	Originally called 'nextstring'
 */
Common::String Avalot::readAlsoStringFromFile() {
	Common::String str;
	byte length = file.readByte();
	for (int i = 0; i < length; i++)
		str += file.readByte();
	return str;
}

void Avalot::scram(Common::String &str) {
	for (uint i = 0; i < str.size(); i++)
		str.setChar(str[i] ^ 177, i);
}

void Avalot::unScramble() {
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++) {
			if (_also[i][j] != nullptr)
				scram(*_also[i][j]);
		}
	}
	scram(_listen);
	scram(_flags);
}

void Avalot::loadAlso(byte num) {
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++) {
			if (_also[i][j] != nullptr)  {
				delete _also[i][j];
				_also[i][j] = nullptr;
			}
		}
	}
	Common::String filename;
	filename = Common::String::format("also%d.avd", num);
	if (!file.open(filename))
		error("AVALANCHE: Lucerna: File not found: %s", filename.c_str());

	file.seek(128);

	byte alsoNum = file.readByte();
	Common::String tmpStr;
	for (int i = 0; i <= alsoNum; i++) {
		for (int j = 0; j < 2; j++) {
			_also[i][j] = new Common::String;
			*_also[i][j] = readAlsoStringFromFile();
		}
		tmpStr = Common::String::format("\x9D%s\x9D", _also[i][0]->c_str());
		*_also[i][0] = tmpStr;
	}

	memset(_lines, 0xFF, sizeof(_lines));

	_lineNum = file.readByte();
	for (int i = 0; i < _lineNum; i++) {
		LineType *curLine = &_lines[i];
		curLine->_x1 = file.readSint16LE();
		curLine->_y1 = file.readSint16LE();
		curLine->_x2 = file.readSint16LE();
		curLine->_y2 = file.readSint16LE();
		curLine->_color = file.readByte();
	}

	memset(_peds, 177, sizeof(_peds));
	byte pedNum = file.readByte();
	for (int i = 0; i < pedNum; i++) {
		PedType *curPed = &_peds[i];
		curPed->_x = file.readSint16LE();
		curPed->_y = file.readSint16LE();
		curPed->_direction = (Direction)file.readByte();
	}

	_fieldNum = file.readByte();
	for (int i = 0; i < _fieldNum; i++) {
		FieldType *curField = &_fields[i];
		curField->_x1 = file.readSint16LE();
		curField->_y1 = file.readSint16LE();
		curField->_x2 = file.readSint16LE();
		curField->_y2 = file.readSint16LE();
	}

	for (int i = 0; i < 15; i++) {
		MagicType *magic = &_magics[i];
		magic->_operation = file.readByte();
		magic->_data = file.readUint16LE();
	}

	for (int i = 0; i < 7; i++) {
		MagicType *portal = &_portals[i];
		portal->_operation = file.readByte();
		portal->_data = file.readUint16LE();
	}

	_flags.clear();
	for (int i = 0;  i < 26; i++)
		_flags += file.readByte();

	int16 listen_length = file.readByte();
	_listen.clear();
	for (int i = 0; i < listen_length; i++)
		_listen += file.readByte();

	drawAlsoLines();

	file.close();
	unScramble();
	for (int i = 0; i <= alsoNum; i++) {
		tmpStr = Common::String::format(",%s,", _also[i][0]->c_str());
		*_also[i][0] = tmpStr;
	}
}

void Avalot::loadRoom(byte num) {
	CursorMan.showMouse(false);

	_vm->_graphics->fleshColors();

	Common::String filename = Common::String::format("place%d.avd", num);
	if (!file.open(filename))
		error("AVALANCHE: Lucerna: File not found: %s", filename.c_str());

	file.seek(146);
	if (!_roomnName.empty())
		_roomnName.clear();
	for (int i = 0; i < 30; i++) {
		char actChar = file.readByte();
		if ((32 <= actChar) && (actChar <= 126))
			_roomnName += actChar;
	}
	// Compression method byte follows this...

	file.seek(177);

	_vm->_graphics->_background = _vm->_graphics->loadPictureRow(file, _vm->_graphics->kBackgroundWidth, _vm->_graphics->kBackgroundHeight);
	_vm->_graphics->refreshBackground();

	file.close();

	loadAlso(num);
	_vm->_background->load(num);
	CursorMan.showMouse(true);
}

void Avalot::zoomOut(int16 x, int16 y) {
	//setlinestyle(dottedln, 0, 1); TODO: Implement it with a dotted line style!!!

	::Graphics::Surface backup;
	backup.copyFrom(_vm->_graphics->_surface);
	
	for (byte i = 1; i <= 20; i ++) {
		int16 x1 = x - (x / 20) * i;
		int16 y1 = y - ((y - 10) / 20) * i;
		int16 x2 = x + (((639 - x) / 20) * i);
		int16 y2 = y + (((161 - y) / 20) * i);

		_vm->_graphics->_surface.frameRect(Common::Rect(x1, y1, x2, y2), kColorWhite);
		_vm->_graphics->refreshScreen();
		_vm->_system->delayMillis(17);
		_vm->_graphics->_surface.copyFrom(backup);
		_vm->_graphics->refreshScreen();
	}

	backup.free();
}

void Avalot::findPeople(byte room) {
	for (int i = 1; i < 29; i++) {
		if (_whereIs[i] == room) {
			if (i < 25)
				_him = (People)(150 + i);
			else
				_her = (People)(150 + i);
		}
	}
}

void Avalot::exitRoom(byte x) {
	_vm->_sound->stopSound();
	_vm->_background->release();
	_seeScroll = true;  // This stops the trippancy system working over the length of this procedure.

	switch (x) {
	case kRoomSpludwicks:
		_vm->_timer->loseTimer(Timer::kReasonAvariciusTalks);
		 _avariciusTalk = 0;
		// He doesn't HAVE to be talking for this to work. It just deletes it IF it exists.
		break;
	case kRoomBridge:
		if (_drawbridgeOpen > 0) {
			_drawbridgeOpen = 4; // Fully open.
			_vm->_timer->loseTimer(Timer::kReasonDrawbridgeFalls);
		}
		break;
	case kRoomOutsideCardiffCastle:
		_vm->_timer->loseTimer(Timer::kReasonCardiffsurvey);
		break;
	case kRoomRobins:
		_vm->_timer->loseTimer(Timer::kReasonGettingTiedUp);
		break;
	}

	_interrogation = 0; // Leaving the room cancels all the questions automatically.
	_seeScroll = false; // Now it can work again!

	_lastRoom = _room;
	if (_room != kRoomMap)
		_lastRoomNotMap = _room;
}


/**
 * Only when entering a NEW town! Not returning to the last one,
 * but choosing another from the map.
 * @remarks	Originally called 'new_town'
 */
void Avalot::enterNewTown() {
	_vm->_menu->setup();

	switch (_room) {
	case kRoomOutsideNottsPub: // Entry into Nottingham.
		if ((_roomCount[kRoomRobins] > 0) && (_beenTiedUp) && (!_takenMushroom))
			_mushroomGrowing = true;
		break;
	case kRoomWiseWomans: // Entry into Argent.
		if (_talkedToCrapulus && (!_lustieIsAsleep)) {
			_spludwickAtHome = !((_roomCount[kRoomWiseWomans] % 3) == 1);
			_crapulusWillTell = !_spludwickAtHome;
		} else {
			_spludwickAtHome = true;
			_crapulusWillTell = false;
		}
		if (_boxContent == kObjectWine)
			_wineState = 3; // Vinegar
		break;
	}

	if ((_room != kRoomOutsideDucks) && (_objects[kObjectOnion - 1]) && !(_onionInVinegar))
		_rottenOnion = true; // You're holding the onion
}

void Avalot::putGeidaAt(byte whichPed, byte ped) {
	if (ped == 0)
		return;
	AnimationType *spr1 = &_vm->_animation->_sprites[1];

	spr1->init(5, false, _vm->_animation); // load Geida
	_vm->_animation->appearPed(1, whichPed);
	spr1->_callEachStepFl = true;
	spr1->_eachStepProc = Animation::kProcGeida;
}

void Avalot::enterRoom(Room roomId, byte ped) {
	_seeScroll = true;  // This stops the trippancy system working over the length of this procedure.

	findPeople(roomId);
	_room = roomId;
	if (ped != 0)
		_roomCount[roomId]++;

	loadRoom(roomId);

	if ((_roomCount[roomId] == 0) && (!setFlag('S')))
		incScore(1);

	_whereIs[kPeopleAvalot - 150] = _room;

	if (_geidaFollows)
		_whereIs[kPeopleGeida - 150] = roomId;

	_roomTime = 0;


	if ((_lastRoom == kRoomMap) && (_lastRoomNotMap != _room))
		enterNewTown();

	switch (roomId) {
	case kRoomYours:
		if (_avvyInBed) {
			_vm->_background->draw(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_timer->addTimer(100, Timer::kProcArkataShouts, Timer::kReasonArkataShouts);
		}
		break;

	case kRoomOutsideYours:
		if (ped > 0) {
			AnimationType *spr1 = &_vm->_animation->_sprites[1];
			if (!_talkedToCrapulus) {
				_whereIs[kPeopleCrapulus - 150] = kRoomOutsideYours;
				spr1->init(8, false, _vm->_animation); // load Crapulus

				if (_roomCount[kRoomOutsideYours] == 1) {
					_vm->_animation->appearPed(1, 3); // Start on the right-hand side of the screen.
					spr1->walkTo(4); // Walks up to greet you.
				} else {
					_vm->_animation->appearPed(1, 4); // Starts where he was before.
					spr1->_facingDir = kDirLeft;
				}

				spr1->_callEachStepFl = true;
				spr1->_eachStepProc = Animation::kProcFaceAvvy; // He always faces Avvy.

			} else
				_whereIs[kPeopleCrapulus - 150] = kRoomNowhere;

			if (_crapulusWillTell) {
				spr1->init(8, false, _vm->_animation);
				_vm->_animation->appearPed(1, 1);
				spr1->walkTo(3);
				_vm->_timer->addTimer(20, Timer::kProcCrapulusSpludOut, Timer::kReasonCrapulusSaysSpludwickOut);
				_crapulusWillTell = false;
			}
		}
		break;

	case kRoomOutsideSpludwicks:
		if ((_roomCount[kRoomOutsideSpludwicks] == 1) && (ped == 1)) {
			_vm->_timer->addTimer(20, Timer::kProcBang, Timer::kReasonExplosion);
			_spludwickAtHome = true;
		}
		break;

	case kRoomSpludwicks:
		if (_spludwickAtHome) {
			AnimationType *spr1 = &_vm->_animation->_sprites[1];
			if (ped > 0) {
				spr1->init(2, false, _vm->_animation); // load Spludwick
				_vm->_animation->appearPed(1, 1);
				_whereIs[kPeopleSpludwick - 150] = kRoomSpludwicks;
			}

			spr1->_callEachStepFl = true;
			spr1->_eachStepProc = Animation::kProcGeida;
		} else
			_whereIs[kPeopleSpludwick - 150] = kRoomNowhere;
		break;

	case kRoomBrummieRoad:
		if (_geidaFollows)
			putGeidaAt(4, ped);
		if (_cwytalotGone) {
			_magics[kColorLightred - 1]._operation = Avalot::kMagicNothing;
			_whereIs[kPeopleCwytalot - 150] = kRoomNowhere;
		} else if (ped > 0) {
			AnimationType *spr1 = &_vm->_animation->_sprites[1];
			spr1->init(4, false, _vm->_animation); // 4 = Cwytalot
			spr1->_callEachStepFl = true;
			spr1->_eachStepProc = Animation::kProcFollowAvvyY;
			_whereIs[kPeopleCwytalot - 150] = kRoomBrummieRoad;

			if (_roomCount[kRoomBrummieRoad] == 1) { // First time here...
				_vm->_animation->appearPed(1, 1); // He appears on the right of the screen...
				spr1->walkTo(3); // ...and he walks up...
			} else {
				// You've been here before.
				_vm->_animation->appearPed(1, 3); // He's standing in your way straight away...
				spr1->_facingDir = kDirLeft;
			}
		}
		break;

	case kRoomArgentRoad:
		if ((_cwytalotGone) && (!_passedCwytalotInHerts) && (ped == 2) && (_roomCount[kRoomArgentRoad] > 3)) {
			AnimationType *spr1 = &_vm->_animation->_sprites[1];
			spr1->init(4, false, _vm->_animation); // 4 = Cwytalot again
			_vm->_animation->appearPed(1, 0);
			spr1->walkTo(1);
			spr1->_vanishIfStill = true;
			_passedCwytalotInHerts = true;
			// whereis[#157] = r__Nowhere; // can we fit this in?
			_vm->_timer->addTimer(20, Timer::kProcCwytalotInHerts, Timer::kReasonCwytalotInHerts);
		}
		break;

	case kRoomBridge:
		if (_drawbridgeOpen == 4) { // open
			_vm->_background->draw(-1, -1, 2); // Position of drawbridge
			_vm->_graphics->refreshBackground();
			_magics[kColorGreen - 1]._operation = Avalot::kMagicNothing; // You may enter the drawbridge.
		}
		if (_geidaFollows)
			putGeidaAt(ped + 2, ped); // load Geida
		break;

	case kRoomRobins:
		if ((ped > 0) && (!_beenTiedUp)) {
			// A welcome party... or maybe not...
			AnimationType *spr1 = &_vm->_animation->_sprites[1];
			spr1->init(6, false, _vm->_animation);
			_vm->_animation->appearPed(1, 1);
			spr1->walkTo(2);
			_vm->_timer->addTimer(36, Timer::kProcGetTiedUp, Timer::kReasonGettingTiedUp);
		}

		if (_beenTiedUp) {
			_whereIs[kPeopleRobinHood - 150] = kRoomNowhere;
			_whereIs[kPeopleFriarTuck - 150] = kRoomNowhere;
		}

		if (_tiedUp)
			_vm->_background->draw(-1, -1, 1);

		if (!_mushroomGrowing)
			_vm->_background->draw(-1, -1, 2);
		_vm->_graphics->refreshBackground();
		break;

	case kRoomOutsideCardiffCastle:
		if (ped > 0) {
			AnimationType *spr1 = &_vm->_animation->_sprites[1];
			switch (_cardiffQuestionNum) {
			case 0 : // You've answered NONE of his questions.
				spr1->init(9, false, _vm->_animation);
				_vm->_animation->appearPed(1, 1);
				spr1->walkTo(2);
				_vm->_timer->addTimer(47, Timer::kProcCardiffSurvey, Timer::kReasonCardiffsurvey);
				break;
			case 5 :
				_magics[1]._operation = Avalot::kMagicNothing;
				break; // You've answered ALL his questions. => nothing happens.
			default: // You've answered SOME of his questions.
				spr1->init(9, false, _vm->_animation);
				_vm->_animation->appearPed(1, 2);
				spr1->_facingDir = kDirRight;
				_vm->_timer->addTimer(3, Timer::kProcCardiffReturn, Timer::kReasonCardiffsurvey);
			}
		}

		if (_cardiffQuestionNum < 5)
			_interrogation = _cardiffQuestionNum;
		else
			_interrogation = 0;
		break;

	case kRoomMap:
		// You're entering the map.
		dawn();
		if (ped > 0)
			zoomOut(_peds[ped]._x, _peds[ped]._y);

		if ((_objects[kObjectWine - 1]) && (_wineState != 3)) {
			_vm->_dialogs->displayScrollChain('q', 9); // Don't want to waste the wine!
			_objects[kObjectWine - 1] = false;
			refreshObjectList();
		}

		_vm->_dialogs->displayScrollChain('q', 69);
		break;

	case kRoomCatacombs:
		if ((ped == 0) || (ped == 3) || (ped == 5) || (ped == 6)) {

			switch (ped) {
			case 3: // Enter from oubliette
				_catacombX = 8;
				_catacombY = 4;
				break;
			case 5: // Enter from du Lustie's
				_catacombX = 8;
				_catacombY = 7;
				break;
			case 6: // Enter from Geida's
				_catacombX = 4;
				_catacombY = 1;
				break;
			}

			_enterCatacombsFromLustiesRoom = true;
			_vm->_animation->catacombMove(ped);
			_enterCatacombsFromLustiesRoom = false;
		}
		break;

	case kRoomArgentPub:
		if (_wonNim)
			_vm->_background->draw(-1, -1, 0);   // No lute by the settle.
		_malagauche = 0; // Ready to boot Malagauche
		if (_givenBadgeToIby) {
			_vm->_background->draw(-1, -1, 7);
			_vm->_background->draw(-1, -1, 8);
		}
		_vm->_graphics->refreshBackground();
		break;

	case kRoomLustiesRoom:
		_npcFacing = 1; // du Lustie.
		if (_vm->_animation->_sprites[0]._id == 0) // Avvy in his normal clothes
			_vm->_timer->addTimer(3, Timer::kProcCallsGuards, Timer::kReasonDuLustieTalks);
		else if (!_enteredLustiesRoomAsMonk) // already
			// Presumably, Avvy dressed as a monk.
			_vm->_timer->addTimer(3, Timer::kProcGreetsMonk, Timer::kReasonDuLustieTalks);

		if (_geidaFollows) {
			putGeidaAt(4, ped);
			if (_lustieIsAsleep) {
				_vm->_background->draw(-1, -1, 4);
				_vm->_graphics->refreshBackground();
			}
		}
		break;

	case kRoomMusicRoom:
		if (_jacquesState > 0) {
			_jacquesState = 5;
			_vm->_background->draw(-1, -1, 1);
			_vm->_graphics->refreshBackground();
			_vm->_background->draw(-1, -1, 3);
			_magics[kColorBrown - 1]._operation = Avalot::kMagicNothing;
			_whereIs[kPeopleJacques - 150] = kRoomNowhere;
		}
		if (ped != 0) {
			_vm->_background->draw(-1, -1, 5);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(6);
			_vm->_sequence->thenShow(5);
			_vm->_sequence->thenShow(7);
			_vm->_sequence->startToClose();
		}
		break;

	case kRoomOutsideNottsPub:
		if (ped == 2) {
			_vm->_background->draw(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(3);
			_vm->_sequence->thenShow(2);
			_vm->_sequence->thenShow(1);
			_vm->_sequence->thenShow(4);
			_vm->_sequence->startToClose();
		}
		break;

	case kRoomOutsideArgentPub:
		if (ped == 2)  {
			_vm->_background->draw(-1, -1, 5);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(6);
			_vm->_sequence->thenShow(5);
			_vm->_sequence->thenShow(7);
			_vm->_sequence->startToClose();
		}
		break;

	case kRoomWiseWomans: {
		AnimationType *spr1 = &_vm->_animation->_sprites[1];
		spr1->init(11, false, _vm->_animation);
		if ((_roomCount[kRoomWiseWomans] == 1) && (ped > 0)) {
			_vm->_animation->appearPed(1, 1); // Start on the right-hand side of the screen.
			spr1->walkTo(3); // Walks up to greet you.
		} else {
			_vm->_animation->appearPed(1, 3); // Starts where she was before.
			spr1->_facingDir = kDirLeft;
		}

		spr1->_callEachStepFl = true;
		spr1->_eachStepProc = Animation::kProcFaceAvvy; // She always faces Avvy.
		}
		break;

	case kRoomInsideCardiffCastle:
		if (ped > 0) {
			_vm->_animation->_sprites[1].init(10, false, _vm->_animation); // Define the dart.
			_vm->_background->draw(-1, -1, 0);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(1);
			if (_arrowInTheDoor)
				_vm->_sequence->thenShow(3);
			else
				_vm->_sequence->thenShow(2);

			if (_takenPen)
				_vm->_background->draw(-1, -1, 3);

			_vm->_sequence->startToClose();
		} else {
			_vm->_background->draw(-1, -1, 0);
			if (_arrowInTheDoor)
				_vm->_background->draw(-1, -1, 2);
			else
				_vm->_background->draw(-1, -1, 1);
			_vm->_graphics->refreshBackground();
		}
		break;

	case kRoomAvvysGarden:
		if (ped == 1)  {
			_vm->_background->draw(-1, -1, 1);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(2);
			_vm->_sequence->thenShow(1);
			_vm->_sequence->thenShow(3);
			_vm->_sequence->startToClose();
		}
		break;

	case kRoomEntranceHall:
	case kRoomInsideAbbey:
	case kRoomYourHall:
		if (ped == 2)  {
#if 0
			// It was the original:
			_vm->_celer->show_one(-1, -1, 2);
			_vm->_sequence->first_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
#endif

			_vm->_background->draw(-1, -1, 1);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(2);
			_vm->_sequence->thenShow(1);
			_vm->_sequence->thenShow(3);
			_vm->_sequence->startToClose();
		}
		break;

	case kRoomAylesOffice:
		if (_aylesIsAwake)
			_vm->_background->draw(-1, -1, 1);
		_vm->_graphics->refreshBackground();
		break; // Ayles awake.

	case kRoomGeidas:
		putGeidaAt(1, ped);
		break; // load Geida

	case kRoomEastHall:
	case kRoomWestHall:
		if (_geidaFollows)
			putGeidaAt(ped + 1, ped);
		break;

	case kRoomLusties:
		if (_geidaFollows)
			putGeidaAt(ped + 5, ped);
		break;

	case kRoomNottsPub:
		if (_sittingInPub)
			_vm->_background->draw(-1, -1, 2);
		_npcFacing = 1; // Port.
		break;

	case kRoomOutsideDucks:
		if (ped == 2) {
			// Shut the door
			_vm->_background->draw(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->firstShow(3);
			_vm->_sequence->firstShow(2);
			_vm->_sequence->thenShow(1);
			_vm->_sequence->thenShow(4);
			_vm->_sequence->startToClose();
		}
		break;

	case kRoomDucks:
		_npcFacing = 1; // Duck.
		break; 
	}

	_seeScroll = false; // Now it can work again!
	_isLoaded = false;
}

void Avalot::thinkAbout(byte object, bool type) {
	const int16 picSize = 966;

	_thinks = object;
	object--;

	setMousePointerWait();

	if (type == Avalot::kThing) {
		if (!file.open("thinks.avd"))
			error("AVALANCHE: Lucerna: File not found: thinks.avd");
	} else { // Avalot::kPerson
		if (!file.open("folk.avd"))
			error("AVALANCHE: Lucerna: File not found: folk.avd");

		object -= 149;
		if (object >= 25)
			object -= 8;
		if (object == 20)
			object--; // Last time...
	}

	CursorMan.showMouse(false);

	file.seek(object * picSize + 65);
	::Graphics::Surface picture = _vm->_graphics->loadPictureGraphic(file);
	_vm->_graphics->drawPicture(_vm->_graphics->_surface, picture, 205, 170);

	picture.free();
	file.close();

	CursorMan.showMouse(true);
	_thinkThing = type;
}

void Avalot::loadDigits() {   // Load the scoring digits & rwlites
	const byte digitsize = 134;
	const byte rwlitesize = 126;

	if (!file.open("digit.avd"))
		error("AVALANCHE: Lucerna: File not found: digit.avd");

	for (int i = 0; i < 10; i++) {
		file.seek(i * digitsize);
		_digits[i] = _vm->_graphics->loadPictureGraphic(file);
	}

	for (int i = 0; i < 9; i++) {
		file.seek(10 * digitsize + i * rwlitesize);
		_directions[i] = _vm->_graphics->loadPictureGraphic(file);
	}

	file.close();
}

void Avalot::drawToolbar() {
	if (!file.open("useful.avd"))
		error("AVALANCHE: Lucerna: File not found: useful.avd");

	file.seek(40);

	CursorMan.showMouse(false);
	::Graphics::Surface picture = _vm->_graphics->loadPictureGraphic(file);
	_vm->_graphics->drawPicture(_vm->_graphics->_surface, picture, 5, 169);

	picture.free();
	file.close();

	CursorMan.showMouse(true);
	_vm->_animation->setOldDirection(kDirNone);
	drawDirection();
}

void Avalot::drawScore() {
	uint16 score = _dnascore;
	int8 numbers[3] = {0, 0, 0};
	for (int i = 0; i < 2; i++) {
		byte divisor = 1;
		for (int j = 0; j < (2 - i); j++)
			divisor *= 10;
		numbers[i] = score / divisor;
		score -= numbers[i] * divisor;
	}
	numbers[2] = score;

	CursorMan.showMouse(false);

	for (int i = 0; i < 3; i++) {
		if (_scoreToDisplay[i] != numbers[i])
			_vm->_graphics->drawPicture(_vm->_graphics->_surface, _digits[numbers[i]], 250 + (i + 1) * 15, 177);
	}

	CursorMan.showMouse(true);

	for (int i = 0; i < 3; i++)
		_scoreToDisplay[i] = numbers[i];
}

void Avalot::incScore(byte num) {    
	for (int i = 1; i <= num; i++) {
		_dnascore++;

		if (_soundFx) {
			for (int j = 1; j <= 97; j++)
				// Length os 2 is a guess, the original doesn't have a delay specified
				_vm->_sound->playNote(177 + _dnascore * 3, 2);
		}
	}
	warning("STUB: Avalot::points()");

	drawScore();
}

void Avalot::useCompass(const Common::Point &cursorPos) {
	byte color = *(byte *)_vm->_graphics->_surface.getBasePtr(cursorPos.x, cursorPos.y / 2);

	switch (color) {
	case kColorGreen:
		_vm->_animation->setDirection(kDirUp);
		_vm->_animation->setMoveSpeed(0, kDirUp);
		drawDirection();
		break;
	case kColorBrown:
		_vm->_animation->setDirection(kDirDown);
		_vm->_animation->setMoveSpeed(0, kDirDown);
		drawDirection();
		break;
	case kColorCyan:
		_vm->_animation->setDirection(kDirLeft);
		_vm->_animation->setMoveSpeed(0, kDirLeft);
		drawDirection();
		break;
	case kColorLightmagenta:
		_vm->_animation->setDirection(kDirRight);
		_vm->_animation->setMoveSpeed(0, kDirRight);
		drawDirection();
		break;
	case kColorRed:
	case kColorWhite:
	case kColorLightcyan:
	case kColorYellow: // Fall-throughs are intended.
		_vm->_animation->stopWalking();
		drawDirection();
		break;
	}
}

void Avalot::fxToggle() {
	warning("STUB: Avalot::fxtoggle()");
}

void Avalot::refreshObjectList() {
	_carryNum = 0;
	if (_thinkThing && !_objects[_thinks - 1])
		thinkAbout(kObjectMoney, Avalot::kThing); // you always have money

	for (int i = 0; i < kObjectNum; i++) {
		if (_objects[i]) {
			_objectList[_carryNum] = i + 1;
			_carryNum++;
		}
	}
}

/**
 * @remarks	Originally called 'verte'
 */
void Avalot::guideAvvy(Common::Point cursorPos) {
	if (!_userMovesAvvy)
		return;

	cursorPos.y /= 2;
	byte what;

	// _vm->_animation->tr[0] is Avalot.)
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	if (cursorPos.x < avvy->_x)
		what = 1;
	else if (cursorPos.x > (avvy->_x + avvy->_info._xLength))
		what = 2;
	else
		what = 0; // On top

	if (cursorPos.y < avvy->_y)
		what += 3;
	else if (cursorPos.y > (avvy->_y + avvy->_info._yLength))
		what += 6;

	switch (what) {
	case 0:
		_vm->_animation->stopWalking();
		break; // Clicked on Avvy: no movement.
	case 1:
		_vm->_animation->setMoveSpeed(0, kDirLeft);
		break;
	case 2:
		_vm->_animation->setMoveSpeed(0, kDirRight);
		break;
	case 3:
		_vm->_animation->setMoveSpeed(0, kDirUp);
		break;
	case 4:
		_vm->_animation->setMoveSpeed(0, kDirUpLeft);
		break;
	case 5:
		_vm->_animation->setMoveSpeed(0, kDirUpRight);
		break;
	case 6:
		_vm->_animation->setMoveSpeed(0, kDirDown);
		break;
	case 7:
		_vm->_animation->setMoveSpeed(0, kDirDownLeft);
		break;
	case 8:
		_vm->_animation->setMoveSpeed(0, kDirDownRight);
		break;
	}    // No other values are possible.

	drawDirection();
}

void Avalot::checkClick() {
	Common::Point cursorPos = _vm->getMousePos();
	_onToolbar = kSlowComputer && ((cursorPos.y >= 169) || (cursorPos.y <= 10));

	/*if (mrelease > 0)
		after_the_scroll = false;*/

	if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
		newMouse(0); // up arrow
	else if ((317 <= cursorPos.y) && (cursorPos.y <= 339))
		newMouse(7); //I-beam
	else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
		newMouse(1); // screwdriver
	else if (!_vm->_menu->isActive()) { // Dropdown can handle its own pointers.
		if (_holdLeftMouse) {
			newMouse(6); // Mark's crosshairs
			guideAvvy(cursorPos); // Normally, if you click on the picture, you're guiding Avvy around.
		} else
			newMouse(3); // fletch
	}

	if (_holdLeftMouse) {
		if ((0 <= cursorPos.y) && (cursorPos.y <= 21)) { // Click on the dropdown menu.
			if (_dropsOk)
				_vm->_menu->update();
		} else if ((317 <= cursorPos.y) && (cursorPos.y <= 339)) { // Click on the command line.
			_vm->_parser->_inputTextPos = (cursorPos.x - 23) / 8;
			if (_vm->_parser->_inputTextPos > _vm->_parser->_inputText.size() + 1)
				_vm->_parser->_inputTextPos = _vm->_parser->_inputText.size() + 1;
			if (_vm->_parser->_inputTextPos < 1)
				_vm->_parser->_inputTextPos = 1;
			_vm->_parser->_inputTextPos--;
			_vm->_parser->plotText();
		} else if ((340 <= cursorPos.y) && (cursorPos.y <= 399)) { // Check the toolbar.
			if ((137 <= cursorPos.x) && (cursorPos.x <= 207)) { // Control Avvy with the compass.
				if (_alive && _avvyIsAwake)
					useCompass(cursorPos);
			} else if ((208 <= cursorPos.x) && (cursorPos.x <= 260)) { // Examine the _thing.
				do {
					_vm->updateEvents();
				} while (_holdLeftMouse);

				if (_thinkThing) {
					_vm->_parser->_thing = _thinks;
					_vm->_parser->_thing += 49;
					_vm->_parser->_person = kPeoplePardon;
				} else {
					_vm->_parser->_person = (People) _thinks;
					_vm->_parser->_thing = _vm->_parser->kPardon;
				}
				callVerb(kVerbCodeExam);
			} else if ((261 <= cursorPos.x) && (cursorPos.x <= 319)) { // Display the score.
				do {
					_vm->updateEvents();
				} while (_holdLeftMouse);

				callVerb(kVerbCodeScore);
			} else if ((320 <= cursorPos.x) && (cursorPos.x <= 357)) { // Change speed.
				_vm->_animation->_sprites[0]._speedX = kWalk;
				_vm->_animation->updateSpeed();
			} else if ((358 <= cursorPos.x) && (cursorPos.x <= 395)) { // Change speed.
				_vm->_animation->_sprites[0]._speedX = kRun;
				_vm->_animation->updateSpeed();
			} else if ((396 <= cursorPos.x) && (cursorPos.x <= 483))
				fxToggle();
			else if ((535 <= cursorPos.x) && (cursorPos.x <= 640))
				_mouseText.insertChar(Dialogs::kControlNewLine, 0);
		} else if (!_dropsOk)
			_mouseText = Common::String(13) + _mouseText;
	}
}

void Avalot::errorLed() {
	warning("STUB: Avalot::errorled()");
}

int8 Avalot::fades(int8 x) {
	warning("STUB: Avalot::fades()");
	return 0;
}

void Avalot::fadeOut(byte n) {
	warning("STUB: Avalot::fadeOut()");
}

void Avalot::dusk() {
	warning("STUB: Avalot::dusk()");
}

void Avalot::fadeIn(byte n) {
	warning("STUB: Avalot::fadeIn()");
}

void Avalot::dawn() {
	warning("STUB: Avalot::dawn()");
}

void Avalot::drawDirection() { // It's data is loaded in load_digits().
	if (_vm->_animation->getOldDirection() == _vm->_animation->getDirection())
		return;

	_vm->_animation->setOldDirection(_vm->_animation->getDirection());

	CursorMan.showMouse(false);
	_vm->_graphics->drawPicture(_vm->_graphics->_surface, _directions[_vm->_animation->getDirection()], 0, 161);
	CursorMan.showMouse(true);
}


void Avalot::gameOver() {
	_userMovesAvvy = false;

	AnimationType *avvy = &_vm->_animation->_sprites[0];
	int16 sx = avvy->_x;
	int16 sy = avvy->_y;

	avvy->remove();
	avvy->init(12, true, _vm->_animation); // 12 = Avalot falls
	avvy->_stepNum = 0;
	avvy->appear(sx, sy, kDirUp);

	_vm->_timer->addTimer(3, Timer::kProcAvalotFalls, Timer::kReasonFallingOver);
	_alive = false;
}

void Avalot::minorRedraw() {
	dusk();

	enterRoom(_room, 0); // Ped unknown or non-existant.

	for (int i = 0; i < 3; i++)
		_scoreToDisplay[i] = -1; // impossible digits
	drawScore();

	dawn();
}

void Avalot::majorRedraw() {
	warning("STUB: Avalot::major_redraw()");
}

uint16 Avalot::bearing(byte whichPed) {
	static const double rad2deg = 180 / 3.14; // Pi
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	PedType *curPed = &_peds[whichPed];

	if (avvy->_x == curPed->_x)
		return 0;
	else if (avvy->_x < curPed->_x) {
		return (uint16)((atan(double((avvy->_y - curPed->_y)) / (avvy->_x - curPed->_x)) * rad2deg) + 90);
	} else {
		return (uint16)((atan(double((avvy->_y - curPed->_y)) / (avvy->_x - curPed->_x)) * rad2deg) + 270);
	}
}

/** 
 * @remarks	Originally called 'sprite_run'
 */
void Avalot::spriteRun() {
	_doingSpriteRun = true;
	_vm->_animation->animLink();
	_doingSpriteRun = false;
}

void Avalot::fixFlashers() {
	_ledStatus = 177;
	_vm->_animation->setOldDirection(kDirNone);
	_vm->_dialogs->setReadyLight(2);
	drawDirection();
}

Common::String Avalot::intToStr(int32 num) {
	return Common::String::format("%d", num);
}

void Avalot::newMouse(byte id) {
	if (id == _currentMouse)
		return;

	_currentMouse = id;
	loadMouse(id);
}

/**
 * Set the mouse pointer to 'HourGlass"
 * @remarks	Originally called 'wait'
 */
void Avalot::setMousePointerWait() {
	newMouse(4);
}

void Avalot::drawShadow(int16 x1, int16 y1, int16 x2, int16 y2, byte hc, byte sc) {
	warning("STUB: Avalot::shadow()");
}

void Avalot::drawShadowBox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t) {
	warning("STUB: Avalot::shbox()");
}

void Avalot::resetVariables() {
	_vm->_animation->setDirection(kDirUp);
	_carryNum = 0;
	for (int i = 0; i < kObjectNum; i++)
		_objects[i] = false;

	_dnascore = 0;
	_money = 0;
	_room = kRoomNowhere;
	_vm->_saveNum = 0;
	for (int i = 0; i < 100; i++)
		_roomCount[i] = 0;

	_wonNim = false;
	_wineState = 0;
	_cwytalotGone = false;
	_passwordNum = 0;
	_aylesIsAwake = false;
	_drawbridgeOpen = 0;
	_avariciusTalk = 0;
	_rottenOnion = false;
	_onionInVinegar = false;
	_givenToSpludwick = 0;
	_brummieStairs = 0;
	_cardiffQuestionNum = 0;
	_passedCwytalotInHerts = false;
	_avvyIsAwake = false;
	_avvyInBed = false;
	_userMovesAvvy = false;
	_npcFacing = 0;
	_givenBadgeToIby = false;
	_friarWillTieYouUp = false;
	_tiedUp = false;
	_boxContent = 0;
	_talkedToCrapulus = false;
	_jacquesState = 0;
	_bellsAreRinging = false;
	_standingOnDais = false;
	_takenPen = false;
	_arrowTriggered = false;
	_arrowInTheDoor = false;
	_favouriteDrink = "";
	_favouriteSong = "";
	_worstPlaceOnEarth = "";
	_spareEvening = "";
	_totalTime = 0;
	_jumpStatus = 0;
	_mushroomGrowing = false;
	_spludwickAtHome = false;
	_lastRoom = 0;
	_lastRoomNotMap = 0;
	_crapulusWillTell = false;
	_enterCatacombsFromLustiesRoom = false;
	_teetotal = false;
	_malagauche = 0;
	_drinking = 0;
	_enteredLustiesRoomAsMonk = false;
	_catacombX = 0;
	_catacombY = 0;
	_avvysInTheCupboard = false;
	_geidaFollows = false;
	_geidaSpin = 0;
	_geidaTime = 0;
	_nextBell = 0;
	_givenPotionToGeida = false;
	_lustieIsAsleep = false;
	_flipToWhere = kRoomNowhere;
	_flipToPed = 0;
	_beenTiedUp = false;
	_sittingInPub = false;
	_spurgeTalkCount = 0;
	_metAvaroid = false;
	_takenMushroom = false;
	_givenPenToAyles = false;
	_askedDogfoodAboutNim = false;

	_vm->_parser->resetVariables();
}

void Avalot::newGame() {
	for (int i = 0; i < kMaxSprites; i++) {
		AnimationType *spr = &_vm->_animation->_sprites[i];
		if (spr->_quick)
			spr->remove();
	}
	// Deallocate sprite. Sorry, beta testers!

	AnimationType *avvy = &_vm->_animation->_sprites[0];
	avvy->init(0, true, _vm->_animation);

	_alive = true;
	resetVariables();

	_vm->_dialogs->setBubbleStateNatural();

	_spareEvening = "answer a questionnaire";
	_favouriteDrink = "beer";
	_money = 30; // 2/6
	_vm->_animation->setDirection(kDirStopped);
	_vm->_parser->_wearing = kObjectClothes;
	_objects[kObjectMoney - 1] = true;
	_objects[kObjectBodkin - 1] = true;
	_objects[kObjectBell - 1] = true;
	_objects[kObjectClothes - 1] = true;

	_thinkThing = true;
	_thinks = 2;
	refreshObjectList();
	_onToolbar = false;
	_seeScroll = false;

	avvy->appear(300, 117, kDirRight); // Needed to initialize Avalot.
	//for (gd = 0; gd <= 30; gd++) for (gm = 0; gm <= 1; gm++) also[gd][gm] = nil;
	// fillchar(previous^,sizeof(previous^),#0); { blank out array }
	_him = kPeoplePardon;
	_her = kPeoplePardon;
	_it = Parser::kPardon;
	_lastPerson = kPeoplePardon; // = Pardon?
	_passwordNum = _vm->_rnd->getRandomNumber(30) + 1; //Random(30) + 1;
	_userMovesAvvy = false;
	_doingSpriteRun = false;
	_avvyInBed = true;
	_enidFilename = "";

	enterRoom(kRoomYours, 1);
	avvy->_visible = false;
	drawScore();
	_vm->_menu->setup();
	_clock.update();
	spriteRun();
}

void Avalot::slowDown() {
	warning("STUB: Avalot::slowdown()");
}

bool Avalot::setFlag(char x) {
	for (uint16 i = 0; i < _flags.size(); i++) {
		if (_flags[i] == x)
			return true;
	}

	return false;
}

bool Avalot::decreaseMoney(uint16 howmuchby) {
	_money -= howmuchby;
	if (_money < 0) {
		_vm->_dialogs->displayScrollChain('Q', 2); // "You are now denariusless!"
		gameOver();
		return false;
	} else
		return true;
}

Common::String Avalot::getName(People whose) {
	static const Common::String kLads[17] = {
		"Avalot", "Spludwick", "Crapulus", "Dr. Duck", "Malagauche", "Friar Tuck",
		"Robin Hood", "Cwytalot", "du Lustie", "the Duke of Cardiff", "Dogfood",
		"A trader", "Ibythneth", "Ayles", "Port", "Spurge", "Jacques"
	};

	static const Common::String kLasses[4] = {"Arkata", "Geida", "\0xB1", "the Wise Woman"};

	if (whose < kPeopleArkata)
		return kLads[whose - kPeopleAvalot];
	else
		return kLasses[whose - kPeopleArkata];
}

byte Avalot::getNameChar(People whose) {
	static const char kLadChar[] = "ASCDMTRwLfgeIyPu";
	static const char kLassChar[] = "kG\0xB1o";

	if (whose < kPeopleArkata)
		return kLadChar[whose - kPeopleAvalot];
	else
		return kLassChar[whose - kPeopleArkata];
}

Common::String Avalot::getThing(byte which) {
	static const Common::String kThings[kObjectNum] = {
		"Wine", "Money-bag", "Bodkin", "Potion", "Chastity belt",
		"Crossbow bolt", "Crossbow", "Lute", "Pilgrim's badge", "Mushroom", "Key",
		"Bell", "Scroll", "Pen", "Ink", "Clothes", "Habit", "Onion"
	};

	Common::String get_thing_result;
	switch (which) {
	case kObjectWine:
		switch (_wineState) {
		case 1:
		case 4:
			get_thing_result = kThings[which - 1];
			break;
		case 3:
			get_thing_result = "Vinegar";
			break;
		}
		break;
	case kObjectOnion:
		if (_rottenOnion)
			get_thing_result = "rotten onion";
		else
			get_thing_result = kThings[which - 1];
		break;
	default:
		get_thing_result = kThings[which - 1];
	}
	return get_thing_result;
}

char Avalot::getThingChar(byte which) {
	static const char kThingsChar[] = "WMBParCLguKeSnIohn"; // V=Vinegar

	char get_thingchar_result;
	switch (which) {
	case kObjectWine:
		if (_wineState == 3)
			get_thingchar_result = 'V'; // Vinegar
		else
			get_thingchar_result = kThingsChar[which - 1];
		break;
	default:
		get_thingchar_result = kThingsChar[which - 1];
	}
	return get_thingchar_result;
}

Common::String Avalot::getItem(byte which) {
	static const Common::String kItems[kObjectNum] = {
		"some wine", "your money-bag", "your bodkin", "a potion", "a chastity belt",
		"a crossbow bolt", "a crossbow", "a lute", "a pilgrim's badge", "a mushroom",
		"a key", "a bell", "a scroll", "a pen", "some ink", "your clothes", "a habit",
		"an onion"
	};

	Common::String get_better_result;
	if (which > 150)
		which -= 149;

	switch (which) {
	case kObjectWine:
		switch (_wineState) {
		case 0:
		case 1:
		case 4:
			get_better_result = kItems[which - 1];
			break;
		case 3:
			get_better_result = "some vinegar";
			break;
		}
		break;
	case kObjectOnion:
		if (_rottenOnion)
			get_better_result = "a rotten onion";
		else if (_onionInVinegar)
			get_better_result = "a pickled onion (in the vinegar)";
		else
			get_better_result = kItems[which - 1];
		break;
	default:
		if ((which < kObjectNum) && (which > 0))
			get_better_result = kItems[which - 1];
		else
			get_better_result = "";
	}
	return get_better_result;
}


Common::String Avalot::f5Does() {
	switch (_room) {
	case kRoomYours:
		if (!_avvyIsAwake)
			return Common::String::format("%cWWake up", kVerbCodeWake);
		else if (_avvyInBed)
			return Common::String::format("%cGGet up", kVerbCodeStand);
		break;
	case kRoomInsideCardiffCastle:
		if (_standingOnDais)
			return Common::String::format("%cCClimb down", kVerbCodeClimb);
		else
			return Common::String::format("%cCClimb up", kVerbCodeClimb);
		break;
	case kRoomNottsPub:
		if (_sittingInPub)
			return Common::String::format("%cSStand up", kVerbCodeStand);
		else
			return Common::String::format("%cSSit down", kVerbCodeSit);
		break;
	case kRoomMusicRoom:
		if (_vm->_animation->inField(5))
			return Common::String::format("%cPPlay the harp", kVerbCodePlay);
		break;
	}

	return Common::String::format("%c", kVerbCodePardon); // If all else fails...
}

void Avalot::loadMouse(byte which) {
	Common::File f;

	if (!f.open("mice.avd"))
		error("AVALANCHE: Gyro: File not found: mice.avd");

	::Graphics::Surface cursor;
	cursor.create(16, 32, ::Graphics::PixelFormat::createFormatCLUT8());
	cursor.fillRect(Common::Rect(0, 0, 16, 32), 255);


	// The AND mask.
	f.seek(kMouseSize * 2 * which + 134);

	::Graphics::Surface mask = _vm->_graphics->loadPictureGraphic(f);

	for (int j = 0; j < mask.h; j++) {
		for (int i = 0; i < mask.w; i++) {
			byte pixel = *(byte *)mask.getBasePtr(i, j);
			if (pixel == 0) {
				*(byte *)cursor.getBasePtr(i, j * 2    ) = 0;
				*(byte *)cursor.getBasePtr(i, j * 2 + 1) = 0;
			}
		}
	}

	mask.free();

	// The OR mask.
	f.seek(kMouseSize * 2 * which + 134 * 2);

	mask = _vm->_graphics->loadPictureGraphic(f);

	for (int j = 0; j < mask.h; j++) {
		for (int i = 0; i < mask.w; i++) {
			byte pixel = *(byte *)mask.getBasePtr(i, j);
			if (pixel != 0) {
				*(byte *)cursor.getBasePtr(i, j * 2    ) = pixel;
				*(byte *)cursor.getBasePtr(i, j * 2 + 1) = pixel;
			}
		}
	}

	mask.free();
	f.close();

	CursorMan.replaceCursor(cursor.getPixels(), 16, 32, kMouseHotSpots[which]._horizontal, kMouseHotSpots[which]._vertical * 2, 255, false);
	cursor.free();
}

void Avalot::setBackgroundColor(byte x) {
	warning("STUB: Avalot::background()");
}

void Avalot::hangAroundForAWhile() {
	for (int i = 0; i < 28; i++)
		slowDown();
}

void Avalot::flipRoom(Room room, byte ped) {
	assert((ped > 0) && (ped < 15));
	if (!_alive) {
		// You can't leave the room if you're dead.
		_vm->_animation->_sprites[0]._moveX = 0;
		_vm->_animation->_sprites[0]._moveY = 0; // Stop him from moving.
		return;
	}

	if ((room == kRoomDummy) && (_room == kRoomLusties)) {
		_vm->_animation->hideInCupboard();
		return;
	}

	if ((_jumpStatus > 0) && (_room == kRoomInsideCardiffCastle)) {
		// You can't *jump* out of Cardiff Castle!
		_vm->_animation->_sprites[0]._moveX = 0;
		return;
	}

	exitRoom(_room);
	dusk();

	for (int16 i = 1; i < _vm->_animation->kSpriteNumbMax; i++) {
		if (_vm->_animation->_sprites[i]._quick)
			_vm->_animation->_sprites[i].remove();
	} // Deallocate sprite

	if (_room == kRoomLustiesRoom)
		_enterCatacombsFromLustiesRoom = true;

	enterRoom(room, ped);
	_vm->_animation->appearPed(0, ped - 1);
	_enterCatacombsFromLustiesRoom = false;
	_vm->_animation->setOldDirection(_vm->_animation->getDirection());
	_vm->_animation->setDirection(_vm->_animation->_sprites[0]._facingDir);
	drawDirection();

	dawn();
}

/**
 * Open the Door.
 * This slides the door open. The data really ought to be saved in
 * the Also file, and will be next time. However, for now, they're
 * here.
 * @remarks	Originally called 'open_the_door'
 */
void Avalot::openDoor(Room whither, byte ped, byte magicnum) {
	switch (_room) {
	case kRoomOutsideYours:
	case kRoomOutsideNottsPub:
	case kRoomOutsideDucks:
		_vm->_sequence->firstShow(1);
		_vm->_sequence->thenShow(2);
		_vm->_sequence->thenShow(3);
		break;
	case kRoomInsideCardiffCastle:
		_vm->_sequence->firstShow(1);
		_vm->_sequence->thenShow(5);
		break;
	case kRoomAvvysGarden:
	case kRoomEntranceHall:
	case kRoomInsideAbbey:
	case kRoomYourHall:
		_vm->_sequence->firstShow(1);
		_vm->_sequence->thenShow(2);
		break;
	case kRoomMusicRoom:
	case kRoomOutsideArgentPub:
		_vm->_sequence->firstShow(5);
		_vm->_sequence->thenShow(6);
		break;
	case kRoomLusties:
		switch (magicnum) {
		case 14:
			if (_avvysInTheCupboard) {
				_vm->_animation->hideInCupboard();
				_vm->_sequence->firstShow(8);
				_vm->_sequence->thenShow(7);
				_vm->_sequence->startToClose();
				return;
			} else {
				_vm->_animation->appearPed(0, 5);
				_vm->_animation->_sprites[0]._facingDir = kDirRight; // added by TT 12/3/1995
				_vm->_sequence->firstShow(8);
				_vm->_sequence->thenShow(9);
			}
			break;
		case 12:
			_vm->_sequence->firstShow(4);
			_vm->_sequence->thenShow(5);
			_vm->_sequence->thenShow(6);
			break;
		}
		break;
	}

	_vm->_sequence->thenFlip(whither, ped);
	_vm->_sequence->startToOpen();
}

void Avalot::setRoom(People persId, Room roomId) {
	_whereIs[persId - kPeopleAvalot] = roomId;	
}

Room Avalot::getRoom(People persId) {
	return _whereIs[persId - kPeopleAvalot];	
}
} // End of namespace Avalanche
