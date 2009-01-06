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
 * $URL$
 * $Id$
 *
 */

#ifdef ENABLE_SAGA2

// "Faery Tale Adventure II: Halls of the Dead" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/scene.h"
#include "saga/gfx.h"

#include "sound/mixer.h"
#include "graphics/surface.h"
#include "graphics/video/smk_player.h"

#include "common/events.h"
#include "common/system.h"

namespace Saga {

class MoviePlayerSMK : Graphics::SMKPlayer {
protected:
	virtual void setPalette(byte *pal);
public:
	MoviePlayerSMK(SagaEngine *vm): _vm(vm), SMKPlayer(vm->_mixer), 
									_eventMan(vm->_system->getEventManager()) {
	}
	~MoviePlayerSMK(void) { }

	void playVideo(const char *filename);
private:
	void processFrame();
	void processEvents();
	PalEntry _smkPalette[256];
	bool _skipVideo;
	SagaEngine *_vm;
	Common::EventManager *_eventMan;
};

void MoviePlayerSMK::setPalette(byte *pal) {
	for (int i = 0; i < 256; i++) {
		_smkPalette[i].red = *pal++;
		_smkPalette[i].green = *pal++;
		_smkPalette[i].blue = *pal++;
	}

	_vm->_gfx->setPalette(_smkPalette, true);
}

void MoviePlayerSMK::processEvents() {
	Common::Event curEvent;
	// Process events, and skip video if esc is pressed
	while (_eventMan->pollEvent(curEvent)) {
		switch (curEvent.type) {
		case Common::EVENT_KEYDOWN:
			if (curEvent.kbd.keycode == Common::KEYCODE_ESCAPE)
				_skipVideo = true;
			break;
		case Common::EVENT_RTL:
		case Common::EVENT_QUIT:
			_skipVideo = true;
			break;
		default:
			break;
		}
	}
}

void MoviePlayerSMK::playVideo(const char *filename) {
	_skipVideo = false;
	debug(0, "Playing video %s", filename);

	if (!loadFile(filename)) {
		warning("Failed to load video file %s", filename);
		return;
	}

	while (getCurFrame() < getFrameCount() && !_skipVideo && !_vm->shouldQuit()) {
		processEvents();
		processFrame();			
	}

	closeFile();
}

void MoviePlayerSMK::processFrame() {
	uint32 startTime = 0;
	decodeNextFrame();

	Graphics::Surface *screen = _vm->_system->lockScreen();
	copyFrameToBuffer((byte *)screen->pixels,
						(_vm->getDisplayInfo().width - getWidth()) / 2,
						(_vm->getDisplayInfo().height - getHeight()) / 2,
						_vm->getDisplayInfo().width);
	_vm->_system->unlockScreen();

	uint32 waitTime = getFrameWaitTime();

	if (!waitTime) {
		warning("dropped frame %i", getCurFrame());
		return;
	}

	// Update the screen
	_vm->_system->updateScreen();

	startTime = _vm->_system->getMillis();

	// Wait before showing the next frame
	while (_vm->_system->getMillis() < startTime + waitTime && !_skipVideo && !_vm->shouldQuit()) {
		processEvents();
		_vm->_system->delayMillis(10);
	}
}

int Scene::FTA2StartProc() {
	_vm->_gfx->showCursor(false);

	MoviePlayerSMK *smkPlayer = new MoviePlayerSMK(_vm);
	smkPlayer->playVideo("trimark.smk");      // Show Ignite logo
	smkPlayer->playVideo("intro.smk");        // Play introduction
	delete smkPlayer;

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2EndProc(FTA2Endings whichEnding) {
	char videoName[20];

	switch (whichEnding) {
	case kFta2BadEndingLaw:
		strcpy(videoName, "end_1.smk");
		break;
	case kFta2BadEndingChaos:
		strcpy(videoName, "end_2.smk");
		break;
	case kFta2GoodEnding1:
		strcpy(videoName, "end_3a.smk");
		break;
	case kFta2GoodEnding2:
		strcpy(videoName, "end_3b.smk");
		break;
	case kFta2BadEndingDeath:
		strcpy(videoName, "end_4.smk");
		break;
	default:
		error("Unknown FTA2 ending");
	}

	_vm->_gfx->showCursor(false);

	// Play ending
	MoviePlayerSMK *smkPlayer = new MoviePlayerSMK(_vm);
	smkPlayer->playVideo(videoName);
	delete smkPlayer;

	return SUCCESS;
}

} // End of namespace Saga

#endif