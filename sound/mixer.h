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

#ifndef MIXER_H
#define MIXER_H

#include <stdio.h>

#ifdef USE_MAD
#include <mad.h>
#endif

#ifdef USE_VORBIS
#include <vorbis/vorbisfile.h>
#endif

#include "common/scummsys.h"
#include "common/system.h"

typedef uint32 PlayingSoundHandle;

class File;

class SoundMixer {
private:
	class Channel {
	public:
		bool _toBeDestroyed;
		int _id;
		Channel() : _id(-1) {}
		virtual void mix(int16 *data, uint len) = 0;
		void destroy() {
			_toBeDestroyed = true;
		}
		virtual void realDestroy() = 0;
		virtual void append(void *sound, uint32 size);
		virtual bool soundFinished();
	};

	class ChannelRaw : public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		uint32 _pos;
		uint32 _size;
		uint32 _fpSpeed;
		uint32 _fpPos;
		uint32 _realSize, _rate;
		byte _flags;
		void *_loop_ptr;
		uint32 _loop_size;

	public:
		ChannelRaw(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags, int id);

		void mix(int16 *data, uint len);
		void realDestroy();
	};

	class ChannelStream : public Channel {
		SoundMixer *_mixer;
		byte *_ptr;
		byte *_endOfData;
		byte *_endOfBuffer;
		byte *_pos;
		uint32 _fpSpeed;
		uint32 _fpPos;
		uint32 _bufferSize;
		uint32 _rate;
		int32 _timeOut;
		int32 _setTimeOut;
		byte _flags;

	public:
		ChannelStream(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags, int32 timout, int32 buffer_size);

		void append(void *sound, uint32 size);
		void mix(int16 *data, uint len);
		void realDestroy();
	};

#ifdef USE_MAD

	class ChannelMP3 : public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		struct mad_stream _stream;
		struct mad_frame _frame;
		struct mad_synth _synth;
		uint32 _silenceCut;
		uint32 _posInFrame;
		uint32 _position;
		uint32 _size;
		byte _flags;

	public:
		ChannelMP3(SoundMixer *mixer, void *sound, uint size, byte flags);

		void mix(int16 *data, uint len);
		void realDestroy();

	};

	class ChannelMP3CDMusic:public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		struct mad_stream _stream;
		struct mad_frame _frame;
		struct mad_synth _synth;
		uint32 _posInFrame;
		uint32 _size;
		uint32 _bufferSize;
		mad_timer_t _duration;
		File *_file;
		bool _initialized;


	public:
		ChannelMP3CDMusic(SoundMixer *mixer, File *file, mad_timer_t duration);

		void mix(int16 *data, uint len);
		void realDestroy();
		bool soundFinished();
	};

#endif

#ifdef USE_VORBIS
	class ChannelVorbis : public Channel {
		SoundMixer *_mixer;
		OggVorbis_File *_ov_file;
		int _end_pos;
		bool _eof_flag, _is_cd_track;

	public:
		ChannelVorbis(SoundMixer *mixer, OggVorbis_File *ov_file, int duration, bool is_cd_track);

		void mix(int16 *data, uint len);
		void realDestroy();
		bool soundFinished();
	};
#endif

	static void onGenerateSamples(void *s, byte *samples, int len);

public:
	typedef void PremixProc (void *param, int16 *data, uint len);

	OSystem *_syst;
	void *_mutex;

	uint _outputRate;

	int16 *_volumeTable;
	int _musicVolume;

	bool _paused;

	enum {
		NUM_CHANNELS = 16
	};

	void *_premixParam;
	PremixProc *_premixProc;

	Channel *_channels[NUM_CHANNELS];
	PlayingSoundHandle *_handles[NUM_CHANNELS];

	int _beginSlots;

	SoundMixer();
	~SoundMixer();

	int insertAt(PlayingSoundHandle *handle, int index, Channel *chan);
	void append(void *data, uint32 len);
	void unInsert(Channel *chan);
	void beginSlots(int index);

	// start playing a raw sound
	enum {
		// Do *NOT* change any of these flags without looking at the code in mixer.cpp
		FLAG_UNSIGNED = 1 << 0,         // unsigned samples
		FLAG_STEREO = 1 << 1,           // sound is in stereo
		FLAG_16BITS = 1 << 2,           // sound is 16 bits wide
		FLAG_AUTOFREE = 1 << 3,         // sound buffer is freed automagically at the end of playing
		FLAG_REVERSE_STEREO = 1 << 4,   // sound should be reverse stereo
		FLAG_LOOP = 1 << 5              // loop the audio
	};
	int playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, int id = -1);
	int playStream(PlayingSoundHandle *handle, int index, void *sound, uint32 size, uint rate,
									byte flags, int32 timeout = 3, int32 buffer_size = 2000000);
#ifdef USE_MAD
	int playMP3(PlayingSoundHandle *handle, void *sound, uint32 size, byte flags);
	int playMP3CDTrack(PlayingSoundHandle *handle, File *file, mad_timer_t duration);
#endif
#ifdef USE_VORBIS
	int playVorbis(PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track);
#endif

	/* Premix procedure, useful when using fmopl adlib */
	void setupPremix(void * param, PremixProc * proc);

	/* mix */
	void mix(int16 * buf, uint len);

	/* stop all currently playing sounds */
	void stopAll();

	/* stop playing a specific sound */
	void stop(int index);

	/* stop playing a specific sound */
	void stopID(int id);

	/* append to existing sound */
	int append(int index, void * sound, uint32 size, uint rate, byte flags);

	/* is any channel active? */
	bool hasActiveChannel();

	/* bind to the OSystem object => mixer will be
	 * invoked automatically when samples need
	 * to be generated */
	bool bindToSystem(OSystem *syst);

	/* set the volume, 0-256 */
	void setVolume(int volume);
	void setMusicVolume(int volume);

	/* pause - unpause */
	void pause(bool paused);

};

struct MP3OffsetTable {					/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};

#endif
