/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENMUSIC_H
#define QUEENMUSIC_H

#include "common/util.h"

class MidiDriver;
class MidiParser;

namespace Queen {

class QueenEngine;
	
class Music {
public:
	Music(MidiDriver *_driver, QueenEngine *vm);
	~Music();
	void playSong(uint16 songNum);
	void stopSong();
	void loop(bool val)	{ _loop = val; }
	
protected:
	bool _loop;
	byte *_musicData;
	uint16 _numSongs;
	uint32 _musicDataSize;
	MidiDriver *_driver;
	MidiParser *_midi;
	
	uint32 songOffset(uint16 songNum);
	uint32 songLength(uint16 songNum);
};

} // End of namespace Queen

#endif
