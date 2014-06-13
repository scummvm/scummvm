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
#include "scumm/players/player_v5m.h"
#include "scumm/scumm.h"

namespace Scumm {

Player_V5M::Player_V5M(ScummEngine *scumm, Audio::Mixer *mixer)
	: Player_Mac(scumm, mixer, 3, 0x07, false) {
	assert(_vm->_game.id == GID_MONKEY);
}

// Try both with and without underscore in the filename, because hfsutils may
// turn the space into an underscore. At least, it did for me.

static const char *monkeyIslandFileNames[] = {
	"Monkey Island",
	"Monkey_Island"
};

bool Player_V5M::checkMusicAvailable() {
	Common::MacResManager resource;

	for (int i = 0; i < ARRAYSIZE(monkeyIslandFileNames); i++) {
		if (resource.exists(monkeyIslandFileNames[i])) {
			return true;
		}
	}

	GUI::MessageDialog dialog(_(
		"Could not find the 'Monkey Island' Macintosh executable to read the\n"
		"instruments from. Music will be disabled."), _("OK"));
	dialog.runModal();
	return false;
}

bool Player_V5M::loadMusic(const byte *ptr) {
	Common::MacResManager resource;
	bool found = false;
	uint i;

	for (i = 0; i < ARRAYSIZE(monkeyIslandFileNames); i++) {
		if (resource.open(monkeyIslandFileNames[i])) {
			found = true;
			break;
		}
	}

	if (!found) {
		return false;
	}

	ptr += 8;
	// TODO: Decipher the unknown bytes in the header. For now, skip 'em
	ptr += 28;

	Common::MacResIDArray idArray = resource.getResIDArray(RES_SND);

	// Load the three channels and their instruments
	for (i = 0; i < 3; i++) {
		assert(READ_BE_UINT32(ptr) == MKTAG('C', 'h', 'a', 'n'));
		uint32 len = READ_BE_UINT32(ptr + 4);
		uint32 instrument = READ_BE_UINT32(ptr + 8);

		_channel[i]._length = len - 20;
		_channel[i]._data = ptr + 12;
		_channel[i]._looped = (READ_BE_UINT32(ptr + len - 8) == MKTAG('L', 'o', 'o', 'p'));
		_channel[i]._pos = 0;
		_channel[i]._pitchModifier = 0;
		_channel[i]._velocity = 0;
		_channel[i]._remaining = 0;
		_channel[i]._notesLeft = true;

		for (uint j = 0; j < idArray.size(); j++) {
			Common::String name = resource.getResName(RES_SND, idArray[j]);
			if (instrument == READ_BE_UINT32(name.c_str())) {
				debug(6, "Player_V5M::loadMusic: Channel %d: Loading instrument '%s'", i, name.c_str());
				Common::SeekableReadStream *stream = resource.getResource(RES_SND, idArray[j]);

				if (!_channel[i].loadInstrument(stream)) {
					resource.close();
					return false;
				}

				break;
			}
		}

		ptr += len;
	}

	resource.close();

	// The last note of each channel is just zeroes. We will adjust this
	// note so that all the channels end at the same time.

	uint32 samples[3];
	uint32 maxSamples = 0;
	for (i = 0; i < 3; i++) {
		samples[i] = 0;
		for (uint j = 0; j < _channel[i]._length; j += 4) {
			samples[i] += durationToSamples(READ_BE_UINT16(&_channel[i]._data[j]));
		}
		if (samples[i] > maxSamples) {
			maxSamples = samples[i];
		}
	}

	for (i = 0; i < 3; i++) {
		_lastNoteSamples[i] = maxSamples - samples[i];
	}

	return true;
}

bool Player_V5M::getNextNote(int ch, uint32 &samples, int &pitchModifier, byte &velocity) {
	if (_channel[ch]._pos >= _channel[ch]._length) {
		if (!_channel[ch]._looped) {
			_channel[ch]._notesLeft = false;
			return false;
		}
		// FIXME: Jamieson630: The jump seems to be happening
		// too quickly! There should maybe be a pause after
		// the last Note Off? But I couldn't find one in the
		// MI1 Lookout music, where I was hearing problems.
		_channel[ch]._pos = 0;
	}
	uint16 duration = READ_BE_UINT16(&_channel[ch]._data[_channel[ch]._pos]);
	byte note = _channel[ch]._data[_channel[ch]._pos + 2];
	samples = durationToSamples(duration);

	if (note != 1) {
		_channel[ch]._instrument.newNote();
	}

	if (note > 1) {
		pitchModifier = noteToPitchModifier(note, &_channel[ch]._instrument);
		velocity = _channel[ch]._data[_channel[ch]._pos + 3];
	} else if (note == 1) {
		// This is guesswork, but Monkey Island uses two different
		// "special" note values: 0, which is clearly a rest, and 1
		// which is... I thought at first it was a "soft" key off, to
		// fade out the note, but listening to the music in a Mac
		// emulator (which unfortunately doesn't work all that well),
		// I hear no trace of fading out.
		//
		// It could mean "change the volume on the current note", but
		// I can't hear that either, and it always seems to use the
		// exact same velocity on this note.
		//
		// So it appears it really just is a "hold the current note",
		// but why? Couldn't they just have made the original note
		// longer?

		pitchModifier = _channel[ch]._pitchModifier;
		velocity = _channel[ch]._velocity;
	} else {
		pitchModifier = 0;
		velocity = 0;
	}

	_channel[ch]._pos += 4;

	if (_channel[ch]._pos >= _channel[ch]._length) {
		samples = _lastNoteSamples[ch];
	}
	return true;
}

} // End of namespace Scumm
