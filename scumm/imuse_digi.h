/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#ifndef IMUSE_DIGI_H
#define IMUSE_DIGI_H

#include "scummsys.h"

#define MAX_DIGITAL_CHANNELS 8
#define MAX_IMUSE_JUMPS 1
#define MAX_IMUSE_REGIONS 3

class Scumm;

class IMuseDigital {
private:

	struct Region {
		uint32 _offset;		// begin of region
		uint32 _length;		// length of region
	};

	struct Jump {
		uint32 _offset;		// jump position
		uint32 _dest;			// jump to 
		uint32 _id;				// id of jump
		uint32 _numLoops;	// allmost 500 except one value: 2
	};

	struct Channel {
		int8 _volumeRight;
		int8 _volume;
		int8 _volumeFade;
		int8 _volumeFadeParam;
		int8 _volumeFadeStep;
		uint32 _delay;
		bool _isJump;
		uint32 _numLoops;
		uint32 _offsetStop;
		Jump _jump[MAX_IMUSE_JUMPS];
		uint32 _numJumps;
		Region _region[MAX_IMUSE_REGIONS];
		uint32 _numRegions;
		uint32 _offset;
		byte *_data;
		uint32 _freq;
		uint32 _channels;
		uint32 _bits;
		uint32 _size;
		int32 _idSound;
		uint32 _mixerSize;
		uint8 _mixerFlags;
		int _mixerChannel;
		bool _used;
		bool _toBeRemoved;
	};
	
	Channel _channel[MAX_DIGITAL_CHANNELS];

	Scumm *_scumm;
	bool _pause;

public:
	IMuseDigital(Scumm *scumm);
	~IMuseDigital();
	void handler();
	void startSound(int sound);
	void stopSound(int sound);
	void stopAll();
	void pause(bool pause);
	int32 doCommand(int a, int b, int c, int d, int e, int f, int g, int h);
	int getSoundStatus(int sound);
};

#endif
