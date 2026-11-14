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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_MEDIA_PLAYBACK_PAUSE_H
#define RIPPER_MEDIA_PLAYBACK_PAUSE_H

#include "audio/mixer.h"
#include "common/noncopyable.h"
#include "video/video_decoder.h"

namespace Ripper {

class ScopedPlaybackPause : private Common::NonCopyable {
public:
	ScopedPlaybackPause(Video::VideoDecoder &decoder, Audio::Mixer &mixer,
			const Audio::SoundHandle *externalAudio) :
			_decoder(decoder), _mixer(mixer),
			_hasExternalAudio(externalAudio && mixer.isSoundHandleActive(*externalAudio)) {
		_decoder.pauseVideo(true);
		if (_hasExternalAudio) {
			_externalAudio = *externalAudio;
			_mixer.pauseHandle(_externalAudio, true);
		}
	}

	~ScopedPlaybackPause() {
		if (_hasExternalAudio)
			_mixer.pauseHandle(_externalAudio, false);
		_decoder.pauseVideo(false);
	}

	bool hasExternalAudio() const { return _hasExternalAudio; }

private:
	Video::VideoDecoder &_decoder;
	Audio::Mixer &_mixer;
	Audio::SoundHandle _externalAudio;
	bool _hasExternalAudio;
};

} // End of namespace Ripper

#endif
