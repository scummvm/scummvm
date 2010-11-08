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

#ifndef ASYLUM_SOUND_H
#define ASYLUM_SOUND_H

#include "asylum/system/config.h"

#include "asylum/shared.h"

#include "common/array.h"

#include "sound/mixer.h"

namespace Asylum {

class AsylumEngine;

struct ResourceEntry;

struct SoundItem {
	ResourceId resourceId;
	int32 field_4;
	int32 field_8;
	int32 field_C;

};

struct FrameSoundItem {
	ResourceId resourceId;
	int32 frameIndex;
	int32 index;
	int32 field_C;
	int32 field_10;
	int32 field_14;

};

struct AmbientSoundItem {
	int32 field_0;
	int32 flags;
	ResourceId resourceId;
	int32 field_C;
	int32 field_10;
	int32 field_14;
	GameFlag flagNum[6];
	int32 x;
	int32 y;

};

struct SoundBufferItem {
	ResourceId resourceId;
	Audio::SoundHandle handle;
	int32 unknown;
};

class Sound {
public:
	Sound(AsylumEngine *engine, Audio::Mixer *mixer);
	~Sound();

	bool addToSoundBuffer(ResourceId resourceId);
    void removeFromSoundBuffer(ResourceId resourceId);
	void clearSoundBuffer();

	/**
	 * Play a sound resource from the supplied resource pack.
	 *
	 * @param overwrite determine if _soundHandle should be overwritten if still active
	 */
	//void playSound(ResourceId resourceId, int32 volume, bool looping = false, int32 panning = 0, bool overwrite = false);
	//void playSound(ResourceId resourceId, bool looping = false, int32 volume = Config.sfxVolume, int32 panning = 0, bool fromBuffer = false);
	void playSound(ResourceId resourceId, bool looping = false, int32 volume = Config.sfxVolume, int32 panning = 0);
	//void playSound(ResourceEntry *resource, bool looping, int32 volume, int32 panning);

	void stopSound(ResourceId resourceId);
	void stopSound();
	void stopAllSounds(bool stopSpeechAndMusic = false);

	void playSpeech(ResourceId resourceId);
	void setSpeech(ResourceId sound, ResourceId speechText);

	void playMusic(ResourceId resourceId, int32 volume = Config.musicVolume);
	void changeMusic(ResourceId resourceId, int32 musicStatusExt);
	void stopMusic();

	void setVolume(ResourceId resourceId, double volume);
	int32 getAdjustedVolume(int32 volume);

	void setPanning(ResourceId resourceId, int32 panning);
	int32 calculatePanningAtPoint(int32 x, int32 y);

	/**
	 * Determine the amount to increase the supplied sound
	 * sample's volume based on the position
	 */
	int32 calculateVolumeAdjustement(int32 x, int32 y, int32 a5, int32 a6);

	/**
	 * Check if the buffered sound sample that matches the provided id currently
	 * has an active handle.
	 *
	 * isPlaying() only manages sound samples, and not
	 * music or speech, as those resources aren't managed beyond simple
	 * start/stop requests.
	 */
	bool isPlaying(ResourceId resourceId);

	/**
	 * Check if the unmanaged sound handle is in use
	 *
	 * This is useful for checking the active state of a sound
	 * in a blowuppuzzle
	 */
	bool isPlaying() { return _mixer->isSoundHandleActive(_soundHandle); }

	// TODO Make sure this is needed
	bool isCacheOk() { error("[Sound::isCacheOk] not implemented!"); }

	// Global resources ids
	ResourceId soundResourceId;
	ResourceId speechTextResourceId;

private:
	AsylumEngine *_vm;

	Audio::Mixer       *_mixer;

	Audio::SoundHandle _speechHandle;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _soundHandle;

	Common::Array<SoundBufferItem> _soundBuffer;

	/**
	 * The resource pointer for the currently playing music file.
	 * This was originally a scene variable, but it makes more sense
	 * to track it uniquely, as this doesn't involve initializing the
	 * scene just to set a single variable
	 */
	uint32 _currentMusicResIndex;

	/**
	 * Find the index within the _soundBuffer array of the
	 * sound sample with provided id.
	 */
	int32  getBufferPosition(ResourceId resourceId);
	void playSoundData(Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte *soundData, int32 soundDataLength, bool loop = false, int32 vol = 0, int32 pan = 0);
};

} // end of namespace Asylum

#endif
