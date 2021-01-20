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

#include "twine/audio/sound.h"
#include "audio/audiostream.h"
#include "audio/decoders/voc.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/types.h"
#include "common/util.h"
#include "twine/scene/collision.h"
#include "twine/flamovies.h"
#include "twine/scene/grid.h"
#include "twine/resources/hqr.h"
#include "twine/scene/movements.h"
#include "twine/resources/resources.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Sound::Sound(TwinEEngine *engine) : _engine(engine) {
}

Sound::~Sound() {
	_engine->_system->getMixer()->stopAll();
}

void Sound::setSamplePosition(int32 channelIdx, int32 x, int32 y, int32 z) {
	if (channelIdx < 0 || channelIdx >= NUM_CHANNELS) {
		return;
	}
	const int32 camX = _engine->_grid->newCameraX * BRICK_SIZE;
	const int32 camY = _engine->_grid->newCameraY * BRICK_HEIGHT;
	const int32 camZ = _engine->_grid->newCameraZ * BRICK_SIZE;
	int32 distance = _engine->_movements->getDistance3D(camX, camY, camZ, x, y, z);
	distance = _engine->_collision->getAverageValue(0, distance, 10000, 255);
	const byte targetVolume = CLIP<byte>(255 - distance, 0, 255);
	_engine->_system->getMixer()->setChannelVolume(samplesPlaying[channelIdx], targetVolume);
}

void Sound::playFlaSample(int32 index, int32 repeat, int32 x, int32 y) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		warning("Failed to play fla sample for index: %i - no free channel", index);
		return;
	}

	uint8 *sampPtr = nullptr;
	const int32 sampSize = HQR::getAllocEntry(&sampPtr, Resources::HQR_FLASAMP_FILE, index);
	if (sampSize == 0) {
		warning("Failed to load %s", Resources::HQR_FLASAMP_FILE);
		return;
	}

	// Fix incorrect sample files first byte
	if (*sampPtr != 'C') {
		_engine->_text->hasHiddenVox = *sampPtr != '\0';
		_engine->_text->voxHiddenIndex++;
		*sampPtr = 'C';
	}

	playSample(channelIdx, index, sampPtr, sampSize, repeat, Resources::HQR_FLASAMP_FILE);
}

void Sound::playSample(int32 index, int32 repeat, int32 x, int32 y, int32 z, int32 actorIdx) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		warning("Failed to play sample for index: %i - no free channel", index);
		return;
	}

	if (actorIdx != -1) {
		setSamplePosition(channelIdx, x, y, z);
		// save the actor index for the channel so we can check the position
		samplesPlayingActors[channelIdx] = actorIdx;
	}

	uint8 *sampPtr = _engine->_resources->samplesTable[index];
	int32 sampSize = _engine->_resources->samplesSizeTable[index];
	playSample(channelIdx, index, sampPtr, sampSize, repeat, Resources::HQR_SAMPLES_FILE, Audio::Mixer::kSFXSoundType, DisposeAfterUse::NO);
}

void Sound::playVoxSample(int32 index) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		warning("Failed to play vox sample for index: %i - no free channel", index);
		return;
	}

	uint8 *sampPtr = nullptr;
	int32 sampSize = HQR::getAllocVoxEntry(&sampPtr, _engine->_text->currentVoxBankFile.c_str(), index, _engine->_text->voxHiddenIndex);
	if (sampSize == 0) {
		warning("Failed to get vox sample for index: %i", index);
		return;
	}

	// Fix incorrect sample files first byte
	if (*sampPtr != 'C') {
		_engine->_text->hasHiddenVox = *sampPtr != '\0';
		_engine->_text->voxHiddenIndex++;
		*sampPtr = 'C';
	}

	playSample(channelIdx, index, sampPtr, sampSize, 1, _engine->_text->currentVoxBankFile.c_str(), Audio::Mixer::kSpeechSoundType);
}

bool Sound::playSample(int channelIdx, int index, uint8 *sampPtr, int32 sampSize, int32 loop, const char *name, Audio::Mixer::SoundType soundType, DisposeAfterUse::Flag disposeFlag) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(sampPtr, sampSize, disposeFlag);
	Audio::SeekableAudioStream *audioStream = Audio::makeVOCStream(stream, DisposeAfterUse::YES);
	if (audioStream == nullptr) {
		warning("Failed to create audio stream for %s", name);
		delete stream;
		return false;
	}
	if (loop == -1) {
		loop = 0;
	}
	_engine->_system->getMixer()->playStream(soundType, &samplesPlaying[channelIdx], Audio::makeLoopingAudioStream(audioStream, loop), index);
	return true;
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

	for (int i = 0; i < NUM_CHANNELS; i++) {
		_engine->_system->getMixer()->stopHandle(samplesPlaying[i]);
	}
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
	const int32 stopChannel = getSampleChannel(index);
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

} // namespace TwinE
