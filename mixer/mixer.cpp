// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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

#include "../stdafx.h"
#include "../bits.h"
#include "../debug.h"

#include "mixer.h"
#include "rate.h"
#include "audiostream.h"

SoundMixer *g_mixer = NULL;

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
	int8 _balance;
	bool _paused;
	int _id;

protected:
	RateConverter *_converter;
	AudioStream *_input;

public:

	Channel(SoundMixer *mixer, PlayingSoundHandle *handle, bool isMusic, int id = -1);
	Channel(SoundMixer *mixer, PlayingSoundHandle *handle, AudioStream *input, bool autofreeStream, bool isMusic, bool reverseStereo = false, int id = -1);
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
	void setVolume(const byte volume) {
		_volume = volume;
	}
	void setBalance(const int8 balance) {
		_balance = balance;
	}
	int getId() const {
		return _id;
	}
};

class ChannelStream : public Channel {
public:
	ChannelStream(SoundMixer *mixer, PlayingSoundHandle *handle, uint rate, byte flags, uint32 buffer_size);
	void append(void *sound, uint32 size);

	void finish();
};

SoundMixer::SoundMixer() {
	_mutex = create_mutex();
	_premixParam = NULL;
	_premixProc = NULL;
	_outputRate = 22050;
	_globalVolume = 0;
	_paused = false;

	for (int i = 0; i != NUM_CHANNELS; i++)
		_channels[i] = NULL;

	_mixerReady = setSoundProc(mixCallback, this);
}

SoundMixer::~SoundMixer() {
	SDL_CloseAudio();
	stopAll();
	delete_mutex(_mutex);
}

bool SoundMixer::setSoundProc(SoundProc proc, void *param) {
	SDL_AudioSpec desired;

	memset(&desired, 0, sizeof(desired));

	/* only one format supported at the moment */
	desired.freq = 22050;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = proc;
	desired.userdata = param;

	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}

	SDL_PauseAudio(0);
	return true;
}

void SoundMixer::setupPremix(PremixProc *proc, void *param) {
	StackLock lock(_mutex);
	_premixParam = param;
	_premixProc = proc;
}

void SoundMixer::newStream(PlayingSoundHandle *handle, uint rate, byte flags, uint32 buffer_size, byte volume, int8 balance) {
	StackLock lock(_mutex);

	Channel *chan = new ChannelStream(this, handle, rate, flags, buffer_size);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void SoundMixer::appendStream(PlayingSoundHandle handle, void *sound, uint32 size) {
	StackLock lock(_mutex);

	if (!handle.isActive())
		return;

	int index = handle.getIndex();

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::appendStream has invalid index %d", index);
		return;
	}

	ChannelStream *chan;
	chan = dynamic_cast<ChannelStream *>(_channels[index]);
	if (!chan) {
		error("Trying to append to nonexistant stream : %d", index);
	} else {
		chan->append(sound, size);
	}
}

void SoundMixer::endStream(PlayingSoundHandle handle) {
	StackLock lock(_mutex);

	// Simply ignore stop requests for handles of sounds that already terminated
	if (!handle.isActive())
		return;

	int index = handle.getIndex();

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::endStream has invalid index %d", index);
		return;
	}

	ChannelStream *chan;
	chan = dynamic_cast<ChannelStream *>(_channels[index]);
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

void SoundMixer::playRaw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags, int id, byte volume, int8 balance, uint32 loopStart, uint32 loopEnd) {
	StackLock lock(_mutex);

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
	AudioStream *input;
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
	Channel *chan = new Channel(this, handle, input, true, false, (flags & SoundMixer::FLAG_REVERSE_STEREO) != 0, id);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void SoundMixer::playInputStream(PlayingSoundHandle *handle, AudioStream *input, bool isMusic, byte volume, int8 balance, int id, bool autofreeStream) {
	StackLock lock(_mutex);

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
	Channel *chan = new Channel(this, handle, input, autofreeStream, isMusic, false, id);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void SoundMixer::mix(int16 *buf, uint len) {
	StackLock lock(_mutex);

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
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] != 0) {
			delete _channels[i];
			_channels[i] = 0;
		}
}

void SoundMixer::stopID(int id) {
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void SoundMixer::stopHandle(PlayingSoundHandle handle) {
	StackLock lock(_mutex);

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
	StackLock lock(_mutex);

	if (!handle.isActive())
		return;

	int index = handle.getIndex();

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::setChannelVolume has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->setVolume(volume);
}

void SoundMixer::setChannelBalance(PlayingSoundHandle handle, int8 balance) {
	StackLock lock(_mutex);

	if (!handle.isActive())
		return;

	int index = handle.getIndex();

	if ((index < 0) || (index >= NUM_CHANNELS)) {
		warning("soundMixer::setChannelBalance has invalid index %d", index);
		return;
	}

	if (_channels[index])
		_channels[index]->setBalance(balance);
}

void SoundMixer::pauseAll(bool paused) {
	_paused = paused;
}

void SoundMixer::pauseID(int id, bool paused) {
	StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			_channels[i]->pause(paused);
			return;
		}
	}
}

void SoundMixer::pauseHandle(PlayingSoundHandle handle, bool paused) {
	StackLock lock(_mutex);

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

void SoundMixer::setVolume(int volume) {
	// Check range
	if (volume > 256)
		volume = 256;
	else if (volume < 0)
		volume = 0;

	_globalVolume = volume;
}

Channel::Channel(SoundMixer *mixer, PlayingSoundHandle *handle, bool isMusic, int id)
	: _mixer(mixer), _handle(handle), _autofreeStream(true), _isMusic(isMusic),
	  _volume(255), _balance(0), _paused(false), _id(id), _converter(0), _input(0) {
	assert(mixer);
}

Channel::Channel(SoundMixer *mixer, PlayingSoundHandle *handle, AudioStream *input,
				bool autofreeStream, bool isMusic, bool reverseStereo, int id)
	: _mixer(mixer), _handle(handle), _autofreeStream(autofreeStream), _isMusic(isMusic),
	  _volume(255), _balance(0), _paused(false), _id(id), _converter(0), _input(input) {
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

		// From the channel balance/volume and the global volume, we compute
		// the effective volume for the left and right channel. Note the
		// slightly odd divisor: the 255 reflects the fact that the maximal
		// value for _volume is 255, while the 127 is there because the
		// balance value ranges from -127 to 127.  The mixer (music/sound)
		// volume is in the range 0 - 256.
		// Hence, the vol_l/vol_r values will be in that range, too
		
		int vol = _mixer->getVolume() * _volume;
		st_volume_t vol_l, vol_r;

		if (_balance == 0) {
			vol_l = vol / 255;
			vol_r = vol / 255;
		} else if (_balance < 0) {
			vol_l = vol / 255;
			vol_r = ((127 + _balance) * vol) / (255 * 127);
		} else {
			vol_l = ((127 - _balance) * vol) / (255 * 127);
			vol_r = vol / 255;
		}

		_converter->flow(*_input, data, len, vol_l, vol_r);
	}
}

ChannelStream::ChannelStream(SoundMixer *mixer, PlayingSoundHandle *handle,
							uint rate, byte flags, uint32 buffer_size)
	: Channel(mixer, handle, true) {
	// Create the input stream
	_input = makeAppendableAudioStream(rate, flags, buffer_size);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo(), (flags & SoundMixer::FLAG_REVERSE_STEREO) != 0);
}

void ChannelStream::finish() {
	((AppendableAudioStream *)_input)->finish();
}

void ChannelStream::append(void *data, uint32 len) {
	((AppendableAudioStream *)_input)->append((const byte *)data, len);
}
