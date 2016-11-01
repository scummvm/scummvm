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

#include "common/events.h"

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

void OSpit::xorollcredittime(uint16 argc, uint16 *argv) {
	// WORKAROUND: The special change stuff only handles one destination and it would
	// be messy to modify the way that currently works. If we use the trap book on Tay,
	// we should be using the Tay end game sequences.
	if (_vm->_vars["returnstackid"] == kStackRspit) {
		_vm->changeToStack(kStackRspit);
		_vm->changeToCard(2);
		return;
	}

	// You used the trap book... why? What were you thinking?
	uint32 gehnState = _vm->_vars["agehn"];

	if (gehnState == 0)         // Gehn who?
		runEndGame(1, 9500);
	else if (gehnState == 4)    // You freed him? Are you kidding me?
		runEndGame(2, 12000);
	else                        // You already spoke with Gehn. What were you thinking?
		runEndGame(3, 8000);
}

void OSpit::xbookclick(uint16 argc, uint16 *argv) {
	// Hide the cursor
	_vm->_cursor->setCursor(kRivenHideCursor);
	_vm->_system->updateScreen();

	// Let's hook onto our video
	VideoEntryPtr video = _vm->_video->findVideoRiven(argv[0]);

	// Convert from the standard QuickTime base time to milliseconds
	// The values are in terms of 1/600 of a second.
	// Have I said how much I just *love* QuickTime? </sarcasm>
	uint32 startTime = argv[1] * 1000 / 600;
	uint32 endTime = argv[2] * 1000 / 600;

	// Track down our hotspot
	Common::String hotspotName = Common::String::format("touchBook%d", argv[3]);
	RivenHotspot *hotspot = _vm->getCard()->getHotspotByName(hotspotName);
	Common::Rect hotspotRect = hotspot->getRect();

	debug(0, "xbookclick:");
	debug(0, "\tVideo Code = %d", argv[0]);
	debug(0, "\tStart Time = %dms", startTime);
	debug(0, "\tEnd Time   = %dms", endTime);
	debug(0, "\tHotspot    = %d -> %s", argv[3], hotspotName.c_str());

	// Just let the video play while we wait until Gehn opens the trap book for us
	while (video->getTime() < startTime && !_vm->shouldQuit()) {
		if (_vm->_video->updateMovies())
			_vm->_system->updateScreen();

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event))
			;

		_vm->_system->delayMillis(10);
	}

	// Break out if we're quitting
	if (_vm->shouldQuit())
		return;

	// Update our hotspot stuff
	if (hotspotRect.contains(_vm->_system->getEventManager()->getMousePos()))
		_vm->_cursor->setCursor(kRivenOpenHandCursor);
	else
		_vm->_cursor->setCursor(kRivenMainCursor);

	_vm->_system->updateScreen();

	// OK, Gehn has opened the trap book and has asked us to go in. Let's watch
	// and see what the player will do...
	while (video->getTime() < endTime && !_vm->shouldQuit()) {
		bool updateScreen = _vm->_video->updateMovies();

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					if (hotspotRect.contains(_vm->_system->getEventManager()->getMousePos()))
						_vm->_cursor->setCursor(kRivenOpenHandCursor);
					else
						_vm->_cursor->setCursor(kRivenMainCursor);
					updateScreen = true;
					break;
				case Common::EVENT_LBUTTONUP:
					if (hotspotRect.contains(_vm->_system->getEventManager()->getMousePos())) {
						// OK, we've used the trap book! We go for ride lady!
						_vm->_scriptMan->stopAllScripts();                  // Stop all running scripts (so we don't remain in the cage)
						_vm->_video->stopVideos();                          // Stop all videos
						_vm->_cursor->setCursor(kRivenHideCursor);          // Hide the cursor
						_vm->getCard()->drawPicture(3);                  // Black out the screen
						_vm->_sound->playSound(0);                          // Play the link sound
						_vm->_video->activateMLST(_vm->getCard()->getMovie(7));    // Activate Gehn Link Video
						_vm->_video->playMovieBlockingRiven(1);             // Play Gehn Link Video
						_vm->_vars["agehn"] = 4;                            // Set Gehn to the trapped state
						_vm->_vars["atrapbook"] = 1;                        // We've got the trap book again
						_vm->_sound->playSound(0);                          // Play the link sound again
						_vm->changeToCard(_vm->getStack()->getCardStackId(0x2885));    // Link out!
						return;
					}
					break;
				default:
					break;
			}
		}

		if (updateScreen && !_vm->shouldQuit())
			_vm->_system->updateScreen();

		_vm->_system->delayMillis(10);
	}

	// Break out if we're quitting
	if (_vm->shouldQuit())
		return;

	// Hide the cursor again
	_vm->_cursor->setCursor(kRivenHideCursor);
	_vm->_system->updateScreen();

	// If there was no click and this is the third time Gehn asks us to
	// use the trap book, he will shoot the player. Dead on arrival.
	// Run the credits from here.
	if (_vm->_vars["agehn"] == 3) {
		_vm->_scriptMan->stopAllScripts();
		runCredits(argv[0], 5000);
		return;
	}

	// There was no click, so just play the rest of the video.
	_vm->_video->waitUntilMovieEnds(video);
}

void OSpit::xooffice30_closebook(uint16 argc, uint16 *argv) {
	// Close the blank linking book if it's open
	uint32 &book = _vm->_vars["odeskbook"];
	if (book != 1)
		return;

	// Set the variable to be "closed"
	book = 0;

	// Play the movie
	_vm->_video->playMovieBlockingRiven(1);

	// Set the hotspots into their correct states
	RivenHotspot *closeBook = _vm->getCard()->getHotspotByName("closeBook");
	RivenHotspot *nullHotspot = _vm->getCard()->getHotspotByName("null");
	RivenHotspot *openBook = _vm->getCard()->getHotspotByName("openBook");

	closeBook->enable(false);
	nullHotspot->enable(false);
	openBook->enable(true);

	// We now need to draw PLST 1 and refresh, but PLST 1 is
	// drawn when refreshing anyway, so don't worry about that.
	_vm->refreshCard();
}

void OSpit::xobedroom5_closedrawer(uint16 argc, uint16 *argv) {
	// Close the drawer if open when clicking on the journal.
	_vm->_video->playMovieBlockingRiven(2);
	_vm->_vars["ostanddrawer"] = 0;
}

void OSpit::xogehnopenbook(uint16 argc, uint16 *argv) {
	_vm->getCard()->drawPicture(_vm->_vars["ogehnpage"]);
}

void OSpit::xogehnbookprevpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 &page = _vm->_vars["ogehnpage"];

	// Decrement the page if it's not the first page
	if (page == 1)
		return;
	page--;

	// Play the page turning sound
	_vm->_sound->playSound(12);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(1);
	_vm->getCard()->drawPicture(page);
}

void OSpit::xogehnbooknextpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 &page = _vm->_vars["ogehnpage"];

	// Increment the page if it's not the last page
	if (page == 13)
		return;
	page++;

	// Play the page turning sound
	_vm->_sound->playSound(13);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(0);
	_vm->getCard()->drawPicture(page);
}

void OSpit::xgwatch(uint16 argc, uint16 *argv) {
	// Hide the cursor
	_vm->_cursor->setCursor(kRivenHideCursor);
	_vm->_system->updateScreen();

	uint32 &prisonCombo = _vm->_vars["pcorrectorder"];
	uint32 soundTime = _vm->_system->getMillis() - 500; // Start the first sound instantly
	byte curSound = 0;

	while (!_vm->shouldQuit()) {
		// Play the next sound every half second
		if (_vm->_system->getMillis() - soundTime >= 500) {
			if (curSound == 5) // Break out after the last sound is done
				break;

			_vm->_sound->playSound(getComboDigit(prisonCombo, curSound) + 13);
			curSound++;
			soundTime = _vm->_system->getMillis();
		}

		// Poll events just to check for quitting
		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {}

		// Cut down on CPU usage
		_vm->_system->delayMillis(10);
	}

	// Now play the video for the watch
	_vm->_video->activateMLST(_vm->getCard()->getMovie(1));
	_vm->_video->playMovieBlockingRiven(1);

	// And, finally, refresh
	_vm->refreshCard();
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
