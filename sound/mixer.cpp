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
#include "common/file.h"
#include "common/util.h"

#include "sound/mixer.h"
#include "sound/rate.h"
#include "sound/audiostream.h"


#pragma mark -
#pragma mark --- Channel classes ---
#pragma mark -


/**
 * Channels used by the sound mixer.
 */
class Channel {
protected:
	SoundMixer *_mixer;
	PlayingSoundHandle *_handle;
	RateConverter *_converter;
	AudioInputStream *_input;
	byte _volume;
	int8 _pan;
	bool _paused;

public:
	int _id;

	Channel(SoundMixer *mixer, PlayingSoundHandle *handle)
		: _mixer(mixer), _handle(handle), _converter(0), _input(0), _volume(0), _pan(0), _paused(false), _id(-1) {
		assert(mixer);
	}
	virtual ~Channel();
	void destroy();
	virtual void mix(int16 *data, uint len);
	virtual void pause(bool paused) {
		_paused = paused;
	}
	virtual bool isPaused() {
		return _paused;
	}
	virtual void setChannelVolume(const byte volume) {
		_volume = volume;
	}
	virtual void setChannelPan(const int8 pan) {
		_pan = pan;
	}
	virtual int getVolume() const {
		return isMusicChannel() ? _mixer->getMusicVolume() : _mixer->getVolume();
	}
	virtual bool isMusicChannel() const	= 0;
};

class ChannelRaw : public Channel {
	byte *_ptr;
public:
	ChannelRaw(SoundMixer *mixer, PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, byte volume, int8 pan, int id, uint32 loopStart, uint32 loopEnd);
	~ChannelRaw();
	bool isMusicChannel() const		{ return false; }
};

class ChannelStream : public Channel {
	bool _finished;
public:
	ChannelStream(SoundMixer *mixer, PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, uint32 buffer_size, byte volume, int8 pan);
	void mix(int16 *data, uint len);
	void append(void *sound, uint32 size);
	bool isMusicChannel() const		{ return true; }
	void finish()					{ _finished = true; }
};

#ifdef USE_MAD
class ChannelMP3 : public Channel {
public:
	ChannelMP3(SoundMixer *mixer, PlayingSoundHandle *handle, File *file, uint size, byte volume, int8 pan);
	bool isMusicChannel() const		{ return false; }
};

class ChannelMP3CDMusic : public Channel {
public:
	ChannelMP3CDMusic(SoundMixer *mixer, PlayingSoundHandle *handle, File *file, mad_timer_t duration, byte volume, int8 pan);
	bool isMusicChannel() const		{ return true; }
};
#endif // USE_MAD

#ifdef USE_VORBIS
class ChannelVorbis : public Channel {
	bool _is_cd_track;
public:
	ChannelVorbis(SoundMixer *mixer, PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track, byte volume, int8 pan);
	bool isMusicChannel() const		{ return _is_cd_track; }
};
#endif // USE_VORBIS


#pragma mark -
#pragma mark --- SoundMixer ---
#pragma mark -


SoundMixer::SoundMixer() {
	_syst = 0;
	_mutex = 0;

	_premixParam = 0;
	_premixProc = 0;
	int i = 0;

	_outputRate = 0;

	_globalVolume = 0;
	_musicVolume = 0;

	_paused = false;

	for (i = 0; i != NUM_CHANNELS; i++)
		_channels[i] = NULL;
}

SoundMixer::~SoundMixer() {
	_syst->clear_sound_proc();
	for (int i = 0; i != NUM_CHANNELS; i++) {
		delete _channels[i];
	}
	_syst->delete_mutex(_mutex);
}

bool SoundMixer::bindToSystem(OSystem *syst) {
	_syst = syst;
	_mutex = _syst->create_mutex();
	_outputRate = (uint) syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	if (_outputRate == 0)
		error("OSystem returned invalid sample rate");

	return syst->set_sound_proc(mixCallback, this, OSystem::SOUND_16BIT);
}

void SoundMixer::setupPremix(PremixProc *proc, void *param) {
	StackLock lock(_mutex);
	_premixParam = param;
	_premixProc = proc;
}

int SoundMixer::newStream(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, uint32 buffer_size, byte volume, int8 pan) {
	StackLock lock(_mutex);
	return insertChannel(handle, new ChannelStream(this, handle, sound, size, rate, flags, buffer_size, volume, pan));
}

void SoundMixer::appendStream(PlayingSoundHandle handle, void *sound, uint32 size) {
	StackLock lock(_mutex);
	
	if (handle == 0)
		return;

	int index = handle - 1;

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::appendStream has invalid index %d", index);
		return;
	}

	ChannelStream *chan;
#if !defined(_WIN32_WCE) && !defined(__PALM_OS__)
	chan = dynamic_cast<ChannelStream *>(_channels[index]);
#else
	chan = (ChannelStream*)_channels[index];
#endif
	if (!chan) {
		error("Trying to append to nonexistant streamer : %d", index);
	} else {
		chan->append(sound, size);
	}
}

void SoundMixer::endStream(PlayingSoundHandle handle) {
	StackLock lock(_mutex);
	
	// Simply ignore stop requests for handles of sounds that already terminated
	if (handle == 0)
		return;

	int index = handle - 1;

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::endStream has invalid index %d", index);
		return;
	}

	ChannelStream *chan;
#if !defined(_WIN32_WCE) && !defined(__PALM_OS__)
	chan = dynamic_cast<ChannelStream *>(_channels[index]);
#else
	chan = (ChannelStream*)_channels[index];
#endif
	if (!chan) {
		error("Trying to end a nonexistant streamer : %d", index);
	} else {
		chan->finish();
	}
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
	if (handle)
		*handle = index + 1;
	return index;
}

int SoundMixer::playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, int id, byte volume, int8 pan, uint32 loopStart, uint32 loopEnd) {
	StackLock lock(_mutex);

	// Prevent duplicate sounds
	if (id != -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] != NULL && _channels[i]->_id == id)
				return -1;
	}

	return insertChannel(handle, new ChannelRaw(this, handle, sound, size, rate, flags, volume, pan, id, loopStart, loopEnd));
}

#ifdef USE_MAD
int SoundMixer::playMP3(PlayingSoundHandle *handle, File *file, uint32 size, byte volume, int8 pan) {
	StackLock lock(_mutex);
	return insertChannel(handle, new ChannelMP3(this, handle, file, size, volume, pan));
}
int SoundMixer::playMP3CDTrack(PlayingSoundHandle *handle, File *file, mad_timer_t duration, byte volume, int8 pan) {
	StackLock lock(_mutex);
	return insertChannel(handle, new ChannelMP3CDMusic(this, handle, file, duration, volume, pan));
}
#endif

#ifdef USE_VORBIS
int SoundMixer::playVorbis(PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track, byte volume, int8 pan) {
	StackLock lock(_mutex);
	return insertChannel(handle, new ChannelVorbis(this, handle, ov_file, duration, is_cd_track, volume, pan));
}
#endif

void SoundMixer::mix(int16 *buf, uint len) {
	StackLock lock(_mutex);

	if (_premixProc && !_paused) {
		_premixProc(_premixParam, buf, len);
	} else {
		//  zero the buf out
		memset(buf, 0, 2 * len * sizeof(int16));
	}

	if (!_paused) {
		// now mix all channels
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] && !_channels[i]->isPaused())
				_channels[i]->mix(buf, len);
	}
}

void SoundMixer::mixCallback(void *s, byte *samples, int len) {
	assert(s);
	assert(samples);
	// Len is the number of bytes in the buffer; we divide it by
	// four to get the number of samples (stereo 16 bit).
	((SoundMixer *)s)->mix((int16 *)samples, len >> 2);
}

void SoundMixer::stopAll() {
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->destroy();
}

void SoundMixer::stopChannel(int index) {
	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::stop has invalid index %d", index);
		return;
	}

	StackLock lock(_mutex);
	if (_channels[index])
		_channels[index]->destroy();
}

void SoundMixer::stopID(int id) {
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != NULL && _channels[i]->_id == id) {
			_channels[i]->destroy();
			return;
		}
	}
}

void SoundMixer::stopHandle(PlayingSoundHandle handle) {
	StackLock lock(_mutex);
	
	// Simply ignore stop requests for handles of sounds that already terminated
	if (handle == 0)
		return;

	int index = handle - 1;

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::stopHandle has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->destroy();
}

void SoundMixer::setChannelVolume(PlayingSoundHandle handle, byte volume) {
	StackLock lock(_mutex);
	
	if (handle == 0)
		return;

	int index = handle - 1;

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::setChannelVolume has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->setChannelVolume(volume);
}

void SoundMixer::setChannelPan(PlayingSoundHandle handle, int8 pan) {
	StackLock lock(_mutex);
	
	if (handle == 0)
		return;

	int index = handle - 1;

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::setChannelVolume has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->setChannelPan(pan);
}

void SoundMixer::pauseAll(bool paused) {
	_paused = paused;
}

void SoundMixer::pauseChannel(int index, bool paused) {
	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::pauseChannel has invalid index %d", index);
		return;
	}

	StackLock lock(_mutex);
	if (_channels[index])
		_channels[index]->pause(paused);
}

void SoundMixer::pauseID(int id, bool paused) {
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != NULL && _channels[i]->_id == id) {
			_channels[i]->pause(paused);
			return;
		}
	}
}

void SoundMixer::pauseHandle(PlayingSoundHandle handle, bool paused) {
	StackLock lock(_mutex);

	// Simply ignore pause/unpause requests for handles of sound that alreayd terminated
	if (handle == 0)
		return;

	int index = handle - 1;

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::pauseHandle has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->pause(paused);
}

bool SoundMixer::hasActiveSFXChannel() {
	// FIXME/TODO: We need to distinguish between SFX and music channels
	// (and maybe also voice) here to work properly in iMuseDigital
	// games. In the past that was achieve using the _beginSlots hack.
	// Since we don't have that anymore, it's not that simple anymore.
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && !_channels[i]->isMusicChannel())
			return true;
	return false;
}

void SoundMixer::setVolume(int volume) {
	// Check range
	if (volume > 256)
		volume = 256;
	else if (volume < 0)
		volume = 0;

	_globalVolume = volume;
}

void SoundMixer::setMusicVolume(int volume) {
	// Check range
	if (volume > 256)
		volume = 256;
	else if (volume < 0)
		volume = 0;

	_musicVolume = volume;
}


#pragma mark -
#pragma mark --- Channel implementations ---
#pragma mark -


Channel::~Channel() {
	delete _converter;
	delete _input;
	if (_handle)
		*_handle = 0;
}

void Channel::destroy() {
	for (int i = 0; i != SoundMixer::NUM_CHANNELS; i++)
		if (_mixer->_channels[i] == this)
			_mixer->_channels[i] = 0;
	delete this;
}

/* len indicates the number of sample *pairs*. So a value of
   10 means that the buffer contains twice 10 sample, each
   16 bits, for a total of 40 bytes.
 */
void Channel::mix(int16 *data, uint len) {
	assert(_input);
	if (_input->eos()) {
		// TODO: call drain method
		destroy();
	} else {
		assert(_converter);

		// The pan value ranges from -127 to +127. That's 255 different values.
		// From the channel pan/volume and the global volume, we compute the
		// effective volume for the left and right channel.
		// Note the slightly odd divisor: the 255 reflects the fact that
		// the maximal value for _volume is 255, while the 254 is there
		// because the maximal left/right pan value is 2*127 = 254.
		// The value getVolume() returns is in the range 0 - 256.
		// Hence, the vol_l/vol_r values will be in that range, too
		
		int vol = getVolume() * _volume;
		st_volume_t vol_l = (127 - _pan) * vol / (255 * 254);
		st_volume_t vol_r = (127 + _pan) * vol / (255 * 254);

		_converter->flow(*_input, data, len, vol_l, vol_r);
	}
}

/* RAW mixer */
ChannelRaw::ChannelRaw(SoundMixer *mixer, PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, byte volume, int8 pan, int id, uint32 loopStart, uint32 loopEnd)
	: Channel(mixer, handle) {
	_id = id;
	_ptr = (byte *)sound;
	_volume = volume;
	_pan = pan;
	
	// Create the input stream
	if (flags & SoundMixer::FLAG_LOOP) {
		if (loopEnd == 0) {
			_input = makeLinearInputStream(flags, _ptr, size, 0, size);
		} else {
			assert(loopStart < loopEnd && loopEnd <= size);
			_input = makeLinearInputStream(flags, _ptr, size, loopStart, loopEnd - loopStart);
		}
	} else {
		_input = makeLinearInputStream(flags, _ptr, size, 0, 0);
	}

	// Get a rate converter instance
	_converter = makeRateConverter(rate, mixer->getOutputRate(), _input->isStereo(), (flags & SoundMixer::FLAG_REVERSE_STEREO) != 0);

	if (!(flags & SoundMixer::FLAG_AUTOFREE))
		_ptr = 0;
}

ChannelRaw::~ChannelRaw() {
	free(_ptr);
}

ChannelStream::ChannelStream(SoundMixer *mixer, PlayingSoundHandle *handle,
                             void *sound, uint32 size, uint rate,
                             byte flags, uint32 buffer_size, byte volume, int8 pan)
	: Channel(mixer, handle) {
	_volume = volume;
	_pan = pan;
	assert(size <= buffer_size);

	// Create the input stream
	_input = makeWrappedInputStream(flags, buffer_size);
	
	// Append the initial data
	((WrappedAudioInputStream *)_input)->append((const byte *)sound, size);

	// Get a rate converter instance
	_converter = makeRateConverter(rate, mixer->getOutputRate(), _input->isStereo(), (flags & SoundMixer::FLAG_REVERSE_STEREO) != 0);

	_finished = false;
}

void ChannelStream::append(void *data, uint32 len) {
	((WrappedAudioInputStream *)_input)->append((const byte *)data, len);
}

void ChannelStream::mix(int16 *data, uint len) {
	assert(_input);
	if (_input->eos()) {
		// TODO: call drain method

		// Normally, the stream stays around even if all its data is used up.
		// This is in case more data is streamed into it. To make the stream
		// go away, one can either stop() it (which takes effect immediately,
		// ignoring any remaining sound data), or finish() it, which means
		// it will finish playing before it terminates itself.
		if (_finished) {
			destroy();
		}
	} else {
		// Invoke the parent implementation.
		Channel::mix(data, len);
	}
}

#ifdef USE_MAD
ChannelMP3::ChannelMP3(SoundMixer *mixer, PlayingSoundHandle *handle, File *file, uint size, byte volume, int8 pan)
	: Channel(mixer, handle) {
	_volume = volume;
	_pan = pan;
	// Create the input stream
	_input = makeMP3Stream(file, mad_timer_zero, size);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo());
}

ChannelMP3CDMusic::ChannelMP3CDMusic(SoundMixer *mixer, PlayingSoundHandle *handle, File *file, mad_timer_t duration, byte volume, int8 pan) 
	: Channel(mixer, handle) {
	_volume = volume;
	_pan = pan;
	// Create the input stream
	_input = makeMP3Stream(file, duration, 0);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo());
}
#endif // USE_MAD

#ifdef USE_VORBIS
ChannelVorbis::ChannelVorbis(SoundMixer *mixer, PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track, byte volume, int8 pan)
	: Channel(mixer, handle) {
	_volume = volume;
	_pan = pan;
	// Create the input stream
	_input = makeVorbisStream(ov_file, duration);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo());
	_is_cd_track = is_cd_track;
}
#endif // USE_VORBIS
