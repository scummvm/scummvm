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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_MUSIC_H
#define XEEN_MUSIC_H

#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "common/mutex.h"
#include "common/queue.h"

namespace OPL {
	class OPL;
}

namespace Xeen {

struct RegisterValue {
	uint8 _regNum;
	uint8 _value;

	RegisterValue(int regNum, int value) {
		_regNum = regNum; _value = value;
	}
};

class Music {
private:
	OPL::OPL *_opl;
	Common::Mutex _driverMutex;
	Common::Queue<RegisterValue> _queue;
	const byte *_effectsData;
	Common::Array<uint16> _effectsOffsets;
private:
	/**
	 * Loads effects data that was embedded in the music driver
	 */
	void loadEffectsData();

	/**
	 * Timer function for OPL
	 */
	void onTimer();

	/**
	 * Flushes any pending writes to the OPL
	 */
	void flush();

	/**
	 * Updates any playing music
	 */
	void update();
protected:
	Audio::Mixer *_mixer;
public:
	Music(Audio::Mixer *mixer);
	~Music();
};

} // End of namespace Xeen

#endif /* XEEN_MUSIC_H */
