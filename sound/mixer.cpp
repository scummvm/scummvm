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

void SoundMixer::uninsert(Channel * chan)
{

	for (int i = 0; i != NUM_CHANNELS; i++) {
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

int SoundMixer::append(int index, void *sound, uint32 size, uint rate, byte flags)
{
	_syst->lock_mutex(_mutex);

	Channel *chan = _channels[index];
	if (!chan) {
		warning("Trying to stream to an unexistant streamer ");
		play_stream(NULL, index, sound, size, rate, flags);
		chan = _channels[index];
	} else {
		chan->append(sound, size);
	}

	_syst->unlock_mutex(_mutex);
	return 1;
}

int SoundMixer::insert_at(PlayingSoundHandle *handle, int index, Channel * chan)
{
	if (_channels[index] != NULL) {
		error("Trying to put a mixer where it cannot go ");
	}
	_channels[index] = chan;
	_handles[index] = handle;
	if (handle)
		*handle = index + 1;
	return index;
}

int SoundMixer::insert(PlayingSoundHandle *handle, Channel * chan)
{
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			return insert_at(handle, i, chan);
		}
	}

	warning("SoundMixer::insert out of mixer slots");
	chan->real_destroy();

	return -1;
}


int SoundMixer::play_raw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate,
												 byte flags)
{
	return insert(handle, new Channel_RAW(this, sound, size, rate, flags));
}

int SoundMixer::play_stream(PlayingSoundHandle *handle, int idx, void *sound, uint32 size,
														uint rate, byte flags)
{
	return insert_at(handle, idx, new Channel_STREAM(this, sound, size, rate, flags));
}

#ifdef COMPRESSED_SOUND_FILE
int SoundMixer::play_mp3(PlayingSoundHandle *handle, void *sound, uint32 size, byte flags)
{
	return insert(handle, new Channel_MP3(this, sound, size, flags));
}
int SoundMixer::play_mp3_cdtrack(PlayingSoundHandle *handle, FILE * file, mad_timer_t duration)
{
	/* Stop the previously playing CD track (if any) */
	return insert(handle, new Channel_MP3_CDMUSIC(this, file, duration));
}
#endif

void SoundMixer::mix(int16 *buf, uint len)
{
	if (_paused) {
		memset(buf, 0, 2 * len * sizeof(int16));
		return;
	}
	
	if (_premix_proc) {
		int i;
		_premix_proc(_premix_param, buf, len);
		for (i = (len - 1); i >= 0; i--) {
			buf[2 * i] = buf[2 * i + 1] = buf[i];
		}
	} else {
		// no premixer available, zero the buf out
		memset(buf, 0, 2 * len * sizeof(int16));
	}

	_syst->lock_mutex(_mutex);
	/* now mix all channels */
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->mix(buf, len);
	_syst->unlock_mutex(_mutex);
}

void SoundMixer::on_generate_samples(void *s, byte *samples, int len)
{
	((SoundMixer *)s)->mix((int16 *)samples, len >> 2);
}

bool SoundMixer::bind_to_system(OSystem *syst)
{
	_volume_table = (int16 *)calloc(256 * sizeof(int16), 1);

	uint rate = (uint) syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	_output_rate = rate;

	_syst = syst;
	_mutex = _syst->create_mutex();

	if (rate == 0)
		error("OSystem returned invalid sample rate");

	return syst->set_sound_proc(this, on_generate_samples, OSystem::SOUND_16BIT);
}

void SoundMixer::stop_all()
{
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->destroy();
}

void SoundMixer::stop(PlayingSoundHandle psh)
{
	if (psh && _channels[psh - 1])
		_channels[psh - 1]->destroy();
}

void SoundMixer::stop(int index)
{
	if (_channels[index])
		_channels[index]->destroy();
}

void SoundMixer::pause(bool paused)
{
	_paused = paused;
}

bool SoundMixer::has_active_channel()
{
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			return true;
	return false;
}

void SoundMixer::setup_premix(void *param, PremixProc *proc)
{
	_premix_param = param;
	_premix_proc = proc;
}

void SoundMixer::set_volume(int volume)
{
	for (int i = 0; i != 256; i++)
		_volume_table[i] = ((int8)i) * volume;
}

void SoundMixer::set_music_volume(int volume)
{
	_music_volume = volume;
}

#ifdef COMPRESSED_SOUND_FILE
bool SoundMixer::Channel::sound_finished()
{
	warning("sound_finished should never be called on a non-MP3 mixer ");
	return false;
}
#endif

void SoundMixer::Channel::append(void *sound, uint32 size)
{
	error("append method should never be called on something else than a _STREAM mixer ");
}

/* RAW mixer */
SoundMixer::Channel_RAW::Channel_RAW(SoundMixer *mixer, void *sound, uint32 size, uint rate,
																		 byte flags)
{
	_mixer = mixer;
	_flags = flags;
	_ptr = sound;
	_pos = 0;
	_fp_pos = 0;
	_fp_speed = (1 << 16) * rate / mixer->_output_rate;
	_to_be_destroyed = false;
	_realsize = size;

	/* adjust the magnitute to prevent division error */
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;

	_rate = rate;
	_size = size * mixer->_output_rate / rate;
	if (_flags & FLAG_16BITS)
		_size = _size >> 1;
	if (_flags & FLAG_STEREO)
		_size = _size >> 1;
}


/*
 * Class that performs cubic interpolation on integer data.
 * It is expected that the data is equidistant, i.e. all have the same
 * horizontal distance. This is obviously the case for sampled audio.
 */
class CubicInterpolator {
protected:
	int x0, x1, x2, x3;
	int a, b, c, d;
	
public:
	CubicInterpolator(int a, int b, int c) : x0(2*a-b), x1(a), x2(b), x3(c)
	{
		// We use a simple linear interpolation for x0
		updateCoefficients();
	}
	
	inline void feedData()
	{
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = 2*x2-x1;	// Simple linear interpolation
		updateCoefficients();
	}

	inline void feedData(int xNew)
	{
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = xNew;
		updateCoefficients();
	}
	
	/* t must be a 16.16 fixed point number between 0 and 1 */
	inline int interpolate(uint32 fp_pos)
	{
		int result = 0;
		int t = fp_pos >> 8;
		result = (a*t + b) >> 8;
		result = (result * t + c) >> 8;
		result = (result * t + d) >> 8;
		result = (result/3 + 1) >> 1;
		
		return result;
	}
		
protected:
	inline void updateCoefficients()
	{
		a = ((-x0*2)+(x1*5)-(x2*4)+x3);
		b = ((x0+x2-(2*x1))*6) << 8;
		c = ((-4*x0)+x1+(x2*4)-x3) << 8;
		d = (x1*6) << 8;
	}
};

static inline int clamped_add_16(int a, int b)
{
	int val = a + b;
	if (val > 0xFFFF)
		return 0xFFFF;
	else
		return val;
}

static int16 *mix_signed_mono_8(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																int fp_speed, const int16 *vol_tab, byte *s_end)
{
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	
	int inc = 1, result;
	CubicInterpolator	interp(vol_tab[*s], vol_tab[*(s+1)], vol_tab[*(s+2)]);

	do {
		do {
			result = interp.interpolate(fp_pos);
			
			*data = clamped_add_16(*data, result);
			*data++;
			*data = clamped_add_16(*data, result);
			*data++;
	
			fp_pos += fp_speed;
			inc = fp_pos >> 16;
			s += inc;
			len--;
			fp_pos &= 0x0000FFFF;
		} while (!inc && len && (s < s_end));
		
		if (s+2 < s_end)
			interp.feedData(vol_tab[*(s+2)]);
		else
			interp.feedData();

	} while (len && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_unsigned_mono_8(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																	int fp_speed, const int16 *vol_tab, byte *s_end)
{
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	
	int inc = 1, result;
	CubicInterpolator	interp(vol_tab[*s ^ 0x80], vol_tab[*(s+1) ^ 0x80], vol_tab[*(s+2) ^ 0x80]);

	do {
		do {
			result = interp.interpolate(fp_pos);
	
			*data = clamped_add_16(*data, result);
			*data++;
			*data = clamped_add_16(*data, result);
			*data++;
	
			fp_pos += fp_speed;
			inc = fp_pos >> 16;
			s += inc;
			len--;
			fp_pos &= 0x0000FFFF;
		} while (!inc && len && (s < s_end));

		if (s+2 < s_end)
			interp.feedData(vol_tab[*(s+2) ^ 0x80]);
		else
			interp.feedData();

	} while (len && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_signed_stereo_8(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																	int fp_speed, const int16 *vol_tab, byte *s_end)
{
	warning("Mixing stereo signed 8 bit is not supported yet ");

	return data;
}
static int16 *mix_unsigned_stereo_8(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																		int fp_speed, const int16 *vol_tab, byte *s_end)
{
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	
	int inc = 1;
	CubicInterpolator	left(vol_tab[*s ^ 0x80], vol_tab[*(s+2) ^ 0x80], vol_tab[*(s+4) ^ 0x80]);
	CubicInterpolator	right(vol_tab[*(s+1) ^ 0x80], vol_tab[*(s+3) ^ 0x80], vol_tab[*(s+5) ^ 0x80]);

	do {
		do {
			*data = clamped_add_16(*data, left.interpolate(fp_pos));
			*data++;
			*data = clamped_add_16(*data, right.interpolate(fp_pos));
			*data++;

			fp_pos += fp_speed;
			inc = (fp_pos >> 16) << 1;
			s += inc;
			len--;
			fp_pos &= 0x0000FFFF;
		} while (!inc && len && (s < s_end));

		if (s+5 < s_end) {
			left.feedData(vol_tab[*(s+4) ^ 0x80]);
			right.feedData(vol_tab[*(s+5) ^ 0x80]);
		} else {
			left.feedData();
			right.feedData();
		}

	} while (len && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_signed_mono_16(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																 int fp_speed, const int16 *vol_tab, byte *s_end)
{
	printf("mix_signed_mono_16\n");
	uint32 fp_pos = *fp_pos_ptr;
	unsigned char volume = ((int)vol_tab[1]) * 32 / 255;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	do {
		int16 sample = (((int16)(*s << 8) | *(s + 1)) * volume) / 32;
		fp_pos += fp_speed;

		*data = clamped_add_16(*data, sample);
		*data++;
		*data = clamped_add_16(*data, sample);
		*data++;

		s += (fp_pos >> 16) << 1;
		fp_pos &= 0x0000FFFF;
	} while ((--len) && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_unsigned_mono_16(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																	 int fp_speed, const int16 *vol_tab, byte *s_end)
{
	warning("Mixing mono unsigned 16 bit is not supported yet ");

	return data;
}
static int16 *mix_signed_stereo_16(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																	 int fp_speed, const int16 *vol_tab, byte *s_end)
{
	printf("mix_signed_stereo_16\n");
	uint32 fp_pos = *fp_pos_ptr;
	unsigned char volume = ((int)vol_tab[1]) * 32 / 255;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	do {
		fp_pos += fp_speed;

		*data = clamped_add_16(*data, (((int16)(*(s) << 8) | *(s + 1)) * volume) / 32);
		*data++;
		*data = clamped_add_16(*data, (((int16)(*(s + 2) << 8) | *(s + 3)) * volume) / 32);
		*data++;

		s += (fp_pos >> 16) << 2;
		fp_pos &= 0x0000FFFF;
	} while ((--len) && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_unsigned_stereo_16(int16 *data, uint * len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
																		 int fp_speed, const int16 *vol_tab, byte *s_end)
{
	warning("Mixing stereo unsigned 16 bit is not supported yet ");

	return data;
}

static int16 *(*mixer_helper_table[16]) (int16 *data, uint * len_ptr, byte **s_ptr,
																				 uint32 *fp_pos_ptr, int fp_speed, const int16 *vol_tab,
																				 byte *s_end) = {
mix_signed_mono_8, mix_unsigned_mono_8, mix_signed_stereo_8, mix_unsigned_stereo_8,
		mix_signed_mono_16, mix_unsigned_mono_16, mix_signed_stereo_16, mix_unsigned_stereo_16};

void SoundMixer::Channel_RAW::mix(int16 *data, uint len)
{
	byte *s, *s_org = NULL;
	uint32 fp_pos;
	byte *end;

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

		s_org = (byte *)malloc(num);
		if (s_org == NULL)
			error("Channel_RAW::mix out of memory");

		uint num_read = fread(s_org, 1, num, (FILE *) _ptr);
		if (num - num_read != 0)
			memset(s_org + num_read, 0x80, num - num_read);

		s = s_org;
		fp_pos = 0;
		end = s_org + num;
	} else {
		s = (byte *)_ptr + _pos;
		fp_pos = _fp_pos;
		end = (byte *)_ptr + _realsize;
	}

	const uint32 fp_speed = _fp_speed;
	const int16 *vol_tab = _mixer->_volume_table;

	mixer_helper_table[_flags & 0x07] (data, &len, &s, &fp_pos, fp_speed, vol_tab, end);

	_pos = s - (byte *)_ptr;
	_fp_pos = fp_pos;

	if (_flags & FLAG_FILE) {
		free(s_org);
	}

	if (_size < 1)
		real_destroy();

}

void SoundMixer::Channel_RAW::real_destroy()
{
	if (_flags & FLAG_AUTOFREE)
		free(_ptr);
	_mixer->uninsert(this);
	delete this;
}

/* STREAM mixer */
SoundMixer::Channel_STREAM::Channel_STREAM(SoundMixer *mixer, void *sound, uint32 size, uint rate,
																					 byte flags)
{
	_mixer = mixer;
	_flags = flags;
	_buffer_size = 1024 * size;
	_ptr = (byte *)malloc(_buffer_size);
	memcpy(_ptr, sound, size);
	_end_of_data = _ptr + size;
	if (_flags & FLAG_AUTOFREE)
		free(sound);
	_pos = _ptr;
	_fp_pos = 0;
	_fp_speed = (1 << 16) * rate / mixer->_output_rate;
	_to_be_destroyed = false;

	/* adjust the magnitute to prevent division error */
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;


	_rate = rate;
}

void SoundMixer::Channel_STREAM::append(void *data, uint32 len)
{
	byte *new_end = _end_of_data + len;
	byte *cur_pos = _pos;					/* This is just to prevent the variable to move during the tests :-) */
	if (new_end > (_ptr + _buffer_size)) {
		/* Wrap-around case */
		if ((_end_of_data < cur_pos) || (new_end >= cur_pos)) {
			warning("Mixer full... Trying to not break too much ");
			return;
		}
		memcpy(_end_of_data, data, (_ptr + _buffer_size) - _end_of_data);
		memcpy(_ptr, (byte *)data + ((_ptr + _buffer_size) - _end_of_data),
					 len - ((_ptr + _buffer_size) - _end_of_data));
	} else {
		if ((_end_of_data < cur_pos) && (new_end >= cur_pos)) {
			warning("Mixer full... Trying to not break too much ");
			return;
		}
		memcpy(_end_of_data, data, len);
	}
	_end_of_data = new_end;
}

void SoundMixer::Channel_STREAM::mix(int16 *data, uint len)
{
	uint32 fp_pos;
	const uint32 fp_speed = _fp_speed;
	const int16 *vol_tab = _mixer->_volume_table;
	byte *end_of_data = _end_of_data;

	if (_to_be_destroyed) {
		real_destroy();
		return;
	}

	fp_pos = _fp_pos;

	if (_pos < end_of_data) {
		mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &fp_pos, fp_speed, vol_tab, end_of_data);
	} else {
		mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &fp_pos, fp_speed, vol_tab,
																			 _ptr + _buffer_size);
		if (len != 0) {
			_pos = _ptr;
			mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &fp_pos, fp_speed, vol_tab,
																				 end_of_data);
		} else
			_to_be_destroyed = true;
	}
	if (len != 0) {
		// FIXME: BBrox, what does this mean? :)
		//        Commented by Ender to remove non-existant
		//        streamer bug in Dig smush movies.
		//warning("Streaming underflow of %d bytes", len);
		//real_destroy();
		//return;
	}
	_fp_pos = fp_pos;
}

void SoundMixer::Channel_STREAM::real_destroy()
{
	free(_ptr);
	_mixer->uninsert(this);
	delete this;
}



/* MP3 mixer goes here */
#ifdef COMPRESSED_SOUND_FILE
SoundMixer::Channel_MP3::Channel_MP3(SoundMixer *mixer, void *sound, uint size, byte flags)
{
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
	   from the start of the sound => we skip about 2 frames (at 22.05 khz).
	 */
	_silence_cut = 576 * 2;
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

void SoundMixer::Channel_MP3::mix(int16 *data, uint len)
{
	mad_fixed_t const *ch;
	const int16 *vol_tab = _mixer->_volume_table;
	unsigned char volume = ((int)vol_tab[1]) * 32 / 255;

	if (_to_be_destroyed) {
		real_destroy();
		return;
	}
	
	while (1) {
		ch = _synth.pcm.samples[0] + _pos_in_frame;

		/* Skip _silence_cut a the start */
		if ((_pos_in_frame < _synth.pcm.length) && (_silence_cut > 0)) {
			int diff = _synth.pcm.length - _pos_in_frame;
			
			if (diff > _silence_cut)
				diff = _silence_cut;
			_silence_cut -= diff;
			ch += diff;
			_pos_in_frame += diff;
		}

		while ((_pos_in_frame < _synth.pcm.length) && (len > 0)) {
			int16 sample = (int16)((scale_sample(*ch) * volume) / 32);
			*data++ += sample;
			*data++ += sample;
			len--;
			ch++;
			_pos_in_frame++;
		}
		if (len == 0)
			return;

		if (_position >= _size) {
			real_destroy();
			return;
		}

		mad_stream_buffer(&_stream, ((unsigned char *)_ptr) + _position,
											_size + MAD_BUFFER_GUARD - _position);

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

void SoundMixer::Channel_MP3::real_destroy()
{
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

SoundMixer::Channel_MP3_CDMUSIC::Channel_MP3_CDMUSIC(SoundMixer *mixer, FILE * file,
																										 mad_timer_t duration)
{
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

void SoundMixer::Channel_MP3_CDMUSIC::mix(int16 *data, uint len)
{
	mad_fixed_t const *ch;
	mad_timer_t frame_duration;
	unsigned char volume = _mixer->_music_volume * 32 / 255;

	if (_to_be_destroyed) {
		real_destroy();
		return;
	}

	if (!_initialized) {
		int skip_loop;
		// just skipped
		memset(_ptr, 0, _buffer_size);
		_size = fread(_ptr, 1, _buffer_size, _file);
		if (!_size) {
			real_destroy();
			return;
		}
		// Resync
		mad_stream_buffer(&_stream, (unsigned char *)_ptr, _size);
		skip_loop = 2;
		while (skip_loop != 0) {
			if (mad_frame_decode(&_frame, &_stream) == 0) {
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
		if (mad_frame_decode(&_frame, &_stream) == 0) {
			_pos_in_frame = 0;
			_initialized = true;
		} else {
			debug(1, "Cannot resume decoding");
			real_destroy();
			return;
		}
	}

	while (1) {
		// Get samples, play samples ... 
		ch = _synth.pcm.samples[0] + _pos_in_frame;
		while ((_pos_in_frame < _synth.pcm.length) && (len > 0)) {
			int16 sample = (int16)((scale_sample(*ch++) * volume) / 32);
			*data++ += sample;
			*data++ += sample;
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
		if (mad_frame_decode(&_frame, &_stream) == -1) {
			if (_stream.error == MAD_ERROR_BUFLEN) {
				int not_decoded;

				if (!_stream.next_frame) {
					memset(_ptr, 0, _buffer_size + MAD_BUFFER_GUARD);
					_size = fread(_ptr, 1, _buffer_size, _file);
					not_decoded = 0;
				} else {
					not_decoded = _stream.bufend - _stream.next_frame;
					memcpy(_ptr, _stream.next_frame, not_decoded);
					_size = fread((unsigned char *)_ptr + not_decoded, 1, _buffer_size - not_decoded, _file);
				}
				_stream.error = (enum mad_error)0;
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

bool SoundMixer::Channel_MP3_CDMUSIC::sound_finished()
{
	return mad_timer_compare(_duration, mad_timer_zero) <= 0;
}

void SoundMixer::Channel_MP3_CDMUSIC::real_destroy()
{
	free(_ptr);
	_mixer->uninsert(this);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	delete this;
}


#endif
