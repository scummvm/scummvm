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

#include "mohawk/riven_stacks/jspit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"

#include "common/events.h"

namespace Mohawk {
namespace RivenStacks {

JSpit::JSpit(MohawkEngine_Riven *vm) :
		DomeSpit(vm, kStackJspit, "jsliders.190", "jsliderbg.190") {

	REGISTER_COMMAND(JSpit, xreseticons);
	REGISTER_COMMAND(JSpit, xicon);
	REGISTER_COMMAND(JSpit, xcheckicons);
	REGISTER_COMMAND(JSpit, xtoggleicon);
	REGISTER_COMMAND(JSpit, xjtunnel103_pictfix);
	REGISTER_COMMAND(JSpit, xjtunnel104_pictfix);
	REGISTER_COMMAND(JSpit, xjtunnel105_pictfix);
	REGISTER_COMMAND(JSpit, xjtunnel106_pictfix);
	REGISTER_COMMAND(JSpit, xvga1300_carriage);
	REGISTER_COMMAND(JSpit, xjdome25_resetsliders);
	REGISTER_COMMAND(JSpit, xjdome25_slidermd);
	REGISTER_COMMAND(JSpit, xjdome25_slidermw);
	REGISTER_COMMAND(JSpit, xjscpbtn);
	REGISTER_COMMAND(JSpit, xjisland3500_domecheck);
	REGISTER_COMMAND(JSpit, xhandlecontroldown);
	REGISTER_COMMAND(JSpit, xhandlecontrolmid);
	REGISTER_COMMAND(JSpit, xhandlecontrolup);
	REGISTER_COMMAND(JSpit, xjplaybeetle_550);
	REGISTER_COMMAND(JSpit, xjplaybeetle_600);
	REGISTER_COMMAND(JSpit, xjplaybeetle_950);
	REGISTER_COMMAND(JSpit, xjplaybeetle_1050);
	REGISTER_COMMAND(JSpit, xjplaybeetle_1450);
	REGISTER_COMMAND(JSpit, xjlagoon700_alert);
	REGISTER_COMMAND(JSpit, xjlagoon800_alert);
	REGISTER_COMMAND(JSpit, xjlagoon1500_alert);
	REGISTER_COMMAND(JSpit, xschool280_playwhark);
	REGISTER_COMMAND(JSpit, xjschool280_resetleft);
	REGISTER_COMMAND(JSpit, xjschool280_resetright);
	REGISTER_COMMAND(JSpit, xjatboundary);
}

void JSpit::xreseticons(uint16 argc, uint16 *argv) {
	// Reset the icons when going to Tay (rspit)
	_vm->_vars["jicons"] = 0;
	_vm->_vars["jiconorder"] = 0;
	_vm->_vars["jrbook"] = 0;
}

// Count up how many icons are pressed
static byte countDepressedIcons(uint32 iconOrderVar) {
	if (iconOrderVar >= (1 << 20))
		return 5;
	else if (iconOrderVar >= (1 << 15))
		return 4;
	else if (iconOrderVar >= (1 << 10))
		return 3;
	else if (iconOrderVar >= (1 << 5))
		return 2;
	else if (iconOrderVar >= (1 << 1))
		return 1;
	else
		return 0;
}

void JSpit::xicon(uint16 argc, uint16 *argv) {
	// Set atemp as the status of whether or not the icon can be depressed.
	if (_vm->_vars["jicons"] & (1 << (argv[0] - 1))) {
		// This icon is depressed. Allow depression only if the last depressed icon was this one.
		if ((_vm->_vars["jiconorder"] & 0x1f) == argv[0])
			_vm->_vars["atemp"] = 1;
		else
			_vm->_vars["atemp"] = 2;
	} else
		_vm->_vars["atemp"] = 0;
}

void JSpit::xcheckicons(uint16 argc, uint16 *argv) {
	// Reset the icons if this is the sixth icon
	uint32 &iconOrderVar = _vm->_vars["jiconorder"];
	if (countDepressedIcons(iconOrderVar) == 5) {
		iconOrderVar = 0;
		_vm->_vars["jicons"] = 0;
		_vm->_sound->playSound(46);
	}
}

void JSpit::xtoggleicon(uint16 argc, uint16 *argv) {
	// Get the variables
	uint32 &iconsDepressed = _vm->_vars["jicons"];
	uint32 &iconOrderVar = _vm->_vars["jiconorder"];

	if (iconsDepressed & (1 << (argv[0] - 1))) {
		// The icon is depressed, now unpress it
		iconsDepressed &= ~(1 << (argv[0] - 1));
		iconOrderVar >>= 5;
	} else {
		// The icon is not depressed, now depress it
		iconsDepressed |= 1 << (argv[0] - 1);
		iconOrderVar = (iconOrderVar << 5) + argv[0];
	}

	// Check if the puzzle is complete now and assign 1 to jrbook if the puzzle is complete.
	if (iconOrderVar == _vm->_vars["jiconcorrectorder"])
		_vm->_vars["jrbook"] = 1;
}

void JSpit::xjtunnel103_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 0))
		_vm->getCard()->drawPicture(2);
	if (iconsDepressed & (1 << 1))
		_vm->getCard()->drawPicture(3);
	if (iconsDepressed & (1 << 2))
		_vm->getCard()->drawPicture(4);
	if (iconsDepressed & (1 << 3))
		_vm->getCard()->drawPicture(5);
	if (iconsDepressed & (1 << 22))
		_vm->getCard()->drawPicture(6);
	if (iconsDepressed & (1 << 23))
		_vm->getCard()->drawPicture(7);
	if (iconsDepressed & (1 << 24))
		_vm->getCard()->drawPicture(8);
}

void JSpit::xjtunnel104_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 9))
		_vm->getCard()->drawPicture(2);
	if (iconsDepressed & (1 << 10))
		_vm->getCard()->drawPicture(3);
	if (iconsDepressed & (1 << 11))
		_vm->getCard()->drawPicture(4);
	if (iconsDepressed & (1 << 12))
		_vm->getCard()->drawPicture(5);
	if (iconsDepressed & (1 << 13))
		_vm->getCard()->drawPicture(6);
	if (iconsDepressed & (1 << 14))
		_vm->getCard()->drawPicture(7);
	if (iconsDepressed & (1 << 15))
		_vm->getCard()->drawPicture(8);
	if (iconsDepressed & (1 << 16))
		_vm->getCard()->drawPicture(9);
}

void JSpit::xjtunnel105_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 3))
		_vm->getCard()->drawPicture(2);
	if (iconsDepressed & (1 << 4))
		_vm->getCard()->drawPicture(3);
	if (iconsDepressed & (1 << 5))
		_vm->getCard()->drawPicture(4);
	if (iconsDepressed & (1 << 6))
		_vm->getCard()->drawPicture(5);
	if (iconsDepressed & (1 << 7))
		_vm->getCard()->drawPicture(6);
	if (iconsDepressed & (1 << 8))
		_vm->getCard()->drawPicture(7);
	if (iconsDepressed & (1 << 9))
		_vm->getCard()->drawPicture(8);
}

void JSpit::xjtunnel106_pictfix(uint16 argc, uint16 *argv) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	if (iconsDepressed & (1 << 16))
		_vm->getCard()->drawPicture(2);
	if (iconsDepressed & (1 << 17))
		_vm->getCard()->drawPicture(3);
	if (iconsDepressed & (1 << 18))
		_vm->getCard()->drawPicture(4);
	if (iconsDepressed & (1 << 19))
		_vm->getCard()->drawPicture(5);
	if (iconsDepressed & (1 << 20))
		_vm->getCard()->drawPicture(6);
	if (iconsDepressed & (1 << 21))
		_vm->getCard()->drawPicture(7);
	if (iconsDepressed & (1 << 22))
		_vm->getCard()->drawPicture(8);
	if (iconsDepressed & (1 << 23))
		_vm->getCard()->drawPicture(9);
}

void JSpit::xvga1300_carriage(uint16 argc, uint16 *argv) {
	// Run the gallows's carriage

	_vm->_cursor->setCursor(kRivenHideCursor);         // Hide the cursor
	_vm->_system->updateScreen();                      // Update
	_vm->_video->playMovieBlockingRiven(1);            // Play handle movie
	_vm->_gfx->scheduleTransition(kRivenTransitionPanDown);
	_vm->changeToCard(_vm->getStack()->getCardStackId(0x18e77));  // Change to card facing up
	_vm->_cursor->setCursor(kRivenHideCursor);         // Hide the cursor (again)
	_vm->_system->updateScreen();                      // Update
	_vm->_video->playMovieBlockingRiven(4);            // Play carriage beginning to drop
	_vm->_gfx->scheduleTransition(kRivenTransitionPanUp);
	_vm->changeToCard(_vm->getStack()->getCardStackId(0x183a9));  // Change to card looking straight again
	_vm->_video->playMovieBlockingRiven(2);

	if (_vm->_vars["jgallows"] == 1) {
		// If the gallows is open, play the up movie and return
		_vm->_video->playMovieBlockingRiven(3);
		return;
	}

	// Give the player 5 seconds to click (anywhere)
	uint32 startTime = _vm->_system->getMillis();
	bool gotClick = false;
	while (_vm->_system->getMillis() - startTime <= 5000 && !gotClick) {
		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					_vm->_system->updateScreen();
					break;
				case Common::EVENT_LBUTTONUP:
					gotClick = true;
					break;
				default:
					break;
			}
		}

		_vm->_system->delayMillis(10);
	}

	_vm->_cursor->setCursor(kRivenHideCursor);             // Hide the cursor
	_vm->_system->updateScreen();                          // Update

	if (gotClick) {
		_vm->_gfx->scheduleTransition(kRivenTransitionBlend);
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x18d4d));  // Move forward
		_vm->_cursor->setCursor(kRivenHideCursor);         // Hide the cursor
		_vm->_system->updateScreen();                      // Update
		_vm->_system->delayMillis(500);                    // Delay a half second before changing again
		_vm->_gfx->scheduleTransition(kRivenTransitionPanLeft);
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x18ab5));  // Turn right
		_vm->_cursor->setCursor(kRivenHideCursor);         // Hide the cursor
		_vm->_system->updateScreen();                      // Update
		_vm->_video->playMovieBlockingRiven(1);            // Play carriage ride movie
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x17167));  // We have arrived at the top
	} else
		_vm->_video->playMovieBlockingRiven(3);            // Too slow!
}

void JSpit::xjdome25_resetsliders(uint16 argc, uint16 *argv) {
	resetDomeSliders(81, 10);
}

void JSpit::xjdome25_slidermd(uint16 argc, uint16 *argv) {
	dragDomeSlider(81, 10);
}

void JSpit::xjdome25_slidermw(uint16 argc, uint16 *argv) {
	checkSliderCursorChange(10);
}

void JSpit::xjscpbtn(uint16 argc, uint16 *argv) {
	runDomeButtonMovie();
}

void JSpit::xjisland3500_domecheck(uint16 argc, uint16 *argv) {
	runDomeCheck();
}

int JSpit::jspitElevatorLoop() {
	Common::Point startPos = _vm->_system->getEventManager()->getMousePos();

	Common::Event event;
	int changeLevel = 0;

	_vm->_cursor->setCursor(kRivenClosedHandCursor);
	_vm->_system->updateScreen();

	for (;;) {
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					if (event.mouse.y > (startPos.y + 10)) {
						changeLevel = -1;
					} else if (event.mouse.y < (startPos.y - 10)) {
						changeLevel = 1;
					} else {
						changeLevel = 0;
					}
					_vm->_system->updateScreen();
					break;
				case Common::EVENT_LBUTTONUP:
					_vm->_cursor->setCursor(kRivenMainCursor);
					_vm->_system->updateScreen();
					return changeLevel;
				default:
					break;
			}
		}
		_vm->_system->delayMillis(10);
	}
}

void JSpit::xhandlecontrolup(uint16 argc, uint16 *argv) {
	int changeLevel = jspitElevatorLoop();

	// If we've moved the handle down, go down a floor
	if (changeLevel == -1) {
		_vm->_video->playMovieBlockingRiven(1);
		_vm->_video->playMovieBlockingRiven(2);
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x1e374));
	}
}

void JSpit::xhandlecontroldown(uint16 argc, uint16 *argv) {
	int changeLevel = jspitElevatorLoop();

	// If we've moved the handle up, go up a floor
	if (changeLevel == 1) {
		_vm->_video->playMovieBlockingRiven(1);
		_vm->_video->playMovieBlockingRiven(2);
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x1e374));
	}
}

void JSpit::xhandlecontrolmid(uint16 argc, uint16 *argv) {
	int changeLevel = jspitElevatorLoop();

	if (changeLevel == 0)
		return;

	// Play the handle moving video
	if (changeLevel == 1)
		_vm->_video->playMovieBlockingRiven(7);
	else
		_vm->_video->playMovieBlockingRiven(6);

	// If the whark's mouth is open, close it
	uint32 &mouthVar = _vm->_vars["jwmouth"];
	if (mouthVar == 1) {
		_vm->_video->playMovieBlockingRiven(3);
		_vm->_video->playMovieBlockingRiven(8);
		mouthVar = 0;
	}

	// Play the elevator video and then change the card
	if (changeLevel == 1) {
		_vm->_video->playMovieBlockingRiven(5);
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x1e597));
	} else {
		_vm->_video->playMovieBlockingRiven(4);
		_vm->changeToCard(_vm->getStack()->getCardStackId(0x1e29c));
	}
}

void JSpit::xjplaybeetle_550(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_600(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_950(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_1050(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_1450(uint16 argc, uint16 *argv) {
	// Play a beetle animation 25% of the time as long as the girl is not present
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0 && _vm->_vars["jgirl"] != 1) ? 1 : 0;
}

void JSpit::xjlagoon700_alert(uint16 argc, uint16 *argv) {
	// Handle sunner reactions (mid-staircase)
	uint32 sunners = _vm->_vars["jsunners"];

	// If the sunners are gone, there's nothing for us to do
	if (sunners != 0) {
		return;
	}

	VideoEntryPtr sunnerAlertVideo = _vm->_video->playMovieRiven(1);

	// Wait for a click while the alert video is playing
	sunnersPlayVideo(sunnerAlertVideo, 0x7BEB);
}

void JSpit::xjlagoon800_alert(uint16 argc, uint16 *argv) {
	// Handle sunner reactions (lower-staircase)

	uint32 &sunners = _vm->_vars["jsunners"];

	if (sunners == 0) {
		// Show the sunners alert video
		VideoEntryPtr sunnerAlertVideo = _vm->_video->playMovieRiven(1);

		// Wait for a click while the alert video is playing
		sunnersPlayVideo(sunnerAlertVideo, 0xB6CA);
	} else if (sunners == 1) {
		// Show the sunners leaving if you moved forward in their "alert" status
		_vm->_video->playMovieBlockingRiven(2);
		_vm->_video->playMovieBlockingRiven(6);
		sunners = 2;
		_vm->refreshCard();
	}
}

void JSpit::xjlagoon1500_alert(uint16 argc, uint16 *argv) {
	// Handle sunner reactions (beach)

	uint32 &sunners = _vm->_vars["jsunners"];

	if (sunners == 0) {
		// Show the sunners alert video
		_vm->_video->playMovieBlockingRiven(3);
	} else if (sunners == 1) {
		// Show the sunners leaving if you moved forward in their "alert" status
		_vm->_video->playMovieBlockingRiven(2);
		sunners = 2;
		_vm->refreshCard();
	}
}

void JSpit::sunnersPlayVideo(VideoEntryPtr &video, uint32 destCardGlobalId) {
	uint32 &sunners = _vm->_vars["jsunners"];

	mouseForceUp();
	while (!video->endOfVideo() && !_vm->shouldQuit()) {
		_vm->doFrame();

		if (mouseIsDown()) {
			video->stop();
			sunners = 1;

			uint16 destCardId = getCardStackId(destCardGlobalId);
			RivenScriptPtr clickScript = _vm->_scriptMan->createScriptFromData(1, kRivenCommandChangeCard, 1, destCardId);
			_vm->_scriptMan->runScript(clickScript, false);
			break;
		}
	}
}

void JSpit::sunnersTopStairsTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		_vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldVideo = _vm->_video->findVideoRiven(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(2, 15) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			VideoEntryPtr video = _vm->_video->playMovieRiven(_vm->_rnd->getRandomNumberRng(1, 3));

			timerTime = video->getDuration().msecs() + _vm->_rnd->getRandomNumberRng(2, 15) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	_vm->installTimer(TIMER(JSpit, sunnersTopStairsTimer), timerTime);
}

void JSpit::sunnersMidStairsTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		_vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldVideo = _vm->_video->findVideoRiven(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(1, 10) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			// Randomize the video
			int randValue = _vm->_rnd->getRandomNumber(5);
			uint16 movie = 4;
			if (randValue == 4)
				movie = 2;
			else if (randValue == 5)
				movie = 3;

			VideoEntryPtr video = _vm->_video->playMovieRiven(movie);

			timerTime = video->getDuration().msecs() + _vm->_rnd->getRandomNumberRng(1, 10) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	_vm->installTimer(TIMER(JSpit, sunnersMidStairsTimer), timerTime);
}

void JSpit::sunnersLowerStairsTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		_vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldVideo = _vm->_video->findVideoRiven(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			VideoEntryPtr video = _vm->_video->playMovieRiven(_vm->_rnd->getRandomNumberRng(3, 5));

			timerTime = video->getDuration().msecs() + _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	_vm->installTimer(TIMER(JSpit, sunnersLowerStairsTimer), timerTime);
}

void JSpit::sunnersBeachTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		_vm->removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	VideoEntryPtr oldvideo = _vm->_video->findVideoRiven(3);
	uint32 timerTime = 500;

	if (!oldvideo || oldvideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			// Unlike the other cards' scripts which automatically
			// activate the MLST, we have to set it manually here.
			uint16 mlstID = _vm->_rnd->getRandomNumberRng(3, 8);
			_vm->_video->activateMLST(_vm->getCard()->getMovie(mlstID));
			VideoEntryPtr video = _vm->_video->playMovieRiven(mlstID);

			timerTime = video->getDuration().msecs() + _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	_vm->installTimer(TIMER(JSpit, sunnersBeachTimer), timerTime);
}

void JSpit::xjschool280_resetleft(uint16 argc, uint16 *argv) {
	// Dummy function. This resets the unneeded video timing variable (dropLeftStart) in
	// the DVD version.
}

void JSpit::xjschool280_resetright(uint16 argc, uint16 *argv) {
	// Dummy function. This resets the unneeded video timing variable (dropRightStart) in
	// the DVD version.
}

void JSpit::redrawWharkNumberPuzzle(uint16 overlay, uint16 number) {
	// Update the screen for the whark number puzzle
	// We don't update the whole screen here because we don't want to overwrite the video data
	_vm->getCard()->drawPicture(overlay);
	_vm->getCard()->drawPicture(number + 1);
	_vm->_gfx->updateScreen(Common::Rect(80, 212, 477, 392));
	_vm->_system->updateScreen();
}

void JSpit::xschool280_playwhark(uint16 argc, uint16 *argv) {
	// The "monstrous" whark puzzle that teaches the number system

	uint32 *posVar;
	uint16 spinMLST, overlayPLST, doomMLST, snackMLST;

	// Choose left or right based on jwharkpos (which is set by the scripts)
	if (_vm->_vars["jwharkpos"] == 1) {
		posVar = &_vm->_vars["jleftpos"];
		spinMLST = 1;
		overlayPLST = 12;
		doomMLST = 3;
		snackMLST = 4;
	} else {
		posVar = &_vm->_vars["jrightpos"];
		spinMLST = 2;
		overlayPLST = 13;
		doomMLST = 5;
		snackMLST = 6;
	}

	// Hide the cursor
	_vm->_cursor->setCursor(kRivenHideCursor);
	_vm->_system->updateScreen();

	// Play the spin movie
	_vm->_video->playMovieBlockingRiven(spinMLST);

	// Get our random number and redraw the area
	uint16 number = _vm->_rnd->getRandomNumberRng(1, 10);
	redrawWharkNumberPuzzle(overlayPLST, number);

	// Handle movement
	// (11560/600)s is the length of each of the two movies. We divide it into 19 parts
	// (one for each of the possible positions the villager can have).
	VideoEntryPtr handle = _vm->_video->playMovieRiven(doomMLST);
	Audio::Timestamp startTime = Audio::Timestamp(0, (11560 / 19) * (*posVar), 600);
	*posVar += number; // Adjust to the end
	Audio::Timestamp endTime = Audio::Timestamp(0, (11560 / 19) * (*posVar), 600);
	handle->setBounds(startTime, endTime);
	_vm->_video->waitUntilMovieEnds(handle);

	if (*posVar > 19) {
		// The villager has died :(
		_vm->_video->playMovieBlockingRiven(snackMLST);
		redrawWharkNumberPuzzle(overlayPLST, number);
		*posVar = 0;
	}

	// Enable the correct hotspots for the movement now
	RivenHotspot *rotateLeft = _vm->getCard()->getHotspotByName("rotateLeft");
	RivenHotspot *rotateRight = _vm->getCard()->getHotspotByName("rotateRight");
	rotateLeft->enable(!rotateLeft->isEnabled());
	rotateRight->enable(!rotateRight->isEnabled());

	// Update the cursor
	_vm->updateCurrentHotspot();
}

void JSpit::xjatboundary(uint16 argc, uint16 *argv) {
	runDemoBoundaryDialog();
}

void JSpit::installCardTimer() {
	switch (getCurrentCardGlobalId()) {
		case 0x77d6: // Sunners, top of stairs
			_vm->installTimer(TIMER(JSpit, sunnersTopStairsTimer), 500);
			break;
		case 0x79bd: // Sunners, middle of stairs
			_vm->installTimer(TIMER(JSpit, sunnersMidStairsTimer), 500);
			break;
		case 0x7beb: // Sunners, bottom of stairs
			_vm->installTimer(TIMER(JSpit, sunnersLowerStairsTimer), 500);
			break;
		case 0xb6ca: // Sunners, shoreline
			_vm->installTimer(TIMER(JSpit, sunnersBeachTimer), 500);
			break;
		default:
			RivenStack::installCardTimer();
	}
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
