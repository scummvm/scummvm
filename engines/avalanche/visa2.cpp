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

	void Visa::setParent(AvalancheEngine *vm) {
		_vm = vm;
	}

	const bool Visa::bubbling = false;
	const bool Visa::report_dixi_errors = true;

	void Visa::unskrimble() {
		for (uint16  fv = 0; fv < _vm->_gyro.bufsize; fv++) 
			_vm->_gyro.buffer[fv] = (char)((!(_vm->_gyro.buffer[fv]) - fv) % 256);
	}

	void Visa::do_the_bubble() {
		_vm->_gyro.bufsize++;
		_vm->_gyro.buffer[_vm->_gyro.bufsize] = 2;
	}

	// File handling.
	void Visa::dixi(char block, byte point) {
		warning("STUB: Visa::dixi()");
	}

	// File handling.
	void Visa::speech(byte who, byte subject) {
		warning("STUB: Visa::speech()");
	}

	void Visa::talkto(byte whom) {
		byte fv;
		bool no_matches;

		warning("STUB: Visa::talkto()");
	}

} // End of namespace Avalanche.
