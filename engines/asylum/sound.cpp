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
 * $URL$
 * $Id$
 *
 */

#include "asylum/sound.h"
#include "asylum/asylum.h"
#include "asylum/config.h"

#include "common/stream.h"
#include "sound/audiostream.h"
#include "sound/adpcm.h"
#include "sound/wave.h"

namespace Asylum {

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer) {
	_speechPack = new ResourcePack(3);
	_soundPack  = new ResourcePack(18);
	_currentMusicResIndex = -666;
}

Sound::~Sound() {
	delete _speechPack;
	delete _soundPack;

	clearSoundBuffer();
}

// from engines/agos/sound.cpp
void convertVolume(int &vol) {
	// DirectSound was orginally used, which specifies volume
	// and panning differently than ScummVM does, using a logarithmic scale
	// rather than a linear one.
	//
	// Volume is a value between -10,000 and 0.
	//
	// In both cases, the -10,000 represents -100 dB. When panning, only
	// one speaker's volume is affected - just like in ScummVM - with
	// negative values affecting the left speaker, and positive values
	// affecting the right speaker. Thus -10,000 means the left speaker is
	// silent.

	int v = CLIP(vol, -10000, 0);
	if (v) {
		vol = (int)((double)Audio::Mixer::kMaxChannelVolume * pow(10.0, (double)v / 2000.0) + 0.5);
	} else {
		vol = Audio::Mixer::kMaxChannelVolume;
	}
}

// from engines/agos/sound.cpp
void convertPan(int &pan) {
	// DirectSound was orginally used, which specifies volume
	// and panning differently than ScummVM does, using a logarithmic scale
	// rather than a linear one.
	//
	// Panning is a value between -10,000 and 10,000.
	//
	// In both cases, the -10,000 represents -100 dB. When panning, only
	// one speaker's volume is affected - just like in ScummVM - with
	// negative values affecting the left speaker, and positive values
	// affecting the right speaker. Thus -10,000 means the left speaker is
	// silent.

	int p = CLIP(pan, -10000, 10000);
	if (p < 0) {
		pan = (int)(255.0 * pow(10.0, (double)p / 2000.0) + 127.5);
	} else if (p > 0) {
		pan = (int)(255.0 * pow(10.0, (double)p / -2000.0) - 127.5);
	} else {
		pan = 0;
	}
}

int Sound::getBufferPosition(uint32 resId) {
	int pos = -1;

	for (uint i = 0; i < _soundBuffer.size(); i++) {
		if (resId == _soundBuffer[i].resId) {
			pos = i;
			break;
		}
	}

	return pos;
}

bool Sound::addToSoundBuffer(uint resId) {
	int exists = getBufferPosition(resId);

	if (exists < 0) {
		SoundBufferItem sound;
		sound.resId = resId;
        sound.handle = _soundHandle;
		_soundBuffer.push_back(sound);
	}

	return (exists < 0) ? true : false;
}

void Sound::removeFromSoundBuffer(uint resId) {
    int pos = getBufferPosition(resId);

	if (pos >= 0) {
		_soundBuffer.remove_at(pos);
	}
}

void Sound::clearSoundBuffer() {
	_soundBuffer.clear();
}

bool Sound::isPlaying(uint resId) {
	int pos = getBufferPosition(resId);

	if (pos < 0) {
		warning("isPlaying: resId %d not currently bufferred", resId);
	} else {
		SoundBufferItem snd = _soundBuffer[pos];
		if (_mixer->isSoundHandleActive(snd.handle)) {
			return true;
        } else {
            removeFromSoundBuffer(resId);
        }
	}

	return false;
}

void Sound::playSound(ResourcePack *pack, uint resId, int volume, bool looping, int panning, bool overwrite) {
	ResourceEntry *resource = pack->getResource(resId);
	if (_mixer->isSoundHandleActive(_soundHandle)) {
		if (overwrite) {
			_mixer->stopHandle(_soundHandle);
			playSound(resource, looping, volume, panning);
		}
	} else {
		// if the current handle isn't active, play the sound
		playSound(resource, looping, volume, panning);
	}
}

void Sound::playSound(ResourceEntry *resource, bool looping, int volume, int panning) {
	playSoundData(&_soundHandle, resource->data, resource->size, looping, volume, panning);
}

void Sound::playSound(ResourcePack *pack, uint resId, bool looping, int volume, int panning) {
	int pos = getBufferPosition(resId);

	if (pos < 0) {
		warning("playSound: resId %d not currently bufferred", resId);
	} else {
		SoundBufferItem snd = _soundBuffer[pos];
		if (_mixer->isSoundHandleActive(snd.handle)) {
			debugC(kDebugLevelSound, "playSound: handle for resId %d already active", resId);
		} else {
			ResourceEntry *ent = _soundPack->getResource(resId);
			playSoundData(&snd.handle, ent->data, ent->size, looping, volume, panning);
            addToSoundBuffer(resId);
		}
	}

}

void Sound::playSound(uint resId, bool looping, int volume, int panning, bool fromBuffer) {
	if (fromBuffer) {
		playSound(_soundPack, resId, looping, volume, panning);
	} else {
		if (_mixer->isSoundHandleActive(_soundHandle)) {
			debugC(kDebugLevelSound, "playSound: temporary sound handle is active");
		} else {
			ResourceEntry *ent = _soundPack->getResource(resId);
			playSound(ent, looping, volume, panning);
            addToSoundBuffer(resId);
		}
	}
}

void Sound::stopSound() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void Sound::stopSound(uint resId) {
	int pos = getBufferPosition(resId);

	if (pos < 0) {
		warning("stopSound: resId %d not currently bufferred", resId);
	} else {
		_mixer->stopHandle(_soundBuffer[pos].handle);
	}
}

void Sound::stopAllSounds(bool stopSpeechAndMusic) {
	_mixer->stopHandle(_soundHandle);

	if (stopSpeechAndMusic) {
		_mixer->stopHandle(_speechHandle);
		_mixer->stopHandle(_musicHandle);
	}

	for (uint i = 0; i < _soundBuffer.size(); i++)
		_mixer->stopHandle(_soundBuffer[i].handle);
}

void Sound::playSpeech(uint resId) {
	ResourceEntry *ent = _speechPack->getResource(resId);

	_mixer->stopHandle(_speechHandle);
	playSoundData(&_speechHandle, ent->data, ent->size, false, 0, 0);
}

void Sound::playMusic(uint resId) {
	stopMusic();

	// TODO Play music :P
}

void Sound::playMusic(ResourcePack *pack, uint resId) {
	stopMusic();

	ResourceEntry *resource = pack->getResource(resId);
	playSoundData(&_musicHandle, resource->data, resource->size, true, Config.musicVolume, 0);
}

void Sound::stopMusic() {
	_mixer->stopHandle(_musicHandle);
}

// from engines/agos/sound.cpp
void Sound::playSoundData(Audio::SoundHandle *handle, byte *soundData, uint soundDataLength, bool loop, int vol, int pan) {
	byte   *buffer, flags;
	uint16 compType;
	int    blockAlign, rate;

	// TODO: Use makeWAVStream() in future, when makeADPCMStream() allows sound looping
	int size = soundDataLength;
	Common::MemoryReadStream stream(soundData, size);
	if (!Audio::loadWAVFromStream(stream, size, rate, flags, &compType, &blockAlign))
		error("playSoundData: Not valid WAV data");

	convertVolume(vol);
	convertPan(pan);

	if (loop == true)
		flags |= Audio::Mixer::FLAG_LOOP;

	if (compType == 2) {
		Audio::AudioStream *sndStream = Audio::makeADPCMStream(&stream, false, size, Audio::kADPCMMS, rate, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign);
		buffer = (byte *)malloc(size * 4);
		size   = sndStream->readBuffer((int16*)buffer, size * 2);
		size  *= 2; // 16bits.
		delete sndStream;
	} else {
		buffer = (byte *)malloc(size);
		memcpy(buffer, soundData + stream.pos(), size);
	}

	// TODO
	// All asylum audio data is being filtered through the playSoundData()
	// method. As such, they're all using kSFXSoundType. There are also
	// enums for kMusicSoundType and kSpeechSoundType.
	//
	// Investigate how this can effect ... anything :P
	_mixer->playRaw(Audio::Mixer::kSFXSoundType, handle, buffer, size, rate, flags | Audio::Mixer::FLAG_AUTOFREE, -1, vol, pan);
}

} // end of namespace Asylum
