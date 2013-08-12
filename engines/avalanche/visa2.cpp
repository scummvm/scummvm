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
#include "avalanche/trip6.h"

#include "common/textconsole.h"


namespace Avalanche {

Visa::Visa(AvalancheEngine *vm) {
	_vm = vm;
}

bool Visa::bubbling = false;
bool Visa::report_dixi_errors = true;

void Visa::unskrimble() {
	for (uint16  fv = 0; fv < _vm->_gyro->bufsize; fv++) 
		_vm->_gyro->buffer[fv] = (~(_vm->_gyro->buffer[fv] - (fv + 1))) % 256;
}

void Visa::do_the_bubble() {
	_vm->_gyro->bufsize++;
	_vm->_gyro->buffer[_vm->_gyro->bufsize - 1] = 2;
}

void Visa::dixi(char block, byte point) {
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

	went_ok = !error;

	if (error) {
		if (report_dixi_errors) {
			Common::String todisplay;
			todisplay.format("%cError accessing scroll %c%s", 7, block, _vm->_gyro->strf(point).c_str());
			_vm->_scrolls->display(todisplay);
		}
		return;
	}

	if (!sezfile.open("avalot.sez")) {
		warning("AVALANCHE: Visa: File not found: avalot.sez");
		return;
	}
	sezfile.seek(sez_offset);
	_vm->_gyro->bufsize = sezfile.readUint16LE();
	sezfile.read(_vm->_gyro->buffer, _vm->_gyro->bufsize);
	sezfile.close();
	unskrimble();

	if (bubbling)
		do_the_bubble();

	_vm->_scrolls->calldrivers();
}

void Visa::speech(byte who, byte subject) {
	Common::File indexfile, sezfile;
	uint16 idx_offset, sez_offset, next_idx_offset;

	if (subject == 0) {
		/* No subject. */

		bubbling = true;
		report_dixi_errors = false;
		dixi('s', who);
		bubbling = false;
		report_dixi_errors = true;
	} else {
		/* Subject given. */

		went_ok = false; /* Assume that until we know otherwise. */

		if (!indexfile.open("converse.avd")) {
			warning("AVALANCHE: Visa: File not found: converse.avd");
			return;
		}

		indexfile.seek(who * 2 - 2);
		idx_offset = indexfile.readUint16LE();
		next_idx_offset = indexfile.readUint16LE();

		if ((idx_offset == 0) || ((((next_idx_offset - idx_offset) / 2) - 1) < subject))  return;

		indexfile.seek(idx_offset + subject * 2);
		sez_offset = indexfile.readUint16LE();
		if ((sez_offset == 0) || (indexfile.err()))
			return;
		indexfile.close();

		if (!sezfile.open("avalot.sez")) {
			warning("AVALANCHE: Visa: File not found: avalot.sez");
			return;
		}
		sezfile.seek(sez_offset);
		_vm->_gyro->bufsize = sezfile.readUint16LE();
		sezfile.read(_vm->_gyro->buffer, _vm->_gyro->bufsize);
		sezfile.close();

		unskrimble();
		do_the_bubble();

		_vm->_scrolls->calldrivers();
		went_ok = true;
	}

	warning("STUB: Visa::speech()");
}

void Visa::talkto(byte whom) {
	byte fv;
	bool no_matches;

	if (_vm->_acci->person == _vm->_acci->pardon) {
		_vm->_acci->person = _vm->_gyro->subjnumber;
		_vm->_gyro->subjnumber = 0;
	}

	if (_vm->_gyro->subjnumber == 0)
		switch (whom) {
		case _vm->_gyro->pspludwick:

			if ((_vm->_gyro->dna.lustie_is_asleep) & (!_vm->_gyro->dna.obj[_vm->_gyro->potion])) {
				dixi('q', 68);
				_vm->_gyro->dna.obj[_vm->_gyro->potion] = true;
				_vm->_lucerna->objectlist();
				_vm->_lucerna->points(3);
				return;
			} else {
				if (_vm->_gyro->dna.talked_to_crapulus)
					// Spludwick - what does he need?
					// 0 - let it through to use normal routine.
					switch (_vm->_gyro->dna.given2spludwick) { 
					case 1: // Falltrough is intended.
					case 2:{
						_vm->_scrolls->display(Common::String("Can you get me ") + _vm->_gyro->get_better(_vm->_gyro->spludwick_order[_vm->_gyro->dna.given2spludwick]) + ", please?" + _vm->_scrolls->kControlRegister + '2' + _vm->_scrolls->kControlSpeechBubble);
						return;
						}
						break;
					case 3: {
						dixi('q', 30); // Need any help with the game?
						return;
						}
						break;
				}
				else {
					dixi('q', 42); // Haven't talked to Crapulus. Go and talk to him.
					return;
				}
			}
			break;

		case _vm->_gyro->pibythneth:
			if (_vm->_gyro->dna.givenbadgetoiby) {
				dixi('q', 33); // Thanks a lot!
				return; // And leave the proc.
			}
			break; // Or... just continue, 'cos he hasn't got it.
		case _vm->_gyro->pdogfood:
			if (_vm->_gyro->dna.wonnim) { // We've won the game.
				dixi('q', 6); // "I'm Not Playing!"
				return; // Zap back.
			} else
				_vm->_gyro->dna.asked_dogfood_about_nim = true;
			break;
		case _vm->_gyro->payles:
			if (!_vm->_gyro->dna.ayles_is_awake) {
				dixi('q', 43); // He's fast asleep!
				return;
			} else if (!_vm->_gyro->dna.given_pen_to_ayles) {
				dixi('q', 44); // Can you get me a pen, Avvy?
				return;
			}
			break;

		case _vm->_gyro->pjacques: {
			dixi('q', 43);
			return;
			}
		case _vm->_gyro->pgeida:
			if (_vm->_gyro->dna.geida_given_potion)
				_vm->_gyro->dna.geida_follows = true;
			else {
				dixi('u', 17);
				return;
			}
			break;
		case _vm->_gyro->pspurge:
			if (~ _vm->_gyro->dna.sitting_in_pub) {
				dixi('q', 71); // Try going over and sitting down.
				return;
			} else {
				if (_vm->_gyro->dna.spurge_talk < 5)
					_vm->_gyro->dna.spurge_talk++;
				if (_vm->_gyro->dna.spurge_talk > 1) { // no. 1 falls through
					dixi('q', 70 + _vm->_gyro->dna.spurge_talk);
					return;
				}
			}
			break;
	}
	// On a subject. Is there any reason to block it?
	else if ((whom == _vm->_gyro->payles) && (!_vm->_gyro->dna.ayles_is_awake)) { 
			dixi('q', 43); // He's fast asleep!
			return;
		}

	if (whom > 149)
		whom -= 149;

	no_matches = true;
	for (fv = 0; fv <= _vm->_trip->numtr; fv++)
		if (_vm->_trip->tr[fv].a.accinum == whom) {
			_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + (fv + 49) + _vm->_scrolls->kControlToBuffer);
			no_matches = false;
			break;
		}

	if (no_matches)
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + _vm->_scrolls->kControlRegister + _vm->_scrolls->kControlToBuffer);

	speech(whom, _vm->_gyro->subjnumber);

	if (!went_ok)
		dixi('n', whom); // File not found!

	if ((_vm->_gyro->subjnumber == 0) && ((whom + 227) == _vm->_gyro->pcrapulus)) { // Crapulus: get the badge - first time only
		_vm->_gyro->dna.obj[_vm->_gyro->badge] = true;
		_vm->_lucerna->objectlist();
		dixi('q', 1); // Circular from Cardiff.
		_vm->_gyro->dna.talked_to_crapulus = true;

		_vm->_gyro->whereis[_vm->_gyro->pcrapulus] = 177; // Crapulus walks off.

		_vm->_trip->tr[1].vanishifstill = true;
		_vm->_trip->tr[1].walkto(3); // Walks away.

		_vm->_lucerna->points(2);
	}
}

} // End of namespace Avalanche.
