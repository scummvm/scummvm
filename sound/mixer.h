/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#ifdef USE_MAD
#include <mad.h>
#endif
#ifdef USE_VORBIS
#include <vorbis/vorbisfile.h>
#endif
#include "common/scummsys.h"
#include "common/system.h"


typedef int32 PlayingSoundHandle;

class Channel;
class File;

class SoundMixer {
	friend class Channel;
public:
	typedef void PremixProc (void *param, int16 *data, uint len);

	enum {
		NUM_CHANNELS = 16
	};

	enum {
		// Do *NOT* change any of these flags without looking at the code in mixer.cpp
		FLAG_UNSIGNED = 1 << 0,         // unsigned samples (default: signed)
		FLAG_STEREO = 1 << 1,           // sound is in stereo (default: mono)
		FLAG_16BITS = 1 << 2,           // sound is 16 bits wide (default: 8bit)
		FLAG_AUTOFREE = 1 << 3,         // sound buffer is freed automagically at the end of playing
		FLAG_REVERSE_STEREO = 1 << 4,   // reverse the left and right stereo channel
		FLAG_LOOP = 1 << 5              // loop the audio
	};

private:
	OSystem *_syst;
	OSystem::MutexRef _mutex;

	void *_premixParam;
	PremixProc *_premixProc;

	uint _outputRate;

	int _globalVolume;
	int _musicVolume;

	bool _paused, _channelsPaused;

	Channel *_channels[NUM_CHANNELS];

public:
	SoundMixer();
	~SoundMixer();

	// start playing a raw sound
	int playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags,
				int id = -1, uint32 loopStart = 0, uint32 loopEnd = 0);
#ifdef USE_MAD
	int playMP3(PlayingSoundHandle *handle, File *file, uint32 size);
	int playMP3CDTrack(PlayingSoundHandle *handle, File *file, mad_timer_t duration);
#endif
#ifdef USE_VORBIS
	int playVorbis(PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track);
#endif

	/** Premix procedure, useful when using fmopl adlib */
	void setupPremix(void * param, PremixProc * proc);

	/** stop all currently playing sounds */
	void stopAll();

	/** stop playing the given channel */
	void stop(int channel);

	/** stop playing the sound with given ID  */
	void stopID(int id);

	/** stop playing the channel for the given handle */
	void stopHandle(PlayingSoundHandle handle);

	bool isChannelActive(PlayingSoundHandle handle);

	/** Start a new stream. */
	int newStream(void *sound, uint32 size, uint rate, byte flags, uint32 buffer_size);

	/** Append to an existing stream. */
	void appendStream(int index, void *sound, uint32 size);

	/** Mark a stream as finished - it will play all its remaining data, then stop. */
	void endStream(int index);

	/** Check whether any SFX channel is active.*/
	bool hasActiveSFXChannel();
	
	/** bind to the OSystem object => mixer will be
	 * invoked automatically when samples need
	 * to be generated */
	bool bindToSystem(OSystem *syst);

	/** pause - unpause */
	void pause(bool paused);

	/** pause - unpause channels, keep adlib music running */
	void pauseChannels(bool paused);

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
	int insertChannel(PlayingSoundHandle *handle, Channel *chan);

	/** mix */
	void mix(int16 * buf, uint len);

	static void mixCallback(void *s, byte *samples, int len);
};

#endif
