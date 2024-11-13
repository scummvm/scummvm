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
#include "common/random.h"

#include "math/quat.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/iff.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/map.h"

namespace Nancy {

enum SoundType {
	kSoundTypeDiamondware,
	kSoundTypeRaw,
	kSoundTypeOgg
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

uint getAdjustedVolume(uint volume) {
	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		return 10 + (volume * 90) / 100;
	} else {
		return volume;
	}
}

Audio::SeekableAudioStream *SoundManager::makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 overrideSamplesPerSec) {
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

	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(
		stream, stream->pos(),
		type == kSoundTypeOgg ? stream->size() : stream->pos() + size,
		disposeAfterUse);

	if (type == kSoundTypeRaw || type == kSoundTypeDiamondware)
		return Audio::makeRawStream(subStream, overrideSamplesPerSec == 0 ? samplesPerSec : overrideSamplesPerSec, flags, DisposeAfterUse::YES);
	else
		return Audio::makeVorbisStream(subStream, DisposeAfterUse::YES);
}

SoundManager::SoundManager() : _shouldRecalculate(false), _mixer(g_system->getMixer()) {}

void SoundManager::loadCommonSounds(IFF *boot) {
	// Persistent sounds that are used across the engine. These originally get loaded inside Logo
	Common::String chunkNames[] = {
		"CANT", "CURT", "GLOB", "SLID", "BULS", "BUDE", "BUOK", "TH1", "TH2",
	};

	Common::SeekableReadStream *chunk = nullptr;
	for (auto const &s : chunkNames) {
		chunk = boot->getChunkStream(s);
		if (chunk) {
			SoundDescription &desc = _commonSounds.getOrCreateVal(s);
			desc.readNormal(*chunk);
			g_nancy->_sound->loadSound(desc);
			_channels[desc.channelID].isPersistent = true;

			delete chunk;
		}
	}

	// Menu sound is stored differently
	chunk = boot->getChunkStream("MSND"); // channel 28
	if (chunk) {
		SoundDescription &desc = _commonSounds.getOrCreateVal("MSND");
		desc.readMenu(*chunk);
		g_nancy->_sound->loadSound(desc);
		_channels[desc.channelID].isPersistent = true;

		delete chunk;
	}
}

SoundManager::~SoundManager() {
	stopAllSounds();
}

void SoundManager::loadSound(const SoundDescription &description, SoundEffectDescription **effectData, bool forceReload) {
	if (description.name == "NO SOUND") {
		return;
	}

	Channel &existing = _channels[description.channelID];
	if (!forceReload && existing.stream != nullptr) {
		// There's a channel already loaded. Check if we're trying to reload the exact same sound
		if (	description.name == existing.name &&
				description.numLoops == existing.numLoops &&
				description.playCommands == existing.playCommands) {

			// When the same sound is already playing at a different volume, adjust to new volume (nancy2 scene 599)
			if (existing.volume != getAdjustedVolume(description.volume)) {
				setVolume(description, description.volume);
			}

			return;
		}
	}

	if (_mixer->isSoundHandleActive(_channels[description.channelID].handle)) {
		_mixer->stopHandle(_channels[description.channelID].handle);
	}

	Channel &chan = _channels[description.channelID];

	delete chan.streamForMixer;
	chan.stream = nullptr;
	chan.streamForMixer = nullptr;

	chan.name = description.name;
	chan.playCommands = description.playCommands;
	chan.numLoops = description.numLoops;
	chan.volume = description.volume;
	chan.panAnchorFrame = description.panAnchorFrame;
	chan.isPanning = description.isPanning;

	if (effectData) {
		// Channel takes ownership of the effect data
		delete chan.effectData;
		chan.effectData = *effectData;
		*effectData = nullptr;
	}

	Common::Path path(description.name + (g_nancy->getGameType() == kGameTypeVampire ? ".dwd" : ".his"));
	Common::SeekableReadStream *file = SearchMan.createReadStreamForMember(path);
	if (file) {
		uint numLoops = chan.numLoops;
		if (chan.playCommands & kPlayRandomTime) {
			// We want to add randomized time delays between repeats, which is not doable with
			// a simple LoopingAudioStream. The delays are added in soundEffectMaintenance();
			numLoops = 1;

			// Decrement the number of loops since we start playing immediately after
			--chan.numLoops;
		}
		chan.stream = makeHISStream(file, DisposeAfterUse::YES, description.samplesPerSec);
		chan.streamForMixer = Audio::makeLoopingAudioStream(chan.stream, numLoops);
	}
}

void SoundManager::playSound(uint16 channelID) {
	if (channelID >= _channels.size() || _channels[channelID].stream == nullptr || isSoundPlaying(channelID))
		return;

	Channel &chan = _channels[channelID];
	chan.stream->seek(0);

	// Set a minimum volume (10 percent was chosen arbitrarily, but sounds reasonably close)
	// Fix for nancy3 scene 6112, but NOT a hack; the original engine also set a minimum volume for all sounds
	chan.volume = getAdjustedVolume(chan.volume);

	// Init 3D sound
	if (chan.playCommands & ~kPlaySequential && chan.effectData) {
		uint16 playCommands = chan.playCommands;

		if (playCommands & kPlayRandomPosition) {
			auto *rand = g_nancy->_randomSource;
			chan.position.set(
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinX, chan.effectData->randomMoveMaxX),
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinY, chan.effectData->randomMoveMaxY),
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinZ, chan.effectData->randomMoveMaxZ));
		}

		if (playCommands == kPlaySequentialPosition) {
			chan.position.set(chan.effectData->fixedPosX, chan.effectData->fixedPosY, chan.effectData->fixedPosZ);
		} else if (playCommands == kPlaySequentialFrameAnchor) {
			// Doesn't seem to be used so we skip implementing it
			warning("Sound play command kPlaySequentialFrameAnchor not implemented");
		} else if (playCommands == kPlayMoveLinear) {
			chan.position.set(chan.effectData->linearMoveStartX, chan.effectData->linearMoveStartY, chan.effectData->linearMoveStartZ);
			chan.positionDelta.set(chan.effectData->linearMoveEndX, chan.effectData->linearMoveEndY, chan.effectData->linearMoveEndZ);
			chan.positionDelta -= chan.position;
			chan.positionDelta /= chan.effectData->numMoveSteps;
			chan.nextStepTime = g_nancy->getTotalPlayTime() + chan.effectData->moveStepTime;
			chan.stepsLeft = chan.effectData->numMoveSteps;
		} else if (playCommands == kPlayRandomMove) {
			auto *rand = g_nancy->_randomSource;
			chan.position.set(
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinX, chan.effectData->randomMoveMaxX),
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinY, chan.effectData->randomMoveMaxY),
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinZ, chan.effectData->randomMoveMaxZ));

			chan.positionDelta.set(
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinX, chan.effectData->randomMoveMaxX),
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinY, chan.effectData->randomMoveMaxY),
				rand->getRandomNumberRngSigned(chan.effectData->randomMoveMinZ, chan.effectData->randomMoveMaxZ));

			chan.positionDelta -= chan.position;
			chan.positionDelta /= chan.effectData->numMoveSteps;
			chan.nextStepTime = g_nancy->getTotalPlayTime() + chan.effectData->moveStepTime;
			chan.stepsLeft = chan.effectData->numMoveSteps;
		} else if (playCommands == kPlayMoveCircular) {
			chan.position.set(chan.effectData->rotateMoveStartX, chan.effectData->rotateMoveStartY, chan.effectData->rotateMoveStartZ);
			chan.nextStepTime = g_nancy->getTotalPlayTime() + chan.effectData->moveStepTime;
			chan.stepsLeft = chan.effectData->numMoveSteps;
		}
	}

	_mixer->playStream(	chan.type,
						&chan.handle,
						chan.streamForMixer,
						channelID,
						(int)chan.volume * 255 / 100,
						0, DisposeAfterUse::NO);

	soundEffectMaintenance(channelID, true);
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
	if (channelID >= _channels.size())
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

void SoundManager::pauseAllSounds(bool pause) {
	_mixer->pauseAll(pause);
}

bool SoundManager::isSoundPlaying(uint16 channelID) const {
	if (channelID >= _channels.size() || !_channels[channelID].stream)
		return false;

	const Channel &chan = _channels[channelID];
	if (chan.playCommands & kPlayRandomTime) {
		return _mixer->isSoundHandleActive(chan.handle) || chan.numLoops != 0;
	} else {
		return _mixer->isSoundHandleActive(chan.handle);
	}
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
	if (channelID >= _channels.size())
		return;

	Channel &chan = _channels[channelID];

	if (isSoundPlaying(channelID)) {
		_mixer->stopHandle(chan.handle);
	}

	// Persistent sounds only stop playing but do not get unloaded
	if (!chan.isPersistent) {
		chan.name = Common::String();
		delete chan.streamForMixer;
		chan.stream = nullptr;
		chan.streamForMixer = nullptr;
		delete chan.effectData;
		chan.effectData = nullptr;
		chan.position.set(0, 0, 0);
		chan.positionDelta.set(0, 0, 0);
		chan.stepsLeft = 0;
		chan.nextStepTime = 0;
	}
}

void SoundManager::stopSound(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		stopSound(description.channelID);
	}
}

void SoundManager::stopSound(const Common::String &chunkName) {
	stopSound(_commonSounds[chunkName]);
}

void SoundManager::stopAllSounds() {
	for (uint i = 0; i < _channels.size(); ++i) {
		stopSound(i);
	}
}

byte SoundManager::getVolume(uint16 channelID) {
	if (channelID >= _channels.size())
		return 0;

	return _mixer->getChannelVolume(_channels[channelID].handle);
}

byte SoundManager::getVolume(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		return getVolume(description.channelID);
	}

	return 0;
}

byte SoundManager::getVolume(const Common::String &chunkName) {
	return getVolume(_commonSounds[chunkName]);
}

void SoundManager::setVolume(uint16 channelID, uint16 volume) {
	if (channelID >= _channels.size() || !isSoundPlaying(channelID))
		return;

	_mixer->setChannelVolume(_channels[channelID].handle, getAdjustedVolume(volume) * 255 / 100);
}

void SoundManager::setVolume(const SoundDescription &description, uint16 volume) {
	if (description.name != "NO SOUND") {
		setVolume(description.channelID, volume);
	}
}

void SoundManager::setVolume(const Common::String &chunkName, uint16 volume) {
	setVolume(_commonSounds[chunkName], volume);
}

uint32 SoundManager::getRate(uint16 channelID) {
	if (channelID >= _channels.size())
		return 0;

	return _mixer->getChannelRate(_channels[channelID].handle);
}

uint32 SoundManager::getRate(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		return getRate(description.channelID);
	}

	return 0;
}

uint32 SoundManager::getRate(const Common::String &chunkName) {
	return getRate(_commonSounds[chunkName]);
}

uint32 SoundManager::getBaseRate(uint16 channelID) {
	if (channelID >= _channels.size() || !_channels[channelID].stream)
		return 0;

	return _channels[channelID].stream->getRate();
}

uint32 SoundManager::getBaseRate(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		return getBaseRate(description.channelID);
	}

	return 0;
}

uint32 SoundManager::getBaseRate(const Common::String &chunkName) {
	return getBaseRate(_commonSounds[chunkName]);
}

void SoundManager::setRate(uint16 channelID, uint32 rate) {
	if (channelID >= _channels.size())
		return;

	_mixer->setChannelRate(_channels[channelID].handle, rate);
}

void SoundManager::setRate(const SoundDescription &description, uint32 rate) {
	if (description.name != "NO SOUND") {
		setRate(description.channelID, rate);
	}
}

void SoundManager::setRate(const Common::String &chunkName, uint32 rate) {
	setRate(_commonSounds[chunkName], rate);
}

Audio::Timestamp SoundManager::getLength(uint16 channelID) {
	if (channelID >= _channels.size() || _channels[channelID].stream == nullptr) {
		return Audio::Timestamp();
	}

	return _channels[channelID].stream->getLength().convertToFramerate(getRate(channelID));
}

Audio::Timestamp SoundManager::getLength(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		return getLength(description.channelID);
	}

	return Audio::Timestamp();
}

Audio::Timestamp SoundManager::getLength(const Common::String &chunkName) {
	return getLength(_commonSounds[chunkName]);
}

void SoundManager::recalculateSoundEffects() {
	_shouldRecalculate = true;

	_positionLerp = 0;

	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		const Nancy::State::Scene::SceneSummary &sceneSummary = NancySceneState.getSceneSummary();
		SceneChangeDescription &sceneInfo = NancySceneState.getSceneInfo();
		Math::Vector3d rotatedFrontVector = NancySceneState.getSceneInfo().listenerFrontVector;
		rotatedFrontVector.normalize();

		int rotation = sceneInfo.frontVectorFrameID < sceneInfo.frameID ?
			360 - (sceneInfo.frameID - sceneInfo.frontVectorFrameID) * sceneSummary.degreesPerRotation :
			(sceneInfo.frontVectorFrameID - sceneInfo.frameID) * sceneSummary.degreesPerRotation;

		Math::Quaternion quat = Math::Quaternion::yAxis(rotation);
		quat.transform(rotatedFrontVector);

		_orientation = rotatedFrontVector;

		for (uint i = 0; i < 3; ++i) {
			if (abs(_orientation.getValue(i)) < Math::epsilon) {
				_orientation.setValue(i, 0);
			}
		}
	}
}

void SoundManager::stopAndUnloadSceneSpecificSounds() {
	byte numSSChans = g_nancy->getStaticData().soundChannelInfo.numSceneSpecificChannels;

	if (g_nancy->getGameType() == kGameTypeVampire && Nancy::State::Map::hasInstance()) {
		// Don't stop the map sound in certain scenes
		uint nextScene = NancySceneState.getNextSceneInfo().sceneID;
		if (nextScene != 0 && (nextScene < 15 || nextScene > 27)) {
			stopSound(NancyMapState.getSound());
		}
	}

	for (uint i = 0; i < numSSChans; ++i) {
		stopSound(i);
	}

	stopSound("MSND");
}

void SoundManager::pauseSceneSpecificSounds(bool pause) {
	byte numSSChans = g_nancy->getStaticData().soundChannelInfo.numSceneSpecificChannels;
	if (g_nancy->getGameType() == kGameTypeVampire && Nancy::State::Map::hasInstance()) {
		if (!pause || g_nancy->getState() != NancyState::kMap) {
			// Stop the map sound in certain scenes
			uint currentScene = NancySceneState.getSceneInfo().sceneID;
			if (currentScene == 0 || (currentScene >= 15 && currentScene <= 27)) {
				g_nancy->_sound->pauseSound(NancyMapState.getSound(), pause);
			}
		}
	}

	for (uint i = 0; i < numSSChans; ++i) {
		g_nancy->_sound->pauseSound(i, pause);
	}
}

void SoundManager::initSoundChannels() {
	const SoundChannelInfo &channelInfo = g_nancy->getStaticData().soundChannelInfo;

	_channels.resize(channelInfo.numChannels);

	for (const short id : channelInfo.speechChannels) {
		_channels[id].type = Audio::Mixer::SoundType::kSpeechSoundType;
	}

	for (const short id : channelInfo.musicChannels) {
		_channels[id].type = Audio::Mixer::SoundType::kMusicSoundType;
	}

	for (const short id : channelInfo.sfxChannels) {
		_channels[id].type = Audio::Mixer::SoundType::kSFXSoundType;
	}
}

SoundManager::Channel::~Channel() {
	delete streamForMixer;
	delete effectData;
}

void SoundManager::soundEffectMaintenance() {
	// Interpolate position and rotation when scene has changed to avoid audible chop in sound
	if (_position != NancySceneState.getSceneSummary().listenerPosition && _positionLerp == 0) {
		++_positionLerp;
	}

	if (_positionLerp > 1) {
		++_positionLerp;
		if (_positionLerp > 10) {
			_position = NancySceneState.getSceneSummary().listenerPosition;
			_positionLerp = 0;
		}
	}

	for (uint i = 0; i < _channels.size(); ++i) {
		soundEffectMaintenance(i);
	}

	_shouldRecalculate = false;
}

void SoundManager::soundEffectMaintenance(uint16 channelID, bool force) {
	if (channelID >= _channels.size() || !isSoundPlaying(channelID))
		return;

	uint32 gameTime = g_nancy->getTotalPlayTime();
	Channel &chan = _channels[channelID];

	// Handle sound effects and 3D sound, which started being used from nancy3.
	// The original engine used DirectSound 3D, whose effects are only approximated.
	// In particular, there are some slight but noticeable differences in panning
	bool hasStepped = force;
	if (g_nancy->getGameType() >= 3 && chan.effectData) {
		uint16 playCommands = chan.playCommands;
		SoundEffectDescription *effectData = chan.effectData;

		// Handle randomized time delay between repeats
		if (playCommands & kPlayRandomTime && !_mixer->isSoundHandleActive(chan.handle) && chan.numLoops != 0) {
			if (chan.nextRepeatTime == 0) {
				// Channel just stopped playing, add a randomized delay
				chan.nextRepeatTime = g_nancy->_randomSource->getRandomNumberRng(effectData->minTimeDelay, effectData->maxTimeDelay) + gameTime;
			} else if (chan.nextRepeatTime < gameTime) {
				// Delay is over, start playing again
				_mixer->playStream(	chan.type,
						&chan.handle,
						chan.stream,
						channelID,
						chan.volume * 255 / 100,
						0, DisposeAfterUse::NO);

				--chan.numLoops;
				chan.nextRepeatTime = 0;
			}
		}

		// Move sound in space
		if (playCommands & kPlayMoveLinear && chan.stepsLeft && gameTime > chan.nextStepTime) {
			chan.nextStepTime = gameTime + chan.effectData->moveStepTime;
			--chan.stepsLeft;
			hasStepped = true;

			if (playCommands == kPlayMoveCircular) {
				// No real uses at least up to nancy5, so this is untested
				Math::Quaternion quat;
				switch (chan.effectData->rotateMoveAxis) {
				case kRotateAroundX:
					quat = Math::Quaternion::xAxis(360.0 / chan.effectData->numMoveSteps);
					break;
				case kRotateAroundY:
					quat = Math::Quaternion::yAxis(360.0 / chan.effectData->numMoveSteps);
					break;
				case kRotateAroundZ:
					quat = Math::Quaternion::zAxis(360.0 / chan.effectData->numMoveSteps);
					break;
				}

				quat.transform(chan.position);
			} else {
				chan.position += chan.positionDelta;
			}
		}
	}

	// Check if the player has moved OR if the sound itself has moved, OR, if we're still interpolating
	// Also, make sure we don't accidentally create a Scene state during game startup
	if (!State::Scene::hasInstance() || (!_shouldRecalculate && !hasStepped && _positionLerp == 0)) {
		return;
	}

	uint16 viewportFrameID = NancySceneState.getSceneInfo().frameID;

	// Old panning algorithm, used in The Vampire Diaries
	if (g_nancy->getGameType() <= kGameTypeNancy2 && chan.isPanning) {
		const State::Scene::SceneSummary &sceneSummary = NancySceneState.getSceneSummary();

		switch (sceneSummary.totalViewAngle) {
		case 180:
			_mixer->setChannelBalance(chan.handle, CLIP<int32>((viewportFrameID - chan.panAnchorFrame) * sceneSummary.degreesPerRotation * 364, -32768, 32767) / 256);
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

	// Panning/volume/rate adjustment used in nancy3 and up. Originally handled by DirectSound 3D
	if (g_nancy->getGameType() >= 3 && chan.effectData &&
			(chan.playCommands & ~kPlaySequential) & (kPlaySequentialFrameAnchor | kPlayRandomPosition | kPlayMoveLinear)) {

		// Interpolate position when we've changed scenes
		Math::Vector3d listenerPos = Math::Vector3d::interpolate(_position, NancySceneState.getSceneSummary().listenerPosition, (float)_positionLerp / 10.0);
		float dist = listenerPos.getDistanceTo(chan.position);
		float volume;

		// Panning is linear, so we calculate it from the difference in degrees
		Math::Vector3d relativeSoundPos = chan.position - listenerPos;
		float pan = Math::Vector3d::angle(_orientation, relativeSoundPos.getNormalized()).getDegrees();
		if (pan > 90) {
			pan = 90 - (pan - 90);
		}

		pan /= 90;

		if (Math::Vector3d::crossProduct(_orientation, relativeSoundPos).y() < 0) {
			pan = -pan;
		}

		// Attenuate sound based on distance
		if (dist <= chan.effectData->minDistance) {
			volume = 255;
		} else if (dist >= chan.effectData->maxDistance) {
			volume = 255.0 / (2 * log2f(chan.effectData->maxDistance - chan.effectData->minDistance + 1));
		} else {
			float dlog = (2 * log2f(dist - chan.effectData->minDistance + 1));
			volume = 255.0 / dlog;

			// Sounds that are closer to the listener shouldn't pan as hard
			// note: slightly inaccurate, compare the ticking sound in nancy3 scene 4015
			pan -= pan / dlog;
		}

		// (Non-linearly) interpolate pan as well
		if (_positionLerp) {
			float lastPan = _mixer->getChannelBalance(chan.handle) / 127.0;
			pan = lastPan + (pan - lastPan) * ((float)_positionLerp / 10.0);
		}

		// Doppler effect is affected by the velocities of the source and listener,
		// as projected onto the vector between source and listener
		Math::Vector3d listenerToSource = chan.position - listenerPos;
		if (listenerToSource.isZero()) {
			return;
		}

		float projectedListenerVelocity, projectedSrcVelocity;

		float soundSpeed = 343.0;

		// It appears the original engine's devs either didn't know or didn't care
		// what the velocity parameters do, so they used the doubled orientation vector
		// as listener velocity, and the doubled position vector of the sound as source velocity.
		// This results in physically incorrect behavior which we replicate
		projectedListenerVelocity = Math::Vector3d::dotProduct(_orientation * 2, listenerToSource) / listenerToSource.length();
		projectedSrcVelocity = -Math::Vector3d::dotProduct(chan.position * 2, listenerToSource) / listenerToSource.length();

		// Calculate the final rate of the sound with doppler effect applied
		uint32 rate = chan.stream->getRate() * (1 + projectedListenerVelocity / soundSpeed) / (1 - projectedSrcVelocity / soundSpeed);

		_mixer->setChannelVolume(chan.handle, ((byte)volume * chan.volume) / 100);
		_mixer->setChannelBalance(chan.handle, pan * 127);
		_mixer->setChannelRate(chan.handle, rate);
	}
}

} // End of namespace Nancy
