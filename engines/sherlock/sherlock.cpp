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

#include "sherlock/sherlock.h"
#include "sherlock/graphics.h"
#include "common/scummsys.h"
#include "common/debug-channels.h"
#include "engines/util.h"

namespace Sherlock {

SherlockEngine::SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc) {
	_animation = nullptr;
	_debugger = nullptr;
	_events = nullptr;
	_journal = nullptr;
	_res = nullptr;
	_rooms = nullptr;
	_screen = nullptr;
	_sound = nullptr;
	_talk = nullptr;
	_useEpilogue2 = false;
}


SherlockEngine::~SherlockEngine() {
	delete _animation;
	delete _debugger;
	delete _events;
	delete _journal;
	delete _res;
	delete _rooms;
	delete _screen;
	delete _sound;
	delete _talk;
}

void SherlockEngine::initialize() {
	initGraphics(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, false);

	DebugMan.addDebugChannel(kDebugScript, "scripts", "Script debug level");

	/*
	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_midi = new MidiPlayer(this, driver);
	_midi->setGM(true);
	_midi->setNativeMT32(native_mt32);
	*/

	_animation = new Animation(this);
	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_journal = new Journal();
	_res = new Resources();
	_rooms = new Rooms();
	_screen = new Screen(this);
	_talk = new Talk();
}

Common::Error SherlockEngine::run() {
	initialize();

	showOpening();

	// TODO: Rest of game
	
	return Common::kNoError;
}

} // End of namespace Comet
