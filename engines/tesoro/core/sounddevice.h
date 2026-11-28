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

#ifndef TESORO_CORE_SOUNDDEVICE_H
#define TESORO_CORE_SOUNDDEVICE_H

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "backends/audiocd/audiocd.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"

namespace Tesoro {

/**
 * The voice and sound effect player: at most three sounds at once, each with an id. A record stays in the active list
 * until update() finds its sound finished, so a finished voice is still found by active() until the next update().
 */
class SoundDevice {
public:
	enum { kRecords = 3 };

	explicit SoundDevice(Audio::Mixer *mixer);

	/**
	 * Returns the id of the new sound, 0 when all three records are busy.
	 */
	uint32 play(Common::SeekableReadStream &dat, uint32 archiveOffset);

	/** The sound with this id is in the active list. */
	bool active(uint32 id) const;

	void stop(uint32 id);

	/** Takes the finished sounds out of the active list. Called every frame. */
	void update();

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handles[kRecords];
	uint _order[kRecords];		// the records, active ones first
	uint32 _ids[kRecords];
	bool _loaded[kRecords];
	uint32 _count = 0;
	uint32 _lastId = 0;
};

class CdAudio {
public:
	void playTrack(uint8 track);
	int track() const { return _track; }

private:
	int _track = 0;
};

} // End of namespace Tesoro

#endif
