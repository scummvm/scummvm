/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef GAMOS_MUSIC_H
#define GAMOS_MUSIC_H



#include "audio/mididrv.h"
#include "common/memstream.h"
#include "common/mutex.h"
#include "common/scummsys.h"
#include "common/timer.h"
#include "common/system.h"

namespace Gamos {

class MidiMusic {
private:
	MidiDriver *_driver = nullptr;
	Common::Array<byte> _pMidiData;

	Common::Mutex _mutex;

	uint32 _dataPos = 0;
	uint32 _dataStart = 0;
	int32 _midiDelayTicks = 0;
	int32 _midiDelayCount = 0;
	uint32 _midiTimeStamp = 0;
	uint32 _midiOp = 0; /* save midi event type between update cycles */
	bool _midiMute = false;

	uint8 _volume;

public:

	MidiMusic();
	~MidiMusic();

	void stopMusic();
	bool playMusic(Common::Array<byte> *midiData);
	void update();

	void setVolume(uint8 volume);

	int16 midi2low();

	static void _timerProc(void *data);
};

}

#endif //GAMOS_MUSIC_H
