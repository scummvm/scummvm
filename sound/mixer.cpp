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

#include "stdafx.h"
#include "mixer.h"
#include "common/engine.h"	// for warning/error/debug
#include "common/file.h"


class Channel {
protected:
	SoundMixer *_mixer;
public:
	bool _toBeDestroyed;
	int _id;
	Channel() : _mixer(0), _toBeDestroyed(false), _id(-1) {}
	virtual ~Channel() {}
	virtual void mix(int16 *data, uint len) = 0;
	void destroy() {
		_toBeDestroyed = true;
	}
	virtual bool isActive();
	virtual bool isMusicChannel() = 0;
};

class ChannelRaw : public Channel {
	byte *_ptr;
	uint32 _pos;
	uint32 _size;
	uint32 _fpSpeed;
	uint32 _fpPos;
	uint32 _realSize, _rate;
	byte _flags;
	byte *_loop_ptr;
	uint32 _loop_size;

public:
	ChannelRaw(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags, int id);
	~ChannelRaw();

	void mix(int16 *data, uint len);
	bool isMusicChannel() {
		return false; // TODO: IS this correct? Or does any Scumm game us this for music?
	}
};

class ChannelStream : public Channel {
	byte *_ptr;
	byte *_endOfData;
	byte *_endOfBuffer;
	byte *_pos;
	uint32 _fpSpeed;
	uint32 _fpPos;
	uint32 _bufferSize;
	uint32 _rate;
	byte _flags;

public:
	ChannelStream(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags, int32 buffer_size);
	~ChannelStream();

	void mix(int16 *data, uint len);
	void append(void *sound, uint32 size);
	bool isMusicChannel() {
		return true;
	}
};

#ifdef USE_MAD

class ChannelMP3 : public Channel {
	byte *_ptr;
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
	~ChannelMP3();

	void mix(int16 *data, uint len);
	bool isMusicChannel() {
		return false;
	}
};

class ChannelMP3CDMusic : public Channel {
	byte *_ptr;
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
	~ChannelMP3CDMusic();

	void mix(int16 *data, uint len);
	bool isActive();
	bool isMusicChannel() {
		return true;
	}
};

#endif

#ifdef USE_VORBIS
class ChannelVorbis : public Channel {
	OggVorbis_File *_ov_file;
	int _end_pos;
	bool _eof_flag, _is_cd_track;

public:
	ChannelVorbis(SoundMixer *mixer, OggVorbis_File *ov_file, int duration, bool is_cd_track);

	void mix(int16 *data, uint len);
	bool isActive();
	bool isMusicChannel() {
		return _is_cd_track;
	}
};
#endif


SoundMixer::SoundMixer() {
	memset(this,0,sizeof(SoundMixer)); // palmos

	_volumeTable = (int16 *)calloc(256 * sizeof(int16), 1);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		_channels[i] = NULL;
	}
}

SoundMixer::~SoundMixer() {
	_syst->clear_sound_proc();
	free(_volumeTable);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		delete _channels[i];
	}
	_syst->delete_mutex(_mutex);
}

int SoundMixer::append(int index, void *sound, uint32 size) {
	_syst->lock_mutex(_mutex);

	Channel *chan = _channels[index];
	if (!chan) {
		error("Trying to stream to a nonexistant streamer : %d", index);
	} else {
		dynamic_cast<ChannelStream *>(chan)->append(sound, size);
	}

	_syst->unlock_mutex(_mutex);
	return 1;
}

int SoundMixer::insertChannel(PlayingSoundHandle *handle, Channel *chan) {
	int index = -1;
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			index = i;
			break;
		}
	}
	if(index == -1) {
		warning("SoundMixer::out of mixer slots");
		delete chan;
		return -1;
	}

	_channels[index] = chan;
	_handles[index] = handle;
	if (handle)
		*handle = index + 1;
	return index;
}

int SoundMixer::playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, int id) {
	return insertChannel(handle, new ChannelRaw(this, sound, size, rate, flags, id));
}

int SoundMixer::playStream(void *sound, uint32 size, uint rate, byte flags, int32 buffer_size) {
	return insertChannel(NULL, new ChannelStream(this, sound, size, rate, flags, buffer_size));
}

#ifdef USE_MAD
int SoundMixer::playMP3(PlayingSoundHandle *handle, void *sound, uint32 size, byte flags) {
	return insertChannel(handle, new ChannelMP3(this, sound, size, flags));
}
int SoundMixer::playMP3CDTrack(PlayingSoundHandle *handle, File *file, mad_timer_t duration) {
	return insertChannel(handle, new ChannelMP3CDMusic(this, file, duration));
}
#endif

#ifdef USE_VORBIS
int SoundMixer::playVorbis(PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track) {
	return insertChannel(handle, new ChannelVorbis(this, ov_file, duration, is_cd_track));
}
#endif

void SoundMixer::mix(int16 *buf, uint len) {
	_syst->lock_mutex(_mutex);
		
	if (_premixProc && !_paused) {
		int i;
		_premixProc(_premixParam, buf, len);
		for (i = (len - 1); i >= 0; i--) {
			buf[2 * i] = buf[2 * i + 1] = buf[i];
		}
	} else {
		//  zero the buf out
		memset(buf, 0, 2 * len * sizeof(int16));
	}

	if (!_paused) {
		// now mix all channels
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i]) {
				if (_channels[i]->_toBeDestroyed) {
					if (_handles[i]) {
						*_handles[i] = 0;
						_handles[i] = NULL;
					}
					delete _channels[i];
					_channels[i] = NULL;
				} else
					_channels[i]->mix(buf, len);
			}
	}
	
	_syst->unlock_mutex(_mutex);
}

void SoundMixer::onGenerateSamples(void *s, byte *samples, int len) {
	((SoundMixer *)s)->mix((int16 *)samples, len >> 2);
}

bool SoundMixer::bindToSystem(OSystem *syst) {
	uint rate = (uint) syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);
	_outputRate = rate;
	_syst = syst;
	_mutex = _syst->create_mutex();

	if (rate == 0)
		error("OSystem returned invalid sample rate");

	return syst->set_sound_proc(onGenerateSamples, this, OSystem::SOUND_16BIT);
}

void SoundMixer::stopAll() {
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->destroy();
}

void SoundMixer::stop(int index) {
	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::stop has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->destroy();
}

void SoundMixer::stopID(int id) {

	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != NULL && _channels[i]->_id == id) {
			_channels[i]->destroy();
			return;
		}
	}
}

void SoundMixer::pause(bool paused) {
	_paused = paused;
}

bool SoundMixer::hasActiveSFXChannel() {
	// FIXME/TODO: We need to distinguish between SFX and music channels
	// (and maybe also voice) here to work properly in iMuseDigital
	// games. In the past that was achieve using the _beginSlots hack.
	// Since we don't have that anymore, it's not that simple anymore.
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && !_channels[i]->isMusicChannel())
			return true;
	return false;
}

bool SoundMixer::isActiveChannel(int index) {
	if (_channels[index])
		return _channels[index]->isActive();
	return false;
}

void SoundMixer::setupPremix(void *param, PremixProc *proc) {
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
	CubicInterpolator(int a0, int b0, int c0) : x0(2 * a0 - b0), x1(a0), x2(b0), x3(c0) {
		// We use a simple linear interpolation for x0
		updateCoefficients();
	}
	
	inline void feedData() {
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = 2 * x2 - x1;	// Simple linear interpolation
		updateCoefficients();
	}

	inline void feedData(int xNew) {
		x0 = x1;
		x1 = x2;
		x2 = x3;
		x3 = xNew;
		updateCoefficients();
	}
	
	/* t must be a 16.16 fixed point number between 0 and 1 */
	inline int interpolate(uint32 fpPos) {
		int result = 0;
		int t = fpPos >> 8;
		result = (a * t + b) >> 8;
		result = (result * t + c) >> 8;
		result = (result * t + d) >> 8;
		result = (result / 3 + 1) >> 1;
		
		return result;
	}
		
protected:
	inline void updateCoefficients() {
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

static int16 *mix_signed_mono_8(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
								int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
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

static int16 *mix_unsigned_mono_8(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
											int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
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

static int16 *mix_signed_stereo_8(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
										int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
	warning("Mixing stereo signed 8 bit is not supported yet ");

	return data;
}
static int16 *mix_unsigned_stereo_8(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
										int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
	uint32 fp_pos = *fp_pos_ptr;
	byte *s = *s_ptr;
	uint len = *len_ptr;

	int inc = 1;
	CubicInterpolator	left(vol_tab[*s ^ 0x80], vol_tab[*(s + 2) ^ 0x80], vol_tab[*(s + 4) ^ 0x80]);
	CubicInterpolator	right(vol_tab[*(s + 1) ^ 0x80], vol_tab[*(s + 3) ^ 0x80], vol_tab[*(s + 5) ^ 0x80]);

	do {
		do {
			if (reverse_stereo == false) {
				*data = clamped_add_16(*data, left.interpolate(fp_pos));
				data++;
				*data = clamped_add_16(*data, right.interpolate(fp_pos));
				data++;
			} else {
				*data = clamped_add_16(*data, right.interpolate(fp_pos));
				data++;
				*data = clamped_add_16(*data, left.interpolate(fp_pos));
				data++;
			}

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
static int16 *mix_signed_mono_16(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
										 int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
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
static int16 *mix_unsigned_mono_16(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
										 int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
	warning("Mixing mono unsigned 16 bit is not supported yet ");

	return data;
}
static int16 *mix_signed_stereo_16(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
										 int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
	uint32 fp_pos = *fp_pos_ptr;
	unsigned char volume = ((int)vol_tab[1]) / 8;
	byte *s = *s_ptr;
	uint len = *len_ptr;
	do {
		fp_pos += fp_speed;

		if (reverse_stereo == false) {
			*data = clamped_add_16(*data, (((int16)(*(s) << 8) | *(s + 1)) * volume) / 32);
			data++;
			*data = clamped_add_16(*data, (((int16)(*(s + 2) << 8) | *(s + 3)) * volume) / 32);
			data++;
		} else {
			*data = clamped_add_16(*data, (((int16)(*(s + 2) << 8) | *(s + 3)) * volume) / 32);
			data++;
			*data = clamped_add_16(*data, (((int16)(*(s) << 8) | *(s + 1)) * volume) / 32);
			data++;
		}
		s += (fp_pos >> 16) << 2;
		fp_pos &= 0x0000FFFF;
	} while ((--len) && (s < s_end));

	*fp_pos_ptr = fp_pos;
	*s_ptr = s;
	*len_ptr = len;

	return data;
}
static int16 *mix_unsigned_stereo_16(int16 *data, uint *len_ptr, byte **s_ptr, uint32 *fp_pos_ptr,
											 int fp_speed, const int16 *vol_tab, byte *s_end, bool reverse_stereo) {
	warning("Mixing stereo unsigned 16 bit is not supported yet ");

	return data;
}

typedef int16 *MixProc(int16 *data, uint *len_ptr, byte **s_ptr,
                      uint32 *fp_pos_ptr, int fp_speed, const int16 *vol_tab,
                      byte *s_end, bool reverse_stereo);

static MixProc *mixer_helper_table[8] = { 
	mix_signed_mono_8, mix_unsigned_mono_8, 
	mix_signed_stereo_8, mix_unsigned_stereo_8,
	mix_signed_mono_16, mix_unsigned_mono_16, 
	mix_signed_stereo_16, mix_unsigned_stereo_16
};

static int16 mixer_element_size[] = {
	1, 1, 
	2, 2,
	2, 2,
	4, 4
};

bool Channel::isActive() {
	error("isActive should never be called on a non-MP3 mixer ");
	return true;
}

/* RAW mixer */
ChannelRaw::ChannelRaw(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags, int id) {
	_id = id;
	_mixer = mixer;
	_flags = flags;
	_ptr = (byte *)sound;
	_pos = 0;
	_fpPos = 0;
	_fpSpeed = (1 << 16) * rate / mixer->_outputRate;
	_realSize = size;

	// adjust the magnitude to prevent division error
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;

	_rate = rate;
	_size = size * mixer->_outputRate / rate;
	if (_flags & SoundMixer::FLAG_16BITS)
		_size = _size >> 1;
	if (_flags & SoundMixer::FLAG_STEREO)
		_size = _size >> 1;

	if (flags & SoundMixer::FLAG_LOOP) {
		_loop_ptr = _ptr;
		_loop_size = _size;
	}
}

ChannelRaw::~ChannelRaw() {
	if (_flags & SoundMixer::FLAG_AUTOFREE)
		free(_ptr);
}

void ChannelRaw::mix(int16 *data, uint len) {
	byte *s, *end;

	if (len > _size)
		len = _size;
	_size -= len;

	s = _ptr + _pos;
	end = _ptr + _realSize;

	const int16 *vol_tab = _mixer->_volumeTable;

	mixer_helper_table[_flags & 0x07] (data, &len, &s, &_fpPos, _fpSpeed, vol_tab, end, (_flags & SoundMixer::FLAG_REVERSE_STEREO) ? true : false);

	_pos = s - _ptr;

	if (_size < 1) {
		if (_flags & SoundMixer::FLAG_LOOP) {
			_ptr = _loop_ptr;
			_size = _loop_size;
			_pos = 0;
			_fpPos = 0;
		} else {
			destroy();
		}
	}
}

#define WARP_WORKAROUND 50000

ChannelStream::ChannelStream(SoundMixer *mixer, void *sound, uint32 size, uint rate,
										 byte flags, int32 buffer_size) {
	_mixer = mixer;
	_flags = flags;
	_bufferSize = buffer_size;
	_ptr = (byte *)malloc(_bufferSize + WARP_WORKAROUND);
	memcpy(_ptr, sound, size);
	_endOfData = _ptr + size;
	_endOfBuffer = _ptr + _bufferSize;
	_pos = _ptr;
	_fpPos = 0;
	_fpSpeed = (1 << 16) * rate / mixer->_outputRate;

	// adjust the magnitude to prevent division error
	while (size & 0xFFFF0000)
		size >>= 1, rate = (rate >> 1) + 1;

	_rate = rate;
}

ChannelStream::~ChannelStream() {
	free(_ptr);
}

void ChannelStream::append(void *data, uint32 len) {
	byte *new_end = _endOfData + len;
	byte *cur_pos = _pos;					/* This is just to prevent the variable to move during the tests :-) */
	if (new_end > (_ptr + _bufferSize)) {
		/* Wrap-around case */
		uint32 size_to_end_of_buffer = _endOfBuffer - _endOfData;
		uint32 new_size = len - size_to_end_of_buffer; 
		new_end = _ptr + new_size;
		if ((_endOfData < cur_pos) || (new_end >= cur_pos)) {
			debug(2, "Mixer full... Trying to not break too much ");
			return;
		}
		memcpy(_endOfData, (byte*)data, size_to_end_of_buffer);
		memcpy(_ptr, (byte *)data + size_to_end_of_buffer, new_size);
	} else {
		if ((_endOfData < cur_pos) && (new_end >= cur_pos)) {
			debug(2, "Mixer full... Trying to not break too much ");
			return;
		}
		memcpy(_endOfData, data, len);
	}
	_endOfData = new_end;
}

void ChannelStream::mix(int16 *data, uint len) {
	const int16 *vol_tab = _mixer->_volumeTable;

	if (_pos == _endOfData) {
		return;
	}

	if (_pos < _endOfData) {
		mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &_fpPos, _fpSpeed, vol_tab, _endOfData, (_flags & SoundMixer::FLAG_REVERSE_STEREO) ? true : false);
	} else {
		int wrap_offset = 0;

		// see if we will wrap
		if (_pos + (mixer_element_size[_flags & 0x07] * len) > _endOfBuffer) {
			wrap_offset = _pos + (mixer_element_size[_flags & 0x07] * len) - _endOfBuffer;
			debug(9, "using wrap workaround for %d bytes", wrap_offset);
			memcpy(_endOfBuffer, _ptr, wrap_offset);
		}
			 
		
		mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &_fpPos, _fpSpeed, vol_tab, _endOfBuffer + wrap_offset, (_flags & SoundMixer::FLAG_REVERSE_STEREO) ? true : false);

		// recover from wrap
		if (wrap_offset)
			_pos = _ptr + wrap_offset;

		// shouldn't happen anymore
		if (len != 0) {
			//FIXME: what is wrong ?
			warning("bad play sound in stream(wrap around)");
			_pos = _ptr;
			mixer_helper_table[_flags & 0x07] (data, &len, &_pos, &_fpPos, _fpSpeed, vol_tab, _endOfData, (_flags & SoundMixer::FLAG_REVERSE_STEREO) ? true : false);
		}
	}
}

#ifdef USE_MAD
ChannelMP3::ChannelMP3(SoundMixer *mixer, void *sound, uint size, byte flags) {
	_mixer = mixer;
	_flags = flags;
	_posInFrame = 0xFFFFFFFF;
	_position = 0;
	_size = size;
	_ptr = (byte *)sound;

	mad_stream_init(&_stream);
#ifdef _WIN32_WCE
	// 11 kHz on WinCE if necessary
	if ((uint)_mixer->_syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0) != 22050)
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

ChannelMP3::~ChannelMP3() {
	if (_flags & SoundMixer::FLAG_AUTOFREE)
		free(_ptr);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);
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
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

void ChannelMP3::mix(int16 *data, uint len) {
	mad_fixed_t const * ch;
	const int16 * vol_tab = _mixer->_volumeTable;
	unsigned char volume = ((int)vol_tab[1]) / 8;

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
			int16 sample = (int16)((scale_sample(*ch++) * volume) / 32);
			*data = clamped_add_16(*data, sample);
			data++;
			*data = clamped_add_16(*data, sample);
			data++;
			len--;
			_posInFrame++;
		}
		if (len == 0)
			return;

		if (_position >= _size) {
			destroy();
			return;
		}

		mad_stream_buffer(&_stream, _ptr + _position,
											_size + MAD_BUFFER_GUARD - _position);

		if (mad_frame_decode(&_frame, &_stream) == -1) {
			/* End of audio... */
			if (_stream.error == MAD_ERROR_BUFLEN) {
				destroy();
				return;
			} else if (!MAD_RECOVERABLE(_stream.error)) {
				error("MAD frame decode error !");
			}
		}
		mad_synth_frame(&_synth, &_frame);
		_posInFrame = 0;
		_position = _stream.next_frame - _ptr;
	}
}

#define MP3CD_BUFFERING_SIZE 131072

ChannelMP3CDMusic::ChannelMP3CDMusic(SoundMixer *mixer, File *file,
														 mad_timer_t duration){
	_mixer = mixer;
	_file = file;
	_duration = duration;
	_initialized = false;
	_bufferSize = MP3CD_BUFFERING_SIZE;
	_ptr = (byte *)malloc(MP3CD_BUFFERING_SIZE);

	mad_stream_init(&_stream);
#ifdef _WIN32_WCE
	// 11 kHz on WinCE if necessary
	if ((uint)_mixer->_syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0) != 22050)
		mad_stream_options(&_stream, MAD_OPTION_HALFSAMPLERATE);
#endif
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
}

ChannelMP3CDMusic::~ChannelMP3CDMusic() {
	free(_ptr);
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);
}

void ChannelMP3CDMusic::mix(int16 *data, uint len) {
	mad_fixed_t const *ch;
	mad_timer_t frame_duration;
	unsigned char volume = _mixer->_musicVolume / 8;

	if (!_initialized) {
		int skip_loop;
		// just skipped
		memset(_ptr, 0, _bufferSize);
		_size = _file->read(_ptr, _bufferSize);
		if (!_size) {
			destroy();
			return;
		}
		// Resync
		mad_stream_buffer(&_stream, _ptr, _size);
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
					destroy();
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
			destroy();
			return;
		}
	}

	while (1) {
		// Get samples, play samples ... 
		ch = _synth.pcm.samples[0] + _posInFrame;
		while ((_posInFrame < _synth.pcm.length) && (len > 0)) {
			int16 sample = (int16)((scale_sample(*ch++) * volume) / 32);
			*data = clamped_add_16(*data, sample);
			data++;
			*data = clamped_add_16(*data, sample);
			data++;
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
					_size = _file->read(_ptr, _bufferSize);
					not_decoded = 0;
				} else {
					not_decoded = _stream.bufend - _stream.next_frame;
					memcpy(_ptr, _stream.next_frame, not_decoded);
					_size = _file->read(_ptr + not_decoded, _bufferSize - not_decoded);
				}
				_stream.error = (enum mad_error)0;
				// Restream
				mad_stream_buffer(&_stream, _ptr, _size + not_decoded);
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

bool ChannelMP3CDMusic::isActive() {
	return mad_timer_compare(_duration, mad_timer_zero) > 0;
}

#endif

#ifdef USE_VORBIS
ChannelVorbis::ChannelVorbis(SoundMixer *mixer, OggVorbis_File *ov_file, int duration, bool is_cd_track) {
	_mixer = mixer;
	_ov_file = ov_file;

	if (duration)
		_end_pos = ov_pcm_tell(ov_file) + duration;
	else
		_end_pos = 0;

	_eof_flag = false;
	_is_cd_track = is_cd_track;
}

void ChannelVorbis::mix(int16 *data, uint len) {

	if (_eof_flag) {
		memset(data, 0, sizeof(int16) * 2 * len);
		return;
	}

	int channels = ov_info(_ov_file, -1)->channels;
	uint len_left = len * channels * 2;
	int16 *samples = new int16[len_left / 2];
	char *read_pos = (char *) samples;
	int volume = _is_cd_track ? _mixer->_musicVolume : _mixer->_volumeTable[1];

	// Read the samples
	while (len_left > 0) {
		long result = ov_read(_ov_file, read_pos, len_left,
#ifdef SCUMM_BIG_ENDIAN
				      1,
#else
				      0,
#endif
				      2, 1, NULL);
		if (result == 0) {
			_eof_flag = true;
			memset(read_pos, 0, len_left);
			break;
		}
		else if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		}
		else if (result < 0) {
			debug(1, "Decode error %d in Vorbis file", result);
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			_eof_flag = true;
			memset(read_pos, 0, len_left);
			break;
		}
		else {
			len_left -= result;
			read_pos += result;
		}
	}

	// Mix the samples in
	for (uint i = 0; i < len; i++) {
		int16 sample = (int16) ((int32) samples[i * channels] * volume / 256);
		*data = clamped_add_16(*data, sample);
		data++;
		if (channels > 1)
			sample = (int16) ((int32) samples[i * channels + 1] * volume / 256);
		*data = clamped_add_16(*data, sample);
		data++;
	}

	delete [] samples;

	if (_eof_flag && ! _is_cd_track)
		destroy();
}

bool ChannelVorbis::isActive() {
	return !_eof_flag && (_end_pos <= 0 && ov_pcm_tell(_ov_file) >= _end_pos);
}

#endif
