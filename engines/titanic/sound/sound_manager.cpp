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

namespace Titanic {

const uint SAMPLING_RATE = 22050;
const uint LATENCY = 100;
const uint CHANNELS_COUNT = 16;

CSoundManager::CSoundManager() : _musicPercent(75.0), _speechPercent(75.0),
	_masterPercent(75.0), _parrotPercent(75.0), _field14(1) {
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

void QSoundManagerSounds::flushChannel(int v1, int iChannel) {
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

QSoundManager::QSoundManager(Audio::Mixer *mixer) : CSoundManager(), QMixer(mixer),
		_field18(0), _field1C(0) {
	Common::fill(&_field4A0[0], &_field4A0[16], 0);

	qsWaveMixInitEx(QMIXCONFIG(SAMPLING_RATE, CHANNELS_COUNT, LATENCY));
	qsWaveMixActivate(true);
	qsWaveMixOpenChannel(0, QMIX_OPENALL);
}

QSoundManager::~QSoundManager() {
	// Close down the mixer
	qsWaveMixCloseSession();
}

CWaveFile *QSoundManager::loadSound(const CString &name) {
	CWaveFile *waveFile = new CWaveFile();

	// Try to load the specified sound
	if (!waveFile->loadSound(name)) {
		delete waveFile;
		return nullptr;
	}

	return waveFile;
}

CWaveFile *QSoundManager::loadSpeech(CDialogueFile *dialogueFile, int speechId) {
	warning("TODO");
	return nullptr;
}

int QSoundManager::proc5() const {
	warning("TODO");
	return 0;
}

int QSoundManager::playSound(CWaveFile &soundRes, CProximity &prox) {
	warning("TODO");
	return 0;
}

void QSoundManager::proc7() {
	warning("TODO");
}

void QSoundManager::proc8(int v) {
	warning("TODO");
}

void QSoundManager::proc9() {
	warning("TODO");
}

void QSoundManager::proc10() {
	warning("TODO");
}

void QSoundManager::proc11() {
	warning("TODO");
}

void QSoundManager::proc12() {
	warning("TODO");
}

void QSoundManager::proc13() {
	warning("TODO");
}

bool QSoundManager::proc14() {
	warning("TODO");
	return false;
}

bool QSoundManager::isActive(const CWaveFile *waveFile) const {
	warning("TODO");
	return false;
}

int QSoundManager::proc16() const {
	warning("TODO");
	return 0;
}

uint QSoundManager::getLatency() const {
	return LATENCY;
}

void QSoundManager::proc19(int v) {
	warning("TODO");
}

void QSoundManager::proc20(int v) {
	warning("TODO");
}

void QSoundManager::proc21(int v) {
	warning("TODO");
}

void QSoundManager::proc29() {
	warning("TODO");
}

void QSoundManager::proc30() {
	warning("TODO");
}

void QSoundManager::soundFreed(Audio::SoundHandle &handle) {
	qsWaveMixFreeWave(handle);
}

} // End of namespace Titanic z
