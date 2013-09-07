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
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* LUCERNA		The screen, [keyboard] and mouse handler.*/

#include "avalanche/avalanche.h"

#include "avalanche/lucerna2.h"
#include "avalanche/gyro2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/visa2.h"
#include "avalanche/timeout2.h"
#include "avalanche/animation.h"
#include "avalanche/enid2.h"
#include "avalanche/celer2.h"
#include "avalanche/pingo2.h"
#include "avalanche/sequence2.h"
#include "avalanche/acci2.h"
#include "avalanche/roomnums.h"

#include "common/rect.h"
#include "common/system.h"

#include "graphics/palette.h"

namespace Avalanche {

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

	if ((_hour == 0) && (_oldHour != 0) && (_oldHour != 17717))
		_vm->_scrolls->displayText(Common::String("Good morning!") + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine
		+ "Yes, it's just past midnight. Are you having an all-night Avvy session? Glad you like the game that much!");

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
	warning("STUB: Clock::chime()");
}

Lucerna::Lucerna(AvalancheEngine *vm) : _fxHidden(false), _clock(vm) {
	_vm = vm;
}

Lucerna::~Lucerna() {
	for (byte i = 0; i < 31; i++) {
		for (byte j = 0; j < 2; j++) {
			if (_vm->_gyro->_also[i][j] != 0)  {
				delete _vm->_gyro->_also[i][j];
				_vm->_gyro->_also[i][j] = 0;
			}
		}
	}
}

void Lucerna::init() {
	for (byte i = 0; i < 31; i++) {
		for (byte j = 0; j < 2; j++)
			_vm->_gyro->_also[i][j] = 0;
	}

#if 0
	if (_vm->_enhanced->atbios)
		_vm->_gyro->atkey = "f1";
	else
		_vm->_gyro->atkey = "alt-";
#endif
}

/**
 * Call a given Verb
 * @remarks	Originally called 'callverb'
 */
void Lucerna::callVerb(byte id) {
	if (id == _vm->_acci->kPardon) {
		_vm->_scrolls->displayText(Common::String("The f5 key lets you do a particular action in certain ") +
			"situations. However, at the moment there is nothing " +
			"assigned to it. You may press alt-A to see what the " +
			"current setting of this key is.");
	} else {
		_vm->_gyro->_weirdWord = false;
		_vm->_acci->_polite = true;
		_vm->_acci->_verb = id;
		_vm->_acci->doThat();
	}
}

void Lucerna::drawAlsoLines() {
	CursorMan.showMouse(false);

	_vm->_graphics->_magics.fillRect(Common::Rect(0, 0, 640, 200), 0);
	_vm->_graphics->_magics.frameRect(Common::Rect(0, 45, 640, 161), 15);

	for (byte i = 0; i < _vm->_gyro->_lineNum; i++) {
		// We had to check if the lines are within the borders of the screen.
		if ((_vm->_gyro->_lines[i]._x1 >= 0) && (_vm->_gyro->_lines[i]._x1 < _vm->_graphics->kScreenWidth) && (_vm->_gyro->_lines[i]._y1 >= 0) && (_vm->_gyro->_lines[i]._y1 < _vm->_graphics->kScreenHeight)
		 && (_vm->_gyro->_lines[i]._x2 >= 0) && (_vm->_gyro->_lines[i]._x2 < _vm->_graphics->kScreenWidth) && (_vm->_gyro->_lines[i]._y2 >= 0) && (_vm->_gyro->_lines[i]._y2 < _vm->_graphics->kScreenHeight))
			_vm->_graphics->_magics.drawLine(_vm->_gyro->_lines[i]._x1, _vm->_gyro->_lines[i]._y1, _vm->_gyro->_lines[i]._x2, _vm->_gyro->_lines[i]._y2, _vm->_gyro->_lines[i]._color);
	}

	CursorMan.showMouse(true);
}

// readAlsoStringFromFile, scram and unScramble are only used in loadAlso

Common::String Lucerna::readAlsoStringFromFile() {
	Common::String str;
	byte length = file.readByte();
	for (int i = 0; i < length; i++)
		str += file.readByte();
	return str;
}

void Lucerna::scram(Common::String &str) {
	for (uint i = 0; i < str.size(); i++)
		str.setChar(str[i] ^ 177, i);
}

void Lucerna::unScramble() {
	for (byte i = 0; i < 31; i++) {
		for (byte j = 0; j < 2; j++) {
			if (_vm->_gyro->_also[i][j] != 0)
				scram(*_vm->_gyro->_also[i][j]);
		}
	}
	scram(_vm->_gyro->_listen);
	scram(_vm->_gyro->_flags);
}

void Lucerna::loadAlso(byte num) {
	for (byte i = 0; i < 31; i++) {
		for (byte j = 0; j < 2; j++) {
			if (_vm->_gyro->_also[i][j] != 0)  {
				delete _vm->_gyro->_also[i][j];
				_vm->_gyro->_also[i][j] = 0;
			}
		}
	}
	Common::String filename;
	filename = filename.format("also%d.avd", num);
	if (!file.open(filename)) {
		warning("AVALANCHE: Lucerna: File not found: %s", filename.c_str());
		return;
	}

	file.seek(128);

	byte alsoNum = file.readByte();
	for (byte i = 0; i <= alsoNum; i++) {
		for (byte j = 0; j < 2; j++) {
			_vm->_gyro->_also[i][j] = new Common::String;
			*_vm->_gyro->_also[i][j] = readAlsoStringFromFile();
		}
		*_vm->_gyro->_also[i][0] = Common::String('\x9D') + *_vm->_gyro->_also[i][0] + Common::String('\x9D');
	}

	memset(_vm->_gyro->_lines, 0xFF, sizeof(_vm->_gyro->_lines));

	_vm->_gyro->_lineNum = file.readByte();
	for (byte i = 0; i < _vm->_gyro->_lineNum; i++) {
		_vm->_gyro->_lines[i]._x1 = file.readSint16LE();
		_vm->_gyro->_lines[i]._y1 = file.readSint16LE();
		_vm->_gyro->_lines[i]._x2 = file.readSint16LE();
		_vm->_gyro->_lines[i]._y2 = file.readSint16LE();
		_vm->_gyro->_lines[i]._color = file.readByte();
	}

	memset(_vm->_gyro->_peds, 177, sizeof(_vm->_gyro->_peds));
	byte pedNum = file.readByte();
	for (byte i = 0; i < pedNum; i++) {
		_vm->_gyro->_peds[i]._x = file.readSint16LE();
		_vm->_gyro->_peds[i]._y = file.readSint16LE();
		_vm->_gyro->_peds[i]._direction = file.readByte();
	}

	_vm->_gyro->_fieldNum = file.readByte();
	for (byte i = 0; i < _vm->_gyro->_fieldNum; i++) {
		_vm->_gyro->_fields[i]._x1 = file.readSint16LE();
		_vm->_gyro->_fields[i]._y1 = file.readSint16LE();
		_vm->_gyro->_fields[i]._x2 = file.readSint16LE();
		_vm->_gyro->_fields[i]._y2 = file.readSint16LE();
	}

	for (byte i = 0; i < 15; i++) {
		_vm->_gyro->_magics[i]._operation = file.readByte();
		_vm->_gyro->_magics[i]._data = file.readUint16LE();
	}

	for (byte i = 0; i < 7; i++) {
		_vm->_gyro->_portals[i]._operation = file.readByte();
		_vm->_gyro->_portals[i]._data = file.readUint16LE();
	}

	_vm->_gyro->_flags.clear();
	for (byte i = 0;  i < 26; i++)
		_vm->_gyro->_flags += file.readByte();

	int16 listen_length = file.readByte();
	_vm->_gyro->_listen.clear();
	for (byte i = 0; i < listen_length; i++)
		_vm->_gyro->_listen += file.readByte();

	drawAlsoLines();

	file.close();
	unScramble();
	for (byte i = 0; i <= alsoNum; i++)
		*_vm->_gyro->_also[i][0] = Common::String(',') + *_vm->_gyro->_also[i][0] + ',';
}

void Lucerna::loadRoom(byte num) {
	CursorMan.showMouse(false);

	_vm->_graphics->fleshColors();

	Common::String filename;
	filename = filename.format("place%d.avd", num);
	if (!file.open(filename)) {
		warning("AVALANCHE: Lucerna: File not found: %s", filename.c_str());
		return;
	}

	file.seek(146);
	if (!_vm->_gyro->_roomnName.empty())
		_vm->_gyro->_roomnName.clear();
	for (byte i = 0; i < 30; i++) {
		char actChar = file.readByte();
		if ((32 <= actChar) && (actChar <= 126))
			_vm->_gyro->_roomnName += actChar;
	}
	// Compression method byte follows this...

	file.seek(177);

	_vm->_graphics->_background = _vm->_graphics->loadPictureRow(file, _vm->_graphics->kBackgroundWidth, _vm->_graphics->kBackgroundHeight);
	_vm->_graphics->refreshBackground();

	file.close();

	loadAlso(num);
	_vm->_celer->loadBackgroundSprites(num);
	CursorMan.showMouse(true);
}



void Lucerna::zoomOut(int16 x, int16 y) {
	warning("STUB: Lucerna::zoomout()");
}

void Lucerna::findPeople(byte room) {
	for (byte i = 1; i < 29; i++) {
		if (_vm->_gyro->_whereIs[i] == room) {
			if (i < 25)
				_vm->_gyro->_him = i + 150;
			else
				_vm->_gyro->_her = i + 150;
		}
	}
}

void Lucerna::exitRoom(byte x) {
	//nosound();
	_vm->_celer->forgetBackgroundSprites();
	_vm->_gyro->_seeScroll = true;  // This stops the trippancy system working over the length of this procedure.

	switch (x) {
	case r__spludwicks:
		_vm->_timeout->lose_timer(_vm->_timeout->reason_avariciustalks);
		 _vm->_gyro->_dna._avariciusTalk = 0;
		// He doesn't HAVE to be talking for this to work. It just deletes it IF it exists.
		break;
	case r__bridge:
		if (_vm->_gyro->_dna._drawbridgeOpen > 0) {
			_vm->_gyro->_dna._drawbridgeOpen = 4; // Fully open.
			_vm->_timeout->lose_timer(_vm->_timeout->reason_drawbridgefalls);
		}
		break;
	case r__outsidecardiffcastle:
		_vm->_timeout->lose_timer(_vm->_timeout->reason_cardiffsurvey);
		break;
	case r__robins:
		_vm->_timeout->lose_timer(_vm->_timeout->reason_getting_tied_up);
		break;
	}

	_vm->_gyro->_interrogation = 0; // Leaving the room cancels all the questions automatically.

	_vm->_gyro->_seeScroll = false; // Now it can work again!

	_vm->_gyro->_dna._lastRoom = _vm->_gyro->_dna._room;
	if (_vm->_gyro->_dna._room != r__map)
		_vm->_gyro->_dna._lastRoomNotMap = _vm->_gyro->_dna._room;
}

void Lucerna::enterNewTown() {
	_vm->_dropdown->setupMenu();

	switch (_vm->_gyro->_dna._room) {
	case r__outsidenottspub: // Entry into Nottingham.
		if ((_vm->_gyro->_dna._roomCount[r__robins] > 0) && (_vm->_gyro->_dna._beenTiedUp) && (!_vm->_gyro->_dna._takenMushroom))
			_vm->_gyro->_dna._mushroomGrowing = true;
		break;
	case r__wisewomans: { // Entry into Argent.
		if (_vm->_gyro->_dna._talkedToCrapulus && (! _vm->_gyro->_dna._lustieIsAsleep)) {
			_vm->_gyro->_dna._spludwickAtHome = !((_vm->_gyro->_dna._roomCount[r__wisewomans] % 3) == 1);
			_vm->_gyro->_dna._crapulusWillTell = ! _vm->_gyro->_dna._spludwickAtHome;
		} else {
			_vm->_gyro->_dna._spludwickAtHome = true;
			_vm->_gyro->_dna._crapulusWillTell = false;
		}
		if (_vm->_gyro->_dna._boxContent == _vm->_gyro->kObjectWine)
			_vm->_gyro->_dna._wineState = 3; // Vinegar
	}
	break;
	}

	if (_vm->_gyro->_dna._room != r__outsideducks) {
		if ((_vm->_gyro->_dna._objects[_vm->_gyro->kObjectOnion - 1]) && !(_vm->_gyro->_dna._onionInVinegar))
			_vm->_gyro->_dna._rottenOnion = true; // You're holding the onion
	}
}



void Lucerna::putGeidaAt(byte whichPed, byte &ped) {
	if (ped == 0)
		return;
	_vm->_animation->tr[1].init(5, false, _vm->_animation); // load Geida
	_vm->_animation->apped(2, whichPed);
	_vm->_animation->tr[1].call_eachstep = true;
	_vm->_animation->tr[1].eachstep = _vm->_animation->kProcGeida;
}

void Lucerna::enterRoom(byte room, byte ped) {
	_vm->_gyro->_seeScroll = true;  // This stops the trippancy system working over the length of this procedure.

	findPeople(room);
	_vm->_gyro->_dna._room = room;
	if (ped != 0)
		_vm->_gyro->_dna._roomCount[room]++;

	loadRoom(room);

	if ((_vm->_gyro->_dna._roomCount[room] == 0) && (! _vm->_gyro->setFlag('S')))
		incScore(1);

	_vm->_gyro->_whereIs[_vm->_gyro->kPeopleAvalot - 150] = _vm->_gyro->_dna._room;

	if (_vm->_gyro->_dna._geidaFollows)
		_vm->_gyro->_whereIs[_vm->_gyro->kPeopleGeida - 150] = room;

	_vm->_gyro->_roomTime = 0;


	if ((_vm->_gyro->_dna._lastRoom == r__map) && (_vm->_gyro->_dna._lastRoomNotMap != _vm->_gyro->_dna._room))
		enterNewTown();


	switch (room) {
	case r__yours:
		if (_vm->_gyro->_dna._avvyInBed) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_graphics->refreshBackground();
			_vm->_timeout->set_up_timer(100, _vm->_timeout->procarkata_shouts, _vm->_timeout->reason_arkata_shouts);
		}
		break;

	case r__outsideyours:
		if (ped > 0) {
			if (! _vm->_gyro->_dna._talkedToCrapulus) {

				_vm->_gyro->_whereIs[_vm->_gyro->kPeopleCrapulus - 150] = r__outsideyours;
				_vm->_animation->tr[1].init(8, false, _vm->_animation); // load Crapulus

				if (_vm->_gyro->_dna._roomCount[r__outsideyours] == 1) {
					_vm->_animation->apped(2, 4); // Start on the right-hand side of the screen.
					_vm->_animation->tr[1].walkto(5); // Walks up to greet you.
				} else {
					_vm->_animation->apped(2, 5); // Starts where he was before.
					_vm->_animation->tr[1].face = Animation::kDirLeft;
				}

				_vm->_animation->tr[1].call_eachstep = true;
				_vm->_animation->tr[1].eachstep = _vm->_animation->kProcFaceAvvy; // He always faces Avvy.

			} else _vm->_gyro->_whereIs[_vm->_gyro->kPeopleCrapulus - 150] = r__nowhere;

			if (_vm->_gyro->_dna._crapulusWillTell) {
				_vm->_animation->tr[1].init(8, false, _vm->_animation);
				_vm->_animation->apped(2, 2);
				_vm->_animation->tr[1].walkto(4);
				_vm->_timeout->set_up_timer(20, _vm->_timeout->proccrapulus_splud_out, _vm->_timeout->reason_crapulus_says_spludwick_out);
				_vm->_gyro->_dna._crapulusWillTell = false;
			}
		}
		break;

	case r__outsidespludwicks:
		if ((_vm->_gyro->_dna._roomCount[r__outsidespludwicks] == 1) && (ped == 1)) {
			_vm->_timeout->set_up_timer(20, _vm->_timeout->procbang, _vm->_timeout->reason_explosion);
			_vm->_gyro->_dna._spludwickAtHome = true;
		}
		break;

	case r__spludwicks:
		if (_vm->_gyro->_dna._spludwickAtHome) {
			if (ped > 0) {
				_vm->_animation->tr[1].init(2, false, _vm->_animation); // load Spludwick
				_vm->_animation->apped(2, 2);
				_vm->_gyro->_whereIs[1] = r__spludwicks;
			}

			_vm->_gyro->_dna._dogFoodPos = 0;  // _vm->_gyro->also Spludwick pos.

			_vm->_animation->tr[1].call_eachstep = true;
			_vm->_animation->tr[1].eachstep = _vm->_animation->kProcGeida;
		} else
			_vm->_gyro->_whereIs[1] = r__nowhere;
		break;

	case r__brummieroad:
		if (_vm->_gyro->_dna._geidaFollows)
			putGeidaAt(5, ped);
		if (_vm->_gyro->_dna._cwytalotGone) {
			_vm->_gyro->_magics[kColorLightred - 1]._operation = _vm->_gyro->kMagicNothing;
			_vm->_gyro->_whereIs[_vm->_gyro->kPeopleCwytalot - 150] = r__nowhere;
		} else {
			if (ped > 0) {
				_vm->_animation->tr[1].init(4, false, _vm->_animation); // 4 = Cwytalot
				_vm->_animation->tr[1].call_eachstep = true;
				_vm->_animation->tr[1].eachstep = _vm->_animation->kProcFollowAvvyY;
				_vm->_gyro->_whereIs[_vm->_gyro->kPeopleCwytalot - 150] = r__brummieroad;

				if (_vm->_gyro->_dna._roomCount[r__brummieroad] == 1) { // First time here...
					_vm->_animation->apped(2, 2); // He appears on the right of the screen...
					_vm->_animation->tr[1].walkto(4); // ...and he walks up...
				} else {
					// You've been here before.
					_vm->_animation->apped(2, 4); // He's standing in your way straight away...
					_vm->_animation->tr[1].face = Animation::kDirLeft;
				}
			}
		}
		break;

	case r__argentroad:
		if ((_vm->_gyro->_dna._cwytalotGone) && (! _vm->_gyro->_dna._passedCwytalotInHerts) && (ped == 2) &&
		        (_vm->_gyro->_dna._roomCount[r__argentroad] > 3)) {
			_vm->_animation->tr[1].init(4, false, _vm->_animation); // 4 = Cwytalot again
			_vm->_animation->apped(2, 1);
			_vm->_animation->tr[1].walkto(2);
			_vm->_animation->tr[1].vanishifstill = true;
			_vm->_gyro->_dna._passedCwytalotInHerts = true;
			// _vm->_gyro->whereis[#157] = r__Nowhere; // can we fit this in?
			_vm->_timeout->set_up_timer(20, _vm->_timeout->proc_cwytalot_in_herts, _vm->_timeout->reason_cwytalot_in_herts);
		}
		break;

	case r__bridge:
		if (_vm->_gyro->_dna._drawbridgeOpen == 4) { // open
			_vm->_celer->drawBackgroundSprite(-1, -1, 3); // Position of drawbridge
			_vm->_graphics->refreshBackground();
			_vm->_gyro->_magics[kColorGreen - 1]._operation = _vm->_gyro->kMagicNothing; // You may enter the drawbridge.
		}
		if (_vm->_gyro->_dna._geidaFollows)
			putGeidaAt(ped + 3, ped); // load Geida
		break;

	case r__robins:
		if (ped > 0) {
			if (! _vm->_gyro->_dna._beenTiedUp) {
				// A welcome party... or maybe not...
				_vm->_animation->tr[1].init(6, false, _vm->_animation);
				_vm->_animation->apped(2, 2);
				_vm->_animation->tr[1].walkto(3);
				_vm->_timeout->set_up_timer(36, _vm->_timeout->procget_tied_up, _vm->_timeout->reason_getting_tied_up);
			}
		}

		if (_vm->_gyro->_dna._beenTiedUp) {
			_vm->_gyro->_whereIs[_vm->_gyro->kPeopleRobinHood - 150] = 0;
			_vm->_gyro->_whereIs[_vm->_gyro->kPeopleFriarTuck - 150] = 0;
		}

		if (_vm->_gyro->_dna._tiedUp)
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);

		if (!_vm->_gyro->_dna._mushroomGrowing)
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_graphics->refreshBackground();
		break;

	case r__outsidecardiffcastle:
		if (ped > 0)
			switch (_vm->_gyro->_dna._cardiffQuestionNum) {
			case 0 : { // You've answered NONE of his questions.
				_vm->_animation->tr[1].init(9, false, _vm->_animation);
				_vm->_animation->apped(2, 2);
				_vm->_animation->tr[1].walkto(3);
				_vm->_timeout->set_up_timer(47, _vm->_timeout->proccardiffsurvey, _vm->_timeout->reason_cardiffsurvey);
			}
			break;
			case 5 :
				_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicNothing;
				break; // You've answered ALL his questions. => nothing happens.
			default: { // You've answered SOME of his questions.
				_vm->_animation->tr[1].init(9, false, _vm->_animation);
				_vm->_animation->apped(2, 3);
				_vm->_animation->tr[1].face = Animation::kDirRight;
				_vm->_timeout->set_up_timer(3, _vm->_timeout->proccardiff_return, _vm->_timeout->reason_cardiffsurvey);
			}
			}
		if (_vm->_gyro->_dna._cardiffQuestionNum < 5)
			_vm->_gyro->_interrogation = _vm->_gyro->_dna._cardiffQuestionNum;
		else _vm->_gyro->_interrogation = 0;
		break;

	case r__map:
		// You're entering the map.
		dawn();
		//setactivepage(cp);
		if (ped > 0)
			zoomOut(_vm->_gyro->_peds[ped - 1]._x, _vm->_gyro->_peds[ped - 1]._y);
		//setactivepage(1 - cp);

		{
			if ((_vm->_gyro->_dna._objects[_vm->_gyro->kObjectWine - 1]) && (_vm->_gyro->_dna._wineState != 3)) {
				_vm->_visa->dixi('q', 9); // Don't want to waste the wine!
				_vm->_gyro->_dna._objects[_vm->_gyro->kObjectWine - 1] = false;
				refreshObjectList();
			}
		}

		_vm->_visa->dixi('q', 69);
		break;

	case r__catacombs:
		if ((ped == 0) || (ped == 3) || (ped == 5) || (ped == 6)) {

			switch (ped) {
			case 3: { // Enter from oubliette
				_vm->_gyro->_dna._catacombX = 8;
				_vm->_gyro->_dna._catacombY = 4;
			}
			break;
			case 5: { // Enter from du Lustie's
				_vm->_gyro->_dna._catacombX = 8;
				_vm->_gyro->_dna._catacombY = 7;
			}
			break;
			case 6: { // Enter from Geida's
				_vm->_gyro->_dna._catacombX = 4;
				_vm->_gyro->_dna._catacombY = 1;
			}
			break;
			}

			_vm->_gyro->_dna._enterCatacombsFromLustiesRoom = true;
			_vm->_animation->catamove(ped);
			_vm->_gyro->_dna._enterCatacombsFromLustiesRoom = false;
		}
		break;

	case r__argentpub:
		if (_vm->_gyro->_dna._wonNim)
			_vm->_celer->drawBackgroundSprite(-1, -1, 1);   // No lute by the settle.
		_vm->_gyro->_dna._malagauche = 0; // Ready to boot Malagauche
		if (_vm->_gyro->_dna._givenBadgeToIby) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 8);
			_vm->_celer->drawBackgroundSprite(-1, -1, 9);
		}
		_vm->_graphics->refreshBackground();
		break;

	case r__lustiesroom:
		_vm->_gyro->_dna._dogFoodPos = 1; // Actually, du Lustie pos.
		if (_vm->_animation->tr[0].whichsprite == 0) // Avvy in his normal clothes
			_vm->_timeout->set_up_timer(3, _vm->_timeout->proccallsguards, _vm->_timeout->reason_du_lustie_talks);
		else if (! _vm->_gyro->_dna._enteredLustiesRoomAsMonk) // already
			// Presumably, Avvy dressed as a monk.
			_vm->_timeout->set_up_timer(3, _vm->_timeout->procgreetsmonk, _vm->_timeout->reason_du_lustie_talks);

		if (_vm->_gyro->_dna._geidaFollows) {
			putGeidaAt(5, ped);
			if (_vm->_gyro->_dna._lustieIsAsleep) {
				_vm->_celer->drawBackgroundSprite(-1, -1, 5);
				_vm->_graphics->refreshBackground();
			}
		}
		break;

	case r__musicroom:
		if (_vm->_gyro->_dna._jacquesState > 0) {
			_vm->_gyro->_dna._jacquesState = 5;
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_celer->drawBackgroundSprite(-1, -1, 4);
			_vm->_gyro->_magics[kColorBrown - 1]._operation = _vm->_gyro->kMagicNothing;
			_vm->_gyro->_whereIs[_vm->_gyro->kPeopleJacques - 150] = 0;
		}
		if (ped != 0) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(6);
			_vm->_sequence->then_show(5);
			_vm->_sequence->then_show(7);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__outsidenottspub:
		if (ped == 2) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(3);
			_vm->_sequence->then_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(4);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__outsideargentpub:
		if (ped == 2)  {
			_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(6);
			_vm->_sequence->then_show(5);
			_vm->_sequence->then_show(7);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__wisewomans:
		_vm->_animation->tr[1].init(11, false, _vm->_animation);
		if ((_vm->_gyro->_dna._roomCount[r__wisewomans] == 1) && (ped > 0)) {
			_vm->_animation->apped(2, 2); // Start on the right-hand side of the screen.
			_vm->_animation->tr[1].walkto(4); // Walks up to greet you.
		} else {
			_vm->_animation->apped(2, 4); // Starts where she was before.
			_vm->_animation->tr[1].face = Animation::kDirLeft;
		}

		_vm->_animation->tr[1].call_eachstep = true;
		_vm->_animation->tr[1].eachstep = _vm->_animation->kProcFaceAvvy; // She always faces Avvy.
		break;

	case r__insidecardiffcastle:
		if (ped > 0) {
			_vm->_animation->tr[1].init(10, false, _vm->_animation); // Define the dart.
			_vm->_celer->drawBackgroundSprite(-1, -1, 1);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(1);
			if (_vm->_gyro->_dna._arrowInTheDoor)
				_vm->_sequence->then_show(3);
			else
				_vm->_sequence->then_show(2);

			if (_vm->_gyro->_dna._takenPen)
				_vm->_celer->drawBackgroundSprite(-1, -1, 4);

			_vm->_sequence->start_to_close();
		} else {
			_vm->_celer->drawBackgroundSprite(-1, -1, 1);
			if (_vm->_gyro->_dna._arrowInTheDoor)
				_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			else
				_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
		}
		break;

	case r__avvysgarden:
		if (ped == 1)  {
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__entrancehall:
	case r__insideabbey:
	case r__yourhall:
		if (ped == 2)  {
#if 0
			// It was the original:
			_vm->_celer->show_one(-1, -1, 2);
			_vm->_sequence->first_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
#endif

			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__aylesoffice:
		if (_vm->_gyro->_dna._aylesIsAwake)
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
		_vm->_graphics->refreshBackground();
		break; // Ayles awake.

	case r__geidas:
		putGeidaAt(2, ped);
		break; // load Geida

	case r__easthall:
	case r__westhall:
		if (_vm->_gyro->_dna._geidaFollows)
			putGeidaAt(ped + 2, ped);
		break;

	case r__lusties:
		if (_vm->_gyro->_dna._geidaFollows)
			putGeidaAt(ped + 6, ped);
		break;

	case r__nottspub:
		if (_vm->_gyro->_dna._sittingInPub)  _vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_gyro->_dna._dogFoodPos = 1; // Actually, du Lustie pos.
		break;

	case r__outsideducks:
		if (ped == 2) {
			// Shut the door
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(3);
			_vm->_sequence->first_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(4);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__ducks:
		_vm->_gyro->_dna._dogFoodPos = 1;
		break; // Actually, Duck pos.
	}

	_vm->_gyro->_seeScroll = false; // Now it can work again!

	if (_vm->_gyro->isLoaded)
		_vm->_gyro->isLoaded = false;
}

void Lucerna::thinkAbout(byte object, bool type) {
	const int16 picSize = 966;

	_vm->_gyro->_thinks = object;
	object--;

	_vm->_gyro->setMousePointerWait();

	switch (type) {
	case Gyro::kThing:
		if (!file.open("thinks.avd")) {
			warning("AVALANCHE: Lucerna: File not found: thinks.avd");
			return;
		}
		break;
	case Gyro::kPerson:
		if (!file.open("folk.avd")) {
			warning("AVALANCHE: Lucerna: File not found: folk.avd");
			return;
		}

		object = object - 149;
		if (object >= 25)
			object -= 8;
		if (object == 20)
			object--; // Last time...

		break;
	}

	file.seek(object * picSize + 65);

	::Graphics::Surface picture = _vm->_graphics->loadPictureGraphic(file);

	_vm->_graphics->drawPicture(_vm->_graphics->_surface, picture, 205, 170);

	picture.free();

	file.close();

	CursorMan.showMouse(false);

#if 0
	setactivepage(3);
	putimage(x, y, p, 0);
	setactivepage(1 - cp);
#endif

	CursorMan.showMouse(true);
	_vm->_gyro->_thinkThing = type;
}

void Lucerna::loadDigits() {   // Load the scoring digits & rwlites
	const byte digitsize = 134;
	const byte rwlitesize = 126;

	if (!file.open("digit.avd")) {
		warning("AVALANCHE: Lucerna: File not found: digit.avd");
		return;
	}

	for (byte i = 0; i < 10; i++) {
		file.seek(i * digitsize);
		_vm->_gyro->_digits[i] = _vm->_graphics->loadPictureGraphic(file);
	}

	for (byte i = 0; i < 9; i++) {
		file.seek(10 * digitsize + i * rwlitesize);
		_vm->_gyro->_directions[i] = _vm->_graphics->loadPictureGraphic(file);
	}

	file.close();
}

void Lucerna::drawToolbar() {
	if (!file.open("useful.avd")) {
		warning("AVALANCHE: Lucerna: File not found: useful.avd");
		return;
	}

	file.seek(40);

	CursorMan.showMouse(false);

	::Graphics::Surface picture = _vm->_graphics->loadPictureGraphic(file);

	_vm->_graphics->drawPicture(_vm->_graphics->_surface, picture, 5, 169);

	picture.free();

	file.close();

	CursorMan.showMouse(true);

	_vm->_gyro->_oldDirection = 177;
	drawDirection();
}

void Lucerna::drawScore() {
	if (_vm->_gyro->kDemo)
		return;

	uint16 score = _vm->_gyro->_dna._score;
	int8 numbers[3] = {0, 0, 0};
	for (byte i = 0; i < 2; i++) {
		byte divisor = 1;
		for (byte j = 0; j < (2 - i); j++)
			divisor *= 10;
		numbers[i] = score / divisor;
		score -= numbers[i] * divisor;
	}
	numbers[2] = score;

	CursorMan.showMouse(false);

	for (byte fv = 0; fv < 3; fv++) {
		if (_vm->_gyro->_scoreToDisplay[fv] != numbers[fv])
			_vm->_graphics->drawPicture(_vm->_graphics->_surface, _vm->_gyro->_digits[numbers[fv]], 250 + (fv + 1) * 15, 177);
	}

	CursorMan.showMouse(true);

	for (byte i = 0; i < 3; i++)
		_vm->_gyro->_scoreToDisplay[i] = numbers[i];
}

void Lucerna::incScore(byte num) {     // Add on no. of points
	for (byte q = 1; q <= num; q++) {
		_vm->_gyro->_dna._score++;

#if 0
		if (soundfx) {
			for (byte fv = 1; fv <= 97; fv++)
				sound(177 + dna.score * 3);
		}
		nosound;
#endif
	}
	warning("STUB: Lucerna::points()");

	drawScore();
}

void Lucerna::useCompass(const Common::Point &cursorPos) {
	byte color = *(byte *)_vm->_graphics->_surface.getBasePtr(cursorPos.x, cursorPos.y / 2);

	switch (color) {
	case kColorGreen:
		_vm->_gyro->_dna._direction = Animation::kDirUp;
		_vm->_animation->rwsp(0, Animation::kDirUp);
		drawDirection();
		break;
	case kColorBrown:
		_vm->_gyro->_dna._direction = Animation::kDirDown;
		_vm->_animation->rwsp(0, Animation::kDirDown);
		drawDirection();
		break;
	case kColorCyan:
		_vm->_gyro->_dna._direction = Animation::kDirLeft;
		_vm->_animation->rwsp(0, Animation::kDirLeft);
		drawDirection();
		break;
	case kColorLightmagenta:
		_vm->_gyro->_dna._direction = Animation::kDirRight;
		_vm->_animation->rwsp(0, Animation::kDirRight);
		drawDirection();
		break;
	case kColorRed:
	case kColorWhite:
	case kColorLightcyan:
	case kColorYellow: // Fall-throughs are intended.
		_vm->_animation->stopwalking();
		drawDirection();
		break;
	}
}

void Lucerna::fxToggle() {
	warning("STUB: Lucerna::fxtoggle()");
}

void Lucerna::refreshObjectList() {
	_vm->_gyro->_dna._carryNum = 0;
	if (_vm->_gyro->_thinkThing && !_vm->_gyro->_dna._objects[_vm->_gyro->_thinks - 1])
		thinkAbout(_vm->_gyro->kObjectMoney, Gyro::kThing); // you always have money

	for (byte i = 0; i < kObjectNum; i++) {
		if (_vm->_gyro->_dna._objects[i]) {
			_vm->_gyro->_dna._carryNum++;
			_vm->_gyro->_objectList[_vm->_gyro->_dna._carryNum] = i + 1;
		}
	}
}

void Lucerna::guideAvvy(Common::Point cursorPos) {
	if (!_vm->_gyro->_dna._userMovesAvvy)
		return;

	cursorPos.y /= 2;
	byte what;

	// _vm->_animation->tr[0] is Avalot.)
	if (cursorPos.x < _vm->_animation->tr[0]._x)
		what = 1;
	else if (cursorPos.x > (_vm->_animation->tr[0]._x + _vm->_animation->tr[0]._info._xLength))
		what = 2;
	else
		what = 0; // On top

	if (cursorPos.y < _vm->_animation->tr[0]._y)
		what += 3;
	else if (cursorPos.y > (_vm->_animation->tr[0]._y + _vm->_animation->tr[0]._info._yLength))
		what += 6;

	switch (what) {
	case 0:
		_vm->_animation->stopwalking();
		break; // Clicked on Avvy: no movement.
	case 1:
		_vm->_animation->rwsp(0, Animation::kDirLeft);
		break;
	case 2:
		_vm->_animation->rwsp(0, Animation::kDirRight);
		break;
	case 3:
		_vm->_animation->rwsp(0, Animation::kDirUp);
		break;
	case 4:
		_vm->_animation->rwsp(0, Animation::kDirUpLeft);
		break;
	case 5:
		_vm->_animation->rwsp(0, Animation::kDirUpRight);
		break;
	case 6:
		_vm->_animation->rwsp(0, Animation::kDirDown);
		break;
	case 7:
		_vm->_animation->rwsp(0, Animation::kDirDownLeft);
		break;
	case 8:
		_vm->_animation->rwsp(0, Animation::kDirDownRight);
		break;
	}    // No other values are possible.

	drawDirection();
}

void Lucerna::checkClick() {
	Common::Point cursorPos = _vm->getMousePos();
	_vm->_gyro->_onToolbar = _vm->_gyro->kSlowComputer && ((cursorPos.y >= 169) || (cursorPos.y <= 10));

	/*if (_vm->_gyro->mrelease > 0)
	_vm->_gyro->after_the_scroll = false;*/

	if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
		_vm->_gyro->newMouse(1); // up arrow
	else if ((317 <= cursorPos.y) && (cursorPos.y <= 339))
		_vm->_gyro->newMouse(8); //I-beam
	else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
		_vm->_gyro->newMouse(2); // screwdriver
	else if (!_vm->_gyro->_dropdownActive) { // Dropdown can handle its own pointers.
		if (_holdLeftMouse) {
			_vm->_gyro->newMouse(7); // Mark's crosshairs
			guideAvvy(cursorPos); // Normally, if you click on the picture, you're guiding Avvy around.
		} else
			_vm->_gyro->newMouse(4); // fletch
	}

	if (_holdLeftMouse) {
		if ((0 <= cursorPos.y) && (cursorPos.y <= 21)) { // Click on the dropdown menu.
			if (_vm->_gyro->_dropsOk)
				_vm->_dropdown->updateMenu();
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
				if (_vm->_gyro->_alive && _vm->_gyro->_dna._avvyIsAwake)
					useCompass(cursorPos);
			} else if ((208 <= cursorPos.x) && (cursorPos.x <= 260)) { // Examine the _thing.
				do {
					_vm->updateEvents();
				} while (_holdLeftMouse);

				if (_vm->_gyro->_thinkThing) {
					_vm->_acci->_thing = _vm->_gyro->_thinks;
					_vm->_acci->_thing += 49;
					_vm->_acci->_person = _vm->_acci->kPardon;
				} else {
					_vm->_acci->_person = _vm->_gyro->_thinks;
					_vm->_acci->_thing = _vm->_acci->kPardon;
				}
				callVerb(_vm->_acci->kVerbCodeExam);
			} else if ((261 <= cursorPos.x) && (cursorPos.x <= 319)) { // Display the score.
				do {
					_vm->updateEvents();
				} while (_holdLeftMouse);

				callVerb(_vm->_acci->kVerbCodeScore);
			} else if ((320 <= cursorPos.x) && (cursorPos.x <= 357)) { // Change speed.
				_vm->_animation->tr[0].xs = _vm->_gyro->kWalk;
				_vm->_animation->newspeed();
			} else if ((358 <= cursorPos.x) && (cursorPos.x <= 395)) { // Change speed.
				_vm->_animation->tr[0].xs = _vm->_gyro->kRun;
				_vm->_animation->newspeed();
			} else if ((396 <= cursorPos.x) && (cursorPos.x <= 483))
				fxToggle();
			else if ((535 <= cursorPos.x) && (cursorPos.x <= 640))
				_vm->_gyro->_mouseText = Common::String(13) + _vm->_gyro->_mouseText;
		} else if (!_vm->_gyro->_dropsOk)
			_vm->_gyro->_mouseText = Common::String(13) + _vm->_gyro->_mouseText;
	}
}

void Lucerna::errorLed() {
	warning("STUB: Lucerna::errorled()");
}

int8 Lucerna::fades(int8 x) {
	warning("STUB: Lucerna::fades()");
	return 0;
}

void Lucerna::fadeOut(byte n) {
	warning("STUB: Lucerna::fadeOut()");
}

void Lucerna::dusk() {
	warning("STUB: Lucerna::dusk()");
}

void Lucerna::fadeIn(byte n) {
	warning("STUB: Lucerna::fadeIn()");
}

void Lucerna::dawn() {
	warning("STUB: Lucerna::dawn()");
}

void Lucerna::drawDirection() { // It's data is loaded in load_digits().
	if (_vm->_gyro->_oldDirection == _vm->_gyro->_dna._direction)
		return;

	_vm->_gyro->_oldDirection = _vm->_gyro->_dna._direction;

	CursorMan.showMouse(false);

	_vm->_graphics->drawPicture(_vm->_graphics->_surface, _vm->_gyro->_directions[_vm->_gyro->_dna._direction], 0, 161);

	CursorMan.showMouse(true);
}


void Lucerna::gameOver() {
	_vm->_gyro->_dna._userMovesAvvy = false;

	int16 sx = _vm->_animation->tr[0]._x;
	int16 sy = _vm->_animation->tr[0]._y;

	_vm->_animation->tr[0].done();
	_vm->_animation->tr[0].init(12, true, _vm->_animation); // 12 = Avalot falls
	_vm->_animation->tr[0].step = 0;
	_vm->_animation->tr[0].appear(sx, sy, 0);

	_vm->_timeout->set_up_timer(3, _vm->_timeout->procavalot_falls, _vm->_timeout->reason_falling_over);
	_vm->_gyro->_alive = false;
}

void Lucerna::minorRedraw() {
	dusk();

	enterRoom(_vm->_gyro->_dna._room, 0); // Ped unknown or non-existant.

	for (byte i = 0; i <= 1; i++) {
		_vm->_gyro->_cp = 1 - _vm->_gyro->_cp;
		_vm->_animation->getback();
	}

	for (byte i = 0; i < 3; i++)
		_vm->_gyro->_scoreToDisplay[i] = -1; // impossible digits
	drawScore();

	dawn();
}

void Lucerna::majorRedraw() {
	warning("STUB: Lucerna::major_redraw()");
}

uint16 Lucerna::bearing(byte whichPed) {
	byte pedId = whichPed - 1; // Different array indexes in Pascal and C.

	const double rad2deg = 180 / 3.14; // Pi

	if (_vm->_animation->tr[0]._x == _vm->_gyro->_peds[pedId]._x)
		return 0;
	else if (_vm->_animation->tr[0]._x < _vm->_gyro->_peds[pedId]._x) {
		return (uint16)((atan(double((_vm->_animation->tr[0]._y - _vm->_gyro->_peds[pedId]._y))
			/ (_vm->_animation->tr[0]._x - _vm->_gyro->_peds[pedId]._x)) * rad2deg) + 90);
	} else {
		return (uint16)((atan(double((_vm->_animation->tr[0]._y - _vm->_gyro->_peds[pedId]._y))
			/ (_vm->_animation->tr[0]._x - _vm->_gyro->_peds[pedId]._x)) * rad2deg) + 270);
	}
}

void Lucerna::spriteRun() {
	_vm->_gyro->_doingSpriteRun = true;

	_vm->_animation->get_back_loretta();
	_vm->_animation->animLink();

	_vm->_gyro->_doingSpriteRun = false;
}

void Lucerna::fixFlashers() {
	_vm->_gyro->_ledStatus = 177;
	_vm->_gyro->_oldDirection = 177;
	_vm->_scrolls->setReadyLight(2);
	drawDirection();
}

} // End of namespace Avalanche
