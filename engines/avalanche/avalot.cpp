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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

/* AVALOT		The kernel of the program. */

#include "avalanche/avalanche.h"

#include "common/math.h"
#include "common/random.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "graphics/palette.h"

namespace Avalanche {

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

const char AvalancheEngine::kSpludwicksOrder[3] = {kObjectOnion, kObjectInk, kObjectMushroom};
const uint16 AvalancheEngine::kNotes[12] = {196, 220, 247, 262, 294, 330, 350, 392, 440, 494, 523, 587};

Room AvalancheEngine::_whereIs[29] = {
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



void AvalancheEngine::handleKeyDown(Common::Event &event) {
	_sound->click();

	if ((Common::KEYCODE_F1 <= event.kbd.keycode) && (event.kbd.keycode <= Common::KEYCODE_F15))
		_parser->handleFunctionKey(event);
	else if ((32 <= event.kbd.ascii) && (event.kbd.ascii <= 128) && (event.kbd.ascii != 47))
		_parser->handleInputText(event);
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
			_animation->handleMoveKey(event);
			drawDirection();
			return;
		}
		// fall through
	case Common::KEYCODE_BACKSPACE:
		_parser->handleBackspace();
		break;
	case Common::KEYCODE_RETURN:
		_parser->handleReturn();
		break;
	default:
		break;
	}

	drawDirection();
}

void AvalancheEngine::setup() {
	init();

	_dialogs->reset();
	fadeOut();
	_graphics->loadDigits();

	_parser->_inputTextPos = 0;
	_parser->_quote = true;

	_animation->resetAnims();

	_dialogs->setReadyLight(2);

	fadeIn();
	_parser->_cursorState = false;
	_parser->cursorOn();
	_animation->_sprites[0]->_speedX = kWalk;
	_animation->updateSpeed();

	_dropdown->init();

	_graphics->drawSoundLight(_sound->_soundFl);

	drawToolbar();

	int16 loadSlot = ConfMan.instance().getInt("save_slot");
	if (loadSlot >= 0) {
		_thinks = 2; // You always have money.
		thinkAbout(kObjectMoney, kThing);

		loadGame(loadSlot);
	} else {
		// We don't need the MainMenu during the whole game, only at the beginning of it.
		MainMenu *mainmenu = new MainMenu(this);
		mainmenu->run();
		delete mainmenu;
		if (_letMeOut)
			return;

		newGame();

		thinkAbout(kObjectMoney, kThing);

		_dialogs->displayScrollChain('Q', 83); // Info on the game, etc.
	}
}

void AvalancheEngine::runAvalot() {
	setup();

	while (!_letMeOut && !shouldQuit()) {
		uint32 beginLoop = _system->getMillis();

		updateEvents(); // The event handler.

		_clock->update();
		_dropdown->update();
		_background->update();
		_animation->animLink();
		checkClick();
		_timer->updateTimer();

		_graphics->drawDebugLines();
		_graphics->refreshScreen();

		uint32 delay = _system->getMillis() - beginLoop;
		if (delay <= 55)
			_system->delayMillis(55 - delay); // Replaces slowdown(); 55 comes from 18.2 Hz (B Flight).
	};

	_closing->exitGame();
}

void AvalancheEngine::init() {
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++)
			_also[i][j] = nullptr;
	}

	_letMeOut = false;
	_currentMouse = 177;
	_dropsOk = true;
	_mouseText = "";
	_cheat = false;
	_cp = 0;
	_ledStatus = 177;
	for (int i = 0; i < 3; i++)
		_scoreToDisplay[i] = -1; // Impossible digits.
	_holdTheDawn = false;

	_graphics->loadMouse(kCurWait);
	CursorMan.showMouse(true);
}

/**
 * Call a given Verb
 * @remarks	Originally called 'callverb'
 */
void AvalancheEngine::callVerb(VerbCode id) {
	if (id == _parser->kPardon) {
		Common::String tmpStr = Common::String::format("The f5 key lets you do a particular action in certain " \
			"situations. However, at the moment there is nothing assigned to it. You may press alt-A to see " \
			"what the current setting of this key is.");
		_dialogs->displayText(tmpStr);
	} else
		_parser->doVerb(id);
}

/**
 * Check is it's possible to give something to Spludwick
 * @remarks	Originally called 'nextstring'
 */
Common::String AvalancheEngine::readAlsoStringFromFile(Common::File &file) {
	Common::String str;
	byte length = file.readByte();
	for (int i = 0; i < length; i++)
		str += file.readByte();
	return str;
}

void AvalancheEngine::scram(Common::String &str) {
	for (uint i = 0; i < str.size(); i++)
		str.setChar(str[i] ^ 177, i);
}

void AvalancheEngine::unScramble() {
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++) {
			if (_also[i][j] != nullptr)
				scram(*_also[i][j]);
		}
	}
	scram(_listen);
	scram(_flags);
}

void AvalancheEngine::loadAlso(byte num) {
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
	Common::File file;
	if (!file.open(filename))
		error("AVALANCHE: File not found: %s", filename.c_str());

	file.seek(128);

	byte alsoNum = file.readByte();
	Common::String tmpStr;
	for (int i = 0; i <= alsoNum; i++) {
		for (int j = 0; j < 2; j++) {
			_also[i][j] = new Common::String;
			*_also[i][j] = readAlsoStringFromFile(file);
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
		curLine->_color = (Color)file.readByte();
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

	int16 size = file.readByte();
	_listen.clear();
	for (int i = 0; i < size; i++)
		_listen += file.readByte();

	_graphics->clearAlso();

	CursorMan.showMouse(false);
	for (int i = 0; i < _lineNum; i++) {
		// We had to check if the lines are within the borders of the screen.
		if ((_lines[i]._x1 >= 0) && (_lines[i]._x1 < kScreenWidth) && (_lines[i]._y1 >= 0) && (_lines[i]._y1 < kScreenHeight)
			&& (_lines[i]._x2 >= 0) && (_lines[i]._x2 < kScreenWidth) && (_lines[i]._y2 >= 0) && (_lines[i]._y2 < kScreenHeight))
			_graphics->setAlsoLine(_lines[i]._x1, _lines[i]._y1, _lines[i]._x2, _lines[i]._y2, _lines[i]._color);
	}
	CursorMan.showMouse(true);

	file.close();

	unScramble();
	for (int i = 0; i <= alsoNum; i++) {
		tmpStr = Common::String::format(",%s,", _also[i][0]->c_str());
		*_also[i][0] = tmpStr;
	}
}

void AvalancheEngine::loadBackground(byte num) {
	Common::String filename = Common::String::format("place%d.avd", num);
	Common::File file;
	if (!file.open(filename))
		error("AVALANCHE: File not found: %s", filename.c_str());

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

	_graphics->loadBackground(file);
	_graphics->refreshBackground();

	file.close();
}

void AvalancheEngine::loadRoom(byte num) {
	CursorMan.showMouse(false);

	loadBackground(num);
	loadAlso(num);
	_background->loadSprites(num);

	CursorMan.showMouse(true);
}

void AvalancheEngine::findPeople(byte room) {
	for (int i = 1; i < 29; i++) {
		if (_whereIs[i] == room) {
			if (i < 25)
				_him = (People)(150 + i);
			else
				_her = (People)(150 + i);
		}
	}
}

void AvalancheEngine::exitRoom(byte x) {
	_sound->stopSound();
	_background->release();
	_animationsEnabled = false;

	switch (x) {
	case kRoomSpludwicks:
		_timer->loseTimer(Timer::kReasonAvariciusTalks);
		 _avariciusTalk = 0;
		// He doesn't HAVE to be talking for this to work. It just deletes it IF it exists.
		break;
	case kRoomBridge:
		if (_drawbridgeOpen > 0) {
			_drawbridgeOpen = 4; // Fully open.
			_timer->loseTimer(Timer::kReasonDrawbridgeFalls);
		}
		break;
	case kRoomOutsideCardiffCastle:
		_timer->loseTimer(Timer::kReasonCardiffsurvey);
		break;
	case kRoomRobins:
		_timer->loseTimer(Timer::kReasonGettingTiedUp);
		break;
	default:
		break;
	}

	_interrogation = 0; // Leaving the room cancels all the questions automatically.
	_animationsEnabled = true;

	_lastRoom = _room;
	if (_room != kRoomMap)
		_lastRoomNotMap = _room;
}

/**
 * Only when entering a NEW town! Not returning to the last one,
 * but choosing another from the map.
 * @remarks	Originally called 'new_town'
 */
void AvalancheEngine::enterNewTown() {
	_dropdown->setup();

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
	default:
		break;
	}

	if ((_room != kRoomOutsideDucks) && (_objects[kObjectOnion - 1]) && !(_onionInVinegar))
		_rottenOnion = true; // You're holding the onion
}

void AvalancheEngine::putGeidaAt(byte whichPed, byte ped) {
	if (ped == 0)
		return;
	AnimationType *spr1 = _animation->_sprites[1];

	spr1->init(5, false); // load Geida
	_animation->appearPed(1, whichPed);
	spr1->_callEachStepFl = true;
	spr1->_eachStepProc = Animation::kProcFollowAvvy;
}

void AvalancheEngine::enterRoom(Room roomId, byte ped) {
	_animationsEnabled = false;

	findPeople(roomId);
	_room = roomId;
	if (ped != 0)
		_roomCount[roomId]++;

	loadRoom(roomId);

	if ((_roomCount[roomId] == 0) && (!getFlag('S')))
		incScore(1);

	_whereIs[kPeopleAvalot - 150] = _room;

	if (_geidaFollows)
		_whereIs[kPeopleGeida - 150] = roomId;

	_roomCycles = 0;

	if ((_lastRoom == kRoomMap) && (_lastRoomNotMap != _room))
		enterNewTown();

	_animation->updateSpeed();

	switch (roomId) {
	case kRoomYours:
		if (_avvyInBed) {
			_background->draw(-1, -1, 2);
			_graphics->refreshBackground();
			_timer->addTimer(100, Timer::kProcArkataShouts, Timer::kReasonArkataShouts);
		}
		break;

	case kRoomOutsideYours:
		if (ped > 0) {
			AnimationType *spr1 = _animation->_sprites[1];
			if (!_talkedToCrapulus) {
				_whereIs[kPeopleCrapulus - 150] = kRoomOutsideYours;
				spr1->init(8, false); // load Crapulus

				if (_roomCount[kRoomOutsideYours] == 1) {
					_animation->appearPed(1, 3); // Start on the right-hand side of the screen.
					spr1->walkTo(4); // Walks up to greet you.
				} else {
					_animation->appearPed(1, 4); // Starts where he was before.
					spr1->_facingDir = kDirLeft;
				}

				spr1->_callEachStepFl = true;
				spr1->_eachStepProc = Animation::kProcFaceAvvy; // He always faces Avvy.

			} else
				_whereIs[kPeopleCrapulus - 150] = kRoomNowhere;

			if (_crapulusWillTell) {
				spr1->init(8, false);
				_animation->appearPed(1, 1);
				spr1->walkTo(3);
				_timer->addTimer(20, Timer::kProcCrapulusSpludOut, Timer::kReasonCrapulusSaysSpludwickOut);
				_crapulusWillTell = false;
			}
		}
		break;

	case kRoomOutsideSpludwicks:
		if ((_roomCount[kRoomOutsideSpludwicks] == 1) && (ped == 1)) {
			_timer->addTimer(20, Timer::kProcBang, Timer::kReasonExplosion);
			_spludwickAtHome = true;
		}
		break;

	case kRoomSpludwicks:
		if (_spludwickAtHome) {
			AnimationType *spr1 = _animation->_sprites[1];
			if (ped > 0) {
				spr1->init(2, false); // load Spludwick
				_animation->appearPed(1, 1);
				_whereIs[kPeopleSpludwick - 150] = kRoomSpludwicks;
			}

			spr1->_callEachStepFl = true;
			spr1->_eachStepProc = Animation::kProcFollowAvvy;
		} else
			_whereIs[kPeopleSpludwick - 150] = kRoomNowhere;
		break;

	case kRoomBrummieRoad:
		if (_geidaFollows)
			putGeidaAt(4, ped);
		if (_cwytalotGone) {
			_magics[kColorLightred - 1]._operation = kMagicNothing;
			_whereIs[kPeopleCwytalot - 150] = kRoomNowhere;
		} else if (ped > 0) {
			AnimationType *spr1 = _animation->_sprites[1];
			spr1->init(4, false); // 4 = Cwytalot
			spr1->_callEachStepFl = true;
			spr1->_eachStepProc = Animation::kProcFollowAvvyY;
			_whereIs[kPeopleCwytalot - 150] = kRoomBrummieRoad;

			if (_roomCount[kRoomBrummieRoad] == 1) { // First time here...
				_animation->appearPed(1, 1); // He appears on the right of the screen...
				spr1->walkTo(3); // ...and he walks up...
			} else {
				// You've been here before.
				_animation->appearPed(1, 3); // He's standing in your way straight away...
				spr1->_facingDir = kDirLeft;
			}
		}
		break;

	case kRoomArgentRoad:
		if ((_cwytalotGone) && (!_passedCwytalotInHerts) && (ped == 2) && (_roomCount[kRoomArgentRoad] > 3)) {
			AnimationType *spr1 = _animation->_sprites[1];
			spr1->init(4, false); // 4 = Cwytalot again
			_animation->appearPed(1, 0);
			spr1->walkTo(1);
			spr1->_vanishIfStill = true;
			_passedCwytalotInHerts = true;
			// whereis[#157] = r__Nowhere; // can we fit this in?
			_timer->addTimer(20, Timer::kProcCwytalotInHerts, Timer::kReasonCwytalotInHerts);
		}
		break;

	case kRoomBridge:
		if (_drawbridgeOpen == 4) { // open
			_background->draw(-1, -1, 2); // Position of drawbridge
			_graphics->refreshBackground();
			_magics[kColorGreen - 1]._operation = kMagicNothing; // You may enter the drawbridge.
		}
		if (_geidaFollows)
			putGeidaAt(ped + 2, ped); // load Geida
		break;

	case kRoomRobins:
		if ((ped > 0) && (!_beenTiedUp)) {
			// A welcome party... or maybe not...
			AnimationType *spr1 = _animation->_sprites[1];
			spr1->init(6, false);
			_animation->appearPed(1, 1);
			spr1->walkTo(2);
			_timer->addTimer(36, Timer::kProcGetTiedUp, Timer::kReasonGettingTiedUp);
		}

		if (_beenTiedUp) {
			_whereIs[kPeopleRobinHood - 150] = kRoomNowhere;
			_whereIs[kPeopleFriarTuck - 150] = kRoomNowhere;
		}

		if (_tiedUp)
			_background->draw(-1, -1, 1);

		if (!_mushroomGrowing)
			_background->draw(-1, -1, 2);
		_graphics->refreshBackground();
		break;

	case kRoomOutsideCardiffCastle:
		if (ped > 0) {
			AnimationType *spr1 = _animation->_sprites[1];
			switch (_cardiffQuestionNum) {
			case 0 : // You've answered NONE of his questions.
				spr1->init(9, false);
				_animation->appearPed(1, 1);
				spr1->walkTo(2);
				_timer->addTimer(47, Timer::kProcCardiffSurvey, Timer::kReasonCardiffsurvey);
				break;
			case 5 :
				_magics[1]._operation = kMagicNothing;
				break; // You've answered ALL his questions. => nothing happens.
			default: // You've answered SOME of his questions.
				spr1->init(9, false);
				_animation->appearPed(1, 2);
				spr1->_facingDir = kDirRight;
				_timer->addTimer(3, Timer::kProcCardiffReturn, Timer::kReasonCardiffsurvey);
			}
		}

		if (_cardiffQuestionNum < 5)
			_interrogation = _cardiffQuestionNum;
		else
			_interrogation = 0;
		break;

	case kRoomMap:
		// You're entering the map.
		fadeIn();
		if (ped > 0)
			_graphics->zoomOut(_peds[ped - 1]._x, _peds[ped - 1]._y);

		if ((_objects[kObjectWine - 1]) && (_wineState != 3)) {
			_dialogs->displayScrollChain('Q', 9); // Don't want to waste the wine!
			_objects[kObjectWine - 1] = false;
			refreshObjectList();
		}

		_dialogs->displayScrollChain('Q', 69);
		break;

	case kRoomCatacombs:
		if ((ped == 0) || (ped == 3) || (ped == 5) || (ped == 6)) {
			switch (ped) {
			case 0:
			default:
				break;
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
			_animation->catacombMove(ped);
			_enterCatacombsFromLustiesRoom = false;
		}
		break;

	case kRoomArgentPub:
		if (_wonNim)
			_background->draw(-1, -1, 0);   // No lute by the settle.
		_malagauche = 0; // Ready to boot Malagauche
		if (_givenBadgeToIby) {
			_background->draw(-1, -1, 7);
			_background->draw(-1, -1, 8);
		}
		_graphics->refreshBackground();
		break;

	case kRoomLustiesRoom:
		_npcFacing = 1; // du Lustie.
		if (_animation->getAvvyClothes() == 0) // Avvy in his normal clothes
			_timer->addTimer(3, Timer::kProcCallsGuards, Timer::kReasonDuLustieTalks);
		else if (!_enteredLustiesRoomAsMonk) // already
			// Presumably, Avvy dressed as a monk.
			_timer->addTimer(3, Timer::kProcGreetsMonk, Timer::kReasonDuLustieTalks);

		if (_geidaFollows) {
			putGeidaAt(4, ped);
			if (_lustieIsAsleep) {
				_background->draw(-1, -1, 4);
				_graphics->refreshBackground();
			}
		}
		break;

	case kRoomMusicRoom:
		if (_jacquesState > 0) {
			_jacquesState = 5;
			_background->draw(-1, -1, 1);
			_graphics->refreshBackground();
			_background->draw(-1, -1, 3);
			_magics[kColorBrown - 1]._operation = kMagicNothing;
			_whereIs[kPeopleJacques - 150] = kRoomNowhere;
		}
		if (ped != 0) {
			_background->draw(-1, -1, 5);
			_graphics->refreshBackground();
			_sequence->startMusicRoomSeq();
		}
		break;

	case kRoomOutsideNottsPub:
		if (ped == 2) {
			_background->draw(-1, -1, 2);
			_graphics->refreshBackground();
			_sequence->startDuckSeq();
		}
		break;

	case kRoomOutsideArgentPub:
		if (ped == 2)  {
			_background->draw(-1, -1, 5);
			_graphics->refreshBackground();
			_sequence->startMusicRoomSeq();
		}
		break;

	case kRoomWiseWomans: {
		AnimationType *spr1 = _animation->_sprites[1];
		spr1->init(11, false);
		if ((_roomCount[kRoomWiseWomans] == 1) && (ped > 0)) {
			_animation->appearPed(1, 1); // Start on the right-hand side of the screen.
			spr1->walkTo(3); // Walks up to greet you.
		} else {
			_animation->appearPed(1, 3); // Starts where she was before.
			spr1->_facingDir = kDirLeft;
		}

		spr1->_callEachStepFl = true;
		spr1->_eachStepProc = Animation::kProcFaceAvvy; // She always faces Avvy.
		}
		break;

	case kRoomInsideCardiffCastle:
		if (ped > 0) {
			_animation->_sprites[1]->init(10, false); // Define the dart.
			_background->draw(-1, -1, 0);
			_graphics->refreshBackground();
			_sequence->startCardiffSeq2();
		} else {
			_background->draw(-1, -1, 0);
			if (_arrowInTheDoor)
				_background->draw(-1, -1, 2);
			else
				_background->draw(-1, -1, 1);
			_graphics->refreshBackground();
		}
		break;

	case kRoomAvvysGarden:
		if (ped == 1)  {
			_background->draw(-1, -1, 1);
			_graphics->refreshBackground();
			_sequence->startGardenSeq();
		}
		break;

	case kRoomEntranceHall:
	case kRoomInsideAbbey:
	case kRoomYourHall:
		if (ped == 2)  {
#if 0
			// It was the original:
			_celer->show_one(-1, -1, 2);
			_sequence->first_show(1);
			_sequence->then_show(3);
			_sequence->start_to_close();
#endif

			_background->draw(-1, -1, 1);
			_graphics->refreshBackground();
			_sequence->startGardenSeq();
		}
		break;

	case kRoomAylesOffice:
		if (_aylesIsAwake)
			_background->draw(-1, -1, 1);
		_graphics->refreshBackground();
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
			_background->draw(-1, -1, 2);
		_npcFacing = 1; // Port.
		break;

	case kRoomOutsideDucks:
		if (ped == 2) {
			// Shut the door
			_background->draw(-1, -1, 2);
			_graphics->refreshBackground();
			_sequence->startDuckSeq();
		}
		break;

	case kRoomDucks:
		_npcFacing = 1; // Duck.
		break;

	default:
		break;
	}

	_animationsEnabled = true;
}

void AvalancheEngine::thinkAbout(byte object, bool type) {
	_thinks = object;
	object--;

	Common::String filename;
	if (type == kThing) {
		filename = "thinks.avd";
	} else { // kPerson
		filename = "folk.avd";

		object -= 149;
		if (object >= 25)
			object -= 8;
		if (object == 20)
			object--; // Last time...
	}

	_graphics->loadMouse(kCurWait);
	CursorMan.showMouse(false);
	_graphics->drawThinkPic(filename, object);
	CursorMan.showMouse(true);

	_thinkThing = type;
}

void AvalancheEngine::drawToolbar() {
	_graphics->drawToolbar();
	_animation->setOldDirection(kDirNone);
	drawDirection();
}

void AvalancheEngine::drawScore() {
	uint16 score = _score;
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
			_graphics->drawDigit(numbers[i], 250 + (i + 1) * 15, 177);
	}

	CursorMan.showMouse(true);

	for (int i = 0; i < 3; i++)
		_scoreToDisplay[i] = numbers[i];
}

void AvalancheEngine::incScore(byte num) {
	for (int i = 1; i <= num; i++) {
		_score++;

		if (_soundFx) {
			for (int j = 1; j <= 97; j++)
				// Length of 2 is a guess, the original doesn't have a delay specified
				_sound->playNote(177 + _score * 3, 2);
		}
	}

	drawScore();
}

void AvalancheEngine::useCompass(const Common::Point &cursorPos) {
	byte color = _graphics->getScreenColor(cursorPos);

	switch (color) {
	case kColorGreen:
		_animation->setDirection(kDirUp);
		_animation->setMoveSpeed(0, kDirUp);
		drawDirection();
		break;
	case kColorBrown:
		_animation->setDirection(kDirDown);
		_animation->setMoveSpeed(0, kDirDown);
		drawDirection();
		break;
	case kColorCyan:
		_animation->setDirection(kDirLeft);
		_animation->setMoveSpeed(0, kDirLeft);
		drawDirection();
		break;
	case kColorLightmagenta:
		_animation->setDirection(kDirRight);
		_animation->setMoveSpeed(0, kDirRight);
		drawDirection();
		break;
	case kColorRed:
	case kColorWhite:
	case kColorLightcyan:
	case kColorYellow: // Fall-throughs are intended.
		_animation->stopWalking();
		drawDirection();
		break;
	default:
		break;
	}
}

void AvalancheEngine::fxToggle() {
	warning("STUB: fxtoggle()");
}

void AvalancheEngine::refreshObjectList() {
	_carryNum = 0;
	if (_thinkThing && !_objects[_thinks - 1])
		thinkAbout(kObjectMoney, kThing); // you always have money

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
void AvalancheEngine::guideAvvy(Common::Point cursorPos) {
	if (!_userMovesAvvy)
		return;

	cursorPos.y /= 2;
	byte what;

	// _animation->tr[0] is Avalot.)
	AnimationType *avvy = _animation->_sprites[0];
	if (cursorPos.x < avvy->_x)
		what = 1;
	else if (cursorPos.x > (avvy->_x + avvy->_xLength))
		what = 2;
	else
		what = 0; // On top

	if (cursorPos.y < avvy->_y)
		what += 3;
	else if (cursorPos.y > (avvy->_y + avvy->_yLength))
		what += 6;

	switch (what) {
	case 0:
	default:
		_animation->stopWalking();
		break; // Clicked on Avvy: no movement.
	case 1:
		_animation->setMoveSpeed(0, kDirLeft);
		break;
	case 2:
		_animation->setMoveSpeed(0, kDirRight);
		break;
	case 3:
		_animation->setMoveSpeed(0, kDirUp);
		break;
	case 4:
		_animation->setMoveSpeed(0, kDirUpLeft);
		break;
	case 5:
		_animation->setMoveSpeed(0, kDirUpRight);
		break;
	case 6:
		_animation->setMoveSpeed(0, kDirDown);
		break;
	case 7:
		_animation->setMoveSpeed(0, kDirDownLeft);
		break;
	case 8:
		_animation->setMoveSpeed(0, kDirDownRight);
		break;
	}    // No other values are possible.

	drawDirection();
}

void AvalancheEngine::checkClick() {
	Common::Point cursorPos = getMousePos();

	/*if (mrelease > 0)
		after_the_scroll = false;*/

	if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
		_graphics->loadMouse(kCurUpArrow); // up arrow
	else if ((317 <= cursorPos.y) && (cursorPos.y <= 339))
		_graphics->loadMouse(kCurIBeam); //I-beam
	else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
		_graphics->loadMouse(kCurScrewDriver); // screwdriver
	else if (!_dropdown->isActive()) { // Dropdown can handle its own pointers.
		if (_holdLeftMouse) {
			_graphics->loadMouse(kCurCrosshair); // Mark's crosshairs
			guideAvvy(cursorPos); // Normally, if you click on the picture, you're guiding Avvy around.
		} else
			_graphics->loadMouse(kCurFletch); // fletch
	}

	if (_holdLeftMouse) {
		if ((0 <= cursorPos.y) && (cursorPos.y <= 21)) { // Click on the dropdown menu.
			if (_dropsOk)
				_dropdown->update();
		} else if ((317 <= cursorPos.y) && (cursorPos.y <= 339)) { // Click on the command line.
			_parser->_inputTextPos = (cursorPos.x - 23) / 8;
			if (_parser->_inputTextPos > _parser->_inputText.size() + 1)
				_parser->_inputTextPos = _parser->_inputText.size() + 1;
			if (_parser->_inputTextPos < 1)
				_parser->_inputTextPos = 1;
			_parser->_inputTextPos--;
			_parser->plotText();
		} else if ((340 <= cursorPos.y) && (cursorPos.y <= 399)) { // Check the toolbar.
			if ((137 <= cursorPos.x) && (cursorPos.x <= 207)) { // Control Avvy with the compass.
				if (_alive && _avvyIsAwake)
					useCompass(cursorPos);
			} else if ((208 <= cursorPos.x) && (cursorPos.x <= 260)) { // Examine the _thing.
				do {
					updateEvents();
				} while (_holdLeftMouse);

				if (_thinkThing) {
					_parser->_thing = _thinks;
					_parser->_thing += 49;
					_parser->_person = kPeoplePardon;
				} else {
					_parser->_person = (People)_thinks;
					_parser->_thing = _parser->kPardon;
				}
				callVerb(kVerbCodeExam);
			} else if ((261 <= cursorPos.x) && (cursorPos.x <= 319)) { // Display the score.
				do {
					updateEvents();
				} while (_holdLeftMouse);

				callVerb(kVerbCodeScore);
			} else if ((320 <= cursorPos.x) && (cursorPos.x <= 357)) { // Change speed.
				_animation->_sprites[0]->_speedX = kWalk;
				_animation->updateSpeed();
			} else if ((358 <= cursorPos.x) && (cursorPos.x <= 395)) { // Change speed.
				_animation->_sprites[0]->_speedX = kRun;
				_animation->updateSpeed();
			} else if ((396 <= cursorPos.x) && (cursorPos.x <= 483))
				_sound->toggleSound();
			else if ((535 <= cursorPos.x) && (cursorPos.x <= 640))
				_mouseText.insertChar(kControlNewLine, 0);
		} else if (!_dropsOk)
			_mouseText = Common::String(13) + _mouseText;
	}
}

void AvalancheEngine::errorLed() {
	_dialogs->setReadyLight(0);
	_graphics->drawErrorLight(true);
	for (int i = 177; i >= 1; i--) {
		_sound->playNote(177 + (i * 177177) / 999, 1);
		_system->delayMillis(1);
	}
	_graphics->drawErrorLight(false);
	_dialogs->setReadyLight(2);
}

/**
 * Displays a fade out, full screen.
 * This version is different to the one in the original, which was fading in 3 steps.
 * @remarks	Originally called 'dusk'
 */
void AvalancheEngine::fadeOut() {
	byte pal[3], tmpPal[3];

	_graphics->setBackgroundColor(kColorBlack);
	if (_fxHidden)
		return;
	_fxHidden = true;

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			g_system->getPaletteManager()->grabPalette((byte *)tmpPal, j, 1);
			_fxPal[i][j][0] = tmpPal[0];
			_fxPal[i][j][1] = tmpPal[1];
			_fxPal[i][j][2] = tmpPal[2];
			if (tmpPal[0] >= 16)
				pal[0] = tmpPal[0] - 16;
			else
				pal[0] = 0;

			if (tmpPal[1] >= 16)
				pal[1] = tmpPal[1] - 16;
			else
				pal[1] = 0;

			if (tmpPal[2] >= 16)
				pal[2] = tmpPal[2] - 16;
			else
				pal[2] = 0;

			g_system->getPaletteManager()->setPalette(pal, j, 1);
		}
		_system->delayMillis(10);
		_graphics->refreshScreen();
	}
}

/**
 * Displays a fade in, full screen.
 * This version is different to the one in the original, which was fading in 3 steps.
 * @remarks	Originally called 'dawn'
 */
void AvalancheEngine::fadeIn() {
	if (_holdTheDawn || !_fxHidden)
		return;

	_fxHidden = false;

	byte pal[3];
	for (int i = 15; i >= 0; i--) {
		for (int j = 0; j < 16; j++) {
			pal[0] = _fxPal[i][j][0];
			pal[1] = _fxPal[i][j][1];
			pal[2] = _fxPal[i][j][2];
			g_system->getPaletteManager()->setPalette(pal, j, 1);
		}
		_system->delayMillis(10);
		_graphics->refreshScreen();
	}

	if ((_room == kRoomYours) && _avvyInBed && _teetotal)
		_graphics->setBackgroundColor(kColorYellow);
}

void AvalancheEngine::drawDirection() { // It's data is loaded in load_digits().
	if (_animation->getOldDirection() == _animation->getDirection())
		return;

	_animation->setOldDirection(_animation->getDirection());

	CursorMan.showMouse(false);
	_graphics->drawDirection(_animation->getDirection(), 0, 161);
	CursorMan.showMouse(true);
}

void AvalancheEngine::gameOver() {
	_userMovesAvvy = false;

	AnimationType *avvy = _animation->_sprites[0];
	int16 sx = avvy->_x;
	int16 sy = avvy->_y;

	avvy->remove();
	avvy->init(12, true); // 12 = Avalot falls
	avvy->_stepNum = 0;
	avvy->appear(sx, sy, kDirUp);

	_timer->addTimer(3, Timer::kProcAvalotFalls, Timer::kReasonFallingOver);
	_alive = false;
}

void AvalancheEngine::minorRedraw() {
	fadeOut();

	enterRoom(_room, 0); // Ped unknown or non-existant.

	for (int i = 0; i < 3; i++)
		_scoreToDisplay[i] = -1; // impossible digits
	drawScore();

	fadeIn();
}

uint16 AvalancheEngine::bearing(byte whichPed) {
	AnimationType *avvy = _animation->_sprites[0];
	PedType *curPed = &_peds[whichPed];

	if (avvy->_x == curPed->_x)
		return 0;

	int16 deltaX = avvy->_x - curPed->_x;
	int16 deltaY = avvy->_y - curPed->_y;
	uint16 result = Common::rad2deg<float,uint16>(atan((float)deltaY / (float)deltaX)); // TODO: Would atan2 be preferable?
	if (avvy->_x < curPed->_x) {
		return result + 90;
	} else {
		return result + 270;
	}
}

/**
 * @remarks	Originally called 'sprite_run'
 */
void AvalancheEngine::spriteRun() {
	_doingSpriteRun = true;
	_animation->animLink();
	_doingSpriteRun = false;
}

// CHECKME: Unused function
void AvalancheEngine::fixFlashers() {
	_ledStatus = 177;
	_animation->setOldDirection(kDirNone);
	_dialogs->setReadyLight(2);
	drawDirection();
}

Common::String AvalancheEngine::intToStr(int32 num) {
	return Common::String::format("%d", num);
}

void AvalancheEngine::resetVariables() {
	_carryNum = 0;
	for (int i = 0; i < kObjectNum; i++)
		_objects[i] = false;

	_score = 0;
	_money = 0;
	_room = kRoomNowhere;
	_saveNum = 0;
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
	_arrowInTheDoor = false;
	_favoriteDrink = "";
	_favoriteSong = "";
	_worstPlaceOnEarth = "";
	_spareEvening = "";
	_totalTime = 0;
	_jumpStatus = 0;
	_mushroomGrowing = false;
	_spludwickAtHome = false;
	_lastRoom = kRoomDummy;
	_lastRoomNotMap = kRoomDummy;
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
	_givenPotionToGeida = false;
	_lustieIsAsleep = false;
	_beenTiedUp = false;
	_sittingInPub = false;
	_spurgeTalkCount = 0;
	_metAvaroid = false;
	_takenMushroom = false;
	_givenPenToAyles = false;
	_askedDogfoodAboutNim = false;
	_startTime = getTimeInSeconds();
}

void AvalancheEngine::resetAllVariables() {
	resetVariables();
	_parser->resetVariables();
	_nim->resetVariables();
	_animation->resetVariables();
	_sequence->resetVariables();
	_background->resetVariables();
	_dropdown->resetVariables();
	_timer->resetVariables();
}

void AvalancheEngine::newGame() {
	for (int i = 0; i < kMaxSprites; i++) {
		AnimationType *spr = _animation->_sprites[i];
		if (spr->_quick)
			spr->remove();
	}
	// Deallocate sprite. Sorry, beta testers!

	AnimationType *avvy = _animation->_sprites[0];
	avvy->init(0, true);

	_alive = true;
	resetAllVariables();

	_dialogs->setBubbleStateNatural();

	_spareEvening = "answer a questionnaire";
	_favoriteDrink = "beer";
	_money = 30; // 2/6
	_animation->setDirection(kDirStopped);
	_parser->_wearing = kObjectClothes;
	_objects[kObjectMoney - 1] = true;
	_objects[kObjectBodkin - 1] = true;
	_objects[kObjectBell - 1] = true;
	_objects[kObjectClothes - 1] = true;

	_thinkThing = true;
	_thinks = 2;
	refreshObjectList();
	_animationsEnabled = true;

	avvy->appear(300, 117, kDirRight); // Needed to initialize Avalot.
	//for (gd = 0; gd <= 30; gd++) for (gm = 0; gm <= 1; gm++) also[gd][gm] = nil;
	// fillchar(previous^,sizeof(previous^),#0); { blank out array }
	_him = kPeoplePardon;
	_her = kPeoplePardon;
	_it = Parser::kPardon;
	_passwordNum = _rnd->getRandomNumber(29) + 1; //Random(30) + 1;
	_userMovesAvvy = false;
	_doingSpriteRun = false;
	_avvyInBed = true;

	_isLoaded = false;

	enterRoom(kRoomYours, 1);
	avvy->_visible = false;
	drawScore();
	_dropdown->setup();
	_clock->update();
	spriteRun();
}

bool AvalancheEngine::getFlag(char x) {
	for (uint16 i = 0; i < _flags.size(); i++) {
		if (_flags[i] == x)
			return true;
	}

	return false;
}

bool AvalancheEngine::decreaseMoney(uint16 amount) {
	_money -= amount;
	if (_money < 0) {
		_dialogs->displayScrollChain('Q', 2); // "You are now denariusless!"
		gameOver();
		return false;
	} else
		return true;
}

Common::String AvalancheEngine::getName(People whose) {
	static const char lads[17][20] = {
		"Avalot",     "Spludwick",  "Crapulus",  "Dr. Duck",  "Malagauche",
		"Friar Tuck", "Robin Hood", "Cwytalot",  "du Lustie", "the Duke of Cardiff",
		"Dogfood",    "A trader",   "Ibythneth", "Ayles",     "Port",
		"Spurge",     "Jacques"
	};

	static const char lasses[4][15] = {"Arkata", "Geida", "\0xB1", "the Wise Woman"};

	if (whose <= kPeopleJacques)
		return Common::String(lads[whose - kPeopleAvalot]);
	else if ((whose >= kPeopleArkata) && (whose <= kPeopleWisewoman))
		return Common::String(lasses[whose - kPeopleArkata]);
	else
		error("getName() - Unexpected character id %d", (byte) whose);
}

Common::String AvalancheEngine::getItem(byte which) {
	static const char items[kObjectNum][18] = {
		"some wine",       "your money-bag", "your bodkin", "a potion",          "a chastity belt",
		"a crossbow bolt", "a crossbow",     "a lute",      "a pilgrim's badge", "a mushroom",
		"a key",           "a bell",         "a scroll",    "a pen",             "some ink",
		"your clothes",    "a habit",        "an onion"
	};

	Common::String result;
	if (which > 150)
		which -= 149;

	switch (which) {
	case kObjectWine:
		switch (_wineState) {
		case 0:
		case 1:
		case 4:
			result = Common::String(items[which - 1]);
			break;
		case 3:
			result = "some vinegar";
			break;
		default:
			break;
		}
		break;
	case kObjectOnion:
		if (_rottenOnion)
			result = "a rotten onion";
		else if (_onionInVinegar)
			result = "a pickled onion (in the vinegar)";
		else
			result = Common::String(items[which - 1]);
		break;
	default:
		if ((which < kObjectNum) && (which > 0))
			result = Common::String(items[which - 1]);
		else
			result = "";
		break;
	}
	return result;
}

Common::String AvalancheEngine::f5Does() {
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
		if (_animation->inField(5))
			return Common::String::format("%cPPlay the harp", kVerbCodePlay);
		break;
	default:
		break;
	}

	return Common::String::format("%c", kVerbCodePardon); // If all else fails...
}

void AvalancheEngine::flipRoom(Room room, byte ped) {
	assert((ped > 0) && (ped < 15));
	if (!_alive) {
		// You can't leave the room if you're dead.
		_animation->_sprites[0]->_moveX = 0;
		_animation->_sprites[0]->_moveY = 0; // Stop him from moving.
		return;
	}

	if ((room == kRoomDummy) && (_room == kRoomLusties)) {
		_animation->hideInCupboard();
		return;
	}

	if ((_jumpStatus > 0) && (_room == kRoomInsideCardiffCastle)) {
		// You can't *jump* out of Cardiff Castle!
		_animation->_sprites[0]->_moveX = 0;
		return;
	}

	exitRoom(_room);
	fadeOut();

	for (int16 i = 1; i < _animation->kSpriteNumbMax; i++) {
		if (_animation->_sprites[i]->_quick)
			_animation->_sprites[i]->remove();
	} // Deallocate sprite

	if (_room == kRoomLustiesRoom)
		_enterCatacombsFromLustiesRoom = true;

	if (room > kRoomMap)
		return;

	enterRoom(room, ped);
	_animation->appearPed(0, ped - 1);
	_enterCatacombsFromLustiesRoom = false;
	_animation->setOldDirection(_animation->getDirection());
	_animation->setDirection(_animation->_sprites[0]->_facingDir);
	drawDirection();

	fadeIn();
}

/**
 * Open the Door.
 * This slides the door open. The data really ought to be saved in
 * the Also file, and will be next time. However, for now, they're
 * here.
 * @remarks	Originally called 'open_the_door'
 */
void AvalancheEngine::openDoor(Room whither, byte ped, byte magicnum) {
	switch (_room) {
	case kRoomOutsideYours:
	case kRoomOutsideNottsPub:
	case kRoomOutsideDucks:
		_sequence->startOutsideSeq(whither, ped);
		break;
	case kRoomInsideCardiffCastle:
		_sequence->startCardiffSeq(whither, ped);
		break;
	case kRoomAvvysGarden:
	case kRoomEntranceHall:
	case kRoomInsideAbbey:
	case kRoomYourHall:
		_sequence->startHallSeq(whither, ped);
		break;
	case kRoomMusicRoom:
	case kRoomOutsideArgentPub:
		_sequence->startMusicRoomSeq2(whither, ped);
		break;
	case kRoomLusties:
		switch (magicnum) {
		case 14:
			if (_avvysInTheCupboard) {
				_animation->hideInCupboard();
				_sequence->startCupboardSeq();
				return;
			} else {
				_animation->appearPed(0, 5);
				_animation->_sprites[0]->_facingDir = kDirRight;
				_sequence->startLustiesSeq2(whither, ped);
			}
			break;
		case 12:
			_sequence->startLustiesSeq3(whither, ped);
			break;
		default:
			break;
		}
		break;
	default:
		_sequence->startDummySeq(whither, ped);
	}
}

void AvalancheEngine::setRoom(People persId, Room roomId) {
	_whereIs[persId - kPeopleAvalot] = roomId;
}

Room AvalancheEngine::getRoom(People persId) {
	return _whereIs[persId - kPeopleAvalot];
}
} // End of namespace Avalanche
