/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/macresman.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/message.h"
#include "scumm/player_mac.h"
#include "scumm/scumm.h"

namespace Scumm {

Player_Mac::Player_Mac(ScummEngine *scumm, Audio::Mixer *mixer, int numberOfChannels, int channelMask)
	: _vm(scumm),
	  _mixer(mixer),
	  _sampleRate(_mixer->getOutputRate()),
	  _soundPlaying(-1),
	  _numberOfChannels(numberOfChannels),
	  _channelMask(channelMask) {
	assert(scumm);
	assert(mixer);
}

void Player_Mac::init() {
	if (!checkMusicAvailable()) {
		_channel = NULL;
		return;
	}

	_channel = new Player_Mac::Channel[_numberOfChannels];

	int i;

	for (i = 0; i < _numberOfChannels; i++) {
		_channel[i]._looped = false;
		_channel[i]._length = 0;
		_channel[i]._data = NULL;
		_channel[i]._pos = 0;
		_channel[i]._pitchModifier = 0;
		_channel[i]._velocity = 0;
		_channel[i]._remaining = 0;
		_channel[i]._notesLeft = false;
		_channel[i]._instrument._data = NULL;
		_channel[i]._instrument._size = 0;
		_channel[i]._instrument._rate = 0;
		_channel[i]._instrument._loopStart = 0;
		_channel[i]._instrument._loopEnd = 0;
		_channel[i]._instrument._baseFreq = 0;
		_channel[i]._instrument._pos = 0;
		_channel[i]._instrument._subPos = 0;
	}

	_pitchTable[116] = 1664510;
	_pitchTable[117] = 1763487;
	_pitchTable[118] = 1868350;
	_pitchTable[119] = 1979447;
	_pitchTable[120] = 2097152;
	_pitchTable[121] = 2221855;
	_pitchTable[122] = 2353973;
	_pitchTable[123] = 2493948;
	_pitchTable[124] = 2642246;
	_pitchTable[125] = 2799362;
	_pitchTable[126] = 2965820;
	_pitchTable[127] = 3142177;
	for (i = 115; i >= 0; --i) {
		_pitchTable[i] = _pitchTable[i + 12] / 2;
	}

	setMusicVolume(255);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_Mac::~Player_Mac() {
	Common::StackLock lock(_mutex);
	_mixer->stopHandle(_soundHandle);
	stopAllSounds_Internal();
	delete[] _channel;
}

void Player_Mac::setMusicVolume(int vol) {
	debug(5, "Player_Mac::setMusicVolume(%d)", vol);
}

void Player_Mac::stopAllSounds_Internal() {
	_soundPlaying = -1;
	for (int i = 0; i < _numberOfChannels; i++) {
		// The channel data is managed by the resource manager, so
		// don't delete that.
		delete[] _channel[i]._instrument._data;
		_channel[i]._instrument._data = NULL;

		_channel[i]._remaining = 0;
		_channel[i]._notesLeft = false;
	}
}

void Player_Mac::stopAllSounds() {
	Common::StackLock lock(_mutex);
	debug(5, "Player_Mac::stopAllSounds()");
	stopAllSounds_Internal();
}

void Player_Mac::stopSound(int nr) {
	Common::StackLock lock(_mutex);
	debug(5, "Player_Mac::stopSound(%d)", nr);

	if (nr == _soundPlaying) {
		stopAllSounds();
	}
}

void Player_Mac::startSound(int nr) {
	Common::StackLock lock(_mutex);
	debug(5, "Player_Mac::startSound(%d)", nr);

	const byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	if (!loadMusic(ptr)) {
		return;
	}

	_soundPlaying = nr;
}

bool Player_Mac::Channel::loadInstrument(Common::SeekableReadStream *stream) {
	// Load the sound
	uint16 soundType = stream->readUint16BE();
	if (soundType != 1) {
		warning("Player_Mac::loadInstrument: Unsupported sound type %d", soundType);
		return false;
	}
	uint16 typeCount = stream->readUint16BE();
	if (typeCount != 1) {
		warning("Player_Mac::loadInstrument: Unsupported data type count %d", typeCount);
		return false;
	}
	uint16 dataType = stream->readUint16BE();
	if (dataType != 5) {
		warning("Player_Mac::loadInstrument: Unsupported data type %d", dataType);
		return false;
	}

	stream->readUint32BE();	// initialization option

	uint16 cmdCount = stream->readUint16BE();
	if (cmdCount != 1) {
		warning("Player_Mac::loadInstrument: Unsupported command count %d", cmdCount);
		return false;
	}
	uint16 command = stream->readUint16BE();
	if (command != 0x8050 && command != 0x8051) {
		warning("Player_Mac::loadInstrument: Unsupported command 0x%04X", command);
		return false;
	}

	stream->readUint16BE(); // 0
	uint32 soundHeaderOffset = stream->readUint32BE();

	stream->seek(soundHeaderOffset);

	uint32 soundDataOffset = stream->readUint32BE();
	uint32 size = stream->readUint32BE();
	uint32 rate = stream->readUint32BE() >> 16;
	uint32 loopStart = stream->readUint32BE();
	uint32 loopEnd = stream->readUint32BE();
	byte encoding = stream->readByte();
	byte baseFreq = stream->readByte();

	if (encoding != 0) {
		warning("Player_Mac::loadInstrument: Unsupported encoding %d", encoding);
		return false;
	}

	stream->skip(soundDataOffset);

	byte *data = new byte[size];
	stream->read(data, size);

	_instrument._data = data;
	_instrument._size = size;
	_instrument._rate = rate;
	_instrument._loopStart = loopStart;
	_instrument._loopEnd = loopEnd;
	_instrument._baseFreq = baseFreq;

	return true;
}

int Player_Mac::getMusicTimer() {
	return 0;
}

int Player_Mac::getSoundStatus(int nr) const {
	return _soundPlaying == nr;
}

int Player_Mac::readBuffer(int16 *data, const int numSamples) {
	Common::StackLock lock(_mutex);

	memset(data, 0, numSamples * 2);
	if (_soundPlaying == -1) {
		return numSamples;
	}

	bool notesLeft = false;

	for (int i = 0; i < _numberOfChannels; i++) {
		if (!(_channelMask & (1 << i))) {
			continue;
		}

		uint samplesLeft = numSamples;
		int16 *ptr = data;

		while (samplesLeft > 0) {
			int generated;
			if (_channel[i]._remaining == 0) {
				uint16 duration;
				byte note, velocity;
				if (getNextNote(i, duration, note, velocity)) {
					if (note > 1) {
						const int pitchIdx = note + 60 - _channel[i]._instrument._baseFreq;
						assert(pitchIdx >= 0);
						// I don't want to use floating-point arithmetics here,
						// but I ran into overflow problems with the church
						// music. It's only once per note, so it should be ok.
						double mult = (double)(_channel[i]._instrument._rate) / (double)_sampleRate;
						_channel[i]._pitchModifier = (int)(mult * _pitchTable[pitchIdx]);
						_channel[i]._velocity = velocity;
					} else {
						_channel[i]._pitchModifier = 0;
						_channel[i]._velocity = 0;
					}

					// The correct formula should be:
					//
					// (duration * 473 * _sampleRate) / (4 * 480 * 480)
					//
					// But that's likely to cause integer overflow, so
					// we do it in two steps and hope that the rounding
					// error won't be noticeable.
					//
					// The original code is a bit unclear on if it should
					// be 473 or 437, but since the comments indicated
					// 473 I'm assuming 437 was a typo.
					_channel[i]._remaining = (duration * _sampleRate) / (4 * 480);
					_channel[i]._remaining = (_channel[i]._remaining * 473) / 480;
				} else {
					_channel[i]._pitchModifier = 0;
					_channel[i]._velocity = 0;
					_channel[i]._remaining = samplesLeft;
				}
			}
			generated = MIN(_channel[i]._remaining, samplesLeft);
			if (_channel[i]._velocity != 0) {
				_channel[i]._instrument.generateSamples(ptr, _channel[i]._pitchModifier, _channel[i]._velocity, generated);
			}
			ptr += generated;
			samplesLeft -= generated;
			_channel[i]._remaining -= generated;
		}

		if (_channel[i]._notesLeft) {
			notesLeft = true;
		}
	}

	if (!notesLeft) {
		stopAllSounds_Internal();
	}

	return numSamples;
}

void Player_Mac::Instrument::generateSamples(int16 *data, int pitchModifier, int volume, int numSamples) {
	int samplesLeft = numSamples;
	while (samplesLeft) {
		_subPos += pitchModifier;
		while (_subPos >= 0x10000) {
			_subPos -= 0x10000;
			_pos++;
			if (_pos >= _loopEnd) {
				_pos = _loopStart;
			}
		}

		int sample = *data + ((_data[_pos] - 129) * 128 * volume) / 255;
		if (sample > 32767) {
			sample = 32767;
		} else if (sample < -32768) {
			sample = -32768;
		}

		*data++ = sample; // (_data[_pos] * 127) / 100;
		samplesLeft--;
	}
}

} // End of namespace Scumm
