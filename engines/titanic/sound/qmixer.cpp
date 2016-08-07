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

namespace Titanic {

QMixer::QMixer(Audio::Mixer *mixer) : _mixer(mixer) {
}

bool QMixer::qsWaveMixInitEx(const QMIXCONFIG &config) {
	// Not currently implemented in ScummVM
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
}

void QMixer::qsWaveMixFreeWave(Audio::SoundHandle &handle) {
	_mixer->stopHandle(handle);
}

void QMixer::qsWaveMixFlushChannel(int iChannel, uint flags) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetPanRate(int iChannel, uint flags, uint rate) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetVolume(int iChannel, uint flags, uint volume) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetSourcePosition(int iChannel, uint flags, const QSVECTOR &position) {
	// Not currently implemented in ScummVM
}

void QMixer::qsWaveMixSetPolarPosition(int iChannel, uint flags, const QSPOLAR &position) {
	// Not currently implemented in ScummVM
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

int QMixer::qsWaveMixPlayEx(int iChannel, uint flags, CWaveFile *mixWave, int loops, const QMIXPLAYPARAMS &params) {
	// Not currently implemented in ScummVM
	return 0;
}

bool QMixer::qsWaveMixIsChannelDone(int iChannel) const {
	// Not currently implemented in ScummVM
	return true;
}

void QMixer::qsWaveMixPump() {
	// TODO: Handle checking for done sounds, and calling their end functions
}

} // End of namespace Titanic z
