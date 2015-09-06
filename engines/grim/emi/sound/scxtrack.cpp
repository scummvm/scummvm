/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/mutex.h"
#include "common/textconsole.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "engines/grim/debug.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/sound/codecs/scx.h"
#include "engines/grim/emi/sound/scxtrack.h"

namespace Grim {

SCXTrack::SCXTrack(Audio::Mixer::SoundType soundType) {
	_disposeAfterPlaying = DisposeAfterUse::NO;
	_soundType = soundType;
	_looping = false;
}

SCXTrack::~SCXTrack() {
	stop();
	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		delete _handle;
	}
}

bool SCXTrack::openSound(const Common::String &filename, const Common::String &soundName, const Audio::Timestamp *start) {
	Common::SeekableReadStream *file = g_resourceloader->openNewStreamFile(filename);
	if (!file) {
		Debug::debug(Debug::Sound, "Stream for %s not open", soundName.c_str());
		return false;
	}
	_soundName = soundName;
	Audio::RewindableAudioStream *scxStream = makeSCXStream(file, start, DisposeAfterUse::YES);
	_stream = scxStream;
	_handle = new Audio::SoundHandle();
	return true;
}

bool SCXTrack::isPlaying() {
	if (!_handle)
		return false;

	return g_system->getMixer()->isSoundHandleActive(*_handle);
}

Audio::Timestamp SCXTrack::getPos() {
	if (!_stream || _looping)
		return Audio::Timestamp(0);
	return dynamic_cast<SCXStream*>(_stream)->getPos();
}

bool SCXTrack::play() {
	if (_stream) {
		Audio::RewindableAudioStream *stream = dynamic_cast<Audio::RewindableAudioStream *>(_stream);
		if (!_looping) {
			stream->rewind();
		}
		return SoundTrack::play();
	}
	return false;
}

void SCXTrack::setLooping(bool looping) {
	if (_looping == looping)
		return;
	_looping = looping;
	if (looping && _stream) {
		_stream = Audio::makeLoopingAudioStream(dynamic_cast<Audio::RewindableAudioStream *>(_stream), 0);
	}
}

} // end of namespace Grim
