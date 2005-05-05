/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/mutex.h"


class AudioStream;
class Channel;
class File;
class OSystem;

class SoundHandle {
	friend class Channel;
	friend class SoundMixer;
	uint32 _val;
public:
	inline SoundHandle() : _val(0xFFFFFFFF) {}
};


class SoundMixer {
public:
	enum {
		/** unsigned samples (default: signed) */
		FLAG_UNSIGNED = 1 << 0,

		/** sound is 16 bits wide (default: 8bit) */
		FLAG_16BITS = 1 << 1,

		/** sample is little endian (default: big endian) */
		FLAG_LITTLE_ENDIAN = 1 << 2,

		/** sound is in stereo (default: mono) */
		FLAG_STEREO = 1 << 3,

		/** reverse the left and right stereo channel */
		FLAG_REVERSE_STEREO = 1 << 4,

		/** sound buffer is freed automagically at the end of playing */
		FLAG_AUTOFREE = 1 << 5,

		/** loop the audio */
		FLAG_LOOP = 1 << 6
	};
	
	enum SoundType {
		kPlainSoundType = 0,

		kMusicSoundType = 1,
		kSFXSoundType = 2,
		kSpeechSoundType = 3
	};
	
	enum {
		kMaxChannelVolume = 255,
		kMaxMixerVolume = 256
	};

private:
	enum {
		NUM_CHANNELS = 16
	};

	OSystem *_syst;
	Common::Mutex _mutex;

	Channel *_premixChannel;

	uint _outputRate;

	int _volumeForSoundType[4];

	bool _paused;
	
	uint32 _handleSeed;
	Channel *_channels[NUM_CHANNELS];

	bool _mixerReady;

public:
	SoundMixer();
	~SoundMixer();



	/**
	 * Is the mixer ready and setup? This may not be the case on systems which
	 * don't support digital sound output. In that case, the mixer proc may
	 * never be called. That in turn can cause breakage in games which use the
	 * premix callback for syncing. In particular, the Adlib MIDI emulation...
	 *
	 * @return whether the mixer is ready and setup
	 */
	bool isReady() const { return _mixerReady; };



	/**
	 * Set the premix stream. This is mainly used for the adlib music, but
	 * is not limited to it. The premix stream is invoked by the mixer whenever
	 * it needs to generate any data, before any other mixing takes place.
	 */
	void setupPremix(AudioStream *stream, SoundType type = kPlainSoundType);



	/**
	 * Start playing the given raw sound data.
	 * Internally, this simply creates an audio input stream wrapping the data
	 * (using the makeLinearInputStream factory function), which is then
	 * passed on to playInputStream.
	 */
	void playRaw(SoundHandle *handle,
				void *sound, uint32 size, uint rate, byte flags,
				int id = -1, byte volume = 255, int8 balance = 0,
				uint32 loopStart = 0, uint32 loopEnd = 0,
				SoundType type = kSFXSoundType);

	/**
	 * Start playing the given audio input stream.
	 */
	void playInputStream(SoundType type, SoundHandle *handle, AudioStream *input,
				int id = -1, byte volume = 255, int8 balance = 0,
				bool autofreeStream = true, bool permanent = false);



	/**
	 * Stop all currently playing sounds.
	 */
	void stopAll(bool force = false);

	/**
	 * Stop playing the sound with given ID.
	 *
	 * @param id the ID of the sound to affect
	 */
	void stopID(int id);

	/**
	 * Stop playing the sound corresponding to the given handle.
	 *
	 * @param handle the sound to affect
	 */
	void stopHandle(SoundHandle handle);



	/**
	 * Pause/unpause the mixer (this temporarily stops all audio processing,
	 * including all regular channels and the premix channel).
	 *
	 * @param paused true to pause the mixer, false to unpause it
	 */
	void pauseAll(bool paused);

	/**
	 * Pause/unpause the sound with the given ID.
	 *
	 * @param id the ID of the sound to affect
	 * @param paused true to pause the sound, false to unpause it
	 */
	void pauseID(int id, bool paused);

	/**
	 * Pause/unpause the sound corresponding to the given handle.
	 *
	 * @param handle the sound to affect
	 * @param paused true to pause the sound, false to unpause it
	 */
	void pauseHandle(SoundHandle handle, bool paused);



	/**
	 * Check if a sound with the given ID is active.
	 *
	 * @param id the ID of the sound to query
	 * @return true if the sound is active
	 */
	bool isSoundIDActive(int id);

	/**
	 * Get the sound ID of handle sound
	 *
	 * @param handle sound to query
	 * @return sound ID if active
	 */
	int getSoundID(SoundHandle handle);

	/**
	 * Check if a sound with the given hANDLE is active.
	 *
	 * @param handle sound to query
	 * @return true if the sound is active
	 */
	bool isSoundHandleActive(SoundHandle handle);

	/**
	 * Check if the mixer is paused (using pauseAll).
	 *
	 * @return true if the mixer is paused
	 */
	bool isPaused();



	/**
	 * Set the channel volume for the given handle.
	 *
	 * @param handle the sound to affect
	 * @param volume the new channel volume (0 - 255)
	 */
	void setChannelVolume(SoundHandle handle, byte volume);

	/**
	 * Set the channel balance for the given handle.
	 *
	 * @param handle the sound to affect
	 * @param balance the new channel balance:
	 *        (-127 ... 0 ... 127) corresponds to (left ... center ... right)
	 */
	void setChannelBalance(SoundHandle handle, int8 balance);

	/**
	 * Get approximation of for how long the Sound ID has been playing.
	 */
	uint32 getSoundElapsedTimeOfSoundID(int id);

	/**
	 * Get approximation of for how long the channel has been playing.
	 */
	uint32 getSoundElapsedTime(SoundHandle handle);

	/**
	 * Check whether any channel of the given sound type is active.
	 * For example, this can be used to check whether any SFX sound
	 * is currently playing, by checking for type kSFXSoundType.
	 *
	 * @param  type the sound type to look for
	 * @return true if any channels of the specified type are active.
	 */
	bool hasActiveChannelOfType(SoundType type);

	/**
	 * Set the volume for the given sound type.
	 *
	 * @param type the sound type
	 * @param volume the new global volume, 0-kMaxMixerVolume
	 */
	void setVolumeForSoundType(SoundType type, int volume);

	/**
	 * Query the global volume.
	 *
	 * @param type the sound type
	 * @return the global music volume, 0-kMaxMixerVolume
	 */
	int getVolumeForSoundType(SoundType type) const;

	/**
	 * Query the system's audio output sample rate. This returns
	 * the same value as OSystem::getOutputSampleRate().
	 *
	 * @return the output sample rate in Hz
	 */
	uint getOutputRate() const { return _outputRate; }

private:
	void insertChannel(SoundHandle *handle, Channel *chan);

	/**
	 * Internal main method -- all the actual mixing work is done from here.
	 */
	void mix(int16 * buf, uint len);

	/**
	 * The mixer callback function, passed on to OSystem::setSoundCallback().
	 * This simply calls the mix() method.
	 */
	static void mixCallback(void *s, byte *samples, int len);
};

#endif
