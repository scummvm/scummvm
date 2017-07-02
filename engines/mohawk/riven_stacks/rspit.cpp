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

namespace Mohawk {
namespace RivenStacks {

RSpit::RSpit(MohawkEngine_Riven *vm) :
		RivenStack(vm, kStackRspit) {

	REGISTER_COMMAND(RSpit, xrshowinventory);
	REGISTER_COMMAND(RSpit, xrhideinventory);
	REGISTER_COMMAND(RSpit, xrcredittime);
	REGISTER_COMMAND(RSpit, xrwindowsetup);
}

void RSpit::xrcredittime(uint16 argc, uint16 *argv) {
	// Nice going, you used the trap book on Tay.

	// The game chooses what ending based on agehn for us,
	// so we just have to play the video and credits.
	// For the record, when agehn == 4, Gehn will thank you for
	// showing him the rebel age and then leave you to die.
	// Otherwise, the rebels burn the book. Epic fail either way.
	runEndGame(1, 1500);
}

void RSpit::xrshowinventory(uint16 argc, uint16 *argv) {
	// Give the trap book and Catherine's journal to the player
	_vm->_vars["atrapbook"] = 1;
	_vm->_vars["acathbook"] = 1;
	_vm->_gfx->showInventory();
}

void RSpit::xrhideinventory(uint16 argc, uint16 *argv) {
	_vm->_gfx->hideInventory();
}

void RSpit::rebelPrisonWindowTimer() {
	// Randomize a video out in the middle of Tay
	uint16 movie = _vm->_rnd->getRandomNumberRng(2, 13);
	_vm->_video->activateMLST(_vm->getCard()->getMovie(movie));
	VideoEntryPtr handle = _vm->_video->playMovieRiven(movie);

	// Ensure the next video starts after this one ends
	uint32 timeUntilNextVideo = handle->getDuration().msecs() + _vm->_rnd->getRandomNumberRng(38, 58) * 1000;

	// Save the time in case we leave the card and return
	_vm->_vars["rvillagetime"] = timeUntilNextVideo + _vm->getTotalPlayTime();

	// Reinstall this timer with the new time
	_vm->installTimer(TIMER(RSpit, rebelPrisonWindowTimer), timeUntilNextVideo);
}

void RSpit::xrwindowsetup(uint16 argc, uint16 *argv) {
	// Randomize what effect happens when you look out into the middle of Tay

	uint32 villageTime = _vm->_vars["rvillagetime"];

	// If we have time leftover from a previous run, set up the timer again
	if (_vm->getTotalPlayTime() < villageTime) {
		_vm->installTimer(TIMER(RSpit, rebelPrisonWindowTimer), villageTime - _vm->getTotalPlayTime());
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
	_vm->installTimer(TIMER(RSpit, rebelPrisonWindowTimer), timeUntilNextVideo);
}


} // End of namespace RivenStacks
} // End of namespace Mohawk
