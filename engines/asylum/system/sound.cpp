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

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include "common/stream.h"

#include "sound/audiostream.h"
#include "sound/decoders/adpcm.h"
#include "sound/decoders/wave.h"

namespace Asylum {

Sound::Sound(AsylumEngine *engine, Audio::Mixer *mixer) : _vm(engine), _mixer(mixer) {
}

Sound::~Sound() {
	clearSoundBuffer();

	// Zero-out passed pointers
	_vm = NULL;
	_mixer = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Playing sounds & music
//////////////////////////////////////////////////////////////////////////

void Sound::playSound(ResourceId resourceId, bool looping, int32 volume, int32 panning) {
	SoundBufferItem *item = getItem(resourceId);

	if (item) {
		if (_mixer->isSoundHandleActive(item->handle)) {
			debugC(kDebugLevelSound, "[Sound::playSound] handle for resource %d already active", resourceId);

			return;
		}

		//warning("[Sound::playSound] resource %d already buffered", resourceId);

		// TODO check what we should do here
		return;
	}

	ResourceEntry *resource = getResource()->get(resourceId);
	playSoundData(Audio::Mixer::kSFXSoundType, &_soundHandle, resource->data, resource->size, looping, volume, panning);
	addToSoundBuffer(resourceId);
}

void Sound::playMusic(ResourceId resourceId, int32 volume) {
	if (resourceId == kResourceNone) {
		stopMusic();
		return;
	}

	// Sets the music volume
	setMusicVolume(volume);

	// Check if music is already playing
	if (_mixer->isSoundHandleActive(_musicHandle))
		return;

	ResourceEntry *resource = getResource()->get(resourceId);
	playSoundData(Audio::Mixer::kMusicSoundType, &_musicHandle, resource->data, resource->size, true, volume, 0);
}

void Sound::changeMusic(uint32 index, int32 musicStatusExt) {
	if (index != getWorld()->musicCurrentResourceIndex) {
		getWorld()->musicCurrentResourceIndex = index;
		getWorld()->musicStatusExt = musicStatusExt;
		getWorld()->musicFlag = 1;
	}
}

bool Sound::isPlaying(ResourceId resourceId) {
	return (getPlayingItem(resourceId) != NULL);
}

//////////////////////////////////////////////////////////////////////////
// Volume & panning
//////////////////////////////////////////////////////////////////////////

void Sound::setVolume(ResourceId resourceId, int32 volume) {
	SoundBufferItem *item = getPlayingItem(resourceId);
	if (!item)
		return;

	convertVolume(volume);

	_mixer->setChannelVolume(item->handle, (byte)volume);
}

void Sound::setMusicVolume(int32 volume) {
	if (volume < -10000)
		return;

	convertVolume(volume);

	_mixer->setChannelVolume(_musicHandle, (byte)volume);
}

void Sound::setPanning(ResourceId resourceId, int32 panning) {
	if (Config.performance == 1)
		return;

	SoundBufferItem *item = getPlayingItem(resourceId);
	if (!item)
		return;

	convertPan(panning);

	_mixer->setChannelBalance(item->handle, (int8)panning);
}

int32 Sound::calculateVolumeAdjustement(int32 x, int32 y, int32 attenuation, int32 delta) {
	if (!attenuation)
		return -(delta * delta);

	Actor *player = getScene()->getActor();
	if (getScene()->getGlobalX() == -1) {
		x -= (player->x1 + player->x2);
		y -= (player->y1 + player->y2);
	} else {
		x -= getScene()->getGlobalX();
		y -= getScene()->getGlobalY();
	}

	int32 adjustedVolume = getAdjustedVolume(x * x + y * y);

	Common::Rational invAtt(100, attenuation);
	Common::Rational v;
	if (invAtt.toInt())
		v = Common::Rational(adjustedVolume, 1) / invAtt;
	else
		v = Common::Rational(delta, 1);

	int32 volume = (v.toInt() - delta) * (v.toInt() - delta);

	if (volume > 10000)
		return -10000;

	return -volume;
}

int32 Sound::getAdjustedVolume(int32 volume) {
	if (volume < 2)
		return volume;

	//warning("[Sound::getAdjustedVolume] not implemented");

	return volume;
}

int32 Sound::calculatePanningAtPoint(int32 x, int32) {
	// y does not seem to be used at all :S

	int delta = x - getWorld()->xLeft;

	if (delta < 0)
		return (getWorld()->stereoReversedFlag ? 10000 : -10000);

	if (delta >= 640)
		return (getWorld()->stereoReversedFlag ? -10000 : 10000);


	int sign, absDelta;
	if (delta > 320) {
		absDelta = delta - 320;
		sign = (getWorld()->stereoReversedFlag ? -1 : 1);
	} else {
		absDelta = 320 - delta;
		sign = (getWorld()->stereoReversedFlag ? 1 : -1);
	}

	Common::Rational v(absDelta, 6);
	int32 volume = v.toInt() * v.toInt();

	if (volume > 10000)
		volume = 10000;

	return volume * sign;
}

//////////////////////////////////////////////////////////////////////////
// Stopping sounds
//////////////////////////////////////////////////////////////////////////
void Sound::stop(ResourceId resourceId) {
	SoundBufferItem *item = getPlayingItem(resourceId);

	if (item != NULL)
		_mixer->stopHandle(item->handle);
}

void Sound::stopAll(ResourceId resourceId) {
	for (Common::Array<SoundBufferItem>::iterator it = _soundBuffer.begin(); it != _soundBuffer.end(); it++)
		if (it->resourceId == resourceId)
			_mixer->stopHandle(it->handle);
}

void Sound::stopAll() {
	for (Common::Array<SoundBufferItem>::iterator it = _soundBuffer.begin(); it != _soundBuffer.end(); it++)
		_mixer->stopHandle(it->handle);
}

void Sound::stopMusic() {
	_mixer->stopHandle(_musicHandle);
}

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////

void Sound::playSoundData(Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte *soundData, uint32 soundDataLength, bool loop, int32 vol, int32 pan) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(soundData, soundDataLength);
	Audio::RewindableAudioStream *sndStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	// Convert volume and panning
	convertVolume(vol);
	convertPan(pan);

	_mixer->playStream(type, handle, Audio::makeLoopingAudioStream(sndStream, loop ? 0 : 1), -1, (byte)vol, (int8)pan);
}

//////////////////////////////////////////////////////////////////////////
// Sound buffer
//////////////////////////////////////////////////////////////////////////

SoundBufferItem *Sound::getItem(ResourceId resourceId) {
	for (uint32 i = 0; i < _soundBuffer.size(); i++)
		if (resourceId == _soundBuffer[i].resourceId)
			return &_soundBuffer[i];

	return NULL;
}

SoundBufferItem *Sound::getPlayingItem(ResourceId resourceId) {
	for (uint32 i = 0; i < _soundBuffer.size(); i++)
		if (resourceId == _soundBuffer[i].resourceId
			&& _mixer->isSoundHandleActive(_soundBuffer[i].handle))
			return &_soundBuffer[i];

	return NULL;
}

bool Sound::addToSoundBuffer(ResourceId resourceId) {
	SoundBufferItem *item = getItem(resourceId);

	if (item == NULL) {
		SoundBufferItem sound;
		sound.resourceId = resourceId;
		sound.handle = _soundHandle;
		_soundBuffer.push_back(sound);
	}

	return (item == NULL) ? true : false;
}

void Sound::removeFromSoundBuffer(ResourceId resourceId) {
	for (uint i = 0; i < _soundBuffer.size(); i++) {
		if (_soundBuffer[i].resourceId == resourceId) {
			_soundBuffer.remove_at(i);
			break;
		}
	}
}

void Sound::clearSoundBuffer() {
	_soundBuffer.clear();
}

//////////////////////////////////////////////////////////////////////////
// Conversion functions
//
// Those are from engines/agos/sound.cpp (FIXME: Move to common code?)
//////////////////////////////////////////////////////////////////////////

void Sound::convertVolume(int32 &vol) {
	// DirectSound was originally used, which specifies volume
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

void Sound::convertPan(int32 &pan) {
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

} // end of namespace Asylum
