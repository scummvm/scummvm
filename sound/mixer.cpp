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
#include "sound/mp3.h"
#include "sound/vorbis.h"


#pragma mark -
#pragma mark --- Channel classes ---
#pragma mark -


/**
 * Channels used by the sound mixer.
 */
class Channel {
private:
	SoundMixer *_mixer;
	PlayingSoundHandle *_handle;
	bool _autofreeStream;
	const bool _isMusic;
	byte _volume;
	int8 _pan;
	bool _paused;
	int _id;

protected:
	RateConverter *_converter;
	AudioInputStream *_input;

public:

	Channel(SoundMixer *mixer, PlayingSoundHandle *handle, bool isMusic, byte volume, int8 pan, int id = -1);
	Channel(SoundMixer *mixer, PlayingSoundHandle *handle, AudioInputStream *input, bool autofreeStream, bool isMusic, byte volume, int8 pan, bool reverseStereo = false, int id = -1);
	virtual ~Channel();

	void mix(int16 *data, uint len);

	bool isFinished() const {
		return _input->endOfStream();
	}
	bool isMusicChannel() const {
		return _isMusic;
	}
	void pause(bool paused) {
		_paused = paused;
	}
	bool isPaused() {
		return _paused;
	}
	void setChannelVolume(const byte volume) {
		_volume = volume;
	}
	void setChannelPan(const int8 pan) {
		_pan = pan;
	}
	int getVolume() const {
		return isMusicChannel() ? _mixer->getMusicVolume() : _mixer->getVolume();
	}
	int getId() const {
		return _id;
	}
};

class ChannelStream : public Channel {
public:
	ChannelStream(SoundMixer *mixer, PlayingSoundHandle *handle, uint rate, byte flags, uint32 buffer_size, byte volume, int8 pan);
	void append(void *sound, uint32 size);

	void finish();
};

#pragma mark -
#pragma mark --- SoundMixer ---
#pragma mark -


SoundMixer::SoundMixer() {
	_syst = OSystem::instance();
	_mutex = _syst->create_mutex();

	_premixParam = 0;
	_premixProc = 0;
	int i = 0;

	_outputRate = (uint) _syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	if (_outputRate == 0)
		error("OSystem returned invalid sample rate");

	_globalVolume = 0;
	_musicVolume = 0;

	_paused = false;
	
	for (i = 0; i != NUM_CHANNELS; i++)
		_channels[i] = 0;

	_mixerReady = _syst->set_sound_proc(mixCallback, this, OSystem::SOUND_16BIT);
}

SoundMixer::~SoundMixer() {
	_syst->clear_sound_proc();
	for (int i = 0; i != NUM_CHANNELS; i++) {
		delete _channels[i];
	}
	_syst->delete_mutex(_mutex);
}

void SoundMixer::setupPremix(PremixProc *proc, void *param) {
	Common::StackLock lock(_mutex);
	_premixParam = param;
	_premixProc = proc;
}

void SoundMixer::newStream(PlayingSoundHandle *handle, uint rate, byte flags, uint32 buffer_size, byte volume, int8 pan) {
	Common::StackLock lock(_mutex);
	insertChannel(handle, new ChannelStream(this, handle, rate, flags, buffer_size, volume, pan));
}

void SoundMixer::appendStream(PlayingSoundHandle handle, void *sound, uint32 size) {
	Common::StackLock lock(_mutex);
	
	if (!handle.isActive())
		return;

	int index = handle.getIndex();

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
	Common::StackLock lock(_mutex);

	// Simply ignore stop requests for handles of sounds that already terminated
	if (!handle.isActive())
		return;

	int index = handle.getIndex();

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

void SoundMixer::insertChannel(PlayingSoundHandle *handle, Channel *chan) {

	int index = -1;
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == 0) {
			index = i;
			break;
		}
	}
	if(index == -1) {
		warning("SoundMixer::out of mixer slots");
		delete chan;
		return;
	}

	_channels[index] = chan;
	if (handle)
		handle->setIndex(index);
}

void SoundMixer::playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, int id, byte volume, int8 pan, uint32 loopStart, uint32 loopEnd) {
	Common::StackLock lock(_mutex);

	// Prevent duplicate sounds
	if (id != -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] != 0 && _channels[i]->getId() == id) {
				if ((flags & SoundMixer::FLAG_AUTOFREE) != 0)
					free(sound);
				return;
			}
	}

	// Create the input stream
	AudioInputStream *input;
	if (flags & SoundMixer::FLAG_LOOP) {
		if (loopEnd == 0) {
			input = makeLinearInputStream(rate, flags, (byte *)sound, size, 0, size);
		} else {
			assert(loopStart < loopEnd && loopEnd <= size);
			input = makeLinearInputStream(rate, flags, (byte *)sound, size, loopStart, loopEnd - loopStart);
		}
	} else {
		input = makeLinearInputStream(rate, flags, (byte *)sound, size, 0, 0);
	}

	// Create the channel
	Channel *chan = new Channel(this, handle, input, true, false, volume, pan, (flags & SoundMixer::FLAG_REVERSE_STEREO) != 0, id);
	insertChannel(handle, chan);
}

#ifdef USE_MAD
void SoundMixer::playMP3(PlayingSoundHandle *handle, File *file, uint32 size, byte volume, int8 pan, int id) {
	// Create the input stream
	AudioInputStream *input = makeMP3Stream(file, mad_timer_zero, size);
	playInputStream(handle, input, false, volume, pan, id);
}
void SoundMixer::playMP3CDTrack(PlayingSoundHandle *handle, File *file, mad_timer_t duration, byte volume, int8 pan, int id) {
	// Create the input stream
	AudioInputStream *input = makeMP3Stream(file, duration, 0);
	playInputStream(handle, input, true, volume, pan, id);
}
#endif

#ifdef USE_VORBIS
void SoundMixer::playVorbis(PlayingSoundHandle *handle, File *file, uint32 size, byte volume, int8 pan, int id) {
	// Create the input stream
	AudioInputStream *input = makeVorbisStream(file, size);
	playInputStream(handle, input, false, volume, pan, id);
}
void SoundMixer::playVorbis(PlayingSoundHandle *handle, OggVorbis_File *ov_file, int duration, bool is_cd_track, byte volume, int8 pan, int id) {
	// Create the input stream
	AudioInputStream *input = makeVorbisStream(ov_file, duration);
	playInputStream(handle, input, is_cd_track, volume, pan, id);
}
#endif

void SoundMixer::playInputStream(PlayingSoundHandle *handle, AudioInputStream *input, bool isMusic, byte volume, int8 pan, int id, bool autofreeStream) {
	Common::StackLock lock(_mutex);

	if (input == 0) {
		warning("input stream is 0");
		return;
	}

	// Prevent duplicate sounds
	if (id != -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] != 0 && _channels[i]->getId() == id) {
				if (autofreeStream)
					delete input;
				return;
			}
	}

	// Create the channel
	Channel *chan = new Channel(this, handle, input, autofreeStream, isMusic, volume, pan, false, id);
	insertChannel(handle, chan);
}

void SoundMixer::mix(int16 *buf, uint len) {
#ifndef __PALM_OS__
	Common::StackLock lock(_mutex);
#endif

	//  zero the buf
	memset(buf, 0, 2 * len * sizeof(int16));

	if (!_paused) {
		if (_premixProc)
			_premixProc(_premixParam, buf, len);

		// now mix all channels
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i]) {
				if (_channels[i]->isFinished()) {
					delete _channels[i];
					_channels[i] = 0;
				} else if (!_channels[i]->isPaused())
					_channels[i]->mix(buf, len);
			}
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
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] != 0) {
			delete _channels[i];
			_channels[i] = 0;
		}
}

void SoundMixer::stopID(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void SoundMixer::stopHandle(PlayingSoundHandle handle) {
	Common::StackLock lock(_mutex);

	// Simply ignore stop requests for handles of sounds that already terminated
	if (!handle.isActive())
		return;

	int index = handle.getIndex();

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::stopHandle has invalid index %d", index);
		return;
	}

	if (_channels[index]) {
		delete _channels[index];
		_channels[index] = 0;
	}
}

void SoundMixer::setChannelVolume(PlayingSoundHandle handle, byte volume) {
	Common::StackLock lock(_mutex);

	if (!handle.isActive())
		return;

	int index = handle.getIndex();

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::setChannelVolume has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->setChannelVolume(volume);
}

void SoundMixer::setChannelPan(PlayingSoundHandle handle, int8 pan) {
	Common::StackLock lock(_mutex);

	if (!handle.isActive())
		return;

	int index = handle.getIndex();

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

void SoundMixer::pauseID(int id, bool paused) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			_channels[i]->pause(paused);
			return;
		}
	}
}

void SoundMixer::pauseHandle(PlayingSoundHandle handle, bool paused) {
	Common::StackLock lock(_mutex);

	// Simply ignore pause/unpause requests for handles of sound that alreayd terminated
	if (!handle.isActive())
		return;

	int index = handle.getIndex();

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
	Common::StackLock lock(_mutex);
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


Channel::Channel(SoundMixer *mixer, PlayingSoundHandle *handle, bool isMusic,
				byte volume, int8 pan, int id)
	: _mixer(mixer), _handle(handle), _autofreeStream(true), _isMusic(isMusic),
	  _volume(volume), _pan(pan), _paused(false), _id(id), _converter(0), _input(0) {
	assert(mixer);
}

Channel::Channel(SoundMixer *mixer, PlayingSoundHandle *handle, AudioInputStream *input,
				bool autofreeStream, bool isMusic, byte volume, int8 pan, bool reverseStereo, int id)
	: _mixer(mixer), _handle(handle), _autofreeStream(autofreeStream), _isMusic(isMusic),
	  _volume(volume), _pan(pan), _paused(false), _id(id), _converter(0), _input(input) {
	assert(mixer);
	assert(input);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo(), reverseStereo);
}

Channel::~Channel() {
	delete _converter;
	if (_autofreeStream)
		delete _input;
	if (_handle)
		_handle->resetIndex();
}

/* len indicates the number of sample *pairs*. So a value of
   10 means that the buffer contains twice 10 sample, each
   16 bits, for a total of 40 bytes.
 */
void Channel::mix(int16 *data, uint len) {
	assert(_input);

	if (_input->endOfData()) {
		// TODO: call drain method
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

ChannelStream::ChannelStream(SoundMixer *mixer, PlayingSoundHandle *handle,
							uint rate, byte flags, uint32 buffer_size, byte volume, int8 pan)
	: Channel(mixer, handle, true, volume, pan) {
	// Create the input stream
	_input = makeWrappedInputStream(rate, flags, buffer_size);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo(), (flags & SoundMixer::FLAG_REVERSE_STEREO) != 0);
}

void ChannelStream::finish() {
	((WrappedAudioInputStream *)_input)->finish();
}

void ChannelStream::append(void *data, uint32 len) {
	((WrappedAudioInputStream *)_input)->append((const byte *)data, len);
}
