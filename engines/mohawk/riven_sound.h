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

#ifndef MOHAWK_RIVEN_SOUND_H
#define MOHAWK_RIVEN_SOUND_H

#include "common/array.h"
#include "common/str.h"

#include "audio/mixer.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Mohawk {

class MohawkEngine_Riven;
class RivenSound;

/**
 * Ambient sound list
 */
struct SLSTRecord {
	uint16 index;
	Common::Array<uint16> soundIds;
	uint16 fadeFlags;
	uint16 loop;
	uint16 globalVolume;
	uint16 u0;
	uint16 suspend;
	Common::Array<uint16> volumes;
	Common::Array<int16> balances;
	Common::Array<uint16> u2;
};

/**
 * Sound manager for Riven
 *
 * The sound manager can play simulteaneously:
 * - An effect sound
 * - A list of ambient sounds
 *
 * The list of ambient sounds can be cross faded
 * with the previously running ambient sounds.
 */
class RivenSoundManager {
public:
	explicit RivenSoundManager(MohawkEngine_Riven *vm);
	~RivenSoundManager();

	/**
	 * Play an effect sound
	 *
	 * @param id Sound ID in the stack
	 * @param volume Playback volume, between 0 and 255
	 * @param playOnDraw Start playing when the current card is drawn instead of immediately
	 */
	void playSound(uint16 id, uint16 volume = 255, bool playOnDraw = false);

	/** Play an effect sound by its resource name */
	void playCardSound(const Common::String &name, uint16 volume = 255, bool playOnDraw = false);

	/** Start playing the scheduled on-draw effect sound, if any. Called by the GraphicsManager. */
	void triggerDrawSound();

	/** Is an effect sound currently playing? */
	bool isEffectPlaying() const;

	/** Stop playing the current effect sound, if any */
	void stopSound();

	/** Start playing an ambient sound list */
	void playSLST(const SLSTRecord &slstRecord);

	/** Stop playing the current ambient sounds */
	void stopAllSLST(bool fade = false);

	/** Update the ambient sounds for fading. Called once per frame. */
	void updateSLST();

private:
	struct AmbientSound {
		RivenSound *sound;
		uint16 targetVolume;
		int16 targetBalance;

		AmbientSound();
	};

	struct AmbientSoundList {
		bool fading;
		bool suspend;
		Common::Array<AmbientSound> sounds;

		AmbientSoundList();
	};

	enum FadeFlags {
		kFadeOutPreviousSounds = 1,
		kFadeInNewSounds = 2
	};

	MohawkEngine_Riven *_vm;

	int16 _mainAmbientSoundId;
	AmbientSoundList _ambientSounds;
	AmbientSoundList _previousAmbientSounds;
	uint32 _nextFadeUpdate;

	RivenSound *_effect;
	bool _effectPlayOnDraw;

	Audio::RewindableAudioStream *makeAudioStream(uint16 id);

	// Ambient sound management
	void addAmbientSounds(const SLSTRecord &record);
	void playAmbientSounds();
	void pauseAmbientSounds();
	void moveAmbientSoundsToPreviousSounds();
	void freePreviousAmbientSounds();

	// Ambient sound fading
	void setTargetVolumes(const SLSTRecord &record);
	void applyTargetVolumes();
	void startFadingAmbientSounds(uint16 flags);
	void fadeAmbientSoundList(AmbientSoundList &list);
	bool fadeVolume(AmbientSound &ambientSound);
	bool fadeBalance(AmbientSound &ambientSound);
	void setAmbientLooping(bool loop);
};

/**
 * A sound used internally by the SoundManager
 */
class RivenSound {
public:
	RivenSound(MohawkEngine_Riven *vm, Audio::RewindableAudioStream *rewindStream, Audio::Mixer::SoundType mixerType);
	~RivenSound();

	/** Start playing the sound stream passed to the constructor */
	void play();

	/** Is the sound currently playing ar paused? */
	bool isPlaying() const;

	/** Pause the playback, the play method resumes */
	void pause();

	/** Get the current volume */
	uint16 getVolume() const;

	/** Change the playback volume */
	void setVolume(uint16 volume);

	/** Get the current balance */
	int16 getBalance() const;

	/** Change the balance */
	void setBalance(int16 balance);

	/** Set the sound to indefinitely loop. Must be called before starting the playback */
	void setLooping(bool loop);

private:
	static byte convertVolume(uint16 volume);
	static int8 convertBalance(int16 balance);

	MohawkEngine_Riven *_vm;

	Audio::SoundHandle _handle;
	Audio::Mixer::SoundType _mixerType;
	Audio::RewindableAudioStream *_stream;

	uint16 _volume;
	int16 _balance;
	bool _looping;
};

} // End of namespace Mohawk

#endif
