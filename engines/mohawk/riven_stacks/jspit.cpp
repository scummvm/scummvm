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

void JSpit::xreseticons(const ArgumentArray &args) {
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

void JSpit::xicon(const ArgumentArray &args) {
	// Set atemp as the status of whether or not the icon can be depressed.
	if (_vm->_vars["jicons"] & (1 << (args[0] - 1))) {
		// This icon is depressed. Allow depression only if the last depressed icon was this one.
		if ((_vm->_vars["jiconorder"] & 0x1f) == args[0])
			_vm->_vars["atemp"] = 1;
		else
			_vm->_vars["atemp"] = 2;
	} else
		_vm->_vars["atemp"] = 0;
}

void JSpit::xcheckicons(const ArgumentArray &args) {
	// Reset the icons if this is the sixth icon
	uint32 &iconOrderVar = _vm->_vars["jiconorder"];
	if (countDepressedIcons(iconOrderVar) == 5) {
		iconOrderVar = 0;
		_vm->_vars["jicons"] = 0;
		_vm->_sound->playSound(46);

		// Wait until the stones have finished rising
		while (_vm->_sound->isEffectPlaying() && !_vm->hasGameEnded()) {
			_vm->doFrame();
		}
	}
}

void JSpit::xtoggleicon(const ArgumentArray &args) {
	// Get the variables
	uint32 &iconsDepressed = _vm->_vars["jicons"];
	uint32 &iconOrderVar = _vm->_vars["jiconorder"];

	if (iconsDepressed & (1 << (args[0] - 1))) {
		// The icon is depressed, now unpress it
		iconsDepressed &= ~(1 << (args[0] - 1));
		iconOrderVar >>= 5;
	} else {
		// The icon is not depressed, now depress it
		iconsDepressed |= 1 << (args[0] - 1);
		iconOrderVar = (iconOrderVar << 5) + args[0];
	}

	// Check if the puzzle is complete now and assign 1 to jrbook if the puzzle is complete.
	if (iconOrderVar == _vm->_vars["jiconcorrectorder"])
		_vm->_vars["jrbook"] = 1;
}

void JSpit::xjtunnel103_pictfix(const ArgumentArray &args) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	_vm->_gfx->beginScreenUpdate();

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

	_vm->_gfx->applyScreenUpdate();
}

void JSpit::xjtunnel104_pictfix(const ArgumentArray &args) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	_vm->_gfx->beginScreenUpdate();

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

	_vm->_gfx->applyScreenUpdate();
}

void JSpit::xjtunnel105_pictfix(const ArgumentArray &args) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	_vm->_gfx->beginScreenUpdate();

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

	_vm->_gfx->applyScreenUpdate();
}

void JSpit::xjtunnel106_pictfix(const ArgumentArray &args) {
	// Get the jicons variable which contains which of the stones are depressed in the rebel tunnel puzzle
	uint32 iconsDepressed = _vm->_vars["jicons"];

	// Now, draw which icons are depressed based on the bits of the variable
	_vm->_gfx->beginScreenUpdate();

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

	_vm->_gfx->applyScreenUpdate();
}

void JSpit::xvga1300_carriage(const ArgumentArray &args) {
	// Run the gallows's carriage

	RivenVideo *handleVideo = _vm->_video->openSlot(1);
	handleVideo->playBlocking();

	_vm->_gfx->beginScreenUpdate();
	_vm->_gfx->scheduleTransition(kRivenTransitionPanDown);
	_vm->getCard()->drawPicture(7);
	_vm->_gfx->enableCardUpdateScript(false);
	_vm->_gfx->applyScreenUpdate();
	_vm->_gfx->enableCardUpdateScript(true);

	// Play carriage beginning to drop
	RivenVideo *beginDropVideo = _vm->_video->openSlot(4);
	beginDropVideo->playBlocking();

	_vm->_gfx->beginScreenUpdate();
	_vm->_gfx->scheduleTransition(kRivenTransitionPanUp);
	_vm->getCard()->drawPicture(1);
	_vm->_gfx->applyScreenUpdate();

	_vm->_cursor->setCursor(kRivenMainCursor);
	mouseForceUp();

	if (_vm->_vars["jgallows"] == 1) {
		RivenVideo *video = _vm->_video->openSlot(2);
		video->playBlocking();

		_vm->delay(5000);

		// If the gallows is open, play the up movie and return
		RivenVideo *upVideo = _vm->_video->openSlot(3);
		upVideo->playBlocking();
		_vm->getCard()->enter(false);
		return;
	}

	bool gotClick = false;

	RivenVideo *video = _vm->_video->openSlot(2);
	video->enable();
	video->play();
	while (!video->endOfVideo()) {
		_vm->doFrame();

		if (mouseIsDown()) {
			gotClick = true;
		}
	}
	video->disable();

	if (gotClick) {
		_vm->_cursor->hideCursor();
	}

	// Give the player 5 seconds to click (anywhere)
	uint32 startTime = _vm->_system->getMillis();
	while (_vm->_system->getMillis() - startTime <= 5000 && !gotClick) {
		_vm->doFrame();

		if (mouseIsDown()) {
			gotClick = true;
			_vm->_cursor->hideCursor();
		}
	}

	if (gotClick) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(3,
                          kRivenCommandChangeCard, 1, getCardStackId(0x18D4D),
                          kRivenCommandTransition, 1, kRivenTransitionPanLeft,
                          kRivenCommandChangeCard, 1, getCardStackId(0x18AB5));
		_vm->_scriptMan->runScript(script, false);

// FIXME: kRivenCommandStoreMovieOpcode takes more arguments.. does it work with the original engine?
//		script = _vm->_scriptMan->createScriptFromData(1,
//		                  kRivenCommandStoreMovieOpcode, 2, kRivenCommandActivateSLST, 2);
//		_vm->_scriptMan->runScript(script, false);

		// Play carriage ride movie
		RivenVideo *rideVideo = _vm->_video->openSlot(1);
		rideVideo->playBlocking();

		// We have arrived at the top
		script = _vm->_scriptMan->createScriptFromData(1,
		                  kRivenCommandChangeCard, 1, getCardStackId(0x17167));
		_vm->_scriptMan->runScript(script, false);

		_vm->_cursor->showCursor();
	} else {
		// Too slow!
		RivenVideo *tooSlowVideo = _vm->_video->openSlot(3);
		tooSlowVideo->playBlocking();
		_vm->getCard()->enter(false);
	}
}

void JSpit::xjdome25_resetsliders(const ArgumentArray &args) {
	resetDomeSliders(10);
}

void JSpit::xjdome25_slidermd(const ArgumentArray &args) {
	dragDomeSlider(10);
}

void JSpit::xjdome25_slidermw(const ArgumentArray &args) {
	checkSliderCursorChange(10);
}

void JSpit::xjscpbtn(const ArgumentArray &args) {
	runDomeButtonMovie();
}

void JSpit::xjisland3500_domecheck(const ArgumentArray &args) {
	runDomeCheck();
}

int JSpit::jspitElevatorLoop() {
	Common::Point startPos = getMouseDragStartPosition();

	_vm->_cursor->setCursor(kRivenClosedHandCursor);

	while (mouseIsDown() && !_vm->hasGameEnded()) {
		_vm->doFrame();

		Common::Point pos = getMousePosition();

		if (pos.y > (startPos.y + 10)) {
			return -1;
		} else if (pos.y < (startPos.y - 10)) {
			return 1;
		}
	}

	return 0;
}

void JSpit::xhandlecontrolup(const ArgumentArray &args) {
	int changeLevel = jspitElevatorLoop();

	// If we've moved the handle down, go down a floor
	if (changeLevel == -1) {
		RivenVideo *firstVideo = _vm->_video->openSlot(1);
		firstVideo->playBlocking();

		_vm->_cursor->hideCursor();
		RivenVideo *secondVideo = _vm->_video->openSlot(2);
		secondVideo->enable();
		secondVideo->play();

		// TODO: Maybe queue a sound using the stored movie opcode instead
		bool playedSound = false;
		while (!secondVideo->endOfVideo() && !_vm->hasGameEnded()) {
			_vm->doFrame();

			if (!playedSound && secondVideo->getTime() > 3333) {
				_vm->getCard()->playSound(1, false);
				playedSound = true;
			}
		}

		secondVideo->disable();
		_vm->_cursor->showCursor();

		RivenScriptPtr changeCard = _vm->_scriptMan->createScriptFromData(1, kRivenCommandChangeCard, 1, getCardStackId(0x1e374));
		_vm->_scriptMan->runScript(changeCard, false);
	}
}

void JSpit::xhandlecontroldown(const ArgumentArray &args) {
	int changeLevel = jspitElevatorLoop();

	// If we've moved the handle up, go up a floor
	if (changeLevel == 1) {
		RivenVideo *firstVideo = _vm->_video->openSlot(1);
		firstVideo->playBlocking();
		RivenVideo *secondVideo = _vm->_video->openSlot(2);
		secondVideo->playBlocking();

		RivenScriptPtr changeCard = _vm->_scriptMan->createScriptFromData(1, kRivenCommandChangeCard, 1, getCardStackId(0x1e374));
		_vm->_scriptMan->runScript(changeCard, false);
	}
}

void JSpit::xhandlecontrolmid(const ArgumentArray &args) {
	int changeLevel = jspitElevatorLoop();

	if (changeLevel == 0)
		return;

	// Play the handle moving video
	RivenVideo *handleVideo;
	if (changeLevel == 1)
		handleVideo = _vm->_video->openSlot(7);
	else
		handleVideo = _vm->_video->openSlot(6);
	handleVideo->playBlocking();

	// If the whark's mouth is open, close it
	uint32 &mouthVar = _vm->_vars["jwmouth"];
	if (mouthVar == 1) {
		RivenVideo *closeVideo1 = _vm->_video->openSlot(3);
		closeVideo1->playBlocking();
		RivenVideo *closeVideo2 = _vm->_video->openSlot(8);
		closeVideo2->playBlocking();
		mouthVar = 0;
	}

	// Play the elevator video and then change the card
	uint16 newCardId;
	if (changeLevel == 1) {
		RivenVideo *elevatorVideo = _vm->_video->openSlot(5);
		elevatorVideo->playBlocking();
		newCardId = getCardStackId(0x1e597);
	} else {
		RivenVideo *elevatorVideo = _vm->_video->openSlot(4);
		elevatorVideo->playBlocking();
		newCardId = getCardStackId(0x1e29c);
	}

	RivenScriptPtr changeCard = _vm->_scriptMan->createScriptFromData(1, kRivenCommandChangeCard, 1, newCardId);
	_vm->_scriptMan->runScript(changeCard, false);
}

void JSpit::xjplaybeetle_550(const ArgumentArray &args) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_600(const ArgumentArray &args) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_950(const ArgumentArray &args) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_1050(const ArgumentArray &args) {
	// Play a beetle animation 25% of the time
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0) ? 1 : 0;
}

void JSpit::xjplaybeetle_1450(const ArgumentArray &args) {
	// Play a beetle animation 25% of the time as long as the girl is not present
	_vm->_vars["jplaybeetle"] = (_vm->_rnd->getRandomNumberRng(0, 3) == 0 && _vm->_vars["jgirl"] != 1) ? 1 : 0;
}

void JSpit::xjlagoon700_alert(const ArgumentArray &args) {
	// Handle sunner reactions (mid-staircase)
	uint32 sunners = _vm->_vars["jsunners"];

	// If the sunners are gone, there's nothing for us to do
	if (sunners != 0) {
		return;
	}

	RivenVideo *sunnerAlertVideo = _vm->_video->openSlot(1);

	// Wait for a click while the alert video is playing
	sunnersPlayVideo(sunnerAlertVideo, 0x7BEB, true);
}

void JSpit::xjlagoon800_alert(const ArgumentArray &args) {
	// Handle sunner reactions (lower-staircase)

	uint32 &sunners = _vm->_vars["jsunners"];

	if (sunners == 0) {
		// Show the sunners alert video
		RivenVideo *sunnerAlertVideo = _vm->_video->openSlot(1);

		// Wait for a click while the alert video is playing
		sunnersPlayVideo(sunnerAlertVideo, 0xB6CA, true);
	} else if (sunners == 1) {
		// Show the sunners leaving if you moved forward in their "alert" status
		RivenVideo *leaving1 = _vm->_video->openSlot(2);
		leaving1->playBlocking();
		RivenVideo *leaving2 = _vm->_video->openSlot(6);
		leaving2->playBlocking();
		sunners = 2;
		_vm->getCard()->enter(false);
	}
}

void JSpit::xjlagoon1500_alert(const ArgumentArray &args) {
	// Handle sunner reactions (beach)

	uint32 &sunners = _vm->_vars["jsunners"];

	if (sunners == 0) {
		// Show the sunners alert video
		RivenVideo *alertVideo = _vm->_video->openSlot(3);
		alertVideo->playBlocking();
	} else if (sunners == 1) {
		// Show the sunners leaving if you moved forward in their "alert" status
		RivenVideo *leavingVideo = _vm->_video->openSlot(2);
		leavingVideo->playBlocking();
		sunners = 2;
		_vm->getCard()->enter(false);
	}
}

void JSpit::sunnersPlayVideo(RivenVideo *video, uint32 destCardGlobalId, bool sunnersShouldFlee) {
	uint32 &sunners = _vm->_vars["jsunners"];

	mouseForceUp();

	video->seek(0);
	video->enable();
	video->play();

	while (!video->endOfVideo() && !_vm->hasGameEnded()) {
		_vm->doFrame();

		if (mouseIsDown() || getAction() == kRivenActionMoveForward) {
			video->stop();

			if (sunnersShouldFlee) {
				sunners = 1;
			}

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
		removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	RivenVideo *oldVideo = _vm->_video->getSlot(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(2, 15) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			RivenVideo *video = _vm->_video->openSlot(_vm->_rnd->getRandomNumberRng(1, 3));
			sunnersPlayVideo(video, 0x79BD, false);

			timerTime = video->getDuration() + _vm->_rnd->getRandomNumberRng(2, 15) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	installTimer(TIMER(JSpit, sunnersTopStairsTimer), timerTime);
}

void JSpit::sunnersMidStairsTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	RivenVideo *oldVideo = _vm->_video->getSlot(1);
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

			RivenVideo *video = _vm->_video->openSlot(movie);
			sunnersPlayVideo(video, 0x7BEB, true);

			timerTime = _vm->_rnd->getRandomNumberRng(1, 10) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	installTimer(TIMER(JSpit, sunnersMidStairsTimer), timerTime);
}

void JSpit::sunnersLowerStairsTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	RivenVideo *oldVideo = _vm->_video->getSlot(1);
	uint32 timerTime = 500;

	if (!oldVideo || oldVideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			RivenVideo *video = _vm->_video->openSlot(_vm->_rnd->getRandomNumberRng(3, 5));
			sunnersPlayVideo(video,  0xB6CA, true);

			timerTime = _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	installTimer(TIMER(JSpit, sunnersLowerStairsTimer), timerTime);
}

void JSpit::sunnersBeachTimer() {
	// If the sunners are gone, we have no video to play
	if (_vm->_vars["jsunners"] != 0) {
		removeTimer();
		return;
	}

	// Play a random sunners video if the script one is not playing already
	// and then set a new timer for when the new video should be played

	RivenVideo *oldvideo = _vm->_video->getSlot(3);
	uint32 timerTime = 500;

	if (!oldvideo || oldvideo->endOfVideo()) {
		uint32 &sunnerTime = _vm->_vars["jsunnertime"];

		if (sunnerTime == 0) {
			timerTime = _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		} else if (sunnerTime < _vm->getTotalPlayTime()) {
			// Unlike the other cards' scripts which automatically
			// activate the MLST, we have to set it manually here.
			uint16 mlstID = _vm->_rnd->getRandomNumberRng(3, 8);
			_vm->getCard()->playMovie(mlstID);
			RivenVideo *video = _vm->_video->openSlot(mlstID);
			video->playBlocking();

			timerTime = _vm->_rnd->getRandomNumberRng(1, 30) * 1000;
		}

		sunnerTime = timerTime + _vm->getTotalPlayTime();
	}

	installTimer(TIMER(JSpit, sunnersBeachTimer), timerTime);
}

void JSpit::xjschool280_resetleft(const ArgumentArray &args) {
	// Dummy function. This resets the unneeded video timing variable (dropLeftStart) in
	// the DVD version.
}

void JSpit::xjschool280_resetright(const ArgumentArray &args) {
	// Dummy function. This resets the unneeded video timing variable (dropRightStart) in
	// the DVD version.
}

void JSpit::redrawWharkNumberPuzzle(uint16 overlay, uint16 number) {
	// Update the screen for the whark number puzzle
	_vm->_gfx->beginScreenUpdate();
	_vm->getCard()->drawPicture(overlay);
	_vm->getCard()->drawPicture(number + 1);
	_vm->_gfx->applyScreenUpdate();
}

void JSpit::xschool280_playwhark(const ArgumentArray &args) {
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

	// Play the spin movie
	RivenVideo *spinVideo = _vm->_video->openSlot(spinMLST);
	spinVideo->seek(0);
	spinVideo->playBlocking();

	// Get our random number and redraw the area
	uint16 number = _vm->_rnd->getRandomNumberRng(1, 10);
	redrawWharkNumberPuzzle(overlayPLST, number);

	// Handle movement
	// (11560/600)s is the length of each of the two movies. We divide it into 19 parts
	// (one for each of the possible positions the villager can have).
	RivenVideo *video = _vm->_video->openSlot(doomMLST);
	uint32 startTime = (11560 / 19) * (*posVar);
	*posVar += number; // Adjust to the end
	uint32 endTime = (11560 / 19) * (*posVar);
	video->enable();
	video->seek(startTime);
	video->playBlocking(endTime);
	video->disable();

	if (*posVar > 19) {
		// The villager has died :(
		RivenVideo *snackVideo = _vm->_video->openSlot(snackMLST);
		snackVideo->seek(0);
		snackVideo->playBlocking();
		redrawWharkNumberPuzzle(overlayPLST, number);
		*posVar = 0;
	}

	// Enable the correct hotspots for the movement now
	RivenHotspot *rotateLeft = _vm->getCard()->getHotspotByName("rotateLeft");
	RivenHotspot *rotateRight = _vm->getCard()->getHotspotByName("rotateRight");
	rotateLeft->enable(!rotateLeft->isEnabled());
	rotateRight->enable(!rotateRight->isEnabled());
}

void JSpit::xjatboundary(const ArgumentArray &args) {
	runDemoBoundaryDialog();
}

void JSpit::installCardTimer() {
	switch (getCurrentCardGlobalId()) {
		case 0x77d6: // Sunners, top of stairs
			installTimer(TIMER(JSpit, sunnersTopStairsTimer), 500);
			break;
		case 0x79bd: // Sunners, middle of stairs
			installTimer(TIMER(JSpit, sunnersMidStairsTimer), 500);
			break;
		case 0x7beb: // Sunners, bottom of stairs
			installTimer(TIMER(JSpit, sunnersLowerStairsTimer), 500);
			break;
		case 0xb6ca: // Sunners, shoreline
			installTimer(TIMER(JSpit, sunnersBeachTimer), 500);
			break;
		default:
			RivenStack::installCardTimer();
	}
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
