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
	for (SoundList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		Sound &sound = *i;
		auto &phaseVar = sound.phaseVar;

		bool active = _mixer->isSoundHandleActive(sound.handle);
		if (!sound.phaseVar.empty()) {
			int value = _engine->getGlobal(sound.phaseVar);
			if (value <= 1) {
				if (value == 1 && !active)
					_engine->setGlobal(phaseVar, 0);
			} else if (value & 2) {
				debug("sample %s restarts (via phase var)", sound.name.c_str());
				_engine->setGlobal(phaseVar, 1);
				_mixer->stopID(sound.id);
				play(sound.process, sound.name, sound.phaseVar, true, sound.id);
			} else if (value & 4) {
				debug("sample %s stops (via phase var)", sound.name.c_str());
				_mixer->stopID(sound.id);
				_engine->setGlobal(phaseVar, 0);
			}
		} else if (!active)
			_engine->reactivate(sound.process);
	}
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

int SoundManager::play(const Common::String &process, const Common::String &resource, const Common::String &phaseVar, bool startPlaying, int id) {
	debug("SoundMan::play %s %s %s %d %d", process.c_str(), resource.c_str(), phaseVar.c_str(), startPlaying, id);
	if (resource.empty())
		return -1;

	{
		auto sample = findSampleByPhaseVar(phaseVar);
		if (sample && playing(sample->id)) {
			debug("already playing");
			return sample->id;
		}
	}
	Common::File *file = new Common::File();
	if (!file->open(resource)) {
		if (!phaseVar.empty())
			_engine->setGlobal(phaseVar, 0);
		warning("no sound %s", resource.c_str());
		return -1;
	}

	Common::String lname(resource);
	lname.toLowercase();

	Audio::SeekableAudioStream *stream = NULL;
	if (lname.hasSuffix(".ogg")) {
		stream = Audio::makeVorbisStream(file, DisposeAfterUse::YES);
	} else if (lname.hasSuffix(".wav")) {
		stream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	}
	if (!stream) {
		warning("could not play sound %s", resource.c_str());
		delete file;
		if (!phaseVar.empty())
			_engine->setGlobal(phaseVar, 0);
		else
			_engine->reactivate(process);
		return -1;
	}
	Audio::SoundHandle handle;
	if (id == -1)
		id = _nextId++;

	_sounds.push_back(Sound(id, process, resource, phaseVar, handle));
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, stream, id);
	//if (sound_off)
	//	setPhaseVar(_sounds.back(), 1);
	return id;
}

bool SoundManager::playing(int id) const {
	return _mixer->isSoundIDActive(id);
}

} // namespace AGDS
