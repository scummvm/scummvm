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

/* BASHER		Handles the keyboard. */

// Note: this unit can record keystrokes, for the demo. If you want it
// to do this, define the o.c.c. RECORD. Setting gyro.demo to True will
// cause them to be played back. 

#ifndef BASHER2_H
#define BASHER2_H

#include "common/scummsys.h"

namespace Avalanche {
class AvalancheEngine;

class Basher {
public:
	uint16 count;



	void setParent(AvalancheEngine *vm);

	void init();

	void plottext();

	void keyboard_link();

	void cursor_on();

	void get_demorec();

	bool demo_ready();

	void cursor_off();

	void filename_edit();

	void normal_edit();

#ifdef RECORD
	void record_one();
#endif

private:
	AvalancheEngine *_vm;

	bool entering_filename;
	byte left_margin;

	void wipetext();

	void do_cursor();

	char firstchar(Common::String x);

	void try_dd();

	void typein();
};

} // End of namespace Avalanche.

#endif // BASHER2_H
