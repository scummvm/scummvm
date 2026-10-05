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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/archive.h"
#include "common/config-manager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/state/logo.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Logo);
}

namespace Nancy {
namespace State {

void Logo::process() {
	switch (_state) {
	case kInit:
		init();
		break;
	case kPlayIntroVideo:
		playIntroVideo();
		break;
	case kStartSound:
		startSound();
		break;
	case kRun:
		run();
		break;
	case kShowScreens:
		runScreens();
		break;
	case kStop:
		stop();
	}
}

void Logo::onStateEnter(const NancyState::NancyState prevState) {
	// Handle returning from the GMM
	if (prevState == NancyState::kPause) {
		if (_state == kPlayIntroVideo) {
			_tvdVideoDecoder.pauseVideo(false);
		} else if (_state == kShowScreens) {
			_screenMovie.pauseVideo(false);
		} else if (_state == kRun) {
			g_nancy->_sound->pauseSound("MSND", false);
		}
	}
}

bool Logo::onStateExit(const NancyState::NancyState nextState) {
	// Handle the GMM being called
	if (nextState == NancyState::kPause) {
		if (_state == kPlayIntroVideo) {
			_tvdVideoDecoder.pauseVideo(true);
		} else if (_state == kShowScreens) {
			_screenMovie.pauseVideo(true);
		} else if (_state == kRun) {
			g_nancy->_sound->pauseSound("MSND", true);
		}

		return false;
	} else {
		return true;
	}
}

void Logo::init() {
	if (g_nancy->getGameType() >= kGameTypeNancy16) {
		// The logo movies are made for the screen's resolution, not the game's
		_videoObj.setNativeResolution(true);
		_screenID = 0;
		_state = kShowScreens;
		startScreen();
		return;
	}

	const ImageChunk *lg0 = (const ImageChunk *)g_nancy->getEngineData("LG0");
	const ImageChunk *plg0 = (const ImageChunk *)g_nancy->getEngineData("PLG0");
	if (!plg0) {
		plg0 = (const ImageChunk *)g_nancy->getEngineData("PLGO");
	}
	assert(lg0);

	_logoImage.init(lg0->imageName);
	_logoImage.registerGraphics();

	if (plg0) {
		_partnerLogoImage.init(plg0->imageName);
		_partnerLogoImage.registerGraphics();
	}

	if (g_nancy->getGameType() == kGameTypeVampire && _tvdVideoDecoder.loadFile("VAMPINTR.AVI")) {
		_tvdVideoDecoder.start();
		_videoObj.moveTo(Common::Rect(0, 0, 640, 480));
		_videoObj._drawSurface.create(_tvdVideoDecoder.getWidth(), _tvdVideoDecoder.getHeight(), _tvdVideoDecoder.getPixelFormat());
		_videoObj.setPalette(_tvdVideoDecoder.getPalette());
		_videoObj.registerGraphics();
		_videoObj.setVisible(true);
		_state = kPlayIntroVideo;
	} else {
		_state = kStartSound;
	}
}

// The Vampire Diaries originally shipped with a launcher that could either start the game
// or play an introduction video. We don't bother giving the player a choice, and just
// play the video before the game logo
void Logo::playIntroVideo() {
	if (_tvdVideoDecoder.needsUpdate()) {
		const Graphics::Surface *frame = _tvdVideoDecoder.decodeNextFrame();
		if (frame) {
			_videoObj._drawSurface.blitFrom(*frame);
			_videoObj.setVisible(true);
		}
	}
	if (_tvdVideoDecoder.endOfVideo() || (g_nancy->_input->getInput().input & NancyInput::kLeftMouseButtonDown)) {
		_state = kStartSound;
		_videoObj.setVisible(false);
		_tvdVideoDecoder.close();
	}
}

void Logo::startSound() {
	g_nancy->_sound->playSound("MSND");

	_startTicks = g_nancy->getTotalPlayTime();
	_state = kRun;
}

void Logo::run() {
	if (g_nancy->getTotalPlayTime() - _startTicks >= g_nancy->getStaticData().logoEndAfter) {
		// Display game logo after partner logo
		if (!_partnerLogoImage._drawSurface.empty() && _partnerLogoImage.isVisible()) {
			_logoImage.setVisible(true);
			_partnerLogoImage.setVisible(false);
			_startTicks = g_nancy->getTotalPlayTime();
		} else {
			_state = kStop;
		}
	}

	if (g_nancy->_input->getInput().input & NancyInput::kLeftMouseButtonDown) {
		_state = kStop;
	}
}

void Logo::startScreen() {
	const LogoScreen *screen = (const LogoScreen *)g_nancy->getEngineData(Common::String::format("LG%u", _screenID));
	if (!screen) {
		_state = kStop;
		return;
	}

	// Nancy17 names a picture for its first screen that it doesn't have
	if (SearchMan.hasFile(screen->imageName) || SearchMan.hasFile(screen->imageName.append(".cif"))) {
		_logoImage.init(screen->imageName);

		// A picture made for the game's resolution gets scaled up with the game
		_logoImage.setNativeResolution(_logoImage._drawSurface.w > g_nancy->_graphics->getGameBounds().width());
		_logoImage.moveTo(screen->position);
		_logoImage.registerGraphics();
	}

	_screenMovie.close();
	_videoObj.setVisible(false);
	if (!screen->movieName.empty() && _screenMovie.loadFile(screen->movieName)) {
		_videoObj.moveTo(Common::Rect(screen->position.x, screen->position.y,
			screen->position.x + _screenMovie.getWidth(), screen->position.y + _screenMovie.getHeight()));
		_videoObj.registerGraphics();
		_screenMovie.start();
	}

	_startTicks = g_nancy->getTotalPlayTime();
}

// A screen lasts until its movie ends, or for a while when it has none; a click skips it
void Logo::runScreens() {
	bool isDone = false;
	if (_screenMovie.isVideoLoaded()) {
		if (_screenMovie.needsUpdate()) {
			const Graphics::Surface *frame = _screenMovie.decodeNextFrame();
			if (frame) {
				GraphicsManager::copyToManaged(*frame, _videoObj._drawSurface);
				_videoObj.setVisible(true);
			}
		}

		isDone = _screenMovie.endOfVideo();
	} else {
		isDone = g_nancy->getTotalPlayTime() - _startTicks >= g_nancy->getStaticData().logoEndAfter;
	}

	if (g_nancy->_input->getInput().input & NancyInput::kLeftMouseButtonDown) {
		isDone = true;
	}

	if (isDone) {
		++_screenID;
		startScreen();
	}
}

void Logo::stop() {
	_screenMovie.close();

	// The original engine checks for N+D and N+C key combos here.
	// For the N+C key combo it looks for some kind of cheat file
	// to initialize the game state with.

	// Nancy16 has no main menu state; its logos lead straight into the first scene
	if (g_nancy->getGameType() >= kGameTypeNancy16) {
		g_nancy->setState(NancyState::kScene);
	} else if (!ConfMan.hasKey("original_menus") || ConfMan.getBool("original_menus")) {
		g_nancy->setState(NancyState::kMainMenu);
	} else {
		g_nancy->setState(NancyState::kScene);
	}
}

} // End of namespace State
} // End of namespace Nancy
