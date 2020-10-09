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
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "engines/grim/debug.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/sound/aifftrack.h"

namespace Grim {

AIFFTrack::AIFFTrack(Audio::Mixer::SoundType soundType) {
	_soundType = soundType;
	_looping = false;
	// A preloaded AIFF track may be played multiple times, so we don't
	// want to dispose after playing. The destructor of SoundTrack will
	// take care of disposing the stream instead.
	_disposeAfterPlaying = DisposeAfterUse::NO;
}

AIFFTrack::~AIFFTrack() {
	stop();
	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		delete _handle;
	}
}

bool AIFFTrack::openSound(const Common::String &filename, const Common::String &soundName, const Audio::Timestamp *start) {
	Common::SeekableReadStream *file = g_resourceloader->openNewStreamFile(filename, true);
	if (!file) {
		Debug::debug(Debug::Sound, "Stream for %s not open", soundName.c_str());
		return false;
	}
	_soundName = soundName;
	Audio::RewindableAudioStream *aiffStream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(aiffStream);
	_stream = aiffStream;
	if (start)
		seekStream->seek(*start);
	if (!_stream)
		return false;
	_handle = new Audio::SoundHandle();
	return true;
}

void AIFFTrack::setLooping(bool looping) {
	if (_looping == looping)
		return;
	_looping = looping;
	if (looping && _stream) {
		_stream = Audio::makeLoopingAudioStream(dynamic_cast<Audio::SeekableAudioStream *>(_stream), 0);
	}
}

bool AIFFTrack::play() {
	if (_stream) {
		Audio::RewindableAudioStream *stream = dynamic_cast<Audio::RewindableAudioStream *>(_stream);
		if (!_looping) {
			stream->rewind();
		}
		return SoundTrack::play();
	}
	return false;
}

bool AIFFTrack::isPlaying() {
	if (!_handle)
		return false;

	return g_system->getMixer()->isSoundHandleActive(*_handle);
}

Audio::Timestamp AIFFTrack::getPos() {
	// FIXME: Return actual stream position.
	return g_system->getMixer()->getSoundElapsedTime(*_handle);
}

} // end of namespace Grim
