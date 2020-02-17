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

#include "bladerunner/music.h"

#include "bladerunner/audio_mixer.h"
#include "bladerunner/aud_stream.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/savefile.h"
#include "bladerunner/game_constants.h"

#include "common/timer.h"
namespace BladeRunner {

Music::Music(BladeRunnerEngine *vm) {
	_vm = vm;
	_channel = -1;
	_musicVolume = BLADERUNNER_ORIGINAL_SETTINGS ? 65 : 100;
	_isPlaying = false;
	_isPaused = false;
	_current.loop = false;
	_isNextPresent = false;
	_data = nullptr;
	_stream = nullptr;
}

Music::~Music() {
	stop(0);
	while (isPlaying()) {
		// wait for the mixer to finish
	}

	_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
	_vm->getTimerManager()->removeTimerProc(timerCallbackNext);
}

bool Music::play(const Common::String &trackName, int volume, int pan, int32 timeFadeIn, int32 timePlay, int loop, int32 timeFadeOut) {
	//Common::StackLock lock(_mutex);

	if (_musicVolume <= 0) {
		return false;
	}

	int volumeAdjusted = volume * _musicVolume / 100;
	int volumeStart = volumeAdjusted;
	if (timeFadeIn > 0) {
		volumeStart = 1;
	}

	if (isPlaying()) {
		if (!_current.name.equalsIgnoreCase(trackName)) {
			_next.name = trackName;
			_next.volume = volume;
			_next.pan = pan;
			_next.timeFadeIn = timeFadeIn;
			_next.timePlay = timePlay;
			_next.loop = loop;
			_next.timeFadeOut = timeFadeOut;
			if (_isNextPresent) {
				stop(2);
			}
			_isNextPresent = true;
		} else {
			_current.loop = loop;
			if (timeFadeIn < 0) {
				timeFadeIn = 0;
			}
			adjustVolume(volumeAdjusted, timeFadeIn);
			adjustPan(volumeAdjusted, timeFadeIn);
		}
		return true;
	}

	_data = getData(trackName);
	if (_data == nullptr) {
		return false;
	}
	_stream = new AudStream(_data);

	_isNextPresent = false;
	_channel = _vm->_audioMixer->playMusic(_stream, volumeStart, mixerChannelEnded, this, _stream->getLength());
	if (_channel < 0) {
		delete _stream;
		_stream = nullptr;
		delete[] _data;
		_data = nullptr;

		return false;
	}
	if (timeFadeIn > 0) {
		adjustVolume(volumeAdjusted, timeFadeIn);
	}
	_current.name = trackName;
	if (timePlay > 0) {
		_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
		_vm->getTimerManager()->installTimerProc(timerCallbackFadeOut, timePlay * 1000 * 1000, this, "BladeRunnerMusicFadeoutTimer");
	} else if (timeFadeOut > 0) {
		_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
		_vm->getTimerManager()->installTimerProc(timerCallbackFadeOut, (_stream->getLength() - timeFadeOut * 1000) * 1000, this, "BladeRunnerMusicFadeoutTimer");
	}
	_isPlaying = true;
	_current.volume = volume;
	_current.pan = pan;
	_current.timeFadeIn = timeFadeIn;
	_current.timePlay = timePlay;
	_current.loop = loop;
	_current.timeFadeOut = timeFadeOut;
	return true;
}

void Music::stop(uint32 delay) {
	Common::StackLock lock(_mutex);

	if (_channel < 0) {
		return;
	}

#if !BLADERUNNER_ORIGINAL_BUGS
	// In original game, on queued music was not removed and it started playing after actor left the scene
	_isNextPresent = false;
#endif

	_current.loop = false;
	_vm->_audioMixer->stop(_channel, 60u * delay);
}

void Music::adjust(int volume, int pan, uint32 delay) {
	if (volume != -1) {
		adjustVolume(_musicVolume * volume/ 100, delay);
	}
	if (pan != -101) {
		adjustPan(pan, delay);
	}
}

bool Music::isPlaying() {
	return _channel >= 0 && _isPlaying;
}

void Music::setVolume(int volume) {
	_musicVolume = volume;
	if (volume <= 0) {
		stop(2);
	} else if (isPlaying()) {
		_vm->_audioMixer->adjustVolume(_channel, _musicVolume * _current.volume / 100, 120);
	}
}

int Music::getVolume() {
	return _musicVolume;
}

void Music::playSample() {
	if (!isPlaying()) {
		play(_vm->_gameInfo->getSfxTrack(kSfxMUSVOL8), 100, 0, 2, -1, 0, 3);
	}
}

void Music::save(SaveFileWriteStream &f) {
	f.writeBool(_isNextPresent);
	f.writeBool(_isPlaying);
	f.writeBool(_isPaused);
	f.writeStringSz(_current.name, 13);
	f.writeInt(_current.volume);
	f.writeInt(_current.pan);
	f.writeInt(_current.timeFadeIn);
	f.writeInt(_current.timePlay);
	f.writeInt(_current.loop);
	f.writeInt(_current.timeFadeOut);
	f.writeStringSz(_next.name, 13);
	f.writeInt(_next.volume);
	f.writeInt(_next.pan);
	f.writeInt(_next.timeFadeIn);
	f.writeInt(_next.timePlay);
	f.writeInt(_next.loop);
	f.writeInt(_next.timeFadeOut);
}

void Music::load(SaveFileReadStream &f) {
	_isNextPresent = f.readBool();
	_isPlaying = f.readBool();
	_isPaused = f.readBool();
	_current.name = f.readStringSz(13);
	_current.volume = f.readInt();
	_current.pan = f.readInt();
	_current.timeFadeIn = f.readInt();
	_current.timePlay = f.readInt();
	_current.loop = f.readInt();
	_current.timeFadeOut = f.readInt();
	_next.name = f.readStringSz(13);
	_next.volume = f.readInt();
	_next.pan = f.readInt();
	_next.timeFadeIn = f.readInt();
	_next.timePlay = f.readInt();
	_next.loop = f.readInt();
	_next.timeFadeOut = f.readInt();

	stop(2);
	if (_isPlaying) {
		if (_channel == -1) {
			play(_current.name,
				_current.volume,
				_current.pan,
				_current.timeFadeIn,
				_current.timePlay,
				_current.loop,
				_current.timeFadeOut);
		} else {
			_isNextPresent = true;
			_next.name = _current.name;
			_next.volume = _current.volume;
			_next.pan = _current.pan;
			_next.timeFadeIn = _current.timeFadeIn;
			_next.timePlay = _current.timePlay;
			_next.loop = _current.loop;
			_next.timeFadeOut = _current.timeFadeOut;
		}
	}
}

void Music::adjustVolume(int volume, uint32 delay) {
	if (_channel >= 0) {
		_vm->_audioMixer->adjustVolume(_channel, volume, delay);
	}
}

void Music::adjustPan(int pan, uint32 delay) {
	if (_channel >= 0) {
		_vm->_audioMixer->adjustPan(_channel, pan, delay);
	}
}

void Music::ended() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	_channel = -1;

	delete[] _data;
	_data = nullptr;

	_vm->getTimerManager()->installTimerProc(timerCallbackNext, 100 * 1000, this, "BladeRunnerMusicNextTimer");
}

void Music::fadeOut() {
	_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
	if (_channel >= 0) {
		if (_current.timeFadeOut < 0) {
			_current.timeFadeOut = 0;
		}
		_vm->_audioMixer->stop(_channel, 60u * _current.timeFadeOut);
	}
}

void Music::next() {
	_vm->getTimerManager()->removeTimerProc(timerCallbackNext);

	if (_isNextPresent) {
		if (_isPaused) {
			_vm->getTimerManager()->installTimerProc(timerCallbackNext, 2000 * 1000, this, "BladeRunnerMusicNextTimer");
		} else {
			play(_next.name.c_str(), _next.volume, _next.pan, _next.timeFadeIn, _next.timePlay, _next.loop, _next.timeFadeOut);
		}
		_current.loop = false;
	} else if (_current.loop) {
		play(_current.name.c_str(), _current.volume, _current.pan, _current.timeFadeIn, _current.timePlay, _current.loop, _current.timeFadeOut);
	}
}

void Music::mixerChannelEnded(int channel, void *data) {
	if (data != nullptr) {
		((Music *)data)->ended();
	}
}

void Music::timerCallbackFadeOut(void *refCon) {
	((Music *)refCon)->fadeOut();
}

void Music::timerCallbackNext(void *refCon) {
	((Music *)refCon)->next();
}

byte *Music::getData(const Common::String &name) {
	// NOTE: This is not part original game, loading data is done in the mixer and its using buffering to limit memory usage
	Common::SeekableReadStream *stream = _vm->getResourceStream(name);

	if (stream == nullptr) {
		return nullptr;
	}

	uint32 size = stream->size();
	byte *data = new byte[size];
	stream->read(data, size);

	delete stream;

	return data;
}

} // End of namespace BladeRunner
