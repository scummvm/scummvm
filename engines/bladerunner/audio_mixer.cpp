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

#include "bladerunner/audio_mixer.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/time.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/timer.h"

namespace BladeRunner {

AudioMixer::AudioMixer(BladeRunnerEngine *vm) {
	_vm = vm;
	for (int i = 0; i < kChannels; ++i) {
		_channels[i].isPresent = false;
	}
	_vm->getTimerManager()->installTimerProc(timerCallback, (1000 / kUpdatesPerSecond) * 1000, this, "BladeRunnerAudioMixerTimer");
}

AudioMixer::~AudioMixer() {
	for (int i = 0; i < kChannels; ++i) {
		stop(i, 0);
	}
	_vm->getTimerManager()->removeTimerProc(timerCallback);
}

int AudioMixer::play(Audio::Mixer::SoundType type, Audio::RewindableAudioStream *stream, int priority, bool loop, int volume, int pan, void (*endCallback)(int, void *), void *callbackData, uint32 trackDurationMs) {
	Common::StackLock lock(_mutex);

	int channel = -1;
	int lowestPriority = 1000000;
	int lowestPriorityChannel = -1;
	for (int i = 0; i < kUsableChannels; ++i) {
		if (!_channels[i].isPresent) {
			channel = i;
			break;
		}
		if (_channels[i].priority < lowestPriority) {
			lowestPriority = _channels[i].priority;
			lowestPriorityChannel = i;
		}
	}
	if (channel == -1) {
		if (priority < lowestPriority) {
			//debug("No available audio channel found - giving up");
			return -1;
		}
		//debug("Stopping lowest priority channel %d with lower prio %d!", lowestPriorityChannel, lowestPriority);
		stop(lowestPriorityChannel, 0);
		channel = lowestPriorityChannel;
	}

	return playInChannel(channel, type, stream, priority, loop, volume, pan, endCallback, callbackData, trackDurationMs);
}

int AudioMixer::playMusic(Audio::RewindableAudioStream *stream, int volume, void(*endCallback)(int, void *), void *callbackData, uint32 trackDurationMs) {
	Common::StackLock lock(_mutex);

	return playInChannel(kMusicChannel, Audio::Mixer::kMusicSoundType, stream, 100, false, volume, 0, endCallback, callbackData, trackDurationMs);
}

void AudioMixer::stop(int channel, uint32 time) {
	Common::StackLock lock(_mutex);

	if (_channels[channel].isPresent) {
		if (time) {
			adjustVolume(channel, 0, time);
		} else {
			_channels[channel].isPresent = false;
			if (_channels[channel].sentToMixer) {
				_vm->_mixer->stopHandle(_channels[channel].handle);
			}
			if (_channels[channel].endCallback != nullptr) {
				_channels[channel].endCallback(channel, _channels[channel].callbackData);
			}
		}
	}
}

int AudioMixer::playInChannel(int channel, Audio::Mixer::SoundType type, Audio::RewindableAudioStream *stream, int priority, bool loop, int volume, int pan, void(*endCallback)(int, void *), void *callbackData, uint32 trackDurationMs) {
	_channels[channel].isPresent = true;
	_channels[channel].stream = stream;
	_channels[channel].priority = priority;
	_channels[channel].loop = loop;
	_channels[channel].volume = volume;
	_channels[channel].volumeTarget = 0;
	_channels[channel].volumeDelta = 0;
	_channels[channel].pan = pan;
	_channels[channel].panTarget = 0;
	_channels[channel].panDelta = 0;
	_channels[channel].endCallback = endCallback;
	_channels[channel].callbackData = callbackData;
	_channels[channel].timeStarted = _vm->_time->currentSystem();
	_channels[channel].trackDurationMs = trackDurationMs;

	Audio::AudioStream *audioStream = stream;

	if (loop) {
		audioStream = new Audio::LoopingAudioStream(stream, 0, DisposeAfterUse::YES);
	}

	if (!_vm->_mixer->isReady()) {
		_channels[channel].sentToMixer = false;
		return channel;
	}
	_channels[channel].sentToMixer = true;

	_vm->_mixer->playStream(
		type,
		&_channels[channel].handle,
		audioStream,
		-1,
		volume * 255 / 100,
		pan * 127 / 100);

	return channel;
}

bool AudioMixer::isActive(int channel) const {
	Common::StackLock lock(_mutex);

	return _channels[channel].isPresent
	       && ((_channels[channel].sentToMixer && _vm->_mixer->isSoundHandleActive(_channels[channel].handle))
	            || (!_channels[channel].sentToMixer && !_channels[channel].loop && (_vm->_time->currentSystem() - _channels[channel].timeStarted < _channels[channel].trackDurationMs)));
}

void AudioMixer::timerCallback(void *self) {
	((AudioMixer *)self)->tick();
}

void AudioMixer::adjustVolume(int channel, int newVolume, uint32 time) {
	Common::StackLock lock(_mutex);

	if (_channels[channel].isPresent) {
		_channels[channel].volumeTarget = newVolume;
		_channels[channel].volumeDelta = ((newVolume - _channels[channel].volume) / (time / 60.0f)) / (float)kUpdatesPerSecond;
	}
}

void AudioMixer::adjustPan(int channel, int newPan, uint32 time) {
	Common::StackLock lock(_mutex);

	if (_channels[channel].isPresent) {
		newPan = CLIP(newPan, -100, 100);
		_channels[channel].panTarget = newPan;
		_channels[channel].panDelta = ((newPan - _channels[channel].pan) / (time / 60.0f)) / (float)kUpdatesPerSecond;
	}
}

void AudioMixer::tick() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < kChannels; ++i) {
		Channel *channel = &_channels[i];
		if (!channel->isPresent) {
			continue;
		}

		if (channel->volumeDelta != 0.0f) {
			channel->volume = CLIP(channel->volume + channel->volumeDelta, 0.0f, 100.0f);

			if ((channel->volumeDelta < 0 && channel->volume <= channel->volumeTarget) || (channel->volumeDelta > 0 && channel->volume >= channel->volumeTarget)) {
				channel->volumeDelta = 0.0f;
			}

			if (channel->sentToMixer) {
				_vm->_mixer->setChannelVolume(channel->handle, (channel->volume * Audio::Mixer::kMaxChannelVolume) / 100); // map [0..100] to [0..kMaxChannelVolume]
			}

			if (channel->volume <= 0.0f) {
				stop(i, 0);
			}
		}

		if (channel->panDelta != 0.0) {
			channel->pan = CLIP(channel->pan + channel->panDelta, -100.0f, 100.0f);

			if ((channel->panDelta < 0 && channel->pan <= channel->panTarget) || (channel->panDelta > 0 && channel->pan >= channel->panTarget)) {
				channel->panDelta = 0.0f;
			}

			if (channel->sentToMixer) {
				_vm->_mixer->setChannelBalance(channel->handle, (channel->pan * 127) / 100); // map [-100..100] to [-127..127]
			}
		}

		if ((channel->sentToMixer && !_vm->_mixer->isSoundHandleActive(channel->handle))
			|| channel->stream->endOfStream()
			|| (!channel->sentToMixer && !channel->loop && _vm->_time->currentSystem() - channel->timeStarted >= channel->trackDurationMs)
		) {
			stop(i, 0);
		}
	}
}

} // End of namespace BladeRunner
