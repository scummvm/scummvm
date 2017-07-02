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

#include "mohawk/riven_stacks/pspit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_sound.h"

namespace Mohawk {
namespace RivenStacks {

PSpit::PSpit(MohawkEngine_Riven *vm) :
		DomeSpit(vm, kStackPspit, "psliders.25", "psliderbg.25") {

	REGISTER_COMMAND(PSpit, xpisland990_elevcombo);
	REGISTER_COMMAND(PSpit, xpscpbtn);
	REGISTER_COMMAND(PSpit, xpisland290_domecheck);
	REGISTER_COMMAND(PSpit, xpisland25_opencard);
	REGISTER_COMMAND(PSpit, xpisland25_resetsliders);
	REGISTER_COMMAND(PSpit, xpisland25_slidermd);
	REGISTER_COMMAND(PSpit, xpisland25_slidermw);
}

void PSpit::catherineIdleTimer() {
	uint32 &cathCheck = _vm->_vars["pcathcheck"];
	uint32 &cathState = _vm->_vars["acathstate"];
	uint16 movie;

	// Choose a random movie based on where Catherine is
	if (cathCheck == 0) {
		static const int movieList[] = { 5, 6, 7, 8 };
		cathCheck = 1;
		movie = movieList[_vm->_rnd->getRandomNumber(3)];
	} else if (cathState == 1) {
		static const int movieList[] = { 11, 14 };
		movie = movieList[_vm->_rnd->getRandomBit()];
	} else {
		static const int movieList[] = { 9, 10, 12, 13 };
		movie = movieList[_vm->_rnd->getRandomNumber(3)];
	}

	// Update her state if she moves from left/right or right/left, resp.
	if (movie == 5 || movie == 7 || movie == 11 || movie == 14)
		cathState = 2;
	else
		cathState = 1;

	// Play the movie, blocking
	_vm->_video->activateMLST(_vm->getCard()->getMovie(movie));
	_vm->_cursor->hideCursor();
	_vm->_video->playMovieBlockingRiven(movie);
	_vm->_cursor->showCursor();
	_vm->_system->updateScreen();

	// Install the next timer for the next video
	uint32 timeUntilNextMovie = _vm->_rnd->getRandomNumber(120) * 1000;

	_vm->_vars["pcathtime"] = timeUntilNextMovie + _vm->getTotalPlayTime();

	_vm->installTimer(TIMER(PSpit, catherineIdleTimer), timeUntilNextMovie);
}

void PSpit::xpisland990_elevcombo(uint16 argc, uint16 *argv) {
	// Play button sound based on argv[0]
	_vm->_sound->playSound(argv[0] + 5);

	// It is impossible to get here if Gehn is not trapped. However,
	// the original also disallows brute forcing the ending if you have
	// not yet trapped Gehn.
	if (_vm->_vars["agehn"] != 4)
		return;

	uint32 &correctDigits = _vm->_vars["pelevcombo"];

	// pelevcombo keeps count of how many buttons we have pressed in the correct order.
	// When pelevcombo is 5, clicking the handle will show the video freeing Catherine.
	if (correctDigits < 5 && argv[0] == getComboDigit(_vm->_vars["pcorrectorder"], correctDigits))
		correctDigits++;
	else
		correctDigits = 0;
}

void PSpit::xpscpbtn(uint16 argc, uint16 *argv) {
	runDomeButtonMovie();
}

void PSpit::xpisland290_domecheck(uint16 argc, uint16 *argv) {
	runDomeCheck();
}

void PSpit::xpisland25_opencard(uint16 argc, uint16 *argv) {
	checkDomeSliders();
}

void PSpit::xpisland25_resetsliders(uint16 argc, uint16 *argv) {
	resetDomeSliders(10, 14);
}

void PSpit::xpisland25_slidermd(uint16 argc, uint16 *argv) {
	dragDomeSlider(10, 14);
}

void PSpit::xpisland25_slidermw(uint16 argc, uint16 *argv) {
	checkSliderCursorChange(14);
}

void PSpit::installCardTimer() {
	if (getCurrentCardGlobalId() == 0x3a85) {
		// Top of elevator on prison island
		// Handle Catherine hardcoded videos
		_vm->installTimer(TIMER(PSpit, catherineIdleTimer), _vm->_rnd->getRandomNumberRng(1, 33) * 1000);
	} else {
		RivenStack::installCardTimer();
	}
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
