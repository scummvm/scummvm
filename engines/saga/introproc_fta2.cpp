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
	MoviePlayerSMK(SagaEngine *vm): _vm(vm), SMKPlayer(vm->_mixer) {
		_frameSkipped = 0;
		_ticks = 0;
	}
	~MoviePlayerSMK(void) { closeFile(); }

	bool load(const char *filename) {
		_skipVideo = false;
		return loadFile(filename);
	}
	void playVideo();
	void stopVideo() { closeFile(); }
private:
	bool processFrame();
	PalEntry _smkPalette[256];
	uint32 _ticks;
	uint16 _frameSkipped;
	bool _skipVideo;
	SagaEngine *_vm;
	Common::Event _event;
};

void MoviePlayerSMK::setPalette(byte *pal) {
	for (int i = 0; i < 256; i++) {
		_smkPalette[i].red = *pal++;
		_smkPalette[i].green = *pal++;
		_smkPalette[i].blue = *pal++;
	}

	_vm->_gfx->setPalette(_smkPalette, true);
}

void MoviePlayerSMK::playVideo() {
	while (getCurFrame() < getFrameCount() && !_skipVideo && !_vm->shouldQuit()) {
		Common::EventManager *eventMan = _vm->_system->getEventManager();
		// process events, and skip video if esc is pressed
		while (eventMan->pollEvent(_event)) {
			switch (_event.type) {
				case Common::EVENT_KEYDOWN:
					if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
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
		decodeNextFrame();
		if (processFrame())
			_vm->_system->updateScreen();
	}
}

bool MoviePlayerSMK::processFrame() {
	Graphics::Surface *screen = _vm->_system->lockScreen();
	copyFrameToBuffer((byte *)screen->pixels,
						(_vm->getDisplayInfo().width - getWidth()) / 2,
						(_vm->getDisplayInfo().height - getHeight()) / 2,
						_vm->getDisplayInfo().width);
	_vm->_system->unlockScreen();

	if (!getAudioLag() || getFrameWaitTime() || _frameSkipped > getFrameRate()) {
		if (_frameSkipped > getFrameRate()) {
			warning("force frame %i redraw", getCurFrame());
			_frameSkipped = 0;
		}

		if (getAudioLag() > 0) {
			while (getAudioLag() > 0) {
				_vm->_system->delayMillis(10);
			}
			// In case the background sound ends prematurely, update
			// _ticks so that we can still fall back on the no-sound
			// sync case for the subsequent frames.
			_ticks = _vm->_system->getMillis();
		} else {
			_ticks += getFrameDelay();
			//while (_vm->_system->getMillis() < _ticks)	// FIXME
				_vm->_system->delayMillis(10);
		}

		return true;
	}

	warning("dropped frame %i", getCurFrame());
	_frameSkipped++;
	return false;
}

int Scene::FTA2StartProc() {
	MoviePlayerSMK *smkPlayer = new MoviePlayerSMK(_vm);

	_vm->_gfx->showCursor(false);

	// Show Ignite logo
	if (smkPlayer->load("trimark.smk")) {
		debug(0, "Playing video trimark.smk");
		smkPlayer->playVideo();
		smkPlayer->stopVideo();
	} else {
		warning("Failed to load video file trimark.smk");
	}

	// Play introduction
	if (smkPlayer->load("intro.smk")) {
		debug(0, "Playing video intro.smk");
		smkPlayer->playVideo();
		smkPlayer->stopVideo();
	} else {
		warning("Failed to load video file intro.smk");
	}

	// Cleanup
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

	MoviePlayerSMK *smkPlayer = new MoviePlayerSMK(_vm);

	_vm->_gfx->showCursor(false);

	// Play ending
	if (smkPlayer->load(videoName)) {
		debug(0, "Playing video %s", videoName);
		smkPlayer->playVideo();
		smkPlayer->stopVideo();
	} else {
		warning("Failed to load video file %s", videoName);
	}

	// Cleanup
	delete smkPlayer;

	return SUCCESS;
}

} // End of namespace Saga
