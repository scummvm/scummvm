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
 */

#include "engines/nancy/sound.h"
#include "engines/nancy/nancy.h"

#include "common/system.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/substream.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"

namespace Nancy {

enum SoundType {
	kSoundTypeRaw,
	kSoundTypeOgg
};

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

	uint16 numChannels, bitsPerSample;
	uint32 samplesPerSec, size;
	SoundType type;

	if (Common::String(buf) == "Her Interactive Sound") {
		// Early HIS file
		if (!readWaveHeader(stream, type, numChannels, samplesPerSec, bitsPerSample, size))
			return 0;
	} else if (Common::String(buf) == "HIS") {
		stream->seek(4);
		if (!readHISHeader(stream, type, numChannels, samplesPerSec, bitsPerSample, size))
			return 0;
	}

	byte flags = 0;
	if (type == kSoundTypeRaw) {
		if (bitsPerSample == 8)		// 8 bit data is unsigned
			flags |= Audio::FLAG_UNSIGNED;
		else if (bitsPerSample == 16)	// 16 bit data is signed little endian
			flags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
		else {
			warning("Unsupported bitsPerSample %d found in HIS file", bitsPerSample);
			return 0;
		}

		if (numChannels == 2)
			flags |= Audio::FLAG_STEREO;
		else if (numChannels != 1) {
			warning("Unsupported number of channels %d found in HIS file", numChannels);
			return 0;
		}

		// Raw PCM, make sure the last packet is complete
		uint sampleSize = (flags & Audio::FLAG_16BITS ? 2 : 1) * (flags & Audio::FLAG_STEREO ? 2 : 1);
		if (size % sampleSize != 0) {
			warning("Trying to play an HIS file with an incomplete PCM packet");
			size &= ~(sampleSize - 1);
		}
	}

	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(stream, stream->pos(), stream->pos() + size, disposeAfterUse);

	if (type == kSoundTypeRaw)
		return Audio::makeRawStream(subStream, samplesPerSec, flags, DisposeAfterUse::YES);
	else
		return Audio::makeVorbisStream(subStream, DisposeAfterUse::YES);
}

SoundManager::SoundManager() {
	_mixer = g_system->getMixer();

	initSoundChannels();
}

SoundManager::~SoundManager() {
	stopAllSounds();
}

void SoundManager::loadSound(const SoundDescription &description) {
	if (description.name == "NO SOUND") {
		return;
	}

	if (_mixer->isSoundHandleActive(_channels[description.channelID].handle)) {
		_mixer->stopHandle(_channels[description.channelID].handle);
	}

	delete _channels[description.channelID].stream;
	_channels[description.channelID].stream = nullptr;

	_channels[description.channelID].name = description.name;
	_channels[description.channelID].numLoops = description.numLoops;
	_channels[description.channelID].volume = description.volume;


	Common::SeekableReadStream *file = SearchMan.createReadStreamForMember(description.name + ".his");
	if (file) {
		_channels[description.channelID].stream = makeHISStream(file, DisposeAfterUse::YES);
	}
}

void SoundManager::playSound(uint16 channelID) {
	if (channelID > 32 || _channels[channelID].stream == 0)
		return;

	_channels[channelID].stream->seek(0);

	_mixer->playStream(	_channels[channelID].type,
						&_channels[channelID].handle,
						Audio::makeLoopingAudioStream(_channels[channelID].stream, _channels[channelID].numLoops),
						channelID,
						_channels[channelID].volume * 255 / 100,
						0, DisposeAfterUse::NO);
}

void SoundManager::playSound(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		playSound(description.channelID);
	}
}

void SoundManager::pauseSound(uint16 channelID, bool pause) {
	if (channelID > 32)
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

bool SoundManager::isSoundPlaying(uint16 channelID) const {
	if (channelID > 32)
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

void SoundManager::stopSound(uint16 channelID) {
	if (channelID > 32)
		return;

	if (isSoundPlaying(channelID)) {
		_mixer->stopHandle(_channels[channelID].handle);
	}
	_channels[channelID].name = Common::String();
	delete _channels[channelID].stream;
	_channels[channelID].stream = nullptr;
}

void SoundManager::stopSound(const SoundDescription &description) {
	if (description.name != "NO SOUND") {
		stopSound(description.channelID);
	}
}

// Returns whether the exception was skipped
void SoundManager::stopAllSounds() {
	for (uint i = 0; i < 32; ++i) {
		stopSound(i);
	}
}

void SoundManager::stopAndUnloadSpecificSounds() {
	// TODO missing if

	for (uint i = 0; i < 10; ++i) {
		stopSound(i);
	}
}

void SoundManager::initSoundChannels() {
	// Original engine hardcoded these and so do we
	_channels[7].type = Audio::Mixer::kSpeechSoundType;
	_channels[8].type = Audio::Mixer::kSpeechSoundType;
	_channels[30].type = Audio::Mixer::kSpeechSoundType;

	_channels[0].type = Audio::Mixer::kMusicSoundType;
	_channels[1].type = Audio::Mixer::kMusicSoundType;
	_channels[2].type = Audio::Mixer::kMusicSoundType;
	_channels[27].type = Audio::Mixer::kMusicSoundType;
	_channels[28].type = Audio::Mixer::kMusicSoundType;
	_channels[29].type = Audio::Mixer::kMusicSoundType;
	_channels[19].type = Audio::Mixer::kMusicSoundType;

	_channels[3].type = Audio::Mixer::kSFXSoundType;
	_channels[4].type = Audio::Mixer::kSFXSoundType;
	_channels[5].type = Audio::Mixer::kSFXSoundType;
	_channels[6].type = Audio::Mixer::kSFXSoundType;
	_channels[20].type = Audio::Mixer::kSFXSoundType;
	_channels[21].type = Audio::Mixer::kSFXSoundType;
	_channels[25].type = Audio::Mixer::kSFXSoundType;
	_channels[26].type = Audio::Mixer::kSFXSoundType;
	_channels[24].type = Audio::Mixer::kSFXSoundType;
	_channels[23].type = Audio::Mixer::kSFXSoundType;
	_channels[22].type = Audio::Mixer::kSFXSoundType;
	_channels[31].type = Audio::Mixer::kSFXSoundType;
	_channels[18].type = Audio::Mixer::kSFXSoundType;
	_channels[17].type = Audio::Mixer::kSFXSoundType;

	_channels[9].type = Audio::Mixer::kPlainSoundType;
	_channels[10].type = Audio::Mixer::kPlainSoundType;
	_channels[11].type = Audio::Mixer::kPlainSoundType;
	_channels[12].type = Audio::Mixer::kPlainSoundType;
	_channels[13].type = Audio::Mixer::kPlainSoundType;
	_channels[14].type = Audio::Mixer::kPlainSoundType;
	_channels[15].type = Audio::Mixer::kPlainSoundType;
	_channels[16].type = Audio::Mixer::kPlainSoundType;

}

} // End of namespace Nancy
