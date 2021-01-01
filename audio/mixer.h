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

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "common/types.h"
#include "common/noncopyable.h"

namespace Audio {

class AudioStream;
class Channel;
class Timestamp;

/**
 * @defgroup audio_mixer Mixer
 * @ingroup audio
 *
 * @brief Mixer class used for playing audio streams.
 * @{
 */

/**
 * A SoundHandle instance corresponds to a specific sound
 * being played using the mixer. It can be used to control that
 * sound (pause it, stop it, etc.).
 * @see Mixer
 */
class SoundHandle {
	friend class Channel;
	friend class MixerImpl;
	uint32 _val;
public:
	inline SoundHandle() : _val(0xFFFFFFFF) {}
};

/**
 * The main audio mixer that handles mixing of an arbitrary number of
 * audio streams (in the form of AudioStream instances).
 */
class Mixer : Common::NonCopyable {
public:
	/** Sound types. */
	enum SoundType {
		kPlainSoundType = 0, /*!< Plain sound. */

		kMusicSoundType = 1, /*!< Music. */
		kSFXSoundType = 2,   /*!< Sound effects. */
		kSpeechSoundType = 3 /*!< Speech. */
	};
	/** Max volumes. */
	enum {
		kMaxChannelVolume = 255, /*!< Max channel volume. */
		kMaxMixerVolume = 256    /*!< Max global volume. */
	};

public:
	Mixer() {}
	virtual ~Mixer() {}



	/**
	 * Check whether the mixer is ready and set up.
	 *
	 * The mixer might not be set up on systems that do not support
	 * digital sound output. In such case, the mixer processing might
	 * never be called. That, in turn, can cause breakage in games that try to
	 * sync with an audio stream. In particular, the AdLib MIDI emulation.
	 *
	 * @return Whether the mixer is ready and set up.
	 *
	 * @todo get rid of this?
	 */
	virtual bool isReady() const = 0;


	/**
	 * Start playing the given audio stream.
	 *
	 * Note that the sound ID assigned here is unique. At most, one stream
	 * with the given ID can play at any given time. Trying to play a sound
	 * with an ID that is already in use causes the new sound to not be played.
	 *
	 * @param type      Type of the stream - voice/SFX/music.
	 * @param handle    A SoundHandle instance that can be used to reference and control
	 *                  the stream using suitable mixer methods.
	 * @param stream    The actual AudioStream to be played.
	 * @param id        Unique ID assigned to this stream.
	 * @param volume    Volume with which to play the sound, ranging from 0 to 255.
	 * @param balance	Balance with which to play the sound, ranging from -127 to 127 (full left to full right).
	 *                  0 is balanced, -128 is invalid.
	 * @param autofreeStream  If set, the stream will be freed after the playback is finished.                  
	 * @param permanent       If set, a plain stopAll call will not stop this particular stream.          
	 * @param reverseStereo   If set, left and right channels will be swapped.
	 */
	virtual void playStream(
		SoundType type,
		SoundHandle *handle,
		AudioStream *stream,
		int id = -1,
		byte volume = kMaxChannelVolume,
		int8 balance = 0,
		DisposeAfterUse::Flag autofreeStream = DisposeAfterUse::YES,
		bool permanent = false,
		bool reverseStereo = false) = 0;

	/**
	 * Stop all currently playing sounds.
	 */
	virtual void stopAll() = 0;

	/**
	 * Stop playing the sound with the given ID.
	 *
	 * @param id  ID of the sound.
	 */
	virtual void stopID(int id) = 0;

	/**
	 * Stop playing the sound corresponding to the given handle.
	 *
	 * @param handle  The sound to stop playing.
	 */
	virtual void stopHandle(SoundHandle handle) = 0;



	/**
	 * Pause or unpause all sounds, including all regular and permanent
	 * channels.
	 *
	 * @param paused  True to pause everything, false to unpause.
	 */
	virtual void pauseAll(bool paused) = 0;

	/**
	 * Pause or unpause the sound with the given ID.
	 *
	 * @param id      ID of the sound.
	 * @param paused  True to pause the sound, false to unpause it.
	 */
	virtual void pauseID(int id, bool paused) = 0;

	/**
	 * Pause or unpause the sound corresponding to the given handle.
	 *
	 * @param handle  The sound to pause or unpause.
	 * @param paused  True to pause the sound, false to unpause it.
	 */
	virtual void pauseHandle(SoundHandle handle, bool paused) = 0;



	/**
	 * Check whether a sound with the given ID is active.
	 *
	 * @param id  ID of the sound to query.
	 *
	 * @return True if the sound is active.
	 */
	virtual bool isSoundIDActive(int id) = 0;

	/**
	 * Get the sound ID for the given handle.
	 *
	 * @param handle The sound to query.
	 *
	 * @return Sound ID if the sound is active.
	 */
	virtual int getSoundID(SoundHandle handle) = 0;

	/**
	 * Check whether a sound with the given handle is active.
	 *
	 * @param handle The sound to query.
	 *
	 * @return True if the sound is active.
	 */
	virtual bool isSoundHandleActive(SoundHandle handle) = 0;


	/**
	 * Set the mute state for a given sound type.
	 *
	 * @param type Sound type. See @ref SoundType.
	 * @param mute Whether to mute (= true) or not (= false).
	 */
	virtual void muteSoundType(SoundType type, bool mute) = 0;

	/**
	 * Query the mute state for a given sound type.
	 *
	 * @param type Sound type. See @ref SoundType.
	 */
	virtual bool isSoundTypeMuted(SoundType type) const = 0;

	/**
	 * Set the channel volume for the given handle.
	 *
	 * @param handle  The sound to affect.
	 * @param volume  The new channel volume, in the range 0 - kMaxChannelVolume.
	 */
	virtual void setChannelVolume(SoundHandle handle, byte volume) = 0;

	/**
	 * Get the channel volume for the given handle.
	 *
	 * @param handle  The sound to affect.
	 *
	 * @return The channel volume.
	 */
	virtual byte getChannelVolume(SoundHandle handle) = 0;

	/**
	 * Set the channel balance for the given handle.
	 *
	 * @param handle   The sound to affect.
	 * @param balance  The new channel balance:
	 *                 (-127 ... 0 ... 127) corresponds to (left ... center ... right)
	 */
	virtual void setChannelBalance(SoundHandle handle, int8 balance) = 0;

	/**
	 * Get the channel balance for the given handle.
	 *
	 * @param handle  The sound to affect.
	 *
	 * @return The channel balance.
	 */
	virtual int8 getChannelBalance(SoundHandle handle) = 0;

	/**
	 * Get an approximation of for how long the channel has been playing.
	 */
	virtual uint32 getSoundElapsedTime(SoundHandle handle) = 0;

	/**
	 * Get an approximation of for how long the channel has been playing.
	 */
	virtual Timestamp getElapsedTime(SoundHandle handle) = 0;

	/**
	 * Check whether any channel of the given sound type is active.
	 *
	 * For example, this can be used to check whether any SFX sound
	 * is currently playing by checking for type kSFXSoundType.
	 *
	 * @param  type  The sound type to query.
	 *
	 * @return True if any channels of the specified type are active.
	 */
	virtual bool hasActiveChannelOfType(SoundType type) = 0;

	/**
	 * Set the volume for the given sound type.
	 *
	 * @param type    Sound type.
	 * @param volume  The new global volume, in the range 0 - kMaxMixerVolume.
	 */
	virtual void setVolumeForSoundType(SoundType type, int volume) = 0;

	/**
	 * Check what the global volume is for a sound type.
	 *
	 * @param type  Sound type.
	 *
	 * @return The global volume, in the range 0 - kMaxMixerVolume.
	 */
	virtual int getVolumeForSoundType(SoundType type) const = 0;

	/**
	 * Return the output sample rate of the system.
	 *
	 * @return The output sample rate in Hz.
	 */
	virtual uint getOutputRate() const = 0;
};

/** @} */
} // End of namespace Audio

#endif
