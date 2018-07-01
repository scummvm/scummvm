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

#include "mohawk/riven_stacks/rspit.h"

#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_inventory.h"
#include "mohawk/riven_video.h"

namespace Mohawk {
namespace RivenStacks {

RSpit::RSpit(MohawkEngine_Riven *vm) :
		RivenStack(vm, kStackRspit) {

	REGISTER_COMMAND(RSpit, xrshowinventory);
	REGISTER_COMMAND(RSpit, xrhideinventory);
	REGISTER_COMMAND(RSpit, xrcredittime);
	REGISTER_COMMAND(RSpit, xrwindowsetup);
}

void RSpit::xrcredittime(const ArgumentArray &args) {
	// Nice going, you used the trap book on Tay.

	// The game chooses what ending based on agehn for us,
	// so we just have to play the video and credits.
	// For the record, when agehn == 4, Gehn will thank you for
	// showing him the rebel age and then leave you to die.
	// Otherwise, the rebels burn the book. Epic fail either way.

	if (_vm->_vars["agehn"] == 4) {
		runEndGame(1, 1500, 712);
	} else {
		runEndGame(1, 1500, 0);
	}
}

void RSpit::xrshowinventory(const ArgumentArray &args) {
}

void RSpit::xrhideinventory(const ArgumentArray &args) {
}

void RSpit::rebelPrisonWindowTimer() {
	// Randomize a video out in the middle of Tay
	uint16 movie = _vm->_rnd->getRandomNumberRng(2, 13);
	_vm->getCard()->playMovie(movie);
	RivenVideo *video = _vm->_video->openSlot(movie);
	video->playBlocking();

	// Ensure the next video starts after this one ends
	uint32 timeUntilNextVideo = _vm->_rnd->getRandomNumberRng(38, 58) * 1000;

	// Save the time in case we leave the card and return
	_vm->_vars["rvillagetime"] = timeUntilNextVideo + _vm->getTotalPlayTime();

	// Reinstall this timer with the new time
	installTimer(TIMER(RSpit, rebelPrisonWindowTimer), timeUntilNextVideo);
}

void RSpit::xrwindowsetup(const ArgumentArray &args) {
	// Randomize what effect happens when you look out into the middle of Tay

	uint32 villageTime = _vm->_vars["rvillagetime"];

	// If we have time leftover from a previous run, set up the timer again
	if (_vm->getTotalPlayTime() < villageTime) {
		installTimer(TIMER(RSpit, rebelPrisonWindowTimer), villageTime - _vm->getTotalPlayTime());
		return;
	}

	uint32 timeUntilNextVideo;

	// Randomize the time until the next video
	if (_vm->_rnd->getRandomNumber(2) == 0 && _vm->_vars["rrichard"] == 0) {
		// In this case, a rebel is placed on a bridge
		// The video itself is handled by the scripts later on
		_vm->_vars["rrebelview"] = 0;
		timeUntilNextVideo = _vm->_rnd->getRandomNumberRng(38, 58) * 1000;
	} else {
		// Otherwise, just a random video from the timer
		_vm->_vars["rrebelview"] = 1;
		timeUntilNextVideo = _vm->_rnd->getRandomNumber(20) * 1000;
	}

	// We don't set rvillagetime here because the scripts later just reset it to 0
	// Of course, because of this, you can't return to the window twice and expect
	// the timer to reinstall itself...

	// Install our timer and we're on our way
	installTimer(TIMER(RSpit, rebelPrisonWindowTimer), timeUntilNextVideo);
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
