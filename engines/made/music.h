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

// Music class

#ifndef MADE_MUSIC_H
#define MADE_MUSIC_H

#include "made.h"

#include "audio/adlib_ms.h"
#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"
#include "audio/mt32gm.h"
#include "audio/midiparser.h"

namespace Made {

class GenericResource;

class MusicPlayer {
private:
	static const uint8 MT32_GOODBYE_MSG[MidiDriver_MT32GM::MT32_DISPLAY_NUM_CHARS];

public:
	MusicPlayer(MadeEngine *vm, bool milesAudio);
	~MusicPlayer();

	void close();

	void playXMIDI(GenericResource *midiResource);
	void playSMF(GenericResource *midiResource);
	void stop();
	void pause();
	void resume();

	bool isPlaying();
	void syncSoundSettings();

private:
	MadeEngine *_vm;
	MidiParser *_parser;
	MidiDriver_Multisource *_driver;

	MusicType _driverType;

	static void timerCallback(void *refCon);
	void onTimer();
};

class MidiDriver_ADLIB_MADE : public MidiDriver_ADLIB_Multisource {
public:
	MidiDriver_ADLIB_MADE(OPL::Config::OplType oplType);

	// TODO Implement AdLib driver logic for Manhole / LGoP2
};

} // End of namespace Made

#endif
