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

#include "asylum/respack.h"
#include "asylum/shared.h"

#include "sound/mixer.h"

namespace Asylum {

typedef struct SoundItem {
	int32 resId;
	int32 field_4;
	int32 field_8;
	int32 field_C;

} SoundItem;

typedef struct FrameSoundItem {
	int32 resId;
	int32 frameIdx;
	int32 index;
	int32 field_C;
	int32 field_10;
	int32 field_14;

} FrameSoundItem;

typedef struct AmbientSoundItem {
	int32 field_0;
	int32 flags;
	int32 resId;
	int32 field_C;
	int32 field_10;
	int32 field_14;
	GameFlag flagNum[6];
	int32 x;
	int32 y;

} AmbientSoundItem;

typedef struct SoundBufferItem {
	int32 resId;
	Audio::SoundHandle handle;
	int32 unknown;
} SoundBufferItem;

class Sound {
public:
	Sound(Audio::Mixer *mixer);
	~Sound();

	bool addToSoundBuffer(int32 resId);
    void removeFromSoundBuffer(int32 resId);
	void clearSoundBuffer();

	/**
	 * Play a sound resource from the supplied resource pack.
	 *
	 * @param overwrite determine if _soundHandle should be overwritten if still active
	 */
	void playSound(ResourcePack *pack, int32 resId, int32 volume, bool looping = false, int32 panning = 0, bool overwrite = false);
	void playSound(ResourcePack *pack, int32 resId, bool looping, int32 volume, int32 panning);
	void playSound(ResourceEntry *resource, bool looping, int32 volume, int32 panning);
	void playSound(int32 resId, bool looping, int32 volume, int32 panning, bool fromBuffer = false);
	void stopSound(int32 resId);
	void stopSound();
	void stopAllSounds(bool stopSpeechAndMusic = false);

	void playSpeech(int32 resId);

	void playMusic(ResourcePack *pack, int32 resId);
	void playMusic(int32 resId);
	void stopMusic();

	/**
	 * Check if the buffered sound sample that matches the provided id currently
	 * has an active handle.
	 *
	 * isPlaying() only manages sound samples, and not
	 * music or speech, as those resources aren't managed beyond simple
	 * start/stop requests.
	 */
	bool isPlaying(int32 resId);

	/**
	 * Check if the unmanaged sound handle is in use
	 *
	 * This is useful for checking the active state of a sound
	 * in a blowuppuzzle
	 */
	bool isPlaying() { return _mixer->isSoundHandleActive(_soundHandle);	}

private:
	Audio::Mixer       *_mixer;

	Audio::SoundHandle _speechHandle;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _soundHandle;

	ResourcePack       *_speechPack;
	ResourcePack       *_soundPack;

	Common::Array<SoundBufferItem> _soundBuffer;

	/**
	 * The resource pointer for the currently playing music file.
	 * This was originally a scene variable, but it makes more sense
	 * to track it uniquely, as this doesn't involve initializing the
	 * scene just to set a single variable
	 */
	int32 _currentMusicResIndex;

	/**
	 * Find the index within the _soundBuffer array of the
	 * sound sample with provided id.
	 */
	int32  getBufferPosition(int32 resId);
	void playSoundData(Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte *soundData, int32 soundDataLength, bool loop = false, int32 vol = 0, int32 pan = 0);
};

} // end of namespace Asylum

#endif
