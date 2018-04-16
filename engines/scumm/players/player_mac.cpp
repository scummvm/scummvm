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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/macresman.h"
#include "engines/engine.h"
#include "scumm/players/player_mac.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/imuse/imuse.h"

namespace Scumm {

Player_Mac::Player_Mac(ScummEngine *scumm, Audio::Mixer *mixer, int numberOfChannels, int channelMask, bool fadeNoteEnds)
	: _vm(scumm),
	  _mixer(mixer),
	  _sampleRate(_mixer->getOutputRate()),
	  _soundPlaying(-1),
	  _numberOfChannels(numberOfChannels),
	  _channelMask(channelMask),
	  _fadeNoteEnds(fadeNoteEnds) {
	assert(scumm);
	assert(mixer);
}

void Player_Mac::init() {
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

	if (!checkMusicAvailable()) {
		return;
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_Mac::~Player_Mac() {
	Common::StackLock lock(_mutex);
	_mixer->stopHandle(_soundHandle);
	stopAllSounds_Internal();
	delete[] _channel;
}

void syncWithSerializer(Common::Serializer &s, Player_Mac::Channel &c) {
	s.syncAsUint16LE(c._pos, VER(94));
	s.syncAsSint32LE(c._pitchModifier, VER(94));
	s.syncAsByte(c._velocity, VER(94));
	s.syncAsUint32LE(c._remaining, VER(94));
	s.syncAsByte(c._notesLeft, VER(94));
}

void syncWithSerializer(Common::Serializer &s, Player_Mac::Instrument &i) {
	s.syncAsUint32LE(i._pos, VER(94));
	s.syncAsUint32LE(i._subPos, VER(94));
}

void Player_Mac::saveLoadWithSerializer(Common::Serializer &s) {
	Common::StackLock lock(_mutex);
	if (s.getVersion() < VER(94)) {
		if (_vm->_game.id == GID_MONKEY && s.isLoading()) {
			IMuse *dummyImuse = IMuse::create(_vm->_system, NULL, NULL);
			dummyImuse->saveLoadIMuse(s, _vm, false);
			delete dummyImuse;
		}
	} else {
		uint32 mixerSampleRate = _sampleRate;
		int i;

		s.syncAsUint32LE(_sampleRate, VER(94));
		s.syncAsSint16LE(_soundPlaying, VER(94));

		if (s.isLoading() && _soundPlaying != -1) {
			const byte *ptr = _vm->getResourceAddress(rtSound, _soundPlaying);
			assert(ptr);
			loadMusic(ptr);
		}

		s.syncArray(_channel, _numberOfChannels, syncWithSerializer);
		for (i = 0; i < _numberOfChannels; i++) {
			syncWithSerializer(s, _channel[i]);
		}

		if (s.isLoading()) {
			// If necessary, adjust the channel data to fit the
			// current sample rate.
			if (_soundPlaying != -1 && _sampleRate != mixerSampleRate) {
				double mult = (double)_sampleRate / (double)mixerSampleRate;
				for (i = 0; i < _numberOfChannels; i++) {
					_channel[i]._pitchModifier = (int)((double)_channel[i]._pitchModifier * mult);
					_channel[i]._remaining = (int)((double)_channel[i]._remaining / mult);
				}
			}
			_sampleRate = mixerSampleRate;
		}
	}
}

void Player_Mac::setMusicVolume(int vol) {
	debug(5, "Player_Mac::setMusicVolume(%d)", vol);
}

void Player_Mac::stopAllSounds_Internal() {
	if (_soundPlaying != -1) {
		_vm->_res->unlock(rtSound, _soundPlaying);
	}
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

	stopAllSounds_Internal();

	const byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	if (!loadMusic(ptr)) {
		return;
	}

	_vm->_res->lock(rtSound, nr);
	_soundPlaying = nr;
}

bool Player_Mac::Channel::loadInstrument(Common::SeekableReadStream *stream) {
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

uint32 Player_Mac::durationToSamples(uint16 duration) {
	// The correct formula should be:
	//
	// (duration * 473 * _sampleRate) / (4 * 480 * 480)
	//
	// But that's likely to cause integer overflow, so we do it in two
	// steps using bitwise operations to perform
	// ((duration * 473 * _sampleRate) / 4096) without overflowing,
	// then divide this by 225
	// (note that 4 * 480 * 480 == 225 * 4096 == 225 << 12)
	//
	// The original code is a bit unclear on if it should be 473 or 437,
	// but since the comments indicated 473 I'm assuming 437 was a typo.
	uint32 samples = (duration * _sampleRate);
	samples = (samples >> 12) * 473 + (((samples & 4095) * 473) >> 12);
	samples = samples / 225;
	return samples;
}

int Player_Mac::noteToPitchModifier(byte note, Instrument *instrument) {
	if (note > 0) {
		const int pitchIdx = note + 60 - instrument->_baseFreq;
		// I don't want to use floating-point arithmetics here, but I
		// ran into overflow problems with the church music in Monkey
		// Island. It's only once per note, so it should be ok.
		double mult = (double)instrument->_rate / (double)_sampleRate;
		return (int)(mult * _pitchTable[pitchIdx]);
	} else {
		return 0;
	}
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
				uint32 samples;
				int pitchModifier;
				byte velocity;
				if (getNextNote(i, samples, pitchModifier, velocity)) {
					_channel[i]._remaining = samples;
					_channel[i]._pitchModifier = pitchModifier;
					_channel[i]._velocity = velocity;

				} else {
					_channel[i]._pitchModifier = 0;
					_channel[i]._velocity = 0;
					_channel[i]._remaining = samplesLeft;
				}
			}
			generated = MIN<uint32>(_channel[i]._remaining, samplesLeft);
			if (_channel[i]._velocity != 0) {
				_channel[i]._instrument.generateSamples(ptr, _channel[i]._pitchModifier, _channel[i]._velocity, generated, _channel[i]._remaining, _fadeNoteEnds);
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

void Player_Mac::Instrument::generateSamples(int16 *data, int pitchModifier, int volume, int numSamples, int remainingSamplesOnNote, bool fadeNoteEnds) {
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

		int newSample = (((int16)((_data[_pos] << 8) ^ 0x8000)) * volume) / 255;

		if (fadeNoteEnds) {
			// Fade out the last 100 samples on each note. Even at
			// low output sample rates this is just a fraction of a
			// second, but it gets rid of distracting "pops" at the
			// end when the sample would otherwise go abruptly from
			// something to nothing. This was particularly
			// noticeable on the distaff notes in Loom.
			//
			// The reason it's conditional is that Monkey Island
			// appears to have a "hold current note" command, and
			// if we fade out the current note in that case we
			// will actually introduce new "pops".

			remainingSamplesOnNote--;
			if (remainingSamplesOnNote < 100) {
				newSample = (newSample * remainingSamplesOnNote) / 100;
			}
		}

		int sample = *data + newSample;
		if (sample > 32767) {
			sample = 32767;
		} else if (sample < -32768) {
			sample = -32768;
		}

		*data++ = sample;
		samplesLeft--;
	}
}

} // End of namespace Scumm
