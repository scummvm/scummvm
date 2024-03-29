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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_SOUND_BGATMOSPHERE_H
#define GOB_SOUND_BGATMOSPHERE_H

#include "common/array.h"
#include "common/mutex.h"
#include "common/random.h"

#include "gob/sound/sound.h"
#include "gob/sound/soundmixer.h"

namespace Audio {
class Mixer;
}

namespace Gob {

class SoundDesc;

class BackgroundAtmosphere : private SoundMixer {
public:
	BackgroundAtmosphere(Audio::Mixer &mixer);
	~BackgroundAtmosphere() override;

	void playBA();
	void stopBA();

	void setPlayMode(Sound::BackgroundPlayMode mode);

	void queueSample(SoundDesc &sndDesc);
	void queueClear();

	void setShadable(bool shadable);
	void shade();
	void unshade();

private:
	Sound::BackgroundPlayMode _playMode;

	Common::Array<SoundDesc *> _queue;
	int _queuePos;
	bool _shaded;
	bool _shadable;

	Common::Mutex _mutex;

	Common::RandomSource _rnd;

	void checkEndSample() override;
	void getNextQueuePos();
};

} // End of namespace Gob

#endif // GOB_SOUND_BGATMOSPHERE_H
