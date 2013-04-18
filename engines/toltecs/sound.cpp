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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "toltecs/toltecs.h"
#include "toltecs/resource.h"
#include "toltecs/segmap.h"
#include "toltecs/sound.h"

namespace Toltecs {

Sound::Sound(ToltecsEngine *vm) : _vm(vm) {
	for (int i = 0; i < kMaxChannels; i++) {
		channels[i].type = kChannelTypeEmpty;
		channels[i].resIndex = -1;
	}
}

Sound::~Sound() {
}

void Sound::playSpeech(int16 resIndex) {
	debug(0, "playSpeech(%d)", resIndex);
	internalPlaySound(resIndex, kChannelTypeSpeech, 50 /*TODO*/, 0);
}

void Sound::playSound(int16 resIndex, int16 type, int16 volume) {

	// TODO: Use the right volumes

	debug(0, "playSound(%d, %d, %d)", resIndex, type, volume);
	
	if (volume == -1 || type == -2) {
		if (type == kChannelTypeBackground) {
			internalPlaySound(resIndex, type, 50 /*TODO*/, 0);
		} else {
			internalPlaySound(resIndex, type, 100 /*TODO*/, 0);
		}
	} else {
		internalPlaySound(resIndex, type, 100 /*TODO*/, 0);
	}

}

void Sound::playSoundAtPos(int16 resIndex, int16 x, int16 y) {

	debug(0, "playSoundAtPos(%d, %d, %d)", resIndex, x, y);

	int16 volume, panning = 0, deltaX = 0;
	int8 scaling = _vm->_segmap->getScalingAtPoint(x, y);

	if (scaling >= 0)
		volume = 50 + ABS(scaling) / 2;
	else
		volume = 50 - ABS(scaling) / 2;

	if (_vm->_cameraX > x)
		deltaX = _vm->_cameraX - x;
	else if (_vm->_cameraX + 640 < x)
		deltaX = x - (_vm->_cameraX + 640);
	if (deltaX > 600)
		deltaX = 600;

	volume = ((100 - deltaX / 6) * volume) / 100;
	
	if (_vm->_cameraX + 320 != x) {
		panning = CLIP(x - (_vm->_cameraX + 320), -381, 381) / 3;
	}

	internalPlaySound(resIndex, 1, volume, panning);

}

void Sound::internalPlaySound(int16 resIndex, int16 type, int16 volume, int16 panning) {

	if (resIndex == -1) {
		// Stop all sounds
		_vm->_mixer->stopAll();
		_vm->_screen->keepTalkTextItemsAlive();
		for (int i = 0; i < kMaxChannels; i++) {
			channels[i].type = kChannelTypeEmpty;
			channels[i].resIndex = -1;
		}
	} else if (type == -2) {
		// Stop sounds with specified resIndex
		for (int i = 0; i < kMaxChannels; i++) {
			if (channels[i].resIndex == resIndex) {
				_vm->_mixer->stopHandle(channels[i].handle);
				channels[i].type = kChannelTypeEmpty;
				channels[i].resIndex = -1;
			}
		}
	} else {

		if (type == -3) {
			// Stop speech and play new sound
			stopSpeech();
		}
	
		// Play new sound in empty channel
		int freeChannel = -1;
		for (int i = 0; i < kMaxChannels; i++) {
			if (channels[i].type == kChannelTypeEmpty || !_vm->_mixer->isSoundHandleActive(channels[i].handle)) {
				freeChannel = i;
				break;
			}
		}
		
		// If all channels are in use no new sound will be played
		if (freeChannel >= 0) {
			Resource *soundResource = _vm->_res->load(resIndex);

			Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
								Audio::makeRawStream(soundResource->data,
								soundResource->size, 22050, Audio::FLAG_UNSIGNED,
								DisposeAfterUse::NO),
								type == kChannelTypeBackground ? 0 : 1);

			channels[freeChannel].type = type;
			channels[freeChannel].resIndex = resIndex;

			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType;
			/*
			switch (type) {
			}
			*/

			_vm->_mixer->playStream(soundType, &channels[freeChannel].handle,
				stream, -1, volume, panning);
		}

	}
	
}

void Sound::updateSpeech() {
	for (int i = 0; i < kMaxChannels; i++) {
		if (channels[i].type == kChannelTypeSpeech && _vm->_mixer->isSoundHandleActive(channels[i].handle)) {
			_vm->_screen->keepTalkTextItemsAlive();
			break;
		}
	}
}

void Sound::stopSpeech() {
	for (int i = 0; i < kMaxChannels; i++) {
		if (channels[i].type == kChannelTypeSpeech) {
			_vm->_mixer->stopHandle(channels[i].handle);
			_vm->_screen->keepTalkTextItemsAlive();
			channels[i].type = kChannelTypeEmpty;
			channels[i].resIndex = -1;
		}
	}
}

void Sound::stopAll() {
	for (int i = 0; i < kMaxChannels; i++) {
		_vm->_mixer->stopHandle(channels[i].handle);
		_vm->_screen->keepTalkTextItemsAlive();
		channels[i].type = kChannelTypeEmpty;
		channels[i].resIndex = -1;
	}
}

void Sound::saveState(Common::WriteStream *out) {
	for (int i = 0; i < kMaxChannels; i++) {
		out->writeSint16LE(channels[i].type);
		out->writeSint16LE(channels[i].resIndex);
	}
}

void Sound::loadState(Common::ReadStream *in) {
	for (int i = 0; i < kMaxChannels; i++) {
		channels[i].type = in->readSint16LE();
		channels[i].resIndex = in->readSint16LE();

		if (channels[i].type != kChannelTypeEmpty) {
			Resource *soundResource = _vm->_res->load(channels[i].resIndex);

			Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
								Audio::makeRawStream(soundResource->data,
								soundResource->size, 22050, Audio::FLAG_UNSIGNED,
								DisposeAfterUse::NO),
								channels[i].type == kChannelTypeBackground ? 0 : 1);

			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType;
			/*
			switch (type) {
			}
			*/

			// TODO: Volume and panning
			int16 volume = (channels[i].type == kChannelTypeBackground) ? 50 : 100;

			_vm->_mixer->playStream(soundType, &channels[i].handle,
				stream, -1, volume, /*panning*/0);
		}
	}
}

} // End of namespace Toltecs
