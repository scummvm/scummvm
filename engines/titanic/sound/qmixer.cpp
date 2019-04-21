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

#include "titanic/sound/qmixer.h"
#include "titanic/debugger.h"
#include "common/system.h"

namespace Titanic {

QMixer::QMixer(Audio::Mixer *mixer) : _mixer(mixer) {
}

QMixer::~QMixer() {
	_channels.clear();
}

bool QMixer::qsWaveMixInitEx(const QMIXCONFIG &config) {
	assert(_channels.empty());
	assert(config.iChannels > 0 && config.iChannels < 256);

	_channels.resize(config.iChannels);
	return true;
}

void QMixer::qsWaveMixActivate(bool fActivate) {
	// Not currently implemented in ScummVM
}

int QMixer::qsWaveMixOpenChannel(int iChannel, QMixFlag mode) {
	// Not currently implemented in ScummVM
	return 0;
}

int QMixer::qsWaveMixEnableChannel(int iChannel, uint flags, bool enabled) {
	// Not currently implemented in ScummVM
	return 0;
}

void QMixer::qsWaveMixCloseSession() {
	_mixer->stopAll();
	_channels.clear();
}

void QMixer::qsWaveMixFreeWave(Audio::SoundHandle &handle) {
	_mixer->stopHandle(handle);
}

void QMixer::qsWaveMixFlushChannel(int iChannel, uint flags) {
	if (flags & QMIX_OPENALL) {
		// Ignore channel, and flush all the channels
		for (uint idx = 0; idx < _channels.size(); ++idx)
			qsWaveMixFlushChannel(idx, 0);
	} else {
		// Flush the specified channel
		Common::List<SoundEntry>::iterator i;
		Common::List<SoundEntry> &sounds = _channels[iChannel]._sounds;
		for (i = sounds.begin(); i != sounds.end(); ++i)
			_mixer->stopHandle((*i)._soundHandle);

		sounds.clear();
	}
}

void QMixer::qsWaveMixSetPanRate(int iChannel, uint flags, uint rate) {
	ChannelEntry &channel = _channels[iChannel];
	channel._panRate = rate;
	channel._volumeChangeStart = channel._volumeChangeEnd = 0;
}

void QMixer::qsWaveMixSetVolume(int iChannel, uint flags, uint volume) {
	ChannelEntry &channel = _channels[iChannel];

	// QMixer volumes go from 0-32767, but we need to convert to 0-255 for ScummVM
	assert(volume <= 32767);
	byte newVolume = (volume >= 32700) ? 255 : volume * 255 / 32767;

	channel._volumeStart = channel._volume;
	channel._volumeEnd = newVolume;
	channel._volumeChangeStart = g_system->getMillis();
	channel._volumeChangeEnd = channel._volumeChangeStart + channel._panRate;
	debugC(DEBUG_DETAILED, kDebugCore, "qsWaveMixSetPanRate vol=%d to %d, start=%u, end=%u",
		channel._volumeStart, channel._volumeEnd, channel._volumeChangeStart, channel._volumeChangeEnd);
}

void QMixer::qsWaveMixSetSourcePosition(int iChannel, uint flags, const QSVECTOR &position) {
	ChannelEntry &channel = _channels[iChannel];

	// Flag whether distance should reset when a new sound is started
	channel._resetDistance = (flags & QMIX_USEONCE) != 0;

	// Currently, we only do a basic simulation of spatial positioning by
	// getting the distance, and proportionately reducing the volume the
	// further away the source is
	channel._distance = sqrt(position.x * position.x + position.y * position.y
		+ position.z * position.z);
}

void QMixer::qsWaveMixSetPolarPosition(int iChannel, uint flags, const QSPOLAR &position) {
	ChannelEntry &channel = _channels[iChannel];

	// Flag whether distance should reset when a new sound is started
	channel._resetDistance = (flags & QMIX_USEONCE) != 0;

	// Currently, we only do a basic simulation of spatial positioning by
	// getting the distance, and proportionately reducing the volume the
	// further away the source is
	channel._distance = position.range;
}

void QMixer::qsWaveMixSetListenerPosition(const QSVECTOR &position, uint flags) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetListenerOrientation(const QSVECTOR &direction, const QSVECTOR &up, uint flags) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetDistanceMapping(int iChannel, uint flags, const QMIX_DISTANCES &distances) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetFrequency(int iChannel, uint flags, uint frequency) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetSourceVelocity(int iChannel, uint flags, const QSVECTOR &velocity) {
	// Not currently implemented in ScummVM
}

int QMixer::qsWaveMixPlayEx(int iChannel, uint flags, CWaveFile *waveFile, int loops, const QMIXPLAYPARAMS &params) {
	if (iChannel == -1) {
		// Find a free channel
		for (iChannel = 0; iChannel < (int)_channels.size(); ++iChannel) {
			if (_channels[iChannel]._sounds.empty())
				break;
		}
		assert(iChannel != (int)_channels.size());
	}

	// If the new sound replaces current ones, then clear the channel
	ChannelEntry &channel = _channels[iChannel];
	if (flags & QMIX_CLEARQUEUE) {
		if (!channel._sounds.empty() && channel._sounds.front()._started)
			_mixer->stopHandle(channel._sounds.front()._soundHandle);

		channel._sounds.clear();
	}

	// Add the sound to the channel
	channel._sounds.push_back(SoundEntry(waveFile, params.callback, loops, params.dwUser));
	qsWaveMixPump();

	return 0;
}

bool QMixer::qsWaveMixIsChannelDone(int iChannel) const {
	return _channels[iChannel]._sounds.empty();
}

void QMixer::qsWaveMixPump() {
	// Iterate through each of the channels
	for (uint iChannel = 0; iChannel < _channels.size(); ++iChannel) {
		ChannelEntry &channel = _channels[iChannel];

		// If there's a transition in sound volume in progress, handle it
		if (channel._volumeChangeEnd) {
			byte oldVolume = channel._volume;
			uint currentTicks = g_system->getMillis();

			if (currentTicks >= channel._volumeChangeEnd) {
				// Reached end of transition period
				channel._volume = channel._volumeEnd;
				channel._volumeChangeStart = channel._volumeChangeEnd = 0;
			} else {
				// Transition in progress, so figure out new volume
				channel._volume = (int)channel._volumeStart +
					((int)channel._volumeEnd - (int)channel._volumeStart) *
					(int)(currentTicks - channel._volumeChangeStart) / (int)channel._panRate;
			}

			debugC(DEBUG_DETAILED, kDebugCore, "qsWaveMixPump time=%u vol=%d",
				currentTicks, channel._volume);

			if (channel._volume != oldVolume && !channel._sounds.empty()
					&& channel._sounds.front()._started) {
				_mixer->setChannelVolume(channel._sounds.front()._soundHandle,
					channel.getRawVolume());
			}
		}

		// If the playing sound on the channel is finished, then call
		// the callback registered for it, and remove it from the list
		if (!channel._sounds.empty()) {
			SoundEntry &sound = channel._sounds.front();
			if (sound._started && !_mixer->isSoundHandleActive(sound._soundHandle)) {
				// Sound is finished
				if (sound._callback)
					// Call the callback to signal end
					sound._callback(iChannel, sound._waveFile, sound._userData);

				// Remove sound record from channel
				channel._sounds.erase(channel._sounds.begin());
			}
		}

		// If there's an unstarted sound at the front of a channel's
		// sound list, then start it playing
		if (!channel._sounds.empty()) {
			SoundEntry &sound = channel._sounds.front();
			if (!sound._started) {
				if (channel._resetDistance)
					channel._distance = 0.0;

				// Play the wave
				sound._soundHandle = sound._waveFile->play(
					sound._loops, channel.getRawVolume());
				sound._started = true;
			}
		}
	}
}

/*------------------------------------------------------------------------*/

byte QMixer::ChannelEntry::getRawVolume() const {
	// Emperically decided adjustment divisor for distances
	const double ADJUSTMENT_FACTOR = 5.0;

	double r = 1.0 + (_distance / ADJUSTMENT_FACTOR);
	double percent = 1.0 / (r * r);

	double newVolume = _volume * percent;
	return (byte)newVolume;
}

} // End of namespace Titanic
