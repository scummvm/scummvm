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

/*
	From Markus Magnuson (superqult) we got this information:
	Mac0
	---
	   4 bytes - 'SOUN'
	BE 4 bytes - block length

		   4 bytes  - 'Mac0'
		BE 4 bytes  - (blockLength - 27)
		   28 bytes - ???

		   do this three times (once for each channel):
			  4 bytes  - 'Chan'
		   BE 4 bytes  - channel length
			  4 bytes  - instrument name (e.g. 'MARI')

			  do this for ((chanLength-24)/4) times:
				 2 bytes  - note duration
				 1 byte   - note value
				 1 byte   - note velocity

			  4 bytes - ???
			  4 bytes - 'Loop'/'Done'
			  4 bytes - ???

	   1 byte - 0x09
	---

	The instruments presumably correspond to the snd resource names in the
	Monkey Island executable:

	Instruments
	"MARI" - MARIMBA
	"PLUC" - PLUCK
	"HARM" - HARMONIC
	"PIPE" - PIPEORGAN
	"TROM" - TROMBONE
	"STRI" - STRINGS
	"HORN" - HORN
	"VIBE" - VIBES
	"SHAK" - SHAKUHACHI
	"PANP" - PANPIPE
	"WHIS" - WHISTLE
	"ORGA" - ORGAN3
	"BONG" - BONGO
	"BASS" - BASS

	---

	Note values <= 1 are silent.
*/

#include "common/macresman.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/message.h"
#include "scumm/player_v5m.h"
#include "scumm/scumm.h"

#define RES_SND MKTAG('s', 'n', 'd', ' ')

namespace Scumm {

Player_V5M::Player_V5M(ScummEngine *scumm, Audio::Mixer *mixer)
	: _vm(scumm),
	  _mixer(mixer),
	  _sampleRate(_mixer->getOutputRate()),
	  _soundPlaying(-1) {

	assert(scumm);
	assert(mixer);
	assert(_vm->_game.id == GID_MONKEY);

	int i;

	for (i = 0; i < 3; i++) {
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

	Common::MacResManager resource;
	if (!resource.exists("Monkey Island")) {
		GUI::MessageDialog dialog(_(
			"Could not find the 'Monkey Island' Macintosh executable to read the\n"
			"instruments from. Music will be disabled."), _("OK"));
		dialog.runModal();
		return;
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_V5M::~Player_V5M() {
	Common::StackLock lock(_mutex);
	_mixer->stopHandle(_soundHandle);
	stopAllSounds_Internal();
}

void Player_V5M::setMusicVolume(int vol) {
	debug(5, "Player_V5M::setMusicVolume(%d)", vol);
}

void Player_V5M::stopAllSounds_Internal() {
	_soundPlaying = -1;
	for (int i = 0; i < 3; i++) {
		// The channel data is managed by the resource manager, so
		// don't delete that.
		delete[] _channel[i]._instrument._data;
		_channel[i]._instrument._data = NULL;

		_channel[i]._remaining = 0;
		_channel[i]._notesLeft = false;
	}
}

void Player_V5M::stopAllSounds() {
	Common::StackLock lock(_mutex);
	debug(5, "Player_V5M::stopAllSounds()");
	stopAllSounds_Internal();
}

void Player_V5M::stopSound(int nr) {
	Common::StackLock lock(_mutex);
	debug(5, "Player_V5M::stopSound(%d)", nr);

	if (nr == _soundPlaying) {
		stopAllSounds();
	}
}

void Player_V5M::startSound(int nr) {
	Common::StackLock lock(_mutex);
	debug(5, "Player_V5M::startSound(%d)", nr);

	Common::MacResManager resource;
	if (!resource.open("Monkey Island")) {
		return;
	}

	const byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	const byte *src = ptr;
	uint i;

	src += 8;
	// TODO: Decipher the unknown bytes in the header. For now, skip 'em
	src += 28;

	Common::MacResIDArray idArray = resource.getResIDArray(RES_SND);

	// Load the three channels and their instruments
	for (i = 0; i < 3; i++) {
		assert(READ_BE_UINT32(src) == MKTAG('C', 'h', 'a', 'n'));
		uint32 len = READ_BE_UINT32(src + 4);
		uint32 instrument = READ_BE_UINT32(src + 8);

		_channel[i]._length = len - 24;
		_channel[i]._data = src + 12;
		_channel[i]._looped = (READ_BE_UINT32(src + len - 8) == MKTAG('L', 'o', 'o', 'p'));
		_channel[i]._pos = 0;
		_channel[i]._pitchModifier = 0;
		_channel[i]._velocity = 0;
		_channel[i]._remaining = 0;
		_channel[i]._notesLeft = true;

		for (uint j = 0; j < idArray.size(); j++) {
			Common::String name = resource.getResName(RES_SND, idArray[j]);
			if (instrument == READ_BE_UINT32(name.c_str())) {
				debug(6, "Channel %d: Loading instrument '%s'", i, name.c_str());
				Common::SeekableReadStream *stream = resource.getResource(RES_SND, idArray[j]);

				if (!_channel[i].loadInstrument(stream)) {
					resource.close();
					return;
				}

				break;
			}
		}

		src += len;
	}

	resource.close();
	_soundPlaying = nr;
}

bool Player_V5M::Channel::loadInstrument(Common::SeekableReadStream *stream) {
	// Load the sound
	uint16 soundType = stream->readUint16BE();
	if (soundType != 1) {
		warning("Player_V5M::loadInstrument: Unsupported sound type %d", soundType);
		return false;
	}
	uint16 typeCount = stream->readUint16BE();
	if (typeCount != 1) {
		warning("Player_V5M::loadInstrument: Unsupported data type count %d", typeCount);
		return false;
	}
	uint16 dataType = stream->readUint16BE();
	if (dataType != 5) {
		warning("Player_V5M::loadInstrument: Unsupported data type %d", dataType);
		return false;
	}

	stream->readUint32BE();	// initialization option

	uint16 cmdCount = stream->readUint16BE();
	if (cmdCount != 1) {
		warning("Player_V5M::loadInstrument: Unsupported command count %d", cmdCount);
		return false;
	}
	uint16 command = stream->readUint16BE();
	if (command != 0x8050 && command != 0x8051) {
		warning("Player_V5M::loadInstrument: Unsupported command 0x%04X", command);
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
		warning("Player_V5M::loadInstrument: Unsupported encoding %d", encoding);
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

int Player_V5M::getMusicTimer() {
	return 0;
}

int Player_V5M::getSoundStatus(int nr) const {
	return _soundPlaying == nr;
}

int Player_V5M::readBuffer(int16 *data, const int numSamples) {
	Common::StackLock lock(_mutex);

	memset(data, 0, numSamples * 2);
	if (_soundPlaying == -1) {
		return numSamples;
	}

	bool notesLeft = false;

	for (int i = 0; i < 3; i++) {
		uint samplesLeft = numSamples;
		int16 *ptr = data;

		while (samplesLeft > 0) {
			int generated;
			if (_channel[i]._remaining == 0) {
				uint16 duration;
				byte note, velocity;
				if (_channel[i].getNextNote(duration, note, velocity)) {
					if (note > 1) {
						const int pitchIdx = note + 60 - _channel[i]._instrument._baseFreq;
						assert(pitchIdx >= 0);
						// I don't want to use floating-point arithmetics here,
						// but I ran into overflow problems with the church
						// music. It's only once per note, so it should be ok.
						double mult = (double)(_channel[i]._instrument._rate) / (double)_sampleRate;
						_channel[i]._pitchModifier = mult * _pitchTable[pitchIdx];
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

bool Player_V5M::Channel::getNextNote(uint16 &duration, byte &note, byte &velocity) {
	_instrument.newNote();
	if (_pos >= _length) {
		if (!_looped) {
			_notesLeft = false;
			return false;
		}
		// FIXME: Jamieson630: The jump seems to be happening
		// too quickly! There should maybe be a pause after
		// the last Note Off? But I couldn't find one in the
		// MI1 Lookout music, where I was hearing problems.
		_pos = 0;
	}
	duration = READ_BE_UINT16(&_data[_pos]);
	note = _data[_pos + 2];
	velocity = _data[_pos + 3];
	_pos += 4;
	return true;
}

void Player_V5M::Instrument::generateSamples(int16 *data, int pitchModifier, int volume, int numSamples) {
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
