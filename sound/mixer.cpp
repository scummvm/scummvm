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
#include "mixer.h"

SoundMixer::SoundMixer() {
	_volumeTable = (int16 *)calloc(256 * sizeof(int16), 1);
}

SoundMixer::~SoundMixer() {
	free(_volumeTable);
}

void SoundMixer::unInsert(Channel * chan) {
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

int SoundMixer::append(int index, void * sound, uint32 size, uint rate, byte flags) {
	_syst->lock_mutex(_mutex);

	Channel * chan = _channels[index];
	if (!chan) {
		warning("Trying to stream to an unexistant streamer : %d", index);
		playStream(NULL, index, sound, size, rate, flags);
		chan = _channels[index];
	} else {
		chan->append(sound, size);
		if (flags & FLAG_AUTOFREE)
			free(sound);
	}

	_syst->unlock_mutex(_mutex);
	return 1;
}

int SoundMixer::insertAt(PlayingSoundHandle * handle, int index, Channel * chan) {
	if(index == -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] == NULL) { index = i; break; }
		if(index == -1) {
			warning("SoundMixer::out of mixer slots");
			return -1;
		}
	}
	if (_channels[index] != NULL) {
		error("Trying to put a mixer where it cannot go ");
	}
	_channels[index] = chan;
	_handles[index] = handle;
	if (handle)
		*handle = index + 1;
	return index;
}

int SoundMixer::playRaw(PlayingSoundHandle * handle, void * sound, uint32 size, uint rate,
												 byte flags) {
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			return insertAt(handle, i, new ChannelRaw(this, sound, size, rate, flags));
		}
	}

	warning("SoundMixer::out of mixer slots");
	return -1;
}

int SoundMixer::playStream(PlayingSoundHandle * handle, int idx, void * sound, uint32 size,
														uint rate, byte flags) {
	return insertAt(handle, idx, new ChannelStream(this, sound, size, rate, flags));
}

#ifdef COMPRESSED_SOUND_FILE
int SoundMixer::playMP3(PlayingSoundHandle * handle, void *sound, uint32 size, byte flags) {
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			return insertAt(handle, i, new ChannelMP3(this, sound, size, flags));
		}
	}

	warning("SoundMixer::out of mixer slots");
	return -1;
}
int SoundMixer::playMP3CDTrack(PlayingSoundHandle * handle, File * file, mad_timer_t duration) {
	/* Stop the previously playing CD track (if any) */
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			return insertAt(handle, i, new ChannelMP3CDMusic(this, file, duration));
		}
	}

	warning("SoundMixer::out of mixer slots");
	return -1;
}
#endif

void SoundMixer::mix(int16 *buf, uint len) {
	if (_paused) {
		memset(buf, 0, 2 * len * sizeof(int16));
		return;
	}
	
	if (_premixProc) {
		int i;
		_premixProc(_premixParam, buf, len);
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

void SoundMixer::onGenerateSamples(void * s, byte * samples, int len) {
	((SoundMixer *)s)->mix((int16 *)samples, len >> 2);
}

bool SoundMixer::bindToSystem(OSystem * syst) {
	uint rate = (uint) syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	_outputRate = rate;

	_syst = syst;
	_mutex = _syst->create_mutex();

	if (rate == 0)
		error("OSystem returned invalid sample rate");

	return syst->set_sound_proc(this, onGenerateSamples, OSystem::SOUND_16BIT);
}

void SoundMixer::stopAll() {
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->destroy();
}

void SoundMixer::stop(PlayingSoundHandle psh) {
	if (psh && _channels[psh - 1])
		_channels[psh - 1]->destroy();
}

void SoundMixer::stop(int index) {
	if (_channels[index])
		_channels[index]->destroy();
}

void SoundMixer::pause(bool paused) {
	_paused = paused;
}

bool SoundMixer::hasActiveChannel() {
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			return true;
	return false;
}

void SoundMixer::setupPremix(void * param, PremixProc * proc) {
	_premixParam = param;
	_premixProc = proc;
}

void SoundMixer::setVolume(int volume) {
	int i;

	// Check range
	if (volume > 256)
		volume = 256;
	else if (volume < 0)
		volume = 0;

	// The volume table takes 8 bit unsigned data as index and returns 16 bit signed
	for (i = 0; i < 128; i++)
		_volumeTable[i] = i * volume;

	for (i = -128; i < 0; i++)
		_volumeTable[i + 256] = i * volume;
}

void SoundMixer::setMusicVolume(int volume) {
	// Check range
	if (volume > 256)
		volume = 256;
	else if (volume < 0)
		volume = 0;

	_musicVolume = volume;
}

#ifdef COMPRESSED_SOUND_FILE
bool SoundMixer::Channel::soundFinished() {
	warning("sound_finished should never be called on a non-MP3 mixer ");
	return false;
}
#endif

void SoundMixer::Channel::append(void * sound, uint32 size) {
	error("append method should never be called on something else than a _STREAM mixer ");
}

/* RAW mixer */
SoundMixer::ChannelRaw::ChannelRaw(SoundMixer * mixer, void * sound, uint32 size, uint rate,
																 byte flags) {
	_mixer = mixer;
	_flags = flags;
	_ptr = sound;
	_pos = 0;
	_fpPos = 0;
	_fpSpeed = (1 << 16) * rate / mixer->_outputRate;
	_toBeDestroyed = false;
	_realSize = size;

	// adjust the magnitude to prevent division error
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;

	_rate = rate;
	_size = size * mixer->_outputRate / rate;
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
	CubicInterpolator(int a, int b, int c) : x0(2 * a - b), x1(a), x2(b), x3(c)
	{
		// We use a simple linear interpolation for x0
		updateCoefficients();
	}
	
	inline void feedData()
	{
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = 2 * x2 - x1;	// Simple linear interpolation
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
	inline int interpolate(uint32 fpPos)
	{
		int result = 0;
		int t = fpPos >> 8;
		result = (a * t + b) >> 8;
		result = (result * t + c) >> 8;
		result = (result * t + d) >> 8;
		result = (result / 3 + 1) >> 1;
		
		return result;
	}
		
protected:
	inline void updateCoefficients()
	{
		a = ((-x0 * 2) + (x1 * 5) - (x2 * 4) + x3);
		b = ((x0 + x2 - (2 * x1)) * 6) << 8;
		c = ((-4 * x0) + x1 + (x2 * 4) - x3) << 8;
		d = (x1 * 6) << 8;
	}
};

static inline int clamped_add_16(int a, int b) {
	int val = a + b;

	if (val > 32767) {
		return 32767;
	} else if (val < -32768) {
		return -32768;
	} else
		return val;
}

static int16 * mix_signed_mono_8(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
								int fp_speed, const int16 * vol_tab, byte * s_end) {
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	
	int inc = 1, result;
	CubicInterpolator interp(vol_tab[*s], vol_tab[*(s + 1)], vol_tab[*(s + 2)]);

	do {
		do {
			result = interp.interpolate(fp_pos);
			
			*data = clamped_add_16(*data, result);
			data++;
			*data = clamped_add_16(*data, result);
			data++;
	
			fp_pos += fp_speed;
			inc = fp_pos >> 16;
			s += inc;
			len--;
			fp_pos &= 0x0000FFFF;
		} while (!inc && len && (s < s_end));
		
		if (s + 2 < s_end)
			interp.feedData(vol_tab[*(s + 2)]);
		else
			interp.feedData();

	} while (len && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}

static int16 * mix_unsigned_mono_8(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
											int fp_speed, const int16 * vol_tab, byte * s_end) {
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	
	int inc = 1, result;
	CubicInterpolator interp(vol_tab[*s ^ 0x80], vol_tab[*(s + 1) ^ 0x80], vol_tab[*(s + 2) ^ 0x80]);

	do {
		do {
			result = interp.interpolate(fp_pos);
	
			*data = clamped_add_16(*data, result);
			data++;
			*data = clamped_add_16(*data, result);
			data++;
	
			fp_pos += fp_speed;
			inc = fp_pos >> 16;
			s += inc;
			len--;
			fp_pos &= 0x0000FFFF;
		} while (!inc && len && (s < s_end));

		if (s + 2 < s_end)
			interp.feedData(vol_tab[*(s + 2) ^ 0x80]);
		else
			interp.feedData();

	} while (len && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}

static int16 * mix_signed_stereo_8(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
										int fp_speed, const int16 * vol_tab, byte *s_end) {
	warning("Mixing stereo signed 8 bit is not supported yet ");

	return data;
}
static int16 * mix_unsigned_stereo_8(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
										int fp_speed, const int16 * vol_tab, byte * s_end) {
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	
	int inc = 1;
	CubicInterpolator	left(vol_tab[*s ^ 0x80], vol_tab[*(s + 2) ^ 0x80], vol_tab[*(s + 4) ^ 0x80]);
	CubicInterpolator	right(vol_tab[*(s + 1) ^ 0x80], vol_tab[*(s + 3) ^ 0x80], vol_tab[*(s + 5) ^ 0x80]);

	do {
		do {
			*data = clamped_add_16(*data, left.interpolate(fp_pos));
			data++;
			*data = clamped_add_16(*data, right.interpolate(fp_pos));
			data++;

			fp_pos += fp_speed;
			inc = (fp_pos >> 16) << 1;
			s += inc;
			len--;
			fp_pos &= 0x0000FFFF;
		} while (!inc && len && (s < s_end));

		if (s + 5 < s_end) {
			left.feedData(vol_tab[*(s + 4) ^ 0x80]);
			right.feedData(vol_tab[*(s + 5) ^ 0x80]);
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
static int16 * mix_signed_mono_16(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
										 int fp_speed, const int16 * vol_tab, byte * s_end) {
	uint32 fp_pos = *fp_pos_ptr;
	unsigned char volume = ((int)vol_tab[1]) / 8;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	do {
		int16 sample = (((int16)(*s << 8) | *(s + 1)) * volume) / 32;
		fp_pos += fp_speed;

		*data = clamped_add_16(*data, sample);
		data++;
		*data = clamped_add_16(*data, sample);
		data++;

		s += (fp_pos >> 16) << 1;
		fp_pos &= 0x0000FFFF;
	} while ((--len) && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_unsigned_mono_16(int16 *data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
										 int fp_speed, const int16 * vol_tab, byte * s_end) {
	warning("Mixing mono unsigned 16 bit is not supported yet ");

	return data;
}
static int16 *mix_signed_stereo_16(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
										 int fp_speed, const int16 * vol_tab, byte * s_end) {
	uint32 fp_pos = *fp_pos_ptr;
	unsigned char volume = ((int)vol_tab[1]) / 8;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	do {
		fp_pos += fp_speed;

		*data = clamped_add_16(*data, (((int16)(*(s) << 8) | *(s + 1)) * volume) / 32);
		data++;
		*data = clamped_add_16(*data, (((int16)(*(s + 2) << 8) | *(s + 3)) * volume) / 32);
		data++;

		s += (fp_pos >> 16) << 2;
		fp_pos &= 0x0000FFFF;
	} while ((--len) && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 * mix_unsigned_stereo_16(int16 * data, uint * len_ptr, byte ** s_ptr, uint32 * fp_pos_ptr,
											 int fp_speed, const int16 * vol_tab, byte * s_end) {
	warning("Mixing stereo unsigned 16 bit is not supported yet ");

	return data;
}

static int16 * (*mixer_helper_table[8]) (int16 * data, uint * len_ptr, byte ** s_ptr,
										 uint32 * fp_pos_ptr, int fp_speed, const int16 * vol_tab,
										 byte * s_end) = { 
	mix_signed_mono_8, mix_unsigned_mono_8, 
	mix_signed_stereo_8, mix_unsigned_stereo_8,
	mix_signed_mono_16, mix_unsigned_mono_16, 
	mix_signed_stereo_16, mix_unsigned_stereo_16
};

void SoundMixer::ChannelRaw::mix(int16 * data, uint len) {
	byte *s, *s_org = NULL;
	uint32 fp_pos;
	byte *end;

	if (_toBeDestroyed) {
		realDestroy();
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
		uint num = len * _fpSpeed >> 16;

		s_org = (byte *)malloc(num);
		if (s_org == NULL)
			error("ChannelRaw::mix out of memory");

		uint num_read = ((File *)_ptr)->read(s_org, num);
		if (num - num_read != 0)
			memset(s_org + num_read, 0x80, num - num_read);

		s = s_org;
		fp_pos = 0;
		end = s_org + num;
	} else {
		s = (byte *)_ptr + _pos;
		fp_pos = _fpPos;
		end = (byte *)_ptr + _realSize;
	}

	const uint32 fp_speed = _fpSpeed;
	const int16 *vol_tab = _mixer->_volumeTable;

	mixer_helper_table[_flags & 0x07] (data, &len, &s, &fp_pos, fp_speed, vol_tab, end);

	_pos = s - (byte *)_ptr;
	_fpPos = fp_pos;

	if (_flags & FLAG_FILE) {
		free(s_org);
	}

	if (_size < 1)
		realDestroy();

}

void SoundMixer::ChannelRaw::realDestroy() {
	if (_flags & FLAG_AUTOFREE)
		free(_ptr);
	_mixer->unInsert(this);
	delete this;
}

SoundMixer::ChannelStream::ChannelStream(SoundMixer * mixer, void * sound, uint32 size, uint rate,
										 byte flags) {
	_mixer = mixer;
	_flags = flags;
	_bufferSize = 1024 * size;
	_ptr = (byte *)malloc(_bufferSize);
	memcpy(_ptr, sound, size);
	_endOfData = _ptr + size;
	if (_flags & FLAG_AUTOFREE)
		free(sound);
	_pos = _ptr;
	_fpPos = 0;
	_fpSpeed = (1 << 16) * rate / mixer->_outputRate;
	_toBeDestroyed = false;

	/* adjust the magnitute to prevent division error */
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;

	_rate = rate;
}

void SoundMixer::ChannelStream::append(void * data, uint32 len) {
	byte *new_end = _endOfData + len;
	byte *cur_pos = _pos;					/* This is just to prevent the variable to move during the tests :-) */
	if (new_end > (_ptr + _bufferSize)) {
		/* Wrap-around case */
		new_end = _ptr + len - ((_ptr + _bufferSize) - _endOfData);
		if ((_endOfData < cur_pos) || (new_end >= cur_pos)) {
			warning("Mixer full... Trying to not break too much ");
			return;
		}
		memcpy(_endOfData, data, (_ptr + _bufferSize) - _endOfData);
		memcpy(_ptr, (byte *)data + ((_ptr + _bufferSize) - _endOfData),
					 len - ((_ptr + _bufferSize) - _endOfData));
	} else {
		if ((_endOfData < cur_pos) && (new_end >= cur_pos)) {
			warning("Mixer full... Trying to not break too much ");
			return;
		}
		memcpy(_endOfData, data, len);
	}
	_endOfData = new_end;
}

void SoundMixer::ChannelStream::mix(int16 * data, uint len) {
	uint32 fp_pos;
	const uint32 fp_speed = _fpSpeed;
	const int16 * vol_tab = _mixer->_volumeTable;
	byte * end_of_data = _endOfData;

	if (_toBeDestroyed) {
		realDestroy();
		return;
	}

	fp_pos = _fpPos;

	if (_pos < end_of_data) {
		mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &fp_pos, fp_speed, vol_tab, end_of_data);
	} else {
		_toBeDestroyed = true;
	}

	_fpPos = fp_pos;
}

void SoundMixer::ChannelStream::realDestroy() {
	free(_ptr);
	_mixer->unInsert(this);
	delete this;
}

#ifdef COMPRESSED_SOUND_FILE
SoundMixer::ChannelMP3::ChannelMP3(SoundMixer * mixer, void * sound, uint size, byte flags) {
	_mixer = mixer;
	_flags = flags;
	_posInFrame = 0xFFFFFFFF;
	_position = 0;
	_size = size;
	_ptr = sound;
	_toBeDestroyed = false;

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
	_silenceCut = 576 * 2;
}

static inline int scale_sample(mad_fixed_t sample) {
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

void SoundMixer::ChannelMP3::mix(int16 * data, uint len) {
	mad_fixed_t const * ch;
	const int16 * vol_tab = _mixer->_volumeTable;
	unsigned char volume = ((int)vol_tab[1]) / 8;

	if (_toBeDestroyed) {
		realDestroy();
		return;
	}
	
	while (1) {
		ch = _synth.pcm.samples[0] + _posInFrame;

		/* Skip _silence_cut a the start */
		if ((_posInFrame < _synth.pcm.length) && (_silenceCut > 0)) {
			uint32 diff = _synth.pcm.length - _posInFrame;
			
			if (diff > _silenceCut)
				diff = _silenceCut;
			_silenceCut -= diff;
			ch += diff;
			_posInFrame += diff;
		}

		while ((_posInFrame < _synth.pcm.length) && (len > 0)) {
			int16 sample = (int16)((scale_sample(*ch) * volume) / 32);
			*data++ += sample;
			*data++ += sample;
			len--;
			ch++;
			_posInFrame++;
		}
		if (len == 0)
			return;

		if (_position >= _size) {
			realDestroy();
			return;
		}

		mad_stream_buffer(&_stream, ((unsigned char *)_ptr) + _position,
											_size + MAD_BUFFER_GUARD - _position);

		if (mad_frame_decode(&_frame, &_stream) == -1) {
			/* End of audio... */
			if (_stream.error == MAD_ERROR_BUFLEN) {
				realDestroy();
				return;
			} else if (!MAD_RECOVERABLE(_stream.error)) {
				error("MAD frame decode error !");
			}
		}
		mad_synth_frame(&_synth, &_frame);
		_posInFrame = 0;
		_position = (unsigned char *)_stream.next_frame - (unsigned char *)_ptr;
	}
}

void SoundMixer::ChannelMP3::realDestroy() {
	if (_flags & FLAG_AUTOFREE)
		free(_ptr);
	_mixer->unInsert(this);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	delete this;
}

#define MP3CD_BUFFERING_SIZE 131072

SoundMixer::ChannelMP3CDMusic::ChannelMP3CDMusic(SoundMixer * mixer, File * file,
														 mad_timer_t duration){
	_mixer = mixer;
	_file = file;
	_duration = duration;
	_initialized = false;
	_bufferSize = MP3CD_BUFFERING_SIZE;
	_ptr = malloc(MP3CD_BUFFERING_SIZE);
	_toBeDestroyed = false;

	mad_stream_init(&_stream);
#ifdef _WIN32_WCE
	// 11 kHz on WinCE
	mad_stream_options(&_stream, MAD_OPTION_HALFSAMPLERATE);
#endif
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
}

void SoundMixer::ChannelMP3CDMusic::mix(int16 * data, uint len) {
	mad_fixed_t const *ch;
	mad_timer_t frame_duration;
	unsigned char volume = _mixer->_musicVolume / 8;

	if (_toBeDestroyed) {
		realDestroy();
		return;
	}

	if (!_initialized) {
		int skip_loop;
		// just skipped
		memset(_ptr, 0, _bufferSize);
		_size = _file->read(_ptr, _bufferSize);
		if (!_size) {
			realDestroy();
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
					realDestroy();
					return;
				}
			}
		}
		// We are supposed to be in synch
		mad_frame_mute(&_frame);
		mad_synth_mute(&_synth);
		// Resume decoding
		if (mad_frame_decode(&_frame, &_stream) == 0) {
			_posInFrame = 0;
			_initialized = true;
		} else {
			debug(1, "Cannot resume decoding");
			realDestroy();
			return;
		}
	}

	while (1) {
		// Get samples, play samples ... 
		ch = _synth.pcm.samples[0] + _posInFrame;
		while ((_posInFrame < _synth.pcm.length) && (len > 0)) {
			int16 sample = (int16)((scale_sample(*ch++) * volume) / 32);
			*data++ += sample;
			*data++ += sample;
			len--;
			_posInFrame++;
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
					memset(_ptr, 0, _bufferSize + MAD_BUFFER_GUARD);
					_size = fread(_ptr, 1, _bufferSize, _file);
					not_decoded = 0;
				} else {
					not_decoded = _stream.bufend - _stream.next_frame;
					memcpy(_ptr, _stream.next_frame, not_decoded);
					_size = _file->read((unsigned char *)_ptr + not_decoded, _bufferSize - not_decoded);
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
		_posInFrame = 0;
	}
}

bool SoundMixer::ChannelMP3CDMusic::soundFinished() {
	return mad_timer_compare(_duration, mad_timer_zero) <= 0;
}

void SoundMixer::ChannelMP3CDMusic::realDestroy() {
	free(_ptr);
	_mixer->unInsert(this);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	delete this;
}

#endif
