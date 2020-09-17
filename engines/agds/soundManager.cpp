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
	for (SoundList::iterator i = _sounds.begin(); i != _sounds.end();) {
		Sound &sound = *i;
		if (!_mixer->isSoundHandleActive(sound.handle)) {
			//FIXME: re-enable me later
			// if (!sound.phaseVar.empty())
			// 	_engine->setGlobal(sound.phaseVar, -1);
			i = _sounds.erase(i);
		} else {
			// if (!sound.phaseVar.empty())
			// 	_engine->setGlobal(sound.phaseVar, _engine->getGlobal(sound.phaseVar) + 1);
			++i;
		}
	}
}

void SoundManager::stopAll() {
	_mixer->stopAll();
	for (SoundList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		Sound &sound = *i;
		_engine->setGlobal(sound.phaseVar, 0);
	}
}

int SoundManager::play(const Common::String &resource, const Common::String &phaseVar) {
	Common::File *file = new Common::File();
	if (!file->open(resource))
		error("no sound %s", resource.c_str());

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
		return -1;
	}
	Audio::SoundHandle handle;
	int id = _nextId++;
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, stream, id);

	_sounds.push_back(Sound(id, resource, phaseVar, handle));
	return id;
}

} // namespace AGDS
