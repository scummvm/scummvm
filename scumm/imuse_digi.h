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

#include "common/scummsys.h"
#include "scumm/music.h"
#include "sound/mixer.h"

namespace Scumm {

#define MAX_DIGITAL_CHANNELS 8
#define MAX_IMUSE_JUMPS 1
#define MAX_IMUSE_REGIONS 3

class ScummEngine;

class IMuseDigital : public MusicEngine {
private:

	struct Channel {
		int8 _volumeRight;
		int8 _volume;
		int8 _volumeFade;
		int8 _volumeFadeParam;
		int8 _volumeFadeStep;
		int _delay;
		int32 _offsetStop;
		int32 _offset;
		byte *_data;
		int _freq;
		int _channels;
		int _bits;
		int32 _size;
		int _idSound;
		int32 _mixerSize;
		int _mixerFlags;
		PlayingSoundHandle _mixerChannel;
		bool _used;
		bool _toBeRemoved;
	};
	
	Channel _channel[MAX_DIGITAL_CHANNELS];

	ScummEngine *_scumm;
	bool _pause;

	static void timer_handler(void *engine);
	void musicTimer();

public:
	IMuseDigital(ScummEngine *scumm);
	~IMuseDigital();

	void setMasterVolume(int vol) {}

	void startSound(int sound);
	void stopSound(int sound);
	void stopAllSounds();
	void pause(bool pause);
	int32 doCommand(int a, int b, int c, int d, int e, int f, int g, int h);
	int getSoundStatus(int sound) const;
};

} // End of namespace Scumm

#endif
