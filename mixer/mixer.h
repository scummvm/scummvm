// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#include "../stdafx.h"
#include "../bits.h"
#include <SDL.h>

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

typedef void (*SoundProc)(void *param, byte *buf, int len);

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
	MutexRef _mutex;

	void *_premixParam;
	PremixProc *_premixProc;

	uint _outputRate;
	int _globalVolume;
	bool _paused;
	bool _mixerReady;

	Channel *_channels[NUM_CHANNELS];

public:
	SoundMixer();
	~SoundMixer();

	bool isReady() const { return _mixerReady; };

	void setupPremix(PremixProc *proc, void *param);

	// start playing a raw sound
	void playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags,
				int id = -1, byte volume = 255, int8 balance = 0, uint32 loopStart = 0, uint32 loopEnd = 0);

	void playInputStream(PlayingSoundHandle *handle, AudioStream *input, bool isMusic, byte volume = 255, int8 balance = 0, int id = -1, bool autofreeStream = true);

	/** Start a new stream. */
	void newStream(PlayingSoundHandle *handle, uint rate, byte flags, uint32 buffer_size, byte volume = 255, int8 balance = 0);

	/** Append to an existing stream. */
	void appendStream(PlayingSoundHandle handle, void *sound, uint32 size);

	/** Mark a stream as finished - it will play all its remaining data, then stop. */
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

	/** set the global volume, 0-256 */
	void setVolume(int volume);

	/** query the global volume, 0-256 */
	int getVolume() const { return _globalVolume; }

	/** query the output rate in kHz */
	uint getOutputRate() const { return _outputRate; }

private:
	bool setSoundProc(SoundProc proc, void *param);

	void insertChannel(PlayingSoundHandle *handle, Channel *chan);

	/** main mixer method */
	void mix(int16 * buf, uint len);

	static void mixCallback(void *s, byte *samples, int len);
};

#endif
