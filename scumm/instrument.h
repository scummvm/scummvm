/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "stdafx.h"
#include "common/scummsys.h"

#define NATIVE_MT32 false

class Serializer;
class MidiChannel;

class InstrumentInternal {
public:
	virtual void saveOrLoad (Serializer *s) = 0;
	virtual void send (MidiChannel *mc) = 0;
};

class Instrument {
private:
	byte _type;
	InstrumentInternal *_instrument;

public:
	enum {
		itNone = 0,
		itProgram = 1,
		itAdlib = 2,
		itRoland = 3
	};

	Instrument() : _type (0), _instrument (0) { }

	void clear();
	void program (byte program, bool mt32);
	void adlib (byte *instrument);
	void roland (byte *instrument);

	byte getType() { return _type; }
	void saveOrLoad (Serializer *s);
	void send (MidiChannel *mc) { if (_instrument) _instrument->send (mc); }
};

#endif
