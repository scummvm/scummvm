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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"
#include "access/martian/martian_tunnel.h"
#include "access/martian/martian_scripts.h"

namespace Access {

namespace Martian {

MartianScripts::MartianScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (MartianEngine *)_vm;
	_tunnel = new MartianTunnel(_game);
}

MartianScripts::~MartianScripts() {
	delete _tunnel;
}

void MartianScripts::cmdSpecial0() {
	// Abduction scene
	_vm->_sound->stopSound();
	_vm->_midi->stopSong();

	_vm->_midi->loadMusic(47, 1);
	_vm->_midi->midiPlay();
	_vm->_midi->setLoop(true);

	_vm->_screen->forceFadeOut();
	_vm->_files->loadScreen("HOUSE.SC");

	_vm->_video->setVideo(_vm->_screen, Common::Point(46, 30), "HVID.VID", 20);

	_vm->_events->hideCursor();
	_vm->_events->_vbCount = 300;
	while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
		_vm->_events->pollEventsAndWait();

	do {
		_vm->_video->playVideo();
		_vm->_events->pollEvents();
		if (_vm->_video->_videoFrame == 4) {
			_vm->_screen->flashPalette(16);
			_vm->_sound->playSound(4);
			do {
				_vm->_events->pollEvents();
			} while (!_vm->shouldQuit() && _vm->_sound->isSFXPlaying());
			//
			// TODO:
			//
			// The original sets these, probably to hold the video until the sound finishes?
			// But, if we do that they never get past frame 4, because in the next iteration
			// of the loop the timer is still "counting down", so we stay on frame 4 forever.
			//
			// Need to double-check the exact flow of the original here.
			//
			//_vm->_timers[31]._timer = _vm->_timers[31]._initTm = 40;
		}
	} while (!_vm->_video->_videoEnd && !_vm->shouldQuit());

	if (_vm->shouldQuit())
		return;

	_vm->_screen->flashPalette(12);
	_vm->_sound->playSound(4);
	do {
		_vm->_events->pollEvents();
	} while (!_vm->shouldQuit() && _vm->_sound->isSFXPlaying());

	_vm->_events->showCursor();
	_vm->_midi->stopSong();
	_vm->_midi->freeMusic();
}

void MartianScripts::cmdSpecial1(int param1, int param2) {
	//
	// Special 1 is a scene transition with some explanatory text
	//
	_vm->_events->hideCursor();

	if ((byte)param1 != (byte)-1) {
		_vm->_files->loadScreen(49, param1);
		_vm->_buffer2.copyBuffer(_vm->_screen);
	}

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();
	_vm->_events->showCursor();

	_vm->establish(0, param2);
}

void MartianScripts::cmdSpecial2() {
	_tunnel->tunnel2();
};

void MartianScripts::cmdSpecial3() {
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_files->loadScreen(57, 3);
	_vm->_buffer2.copyFrom(*_vm->_screen);

	_vm->_screen->setIconPalette();
	_vm->_events->showCursor();
	_vm->_screen->forceFadeIn();
}

void MartianScripts::cmdSpecial4() {
	_tunnel->tunnel4();
}

void MartianScripts::doIntro(int param1) {
	_game->doSpecial5(param1);
}

void MartianScripts::cmdSpecial6() {
	// A special transition screen after the jetpack in the outpost.
	warning("cmdSpecial6: TODO: Store current music");
	_vm->_midi->stopSong();
	_vm->_screen->setDisplayScan();
	_vm->_events->clearEvents();
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_files->loadScreen(49, 9);
	_vm->_events->showCursor();
	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	Resource *cellsRes = _vm->_files->loadFile("CELLS00.LZ");
	_vm->_objectsTable[0] = new SpriteResource(_vm, cellsRes);
	delete cellsRes;

	_vm->_timers[20]._timer = _vm->_timers[20]._initTm = 30;
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charSet._hi = 10;
	_vm->_fonts._charFor._lo = 1;
	_vm->_fonts._charFor._hi = 255;

	_vm->_screen->_maxChars = 50;
	_vm->_screen->_printOrg = _vm->_screen->_printStart = Common::Point(24, 18);

	Resource *notesRes = _vm->_files->loadFile("ETEXT.DAT");
	notesRes->_stream->seek(72);
	uint16 offset = notesRes->_stream->readUint16LE();
	notesRes->_stream->seek(offset);

	// Read the message
	Common::String msg = notesRes->_stream->readString();

	//display the message
	_game->showExpositionText(msg);

	delete notesRes;
	delete _vm->_objectsTable[0];
	_vm->_objectsTable[0] = nullptr;
	_vm->_midi->stopSong();

	// WORKAROUND: Reset Tex's scale flag after jetpack.
	// (bug also present in original game)
	_vm->_player->_flags &= ~IMGFLAG_UNSCALED;

	warning("cmdSpecial6: TODO: Restore original music");
}

void MartianScripts::cmdSpecial7() {
	_vm->_room->clearRoom();
	_vm->_midi->loadMusic(47, 8);

	_vm->_sound->freeSounds();
	Resource *sound = _vm->_sound->loadSound(46, 14);
	_vm->_sound->_soundTable.push_back(SoundEntry(sound, 1));

	_vm->_screen->setDisplayScan();
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(40, 3);
	_vm->_buffer1.copyBuffer(_vm->_screen);
	_vm->_buffer2.copyBuffer(_vm->_screen);

	_vm->_events->showCursor();
	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	// Load objects specific to this special scene
	Resource *data = _vm->_files->loadFile(40, 2);
	_game->_objectsTable[40] = new SpriteResource(_vm, data);
	delete data;

	// Load animation data
	_vm->_animation->freeAnimationData();
	Resource *animResource = _vm->_files->loadFile(40, 1);
	_vm->_animation->loadAnimations(animResource);
	delete animResource;

	// Load script
	Resource *newScript = _vm->_files->loadFile(40, 0);
	setScript(newScript);

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 0;
	searchForSequence();
	executeScript();
	_vm->_sound->playSound(0);

	do {
		charLoop();
		_vm->_events->pollEvents();
	} while (_vm->_flags[134] != 1);

	do {
		_vm->_events->pollEvents();
	} while (!_vm->shouldQuit() && _vm->_sound->isSFXPlaying());

	_vm->_animation->clearTimers();
	_vm->_animation->freeAnimationData();
	_vm->_scripts->freeScriptData();
	_vm->_sound->freeSounds();

	_vm->_screen->forceFadeOut();
	_vm->_midi->midiPlay();
	_vm->_midi->setLoop(true);
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(40, 4);
	_vm->_buffer1.copyBuffer(_vm->_screen);
	_vm->_buffer2.copyBuffer(_vm->_screen);

	_vm->_events->showCursor();
	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	// Setup fonts
	_vm->_fonts._charSet._hi = 10;
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charFor._lo = 247;
	_vm->_fonts._charFor._hi = 255;
	_vm->_screen->_maxChars = 50;
	_vm->_screen->_printOrg = Common::Point(24, 18);
	_vm->_screen->_printStart = Common::Point(24, 18);

	_game->showExpositionText(Common::String(SPEC7MESSAGE));

	_vm->_events->showCursor();
	_vm->_screen->copyBuffer(&_vm->_buffer1);
	_vm->_events->hideCursor();

	_vm->_video->setVideo(_vm->_screen, Common::Point(120, 16), FileIdent(40, 5), 10);

	while (!_vm->shouldQuit() && !_vm->_video->_videoEnd) {
		_vm->_video->playVideo();
		_vm->_events->pollEventsAndWait();
	}

	_vm->_sound->freeSounds();
	sound = _vm->_sound->loadSound(40, 8);
	_vm->_sound->_soundTable.push_back(SoundEntry(sound, 1));
	sound = _vm->_sound->loadSound(40, 9);
	_vm->_sound->_soundTable.push_back(SoundEntry(sound, 1));
	sound = _vm->_sound->loadSound(40, 10);
	_vm->_sound->_soundTable.push_back(SoundEntry(sound, 1));

	_vm->_screen->forceFadeOut();
	_vm->_files->loadScreen(40, 7);
	_vm->_destIn = _vm->_screen;

	_vm->_screen->plotImage(_game->_objectsTable[40], 8, Common::Point(176, 104));
	_vm->_screen->plotImage(_game->_objectsTable[40], 7, Common::Point(160, 102));
	_vm->_events->showCursor();
	_vm->_screen->forceFadeIn();

	_vm->_events->_vbCount = 100;
	while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
		_vm->_events->pollEventsAndWait();

	_vm->_sound->playSound(0);
	do {
		_vm->_events->pollEvents();
	} while (!_vm->shouldQuit() && _vm->_sound->isSFXPlaying());

	_vm->_events->_vbCount = 80;
	while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
		_vm->_events->pollEventsAndWait();

	_vm->_sound->playSound(1);
	do {
		_vm->_events->pollEvents();
	} while (!_vm->shouldQuit() && _vm->_sound->isSFXPlaying());

	_vm->_events->_vbCount = 80;
	while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
		_vm->_events->pollEventsAndWait();

	_vm->_sound->playSound(2);
	do {
		_vm->_events->pollEvents();
	} while (!_vm->shouldQuit() && _vm->_sound->isSFXPlaying());

	_vm->_sound->freeSounds();

	delete _game->_objectsTable[40];
	_game->_objectsTable[40] = nullptr;

	_vm->_events->hideCursor();
	_vm->_screen->forceFadeOut();
	_vm->_files->loadScreen(40, 6);
	_vm->_events->showCursor();
	_vm->_screen->forceFadeIn();

	_vm->_events->waitKeyActionMouse();
	_vm->_midi->stopSong();
	_vm->_midi->freeMusic();

	// The original was jumping to the restart label in main
	_vm->_restartFl = true;
	_vm->_events->pollEvents();
}

void MartianScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 0:
		cmdSpecial0();
		break;
	case 1:
		cmdSpecial1(param1, param2);
		break;
	case 2:
		cmdSpecial2();
		break;
	case 3:
		cmdSpecial3();
		break;
	case 4:
		cmdSpecial4();
		break;
	case 5:
		doIntro(param1);
		break;
	case 6:
		cmdSpecial6();
		break;
	case 7:
		cmdSpecial7();
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}

typedef void(MartianScripts::*MartianScriptMethodPtr)();

void MartianScripts::executeCommand(int commandIndex) {
	Scripts::executeCommand(commandIndex);
}

} // End of namespace Martian

} // End of namespace Access
