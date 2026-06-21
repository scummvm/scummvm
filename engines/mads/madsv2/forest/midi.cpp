/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "audio/midiparser_smf.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/forest/forest.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

void MidiPlayer::send(uint32 b) {
	//_driver->send(b);
	Audio::MidiPlayer::send(b);
}

void MidiPlayer::play(const char *name) {
	// Open up the MIDI file
	Common::SeekableReadStream *f = env_open(name);
	if (!f) {
		warning("MIDI not found - %s", name);
		return;
	}

#if 0
	// Read in the file contents
	size_t size = f->size();
	byte *buf = (byte *)malloc(size);
	f->read(buf, size);
	delete f;

	// Set up the parser
	_parser = new MidiParser_SMF();
	if (!_parser->loadMusic(buf, size))
		error("midiPlay() couldn't load music resource");

	_parser->setTrack(0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
#else
	warning("TODO: Support hmi midi playback");
	delete f;
#endif
}

void midi_play(const char *name) {
	g_engine->_midiPlayer.play(name);
}

void midi_stop() {
	g_engine->_midiPlayer.stop();
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
