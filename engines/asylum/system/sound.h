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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_SYSTEM_SOUND_H
#define ASYLUM_SYSTEM_SOUND_H

#include "common/array.h"
#include "common/rect.h"

#include "audio/mixer.h"

#include "asylum/system/config.h"

#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

struct ResourceEntry;

struct SoundItem {
	ResourceId resourceId;
	int32 field_4;
	int32 field_8;
	int32 field_C;

	SoundItem() {
		resourceId = kResourceNone;
		field_4 = 0;
		field_8 = 0;
		field_C = 0;
	}
};

struct FrameSoundItem {
	ResourceId resourceId;
	uint32 frameIndex;
	int32 index;
	int32 field_C;
	int32 field_10;
	int32 field_14;

	FrameSoundItem() {
		resourceId = kResourceNone;
		frameIndex = 0;
		index = 0;
		field_C = 0;
		field_10 = 0;
		field_14 = 0;
	}
};

struct AmbientSoundItem {
	int32 field_0;
	int32 flags;
	ResourceId resourceId;
	int32 delta;
	int32 attenuation;
	int32 nextTick;
	int32 flagNum[6];
	Common::Point point;

	AmbientSoundItem() {
		field_0 = 0;
		flags = 0;
		resourceId = kResourceNone;
		delta = 0;
		attenuation = 0;
		nextTick = 0;
		memset(&flagNum, 0, sizeof(flagNum));
	}
};

struct SoundQueueItem {
	ResourceId resourceId;
	Audio::SoundHandle handle;

	SoundQueueItem() {
		resourceId = kResourceNone;
	}
};

class Sound {
public:
	Sound(AsylumEngine *engine, Audio::Mixer *mixer);
	~Sound();

	//////////////////////////////////////////////////////////////////////////
	// Playing sounds & music
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Play sound
	 *
	 * @param resourceId Identifier for the resource.
	 * @param looping    true to looping.
	 * @param volume 	 The volume.
	 * @param panning    The panning.
	 */
	void playSound(ResourceId resourceId, bool looping = false, int32 volume = Config.sfxVolume, int32 panning = 0);

	/**
	 * Play music
	 *
	 * @param resourceId Identifier for the resource.
	 * @param volume 	 The volume.
	 */
	void playMusic(ResourceId resourceId, int32 volume = Config.musicVolume);

	/**
	 * Change music
	 *
	 * @param index 		 Zero-based index of the music
	 * @param musicStatusExt The music status.
	 */
	void changeMusic(int32 index, int32 musicStatusExt);

	/**
	 * Query if a sound with the resource id is playing.
	 *
	 * @param resourceId Identifier for the resource.
	 *
	 * @return true if playing, false if not.
	 */
	bool isPlaying(ResourceId resourceId);

	//////////////////////////////////////////////////////////////////////////
	// Volume & panning
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sets the volume for a buffered sound resource
	 *
	 * @param resourceId Identifier for the resource.
	 * @param volume 	 The volume.
	 */
	void setVolume(ResourceId resourceId, int32 volume);

	/**
	 * Sets the music volume.
	 *
	 * @param volume The volume.
	 */
	void setMusicVolume(int32 volume);

	/**
	 * Gets the music volume.
	 *
	 * @return The music volume.
	 */
	int32 getMusicVolume() { return _musicVolume; }

	/**
	 * Sets the panning for a buffered sound resource
	 *
	 * @param resourceId Identifier for the resource.
	 * @param panning    The panning.
	 */
	void setPanning(ResourceId resourceId, int32 panning);

	/**
	 * Determine the amount to increase the supplied sound sample's volume based on the position.
	 *
	 * @param point       The coordinates
	 * @param attenuation The attenuation.
	 * @param delta       The delta.
	 *
	 * @return The calculated volume adjustement.
	 */
	int32 calculateVolumeAdjustement(const Common::Point &point, int32 attenuation, int32 delta);

	/**
	 * Gets an adjusted volume.
	 *
	 * @param volume The volume.
	 *
	 * @return The adjusted volume.
	 */
	int32 getAdjustedVolume(int32 volume) const;

	/**
	 * Calculates the panning at point.
	 *
	 * @param point The coordinates.
	 *
	 * @return The calculated panning at point.
	 */
	int32 calculatePanningAtPoint(const Common::Point &point);

	//////////////////////////////////////////////////////////////////////////
	// Stopping sounds
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Stop the first playing sound with the ResourceId
	 *
	 * @param resourceId Identifier for the resource.
	 */
	void stop(ResourceId resourceId);

	/**
	 * Stop all sounds with the ResourceId
	 *
	 * @param resourceId Identifier for the resource.
	 */
	void stopAll(ResourceId resourceId);

	/**
	 * Stop all buffered sounds
	 */
	void stopAll();

	/**
	 * Stop music.
	 */
	void stopMusic();

	//////////////////////////////////////////////////////////////////////////
	// Conversion functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Convert volume to a ScummVM mixer value
	 *
	 * @param [in,out] vol The volume.
	 */
	static void convertVolumeFrom(int32 &vol);

	/**
	 * Convert ScummVM mixer value to a volume
	 *
	 * @param [in,out] vol The volume.
	 */
	static void convertVolumeTo(int32 &vol);

private:
	AsylumEngine *_vm;

	Audio::Mixer       *_mixer;

	Audio::SoundHandle _musicHandle;
	int32 _musicVolume;

	Common::Array<SoundQueueItem> _soundQueue;

	//////////////////////////////////////////////////////////////////////////
	// Sound queue
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Find the index within the sound queue of the sound sample with provided id.
	 *
	 * @param resourceId Identifier for the resource.
	 *
	 * @return The item.
	 */
	SoundQueueItem *getItem(ResourceId resourceId);

	/**
	 * Find the index within the sound queue of the playing sound sample with provided id.
	 *
	 * @param resourceId Identifier for the resource.
	 *
	 * @return The playing item.
	 */
	SoundQueueItem *getPlayingItem(ResourceId resourceId);

	/**
	 * Adds a sound to the sound queue.
	 *
	 * @param resourceId Identifier for the resource.
	 *
	 * @return the sound buffer item
	 */
	SoundQueueItem *addToQueue(ResourceId resourceId);

	/**
	 * Clears the sound queue from finished sounds
	 */
	void cleanupQueue();

	//////////////////////////////////////////////////////////////////////////
	// Helper functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Checks if the sound file is valid
	 *
	 * @return true if valid, false if not.
	 */
	bool isValidSoundResource(ResourceId resourceId);

	/**
	 * Play sound data.
	 *
	 * @param type 				 The type.
	 * @param handle             The handle.
	 * @param soundData          The sound data
	 * @param soundDataLength    Length of the sound data.
	 * @param loop 				 true to loop.
	 * @param vol 				 The volume.
	 * @param pan 				 The pan.
	 */
	void playSoundData(Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte *soundData, uint32 soundDataLength, bool loop = false, int32 vol = 0, int32 pan = 0);

	//////////////////////////////////////////////////////////////////////////
	// Conversion functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Convert pan.
	 *
	 * @param [in,out] pan The pan.
	 */
	static void convertPan(int32 &pan);
};

} // end of namespace Asylum

#endif // ASYLUM_SYSTEM_SOUND_H
