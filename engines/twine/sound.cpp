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

#include "twine/sound.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/types.h"
#include "common/util.h"
#include "twine/collision.h"
#include "twine/flamovies.h"
#include "twine/grid.h"
#include "twine/hqrdepack.h"
#include "twine/movements.h"
#include "twine/resources.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Sound::Sound(TwinEEngine *engine) : _engine(engine) {
}

void Sound::sampleVolume(int32 chan, int32 volume) {
	if (chan == -1) {
		_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);
		return;
	}
	if (chan < 0 || chan > ARRAYSIZE(samplesPlaying)) {
		error("Given channel index is out of bounds: %i", chan);
	}
	_engine->_system->getMixer()->setChannelVolume(samplesPlaying[chan], volume / 2);
}

void Sound::playFlaSample(int32 index, int32 frequency, int32 repeat, int32 x, int32 y) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		return;
	}

	const Common::String& sampfile = Common::String::format(FLA_DIR "%s", Resources::HQR_FLASAMP_FILE);

	uint8 *sampPtr;
	int32 sampSize = _engine->_hqrdepack->hqrGetallocEntry(&sampPtr, sampfile.c_str(), index);
	// Fix incorrect sample files first byte
	if (*sampPtr != 'C') {
		*sampPtr = 'C';
	}

	Common::MemoryReadStream stream(sampPtr, sampSize, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(&stream, DisposeAfterUse::NO);
	// TODO: mgerhardy repeat flag
	_engine->_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &samplesPlaying[channelIdx], audioStream, index);
}

void Sound::setSamplePosition(int32 chan, int32 x, int32 y, int32 z) {
	int32 distance;
	distance = ABS(_engine->_movements->getDistance3D(_engine->_grid->newCameraX << 9, _engine->_grid->newCameraY << 8, _engine->_grid->newCameraZ << 9, x, y, z));
	distance = _engine->_collision->getAverageValue(0, distance, 10000, 255);
	if (distance > 255) { // don't play it if its to far away
		distance = 255;
	}

#if 0 // TODO
	Mix_SetDistance(chan, distance);
#endif
}

void Sound::playSample(int32 index, int32 frequency, int32 repeat, int32 x, int32 y, int32 z, int32 actorIdx) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		return;
	}
	uint8 *sampPtr;
	int32 sampSize = _engine->_hqrdepack->hqrGetallocEntry(&sampPtr, Resources::HQR_SAMPLES_FILE, index);
	// Fix incorrect sample files first byte
	if (*sampPtr != 'C') {
		*sampPtr = 'C';
	}

	if (actorIdx != -1) {
		setSamplePosition(channelIdx, x, y, z);

		// save the actor index for the channel so we can check the position
		samplesPlayingActors[channelIdx] = actorIdx;
	}

	Common::MemoryReadStream stream(sampPtr, sampSize, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(&stream, DisposeAfterUse::NO);
	// TODO: mgerhardy repeat flag
	_engine->_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &samplesPlaying[channelIdx], audioStream, index);
}

void Sound::resumeSamples() {
	if (!_engine->cfgfile.Sound) {
		return;
	}
	_engine->_system->getMixer()->pauseAll(false);
}

void Sound::pauseSamples() {
	if (!_engine->cfgfile.Sound) {
		return;
	}
	_engine->_system->getMixer()->pauseAll(true);
}

void Sound::stopSamples() {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	_engine->_system->getMixer()->stopAll();
	memset(samplesPlayingActors, -1, sizeof(samplesPlayingActors));
}

int32 Sound::getActorChannel(int32 index) {
	for (int32 c = 0; c < NUM_CHANNELS; c++) {
		if (samplesPlayingActors[c] == index) {
			return c;
		}
	}
	return -1;
}

int32 Sound::getSampleChannel(int32 index) {
	for (int32 c = 0; c < NUM_CHANNELS; c++) {
		if (_engine->_system->getMixer()->getSoundID(samplesPlaying[c]) == index) {
			return c;
		}
	}
	return -1;
}

void Sound::removeSampleChannel(int32 c) {
	samplesPlayingActors[c] = -1;
}

void Sound::stopSample(int32 index) {
	if (!_engine->cfgfile.Sound) {
		return;
	}
	int32 stopChannel = getSampleChannel(index);
	if (stopChannel != -1) {
		_engine->_system->getMixer()->stopID(index);
		removeSampleChannel(stopChannel);
	}
}

bool Sound::isChannelPlaying(int32 chan) {
	if (chan >= 0 && chan < ARRAYSIZE(samplesPlaying)) {
		if (_engine->_system->getMixer()->isSoundHandleActive(samplesPlaying[chan])) {
			return true;
		}
		removeSampleChannel(chan);
	}
	return false;
}

int32 Sound::isSamplePlaying(int32 index) {
	const int32 chan = getSampleChannel(index);
	return isChannelPlaying(chan);
}

int32 Sound::getFreeSampleChannelIndex() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (!_engine->_system->getMixer()->isSoundHandleActive(samplesPlaying[i])) {
			return i;
		}
	}
	return -1;
}

void Sound::playVoxSample(int32 index) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx != -1) {
		return;
	}

	uint8 *sampPtr = nullptr;
	int32 sampSize = _engine->_hqrdepack->hqrGetallocVoxEntry(&sampPtr, _engine->_text->currentVoxBankFile.c_str(), index, _engine->_text->voxHiddenIndex);

	// Fix incorrect sample files first byte
	if (*sampPtr != 'C') {
		_engine->_text->hasHiddenVox = *sampPtr;
		_engine->_text->voxHiddenIndex++;
		*sampPtr = 'C';
	}

	Common::MemoryReadStream stream(sampPtr, sampSize, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(&stream, DisposeAfterUse::NO);
	_engine->_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &samplesPlaying[channelIdx], audioStream, index);
}

} // namespace TwinE
