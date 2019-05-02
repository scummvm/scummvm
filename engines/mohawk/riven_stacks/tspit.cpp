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

#include "mohawk/riven_stacks/tspit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_inventory.h"
#include "mohawk/riven_video.h"

#include "common/events.h"

namespace Mohawk {
namespace RivenStacks {

TSpit::TSpit(MohawkEngine_Riven *vm) :
		DomeSpit(vm, kStackTspit, "tsliders.190", "tsliderbg.190") {

	REGISTER_COMMAND(TSpit, xtexterior300_telescopedown);
	REGISTER_COMMAND(TSpit, xtexterior300_telescopeup);
	REGISTER_COMMAND(TSpit, xtisland390_covercombo);
	REGISTER_COMMAND(TSpit, xtatrusgivesbooks);
	REGISTER_COMMAND(TSpit, xtchotakesbook);
	REGISTER_COMMAND(TSpit, xthideinventory);
	REGISTER_COMMAND(TSpit, xt7500_checkmarbles);
	REGISTER_COMMAND(TSpit, xt7600_setupmarbles);
	REGISTER_COMMAND(TSpit, xt7800_setup);
	REGISTER_COMMAND(TSpit, xdrawmarbles);
	REGISTER_COMMAND(TSpit, xtakeit);
	REGISTER_COMMAND(TSpit, xtscpbtn);
	REGISTER_COMMAND(TSpit, xtisland4990_domecheck);
	REGISTER_COMMAND(TSpit, xtisland5056_opencard);
	REGISTER_COMMAND(TSpit, xtisland5056_resetsliders);
	REGISTER_COMMAND(TSpit, xtisland5056_slidermd);
	REGISTER_COMMAND(TSpit, xtisland5056_slidermw);
	REGISTER_COMMAND(TSpit, xtatboundary);
}

void TSpit::xtexterior300_telescopedown(const ArgumentArray &args) {
	// First, show the button movie
	RivenVideo *buttonVideo = _vm->_video->openSlot(3);
	buttonVideo->seek(0);
	buttonVideo->enable();
	buttonVideo->playBlocking();

	// Don't do anything else if the telescope power is off
	if (_vm->_vars["ttelevalve"] == 0)
		return;

	uint32 &telescopePos = _vm->_vars["ttelescope"];
	uint32 &telescopeCover = _vm->_vars["ttelecover"];

	if (telescopePos != 1) {
		// We're not at the bottom, and we can move down again

		// Play a piece of the moving down movie
		static const uint32 timeIntervals[] = { 4320, 3440, 2660, 1760, 880, 0 };
		uint16 movieCode = telescopeCover ? 1 : 2;
		RivenVideo *video = _vm->_video->openSlot(movieCode);
		video->enable();
		video->seek(timeIntervals[telescopePos]);
		_vm->_sound->playCardSound("tTeleMove"); // Play the moving sound
		video->playBlocking(timeIntervals[telescopePos - 1]);
		video->stop();

		// Now move the telescope down a position and refresh
		telescopePos--;
		_vm->getCard()->enter(false);
		return;
	}

	// We're at the bottom, which means one of two things can happen...
	if (telescopeCover == 1 && _vm->_vars["ttelepin"] == 1) {
		// ...if the cover is open and the pin is up, the game is now over.
		xtopenfissure();
	} else {
		// ...the telescope can't move down anymore.
		// Play the sound of not being able to move
		_vm->_sound->playCardSound("tTelDnMore");
	}
}

void TSpit::xtexterior300_telescopeup(const ArgumentArray &args) {
	// First, show the button movie
	RivenVideo *buttonVideo = _vm->_video->openSlot(3);
	buttonVideo->seek(0);
	buttonVideo->enable();
	buttonVideo->playBlocking();

	// Don't do anything else if the telescope power is off
	if (_vm->_vars["ttelevalve"] == 0)
		return;

	uint32 &telescopePos = _vm->_vars["ttelescope"];

	// Check if we can't move up anymore
	if (telescopePos == 5) {
		// Play the sound of not being able to move
		_vm->_sound->playCardSound("tTelDnMore");
		return;
	}

	// Play a piece of the moving up movie
	static const uint32 timeIntervals[] = { 0, 800, 1680, 2560, 3440, 4320 };
	uint16 movieCode = _vm->_vars["ttelecover"] ? 4 : 5;
	RivenVideo *video = _vm->_video->openSlot(movieCode);
	video->enable();
	video->seek(timeIntervals[telescopePos - 1]);
	_vm->_sound->playCardSound("tTeleMove"); // Play the moving sound
	video->playBlocking(timeIntervals[telescopePos]);
	video->stop();

	// Now move the telescope up a position and refresh
	telescopePos++;
	_vm->getCard()->enter(false);
}

void TSpit::xtopenfissure() {
	if (_vm->_vars["pcage"] == 2) {
		// The best ending: Catherine is free, Gehn is trapped, Atrus comes to rescue you.
		// And now we fall back to Earth... all the way...
		_vm->getCard()->playMovie(8);
		runEndGame(8, 5000, 2640);
	} else if (_vm->_vars["agehn"] == 4) {
		// The ok ending: Catherine is still trapped, Gehn is trapped, Atrus comes to rescue you.
		// Nice going! Catherine and the islanders are all dead now! Just go back to your home...
		_vm->getCard()->playMovie(9);
		runEndGame(9, 5000, 2088);
	} else if (_vm->_vars["atrapbook"] == 1) {
		// The bad ending: Catherine is trapped, Gehn is free, Atrus gets shot by Gehn,
		// And then you get shot by Cho. Nice going! Catherine and the islanders are dead
		// and you have just set Gehn free from Riven, not to mention you're dead.
		_vm->getCard()->playMovie(10);
		runEndGame(10, 5000, 1703);
	} else {
		// The impossible ending: You don't have Catherine's journal and yet you were somehow
		// able to open the hatch on the telescope. The game provides an ending for those who
		// cheat, load a saved game with the combo, or just guess the telescope combo. Atrus
		// doesn't come and you just fall into the fissure.
		_vm->getCard()->playMovie(11);
		runEndGame(11, 5000, 0);
	}
}

void TSpit::xtisland390_covercombo(const ArgumentArray &args) {
	// Called when clicking the telescope cover buttons. args[0] is the button number (1...5).
	uint32 &correctDigits = _vm->_vars["tcovercombo"];

	if (correctDigits < 5 && args[0] == getComboDigit(_vm->_vars["tcorrectorder"], correctDigits))
		correctDigits++;
	else
		correctDigits = 0;

	// If we have hit the correct 5 buttons in a row, activate the hotspot to open up the
	// telescope cover.
	RivenHotspot *openCover = _vm->getCard()->getHotspotByName("openCover");
	openCover->enable(correctDigits == 5);
}

// Atrus' Journal and Trap Book are added to inventory
void TSpit::xtatrusgivesbooks(const ArgumentArray &args) {
	// Give the player Atrus' Journal and the Trap book
}

// Trap Book is removed from inventory
void TSpit::xtchotakesbook(const ArgumentArray &args) {
	// And now Cho takes the trap book
}

void TSpit::xthideinventory(const ArgumentArray &args) {
}

// Marble Puzzle related constants
static const uint32 kMarbleCount = 6;
static const int kSmallMarbleWidth = 4;
static const int kSmallMarbleHeight = 2;
//static const int kLargeMarbleSize = 8;
static const int kMarbleHotspotSize = 13;
static const char *s_marbleNames[] = { "tred", "torange", "tyellow", "tgreen", "tblue", "tviolet" };

// Marble Puzzle helper functions
// The y portion takes the upper 16 bits, while the x portion takes the lower 16 bits
static void setMarbleX(uint32 &var, byte x) {
	var = (var & 0xff00) | (x + 1);
}

static void setMarbleY(uint32 &var, byte y) {
	var = ((y + 1) << 16) | (var & 0xff);
}

static byte getMarbleX(uint32 var) {
	return (var & 0xff) - 1;
}

static byte getMarbleY(uint32 var) { // Give that that Y you old hag! </bad Seinfeld reference>
	return ((var >> 16) & 0xff) - 1;
}

static Common::Rect generateMarbleGridRect(uint16 x, uint16 y) {
	// x/y in terms of 0!
	static const int marbleGridOffsetX[] = { 134, 202, 270, 338, 406 };
	static const int marbleGridOffsetY[] = {  24,  92, 159, 227, 295 };

	uint16 offsetX = marbleGridOffsetX[x / 5] + (x % 5) * kMarbleHotspotSize;
	uint16 offsetY = marbleGridOffsetY[y / 5] + (y % 5) * kMarbleHotspotSize;
	return Common::Rect(offsetX, offsetY, offsetX + kMarbleHotspotSize, offsetY + kMarbleHotspotSize);
}

void TSpit::xt7500_checkmarbles(const ArgumentArray &args) {
	// Set apower if the marbles are in their correct spot.

	bool valid = true;
	static const uint32 marbleFinalValues[] = { 1114121, 1441798, 0, 65552, 65558, 262146 };

	for (uint16 i = 0; i < kMarbleCount; i++)
		if (_vm->_vars[s_marbleNames[i]] != marbleFinalValues[i]) {
			valid = false;
			break;
		}

	// If we have the correct combo, activate the power and reset the marble positions
	// Otherwise, make sure the power is off
	if (valid) {
		_vm->_vars["apower"] = 1;
		for (uint16 i = 0; i < kMarbleCount; i++)
			_vm->_vars[s_marbleNames[i]] = 0;
	} else
		_vm->_vars["apower"] = 0;
}

void TSpit::xt7600_setupmarbles(const ArgumentArray &args) {
	// Draw the small marbles when we're a step away from the waffle

	// Convert from marble X coordinate to screen X coordinate
	static const uint16 xPosOffsets[] = {
			246, 245, 244, 243, 243, 241, 240, 240, 239, 238, 237, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 226, 225
	};

	// Convert from marble Y coordinate to screen Y coordinate
	static const uint16 yPosOffsets[] = {
			261, 263, 265, 267, 268, 270, 272, 274, 276, 278, 281, 284, 285, 288, 290, 293, 295, 298, 300, 303, 306, 309, 311, 314, 316
	};

	// Handle spacing for y coordinates due to the angle
	static const double yAdjusts[] = {
			4.56, 4.68, 4.76, 4.84, 4.84, 4.96, 5.04, 5.04, 5.12, 5.2, 5.28, 5.28, 5.36, 5.44, 5.4, 5.6, 5.72, 5.8, 5.88, 5.96, 6.04, 6.12, 6.2, 6.2, 6.28
	};

	// Waffle state of 0 is up, 1 down
	bool waffleDown = _vm->_vars["twaffle"] != 0;

	// Note that each of the small marble images is exactly 4x2
	// The original seems to scale the marble images from extras.mhk, but
	// we're using the pre-scaled images in the stack.
	uint16 baseBitmapId = _vm->findResourceID(ID_TBMP, buildCardResourceName("tsmallred"));

	for (uint16 i = 0; i < kMarbleCount; i++) {
		uint32 var = _vm->_vars[s_marbleNames[i]];

		if (var == 0) {
			// The marble is still in its initial place
			// (Note that this is still drawn even if the waffle is down)
			static const uint16 defaultX[] = { 375, 377, 379, 381, 383, 385 };
			static const uint16 defaultY[] = { 253, 257, 261, 265, 268, 273 };
			_vm->_gfx->copyImageToScreen(baseBitmapId + i, defaultX[i], defaultY[i], defaultX[i] + kSmallMarbleWidth, defaultY[i] + kSmallMarbleHeight);
		} else if (waffleDown) {
			// The marble is on the grid and the waffle is down
			// (Nothing to draw here)
		} else {
			// The marble is on the grid and the waffle is up
			int marbleX = (int)floor(getMarbleX(var) * yAdjusts[getMarbleY(var)] + xPosOffsets[getMarbleY(var)] + 0.5);
			int marbleY = yPosOffsets[getMarbleY(var)];
			_vm->_gfx->copyImageToScreen(baseBitmapId + i, marbleX, marbleY, marbleX + kSmallMarbleWidth, marbleY + kSmallMarbleHeight);
		}
	}
}

void TSpit::setMarbleHotspots() {
	// Set the hotspots
	for (uint16 i = 0; i < kMarbleCount; i++) {
		uint32 marblePos = _vm->_vars[s_marbleNames[i]];
		RivenHotspot *marbleHotspot = _vm->getCard()->getHotspotByName(s_marbleNames[i]);

		if (marblePos == 0) // In the receptacle
			marbleHotspot->setRect(_marbleBaseHotspots[i]);
		else                 // On the grid
			marbleHotspot->setRect(generateMarbleGridRect(getMarbleX(marblePos), getMarbleY(marblePos)));
	}
}

void TSpit::xt7800_setup(const ArgumentArray &args) {
	// First, let's store the base receptacle hotspots for the marbles
	if (_marbleBaseHotspots.empty())
		for (uint16 i = 0; i < kMarbleCount; i++) {
			RivenHotspot *marbleHotspot = _vm->getCard()->getHotspotByName(s_marbleNames[i]);
			_marbleBaseHotspots.push_back(marbleHotspot->getRect());
		}

	// Move the marble hotspots based on their position variables
	setMarbleHotspots();
	_vm->_vars["themarble"] = 0;
}

void TSpit::drawMarbles() {
	_vm->_gfx->beginScreenUpdate();
	for (uint32 i = 0; i < kMarbleCount; i++) {
		// Don't draw the marble if we're holding it
		if (_vm->_vars["themarble"] - 1 == i)
			continue;

		RivenHotspot *marbleHotspot = _vm->getCard()->getHotspotByName(s_marbleNames[i]);

		Common::Rect rect = marbleHotspot->getRect();
		// Trim the rect down a bit
		rect.left += 3;
		rect.top += 3;
		rect.right -= 2;
		rect.bottom -= 2;
		_vm->_gfx->drawExtrasImage(i + 200, rect);
	}
	_vm->_gfx->applyScreenUpdate();
}

void TSpit::xdrawmarbles(const ArgumentArray &args) {
	// Draw marbles in the closeup
	drawMarbles();
}

void TSpit::xtakeit(const ArgumentArray &args) {
	// Pick up and move a marble

	// First, let's figure out what marble we're now holding
	uint32 &marble = _vm->_vars["themarble"];
	marble = 0;

	for (uint32 i = 0; i < kMarbleCount; i++) {
		RivenHotspot *marbleHotspot = _vm->getCard()->getHotspotByName(s_marbleNames[i]);
		if (marbleHotspot->containsPoint(getMousePosition())) {
			marble = i + 1;
			break;
		}
	}

	if (marble == 0) {
		// xtakeit() shouldn't be called if we're not on a marble hotspot,
		// but maybe another mouse moved event was received between the moment
		// this script was queued and the moment it was executed.
		return;
	}

	// Redraw the background
	_vm->getCard()->drawPicture(1);

	// Loop until the player lets go (or quits)
	while (mouseIsDown() && !_vm->hasGameEnded()) {
		_vm->doFrame();
	}

	// Check if we landed in a valid location and no other marble has that location
	uint32 &marblePos = _vm->_vars[s_marbleNames[marble - 1]];

	bool foundMatch = false;
	for (int y = 0; y < 25 && !foundMatch; y++) {
		for (int x = 0; x < 25 && !foundMatch; x++) {
			Common::Rect testHotspot = generateMarbleGridRect(x, y);

			// Let's try to place the marble!
			if (testHotspot.contains(getMousePosition())) {
				// Set this as the position
				setMarbleX(marblePos, x);
				setMarbleY(marblePos, y);

				// Let's make sure no other marble is in this spot...
				for (uint16 i = 0; i < kMarbleCount; i++)
					if (i != marble - 1 && _vm->_vars[s_marbleNames[i]] == marblePos)
						marblePos = 0;

				// We have a match
				foundMatch = true;
			}
		}
	}

	// If we still don't have a match, reset it to the original location
	if (!foundMatch)
		marblePos = 0;

	// Check the new hotspots and refresh everything
	marble = 0;
	setMarbleHotspots();
	drawMarbles();
}

void TSpit::xtscpbtn(const ArgumentArray &args) {
	runDomeButtonMovie();
}

void TSpit::xtisland4990_domecheck(const ArgumentArray &args) {
	runDomeCheck();
}

void TSpit::xtisland5056_opencard(const ArgumentArray &args) {
	checkDomeSliders();
}

void TSpit::xtisland5056_resetsliders(const ArgumentArray &args) {
	resetDomeSliders(24);
}

void TSpit::xtisland5056_slidermd(const ArgumentArray &args) {
	dragDomeSlider(24);
}

void TSpit::xtisland5056_slidermw(const ArgumentArray &args) {
	checkSliderCursorChange(24);
}

void TSpit::xtatboundary(const ArgumentArray &args) {
	runDemoBoundaryDialog();
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
