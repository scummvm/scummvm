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

#ifndef ASYLUM_SOUND_H_
#define ASYLUM_SOUND_H_

#include "sound/mixer.h"

#include "asylum/respack.h"

namespace Asylum {

typedef struct SoundItem {
	uint32 resId;
	uint32 field_4;
	uint32 field_8;
	uint32 field_C;

} SoundItem;

typedef struct FrameSoundItem {
	uint32 resId;
	uint32 frameIdx;
	uint32 index;
	uint32 field_C;
	uint32 field_10;
	uint32 field_14;

} FrameSoundItem;

typedef struct AmbientSoundItem {
	uint32 field_0;
	uint32 flags;
	uint32 resId;
	uint32 field_C;
	uint32 field_10;
	uint32 field_14;
	uint32 flagNum[6];
	uint32 x;
	uint32 y;

} AmbientSoundItem;

typedef struct SoundBufferItem {
	uint32 resId;
	Audio::SoundHandle handle;
	uint32 unknown;
} SoundBufferItem;

class Sound {
public:
	Sound(Audio::Mixer *mixer);
	~Sound();

	bool addToSoundBuffer(uint resId);
	void clearSoundBuffer();

	/**
	 * Play a sound resource from the supplied resource pack.
	 *
	 * @param overwrite determine if _soundHandle should be overwritten if still active
	 */
	void playSound(ResourcePack *pack, uint resId, int volume, bool looping = false, int panning = 0, bool overwrite = false);
	void playSound(ResourcePack *pack, uint resId, bool looping, int volume, int panning);
	void playSound(ResourceEntry *resource, bool looping, int volume, int panning);
	void playSound(uint resId, bool looping, int volume, int panning, bool fromBuffer = false);
	void stopSound(uint resId);
	void stopSound();
	void stopAllSounds(bool stopSpeechAndMusic = false);

	void playSpeech(uint resId);

	void playMusic(ResourcePack *pack, uint resId);
	void playMusic(uint resId);
	void stopMusic();

	/**
	 * Check if the buffered sound sample that matches the provided id currently
	 * has an active handle.
	 *
	 * isPlaying() only manages sound samples, and not
	 * music or speech, as those resources aren't managed beyond simple
	 * start/stop requests.
	 */
	bool isPlaying(uint resId);

	/**
	 * Check if the unmanaged sound handle is in use
	 *
	 * This is useful for checking the active state of a sound
	 * in a blowuppuzzle
	 */
	bool isPlaying() { return _mixer->isSoundHandleActive(_soundHandle); }

private:
	Audio::Mixer       *_mixer;

	Audio::SoundHandle _speechHandle;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _soundHandle;

	ResourcePack       *_speechPack;
	ResourcePack       *_soundPack;

	Common::Array<SoundBufferItem> _soundBuffer;

	/**
	 * Find the index within the _soundBuffer array of the
	 * sound sample with provided id.
	 */
	int  getBufferPosition(uint32 resId);
	void playSoundData(Audio::SoundHandle *handle, byte *soundData, uint32 soundDataLength, bool loop = false, int vol = 0, int pan = 0);
};

} // end of namespace Asylum

#endif
