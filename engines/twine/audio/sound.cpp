/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "twine/audio/sound.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/text-to-speech.h"
#include "common/types.h"
#include "common/util.h"
#include "twine/parser/text.h"
#include "twine/scene/collision.h"
#include "twine/movies.h"
#include "twine/scene/grid.h"
#include "twine/resources/hqr.h"
#include "twine/scene/movements.h"
#include "twine/resources/resources.h"
#include "twine/shared.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Sound::Sound(TwinEEngine *engine) : _engine(engine) {
}

Sound::~Sound() {
	_engine->_system->getMixer()->stopAll();
}

void Sound::startRainSample() {
	if (!_engine->_cfgfile.Sound) {
		return;
	}
#if 0
	const int sample = SAMPLE_RAIN;
	if (CubeMode == CUBE_EXTERIEUR && !TEMPETE_FINIE && !isSamplePlaying(sample)) {
		const int rate = 0x1000;
		const int offset = 300;
		const int repeat = 0;
		const int panning = 64;
		const int volumeRain = 70;
		// TODO: mixSample(sample, rate, offset, repeat, panning, volumeRain);
	}

	RestartRainSample = false;
#endif
}

void Sound::setChannelRate(int32 channelIdx, uint32 rate) {
	if (channelIdx < 0 || channelIdx >= NUM_CHANNELS) {
		return;
	}
	_engine->_system->getMixer()->setChannelRate(_samplesPlaying[channelIdx], rate);
}

void Sound::setChannelBalance(int32 channelIdx, uint8 volumeLeft, uint8 volumeRight) {
	if (channelIdx < 0 || channelIdx >= NUM_CHANNELS) {
		return;
	}
	Audio::Mixer *mixer = _engine->_system->getMixer();
	const Audio::SoundHandle &handle = _samplesPlaying[channelIdx];
	// balance value ranges from -127 to 127
	int8 balance = (int8)(((int16)volumeRight - (int16)volumeLeft) * 127 / 255);
	mixer->setChannelBalance(handle, balance);
}

void Sound::setChannelPosition(int32 channelIdx, int32 x, int32 y, int32 z) {
	if (channelIdx < 0 || channelIdx >= NUM_CHANNELS) {
		return;
	}
	const int32 camX = _engine->_grid->_startCube.x * SIZE_BRICK_XZ;
	const int32 camY = _engine->_grid->_startCube.y * SIZE_BRICK_Y;
	const int32 camZ = _engine->_grid->_startCube.z * SIZE_BRICK_XZ;
	int32 distance = getDistance3D(camX, camY, camZ, x, y, z);
	distance = boundRuleThree(0, distance, 10000, 255);
	byte targetVolume = 0;
	if (distance < Audio::Mixer::kMaxChannelVolume) {
		targetVolume = Audio::Mixer::kMaxChannelVolume - distance;
	}
	_engine->_system->getMixer()->setChannelVolume(_samplesPlaying[channelIdx], targetVolume);
}

void Sound::playFlaSample(int32 index, int16 rate, int32 repeat, uint8 volumeLeft, uint8 volumeRight) {
	if (!_engine->_cfgfile.Sound) {
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
		_engine->_text->_hasHiddenVox = *sampPtr != '\0';
		_engine->_text->_voxHiddenIndex++;
		*sampPtr = 'C';
	}

	Common::MemoryReadStream *stream = new Common::MemoryReadStream(sampPtr, sampSize, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	if (playSample(channelIdx, index, audioStream, repeat, Resources::HQR_FLASAMP_FILE)) {
		setChannelRate(channelIdx, rate);
		setChannelBalance(channelIdx, volumeLeft, volumeRight);
	}
}

void Sound::mixSample(int32 index, uint16 pitchbend, int32 repeat, uint8 volumeLeft, uint8 volumeRight) {
	mixSample3D(index, pitchbend, repeat, {}, -1);
	int channelIdx = getSampleChannel(index);
	setChannelBalance(channelIdx, volumeLeft, volumeRight);
}

void Sound::mixSample3D(int32 index, uint16 pitchbend, int32 repeat, const IVec3 &pos, int32 actorIdx) {
	if (!_engine->_cfgfile.Sound) {
		return;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		warning("Failed to play sample for index: %i - no free channel", index);
		return;
	}

	if (actorIdx != -1) {
		// TODO: implement balance
		setChannelPosition(channelIdx, pos.x, pos.y, pos.z);
		// save the actor index for the channel so we can check the position
		_samplesPlayingActors[channelIdx] = actorIdx;
	} else {
		_samplesPlayingActors[channelIdx] = -1;
	}

	uint8 *sampPtr = _engine->_resources->_samplesTable[index];
	uint32 sampSize = _engine->_resources->_samplesSizeTable[index];
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(sampPtr, sampSize, DisposeAfterUse::NO);
	Audio::SeekableAudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	playSample(channelIdx, index, audioStream, repeat, Resources::HQR_SAMPLES_FILE, Audio::Mixer::kSFXSoundType);
	uint32 rate = 11025 + (pitchbend - 0x1000);
	setChannelRate(channelIdx, rate);
}

bool Sound::playVoxSample(const TextEntry *text) {
	if (!_engine->_cfgfile.Sound || text == nullptr) {
		return false;
	}

	int channelIdx = getFreeSampleChannelIndex();
	if (channelIdx == -1) {
		warning("Failed to play vox sample for index: %i - no free channel", text->index);
		return false;
	}

	if (_engine->isAndroid()) {
		Common::Path basename(Common::String::format("%s%03i", _engine->_text->_currentOggBaseFile.c_str(), text->index));
		Audio::SeekableAudioStream *audioStream = Audio::SeekableAudioStream::openStreamFile(basename);
		if (audioStream != nullptr) {
			return playSample(channelIdx, text->index, audioStream, 1, _engine->_text->_currentOggBaseFile.c_str(), Audio::Mixer::kSpeechSoundType);
		}
	}

	uint8 *sampPtr = nullptr;
	int32 sampSize = HQR::getAllocVoxEntry(&sampPtr, _engine->_text->_currentVoxBankFile.c_str(), text->index, _engine->_text->_voxHiddenIndex);
	if (sampSize == 0) {
		if (ConfMan.hasKey("tts_narrator") && ConfMan.getBool("tts_narrator")) {
			Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
			if (ttsMan != nullptr) {
				ttsMan->stop();
				return ttsMan->say(text->string);
			}
		} else {
			debug(4, "TTS disabled");
		}
		warning("Failed to get vox sample for index: %i", text->index);
		return false;
	}

	// Fix incorrect sample files first byte
	if (*sampPtr != 'C') {
		_engine->_text->_hasHiddenVox = *sampPtr != '\0';
		_engine->_text->_voxHiddenIndex++;
		*sampPtr = 'C';
	}
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(sampPtr, sampSize, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	return playSample(channelIdx, text->index, audioStream, 1, _engine->_text->_currentVoxBankFile.c_str(), Audio::Mixer::kSpeechSoundType);
}

bool Sound::playSample(int32 channelIdx, int32 index, Audio::SeekableAudioStream *audioStream, int32 loop, const char *name, Audio::Mixer::SoundType soundType) {
	if (audioStream == nullptr) {
		warning("Failed to create audio stream for %s: %i", name, index);
		return false;
	}

	// infinite loop
	if (loop == -1) {
		loop = 0;
	}
	Audio::AudioStream *loopStream = Audio::makeLoopingAudioStream(audioStream, loop);
	Audio::SoundHandle *handle = &_samplesPlaying[channelIdx];
	const byte volume = Audio::Mixer::kMaxChannelVolume;
	_engine->_system->getMixer()->playStream(soundType, handle, loopStream, index, volume);
	return true;
}

void Sound::resumeSamples() {
	if (!_engine->_cfgfile.Sound) {
		return;
	}
	_engine->_system->getMixer()->pauseAll(false);
}

void Sound::pauseSamples() {
	if (!_engine->_cfgfile.Sound) {
		return;
	}
	_engine->_system->getMixer()->pauseAll(true);
}

void Sound::stopSamples() { // HQ_StopSample
	if (!_engine->_cfgfile.Sound) {
		return;
	}

	for (int channelIdx = 0; channelIdx < NUM_CHANNELS; channelIdx++) {
		_engine->_system->getMixer()->stopHandle(_samplesPlaying[channelIdx]);
	}
	memset(_samplesPlayingActors, -1, sizeof(_samplesPlayingActors));
}

int32 Sound::getActorChannel(int32 actorIdx) {
	for (int32 channelIdx = 0; channelIdx < NUM_CHANNELS; channelIdx++) {
		if (_samplesPlayingActors[channelIdx] == actorIdx) {
			return channelIdx;
		}
	}
	return -1;
}

int32 Sound::getSampleChannel(int32 index) {
	for (int32 channelIdx = 0; channelIdx < NUM_CHANNELS; channelIdx++) {
		if (_engine->_system->getMixer()->getSoundID(_samplesPlaying[channelIdx]) == index) {
			return channelIdx;
		}
	}
	return -1;
}

void Sound::removeChannelWatch(int32 channelIdx) {
	_samplesPlayingActors[channelIdx] = -1;
}

void Sound::stopSample(int32 index) {
	if (!_engine->_cfgfile.Sound) {
		return;
	}
	const int32 channelIdx = getSampleChannel(index);
	if (channelIdx != -1) {
		_engine->_system->getMixer()->stopID(index);
		removeChannelWatch(channelIdx);
	}
}

bool Sound::isChannelPlaying(int32 channelIdx) {
	if (channelIdx >= 0 && channelIdx < ARRAYSIZE(_samplesPlaying)) {
		if (_engine->_system->getMixer()->isSoundHandleActive(_samplesPlaying[channelIdx])) {
			return true;
		}
		removeChannelWatch(channelIdx);
	}
	return false;
}

int32 Sound::isSamplePlaying(int32 index) {
	const int32 channelIdx = getSampleChannel(index);
	return isChannelPlaying(channelIdx);
}

int32 Sound::getFreeSampleChannelIndex() {
	for (int channelIdx = 0; channelIdx < NUM_CHANNELS; channelIdx++) {
		if (!_engine->_system->getMixer()->isSoundHandleActive(_samplesPlaying[channelIdx])) {
			return channelIdx;
		}
	}
	return -1;
}

} // namespace TwinE
