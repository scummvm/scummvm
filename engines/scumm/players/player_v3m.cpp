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
	We have the following information from Lars Christensen (lechimp) and
	Jamieson Christian (jamieson630):

	RESOURCE DATA
	LE 2 bytes Resource size
	2 bytes Unknown
	2 bytes 'so'
	14 bytes Unknown
	BE 2 bytes Instrument for Stream 1
	BE 2 bytes Instrument for Stream 2
	BE 2 bytes Instrument for Stream 3
	BE 2 bytes Instrument for Stream 4
	BE 2 bytes Instrument for Stream 5
	BE 2 bytes Offset to Stream 1
	BE 2 bytes Offset to Stream 2
	BE 2 bytes Offset to Stream 3
	BE 2 bytes Offset to Stream 4
	BE 2 bytes Offset to Stream 5
	? bytes The streams

	STREAM DATA
	BE 2 bytes Unknown (always 1?)
	2 bytes Unknown (always 0?)
	BE 2 bytes Number of events in stream
	? bytes Stream data

	Each stream event is exactly 3 bytes, therefore one can
	assert that numEvents == (streamSize - 6) / 3. The
	polyphony of a stream appears to be 1; in other words, only
	one note at a time can be playing in each stream. The next
	event is not executed until the current note (or rest) is
	finished playing; therefore, note duration also serves as the
	time delta between events.

	FOR EACH EVENTS
	BE 2 bytes Note duration
	1 byte Note number to play (0 = rest/silent)

	Oh, and quick speculation -- Stream 1 may be used for a
	single-voice interleaved version of the music, where Stream 2-
	5 represent a version of the music in up to 4-voice
	polyphony, one voice per stream. I postulate thus because
	the first stream of the Mac Loom theme music contains
	interleaved voices, whereas the second stream seemed to
	contain only the pizzicato bottom-end harp. Stream 5, in this
	example, is empty, so if my speculation is correct, this
	particular musical number supports 3-voice polyphony at
	most. I must check out Streams 3 and 4 to see what they
	contain.

	==========

	The instruments appear to be identified by their resource IDs:

	1000	Dual Harp
	10895	harp1
	11445	strings1
	11548	silent
	13811	staff1
	15703	brass1
	16324	flute1
	25614	accordian 1
	28110	f horn1
	29042	bassoon1
*/

#include "common/macresman.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/message.h"
#include "scumm/players/player_v3m.h"
#include "scumm/scumm.h"

namespace Scumm {

Player_V3M::Player_V3M(ScummEngine *scumm, Audio::Mixer *mixer)
	: Player_Mac(scumm, mixer, 5, 0x1E, true) {
	assert(_vm->_game.id == GID_LOOM);

	// Channel 0 seems to be what was played on low-end macs, that couldn't
	// handle multi-channel music and play the game at the same time. I'm
	// not sure if stream 4 is ever used, but let's use it just in case.
}

// \xAA is a trademark glyph in Mac OS Roman. We try that, but also the Windows
// version, the UTF-8 version, and just plain without in case the file system
// can't handle exotic characters like that.

static const char *loomFileNames[] = {
	"Loom\xAA",
	"Loom\x99",
	"Loom\xE2\x84\xA2",
	"Loom"
};

bool Player_V3M::checkMusicAvailable() {
	Common::MacResManager resource;

	for (int i = 0; i < ARRAYSIZE(loomFileNames); i++) {
		if (resource.exists(loomFileNames[i])) {
			return true;
		}
	}

	GUI::MessageDialog dialog(_(
		"Could not find the 'Loom' Macintosh executable to read the\n"
		"instruments from. Music will be disabled."), _("OK"));
	dialog.runModal();
	return false;
}

bool Player_V3M::loadMusic(const byte *ptr) {
	Common::MacResManager resource;
	bool found = false;

	for (int i = 0; i < ARRAYSIZE(loomFileNames); i++) {
		if (resource.open(loomFileNames[i])) {
			found = true;
			break;
		}
	}

	if (!found) {
		return false;
	}

	if (ptr[4] != 's' || ptr[5] != 'o') {
		// Like the original we ignore all sound resources which do not have
		// a 'so' tag in them.
		// See bug #3602239 ("Mac Loom crashes using opening spell on
		// gravestone") for a case where this is required. Loom Mac tries to
		// play resource 11 here. This resource is no Mac sound resource
		// though, it is a PC Speaker resource. A test with the original
		// interpreter also has shown that no sound is played while the
		// screen is shaking.
		debug(5, "Player_V3M::loadMusic: Skipping unknown music type %02X%02X", ptr[4], ptr[5]);
		resource.close();
		return false;
	}

	uint i;
	for (i = 0; i < 5; i++) {
		int instrument = READ_BE_UINT16(ptr + 20 + 2 * i);
		int offset = READ_BE_UINT16(ptr + 30 + 2 * i);

		_channel[i]._looped = false;
		_channel[i]._length = READ_BE_UINT16(ptr + offset + 4) * 3;
		_channel[i]._data = ptr + offset + 6;
		_channel[i]._pos = 0;
		_channel[i]._pitchModifier = 0;
		_channel[i]._velocity = 0;
		_channel[i]._remaining = 0;
		_channel[i]._notesLeft = true;

		Common::SeekableReadStream *stream = resource.getResource(RES_SND, instrument);
		if (_channel[i].loadInstrument(stream)) {
			debug(6, "Player_V3M::loadMusic: Channel %d - Loaded Instrument %d (%s)", i, instrument, resource.getResName(RES_SND, instrument).c_str());
		} else {
			resource.close();
			return false;
		}
	}

	resource.close();
	return true;
}

bool Player_V3M::getNextNote(int ch, uint32 &samples, int &pitchModifier, byte &velocity) {
	_channel[ch]._instrument.newNote();
	if (_channel[ch]._pos >= _channel[ch]._length) {
		if (!_channel[ch]._looped) {
			_channel[ch]._notesLeft = false;
			return false;
		}
		_channel[ch]._pos = 0;
	}
	uint16 duration = READ_BE_UINT16(&_channel[ch]._data[_channel[ch]._pos]);
	byte note = _channel[ch]._data[_channel[ch]._pos + 2];
	samples = durationToSamples(duration);
	if (note > 0) {
		pitchModifier = noteToPitchModifier(note, &_channel[ch]._instrument);
		velocity = 127;
	} else {
		pitchModifier = 0;
		velocity = 0;
	}
	_channel[ch]._pos += 3;
	return true;
}

} // End of namespace Scumm
