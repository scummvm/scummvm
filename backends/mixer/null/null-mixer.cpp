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

#include "backends/mixer/null/null-mixer.h"
#include "common/savefile.h"

NullMixerManager::NullMixerManager() : MixerManager() {
	_outputRate = 22050;
	_callsCounter = 0;
	_samples = 8192;
	while (_samples * 16 > _outputRate * 2)
		_samples >>= 1;
	_samplesBuf = new uint8[_samples * 4];
}

NullMixerManager::~NullMixerManager() {
	delete _samplesBuf;
}

void NullMixerManager::init() {
	_mixer = new Audio::MixerImpl(_outputRate);
	assert(_mixer);
	_mixer->setReady(true);
}

void NullMixerManager::suspendAudio() {
	_audioSuspended = true;
}

int NullMixerManager::resumeAudio() {
	if (!_audioSuspended) {
		return -2;
	}
	_audioSuspended = false;
	return 0;
}

void NullMixerManager::update(uint8 callbackPeriod) {
	if (_audioSuspended) {
		return;
	}
	_callsCounter++;
	if ((_callsCounter % callbackPeriod) == 0) {
		assert(_mixer);
		_mixer->mixCallback(_samplesBuf, _samples);
	}
}
