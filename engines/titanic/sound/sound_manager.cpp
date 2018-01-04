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

#include "titanic/sound/sound_manager.h"
#include "titanic/events.h"
#include "titanic/titanic.h"

namespace Titanic {

const uint LATENCY = 100;
const uint CHANNELS_COUNT = 16;

CSoundManager::CSoundManager() : _musicPercent(75.0), _speechPercent(75.0),
	_masterPercent(75.0), _parrotPercent(75.0), _handleCtr(1) {
}

uint CSoundManager::getModeVolume(VolumeMode mode) {
	switch (mode) {
	case VOL_NORMAL:
		return (uint)_masterPercent;
	case VOL_QUIET:
		return (uint)(_masterPercent * 30 / 100);
	case VOL_VERY_QUIET:
		return (uint)(_masterPercent * 15 / 100);
	default:
		return 0;
	}
}

/*------------------------------------------------------------------------*/

void QSoundManagerSounds::add(CWaveFile *waveFile, int iChannel, CEndTalkerFn endFn, TTtalker *talker) {
	push_back(new QSoundManagerSound(waveFile, iChannel, endFn, talker));
}

void QSoundManagerSounds::flushChannel(int iChannel) {
	for (iterator i = begin(); i != end(); ++i) {
		QSoundManagerSound *item = *i;
		if (item->_iChannel == iChannel) {
			if (item->_endFn)
				item->_endFn(item->_talker);

			remove(item);
			delete item;
			break;
		}
	}
}

void QSoundManagerSounds::flushChannel(CWaveFile *waveFile, int iChannel) {
	for (iterator i = begin(); i != end(); ++i) {
		QSoundManagerSound *item = *i;
		if (item->_waveFile->isLoaded() && item->_iChannel == iChannel) {
			if (item->_endFn)
				item->_endFn(item->_talker);

			remove(item);
			delete item;
			break;
		}
	}
}

bool QSoundManagerSounds::contains(const CWaveFile *waveFile) const {
	for (const_iterator i = begin(); i != end(); ++i) {
		const QSoundManagerSound *item = *i;
		if (item->_waveFile == waveFile)
			return true;
	}

	return false;
}

/*------------------------------------------------------------------------*/

void QSoundManager::Slot::clear() {
	_waveFile = nullptr;
	_isTimed = false;
	_ticks = 0;
	_channel = -1;
	_handle = 0;
	_positioningMode = POSMODE_NONE;
}

/*------------------------------------------------------------------------*/

QSoundManager::QSoundManager(Audio::Mixer *mixer) : CSoundManager(), QMixer(mixer),
		_field18(0), _field1C(0) {
	_slots.resize(48);
	Common::fill(&_channelsVolume[0], &_channelsVolume[16], 0);
	Common::fill(&_channelsMode[0], &_channelsMode[16], 0);

	qsWaveMixInitEx(QMIXCONFIG(AUDIO_SAMPLING_RATE, CHANNELS_COUNT, LATENCY));
	qsWaveMixActivate(true);
	qsWaveMixOpenChannel(0, QMIX_OPENALL);
}

QSoundManager::~QSoundManager() {
	// Close down the mixer
	qsWaveMixCloseSession();
}

CWaveFile *QSoundManager::loadSound(const CString &name) {
	CWaveFile *waveFile = new CWaveFile(_mixer);

	// Try to load the specified sound
	if (!waveFile->loadSound(name)) {
		delete waveFile;
		return nullptr;
	}

	return waveFile;
}

CWaveFile *QSoundManager::loadSpeech(CDialogueFile *dialogueFile, int speechId) {
	CWaveFile *waveFile = new CWaveFile(_mixer);

	// Try to load the specified sound
	if (!waveFile->loadSpeech(dialogueFile, speechId)) {
		delete waveFile;
		return nullptr;
	}

	return waveFile;
}

CWaveFile *QSoundManager::loadMusic(const CString &name) {
	CWaveFile *waveFile = new CWaveFile(_mixer);

	// Try to load the specified sound
	if (!waveFile->loadMusic(name)) {
		delete waveFile;
		return nullptr;
	}

	return waveFile;
}

CWaveFile *QSoundManager::loadMusic(CAudioBuffer *buffer, DisposeAfterUse::Flag disposeAfterUse) {
	CWaveFile *waveFile = new CWaveFile(_mixer);

	// Try to load the specified audio buffer
	if (!waveFile->loadMusic(buffer, disposeAfterUse)) {
		delete waveFile;
		return nullptr;
	}

	return waveFile;
}

int QSoundManager::playSound(CWaveFile &waveFile, CProximity &prox) {
	int channel = -1;
	uint flags = QMIX_CLEARQUEUE;

	if (prox._priorSoundHandle >= 1) {
		// This sound should only be started after a prior one finishes,
		// so scan the slots for the specified sound
		for (uint idx = 0; idx < _slots.size(); ++idx) {
			if (_slots[idx]._handle == prox._priorSoundHandle) {
				channel = _slots[idx]._channel;
				flags = QMIX_QUEUEWAVE;
				break;
			}
		}
	}

	if (channel >= 0 || (channel = resetChannel(prox._channelMode)) != -1) {
		return playWave(&waveFile, channel, flags, prox);
	}

	return 0;
}

void QSoundManager::stopSound(int handle) {
	resetChannel(10);

	for (uint idx = 0; idx < _slots.size(); ++idx) {
		Slot &slot = _slots[idx];
		if (slot._handle == handle) {
			qsWaveMixFlushChannel(slot._channel);
			_sounds.flushChannel(slot._channel);
			resetChannel(10);
		}
	}
}

void QSoundManager::stopChannel(int channel) {
	int endChannel;
	switch (channel) {
	case 0:
	case 3:
		endChannel = channel + 3;
		break;
	case 6:
		endChannel = 10;
		break;
	case 10:
		endChannel = 48;
		break;
	default:
		return;
	}

	for (; channel < endChannel; ++channel) {
		qsWaveMixFlushChannel(channel);
		_sounds.flushChannel(channel);
	}
}

void QSoundManager::setCanFree(int handle) {
	for (uint idx = 0; idx < _slots.size(); ++idx) {
		if (_slots[idx]._handle == handle)
			_slots[idx]._isTimed = true;
	}
}

void QSoundManager::stopAllChannels() {
	qsWaveMixFlushChannel(0, QMIX_OPENALL);

	for (int idx = 0; idx < 16; ++idx)
		_sounds.flushChannel(idx);
	resetChannel(10);
}

int QSoundManager::resetChannel(int iChannel) {
	int newChannel = -1;
	int channelStart = 10;
	int channelEnd = 16;

	if (iChannel != 10) {
		qsWaveMixFlushChannel(iChannel);
		_sounds.flushChannel(iChannel);
		channelStart = iChannel;
		channelEnd = iChannel + 1;
	} else {
		uint ticks = g_vm->_events->getTicksCount();

		for (uint idx = 0; idx < _slots.size(); ++idx) {
			Slot &slot = _slots[idx];
			if (slot._isTimed && slot._ticks && ticks > slot._ticks) {
				qsWaveMixFlushChannel(slot._channel);
				_sounds.flushChannel(slot._channel);
			}
		}
	}

	for (iChannel = channelStart; iChannel < channelEnd; ++iChannel) {
		if (qsWaveMixIsChannelDone(iChannel)) {
			// Scan through the slots, and reset any slot using the channel
			for (uint idx = 0; idx < _slots.size(); ++idx) {
				Slot &slot = _slots[idx];
				if (slot._channel == iChannel)
					slot.clear();
			}

			// Use the empty channel
			newChannel = iChannel;
		}
	}

	return newChannel;
}

void QSoundManager::setVolume(int handle, uint volume, uint seconds) {
	for (uint idx = 0; idx < _slots.size(); ++idx) {
		Slot &slot = _slots[idx];
		if (slot._handle == handle) {
			assert(slot._channel >= 0);
			_channelsVolume[slot._channel] = volume;
			updateVolume(slot._channel, seconds * 1000);

			if (!volume) {
				uint ticks = g_vm->_events->getTicksCount() + seconds * 1000;
				if (!slot._ticks || ticks >= slot._ticks)
					slot._ticks = ticks;
			} else {
				slot._ticks = 0;
			}
			break;
		}
	}
}

void QSoundManager::setVectorPosition(int handle, double x, double y, double z, uint panRate) {
	for (uint idx = 0; idx < _slots.size(); ++idx) {
		Slot &slot = _slots[idx];
		if (slot._handle == handle) {
			qsWaveMixSetPanRate(slot._channel, QMIX_USEONCE, panRate);
			qsWaveMixSetSourcePosition(slot._channel, QMIX_USEONCE, QSVECTOR(x, y, z));
			break;
		}
	}
}

void QSoundManager::setPolarPosition(int handle, double range, double azimuth, double elevation, uint panRate) {
	for (uint idx = 0; idx < _slots.size(); ++idx) {
		Slot &slot = _slots[idx];
		if (slot._handle == handle) {
			qsWaveMixSetPanRate(slot._channel, QMIX_USEONCE, panRate);
			qsWaveMixSetPolarPosition(slot._channel, QMIX_USEONCE,
				QSPOLAR(azimuth, range, elevation));
			break;
		}
	}
}

bool QSoundManager::isActive(int handle) {
	resetChannel(10);

	for (uint idx = 0; idx < _slots.size(); ++idx) {
		if (_slots[idx]._handle == handle)
			return true;
	}

	return false;
}

bool QSoundManager::isActive(const CWaveFile *waveFile) {
	return _sounds.contains(waveFile);
}

void QSoundManager::waveMixPump() {
	qsWaveMixPump();
}

uint QSoundManager::getLatency() const {
	return LATENCY;
}

void QSoundManager::setMusicPercent(double percent) {
	_musicPercent = percent;
	updateVolumes();
}

void QSoundManager::setSpeechPercent(double percent) {
	_speechPercent = percent;
	updateVolumes();
}

void QSoundManager::setMasterPercent(double percent) {
	_masterPercent = percent;
	updateVolumes();
}

void QSoundManager::setParrotPercent(double percent) {
	_parrotPercent = percent;
}

void QSoundManager::setListenerPosition(double posX, double posY, double posZ,
		double directionX, double directionY, double directionZ, bool stopSounds) {
	if (stopSounds) {
		// Stop any running sounds
		for (uint idx = 0; idx < _slots.size(); ++idx) {
			if (_slots[idx]._positioningMode != 0)
				stopSound(_slots[idx]._handle);
		}
	}

	qsWaveMixSetListenerPosition(QSVECTOR(posX, posY, posZ));
	qsWaveMixSetListenerOrientation(QSVECTOR(directionX, directionY, directionZ),
		QSVECTOR(0.0, 0.0, -1.0));
}

int QSoundManager::playWave(CWaveFile *waveFile, int iChannel, uint flags, CProximity &prox) {
	if (!waveFile || !waveFile->isLoaded())
		return 0;

	prox._channelVolume = CLIP(prox._channelVolume, 0, 100);
	prox._balance = CLIP(prox._balance, -100, 100);

	int slotIndex = findFreeSlot();
	if (slotIndex == -1)
		return -1;

	// Set the volume
	setChannelVolume(iChannel, prox._channelVolume, prox._channelMode);

	switch (prox._positioningMode) {
	case POSMODE_POLAR:
		qsWaveMixSetPolarPosition(iChannel, 8, QSPOLAR(prox._azimuth, prox._range, prox._elevation));
		qsWaveMixEnableChannel(iChannel, QMIX_CHANNEL_ELEVATION, true);
		qsWaveMixSetDistanceMapping(iChannel, 8, QMIX_DISTANCES(5.0, 3.0, 1.0));
		break;

	case POSMODE_VECTOR:
		qsWaveMixSetSourcePosition(iChannel, 8, QSVECTOR(prox._posX, prox._posY, prox._posZ));
		qsWaveMixEnableChannel(iChannel, QMIX_CHANNEL_ELEVATION, true);
		qsWaveMixSetDistanceMapping(iChannel, 8, QMIX_DISTANCES(5.0, 3.0, 1.0));
		break;

	default:
		qsWaveMixEnableChannel(iChannel, QMIX_CHANNEL_ELEVATION, true);
		qsWaveMixSetPolarPosition(iChannel, 8, QSPOLAR(0.0, 1.0, 0.0));
		break;
	}

	if (prox._frequencyMultiplier || prox._frequencyAdjust != 1.875) {
		uint freq = (uint)(waveFile->getFrequency() * prox._frequencyMultiplier);
		qsWaveMixSetFrequency(iChannel, 8, freq);
	}

	_sounds.add(waveFile, iChannel, prox._endTalkerFn, prox._talker);

	QMIXPLAYPARAMS playParams;
	playParams.callback = soundFinished;
	playParams.dwUser = this;
	if (!qsWaveMixPlayEx(iChannel, flags, waveFile, prox._repeated ? -1 : 0, playParams)) {
		Slot &slot = _slots[slotIndex];
		slot._handle = _handleCtr++;
		slot._channel = iChannel;
		slot._waveFile = waveFile;
		slot._positioningMode = prox._positioningMode;

		return slot._handle;
	} else {
		_sounds.flushChannel(waveFile, iChannel);
		if (prox._disposeAfterUse == DisposeAfterUse::YES)
			delete waveFile;
		return 0;
	}
}

void QSoundManager::soundFreed(Audio::SoundHandle &handle) {
	qsWaveMixFreeWave(handle);
}

void QSoundManager::updateVolume(int channel, uint panRate) {
	double volume = _channelsVolume[channel] * 327;

	switch (_channelsMode[channel]) {
	case 0:
	case 1:
	case 2:
		volume = (_speechPercent * volume) / 100;
		break;
	case 3:
	case 4:
	case 5:
		volume = (75 * volume) / 100;
		break;
	case 6:
	case 7:
	case 8:
	case 9:
		volume = (_masterPercent * volume) / 100;
		break;
	default:
		break;
	}

	volume = (_musicPercent * volume) / 100;
	qsWaveMixSetPanRate(channel, 0, panRate);
	qsWaveMixSetVolume(channel, 0, (uint)volume);
}

void QSoundManager::updateVolumes() {
	for (uint idx = 0; idx < CHANNELS_COUNT; ++idx)
		updateVolume(idx, 250);
}

void QSoundManager::soundFinished(int iChannel, CWaveFile *waveFile, void *soundManager) {
	static_cast<QSoundManager *>(soundManager)->_sounds.flushChannel(waveFile, iChannel);
}

int QSoundManager::findFreeSlot() {
	for (uint idx = 0; idx < _slots.size(); ++idx) {
		if (!_slots[idx]._waveFile)
			return idx;
	}

	return -1;
}

void QSoundManager::setChannelVolume(int iChannel, uint volume, uint mode) {
	_channelsVolume[iChannel] = volume;
	_channelsMode[iChannel] = mode;
	updateVolume(iChannel, 250);
}

} // End of namespace Titanic z
