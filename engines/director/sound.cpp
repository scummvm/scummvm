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

// SNDDecoder based on snd2wav by Abraham Macias Paredes
// https://github.com/System25/drxtract/blob/master/snd2wav
// License: GNU GPL v2 (see COPYING file for details)

#include "audio/audiostream.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/substream.h"

#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "audio/softsynth/pcspk.h"
#include "audio/decoders/aiff.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/castmember/sound.h"

namespace Director {

DirectorSound::DirectorSound(Window *window) : _window(window) {
	uint numChannels = 2;
	if (g_director->getVersion() >= 300) {
		numChannels = 4;
	}

	for (uint i = 1; i <= numChannels; i++) {
		_channels[i] = new SoundChannel();
	}

	_mixer = g_system->getMixer();

	_speaker = new Audio::PCSpeaker();
	_mixer->playStream(Audio::Mixer::kSFXSoundType,
		&_pcSpeakerHandle, _speaker, -1, 50, 0, DisposeAfterUse::NO, true);

	_enable = true;
}

DirectorSound::~DirectorSound() {
	this->stopSound();
	unloadSampleSounds();
	delete _speaker;
	for (auto it : _channels)
		delete it._value;
}

SoundChannel *DirectorSound::getChannel(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return nullptr;
	return _channels[soundChannel];
}

void DirectorSound::playFile(Common::String filename, uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	if (debugChannelSet(-1, kDebugFast))
		return;

	AudioFileDecoder af(filename);
	Audio::AudioStream *sound = af.getAudioStream(false, false, DisposeAfterUse::YES);

	cancelFade(soundChannel);
	stopSound(soundChannel);

	setChannelDefaultVolume(soundChannel);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel]->handle, sound, -1, getChannelVolume(soundChannel));

	// Set the last played sound so that cast member 0 in the sound channel doesn't stop this file.
	setLastPlayedSound(soundChannel, SoundID(), false);
}

void DirectorSound::playMCI(Audio::AudioStream &stream, uint32 from, uint32 to) {
	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(&stream);
	Audio::SubSeekableAudioStream *subSeekStream = new Audio::SubSeekableAudioStream(seekStream, Audio::Timestamp(from, seekStream->getRate()), Audio::Timestamp(to, seekStream->getRate()));

	// TODO: make sound enable settings work on this one
	_mixer->stopHandle(_scriptSound);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_scriptSound, subSeekStream);
}

uint8 DirectorSound::getChannelVolume(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return 0;

	return _enable ? _channels[soundChannel]->volume : 0;
}

void DirectorSound::setChannelDefaultVolume(int soundChannel) {
	int vol = _volumes.getValOrDefault(soundChannel, g_director->_defaultVolume);

	_channels[soundChannel]->volume = vol;
}

void DirectorSound::playStream(Audio::AudioStream &stream, uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	cancelFade(soundChannel);

	_mixer->stopHandle(_channels[soundChannel]->handle);

	setChannelDefaultVolume(soundChannel);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel]->handle, &stream, -1, getChannelVolume(soundChannel));
}

void DirectorSound::playSound(SoundID soundID, uint8 soundChannel, bool forPuppet) {
	switch (soundID.type) {
	case kSoundCast:
		playCastMember(CastMemberID(soundID.u.cast.member, soundID.u.cast.castLib), soundChannel, forPuppet);
		break;
	case kSoundExternal:
		playExternalSound(soundID.u.external.menu, soundID.u.external.submenu, soundChannel);
		break;
	}
}

void DirectorSound::playCastMember(CastMemberID memberID, uint8 soundChannel, bool forPuppet) {
	if (!assertChannel(soundChannel))
		return;

	if (memberID.member == 0) {
		// Normally cast member 0 stops the sound.
		// But there are some sounds where it doesn't. Those are:
		//   1. playFile
		//   2. FPlay
		//   3. non-puppet looping sounds
		//   4. maybe more?
		if (shouldStopOnZero(soundChannel)) {
			stopSound(soundChannel);
			// Director 4 will stop after the current loop iteration, but
			// Director 3 will continue looping until the sound is replaced.
		} else if (g_director->getVersion() >= 400) {
			// If there is a loopable stream specified, set the loop to expire by itself
			if (_channels[soundChannel]->loopPtr) {
				debugC(5, kDebugSound, "DirectorSound::playCastMember(): telling loop in channel %d to stop", soundChannel);
				_channels[soundChannel]->loopPtr->setRemainingIterations(1);
				_channels[soundChannel]->loopPtr = nullptr;
			}

			// Don't stop the currently playing sound, just set the last played sound to 0.
			setLastPlayedSound(soundChannel, SoundID(), false);
		}
	} else {
		CastMember *soundCast = _window->getCurrentMovie()->getCastMember(memberID);
		if (soundCast) {
			if (soundCast->_type != kCastSound) {
				warning("DirectorSound::playCastMember: attempted to play a non-SoundCastMember %s", memberID.asString().c_str());
			} else {
				bool looping = ((SoundCastMember *)soundCast)->_looping;
				bool stopOnZero = true;

				// For a non-puppet sound, if the sound is the same ID as the last
				// played, do nothing.
				if (!forPuppet && isLastPlayedSound(soundChannel, memberID))
					return;

				if (!forPuppet && looping) {
					// We know that this is a non-puppet, looping sound.
					// We don't want to stop it if this channel's cast member changes to 0.
					stopOnZero = false;
				}

				AudioDecoder *ad = ((SoundCastMember *)soundCast)->_audio;
				if (!ad) {
					warning("DirectorSound::playCastMember: no audio data attached to %s", memberID.asString().c_str());
					return;
				}

				Audio::AudioStream *as;
				as = ad->getAudioStream(looping, forPuppet);

				if (!as) {
					warning("DirectorSound::playCastMember: audio data failed to load from cast");
					return;
				}
				// For looping sounds, keep a copy of the AudioStream so it is
				// possible to gracefully stop the playback
				if (looping)
					_channels[soundChannel]->loopPtr = dynamic_cast<Audio::LoopableAudioStream *>(as);
				else
					_channels[soundChannel]->loopPtr = nullptr;
				playStream(*as, soundChannel);
				debugC(5, kDebugSound, "DirectorSound::playCastMember(): playing cast ID %s, channel %d, looping %d, stopOnZero %d, forPuppet %d, volume %d", memberID.asString().c_str(), soundChannel, looping, stopOnZero, forPuppet, _channels[soundChannel]->volume);
				setLastPlayedSound(soundChannel, memberID, stopOnZero);
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
	if (!assertChannel(soundChannel))
		return;

	debugC(5, kDebugSound, "DirectorSound::registerFade(): registered fading channel %d %s over %d ticks", soundChannel, fadeIn ? "in" : "out", ticks);

	// sound enable is not working on fade sounds, so we just return directly when sounds are not enabling
	if (!_enable)
		return;

	cancelFade(soundChannel);

	int startVol = fadeIn ? 0 :  _channels[soundChannel]->volume;
	int targetVol = fadeIn ? _channels[soundChannel]->volume : 0;

	_channels[soundChannel]->fade = new FadeParams(startVol, targetVol, ticks, _window->getVM()->getMacTicks(), fadeIn);
	_mixer->setChannelVolume(_channels[soundChannel]->handle, startVol);

	_channels[soundChannel]->volume = startVol;
}

bool DirectorSound::fadeChannels() {
	bool ongoing = false;

	for (auto &it : _channels) {
		FadeParams *fade = it._value->fade;
		if (!fade)
			continue;

		fade->lapsedTicks = _window->getVM()->getMacTicks() - fade->startTicks;
		if (fade->lapsedTicks > fade->totalTicks) {
			continue;
		}

		int fadeVol;
		if (fade->fadeIn) {
			fadeVol = MIN(fade->lapsedTicks * ((float)fade->targetVol / fade->totalTicks), (float)Audio::Mixer::kMaxChannelVolume);
		} else {
			fadeVol = MAX((fade->totalTicks - fade->lapsedTicks) * ((float)fade->startVol / fade->totalTicks), (float)0);
		}

		debugC(5, kDebugSound, "DirectorSound::fadeChannel(): fading channel %d volume to %d", it._key, fadeVol);
		_mixer->setChannelVolume(it._value->handle, fadeVol);

		it._value->volume = fadeVol;
		ongoing = true;
	}
	return ongoing;
}

void DirectorSound::cancelFade(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;
	// NOTE: It is assumed that soundChannel has already been validated, which is
	// why this method is private.

	if (_channels[soundChannel]->fade) {
		int restoreVol = _channels[soundChannel]->fade->fadeIn ? _channels[soundChannel]->fade->targetVol : _channels[soundChannel]->fade->startVol;
		debugC(5, kDebugSound, "DirectorSound::cancelFade(): resetting channel %d volume to %d", soundChannel, restoreVol);
		_mixer->setChannelVolume(_channels[soundChannel]->handle, restoreVol);

		delete _channels[soundChannel]->fade;
		_channels[soundChannel]->fade = nullptr;
	}
}

bool DirectorSound::isChannelActive(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return false;

	if (!_mixer->isSoundHandleActive(_channels[soundChannel]->handle))
		return false;

	// Looped sounds are considered to be inactive after the first play
	// WORKAROUND HACK
	if (_channels[soundChannel]->loopPtr != nullptr)
		return _channels[soundChannel]->loopPtr->getCompleteIterations() < 1;

	return true;
}

bool DirectorSound::assertChannel(int soundChannel) {
	if (soundChannel <= 0) {
		warning("DirectorSound::assertChannel(): Invalid sound channel %d", soundChannel);
		return false;
	}
	if (!_channels.contains(soundChannel)) {
		debugC(5, kDebugSound, "DirectorSound::assertChannel(): allocating sound channel %d", soundChannel);
		_channels[soundChannel] = new SoundChannel();
	}
	return true;
}

void DirectorSound::loadSampleSounds(uint type) {
	if (type < kMinSampledMenu || type > kMaxSampledMenu) {
		warning("DirectorSound::loadSampleSounds: Invalid menu number %d", type);
		return;
	}

	if (!_sampleSounds[type - kMinSampledMenu].empty())
		return;

	// trying to load external sample sounds
	// lazy loading
	uint32 tag = MKTAG('C', 'S', 'N', 'D');
	uint id = 0xFF;
	Archive *archive = nullptr;

	for (auto &it : g_director->_allOpenResFiles) {
		if (!g_director->_allSeenResFiles.contains(it)) {
			warning("DirectorSound::loadSampleSounds(): file %s not found in allSeenResFiles, skipping", it.toString().c_str());
			break;
		}
		Common::Array<uint16> idList = g_director->_allSeenResFiles[it]->getResourceIDList(tag);
		for (uint j = 0; j < idList.size(); j++) {
			if (static_cast<uint>(idList[j] & 0xFF) == type) {
				id = idList[j];
				archive = g_director->_allSeenResFiles[it];
				break;
			}
		}
		if (id != 0xFF)
			break;
	}

	if (!archive) {
		warning("DirectorSound::loadSampleSounds(): could not find a valid archive");
		return;
	}

	if (id == 0xFF) {
		warning("Score::loadSampleSounds: can not find CSND resource with id %d", type);
		return;
	}

	Common::SeekableReadStreamEndian *csndData = archive->getResource(tag, id);

	/*uint32 flag = */ csndData->readUint32();

	// the flag should be 0x604E
	// i'm not sure what's that mean, but it occurs in those csnd files

	// contains how many csnd data
	uint16 num = csndData->readUint16();

	// read the offset first;
	Common::Array<uint32> offset(num);
	for (uint i = 0; i < num; i++)
		offset[i] = csndData->readUint32();

	for (uint i = 0; i < num; i++) {
		csndData->seek(offset[i]);

		SNDDecoder *ad = new SNDDecoder();
		ad->loadExternalSoundStream(*csndData);
		_sampleSounds[type - kMinSampledMenu].push_back(ad);
	}

	delete csndData;
}

void DirectorSound::unloadSampleSounds() {
	for (uint i = 0; i < kNumSampledMenus; i++) {
		for (uint j = 0; j < _sampleSounds[i].size(); j++) {
			delete _sampleSounds[i][j];
		}
		_sampleSounds[i].clear();
	}
}

void DirectorSound::playExternalSound(uint16 menu, uint16 submenu, uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	SoundID soundId(kSoundExternal, menu, submenu);
	// If the sound is the same ID ast the last played, do nothing.
	if (isLastPlayedSound(soundChannel, soundId))
		return;

	if (menu < kMinSampledMenu || menu > kMaxSampledMenu) {
		warning("DirectorSound::playExternalSound: Invalid menu number %d", menu);
		return;
	}

	Common::Array<AudioDecoder *> &menuSounds = _sampleSounds[menu - kMinSampledMenu];
	if (menuSounds.empty())
		loadSampleSounds(menu);

	if (1 <= submenu && submenu <= menuSounds.size()) {
		debugC(5, kDebugSound, "DirectorSound::playExternalSound(): playing menu ID %d, submenu ID %d, channel %d, volume %d", menu, submenu, soundChannel, _channels[soundChannel]->volume);
		playStream(*(menuSounds[submenu - 1]->getAudioStream()), soundChannel);
		setLastPlayedSound(soundChannel, soundId);
	} else {
		warning("DirectorSound::playExternalSound: Could not find sound %d %d", menu, submenu);
	}
}

void DirectorSound::changingMovie() {
	for (auto it : _channels) {
		it._value->movieChanged = true;
		if (isChannelPuppet(it._key)) {
			setPuppetSound(SoundID(), it._key); // disable puppet sound
		}

		if (isChannelActive(it._key)) {
			// Don't stop this sound until there's a new, non-zero sound in this channel.
			it._value->stopOnZero = false;

			// If this is a looping sound, make it loop automatically until that happens.
			const SoundID &lastPlayedSound = it._value->lastPlayedSound;
			if (lastPlayedSound.type == kSoundCast) {
				CastMemberID memberID(lastPlayedSound.u.cast.member, lastPlayedSound.u.cast.castLib);
				CastMember *soundCast = _window->getCurrentMovie()->getCastMember(memberID);
				if (soundCast && soundCast->_type == kCastSound && static_cast<SoundCastMember *>(soundCast)->_looping) {
					_mixer->loopChannel(it._value->handle);
				}
			}
		}
	}
	unloadSampleSounds(); // TODO: we can possibly keep this between movies
}

void DirectorSound::setLastPlayedSound(uint8 soundChannel, SoundID soundId, bool stopOnZero) {
	_channels[soundChannel]->lastPlayedSound = soundId;
	_channels[soundChannel]->stopOnZero = stopOnZero;
	_channels[soundChannel]->movieChanged = false;
}

bool DirectorSound::isLastPlayedSound(uint8 soundChannel, const SoundID &soundId) {
	return !_channels[soundChannel]->movieChanged && _channels[soundChannel]->lastPlayedSound == soundId;
}

bool DirectorSound::shouldStopOnZero(uint8 soundChannel) {
	return _channels[soundChannel]->stopOnZero;
}

void DirectorSound::stopSound(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	debugC(5, kDebugSound, "DirectorSound::stopSound(): stopping channel %d", soundChannel);
	if (_channels[soundChannel]->loopPtr)
		_channels[soundChannel]->loopPtr = nullptr;
	cancelFade(soundChannel);
	_mixer->stopHandle(_channels[soundChannel]->handle);
	setLastPlayedSound(soundChannel, SoundID());
	return;
}

void DirectorSound::stopSound() {
	debugC(5, kDebugSound, "DirectorSound::stopSound(): stopping all channels");
	for (auto it : _channels) {
		if (it._value->loopPtr)
			it._value->loopPtr = nullptr;
		cancelFade(it._key);

		_mixer->stopHandle(it._value->handle);
		setLastPlayedSound(it._key, SoundID());
	}

	_mixer->stopHandle(_scriptSound);
	_mixer->stopHandle(_pcSpeakerHandle);
}

void DirectorSound::systemBeep() {
	debugC(5, kDebugSound, "DirectorSound::systemBeep(): beep!");
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 500, 150);
}

bool DirectorSound::isChannelPuppet(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return false;

	// cast member ID 0 means "not a puppet"
	if (_channels[soundChannel]->puppet.type == kSoundCast && _channels[soundChannel]->puppet.u.cast.member == 0)
		return false;

	return true;
}

void DirectorSound::setPuppetSound(SoundID soundId, uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	_channels[soundChannel]->newPuppet = true;
	_channels[soundChannel]->puppet = soundId;
	_channels[soundChannel]->stopOnZero = true;
}

void DirectorSound::playPuppetSound(uint8 soundChannel) {
	if (!assertChannel(soundChannel))
		return;

	// only play if the puppet was just set
	if (!_channels[soundChannel]->newPuppet)
		return;

	debugC(5, kDebugSound, "DirectorSound::playPuppetSound(): playing on channel %d", soundChannel);

	_channels[soundChannel]->newPuppet = false;
	playSound(_channels[soundChannel]->puppet, soundChannel, true);
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
	for (auto &it : g_director->_allOpenResFiles) {
		id = g_director->_allSeenResFiles[it]->findResourceID(tag, sndName, true);
		if (id != 0xFFFF) {
			archive = g_director->_allSeenResFiles[it];
			break;
		}
	}

	if (id == 0xFFFF) {
		warning("DirectorSound:playFPlaySound: can not find sound %s", sndName.c_str());
		return;
	}

	Common::SeekableReadStreamEndian *sndData = archive->getResource(tag, id);
	if (sndData != nullptr) {
		SNDDecoder ad;
		ad.loadStream(*sndData);
		delete sndData;

		Audio::AudioStream *as;
		bool looping = false;

		if (!_fplayQueue.empty() && _fplayQueue.front().equalsIgnoreCase("continuous")) {
			_fplayQueue.pop();
			looping = true;
		}

		// FPlay is controlled by Lingo, not the score, like a puppet,
		// so we'll get the puppet version of the stream.
		as = ad.getAudioStream(looping, true);

		if (!as) {
			warning("DirectorSound:playFPlaySound: failed to get audio stream");
			return;
		}

		// update current playing sound
		_currentSoundName = sndName;

		playStream(*as, 1);
	}

	// Set the last played sound so that cast member 0 in the sound channel doesn't stop this file.
	setLastPlayedSound(1, SoundID(), false);
}

void DirectorSound::playFPlaySound(const Common::Array<Common::String> &fplayList) {
	for (uint i = 0; i < fplayList.size(); i++)
		_fplayQueue.push(fplayList[i]);

	// stop the previous sound, because new one is coming
	if (isChannelActive(1))
		stopSound(1);

	playFPlaySound();
}

void DirectorSound::setChannelVolumeInternal(uint8 soundChannel, uint8 volume) {
	if (volume == _channels[soundChannel]->volume)
		return;

	cancelFade(soundChannel);

	_channels[soundChannel]->volume = volume;
	_volumes[soundChannel] = volume;

	if (_enable)
		_mixer->setChannelVolume(_channels[soundChannel]->handle, _channels[soundChannel]->volume);
}

// -1 represent all the sound channel
void DirectorSound::setChannelVolume(int channel, uint8 volume) {
	if (channel != -1) {
		if (!assertChannel(channel))
			return;
		debugC(5, kDebugSound, "DirectorSound::setChannelVolume: setting channel %d to volume %d", channel, volume);
		setChannelVolumeInternal(channel, volume);
	} else {
		debugC(5, kDebugSound, "DirectorSound::setChannelVolume: setting all channels to volume %d", volume);
		for (uint i = 0; i < _channels.size(); i++)
			setChannelVolumeInternal(i + 1, volume);
	}
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
		if (cmd == 0x8050 || cmd == 0x8051) {
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

Audio::AudioStream *SNDDecoder::getAudioStream(bool looping, bool forPuppet, DisposeAfterUse::Flag disposeAfterUse) {
	if (!_data)
		return nullptr;
	byte *buffer = (byte *)malloc(_size);
	memcpy(buffer, _data, _size);

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(buffer, _size, _rate, _flags, disposeAfterUse);

	if (looping) {
		if (hasLoopBounds()) {
			// FIXME: determine the correct behaviour for non-consecutive loop bounds
			if (_loopEnd <= _loopStart) {
				warning("SNDDecoder::getAudioStream: Looping sound has non-consecutive bounds, using entire sample");
				return new Audio::LoopingAudioStream(stream, 0);
			} else {
				// Return an automatically looping stream.
				debugC(5, kDebugSound, "DirectorSound::getAudioStream(): returning a loop at positions start: %i, end: %i", _loopStart, _loopEnd);
				return new Audio::SubLoopingAudioStream(stream, 0, Audio::Timestamp(0, _loopStart, _rate), Audio::Timestamp(0, _loopEnd, _rate));
			}
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

bool SNDDecoder::hasValidLoopBounds() {
	return hasLoopBounds() && _loopStart < _loopEnd && _loopEnd <= _size;
}

void SNDDecoder::resetLoopBounds() {
	_loopStart = _loopEnd = 0;
}

AudioFileDecoder::AudioFileDecoder(Common::String &path)
		: AudioDecoder() {
	_path = path;
}

AudioFileDecoder::~AudioFileDecoder() {
}

Audio::AudioStream *AudioFileDecoder::getAudioStream(bool looping, bool forPuppet, DisposeAfterUse::Flag disposeAfterUse) {
	if (_path.empty())
		return nullptr;

	Common::Path newPath = findAudioPath(_path);
	Common::SeekableReadStream *copiedStream = Common::MacResManager::openFileOrDataFork(newPath);
	if (!copiedStream) {
		warning("Failed to open %s", _path.c_str());
		return nullptr;
	}

	uint32 magic1 = copiedStream->readUint32BE();
	copiedStream->readUint32BE();
	uint32 magic2 = copiedStream->readUint32BE();
	copiedStream->seek(0);

	Audio::RewindableAudioStream *stream = nullptr;
	if (magic1 == MKTAG('R', 'I', 'F', 'F') &&
		magic2 == MKTAG('W', 'A', 'V', 'E')) {
		stream = Audio::makeWAVStream(copiedStream, disposeAfterUse);
	} else if (magic1 == MKTAG('F', 'O', 'R', 'M') &&
				(magic2 == MKTAG('A', 'I', 'F', 'F') || magic2 == MKTAG('A', 'I', 'F', 'C'))) {
		stream = Audio::makeAIFFStream(copiedStream, disposeAfterUse);
	} else {
		warning("Unknown file type for %s", _path.c_str());
		delete copiedStream;
	}

	if (stream) {
		if (looping) {
			return new Audio::LoopingAudioStream(stream, 0);
		}
		return stream;
	}

	return nullptr;
}

} // End of namespace Director
