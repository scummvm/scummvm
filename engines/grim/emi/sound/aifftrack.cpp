/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/mutex.h"
#include "common/textconsole.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/sound/aifftrack.h"

namespace Grim {

AIFFTrack::AIFFTrack(Audio::Mixer::SoundType soundType, DisposeAfterUse::Flag disposeOfStream) {
	_soundType = soundType;
	_looping = false;
	_disposeAfterPlaying = disposeOfStream;
}

AIFFTrack::~AIFFTrack() {
	stop();
	delete _handle;
}
	
bool AIFFTrack::openSound(Common::String soundName, Common::SeekableReadStream *file) {
	if (!file) {
		warning("Stream for %s not open", soundName.c_str());
		return false;
	}
	_soundName = soundName;
	_stream = Audio::makeAIFFStream(file, DisposeAfterUse::NO);
	if (!_stream)
		return false;
	_handle = new Audio::SoundHandle();
	return true;
}

void AIFFTrack::setLooping(bool looping) {
	_looping = looping;
	if (looping && _stream) {
		_stream = Audio::makeLoopingAudioStream(static_cast<Audio::SeekableAudioStream *>(_stream), 0);
	}
}

bool AIFFTrack::play() {
	if (_stream) {
		Audio::RewindableAudioStream *stream = static_cast<Audio::RewindableAudioStream *>(_stream);
		if (!_looping) {
			stream->rewind();
		}
		return SoundTrack::play();
	}
	return false;
}

} // end of namespace Grim
