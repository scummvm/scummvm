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

// SNDDecoder based on snd2wav by Abraham Macias Paredes
// https://github.com/System25/drxtract/blob/master/snd2wav
// License: GNU GPL v2 (see COPYING file for details)

#include "common/file.h"
#include "common/substream.h"

#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "audio/softsynth/pcspk.h"
#include "audio/decoders/aiff.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/castmember.h"
#include "director/sound.h"

namespace Director {

DirectorSound::DirectorSound(DirectorEngine *vm) : _vm(vm) {
	uint numChannels = 2;
	if (g_director->getVersion() >= 400) {
		numChannels = 4;
	}

	for (uint i = 0; i < numChannels; i++) {
		_channels.push_back(SoundChannel());
	}

	_mixer = g_system->getMixer();

	_speaker = new Audio::PCSpeaker();
	_mixer->playStream(Audio::Mixer::kSFXSoundType,
		&_pcSpeakerHandle, _speaker, -1, 50, 0, DisposeAfterUse::NO, true);

	_enable = true;
	_puppet = false;
}

DirectorSound::~DirectorSound() {
	this->stopSound();
	delete _speaker;
}

SoundChannel *DirectorSound::getChannel(uint8 soundChannel) {
	if (!isChannelValid(soundChannel))
		return nullptr;
	return &_channels[soundChannel - 1];
}

void DirectorSound::playFile(Common::String filename, uint8 soundChannel) {
	if (debugChannelSet(-1, kDebugFast))
		return;

	AudioFileDecoder af(filename);
	Audio::AudioStream *sound = af.getAudioStream(false, DisposeAfterUse::YES);

	cancelFade(soundChannel);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel - 1].handle, sound, -1, getChannelVolume(soundChannel));
}

void DirectorSound::playMCI(Audio::AudioStream &stream, uint32 from, uint32 to) {
	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(&stream);
	Audio::SubSeekableAudioStream *subSeekStream = new Audio::SubSeekableAudioStream(seekStream, Audio::Timestamp(from, seekStream->getRate()), Audio::Timestamp(to, seekStream->getRate()));

	// TODO: make sound enable settings work on this one
	_mixer->stopHandle(_scriptSound);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_scriptSound, subSeekStream);
}

uint8 DirectorSound::getChannelVolume(uint8 soundChannel) {
	return _enable ? _channels[soundChannel - 1].volume : 0;
}

void DirectorSound::playStream(Audio::AudioStream &stream, uint8 soundChannel) {
	if (!isChannelValid(soundChannel))
		return;

	cancelFade(soundChannel);
	_mixer->stopHandle(_channels[soundChannel - 1].handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel - 1].handle, &stream, -1, getChannelVolume(soundChannel));
}

void DirectorSound::playCastMember(CastMemberID memberID, uint8 soundChannel, bool allowRepeat) {
	if (!isChannelValid(soundChannel))
		return;

	if (memberID.member == 0) {
		stopSound(soundChannel);
	} else {
		CastMember *soundCast = _vm->getCurrentMovie()->getCastMember(memberID);
		if (soundCast) {
			if (soundCast->_type != kCastSound) {
				warning("DirectorSound::playCastMember: attempted to play a non-SoundCastMember %s", memberID.asString().c_str());
			} else {
				if (!allowRepeat && checkLastPlayCast(soundChannel, memberID))
					return;
				bool looping = ((SoundCastMember *)soundCast)->_looping;
				AudioDecoder *ad = ((SoundCastMember *)soundCast)->_audio;
				if (!ad) {
					warning("DirectorSound::playCastMember: no audio data attached to %s", memberID.asString().c_str());
					return;
				}

				Audio::AudioStream *as;
				as = ad->getAudioStream(looping);

				if (!as) {
					warning("DirectorSound::playCastMember: audio data failed to load from cast");
					return;
				}
				playStream(*as, soundChannel);
				setLastPlayCast(soundChannel, memberID);
			}
		} else {
			warning("DirectorSound::playCastMember: couldn't find %s", memberID.asString().c_str());
		}
	}
}

void DirectorSound::setSoundEnabled(bool enabled) {
	if (_enable == enabled)
		return;
	if (!enabled)
		stopSound();
	_enable = enabled;
}

void SNDDecoder::loadExternalSoundStream(Common::SeekableReadStreamEndian &stream) {
	_size = stream.readUint32BE();

	uint16 sampleRateFlag = stream.readUint16();
	/*uint16 unk2 = */ stream.readUint16();

	_data = (byte *)malloc(_size);
	stream.read(_data, _size);

	switch (sampleRateFlag) {
	case 1:
		_rate = 22254;
		break;
	case 2:
		_rate = 11127;
		break;
	case 3:
		_rate = 7300;
		break;
	case 4:
		_rate = 5500;
		break;
	default:
		warning("DirectorSound::loadExternalSoundStream: Can't handle sampleRateFlag %d, using default one", sampleRateFlag);
		_rate = 5500;
		break;
	}

	// this may related to the unk2 flag
	// TODO: figure out how to read audio flags
	_flags = Audio::FLAG_UNSIGNED;
	_channels = 1;
}

void DirectorSound::registerFade(uint8 soundChannel, bool fadeIn, int ticks) {
	if (!isChannelValid(soundChannel))
		return;

	// sound enable is not working on fade sounds, so we just return directly when sounds are not enabling
	if (!_enable)
		return;

	cancelFade(soundChannel);

	int startVol = fadeIn ? 0 :  _channels[soundChannel - 1].volume;
	int targetVol = fadeIn ? _channels[soundChannel - 1].volume : 0;

	_channels[soundChannel - 1].fade = new FadeParams(startVol, targetVol, ticks, _vm->getMacTicks(), fadeIn);
	_mixer->setChannelVolume(_channels[soundChannel - 1].handle, startVol);
}

bool DirectorSound::fadeChannel(uint8 soundChannel) {
	if (!isChannelValid(soundChannel) || !isChannelActive(soundChannel))
		return false;

	FadeParams *fade = _channels[soundChannel - 1].fade;
	if (!fade)
		return false;

	fade->lapsedTicks = _vm->getMacTicks() - fade->startTicks;
	if (fade->lapsedTicks > fade->totalTicks) {
		cancelFade(soundChannel);
		return false;
	}

	int fadeVol;
	if (fade->fadeIn) {
		fadeVol = MIN(fade->lapsedTicks * ((float)fade->targetVol / fade->totalTicks), (float)Audio::Mixer::kMaxChannelVolume);
	} else {
		fadeVol = MAX((fade->totalTicks - fade->lapsedTicks) * ((float)fade->startVol / fade->totalTicks), (float)0);
	}

	_mixer->setChannelVolume(_channels[soundChannel - 1].handle, fadeVol);
	return true;
}

void DirectorSound::cancelFade(uint8 soundChannel) {
	// NOTE: It is assumed that soundChannel has already been validated, which is
	// why this method is private.

	if (_channels[soundChannel - 1].fade) {
		_mixer->setChannelVolume(_channels[soundChannel - 1].handle, _channels[soundChannel - 1].fade->targetVol);

		delete _channels[soundChannel - 1].fade;
		_channels[soundChannel - 1].fade = nullptr;
	}
}

bool DirectorSound::isChannelActive(uint8 soundChannel) {
	if (!isChannelValid(soundChannel))
		return false;
	return _mixer->isSoundHandleActive(_channels[soundChannel - 1].handle);
}

bool DirectorSound::isChannelValid(uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
		return false;
	}
	return true;
}

void DirectorSound::playExternalSound(AudioDecoder *ad, uint8 soundChannel, uint8 externalSoundID) {
	if (!isChannelValid(soundChannel))
		return;

	// use castMemberID info to check, castLib -1 represent for externalSound
	// this should be amended by some kind of union which contains CastMemberID and externalSound info
	if (isChannelActive(soundChannel) && checkLastPlayCast(soundChannel, CastMemberID(externalSoundID, -1)))
		return;

	playStream(*(ad->getAudioStream()), soundChannel);
	setLastPlayCast(soundChannel, CastMemberID(externalSoundID, -1));
}

void DirectorSound::changingMovie() {
	for (uint i = 0; i < _channels.size(); i++)
		_channels[i]._movieChanged = true;
}

void DirectorSound::setLastPlayCast(uint8 soundChannel, CastMemberID castMemberId) {
	_channels[soundChannel - 1].lastPlayingCast = castMemberId;
	_channels[soundChannel - 1]._movieChanged = false;
}

bool DirectorSound::checkLastPlayCast(uint8 soundChannel, const CastMemberID &castMemberId) {
	return !_channels[soundChannel - 1]._movieChanged && _channels[soundChannel - 1].lastPlayingCast == castMemberId;
}

void DirectorSound::stopSound(uint8 soundChannel) {
	if (!isChannelValid(soundChannel))
		return;

	cancelFade(soundChannel);
	_mixer->stopHandle(_channels[soundChannel - 1].handle);
	setLastPlayCast(soundChannel, CastMemberID(0, 0));
	return;
}

void DirectorSound::stopSound() {
	for (uint i = 0; i < _channels.size(); i++) {
		cancelFade(i + 1);

		_mixer->stopHandle(_channels[i].handle);
		setLastPlayCast(i + 1, CastMemberID(0, 0));
	}

	_mixer->stopHandle(_scriptSound);
	_mixer->stopHandle(_pcSpeakerHandle);
}

void DirectorSound::systemBeep() {
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 500, 150);
}

void DirectorSound::playFPlaySound() {
	if (_fplayQueue.empty())
		return;
	// only when the previous sound is finished, shall we play next one
	if (isChannelActive(1))
		return;

	Common::String sndName = _fplayQueue.pop();
	if (sndName.equalsIgnoreCase("stop")) {
		stopSound(1);
		_currentSoundName = "";

		if (_fplayQueue.empty())
			return;
		else
			sndName = _fplayQueue.pop();
	}

	uint32 tag = MKTAG('s', 'n', 'd', ' ');
	uint id = 0xFFFF;
	Archive *archive = nullptr;

	// iterate opened ResFiles
	for (Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = g_director->_openResFiles.begin(); it != g_director->_openResFiles.end(); ++it) {
		id = it->_value->findResourceID(tag, sndName, true);
		if (id != 0xFFFF) {
			archive = it->_value;
			break;
		}
	}

	if (id == 0xFFFF) {
		warning("DirectorSound:playFPlaySound: can not find sound %s", sndName.c_str());
		return;
	}

	Common::SeekableReadStreamEndian *sndData = archive->getResource(tag, id);
	if (sndData != nullptr) {
		SNDDecoder *ad = new SNDDecoder();
		ad->loadStream(*sndData);
		delete sndData;

		Audio::AudioStream *as;
		bool looping = false;

		if (!_fplayQueue.empty() && _fplayQueue.front().equalsIgnoreCase("continuous")) {
			_fplayQueue.pop();
			looping = true;
		}
		as = ad->getAudioStream(looping);

		if (!as) {
			warning("DirectorSound:playFPlaySound: failed to get audio stream");
			return;
		}

		// update current playing sound
		_currentSoundName = sndName;

		playStream(*as, 1);
		delete ad;
	}
}

void DirectorSound::playFPlaySound(const Common::Array<Common::String> &fplayList) {
	for (uint i = 0; i < fplayList.size(); i++)
		_fplayQueue.push(fplayList[i]);

	// stop the previous sound, because new one is comming
	if (isChannelActive(1))
		stopSound(1);

	playFPlaySound();
}

void DirectorSound::setSoundLevelInternal(uint8 soundChannel, uint8 soundLevel) {
	// we have 8 level of sounds, and in ScummVM, we have range 0 to 255, thus 1 level represent 32
	_channels[soundChannel - 1].volume = soundLevel * 32;
	if (_enable && isChannelActive(soundChannel))
		_mixer->setChannelVolume(_channels[soundChannel - 1].handle, _channels[soundChannel - 1].volume);
}

// -1 represent all the sound channel
void DirectorSound::setSouldLevel(int channel, uint8 soundLevel) {
	if (soundLevel >= 8) {
		warning("DirectorSound::setSoundLevel: soundLevel %d out of bounds", soundLevel);
		return;
	}

	if (channel != -1) {
		if (!isChannelValid(channel))
			return;
		setSoundLevelInternal(channel, soundLevel);
	} else {
		for (uint i = 0; i < _channels.size(); i++)
			setSoundLevelInternal(i + 1, soundLevel);
	}
}

uint8 DirectorSound::getSoundLevel(uint8 soundChannel) {
	if (!isChannelValid(soundChannel))
		return 0;
	return _channels[soundChannel - 1].volume / 32;
}

SNDDecoder::SNDDecoder()
		: AudioDecoder() {
	_data = nullptr;
	_channels = 0;
	_size = 0;
	_rate = 0;
	_flags = 0;
	_loopStart = _loopEnd = 0;
}

SNDDecoder::~SNDDecoder() {
	if (_data) {
		free(_data);
	}
}

bool SNDDecoder::loadStream(Common::SeekableReadStreamEndian &stream) {
	if (_data) {
		free(_data);
		_data = nullptr;
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "snd header:");
		stream.hexdump(0x4e);
	}

	uint16 format = stream.readUint16();
	if (format == 1) {
		uint16 dataTypeCount = stream.readUint16();
		for (uint16 i = 0; i < dataTypeCount; i++) {
			uint16 dataType = stream.readUint16();
			if (dataType == 5) {
				// Sampled sound data
				uint32 options = stream.readUint32();
				_channels = (options & 0x80) ? 1 : 2;
				if (!processCommands(stream))
					return false;
			} else {
				warning("SNDDecoder: Unsupported data type: %d", dataType);
				return false;
			}
		}
	} else if (format == 2) {
		_channels = 1;
		/*uint16 refCount =*/stream.readUint16();
		if (!processCommands(stream))
			return false;
	} else {
		warning("SNDDecoder: Bad format: %d", format);
		return false;
	}

	return true;
}

bool SNDDecoder::processCommands(Common::SeekableReadStreamEndian &stream) {
	uint16 cmdCount = stream.readUint16();
	for (uint16 i = 0; i < cmdCount; i++) {
		uint16 cmd = stream.readUint16();
		if (cmd == 0x8051) {
			if (!processBufferCommand(stream))
				return false;
		} else {
			warning("SNDDecoder: Unsupported command: %d", cmd);
			return false;
		}
	}

	return true;
}

bool SNDDecoder::processBufferCommand(Common::SeekableReadStreamEndian &stream) {
	if (_data) {
		warning("SNDDecoder: Already read data");
		return false;
	}

	/*uint16 unk1 =*/stream.readUint16();
	int32 offset = stream.readUint32();
	if (offset != stream.pos()) {
		warning("SNDDecoder: Bad sound header offset. Expected: %d, read: %d", (int)stream.pos(), offset);
		return false;
	}
	/*uint32 dataPtr =*/stream.readUint32();
	uint32 param = stream.readUint32();
	_rate = stream.readUint16();
	/*uint16 rateExt =*/stream.readUint16();
	_loopStart = stream.readUint32();
	_loopEnd = stream.readUint32();
	byte encoding = stream.readByte();
	byte baseFrequency = stream.readByte();
	if (baseFrequency != 0x3c) {
		warning("SNDDecoder: Unsupported base frequency: %d", baseFrequency);
		return false;
	}
	uint32 frameCount = 0;
	uint16 bits = 8;
	if (encoding == 0x00) {
		// Standard sound header
		frameCount = param / _channels;
	} else if (encoding == 0xff) {
		// Extended sound header
		_channels = param;
		frameCount = stream.readUint32();
		for (uint32 i = 0; i < 0x0a; i++) {
			// aiff sample rate
			stream.readByte();
		}
		/*uint32 markerChunk =*/stream.readUint32();
		/*uint32 instrumentsChunk =*/stream.readUint32();
		/*uint32 aesRecording =*/stream.readUint32();
		bits = stream.readUint16();

		// future use
		stream.readUint16();
		stream.readUint32();
		stream.readUint32();
		stream.readUint32();
	} else if (encoding == 0xfe) {
		// Compressed sound header
		warning("SNDDecoder: Compressed sound header not supported");
		return false;
	} else {
		warning("SNDDecoder: Bad encoding: %d", encoding);
		return false;
	}

	_flags = 0;
	_flags |= (_channels == 2) ? Audio::FLAG_STEREO : 0;
	_flags |= (bits == 16) ? Audio::FLAG_16BITS : 0;
	_flags |= (bits == 8) ? Audio::FLAG_UNSIGNED : 0;
	_size = frameCount * _channels * (bits == 16 ? 2 : 1);

	_data = (byte *)malloc(_size);
	assert(_data);
	stream.read(_data, _size);

	return true;
}

Audio::AudioStream *SNDDecoder::getAudioStream(bool looping, DisposeAfterUse::Flag disposeAfterUse) {
	if (!_data)
		return nullptr;
	byte *buffer = (byte *)malloc(_size);
	memcpy(buffer, _data, _size);

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(buffer, _size, _rate, _flags, disposeAfterUse);

	if (looping) {
		if (hasLoopBounds()) {
			return new Audio::SubLoopingAudioStream(stream, 0, Audio::Timestamp(0, _loopStart, _rate), Audio::Timestamp(0, _loopEnd, _rate));
		} else {
			// Not sure if looping sounds can appear without loop bounds.
			// Let's just log a warning and loop the entire sound...
			warning("SNDDecoder::getAudioStream: Looping sound has no loop bounds");
			return new Audio::LoopingAudioStream(stream, 0);
		}
	}

	return stream;
}

bool SNDDecoder::hasLoopBounds() {
	return _loopStart != 0 || _loopEnd != 0;
}

AudioFileDecoder::AudioFileDecoder(Common::String &path)
		: AudioDecoder() {
	_path = path;
}

Audio::AudioStream *AudioFileDecoder::getAudioStream(bool looping, DisposeAfterUse::Flag disposeAfterUse) {
	if (_path.empty())
		return nullptr;

	Common::File *file = new Common::File();
	if (!file->open(_path)) {
		warning("Failed to open %s", _path.c_str());
		return nullptr;
	}
	uint32 magic1 = file->readUint32BE();
	file->readUint32BE();
	uint32 magic2 = file->readUint32BE();
	file->seek(0);

	Audio::RewindableAudioStream *stream = nullptr;
	if (magic1 == MKTAG('R', 'I', 'F', 'F') &&
		magic2 == MKTAG('W', 'A', 'V', 'E')) {
		stream = Audio::makeWAVStream(file, disposeAfterUse);
	} else if (magic1 == MKTAG('F', 'O', 'R', 'M') &&
				magic2 == MKTAG('A', 'I', 'F', 'F')) {
		stream = Audio::makeAIFFStream(file, disposeAfterUse);
	} else {
		warning("Unknown file type for %s", _path.c_str());
	}

	if (stream) {
		if (looping)
			return new Audio::LoopingAudioStream(stream, 0);
		else
			return stream;
	}

	return nullptr;
}

} // End of namespace Director
