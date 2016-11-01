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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/riven_stacks/bspit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"

#include "common/events.h"

namespace Mohawk {
namespace RivenStacks {

BSpit::BSpit(MohawkEngine_Riven *vm) :
		DomeSpit(vm, kStackBspit) {

	REGISTER_COMMAND(BSpit, xblabopenbook);
	REGISTER_COMMAND(BSpit, xblabbookprevpage);
	REGISTER_COMMAND(BSpit, xblabbooknextpage);
	REGISTER_COMMAND(BSpit, xsoundplug);
	REGISTER_COMMAND(BSpit, xbchangeboiler);
	REGISTER_COMMAND(BSpit, xbupdateboiler);
	REGISTER_COMMAND(BSpit, xbsettrap);
	REGISTER_COMMAND(BSpit, xbcheckcatch);
	REGISTER_COMMAND(BSpit, xbait);
	REGISTER_COMMAND(BSpit, xbfreeytram);
	REGISTER_COMMAND(BSpit, xbaitplate);
	REGISTER_COMMAND(BSpit, xbisland190_opencard);
	REGISTER_COMMAND(BSpit, xbisland190_resetsliders);
	REGISTER_COMMAND(BSpit, xbisland190_slidermd);
	REGISTER_COMMAND(BSpit, xbisland190_slidermw);
	REGISTER_COMMAND(BSpit, xbscpbtn);
	REGISTER_COMMAND(BSpit, xbisland_domecheck);
	REGISTER_COMMAND(BSpit, xvalvecontrol);
	REGISTER_COMMAND(BSpit, xbchipper);
}

void BSpit::xblabopenbook(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 page = _vm->_vars["blabpage"];

	// Draw the image of the page based on the blabbook variable
	_vm->getCard()->drawPicture(page);

	if (page == 14) {
		// Draw the dome combination
		// The images for the numbers are tBMP's 364 through 368
		// The start point is at (240, 82)
		uint32 domeCombo = _vm->_vars["adomecombo"];
		static const uint16 kNumberWidth = 32;
		static const uint16 kNumberHeight = 24;
		static const uint16 kDstX = 240;
		static const uint16 kDstY = 82;
		byte numCount = 0;

		for (int bitPos = 24; bitPos >= 0; bitPos--) {
			if (domeCombo & (1 << bitPos)) {
				uint16 offset = (24 - bitPos) * kNumberWidth;
				Common::Rect srcRect = Common::Rect(offset, 0, offset + kNumberWidth, kNumberHeight);
				Common::Rect dstRect = Common::Rect(numCount * kNumberWidth + kDstX, kDstY, (numCount + 1) * kNumberWidth + kDstX, kDstY + kNumberHeight);
				_vm->_gfx->drawImageRect(numCount + 364, srcRect, dstRect);
				numCount++;
			}
		}

		assert(numCount == 5); // Sanity check
	}
}

void BSpit::xblabbookprevpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 &page = _vm->_vars["blabpage"];

	// Decrement the page if it's not the first page
	if (page == 1)
		return;
	page--;

	// Play the page turning sound
	_vm->_sound->playSound(22);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(1);
	_vm->getCard()->drawPicture(page);
}

void BSpit::xblabbooknextpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 &page = _vm->_vars["blabpage"];

	// Increment the page if it's not the last page
	if (page == 22)
		return;
	page++;

	// Play the page turning sound
	_vm->_sound->playSound(23);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(0);
	_vm->getCard()->drawPicture(page);
}

void BSpit::xsoundplug(uint16 argc, uint16 *argv) {
	if (_vm->_vars["bheat"] != 0)
		_vm->getCard()->playSound(1);
	else if (_vm->_vars["bcratergg"] != 0)
		_vm->getCard()->playSound(2);
	else
		_vm->getCard()->playSound(3);
}

void BSpit::xbchangeboiler(uint16 argc, uint16 *argv) {
	uint32 heat = _vm->_vars["bheat"];
	uint32 water = _vm->_vars["bblrwtr"];
	uint32 platform = _vm->_vars["bblrgrt"];

	// Stop any background videos
	_vm->_video->stopVideos();

	if (argv[0] == 1) {
		// Water is filling/draining from the boiler
		if (water == 0) {
			if (platform == 1)
				_vm->_video->activateMLST(_vm->getCard()->getMovie(12));
			else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(10));
		} else if (heat == 1) {
			if (platform == 1)
				_vm->_video->activateMLST(_vm->getCard()->getMovie(22));
			else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(19));
		} else {
			if (platform == 1)
				_vm->_video->activateMLST(_vm->getCard()->getMovie(16));
			else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(13));
		}
	} else if (argv[0] == 2 && water != 0) {
		if (heat == 1) {
			// Turning on the heat
			if (platform == 1)
				_vm->_video->activateMLST(_vm->getCard()->getMovie(23));
			else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(20));
		} else {
			// Turning off the heat
			if (platform == 1)
				_vm->_video->activateMLST(_vm->getCard()->getMovie(18));
			else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(15));
		}
	} else if (argv[0] == 3) {
		if (platform == 1) {
			// Lowering the platform
			if (water == 1) {
				if (heat == 1)
					_vm->_video->activateMLST(_vm->getCard()->getMovie(24));
				else
					_vm->_video->activateMLST(_vm->getCard()->getMovie(17));
			} else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(11));
		} else {
			// Raising the platform
			if (water == 1) {
				if (heat == 1)
					_vm->_video->activateMLST(_vm->getCard()->getMovie(21));
				else
					_vm->_video->activateMLST(_vm->getCard()->getMovie(14));
			} else
				_vm->_video->activateMLST(_vm->getCard()->getMovie(9));
		}
	}

	if (argc > 1)
		_vm->getCard()->playSound(argv[1]);
	else if (argv[0] == 2)
		_vm->getCard()->playSound(1);

	_vm->_cursor->setCursor(kRivenHideCursor);
	_vm->_video->playMovieBlockingRiven(11);
}

void BSpit::xbupdateboiler(uint16 argc, uint16 *argv) {
	if (_vm->_vars["bheat"] != 0) {
		if (_vm->_vars["bblrgrt"] == 0) {
			_vm->_video->activateMLST(_vm->getCard()->getMovie(8));
			_vm->_video->playMovieRiven(8);
		} else {
			_vm->_video->activateMLST(_vm->getCard()->getMovie(7));
			_vm->_video->playMovieRiven(7);
		}
	} else {
		VideoEntryPtr video = _vm->_video->findVideoRiven(7);
		if (video)
			video->setEnabled(false);
		video = _vm->_video->findVideoRiven(8);
		if (video)
			video->setEnabled(false);
	}
}

static void ytramTrapTimer(MohawkEngine_Riven *vm) {
	// Remove this timer
	vm->removeTimer();

	// FIXME: Improve the timer system (use a functor ?)

	// Check if we've caught a Ytram
	BSpit *bspit = dynamic_cast<BSpit *>(vm->getStack());
	if (!bspit) {
		error("Unexpected stack type in 'ytramTrapTimer'");
	}

	bspit->checkYtramCatch(true);
}

void BSpit::xbsettrap(uint16 argc, uint16 *argv) {
	// Set the Ytram trap

	// We can catch the Ytram between 10 seconds and 3 minutes from now
	uint32 timeUntilCatch = _vm->_rnd->getRandomNumberRng(10, 60 * 3) * 1000;
	_vm->_vars["bytramtime"] = timeUntilCatch + _vm->getTotalPlayTime();

	// And set the timer too
	_vm->installTimer(&ytramTrapTimer, timeUntilCatch);
}

void BSpit::checkYtramCatch(bool playSound) {
	// Check if we've caught a Ytram

	uint32 &ytramTime = _vm->_vars["bytramtime"];

	// If the trap still has not gone off, reinstall our timer
	// This is in case you set the trap, walked away, and returned
	if (_vm->getTotalPlayTime() < ytramTime) {
		_vm->installTimer(&ytramTrapTimer, ytramTime - _vm->getTotalPlayTime());
		return;
	}

	// Increment the movie per catch (max = 3)
	uint32 &ytramMovie = _vm->_vars["bytram"];
	ytramMovie++;
	if (ytramMovie > 3)
		ytramMovie = 3;

	// Reset variables
	_vm->_vars["bytrapped"] = 1;
	_vm->_vars["bbait"] = 0;
	_vm->_vars["bytrap"] = 0;
	ytramTime = 0;

	// Play the capture sound, if requested
	if (playSound)
		_vm->_sound->playSound(33);
}

void BSpit::xbcheckcatch(uint16 argc, uint16 *argv) {
	// Just pass our parameter along...
	checkYtramCatch(argv[0] != 0);
}

void BSpit::xbait(uint16 argc, uint16 *argv) {
	// Set the cursor to the pellet
	_vm->_cursor->setCursor(kRivenPelletCursor);
	_vm->_system->updateScreen();

	// Loop until the player lets go (or quits)
	Common::Event event;
	bool mouseDown = true;
	while (mouseDown) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONUP)
				mouseDown = false;
			else if (event.type == Common::EVENT_MOUSEMOVE)
				_vm->_system->updateScreen();
			else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RTL)
				return;
		}

		_vm->_system->delayMillis(10); // Take it easy on the CPU
	}

	// Set back the cursor
	_vm->_cursor->setCursor(kRivenMainCursor);
	_vm->_system->updateScreen();

	RivenHotspot *bait = _vm->getCard()->getHotspotByBlstId(9);
	RivenHotspot *baitPlate = _vm->getCard()->getHotspotByBlstId(16);

	// Set the bait if we put it on the plate
	if (baitPlate->containsPoint(_vm->_system->getEventManager()->getMousePos())) {
		_vm->_vars["bbait"] = 1;
		_vm->getCard()->drawPicture(4);

		bait->enable(false); // Disable bait hotspot
		baitPlate->enable(true); // Enable baitplate hotspot
	}
}

void BSpit::xbfreeytram(uint16 argc, uint16 *argv) {
	// Play a random Ytram movie after freeing it
	uint16 mlstId;

	switch (_vm->_vars["bytram"]) {
		case 1:
			mlstId = 11;
			break;
		case 2:
			mlstId = 12;
			break;
		default:
			mlstId = _vm->_rnd->getRandomNumberRng(13, 15);
			break;
	}

	// Activate the MLST and play the video
	_vm->_video->activateMLST(_vm->getCard()->getMovie(mlstId));
	_vm->_video->playMovieBlockingRiven(11);

	// Now play the second movie
	_vm->_video->activateMLST(_vm->getCard()->getMovie(mlstId + 5));
	_vm->_video->playMovieBlockingRiven(12);
}

void BSpit::xbaitplate(uint16 argc, uint16 *argv) {
	// Remove the pellet from the plate and put it in your hand
	_vm->_cursor->setCursor(kRivenPelletCursor);
	_vm->getCard()->drawPicture(3);

	// Loop until the player lets go (or quits)
	Common::Event event;
	bool mouseDown = true;
	while (mouseDown) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONUP)
				mouseDown = false;
			else if (event.type == Common::EVENT_MOUSEMOVE)
				_vm->_system->updateScreen();
			else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RTL)
				return;
		}

		_vm->_system->delayMillis(10); // Take it easy on the CPU
	}

	// Set back the cursor
	_vm->_cursor->setCursor(kRivenMainCursor);
	_vm->_system->updateScreen();

	RivenHotspot *bait = _vm->getCard()->getHotspotByBlstId(9);
	RivenHotspot *baitPlate = _vm->getCard()->getHotspotByBlstId(16);

	// Set the bait if we put it on the plate, remove otherwise
	if (baitPlate->containsPoint(_vm->_system->getEventManager()->getMousePos())) {
		_vm->_vars["bbait"] = 1;
		_vm->getCard()->drawPicture(4);
		bait->enable(false); // Disable bait hotspot
		baitPlate->enable(true); // Enable baitplate hotspot
	} else {
		_vm->_vars["bbait"] = 0;
		bait->enable(true); // Enable bait hotspot
		baitPlate->enable(false); // Disable baitplate hotspot
	}
}

void BSpit::xbisland190_opencard(uint16 argc, uint16 *argv) {
	checkDomeSliders();
}

void BSpit::xbisland190_resetsliders(uint16 argc, uint16 *argv) {
	resetDomeSliders(41, 9);
}

void BSpit::xbisland190_slidermd(uint16 argc, uint16 *argv) {
	dragDomeSlider(41, 9);
}

void BSpit::xbisland190_slidermw(uint16 argc, uint16 *argv) {
	checkSliderCursorChange(9);
}

void BSpit::xbscpbtn(uint16 argc, uint16 *argv) {
	runDomeButtonMovie();
}

void BSpit::xbisland_domecheck(uint16 argc, uint16 *argv) {
	runDomeCheck();
}

void BSpit::xvalvecontrol(uint16 argc, uint16 *argv) {
	Common::Point startPos = _vm->_system->getEventManager()->getMousePos();

	// Get the variable for the valve
	uint32 &valve = _vm->_vars["bvalve"];

	int changeX = 0;
	int changeY = 0;
	bool done = false;

	// Set the cursor to the closed position
	_vm->_cursor->setCursor(kRivenClosedHandCursor);
	_vm->_system->updateScreen();

	while (!done) {
		Common::Event event;

		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					changeX = event.mouse.x - startPos.x;
					changeY = startPos.y - event.mouse.y;
					_vm->_system->updateScreen();
					break;
				case Common::EVENT_LBUTTONUP:
					// FIXME: These values for changes in x/y could be tweaked.
					if (valve == 0 && changeY <= -10) {
						valve = 1;
						_vm->_cursor->setCursor(kRivenHideCursor);
						_vm->_system->updateScreen();
						_vm->_video->playMovieBlockingRiven(2);
						_vm->refreshCard();
					} else if (valve == 1) {
						if (changeX >= 0 && changeY >= 10) {
							valve = 0;
							_vm->_cursor->setCursor(kRivenHideCursor);
							_vm->_system->updateScreen();
							_vm->_video->playMovieBlockingRiven(3);
							_vm->refreshCard();
						} else if (changeX <= -10 && changeY <= 10) {
							valve = 2;
							_vm->_cursor->setCursor(kRivenHideCursor);
							_vm->_system->updateScreen();
							_vm->_video->playMovieBlockingRiven(1);
							_vm->refreshCard();
						}
					} else if (valve == 2 && changeX >= 10) {
						valve = 1;
						_vm->_cursor->setCursor(kRivenHideCursor);
						_vm->_system->updateScreen();
						_vm->_video->playMovieBlockingRiven(4);
						_vm->refreshCard();
					}
					done = true;
				default:
					break;
			}
		}
		_vm->_system->delayMillis(10);
	}

	// If we changed state and the new state is that the valve is flowing to
	// the boiler, we need to update the boiler state.
	if (valve == 1) {
		if (_vm->_vars["bidvlv"] == 1) { // Check which way the water is going at the boiler
			if (_vm->_vars["bblrarm"] == 1) {
				// If the pipe is open, make sure the water is drained out
				_vm->_vars["bheat"] = 0;
				_vm->_vars["bblrwtr"] = 0;
			} else {
				// If the pipe is closed, fill the boiler again
				_vm->_vars["bheat"] = _vm->_vars["bblrvalve"];
				_vm->_vars["bblrwtr"] = 1;
			}
		} else {
			// Have the grating inside the boiler match the switch outside
			_vm->_vars["bblrgrt"] = (_vm->_vars["bblrsw"] == 1) ? 0 : 1;
		}
	}
}

void BSpit::xbchipper(uint16 argc, uint16 *argv) {
	// Why is this an external command....?
	if (_vm->_vars["bvalve"] == 2)
		_vm->_video->playMovieBlockingRiven(2);
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
