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

#include "agds/soundManager.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace AGDS {

void SoundManager::tick() {
	for (auto it = _sounds.begin(); it != _sounds.end();) {
		Sound &sound = *it;
		auto &phaseVar = sound.phaseVar;

		bool active = playing(sound.id);
		if (!sound.phaseVar.empty()) {
			int value = _engine->getGlobal(sound.phaseVar);
			if (value <= 1) {
				if (value == 1 && !active) {
					debug("sample %s:%s resets phase var to 0", sound.resource.c_str(), sound.filename.c_str());
					_engine->setGlobal(phaseVar, 0);
				}
			} else if (value & 2) {
				debug("sample %s:%s restarts (via phase var)", sound.resource.c_str(), sound.filename.c_str());
				_engine->setGlobal(phaseVar, 1);
				_mixer->stopID(sound.id);
				play(sound.process, sound.resource, sound.filename, sound.phaseVar, true, sound.volume, sound.pan, sound.id);
			} else if (value & 4) {
				debug("sample %s:%s stops (via phase var)", sound.resource.c_str(), sound.filename.c_str());
				_mixer->stopID(sound.id);
				_engine->setGlobal(phaseVar, 0);
			}
			++it;
		} else if (!active) {
			_engine->reactivate(sound.process, "sound " + sound.resource + " inactive", true);
			it = _sounds.erase(it);
		} else
			++it;
	}
	_engine->runPendingReactivatedProcesses();
}

const Sound *SoundManager::find(int id) const {
	for (auto i = _sounds.begin(); i != _sounds.end(); ++i) {
		auto &sound = *i;
		if (sound.id == id)
			return &sound;
	}
	return nullptr;
}

Sound *SoundManager::findSampleByPhaseVar(const Common::String &phaseVar) {
	if (phaseVar.empty())
		return nullptr;

	for (auto i = _sounds.begin(); i != _sounds.end(); ++i) {
		auto &sound = *i;
		if (sound.phaseVar == phaseVar) {
			return &sound;
		}
	}
	return nullptr;
}

void SoundManager::stopAll() {
	_mixer->stopAll();
	for (auto i = _sounds.begin(); i != _sounds.end(); ++i) {
		auto &sound = *i;
		if (!sound.phaseVar.empty())
			_engine->setGlobal(sound.phaseVar, 0);
	}
	_sounds.clear();
}

void SoundManager::stopAllFrom(const Common::String &process) {
	for (auto i = _sounds.begin(); i != _sounds.end();) {
		auto &sound = *i;
		if (sound.process == process) {
			_mixer->stopID(sound.id);
			if (!sound.phaseVar.empty())
				_engine->setGlobal(sound.phaseVar, 0);
			i = _sounds.erase(i);
		} else {
			++i;
		}
	}
}

int SoundManager::play(Common::String process, const Common::String &resource, const Common::String &filename, const Common::String &phaseVar, bool startPlaying, int volume, int pan, int id, bool ambient) {
	debug("SoundMan::play(process: '%s', resource: '%s', filename: '%s', phaseVar: '%s', start: %d, volume: %d, pan: %d, id: %d, ambient: %d", process.c_str(), resource.c_str(), filename.c_str(), phaseVar.c_str(), startPlaying, volume, pan, id, ambient);
	if (filename.empty())
		return -1;

	{
		auto sample = findSampleByPhaseVar(phaseVar);
		if (sample && playing(sample->id)) {
			debug("already playing");
			return sample->id;
		}
	}
	Common::ScopedPtr<Common::File> file(new Common::File());
	if (!file->open(Common::Path{filename})) {
		if (!phaseVar.empty())
			_engine->setGlobal(phaseVar, 1);
		warning("no sound %s", filename.c_str());
		return -1;
	}
	if (ambient)
		process.clear();

	Common::String lname(filename);
	lname.toLowercase();

	Audio::SeekableAudioStream *stream = NULL;
	if (lname.hasSuffix(".ogg")) {
		stream = Audio::makeVorbisStream(file.release(), DisposeAfterUse::YES);
	} else if (lname.hasSuffix(".wav")) {
		stream = Audio::makeWAVStream(file.release(), DisposeAfterUse::YES);
	}
	if (!stream) {
		warning("could not play sound %s", filename.c_str());
		if (!phaseVar.empty())
			_engine->setGlobal(phaseVar, _engine->getGlobal(phaseVar) ? 1 : 0);
		else
			_engine->reactivate(process, "no sound");
		return -1;
	}
	if (id == -1)
		id = _nextId++;

	_sounds.push_back(Sound(id, process, resource, filename, phaseVar, volume, pan));
	auto handle = &_sounds.back().handle;
	if (startPlaying)
		_mixer->playStream(
			ambient ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kPlainSoundType,
			&_sounds.back().handle, stream, id,
			volume * Audio::Mixer::kMaxChannelVolume / 100, pan * 127 / 100);
	if (ambient)
		_mixer->loopChannel(*handle);
	// if (sound_off)
	//	setPhaseVar(_sounds.back(), 1);
	return id;
}

bool SoundManager::playing(int id) const {
	return _mixer->isSoundIDActive(id);
}

} // namespace AGDS
