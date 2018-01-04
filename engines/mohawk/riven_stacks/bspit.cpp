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
#include "mohawk/riven_video.h"

namespace Mohawk {
namespace RivenStacks {

BSpit::BSpit(MohawkEngine_Riven *vm) :
		DomeSpit(vm, kStackBspit, "bSliders.190", "bSliderBG.190") {

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

void BSpit::xblabopenbook(const ArgumentArray &args) {
	// Get the variable
	uint32 page = _vm->_vars["blabpage"];

	// Draw the image of the page based on the blabbook variable
	_vm->getCard()->drawPicture(page);

	if (page == 14) {
		labBookDrawDomeCombination();
	}
}

void BSpit::labBookDrawDomeCombination() const {
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

void BSpit::xblabbookprevpage(const ArgumentArray &args) {
	// Get the page variable
	uint32 &page = _vm->_vars["blabpage"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the first page
		if (page == 1)
			return;

		// Update the page number
		page--;

		pageTurn(kRivenTransitionWipeRight);
		_vm->getCard()->drawPicture(page);

		if (page == 14) {
			labBookDrawDomeCombination();
		}

		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void BSpit::xblabbooknextpage(const ArgumentArray &args) {
	// Get the page variable
	uint32 &page = _vm->_vars["blabpage"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the last page
		if (page == 22)
			return;

		// Update the page number
		page++;

		pageTurn(kRivenTransitionWipeLeft);
		_vm->getCard()->drawPicture(page);

		if (page == 14) {
			labBookDrawDomeCombination();
		}

		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void BSpit::xsoundplug(const ArgumentArray &args) {
	if (_vm->_vars["bcratergg"] == 0) {
		if (_vm->_vars["bblrwtr"] == 0) {
			_vm->getCard()->overrideSound(0, 2);
		} else {
			_vm->getCard()->overrideSound(0, 3);
		}
	} else {
		_vm->getCard()->overrideSound(0, 1);
	}
}

void BSpit::xbchangeboiler(const ArgumentArray &args) {
	uint32 heat = _vm->_vars["bheat"];
	uint32 water = _vm->_vars["bblrwtr"];
	uint32 platform = _vm->_vars["bblrgrt"];

	// Stop any background videos
	_vm->_video->closeVideos();

	if (args[0] == 1) {
		// Water is filling/draining from the boiler
		if (water == 0) {
			if (platform == 1)
				_vm->getCard()->playMovie(12);
			else
				_vm->getCard()->playMovie(10);
		} else if (heat == 1) {
			if (platform == 1)
				_vm->getCard()->playMovie(22);
			else
				_vm->getCard()->playMovie(19);
		} else {
			if (platform == 1)
				_vm->getCard()->playMovie(16);
			else
				_vm->getCard()->playMovie(13);
		}
	} else if (args[0] == 2 && water != 0) {
		if (heat == 1) {
			// Turning on the heat
			if (platform == 1)
				_vm->getCard()->playMovie(23);
			else
				_vm->getCard()->playMovie(20);
		} else {
			// Turning off the heat
			if (platform == 1)
				_vm->getCard()->playMovie(18);
			else
				_vm->getCard()->playMovie(15);
		}
	} else if (args[0] == 3) {
		if (platform == 1) {
			// Lowering the platform
			if (water == 1) {
				if (heat == 1)
					_vm->getCard()->playMovie(24);
				else
					_vm->getCard()->playMovie(17);
			} else {
				_vm->getCard()->playMovie(11);
			}
		} else {
			// Raising the platform
			if (water == 1) {
				if (heat == 1)
					_vm->getCard()->playMovie(21);
				else
					_vm->getCard()->playMovie(14);
			} else {
				_vm->getCard()->playMovie(9);
			}
		}
	}

	if (args.size() > 1)
		_vm->getCard()->playSound(args[1]);
	else if (args[0] == 2)
		_vm->getCard()->playSound(1);

	RivenVideo *video = _vm->_video->openSlot(11);
	video->playBlocking();
}

void BSpit::xbupdateboiler(const ArgumentArray &args) {
	if (_vm->_vars["bheat"] != 0) {
		if (_vm->_vars["bblrgrt"] == 0) {
			_vm->getCard()->playMovie(8);
		} else {
			_vm->getCard()->playMovie(7);
		}
	} else {
		RivenVideo *video = _vm->_video->getSlot(7);
		if (video) {
			video->disable();
			video->stop();
		}
		video = _vm->_video->getSlot(8);
		if (video) {
			video->disable();
			video->stop();
		}
	}
}

void BSpit::ytramTrapTimer() {
	// Remove this timer
	removeTimer();

	// Check if we've caught a Ytram
	checkYtramCatch(true);
}

void BSpit::xbsettrap(const ArgumentArray &args) {
	// Set the Ytram trap

	// We can catch the Ytram between 10 seconds and 3 minutes from now
	uint32 timeUntilCatch = _vm->_rnd->getRandomNumberRng(10, 60 * 3) * 1000;
	_vm->_vars["bytramtime"] = timeUntilCatch + _vm->getTotalPlayTime();

	// And set the timer too
	installTimer(TIMER(BSpit, ytramTrapTimer), timeUntilCatch);
}

void BSpit::checkYtramCatch(bool playSound) {
	// Check if we've caught a Ytram

	uint32 &ytramTime = _vm->_vars["bytramtime"];

	// The trap has been moved back up.
	// You can't catch those sneaky Ytrams that way.
	if (ytramTime == 0) {
		return;
	}

	// If the trap still has not gone off, reinstall our timer
	// This is in case you set the trap, walked away, and returned
	if (_vm->getTotalPlayTime() < ytramTime) {
		installTimer(TIMER(BSpit, ytramTrapTimer), ytramTime - _vm->getTotalPlayTime());
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

void BSpit::xbcheckcatch(const ArgumentArray &args) {
	// Just pass our parameter along...
	checkYtramCatch(args[0] != 0);
}

void BSpit::xbait(const ArgumentArray &args) {
	// Set the cursor to the pellet
	_vm->_cursor->setCursor(kRivenPelletCursor);

	// Loop until the player lets go (or quits)
	while (mouseIsDown() && !_vm->hasGameEnded()) {
		_vm->doFrame();
	}

	// Set back the cursor
	_vm->_cursor->setCursor(kRivenMainCursor);

	RivenHotspot *bait = _vm->getCard()->getHotspotByBlstId(9);
	RivenHotspot *baitPlate = _vm->getCard()->getHotspotByBlstId(16);

	// Set the bait if we put it on the plate
	if (baitPlate->containsPoint(getMousePosition())) {
		_vm->_vars["bbait"] = 1;
		_vm->getCard()->drawPicture(4);

		bait->enable(false); // Disable bait hotspot
		baitPlate->enable(true); // Enable baitplate hotspot
	}
}

void BSpit::xbfreeytram(const ArgumentArray &args) {
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
			// The original did rand(13, 14)
			mlstId = _vm->_rnd->getRandomNumberRng(13, 15);
			break;
	}

	// Play the video
	_vm->getCard()->playMovie(mlstId);
	RivenVideo *first = _vm->_video->openSlot(11);
	first->playBlocking();

	// Now play the second movie
	_vm->getCard()->playMovie(mlstId + 5);
	RivenVideo *second = _vm->_video->openSlot(12);
	second->playBlocking();

	_vm->getCard()->drawPicture(4);
}

void BSpit::xbaitplate(const ArgumentArray &args) {
	// Remove the pellet from the plate and put it in your hand
	_vm->_cursor->setCursor(kRivenPelletCursor);
	_vm->getCard()->drawPicture(3);

	// Loop until the player lets go (or quits)
	while (mouseIsDown() && !_vm->hasGameEnded()) {
		_vm->doFrame();
	}

	// Set back the cursor
	_vm->_cursor->setCursor(kRivenMainCursor);

	RivenHotspot *bait = _vm->getCard()->getHotspotByBlstId(9);
	RivenHotspot *baitPlate = _vm->getCard()->getHotspotByBlstId(16);

	// Set the bait if we put it on the plate, remove otherwise
	if (baitPlate->containsPoint(getMousePosition())) {
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

void BSpit::xbisland190_opencard(const ArgumentArray &args) {
	checkDomeSliders();
}

void BSpit::xbisland190_resetsliders(const ArgumentArray &args) {
	resetDomeSliders(9);
}

void BSpit::xbisland190_slidermd(const ArgumentArray &args) {
	dragDomeSlider(9);
}

void BSpit::xbisland190_slidermw(const ArgumentArray &args) {
	checkSliderCursorChange(9);
}

void BSpit::xbscpbtn(const ArgumentArray &args) {
	runDomeButtonMovie();
}

void BSpit::xbisland_domecheck(const ArgumentArray &args) {
	runDomeCheck();
}

void BSpit::xvalvecontrol(const ArgumentArray &args) {
	Common::Point startPos = getMouseDragStartPosition();

	// Set the cursor to the closed position
	_vm->_cursor->setCursor(kRivenClosedHandCursor);

	while (mouseIsDown()) {
		Common::Point mousePos = getMousePosition();
		int changeX = mousePos.x - startPos.x;
		int changeY = startPos.y - mousePos.y;

		// Get the variable for the valve
		uint32 valve = _vm->_vars["bvalve"];

		// FIXME: These values for changes in x/y could be tweaked.
		if (valve == 0 && changeY <= -10) {
			valveChangePosition(1, 2, 2);
		} else if (valve == 1) {
			if (changeX >= 0 && changeY >= 10) {
				valveChangePosition(0, 3, 1);
			} else if (changeX <= -10 && changeY <= 10) {
				valveChangePosition(2, 1, 3);
			}
		} else if (valve == 2 && changeX >= 10) {
			valveChangePosition(1, 4, 2);
		}

		_vm->doFrame();
	}
}

void BSpit::valveChangePosition(uint32 valvePosition, uint16 videoId, uint16 pictureId) {
	RivenVideo *video = _vm->_video->openSlot(videoId);
	video->seek(0);
	video->playBlocking();

	_vm->getCard()->drawPicture(pictureId);

	// If we changed state and the new state is that the valve is flowing to
	// the boiler, we need to update the boiler state.
	if (valvePosition == 1) {
		// Check which way the water is going at the boiler
		if (_vm->_vars["bidvlv"] == 1) {
			if (_vm->_vars["bblrarm"] == 1 && _vm->_vars["bblrwtr"] == 1) {
				// If the pipe is open, make sure the water is drained out
				_vm->_vars["bheat"] = 0;
				_vm->_vars["bblrwtr"] = 0;
				_vm->_sound->playCardSound("bBlrFar");
			}

			if (_vm->_vars["bblrarm"] == 0 && _vm->_vars["bblrwtr"] == 0) {
				// If the pipe is closed, fill the boiler again
				_vm->_vars["bheat"] = _vm->_vars["bblrvalve"];
				_vm->_vars["bblrwtr"] = 1;
				_vm->_sound->playCardSound("bBlrFar");
			}
		} else {
			// Have the grating inside the boiler match the switch outside
			_vm->_vars["bblrgrt"] = (_vm->_vars["bblrsw"] == 1) ? 0 : 1;
		}
	}

	_vm->_vars["bvalve"] = valvePosition;
}

void BSpit::xbchipper(const ArgumentArray &args) {
	Common::Point startPos = getMouseDragStartPosition();

	bool pulledLever = false;
	while (mouseIsDown() && !_vm->hasGameEnded()) {
		Common::Point pos = getMousePosition();
		if (pos.y > startPos.y) {
			pulledLever = true;
			break;
		}

		_vm->doFrame();
	}

	if (pulledLever) {
		RivenVideo *video = _vm->_video->openSlot(2);
		video->seek(0);
		video->playBlocking();
	}
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
