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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* VISA		The new Sez handler. (Replaces Access.) */

#include "avalanche/avalanche.h"

#include "avalanche/visa2.h"
#include "avalanche/gyro2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/acci2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/animation.h"

#include "common/textconsole.h"

namespace Avalanche {

Visa::Visa(AvalancheEngine *vm) {
	_vm = vm;
	noError = true;
}

void Visa::unSkrimble() {
	for (uint16  i = 0; i < _vm->_gyro->_bufSize; i++)
		_vm->_gyro->_buffer[i] = (~(_vm->_gyro->_buffer[i] - (i + 1))) % 256;
}

void Visa::doTheBubble() {
	_vm->_gyro->_bufSize++;
	_vm->_gyro->_buffer[_vm->_gyro->_bufSize - 1] = 2;
}

void Visa::displayScrollChain(char block, byte point, bool report, bool bubbling) {
	Common::File indexfile, sezfile;
	uint16 idx_offset, sez_offset;
	bool error = false;

	if (!indexfile.open("avalot.idx")) {
		warning("AVALANCHE: Visa: File not found: avalot.idx");
		return;
	}

	indexfile.seek((toupper(block) - 65) * 2);
	idx_offset = indexfile.readUint16LE();
	if (idx_offset == 0)
		error = true;

	indexfile.seek(idx_offset + point * 2);
	sez_offset = indexfile.readUint16LE();
	if (sez_offset == 0)
		error = true;

	indexfile.close();

	noError = !error;

	if (error) {
		if (report) {
			Common::String todisplay;
			todisplay.format("%cError accessing scroll %c%s", 7, block, _vm->_gyro->intToStr(point).c_str());
			_vm->_scrolls->displayText(todisplay);
		}
		return;
	}

	if (!sezfile.open("avalot.sez")) {
		warning("AVALANCHE: Visa: File not found: avalot.sez");
		return;
	}
	sezfile.seek(sez_offset);
	_vm->_gyro->_bufSize = sezfile.readUint16LE();
	sezfile.read(_vm->_gyro->_buffer, _vm->_gyro->_bufSize);
	sezfile.close();
	unSkrimble();

	if (bubbling)
		doTheBubble();

	_vm->_scrolls->callScrollDriver();
}

void Visa::speak(byte who, byte subject) {
	Common::File indexfile, sezfile;

	if (subject == 0) { // No subject.
		displayScrollChain('s', who, false, true);
	} else { // Subject given.
		noError = false; // Assume that until we know otherwise.

		if (!indexfile.open("converse.avd")) {
			warning("AVALANCHE: Visa: File not found: converse.avd");
			return;
		}

		indexfile.seek(who * 2 - 2);
		uint16 idx_offset = indexfile.readUint16LE();
		uint16 next_idx_offset = indexfile.readUint16LE();

		if ((idx_offset == 0) || ((((next_idx_offset - idx_offset) / 2) - 1) < subject))
			return;

		indexfile.seek(idx_offset + subject * 2);
		uint16 sez_offset = indexfile.readUint16LE();
		if ((sez_offset == 0) || (indexfile.err()))
			return;
		indexfile.close();

		if (!sezfile.open("avalot.sez")) {
			warning("AVALANCHE: Visa: File not found: avalot.sez");
			return;
		}
		sezfile.seek(sez_offset);
		_vm->_gyro->_bufSize = sezfile.readUint16LE();
		sezfile.read(_vm->_gyro->_buffer, _vm->_gyro->_bufSize);
		sezfile.close();

		unSkrimble();
		doTheBubble();

		_vm->_scrolls->callScrollDriver();
		noError = true;
	}
}

void Visa::talkTo(byte whom) {
	if (_vm->_acci->_person == _vm->_acci->kPardon) {
		_vm->_acci->_person = _vm->_gyro->_subjectNum;
		_vm->_gyro->_subjectNum = 0;
	}

	if (_vm->_gyro->_subjectNum == 0)
		switch (whom) {
		case Gyro::kPeopleSpludwick:
			if ((_vm->_gyro->_dna._lustieIsAsleep) & (!_vm->_gyro->_dna._objects[_vm->_gyro->kObjectPotion - 1])) {
				displayScrollChain('q', 68);
				_vm->_gyro->_dna._objects[_vm->_gyro->kObjectPotion - 1] = true;
				_vm->_lucerna->refreshObjectList();
				_vm->_lucerna->incScore(3);
				return;
			} else {
				if (_vm->_gyro->_dna._talkedToCrapulus)
					// Spludwick - what does he need?
					// 0 - let it through to use normal routine.
					switch (_vm->_gyro->_dna._givenToSpludwick) {
					case 1: // Falltrough is intended.
					case 2:{
						_vm->_scrolls->displayText(Common::String("Can you get me ") + _vm->_gyro->getItem(_vm->_gyro->kSpludwicksOrder[_vm->_gyro->_dna._givenToSpludwick]) + ", please?" + _vm->_scrolls->kControlRegister + '2' + _vm->_scrolls->kControlSpeechBubble);
						return;
						}
						break;
					case 3: {
						displayScrollChain('q', 30); // Need any help with the game?
						return;
						}
						break;
				}
				else {
					displayScrollChain('q', 42); // Haven't talked to Crapulus. Go and talk to him.
					return;
				}
			}
			break;
		case Gyro::kPeopleIbythneth:
			if (_vm->_gyro->_dna._givenBadgeToIby) {
				displayScrollChain('q', 33); // Thanks a lot!
				return; // And leave the proc.
			}
			break; // Or... just continue, 'cos he hasn't got it.
		case Gyro::kPeopleDogfood:
			if (_vm->_gyro->_dna._wonNim) { // We've won the game.
				displayScrollChain('q', 6); // "I'm Not Playing!"
				return; // Zap back.
			} else
				_vm->_gyro->_dna._askedDogfoodAboutNim = true;
			break;
		case Gyro::kPeopleAyles:
			if (!_vm->_gyro->_dna._aylesIsAwake) {
				displayScrollChain('q', 43); // He's fast asleep!
				return;
			} else if (!_vm->_gyro->_dna._givenPenToAyles) {
				displayScrollChain('q', 44); // Can you get me a pen, Avvy?
				return;
			}
			break;

		case Gyro::kPeopleJacques: {
			displayScrollChain('q', 43);
			return;
			}
		case Gyro::kPeopleGeida:
			if (_vm->_gyro->_dna._givenPotionToGeida)
				_vm->_gyro->_dna._geidaFollows = true;
			else {
				displayScrollChain('u', 17);
				return;
			}
			break;
		case Gyro::kPeopleSpurge:
			if (!_vm->_gyro->_dna._sittingInPub) {
				displayScrollChain('q', 71); // Try going over and sitting down.
				return;
			} else {
				if (_vm->_gyro->_dna._spurgeTalkCount < 5)
					_vm->_gyro->_dna._spurgeTalkCount++;
				if (_vm->_gyro->_dna._spurgeTalkCount > 1) { // no. 1 falls through
					displayScrollChain('q', 70 + _vm->_gyro->_dna._spurgeTalkCount);
					return;
				}
			}
			break;
	}
	// On a subject. Is there any reason to block it?
	else if ((whom == _vm->_gyro->kPeopleAyles) && (!_vm->_gyro->_dna._aylesIsAwake)) {
			displayScrollChain('q', 43); // He's fast asleep!
			return;
		}

	if (whom > 149)
		whom -= 149;

	bool noMatches = true;
	for (int16 i = 0; i <= _vm->_animation->kSpriteNumbMax; i++)
		if (_vm->_animation->tr[i]._stat.accinum == whom) {
			_vm->_scrolls->displayText(Common::String(_vm->_scrolls->kControlRegister) + (i + 49) + _vm->_scrolls->kControlToBuffer);
			noMatches = false;
			break;
		}

	if (noMatches)
		_vm->_scrolls->displayText(Common::String(_vm->_scrolls->kControlRegister) + _vm->_scrolls->kControlRegister + _vm->_scrolls->kControlToBuffer);

	speak(whom, _vm->_gyro->_subjectNum);

	if (!noError)
		displayScrollChain('n', whom); // File not found!

	if ((_vm->_gyro->_subjectNum == 0) && ((whom + 149) == _vm->_gyro->kPeopleCrapulus)) { // Crapulus: get the badge - first time only
		_vm->_gyro->_dna._objects[_vm->_gyro->kObjectBadge - 1] = true;
		_vm->_lucerna->refreshObjectList();
		displayScrollChain('q', 1); // Circular from Cardiff.
		_vm->_gyro->_dna._talkedToCrapulus = true;

		_vm->_gyro->_whereIs[_vm->_gyro->kPeopleCrapulus - 150] = 177; // Crapulus walks off.

		_vm->_animation->tr[1]._vanishIfStill = true;
		_vm->_animation->tr[1].walkTo(3); // Walks away.

		_vm->_lucerna->incScore(2);
	}
}

} // End of namespace Avalanche.
