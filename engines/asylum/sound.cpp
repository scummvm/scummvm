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
#include "sound/decoders/adpcm.h"
#include "sound/decoders/wave.h"

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
void convertVolume(int32 &vol) {
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

	int32 v = CLIP(vol, -10000, 0);
	if (v) {
		vol = (int)((double)Audio::Mixer::kMaxChannelVolume * pow(10.0, (double)v / 2000.0) + 0.5);
	} else {
		vol = Audio::Mixer::kMaxChannelVolume;
	}
}

// from engines/agos/sound.cpp
void convertPan(int32 &pan) {
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

	int32 p = CLIP(pan, -10000, 10000);
	if (p < 0) {
		pan = (int)(255.0 * pow(10.0, (double)p / 2000.0) + 127.5);
	} else if (p > 0) {
		pan = (int)(255.0 * pow(10.0, (double)p / -2000.0) - 127.5);
	} else {
		pan = 0;
	}
}

int32 Sound::getBufferPosition(int32 resId) {
	int32 pos = -1;

	for (uint32 i = 0; i < _soundBuffer.size(); i++) {
		if (resId == _soundBuffer[i].resId) {
			pos = i;
			break;
		}
	}

	return pos;
}

bool Sound::addToSoundBuffer(int32 resId) {
	int32 exists = getBufferPosition(resId);

	if (exists < 0) {
		SoundBufferItem sound;
		sound.resId = resId;
        sound.handle = _soundHandle;
		_soundBuffer.push_back(sound);
	}

	return (exists < 0) ? true : false;
}

void Sound::removeFromSoundBuffer(int32 resId) {
    int32 pos = getBufferPosition(resId);

	if (pos >= 0) {
		_soundBuffer.remove_at(pos);
	}
}

void Sound::clearSoundBuffer() {
	_soundBuffer.clear();
}

bool Sound::isPlaying(int32 resId) {
	int32 pos = getBufferPosition(resId);

	if (pos < 0) {
		//warning("isPlaying: resId %d not currently bufferred", resId);
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

void Sound::playSound(ResourcePack *pack, int32 resId, int32 volume, bool looping, int32 panning, bool overwrite) {
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

void Sound::playSound(ResourceEntry *resource, bool looping, int32 volume, int32 panning) {
	playSoundData(Audio::Mixer::kSFXSoundType, &_soundHandle, resource->data, resource->size, looping, volume, panning);
}

void Sound::playSound(ResourcePack *pack, int32 resId, bool looping, int32 volume, int32 panning) {
	int32 pos = getBufferPosition(resId);

	if (pos < 0) {
		warning("playSound: resId %d not currently bufferred", resId);
	} else {
		SoundBufferItem snd = _soundBuffer[pos];
		if (_mixer->isSoundHandleActive(snd.handle)) {
			debugC(kDebugLevelSound, "playSound: handle for resId %d already active", resId);
		} else {
			ResourceEntry *ent = _soundPack->getResource(resId);
			playSoundData(Audio::Mixer::kSFXSoundType, &snd.handle, ent->data, ent->size, looping, volume, panning);
            addToSoundBuffer(resId);
		}
	}

}

void Sound::playSound(int32 resId, bool looping, int32 volume, int32 panning, bool fromBuffer) {
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

void Sound::stopSound(int32 resId) {
	int32 pos = getBufferPosition(resId);

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

	for (uint32 i = 0; i < _soundBuffer.size(); i++)
		_mixer->stopHandle(_soundBuffer[i].handle);
}

void Sound::playSpeech(int32 resId) {
	ResourceEntry *ent = _speechPack->getResource(resId);

	_mixer->stopHandle(_speechHandle);
	playSoundData(Audio::Mixer::kSpeechSoundType, &_speechHandle, ent->data, ent->size, false, 0, 0);
}

void Sound::playMusic(int32 resId) {
	stopMusic();

	// TODO Play music :P
}

void Sound::playMusic(ResourcePack *pack, int32 resId) {
	stopMusic();

	ResourceEntry *resource = pack->getResource(resId);
	playSoundData(Audio::Mixer::kMusicSoundType, &_musicHandle, resource->data, resource->size, true, Config.musicVolume, 0);
}

void Sound::stopMusic() {
	_mixer->stopHandle(_musicHandle);
}

// from engines/agos/sound.cpp
void Sound::playSoundData(Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte *soundData, int32 soundDataLength, bool loop, int32 vol, int32 pan) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(soundData, soundDataLength);
	Audio::RewindableAudioStream *sndStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	// FIXME need to convert the volume properly
	vol = Audio::Mixer::kMaxChannelVolume;

	_mixer->playStream(type, handle, Audio::makeLoopingAudioStream(sndStream, loop ? 0 : 1), -1, vol, pan);
}

} // end of namespace Asylum
