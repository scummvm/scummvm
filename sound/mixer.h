/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef _mixer_h_included
#define _mixer_h_included

#ifdef COMPRESSED_SOUND_FILE
#include <mad.h>
#endif

typedef uint32 PlayingSoundHandle;
class SoundMixer {
private:	
	class Channel {
	public:
		bool _to_be_destroyed;
		virtual void mix(int16 *data, uint len) = 0;
		void destroy() { _to_be_destroyed = true; }
		virtual void real_destroy() = 0;
		virtual void append(void *sound, uint32 size) = 0;
#ifdef COMPRESSED_SOUND_FILE
		virtual bool sound_finished();
#endif
	};

	class Channel_RAW : public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		uint32 _pos;
		uint32 _size;
		uint32 _fp_speed;
		uint32 _fp_pos;
		uint32 _realsize, _rate;
		byte _flags;
		

	public:
		void append(void *sound, uint32 size);		
		void mix(int16 *data, uint len);
		Channel_RAW(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags);
		void real_destroy();
	};

#ifdef COMPRESSED_SOUND_FILE

	class Channel_MP3 : public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		struct mad_stream _stream;
		struct mad_frame _frame;
		struct mad_synth _synth;
		uint32 _silence_cut;
		uint32 _pos_in_frame;
		uint32 _position;
		uint32 _size;
		byte _flags;

	public:
		virtual void append(void *sound, uint32 size) {}	// FIXME, FAKE
		void mix(int16 *data, uint len);
		Channel_MP3(SoundMixer *mixer, void *sound, uint size, byte flags);
		void real_destroy();

	};

	class Channel_MP3_CDMUSIC : public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		struct mad_stream _stream;
		struct mad_frame _frame;
		struct mad_synth _synth;
		uint32 _pos_in_frame;
		uint32 _size;
		uint32 _buffer_size;
		mad_timer_t _duration;
		FILE   *_file;
		bool _initialized;
	public:
		virtual void append(void *sound, uint32 size) {}	// FIXME, FAKE
		void mix(int16 *data, uint len);
		Channel_MP3_CDMUSIC(SoundMixer *mixer, FILE* file, mad_timer_t duration);
		void real_destroy();		
		bool sound_finished();
	};

#endif

	static void on_generate_samples(void *s, byte *samples, int len);

public:
	typedef void PremixProc(void *param, int16 *data, uint len);

	uint _output_rate;

	int16 *_volume_table;

	bool _paused;

	enum {
		NUM_CHANNELS = 16,
	};

	void *_premix_param;
	PremixProc *_premix_proc;

	Channel *_channels[NUM_CHANNELS];
	PlayingSoundHandle *_handles[NUM_CHANNELS];
	
	int insert(PlayingSoundHandle *handle, Channel *chan);
	void append(void *data, uint32 len);
	void uninsert(Channel *chan);

	/* start playing a raw sound */
	enum {
		FLAG_AUTOFREE = 1,
		FLAG_UNSIGNED = 2, /* unsigned samples */
		FLAG_FILE = 4,	   /* sound is a FILE * that's read from */
	};
	int play_raw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags);
#ifdef COMPRESSED_SOUND_FILE
	int play_mp3(PlayingSoundHandle *handle, void *sound, uint32 size, byte flags);
	int play_mp3_cdtrack(PlayingSoundHandle *handle, FILE* file, mad_timer_t duration);
#endif

	/* Premix procedure, useful when using fmopl adlib */
	void setup_premix(void *param, PremixProc *proc);

	/* mix */
	void mix(int16 *buf, uint len);

	/* stop all currently playing sounds */
	void stop_all();

	/* stop playing a specific sound */
	void stop(PlayingSoundHandle psh);
	void stop(int index);

	/* append to existing sound */
	int append(int index, void *sound, uint32 size, uint rate, byte flags);

	/* is any channel active? */
	bool has_active_channel();

	/* bind to the OSystem object => mixer will be
	 * invoked automatically when samples need
	 * to be generated */
	bool bind_to_system(OSystem *syst);

	/* set the volume, 0-256 */
	void set_volume(int volume);

	/* pause - unpause */
	void pause(bool paused);

};

struct MP3OffsetTable {	/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};

struct BundleAudioTable { /* Dig/CMI .bun audio */
 char filename[13];
 int size;
 int offset;
};
#endif /* _mixer_h_included */
