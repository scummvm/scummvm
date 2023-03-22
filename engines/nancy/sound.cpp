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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/substream.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/map.h"

namespace Nancy {

enum SoundType {
	kSoundTypeDiamondware,
	kSoundTypeRaw,
	kSoundTypeOgg
};

// Table valid for vampire diaries and nancy1, could be (and probably is) different between games
static const Audio::Mixer::SoundType channelSoundTypes[] = {
	Audio::Mixer::kMusicSoundType, // channel 0
	Audio::Mixer::kMusicSoundType,
	Audio::Mixer::kMusicSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType, // 5
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSpeechSoundType,
	Audio::Mixer::kSpeechSoundType,
	Audio::Mixer::kPlainSoundType,
	Audio::Mixer::kPlainSoundType, // 10
	Audio::Mixer::kPlainSoundType,
	Audio::Mixer::kPlainSoundType,
	Audio::Mixer::kPlainSoundType,
	Audio::Mixer::kPlainSoundType,
	Audio::Mixer::kPlainSoundType, // 15
	Audio::Mixer::kPlainSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kMusicSoundType,
	Audio::Mixer::kSFXSoundType, // 20
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kSFXSoundType, // 25
	Audio::Mixer::kSFXSoundType,
	Audio::Mixer::kMusicSoundType,
	Audio::Mixer::kMusicSoundType,
	Audio::Mixer::kMusicSoundType,
	Audio::Mixer::kSpeechSoundType, // 30
	Audio::Mixer::kSFXSoundType
};

bool readDiamondwareHeader(Common::SeekableReadStream *stream, SoundType &type, uint16 &numChannels,
					uint32 &samplesPerSec, uint16 &bitsPerSample, uint32 &size) {
	stream->skip(2);

	if (stream->readByte() != 1 || stream->readByte() > 1) {
		// Version, up to 1.1 is supported
		return false;
	}

	stream->skip(5); // sound id, reserved

	if (stream->readByte() != 0) {
		// Compression type, only uncompressed (0) is supported
		return false;
	}

	samplesPerSec = stream->readUint16LE();
	numChannels = stream->readByte();
	bitsPerSample = stream->readByte();
	stream->skip(2); // Absolute value of largest sample in file
	size = stream->readUint32LE();
	stream->skip(4); // Number of samples
	uint dataOffset = stream->readUint16LE();
	stream->seek(dataOffset);

	type = kSoundTypeDiamondware;

	return true;
}

bool readWaveHeader(Common::SeekableReadStream *stream, SoundType &type, uint16 &numChannels,
					uint32 &samplesPerSec, uint16 &bitsPerSample, uint32 &size) {
	// The earliest HIS files are just WAVE files with the first 22 bytes of
	// the file overwritten with a string, so most of this is copied from the
	// standard WAVE decoder
	numChannels = stream->readUint16LE();
	samplesPerSec = stream->readUint32LE();
	stream->skip(6);
	bitsPerSample = stream->readUint16LE();

	char buf[4 + 1];
	stream->read(buf, 4);
	buf[4] = 0;

	if (Common::String(buf) != "data") {
		warning("Data chunk not found in HIS file");
		return false;
	}

	size = stream->readUint32LE();

	if (stream->eos() || stream->err()) {
		warning("Error reading HIS file");
		return false;
	}

	type = kSoundTypeRaw;

	return true;
}

bool readHISHeader(Common::SeekableReadStream *stream, SoundType &type, uint16 &numChannels,
					uint32 &samplesPerSec, uint16 &bitsPerSample, uint32 &size) {
	uint32 ver;
	ver = stream->readUint16LE() << 16;
	ver |= stream->readUint16LE();
	bool hasType = false;

	switch (ver) {
	case 0x00010000:
		break;
	case 0x00020000:
		hasType = true;
		break;
	default:
		warning("Unsupported version %d.%d found in HIS file", ver >> 16, ver & 0xffff);
		return false;
	}

	// Same data as Wave fmt chunk
	stream->skip(2); // AudioFormat
	numChannels = stream->readUint16LE();
	samplesPerSec = stream->readUint32LE();
	stream->skip(6); // ByteRate and BlockAlign
	bitsPerSample = stream->readUint16LE();

	size = stream->readUint32LE();

	if (hasType) {
		uint16 tp = stream->readUint16LE();
		switch (tp) {
		case 1:
			type = kSoundTypeRaw;
			break;
		case 2:
			type = kSoundTypeOgg;
			break;
		default:
			warning("Unsupported sound type %d found in HIS file", tp);
			return false;
		}
	} else
		type = kSoundTypeRaw;

	if (stream->eos() || stream->err()) {
		warning("Error reading HIS file");
		return false;
	}

	return true;
}

Audio::SeekableAudioStream *SoundManager::makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	char buf[22];

	stream->read(buf, 22);
	buf[21] = 0;
	Common::String headerID(buf);

	uint16 numChannels = 0, bitsPerSample = 0;
	uint32 samplesPerSec = 0, size = 0;
	SoundType type = kSoundTypeRaw;

	if (headerID == "DiamondWare Digitized") {
		if (!readDiamondwareHeader(stream, type, numChannels, samplesPerSec, bitsPerSample, size))
			return nullptr;
	} else if (headerID == "Her Interactive Sound") {
		// Early HIS file
		if (!readWaveHeader(stream, type, numChannels, samplesPerSec, bitsPerSample, size))
			return nullptr;
	} else if (headerID == "HIS") {
		stream->seek(4);
		if (!readHISHeader(stream, type, numChannels, samplesPerSec, bitsPerSample, size))
			return nullptr;
	}

	byte flags = 0;
	if (type == kSoundTypeRaw || type == kSoundTypeDiamondware) {
		if (bitsPerSample == 8) {			// 8 bit data is unsigned in HIS files and signed in DWD files
			flags |= (type == kSoundTypeRaw ? Audio::FLAG_UNSIGNED : Audio::FLAG_LITTLE_ENDIAN);
		} else if (bitsPerSample == 16) {	// 16 bit data is signed little endian
			flags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
		} else {
			warning("Unsupported bitsPerSample %d found in HIS file", bitsPerSample);
			return nullptr;
		}

		if (numChannels == 2) {
			flags |= Audio::FLAG_STEREO;
		} else if (numChannels != 1) {
			warning("Unsupported number of channels %d found in HIS file", numChannels);
			return nullptr;
		}

		// Raw PCM, make sure the last packet is complete
		uint sampleSize = (flags & Audio::FLAG_16BITS ? 2 : 1) * (flags & Audio::FLAG_STEREO ? 2 : 1);
		if (size % sampleSize != 0) {
			warning("Trying to play an %s file with an incomplete PCM packet", type == kSoundTypeDiamondware ? "DWD" : "HIS");
			size &= ~(sampleSize - 1);
		}
	}

	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(stream, stream->pos(), stream->pos() + size, disposeAfterUse);

	if (type == kSoundTypeRaw || type == kSoundTypeDiamondware)
		return Audio::makeRawStream(subStream, samplesPerSec, flags, DisposeAfterUse::YES);
	else
		return Audio::makeVorbisStream(subStream, DisposeAfterUse::YES);
}

SoundManager::SoundManager() {
	_mixer = g_system->getMixer();

	initSoundChannels();
}

void SoundManager::loadCommonSounds() {
	// Persistent sounds that are used across the engine. These originally get loaded inside Logo
	Common::String chunkNames[] = {
		"CANT", "CURT", "GLOB", "SLID", "BULS", "BUDE", "BUOK", "TH1", "TH2",
	};

	Common::SeekableReadStream *chunk = nullptr;
	for (auto const &s : chunkNames) {
		chunk = g_nancy->getBootChunkStream(s);
		if (chunk) {
			SoundDescription &desc = _commonSounds.getOrCreateVal(s);
			desc.read(*chunk, SoundDescription::kNormal);
			g_nancy->_sound->loadSound(desc);
		}
	}

	// Menu sound is stored differently
	chunk = g_nancy->getBootChunkStream("MSND"); // channel 28
	if (chunk) {
		SoundDescription &desc = _commonSounds.getOrCreateVal("MSND");
		desc.read(*chunk, SoundDescription::kMenu);
		g_nancy->_sound->loadSound(desc);
	}
}

SoundManager::~SoundManager() {
	stopAllSounds();
}

void SoundManager::loadSound(const SoundDescription &description, bool panning) {
	if (description.name == "NO SOUND") {
		return;
	}

	if (_mixer->isSoundHandleActive(_channels[description.channelID].handle)) {
		_mixer->stopHandle(_channels[description.channelID].handle);
	}

	Channel &chan = _channels[description.channelID];

	delete chan.stream;
	chan.stream = nullptr;

	chan.name = description.name;
	chan.numLoops = description.numLoops;
	chan.volume = description.volume;
	chan.panAnchorFrame = description.panAnchorFrame;
	chan.isPanning = panning;

	Common::SeekableReadStream *file = SearchMan.createReadStreamForMember(description.name + (g_nancy->getGameType() == kGameTypeVampire ? ".dwd" : ".his"));
	if (file) {
		_channels[description.channelID].stream = makeHISStream(file, DisposeAfterUse::YES);
	}
}

void SoundManager::playSound(uint16 channelID) {
	if (channelID > 31 || _channels[channelID].stream == nullptr)
		return;

	Channel &chan = _channels[channelID];
	chan.stream->seek(0);

	_mixer->playStream(	chan.type,
						&chan.handle,
						Audio::makeLoopingAudioStream(chan.stream, chan.numLoops),
						channelID,
						chan.volume * 255 / 100,
						0, DisposeAfterUse::NO);

	if (chan.isPanning) {
		calculatePan(channelID);
	}
}

void SoundManager::playSound(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		playSound(description.channelID);
	}
}

void SoundManager::playSound(const Common::String &chunkName) {
	const SoundDescription &desc = _commonSounds[chunkName];

	if (!isSoundPlaying(desc)) {
		loadSound(desc);
	}

	playSound(desc);
}

void SoundManager::pauseSound(uint16 channelID, bool pause) {
	if (channelID > 31)
		return;

	if (isSoundPlaying(channelID)) {
		g_system->getMixer()->pauseHandle(_channels[channelID].handle, pause);
	}
}

void SoundManager::pauseSound(const SoundDescription &description, bool pause) {
	if (description.name != "NO SOUND") {
		pauseSound(description.channelID, pause);
	}
}

void SoundManager::pauseSound(const Common::String &chunkName, bool pause) {
	pauseSound(_commonSounds[chunkName], pause);
}

bool SoundManager::isSoundPlaying(uint16 channelID) const {
	if (channelID > 31)
		return false;

	return _mixer->isSoundHandleActive(_channels[channelID].handle);
}

bool SoundManager::isSoundPlaying(const SoundDescription &description) const {
	if (description.name == "NO SOUND") {
		return false;
	} else {
		return isSoundPlaying(description.channelID);
	}
}

bool SoundManager::isSoundPlaying(const Common::String &chunkName) const {
	return isSoundPlaying(_commonSounds[chunkName]);
}

void SoundManager::stopSound(uint16 channelID) {
	if (channelID > 31)
		return;

	Channel &chan = _channels[channelID];

	if (isSoundPlaying(channelID)) {
		_mixer->stopHandle(chan.handle);
	}
	chan.name = Common::String();
	delete chan.stream;
	chan.stream = nullptr;
}

void SoundManager::stopSound(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		stopSound(description.channelID);
	}
}

void SoundManager::stopSound(const Common::String &chunkName) {
	stopSound(_commonSounds[chunkName]);
}

// Returns whether the exception was skipped
void SoundManager::stopAllSounds() {
	for (uint i = 0; i < 31; ++i) {
		stopSound(i);
	}
}

void SoundManager::calculatePan(uint16 channelID) {
	uint16 viewportFrameID = NancySceneState.getSceneInfo().frameID;
	const State::Scene::SceneSummary &sceneSummary = NancySceneState.getSceneSummary();
	Channel &chan = _channels[channelID];
	if (chan.isPanning) {
		switch (sceneSummary.totalViewAngle) {
		case 180:
			_mixer->setChannelBalance(chan.handle, CLIP<int32>((viewportFrameID - chan.panAnchorFrame) * sceneSummary.soundPanPerFrame * 364, -32768, 32767) / 256);
			break;
		case 360: {
			int16 adjustedViewportFrame = viewportFrameID - chan.panAnchorFrame;
			if (adjustedViewportFrame < 0) {
				adjustedViewportFrame += sceneSummary.numberOfVideoFrames;
			}

			// Divide the virtual space into quarters
			uint16 q1 = sceneSummary.numberOfVideoFrames / 4;
			uint16 q2 = sceneSummary.numberOfVideoFrames / 2;
			uint16 q3 = sceneSummary.numberOfVideoFrames * 3 / 4;

			float balance;

			if (adjustedViewportFrame < q1) {
				balance = (float)adjustedViewportFrame / q1;
				balance *= 32767;
				balance = 32768 - balance;
			} else if (adjustedViewportFrame < q2) {
				balance = (float)(adjustedViewportFrame - q1) / q1;
				balance *= 32767;
			} else if (adjustedViewportFrame < q3) {
				balance = (float)(adjustedViewportFrame - q2) / q1;
				balance *= 32767;
				balance += 32768;
			} else {
				balance = (float)(adjustedViewportFrame - q3) / q1;
				balance *= 32767;
				balance = 65535 - balance;
			}

			// The original engine's algorithm is broken and results in flipped
			// stereo; the following line fixes this bug
			balance = 65535 - balance;

			_mixer->setChannelBalance(chan.handle, (balance - 32768) / 256);
			break;
			}
		default:
			_mixer->setChannelBalance(chan.handle, 0);
			break;
		}
	}
}

void SoundManager::calculatePan(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		calculatePan(description.channelID);
	}
}

void SoundManager::calculatePanForAllSounds() {
	for (uint i = 0; i < 31; ++i) {
		calculatePan(i);
	}
}

void SoundManager::stopAndUnloadSpecificSounds() {
	if (g_nancy->getGameType() == kGameTypeVampire && Nancy::State::Map::hasInstance()) {
		// Don't stop the map sound in certain scenes
		uint nextScene = NancySceneState.getNextSceneInfo().sceneID;
		if (nextScene != 0 && (nextScene < 15 || nextScene > 27)) {
			stopSound(NancyMapState.getSound());
		}
	}

	for (uint i = 0; i < 10; ++i) {
		stopSound(i);
	}

	stopSound("MSND");
}

void SoundManager::initSoundChannels() {
	// Channel types are hardcoded in the original engine
	for (uint i = 0; i < 31; ++i) {
		_channels[i].type = channelSoundTypes[i];
	}
}

} // End of namespace Nancy
