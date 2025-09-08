
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

#ifndef M4_SOUND_PLATFORM_MIDI_H
#define M4_SOUND_PLATFORM_MIDI_H

#include "m4/m4_types.h"

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"

namespace M4 {
namespace Sound {

class Midi {
private:
	static int _midiEndTrigger;

	Common::Mutex _mutex;

	MusicType _deviceType;

	MidiDriver_Multisource *_driver;
	MidiParser *_midiParser;
	byte *_midiData;

	bool _paused;

protected:
	static void onTimer(void *data);

public:
	Midi();
	~Midi();

	int open();

	void load(byte *in, int32 size);
	void play();
	void pause(bool pause);
	void stop();
	bool isPlaying();
	void startFade(uint16 duration, uint16 targetVolume);
	bool isFading();

	void syncSoundSettings();

	void midi_play(const char *name, int volume, bool loop, int trigger, int roomNum);
	void task();
	void loop();
	void midi_fade_volume(int targetVolume, int duration);
};

} // namespace Sound

void midi_play(const char *name, int volume, bool loop, int trigger, int roomNum);
void midi_loop();
void midi_stop();
void midi_fade_volume(int targetVolume, int duration);

} // namespace M4

#endif
