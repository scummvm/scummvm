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


#include "asylum/system/config.h"
#include "asylum/system/sound.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/decoders/adpcm.h"
#include "sound/decoders/wave.h"

namespace Asylum {

Sound::Sound(AsylumEngine *engine, Audio::Mixer *mixer) : _vm(engine), _mixer(mixer) {
	_currentMusicResIndex = kResourceMusicStopped;
}

Sound::~Sound() {
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
	// DirectSound was originally used, which specifies volume
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

void Sound::setVolume(ResourceId resourceId, double volume) {
	error("[Sound::setVolume] not implemented");
}

int32 Sound::getAdjustedVolume(int32 volume) {
	error("[Sound::getAdjustedVolume] not implemented");
}

int32 Sound::calculateVolumeAdjustement(int32 x, int32 y, int32 a5, int32 a6) {
	error("[Sound::calculateVolume] not implemented");
}

void Sound::setPanning(ResourceId resourceId, int32 panning) {
	error("[Sound::setPanning] not implemented");
}

int32 Sound::calculatePanningAtPoint(int32 x, int32 y) {
	error("[Sound::calculatePanningAtPoint] not implemented");
}

int32 Sound::getBufferPosition(ResourceId resourceId) {
	int32 pos = -1;

	for (uint32 i = 0; i < _soundBuffer.size(); i++) {
		if (resourceId == _soundBuffer[i].resourceId) {
			pos = i;
			break;
		}
	}

	return pos;
}

bool Sound::addToSoundBuffer(ResourceId resourceId) {
	int32 exists = getBufferPosition(resourceId);

	if (exists < 0) {
		SoundBufferItem sound;
		sound.resourceId = resourceId;
		sound.handle = _soundHandle;
		_soundBuffer.push_back(sound);
	}

	return (exists < 0) ? true : false;
}

void Sound::removeFromSoundBuffer(ResourceId resourceId) {
	int32 pos = getBufferPosition(resourceId);

	if (pos >= 0) {
		_soundBuffer.remove_at(pos);
	}
}

void Sound::clearSoundBuffer() {
	_soundBuffer.clear();
}

bool Sound::isPlaying(ResourceId resourceId) {
	int32 pos = getBufferPosition(resourceId);

	if (pos < 0) {
		//warning("isPlaying: resource %d not currently buffered", resourceId);
	} else {
		SoundBufferItem snd = _soundBuffer[pos];
		if (_mixer->isSoundHandleActive(snd.handle)) {
			return true;
		} else {
			removeFromSoundBuffer(resourceId);
		}
	}

	return false;
}

//void Sound::playSound(ResourceId resourceId, int32 volume, bool looping, int32 panning, bool overwrite) {
//	ResourceEntry *resource = getResource()->get(resourceId);
//	if (_mixer->isSoundHandleActive(_soundHandle)) {
//		if (overwrite) {
//			_mixer->stopHandle(_soundHandle);
//			playSound(resource, looping, volume, panning);
//		}
//	} else {
//		// if the current handle isn't active, play the sound
//		playSound(resource, looping, volume, panning);
//	}
//}
//
//void Sound::playSound(ResourceEntry *resource, bool looping, int32 volume, int32 panning) {
//	playSoundData(Audio::Mixer::kSFXSoundType, &_soundHandle, resource->data, resource->size, looping, volume, panning);
//}

void Sound::playSound(ResourceId resourceId, bool looping, int32 volume, int32 panning) {
	int32 pos = getBufferPosition(resourceId);

	if (pos < 0) {
		warning("playSound: resource %d not currently bufferred", resourceId);
	} else {
		SoundBufferItem snd = _soundBuffer[pos];
		if (_mixer->isSoundHandleActive(snd.handle)) {
			debugC(kDebugLevelSound, "playSound: handle for resource %d already active", resourceId);
		} else {
			ResourceEntry *ent = getResource()->get(resourceId);
			playSoundData(Audio::Mixer::kSFXSoundType, &snd.handle, ent->data, ent->size, looping, volume, panning);
			addToSoundBuffer(resourceId);
		}
	}

}

//void Sound::playSound(ResourceId resourceId, bool looping, int32 volume, int32 panning, bool fromBuffer) {
//	if (fromBuffer) {
//		playSound(resourceId, looping, volume, panning);
//	} else {
//		if (_mixer->isSoundHandleActive(_soundHandle)) {
//			debugC(kDebugLevelSound, "playSound: temporary sound handle is active");
//		} else {
//			ResourceEntry *ent = getResource()->get(resourceId);
//			playSound(resourceId, looping, volume, panning);
//			addToSoundBuffer(resourceId);
//		}
//	}
//}

void Sound::stopSound() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void Sound::stopSound(ResourceId resourceId) {
	int32 pos = getBufferPosition(resourceId);

	if (pos < 0) {
		warning("stopSound: resource %d not currently bufferred", resourceId);
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

void Sound::playSpeech(ResourceId resourceId) {
	ResourceEntry *ent = getResource()->get(resourceId);

	_mixer->stopHandle(_speechHandle);
	playSoundData(Audio::Mixer::kSpeechSoundType, &_speechHandle, ent->data, ent->size, false, 0, 0);
}

void Sound::playMusic(ResourceId resourceId, int32 volume) {
	if (resourceId == kResourceNone)
		return;

	stopMusic();

	ResourceEntry *resource = getResource()->get(resourceId);
	playSoundData(Audio::Mixer::kMusicSoundType, &_musicHandle, resource->data, resource->size, true, volume, 0);
}

void Sound::changeMusic(ResourceId resourceId, int32 musicStatusExt) {
	error("[Sound::changeMusic] not implemented!");
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
