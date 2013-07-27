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
		_vm->_gyro->buffer[fv] = (char)((!(_vm->_gyro->buffer[fv]) - fv) % 256);
}

void Visa::do_the_bubble() {
	_vm->_gyro->bufsize++;
	_vm->_gyro->buffer[_vm->_gyro->bufsize] = 2;
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

	if (bubbling)  do_the_bubble();

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

	warning("STUB: Visa::talkto()");
}

} // End of namespace Avalanche.
