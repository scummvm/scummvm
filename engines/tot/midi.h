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

#ifndef TOT_MIDI_H
#define TOT_MIDI_H

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"

namespace Tot {

class MidiPlayer {
protected:
	MidiDriver_Multisource *_driver;
	MidiParser *_parserMusic;
	byte *_musicData;

	bool _paused;

	static void onTimer(void *data);

public:
	MidiPlayer();
	~MidiPlayer();

	// Creates and opens the relevant parsers and drivers
	int open();
	// Loads music or SFX data supported by the MidiParser
	void load(Common::SeekableReadStream *in, int32 size = -1);

	void play(int track);

	void setLoop(bool loop);

	bool isPlaying();
	void stop();
	void pause(bool b);
	void syncSoundSettings();

	void setSourceVolume(uint8 volume);
	void startFadeOut();
	void startFadeIn();
	bool isFading();
};

} // End of namespace Tot

#endif
