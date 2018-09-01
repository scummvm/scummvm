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

#include "mohawk/riven_stacks/ospit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_inventory.h"
#include "mohawk/riven_video.h"

namespace Mohawk {
namespace RivenStacks {

OSpit::OSpit(MohawkEngine_Riven *vm) :
		RivenStack(vm, kStackOspit) {

	REGISTER_COMMAND(OSpit, xorollcredittime);
	REGISTER_COMMAND(OSpit, xbookclick);
	REGISTER_COMMAND(OSpit, xooffice30_closebook);
	REGISTER_COMMAND(OSpit, xobedroom5_closedrawer);
	REGISTER_COMMAND(OSpit, xogehnopenbook);
	REGISTER_COMMAND(OSpit, xogehnbookprevpage);
	REGISTER_COMMAND(OSpit, xogehnbooknextpage);
	REGISTER_COMMAND(OSpit, xgwatch);
}

void OSpit::xorollcredittime(const ArgumentArray &args) {
	// WORKAROUND: The special change stuff only handles one destination and it would
	// be messy to modify the way that currently works. If we use the trap book on Tay,
	// we should be using the Tay end game sequences.
	if (_vm->_vars["returnstackid"] == kStackRspit) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptWithCommand(
				new RivenStackChangeCommand(_vm, kStackRspit, 0x3338, true, false));
		_vm->_scriptMan->runScript(script, false);
		return;
	}

	// You used the trap book... why? What were you thinking?
	uint32 gehnState = _vm->_vars["agehn"];

	if (gehnState == 0)         // Gehn who?
		runEndGame(1, 9500, 1225);
	else if (gehnState == 4)    // You freed him? Are you kidding me?
		runEndGame(2, 12000, 558);
	else                        // You already spoke with Gehn. What were you thinking?
		runEndGame(3, 8000, 857);
}

void OSpit::xbookclick(const ArgumentArray &args) {
	// Let's hook onto our video
	RivenVideo *video = _vm->_video->getSlot(args[0]);

	// Convert from the standard QuickTime base time to milliseconds
	// The values are in terms of 1/600 of a second.
	// Have I said how much I just *love* QuickTime? </sarcasm>
	uint32 startTime = args[1] * 1000 / 600;
	uint32 endTime = args[2] * 1000 / 600;

	// Track down our hotspot
	Common::String hotspotName = Common::String::format("touchBook%d", args[3]);
	RivenHotspot *hotspot = _vm->getCard()->getHotspotByName(hotspotName);
	Common::Rect hotspotRect = hotspot->getRect();

	debug(0, "xbookclick:");
	debug(0, "\tVideo Code = %d", args[0]);
	debug(0, "\tStart Time = %dms", startTime);
	debug(0, "\tEnd Time   = %dms", endTime);
	debug(0, "\tHotspot    = %d -> %s", args[3], hotspotName.c_str());

	// Just let the video play while we wait until Gehn opens the trap book for us
	while (video->getTime() < startTime && !_vm->hasGameEnded()) {
		_vm->doFrame();
	}

	// Break out if we're quitting
	if (_vm->hasGameEnded())
		return;

	// OK, Gehn has opened the trap book and has asked us to go in. Let's watch
	// and see what the player will do...
	while (video->getTime() < endTime && !_vm->hasGameEnded()) {
		if (hotspotRect.contains(getMousePosition()))
			_vm->_cursor->setCursor(kRivenOpenHandCursor);
		else
			_vm->_cursor->setCursor(kRivenMainCursor);

		if (mouseIsDown()) {
			if (hotspotRect.contains(getMousePosition())) {
				// OK, we've used the trap book! We go for ride lady!
				_vm->_video->closeVideos();                          // Stop all videos
				_vm->_cursor->setCursor(kRivenHideCursor);          // Hide the cursor
				_vm->_gfx->scheduleTransition(kRivenTransitionBlend);
				_vm->getCard()->drawPicture(3);                  // Black out the screen
				_vm->_sound->playSound(0);                          // Play the link sound
				_vm->delay(12000);
				_vm->getCard()->playMovie(7);    // Activate Gehn Link Video
				RivenVideo *linkVideo = _vm->_video->openSlot(1);             // Play Gehn Link Video
				linkVideo->playBlocking();
				_vm->_vars["ocage"] = 1;
				_vm->_vars["agehn"] = 4;                            // Set Gehn to the trapped state
				_vm->_vars["atrapbook"] = 1;                        // We've got the trap book again
				_vm->_sound->playSound(0);                          // Play the link sound again
				_vm->_gfx->scheduleTransition(kRivenTransitionBlend);
				_vm->changeToCard(_vm->getStack()->getCardStackId(0x2885));    // Link out!
				_vm->_inventory->forceVisible(true);
				_vm->delay(2000);
				_vm->_inventory->forceVisible(false);
				_vm->_scriptMan->stopAllScripts();                  // Stop all running scripts (so we don't remain in the cage)
				return;
			}
		}

		_vm->doFrame();
	}

	// Break out if we're quitting
	if (_vm->hasGameEnded())
		return;

	// If there was no click and this is the third time Gehn asks us to
	// use the trap book, he will shoot the player. Dead on arrival.
	// Run the credits from here.
	if (_vm->_vars["agehn"] == 3) {
		runCredits(args[0], 5000, 995);
		return;
	}

	// There was no click, so just play the rest of the video.
	video->playBlocking();
}

void OSpit::xooffice30_closebook(const ArgumentArray &args) {
	// Close the blank linking book if it's open
	uint32 &book = _vm->_vars["odeskbook"];
	if (book != 1)
		return;

	// Set the variable to be "closed"
	book = 0;

	// Play the movie
	RivenVideo *video = _vm->_video->openSlot(1);
	video->seek(0);
	video->playBlocking();

	// Set the hotspots into their correct states
	RivenHotspot *closeBook = _vm->getCard()->getHotspotByName("closeBook");
	RivenHotspot *nullHotspot = _vm->getCard()->getHotspotByName("null");
	RivenHotspot *openBook = _vm->getCard()->getHotspotByName("openBook");

	closeBook->enable(false);
	nullHotspot->enable(false);
	openBook->enable(true);

	_vm->getCard()->drawPicture(1);
}

void OSpit::xobedroom5_closedrawer(const ArgumentArray &args) {
	// Close the drawer if open when clicking on the journal.
	RivenVideo *video = _vm->_video->openSlot(2);
	video->playBlocking();
	_vm->_vars["ostanddrawer"] = 0;
}

void OSpit::xogehnopenbook(const ArgumentArray &args) {
	_vm->getCard()->drawPicture(_vm->_vars["ogehnpage"]);
}

void OSpit::xogehnbookprevpage(const ArgumentArray &args) {
	// Get the page variable
	uint32 &page = _vm->_vars["ogehnpage"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the first page
		if (page == 1)
			return;

		// Update the page number
		page--;

		pageTurn(kRivenTransitionWipeRight);
		_vm->getCard()->drawPicture(page);
		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void OSpit::xogehnbooknextpage(const ArgumentArray &args) {
	// Get the page variable
	uint32 &page = _vm->_vars["ogehnpage"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the last page
		if (page == 13)
			return;

		// Update the page number
		page++;

		pageTurn(kRivenTransitionWipeLeft);
		_vm->getCard()->drawPicture(page);
		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void OSpit::xgwatch(const ArgumentArray &args) {
	// Hide the cursor
	_vm->_cursor->setCursor(kRivenHideCursor);

	uint32 prisonCombo = _vm->_vars["pcorrectorder"];

	byte curSound = 0;
	while (curSound < 5 && !_vm->hasGameEnded()) {
		// Play a sound every half second
		_vm->_sound->playSound(getComboDigit(prisonCombo, curSound) + 13);
		_vm->delay(500);

		curSound++;
	}

	// Now play the video for the watch
	_vm->getCard()->playMovie(1);
	RivenVideo *watchVideo = _vm->_video->openSlot(1);
	watchVideo->playBlocking();
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
