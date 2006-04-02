/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include "stdafx.h"
#include "bits.h"

class AudioStream;
class Channel;

class PlayingSoundHandle {
	friend class Channel;
	friend class SoundMixer;
	int val;
	int getIndex() const { return val - 1; }
	void setIndex(int i) { val = i + 1; }
	void resetIndex() { val = 0; }
public:
	PlayingSoundHandle() { resetIndex(); }
	bool isActive() const { return val > 0; }
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

private:
	enum {
		NUM_CHANNELS = 16
	};

	MutexRef _mutex;

	Channel *_premixChannel;

	uint _outputRate;

	int _globalVolume;

	bool _paused;
	
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
	void setupPremix(AudioStream *stream);



	/**
	 * Start playing the given raw sound data.
	 * Internally, this simply creates an audio input stream wrapping the data
	 * (using the makeLinearInputStream factory function), which is then
	 * passed on to playInputStream.
	 */
	void playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags,
				int id = -1, byte volume = 255, int8 balance = 0,
				uint32 loopStart = 0, uint32 loopEnd = 0);

	/**
	 * Start playing the given audio input stream.
	 */
	void playInputStream(PlayingSoundHandle *handle, AudioStream *input, bool isMusic,
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
	void stopHandle(PlayingSoundHandle handle);



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
	void pauseHandle(PlayingSoundHandle handle, bool paused);



	/**
	 * Check if a sound with the given ID is active.
	 *
	 * @param id the ID of the sound to query
	 * @return true if the sound is active
	 */
	bool isSoundIDActive(int id);

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
	void setChannelVolume(PlayingSoundHandle handle, byte volume);

	/**
	 * Set the channel balance for the given handle.
	 *
	 * @param handle the sound to affect
	 * @param balance the new channel balance:
	 *        (-127 ... 0 ... 127) corresponds to (left ... center ... right)
	 */
	void setChannelBalance(PlayingSoundHandle handle, int8 balance);

	/**
	 * Get approximation of for how long the channel has been playing.
	 */
	uint32 getSoundElapsedTime(PlayingSoundHandle handle);

	/**
	 * Set the global volume.
	 *
	 * @param volume the new global volume, 0-256
	 */
	void setVolume(int volume);

	/**
	 * Query the global volume.
	 *
	 * @return the global music volume, 0-256
	 */
	int getVolume() const { return _globalVolume; }

	/**
	 * Query the system's audio output sample rate. This returns
	 * the same value as OSystem::getOutputSampleRate().
	 *
	 * @return the output sample rate in Hz
	 */
	uint getOutputRate() const { return _outputRate; }

private:
	void insertChannel(PlayingSoundHandle *handle, Channel *chan);

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

extern SoundMixer *g_mixer;

#endif
