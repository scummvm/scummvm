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

#include "gnap/sound.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

namespace Gnap {

SoundMan::SoundMan(GnapEngine *vm) : _vm(vm) {
}

SoundMan::~SoundMan() {
}

void SoundMan::playSound(int resourceId, bool looping) {
	SoundItem soundItem;
	soundItem._resourceId = resourceId;

	SoundResource *soundResource = _vm->_soundCache->get(resourceId);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(soundResource->_data, soundResource->_size, DisposeAfterUse::NO);
	Audio::AudioStream *audioStream = Audio::makeLoopingAudioStream(Audio::makeWAVStream(stream, DisposeAfterUse::YES), looping ? 0 : 1);

	_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &soundItem._handle, audioStream);

	_items.push_back(soundItem);

}

void SoundMan::stopSound(int resourceId) {
	const int index = find(resourceId);
	if (index < 0)
		return;

	_vm->_soundCache->release(_items[index]._resourceId);
	_vm->_mixer->stopHandle(_items[index]._handle);
	_items.remove_at(index);
}

void SoundMan::setSoundVolume(int resourceId, int volume) {
	if (resourceId == -1 || volume < 0 || volume > 100)
		return;

	const int index = find(resourceId);
	if (index < 0)
		return;

	int realVol = volume * 2.55;
	_vm->_mixer->setChannelVolume(_items[index]._handle, realVol);
}

bool SoundMan::isSoundPlaying(int resourceId) {
	const int index = find(resourceId);
	if (index < 0)
		return false;

	return _vm->_mixer->isSoundHandleActive(_items[index]._handle);
}

void SoundMan::stopAll() {
	for (int index = 0; index < (int)_items.size(); ++index) {
		_vm->_soundCache->release(_items[index]._resourceId);
		_vm->_mixer->stopHandle(_items[index]._handle);
	}
}

void SoundMan::update() {
	for (int index = 0; index < (int)_items.size(); ++index)
		if (!_vm->_mixer->isSoundHandleActive(_items[index]._handle)) {
			_vm->_soundCache->release(_items[index]._resourceId);
			_items.remove_at(index);
			--index;
		}
}

int SoundMan::find(int resourceId) {
	for (int index = 0; index < (int)_items.size(); ++index)
		if (_items[index]._resourceId == resourceId)
			return index;
	return -1;
}

} // End of namespace Gnap
