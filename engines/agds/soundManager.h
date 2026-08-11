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

#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "audio/mixer.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}
namespace Audio {
class Mixer;
}

namespace AGDS {
class AGDSEngine;

struct Sound {
	int id;
	Common::String process;
	Common::String resource;
	Common::String filename;
	Common::String phaseVar;
	Audio::SoundHandle handle;
	int volume;
	int pan;
	int group;
	bool paused;
	Sound(int id_, const Common::String &process_, const Common::String &res, const Common::String &filename_, const Common::String &var, int volume_, int pan_, int group_ = 0) : id(id_), process(process_), resource(res), filename(filename_), phaseVar(var), handle(), volume(volume_), pan(pan_), group(group_), paused(false) {
	}

	int leftVolume() const {
		return pan < 0 ? volume : volume * (100 - pan) / 100;
	}

	int rightVolume() const {
		return pan < 0 ? volume * (100 + pan) / 100 : volume;
	}
};

class SoundManager {
	using SoundList = Common::List<Sound>;
	int _nextId;
	AGDSEngine *_engine;
	Audio::Mixer *_mixer;
	SoundList _sounds;

public:
	SoundManager(AGDSEngine *engine, Audio::Mixer *mixer) : _nextId(1), _engine(engine), _mixer(mixer) {}
	void tick();
	int play(Common::String process, const Common::String &resource, const Common::String &filename, const Common::String &phaseVar, bool startPlaying, int volume, int pan, int id = -1, bool ambient = false);
	bool playing(int id) const;
	void stopAllFrom(const Common::String &process);
	void stopAll();
	const Sound *find(int id) const;
	Sound *findSampleByPhaseVar(const Common::String &phaseVar);
};

} // End of namespace AGDS

#endif /* AGDS_SOUND_MANAGER_H */
