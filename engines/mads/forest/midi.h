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

#ifndef MADS_FOREST_MIDI_H
#define MADS_FOREST_MIDI_H

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"

namespace MADS {
namespace Forest {

extern bool midi_playing;

class MidiPlayer {
public:
	MidiPlayer();
	~MidiPlayer();

	int open();
	void load(Common::SeekableReadStream *in, int64 size = -1);
	void play();
	void pause(bool pause);
	void stop();

	void setLoop(bool loop);
	bool isPlaying();

	void syncSoundSettings();

private:
	Common::Mutex _mutex;
	MidiDriver_Multisource *_driver;
	MusicType _deviceType;

	MidiParser *_parser;
	byte *_data;

	bool _paused;

protected:
	static void onTimer(void *data);
};

extern void midi_play(const char *name);
extern void midi_stop();
extern void midi_loop();

} // namespace Forest
} // namespace MADS

#endif
