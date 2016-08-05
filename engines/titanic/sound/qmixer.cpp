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
	// Not current implemented in ScummVM
	return true;
}

void QMixer::qsWaveMixActivate(bool fActivate) {
	// Not current implemented in ScummVM
}

int QMixer::qsWaveMixOpenChannel(int iChannel, WaveMixOpenChannel mode) {
	// Not current implemented in ScummVM
	return 0;
}

void QMixer::qsWaveMixCloseSession() {
	_mixer->stopAll();
}

void QMixer::qsWaveMixFreeWave(Audio::SoundHandle &handle) {
	_mixer->stopHandle(handle);
}

} // End of namespace Titanic z
