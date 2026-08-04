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

#include "audio/audiostream.h"
#include "audio/decoders/mac_snd.h"
#include "common/textconsole.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/sound/mac_sound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

MacSoundDriver::MacSoundDriver(Audio::Mixer *mixer, MacResourceProvider *resources, int section) :
		SoundDriver(mixer), _resources(resources), _section(section) {
}

void MacSoundDriver::setPaused(bool paused) {
	if (_paused == paused)
		return;
	_paused = paused;
	for (uint i = 0; i < _handles.size(); ++i) {
		if (_mixer->isSoundHandleActive(_handles[i]))
			_mixer->pauseHandle(_handles[i], paused);
	}
}

void MacSoundDriver::discardFinishedSounds() {
	for (uint i = _handles.size(); i > 0; --i) {
		if (!_mixer->isSoundHandleActive(_handles[i - 1]))
			_handles.remove_at(i - 1);
	}
}

int MacSoundDriver::command(int commandId, int param) {
	switch (commandId) {
	case 0:
		return stop();
	case 6:
		setPaused(true);
		return 0;
	case 7:
		setPaused(false);
		return 0;
	case 8:
		for (uint i = 0; i < _handles.size(); ++i) {
			if (_mixer->isSoundHandleActive(_handles[i]))
				return 1;
		}
		return 0;
	default:
		break;
	}

	Common::SeekableReadStream *resource = _resources->openSound(_section, commandId);
	if (!resource)
		return 0;

	Audio::SeekableAudioStream *stream = Audio::makeMacSndStream(resource, DisposeAfterUse::YES);
	if (!stream) {
		delete resource;
		warning("Could not decode Macintosh sound %d for section %d", commandId, _section);
		return 0;
	}

	discardFinishedSounds();
	_handles.push_back(Audio::SoundHandle());

	// The DOS driver's second parameter has command-specific meanings. The
	// Macintosh resources contain their final sampled output, so do not guess
	// at a pan or gain conversion until the native call sites are recovered.
	(void)param;
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handles.back(), stream,
		-1, _volume);
	if (_paused)
		_mixer->pauseHandle(_handles.back(), true);
	return 0;
}

int MacSoundDriver::stop() {
	for (uint i = 0; i < _handles.size(); ++i) {
		if (_mixer->isSoundHandleActive(_handles[i]))
			_mixer->stopHandle(_handles[i]);
	}
	_handles.clear();
	_paused = false;
	return 0;
}

void MacSoundDriver::setVolume(int volume) {
	_volume = CLIP<int>(volume, 0, Audio::Mixer::kMaxChannelVolume);
	for (uint i = 0; i < _handles.size(); ++i) {
		if (_mixer->isSoundHandleActive(_handles[i]))
			_mixer->setChannelVolume(_handles[i], (byte)_volume);
	}
}

void MacSoundManager::loadDriver(int sectionNum) {
	removeDriver();
	_driver = new MacSoundDriver(_mixer, _resources, sectionNum);
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
