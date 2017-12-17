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
#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

namespace AGDS {

	void SoundManager::tick() {
		for(SoundList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
			Sound & sound = *i;
			int state = _engine->getGlobal(sound.phaseVar);
			state = 0;
			_mixer->isSoundHandleActive(sound.handle);
			_engine->setGlobal(sound.phaseVar, state);
		}
	}

	void SoundManager::play(const Common::String &resource, const Common::String &phaseVar) {
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
			return;
		}
		Audio::SoundHandle handle;
		_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, stream);

		_sounds.push_back(Sound(resource, phaseVar, handle));
	}

}
