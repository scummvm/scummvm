/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "common/system.h"


class AudioStream;
class Channel;
class File;

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
	typedef void PremixProc (void *param, int16 *data, uint len);

	enum {
		NUM_CHANNELS = 16
	};

	enum {
		FLAG_UNSIGNED = 1 << 0,         /** unsigned samples (default: signed) */
		FLAG_16BITS = 1 << 1,           /** sound is 16 bits wide (default: 8bit) */
		FLAG_LITTLE_ENDIAN = 1 << 2,    /** sample is little endian (default: big endian) */
		FLAG_STEREO = 1 << 3,           /** sound is in stereo (default: mono) */
		FLAG_REVERSE_STEREO = 1 << 4,   /** reverse the left and right stereo channel */
		FLAG_AUTOFREE = 1 << 5,         /** sound buffer is freed automagically at the end of playing */
		FLAG_LOOP = 1 << 6              /** loop the audio */
	};

private:
	OSystem *_syst;
	OSystem::MutexRef _mutex;

	void *_premixParam;
	PremixProc *_premixProc;

	uint _outputRate;

	int _globalVolume;
	int _musicVolume;

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
	 */
	bool isReady() const { return _mixerReady; };

	/**
	 * Set the premix procedure. This is mainly used for the adlib music, but
	 * is not limited to it. The premix proc is invoked by the mixer whenever
	 * it needs to generate any data, before any other mixing takes place. The
	 * premixer than has a chanve to fill the mix buffer with data (usually
	 * music samples). It should generate the specified number of 16bit stereo
	 * samples (i.e. len * 4 bytes). The endianess of these samples shall be
	 * the native endianess.
	 */
	void setupPremix(PremixProc *proc, void *param);

	// start playing a raw sound
	void playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags,
				int id = -1, byte volume = 255, int8 balance = 0, uint32 loopStart = 0, uint32 loopEnd = 0);
#ifdef USE_MAD
	void playMP3(PlayingSoundHandle *handle, File *file, uint32 size, byte volume = 255, int8 balance = 0, int id = -1);
#endif
#ifdef USE_VORBIS
	void playVorbis(PlayingSoundHandle *handle, File *file, uint32 size, byte volume = 255, int8 balance = 0, int id = -1);
#endif
#ifdef USE_FLAC
	void playFlac(PlayingSoundHandle *handle, File *file, uint32 size, byte volume = 255, int8 balance = 0, int id = -1);
#endif

	void playInputStream(PlayingSoundHandle *handle, AudioStream *input, bool isMusic, byte volume = 255, int8 balance = 0, int id = -1, bool autofreeStream = true);


	/** Start a new stream. */
	void newStream(PlayingSoundHandle *handle, uint rate, byte flags, uint32 buffer_size, byte volume = 255, int8 balance = 0);

	/** Append to an existing stream. */
	void appendStream(PlayingSoundHandle handle, void *sound, uint32 size);

	/**
	 * Mark a stream as finished.
	 * Where stopHandle() would stop the sound immediately, when using this
	 * method, the stream will first finish playing all its data before it
	 * finally stops.
	 */
	void endStream(PlayingSoundHandle handle);

	/** stop all currently playing sounds */
	void stopAll();

	/** stop playing the sound with given ID  */
	void stopID(int id);

	/** stop playing the channel for the given handle */
	void stopHandle(PlayingSoundHandle handle);

	/** pause/unpause all channels */
	void pauseAll(bool paused);

	/** pause/unpause the sound with the given ID */
	void pauseID(int id, bool paused);

	/** pause/unpause the channel for the given handle */
	void pauseHandle(PlayingSoundHandle handle, bool paused);

	/** set the channel volume for the given handle (0 - 255) */
	void setChannelVolume(PlayingSoundHandle handle, byte volume);

	/** set the channel balance for the given handle (-127 ... 0 ... 127) (left ... center ... right)*/
	void setChannelBalance(PlayingSoundHandle handle, int8 balance);

	/** get approximation of for how long the channel has been playing */
	uint32 getChannelElapsedTime(PlayingSoundHandle handle);

	/** Check whether any SFX channel is active.*/
	bool hasActiveSFXChannel();

	/** set the global volume, 0-256 */
	void setVolume(int volume);

	/** query the global volume, 0-256 */
	int getVolume() const { return _globalVolume; }

	/** set the music volume, 0-256 */
	void setMusicVolume(int volume);

	/** query the music volume, 0-256 */
	int getMusicVolume() const { return _musicVolume; }

	/** query the output rate in kHz */
	uint getOutputRate() const { return _outputRate; }

private:
	void insertChannel(PlayingSoundHandle *handle, Channel *chan);

	/** main mixer method */
	void mix(int16 * buf, uint len);

	static void mixCallback(void *s, byte *samples, int len);
};

#endif
