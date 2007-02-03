/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/mutex.h"


class OSystem;


namespace Audio {

class AudioStream;
class Channel;
class Mixer;

/**
 * A SoundHandle instances corresponds to a specific sound
 * being played via the mixer. It can be used to control that
 * sound (pause it, stop it, etc.).
 * @see The Mixer class
 */
class SoundHandle {
	friend class Channel;
	friend class Mixer;
	uint32 _val;
public:
	inline SoundHandle() : _val(0xFFFFFFFF) {}
};

/**
 * The main audio mixer handles mixing of an arbitrary number of
 * input audio streams (in the form of AudioStream instances).
 */
class Mixer {
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

	int _volumeForSoundType[4];

	uint32 _handleSeed;
	Channel *_channels[NUM_CHANNELS];

	bool _mixerReady;

public:
	Mixer();
	~Mixer();



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
	void playRaw(
		SoundType type,
		SoundHandle *handle,
		void *sound, uint32 size, uint rate, byte flags,
		int id = -1, byte volume = 255, int8 balance = 0,
		uint32 loopStart = 0, uint32 loopEnd = 0);

	/**
	 * Start playing the given audio input stream.
	 *
	 * Note that the sound id assigned below is unique. At most one stream
	 * with a given idea can play at any given time. Trying to play a sound
	 * with an id that is already in use causes the new sound to be not played.
	 *
	 * @param type	the type (voice/sfx/music) of the stream
	 * @param handle	a SoundHandle which can be used to reference and control
	 *                  the stream via suitable mixer methods
	 * @param input	the actual AudioStream to be played
	 * @param id	a unique id assigned to this stream
	 * @param volume	the volume with which to play the sound, ranging from 0 to 255
	 * @param balance	the balance with which to play the sound, ranging from -128 to 127
	 * @param autofreeStream	a flag indicating whether the stream should be
	 *                          freed after playback finished
	 * @param permanent	a flag indicating whether a plain stopAll call should
	 *                  not stop this particular stream
	 */
	void playInputStream(
		SoundType type,
		SoundHandle *handle,
		AudioStream *input,
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
	 * Pause/unpause all sounds, including all regular channels and the
	 * premix channel.
	 *
	 * @param paused true to pause everything, false to unpause
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
	 * Check if a sound with the given handle is active.
	 *
	 * @param handle sound to query
	 * @return true if the sound is active
	 */
	bool isSoundHandleActive(SoundHandle handle);



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
	uint getOutputRate() const;

protected:
	void insertChannel(SoundHandle *handle, Channel *chan);

	/**
	 * Internal main method -- all the actual mixing work is done from here.
	 */
	void mix(int16 * buf, uint len);

	// FIXME: temporary "public" to allow access to mixCallback
	// from within OSystem::makeMixer()
public:
	/**
	 * The mixer callback function, passed on to OSystem::setSoundCallback().
	 * This simply calls the mix() method.
	 */
	static void mixCallback(void *s, byte *samples, int len);
	
	void setReady(bool ready) { _mixerReady = ready; }
};


} // End of namespace Audio

#endif
