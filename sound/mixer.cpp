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

#include "stdafx.h"
#include "scumm.h"

void SoundMixer::uninsert(Channel *chan) {

	for(int i=0; i!=NUM_CHANNELS; i++) {	
		if (_channels[i] == chan) {
			if (_handles[i]) {
				*_handles[i] = 0;
				_handles[i] = NULL;
			}
			_channels[i] = NULL;
			return;
		}
	}
	error("SoundMixer::channel_deleted chan not found");
}

int SoundMixer::insert(PlayingSoundHandle *handle, Channel *chan) {
	for(int i=0; i!=NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			_channels[i] = chan;
			_handles[i] = handle;
			if (handle)
				*handle = i + 1;
			return i;
		}
	}
	
	warning("SoundMixer::insert out of mixer slots");
	chan->real_destroy();

	return -1;
}


int SoundMixer::play_raw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags) {
	return insert(handle, new Channel_RAW(this, sound, size, rate, flags));
}

#ifdef COMPRESSED_SOUND_FILE
int SoundMixer::play_mp3(PlayingSoundHandle *handle, void *sound, uint32 size, byte flags) {
	return insert(handle, new Channel_MP3(this, sound, size, flags));
}
int SoundMixer::play_mp3_cdtrack(PlayingSoundHandle *handle, int index, FILE* file, mad_timer_t duration) {
	/* Stop the previously playing CD track (if any) */
	stop(index);
	return insert(handle, new Channel_MP3_CDMUSIC(this, file, duration));
}
#endif

void SoundMixer::mix(int16 *buf, uint len) {
	if (_premix_proc) {
		_premix_proc(_premix_param, buf, len);
	} else {
		/* no premixer available, zero the buf out */
		memset(buf, 0, len * sizeof(int16));
	}

	/* now mix all channels */
	for(int i=0; i!=NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->mix(buf, len);
}

void SoundMixer::on_generate_samples(void *s, byte *samples, int len) {
	((SoundMixer*)s)->mix((int16*)samples, len>>1);
}

bool SoundMixer::bind_to_system(OSystem *syst) {	
	_volume_table = (int16*)calloc(256*sizeof(int16),1);
		
	uint rate = (uint)syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	_output_rate = rate;
	
	if (rate == 0)
		error("OSystem returned invalid sample rate");
	
	return syst->set_sound_proc(this, on_generate_samples, OSystem::SOUND_16BIT);
}

void SoundMixer::stop_all() {
	for(int i=0; i!=NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->destroy();
}

void SoundMixer::stop(PlayingSoundHandle psh) {
	if (psh && _channels[psh-1])
		_channels[psh-1]->destroy();
}

void SoundMixer::stop(int index) {
	if (_channels[index])
		_channels[index]->destroy();
}

bool SoundMixer::has_active_channel() {
	for(int i=0; i!=NUM_CHANNELS; i++)
		if (_channels[i])
			return true;
	return false;
}

void SoundMixer::setup_premix(void *param, PremixProc *proc) {
	_premix_param = param;
	_premix_proc = proc;
}

void SoundMixer::set_volume(int volume) {
	for(int i=0; i!=256; i++)
		_volume_table[i] =((int8)i) * volume;
}


/* RAW mixer */
SoundMixer::Channel_RAW::Channel_RAW(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags) {
	_mixer = mixer;
	_flags = flags;
	_ptr = sound;
	_pos = 0;
	_fp_pos = 0;
	_fp_speed = (1 << 16) * rate / mixer->_output_rate;
	_to_be_destroyed = false;

	/* adjust the magnitute to prevent division error */
	while (size & 0xFFFF0000)
		size >>= 1, rate >>= 1;

	_size = size * mixer->_output_rate / rate;
}

void SoundMixer::Channel_RAW::mix(int16 *data, uint len) {
	byte *s, *s_org = NULL;
	uint32 fp_pos;

	if (_to_be_destroyed) {
		real_destroy();
		return;
	}

	if (len > _size)
		len = _size;
	_size -= len;

	/* 
	 * simple support for fread() reading of samples
	 */
	if (_flags & FLAG_FILE) {
		/* determine how many samples to read from the file */
		uint num = len * _fp_speed >> 16;

		s_org = (byte*)malloc(num);
		if (s_org == NULL)
			error("Channel_RAW::mix out of memory");
		
		uint num_read = fread(s_org, 1, num, (FILE*)_ptr);
		if (num - num_read != 0)
			memset(s_org + num_read, 0x80, num - num_read);
				
		s = s_org;
		fp_pos = 0;
	} else {
		s = (byte*)_ptr + _pos;
		fp_pos = _fp_pos;
	}

	const uint32 fp_speed = _fp_speed;
	const int16 *vol_tab = _mixer->_volume_table;

	if (_flags & FLAG_UNSIGNED) {
		do {
			fp_pos += fp_speed;
			*data++ += vol_tab[*s ^ 0x80];
			s += fp_pos >> 16;
			fp_pos &= 0x0000FFFF;
		} while (--len);
	} else {
		do {
			fp_pos += fp_speed;
			*data++ += vol_tab[*s];
			s += fp_pos >> 16;
			fp_pos &= 0x0000FFFF;
		} while (--len);
	}

	_pos = s - (byte*) _ptr;
	_fp_pos = fp_pos;

	if (_flags & FLAG_FILE) {
		free(s_org);
	}

	if (_size < 1)
		real_destroy();

}

void SoundMixer::Channel_RAW::real_destroy() {
	if (_flags & FLAG_AUTOFREE)
		free(_ptr);
	_mixer->uninsert(this);
	delete this;
}


/* MP3 mixer goes here */
#ifdef COMPRESSED_SOUND_FILE
SoundMixer::Channel_MP3::Channel_MP3(SoundMixer *mixer, void *sound, uint size, byte flags) {
	_mixer = mixer;
	_flags = flags;
	_pos_in_frame = 0xFFFFFFFF;
	_position = 0;
	_size = size;
	_ptr = sound;
	_to_be_destroyed = false;
	
	mad_stream_init(&_stream);
#ifdef _WIN32_WCE
	// 11 kHz on WinCE
	mad_stream_options(&_stream, MAD_OPTION_HALFSAMPLERATE);
#endif
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
	/* This variable is the number of samples to cut at the start of the MP3
	   file. This is needed to have lip-sync as the MP3 file have some miliseconds
	   of blank at the start (as, I suppose, the MP3 compression algorithm need to
	   have some silence at the start to really be efficient and to not distort
	   too much the start of the sample).

	   This value was found by experimenting out. If you recompress differently your
	   .SO3 file, you may have to change this value.

	   When using Lame, it seems that the sound starts to have some volume about 50 ms
	   from the start of the sound => we skip about 1024 samples.
	 */
	_silence_cut = 1024;
}

static inline int scale_sample(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize and scale to not saturate when mixing a lot of channels */
	return sample >> (MAD_F_FRACBITS + 2 - 16);
}

void SoundMixer::Channel_MP3::mix(int16 *data, uint len) {
	mad_fixed_t const *ch;

	if (_to_be_destroyed) {
		real_destroy();
		return;
	}

	while (1) {
		ch = _synth.pcm.samples[0] + _pos_in_frame;
		while ((_pos_in_frame < _synth.pcm.length) && (len > 0)) {
			if (_silence_cut > 0) {
				_silence_cut--;
			} else {
				*data++ += scale_sample(*ch++);
				len--;
			}
			_pos_in_frame++;
		}
		if (len == 0)
			return;
		
		if (_position >= _size) {
			real_destroy();
			return;
		}

		mad_stream_buffer(&_stream, ((unsigned char *)_ptr) + _position, _size + MAD_BUFFER_GUARD - _position);

		if (mad_frame_decode(&_frame, &_stream) == -1) {
			/* End of audio... */
			if (_stream.error == MAD_ERROR_BUFLEN) {
				real_destroy();
				return;
			} else if (!MAD_RECOVERABLE(_stream.error)) {
				error("MAD frame decode error !");
			}
		}
		mad_synth_frame(&_synth, &_frame);
		_pos_in_frame = 0;
		_position = (unsigned char *)_stream.next_frame - (unsigned char *)_ptr;
	}
}

void SoundMixer::Channel_MP3::real_destroy() {
	if (_flags & FLAG_AUTOFREE)
		free(_ptr);
	_mixer->uninsert(this);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	delete this;
}

/* MP3 CD music */
#define MP3CD_BUFFERING_SIZE 131072

SoundMixer::Channel_MP3_CDMUSIC::Channel_MP3_CDMUSIC(SoundMixer *mixer, FILE* file, mad_timer_t duration) {
	_mixer = mixer;
	_file = file;
	_duration = duration;
	_initialized = false;
	_buffer_size = MP3CD_BUFFERING_SIZE;
	_ptr = malloc(MP3CD_BUFFERING_SIZE);
	_to_be_destroyed = false;

	mad_stream_init(&_stream);
#ifdef _WIN32_WCE
	// 11 kHz on WinCE
	mad_stream_options(&_stream, MAD_OPTION_HALFSAMPLERATE);
#endif
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
}

void SoundMixer::Channel_MP3_CDMUSIC::mix(int16 *data, uint len) {
	mad_fixed_t const *ch;
	mad_timer_t frame_duration;

	if (_to_be_destroyed) {
		real_destroy();
		return;
	}

	if (!_initialized) {
		int skip_loop;
		// just skipped
		memset(_ptr, 0,_buffer_size);
		_size = fread(_ptr, 1, _buffer_size, _file);
		if (!_size) {
				real_destroy();
				return;
		}
		// Resync
		mad_stream_buffer(&_stream,(unsigned char *)_ptr,_size);
		skip_loop = 2;
		while (skip_loop != 0) {
			if (mad_frame_decode(&_frame,&_stream) == 0) {
				/* Do not decrease duration - see if it's a problem */
				skip_loop--;
				if (skip_loop == 0) {
					mad_synth_frame(&_synth, &_frame);
				}
			} else {
				if (!MAD_RECOVERABLE(_stream.error)) {
					debug(1, "Unrecoverable error while skipping !");
					real_destroy();
					return;
				}
			}
		}
		// We are supposed to be in synch
		mad_frame_mute(&_frame);
		mad_synth_mute(&_synth);
		// Resume decoding
		if (mad_frame_decode(&_frame,&_stream) == 0) {
			_pos_in_frame = 0;
			_initialized = true;
		} else {
			debug(1, "Cannot resume decoding");
			real_destroy();
			return;
		}
	}

	while(1) {
		// Get samples, play samples ... 
		ch = _synth.pcm.samples[0] + _pos_in_frame;
		while ((_pos_in_frame < _synth.pcm.length) && (len > 0)) {
			*data++ += scale_sample(*ch++);
			len--;
			_pos_in_frame++;
		}
		if (len == 0) {
			return;
		}
		// See if we have finished
		// May be incorrect to check the size at the end of a frame but I suppose
		// they are short enough :)   
		frame_duration = _frame.header.duration;
		mad_timer_negate(&frame_duration);
		mad_timer_add(&_duration, frame_duration);
		if (mad_timer_compare(_duration, mad_timer_zero) < 0) {					
			real_destroy();
			return;
		}		
		if (mad_frame_decode(&_frame, &_stream) == -1) {
			if (_stream.error == MAD_ERROR_BUFLEN) {
				int not_decoded;

				if (!_stream.next_frame) {
					memset(_ptr, 0, _buffer_size + MAD_BUFFER_GUARD);
					_size =	fread(_ptr, 1, _buffer_size, _file);
					not_decoded = 0;
				} else {
					not_decoded = _stream.bufend - _stream.next_frame;
					memcpy(_ptr, _stream.next_frame, not_decoded);
					_size =	fread((unsigned char *)_ptr + not_decoded, 1, _buffer_size - not_decoded, _file);
				}
				_stream.error = MAD_ERROR_NONE;
				// Restream
				mad_stream_buffer(&_stream, (unsigned char *)_ptr, _size + not_decoded);
				if (mad_frame_decode(&_frame, &_stream) == -1) {
					debug(1, "Error decoding after restream %d !", _stream.error);
				}
			} else if (!MAD_RECOVERABLE(_stream.error)) {
				error("MAD frame decode error in MP3 CDMUSIC !");
			}
		}
		mad_synth_frame(&_synth, &_frame);
		_pos_in_frame = 0;
	}
}

void SoundMixer::Channel_MP3_CDMUSIC::real_destroy() {
	free(_ptr);
	_mixer->uninsert(this);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	delete this;
}


#endif


