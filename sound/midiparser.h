/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef INCLUDED_MIDIPARSER
#define INCLUDED_MIDIPARSER

class MidiParser;

#include "common/scummsys.h"

class MidiDriver;

class MidiParser {
protected:
	MidiDriver *_driver;
	uint32 _timer_rate;

public:
	virtual bool loadMusic (byte *data, uint32 size) = 0;
	virtual void unloadMusic() = 0;

	void setMidiDriver (MidiDriver *driver) { _driver = driver; }
	void setTimerRate (uint32 rate) { _timer_rate = rate / 500; }
	virtual void onTimer() = 0;

	virtual void setTrack (byte track) = 0;
	virtual void jumpToTick (uint32 tick) = 0;

	static MidiParser *createParser_SMF();
	static MidiParser *createParser_XMIDI();
};

#endif
