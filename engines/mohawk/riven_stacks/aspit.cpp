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

#include "mohawk/riven_stacks/aspit.h"

#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_sound.h"

#include "common/translation.h"

#include "gui/message.h"

namespace Mohawk {
namespace RivenStacks {

ASpit::ASpit(MohawkEngine_Riven *vm) :
		RivenStack(vm, kStackAspit) {

	REGISTER_COMMAND(ASpit, xastartupbtnhide);
	REGISTER_COMMAND(ASpit, xasetupcomplete);
	REGISTER_COMMAND(ASpit, xaatrusopenbook);
	REGISTER_COMMAND(ASpit, xaatrusbookback);
	REGISTER_COMMAND(ASpit, xaatrusbookprevpage);
	REGISTER_COMMAND(ASpit, xaatrusbooknextpage);
	REGISTER_COMMAND(ASpit, xacathopenbook);
	REGISTER_COMMAND(ASpit, xacathbookback);
	REGISTER_COMMAND(ASpit, xacathbookprevpage);
	REGISTER_COMMAND(ASpit, xacathbooknextpage);
	REGISTER_COMMAND(ASpit, xtrapbookback);
	REGISTER_COMMAND(ASpit, xatrapbookclose);
	REGISTER_COMMAND(ASpit, xatrapbookopen);
	REGISTER_COMMAND(ASpit, xarestoregame);
	REGISTER_COMMAND(ASpit, xadisablemenureturn);
	REGISTER_COMMAND(ASpit, xaenablemenureturn);
	REGISTER_COMMAND(ASpit, xalaunchbrowser);
	REGISTER_COMMAND(ASpit, xadisablemenuintro);
	REGISTER_COMMAND(ASpit, xaenablemenuintro);
	REGISTER_COMMAND(ASpit, xademoquit);
	REGISTER_COMMAND(ASpit, xaexittomain);
}

void ASpit::xastartupbtnhide(uint16 argc, uint16 *argv) {
	// The original game hides the start/setup buttons depending on an ini entry.
	// It's safe to ignore this command.
}

void ASpit::xasetupcomplete(uint16 argc, uint16 *argv) {
	// The original game sets an ini entry to disable the setup button and use the
	// start button only. It's safe to ignore this part of the command.
	_vm->_sound->stopSound();
	_vm->changeToCard(1);
}

void ASpit::xaatrusopenbook(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 &page = _vm->_vars["aatruspage"];

	// Set hotspots depending on the page
	RivenHotspot *openBook = _vm->getCard()->getHotspotByName("openBook");
	RivenHotspot *nextPage = _vm->getCard()->getHotspotByName("nextpage");
	RivenHotspot *prevPage = _vm->getCard()->getHotspotByName("prevpage");
	if (page == 1) {
		prevPage->enable(false);
		nextPage->enable(false);
		openBook->enable(true);
	} else {
		prevPage->enable(true);
		nextPage->enable(true);
		openBook->enable(false);
	}

	// Draw the image of the page
	_vm->getCard()->drawPicture(page);
}

void ASpit::xaatrusbookback(uint16 argc, uint16 *argv) {
	// Return to where we were before entering the book
	_vm->changeToStack(_vm->_vars["returnstackid"]);
	_vm->changeToCard(_vm->_vars["returncardid"]);
}

void ASpit::xaatrusbookprevpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 &page = _vm->_vars["aatruspage"];

	// Decrement the page if it's not the first page
	if (page == 1)
		return;
	page--;

	// Play the page turning sound
	if (_vm->getFeatures() & GF_DEMO)
		_vm->_sound->playSound(4);
	else
		_vm->_sound->playSound(3);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(1);
	_vm->getCard()->drawPicture(page);
}

void ASpit::xaatrusbooknextpage(uint16 argc, uint16 *argv) {
	// Get the page variable
	uint32 &page = _vm->_vars["aatruspage"];

	// Increment the page if it's not the last page
	if (((_vm->getFeatures() & GF_DEMO) && page == 6) || page == 10)
		return;
	page++;

	// Play the page turning sound
	if (_vm->getFeatures() & GF_DEMO)
		_vm->_sound->playSound(5);
	else
		_vm->_sound->playSound(4);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(0);
	_vm->getCard()->drawPicture(page);
}

void ASpit::xacathopenbook(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 page = _vm->_vars["acathpage"];

	// Set hotspots depending on the page
	RivenHotspot *openBook = _vm->getCard()->getHotspotByName("openBook");
	RivenHotspot *nextPage = _vm->getCard()->getHotspotByName("nextpage");
	RivenHotspot *prevPage = _vm->getCard()->getHotspotByName("prevpage");
	if (page == 1) {
		prevPage->enable(false);
		nextPage->enable(false);
		openBook->enable(true);
	} else {
		prevPage->enable(true);
		nextPage->enable(true);
		openBook->enable(false);
	}

	// Draw the image of the page
	_vm->getCard()->drawPicture(page);

	// Draw the white page edges
	if (page > 1 && page < 5)
		_vm->getCard()->drawPicture(50);
	else if (page > 5)
		_vm->getCard()->drawPicture(51);

	if (page == 28) {
		// Draw the telescope combination
		// The images for the numbers are tBMP's 13 through 17.
		// The start point is at (156, 247)
		uint32 teleCombo = _vm->_vars["tcorrectorder"];
		static const uint16 kNumberWidth = 32;
		static const uint16 kNumberHeight = 25;
		static const uint16 kDstX = 156;
		static const uint16 kDstY = 247;

		for (byte i = 0; i < 5; i++) {
			uint16 offset = (getComboDigit(teleCombo, i) - 1) * kNumberWidth;
			Common::Rect srcRect = Common::Rect(offset, 0, offset + kNumberWidth, kNumberHeight);
			Common::Rect dstRect = Common::Rect(i * kNumberWidth + kDstX, kDstY, (i + 1) * kNumberWidth + kDstX, kDstY + kNumberHeight);
			_vm->_gfx->drawImageRect(i + 13, srcRect, dstRect);
		}
	}
}

void ASpit::xacathbookback(uint16 argc, uint16 *argv) {
	// Return to where we were before entering the book
	_vm->changeToStack(_vm->_vars["returnstackid"]);
	_vm->changeToCard(_vm->_vars["returncardid"]);
}

void ASpit::xacathbookprevpage(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 &page = _vm->_vars["acathpage"];

	// Increment the page if it's not the first page
	if (page == 1)
		return;
	page--;

	// Play the page turning sound
	_vm->_sound->playSound(5);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(3);
	_vm->getCard()->drawPicture(page);
}

void ASpit::xacathbooknextpage(uint16 argc, uint16 *argv) {
	// Get the variable
	uint32 &page = _vm->_vars["acathpage"];

	// Increment the page if it's not the last page
	if (page == 49)
		return;
	page++;

	// Play the page turning sound
	_vm->_sound->playSound(6);

	// Now update the screen :)
	_vm->_gfx->scheduleTransition(2);
	_vm->getCard()->drawPicture(page);
}

void ASpit::xtrapbookback(uint16 argc, uint16 *argv) {
	// Return to where we were before entering the book
	_vm->_vars["atrap"] = 0;
	_vm->changeToStack(_vm->_vars["returnstackid"]);
	_vm->changeToCard(_vm->_vars["returncardid"]);
}

void ASpit::xatrapbookclose(uint16 argc, uint16 *argv) {
	// Close the trap book
	_vm->_vars["atrap"] = 0;

	// Play the page turning sound
	_vm->_sound->playSound(8);

	_vm->refreshCard();
}

void ASpit::xatrapbookopen(uint16 argc, uint16 *argv) {
	// Open the trap book
	_vm->_vars["atrap"] = 1;

	// Play the page turning sound
	_vm->_sound->playSound(9);

	_vm->refreshCard();
}

void ASpit::xarestoregame(uint16 argc, uint16 *argv) {
	// Launch the load game dialog
	_vm->runLoadDialog();
}

void ASpit::xadisablemenureturn(uint16 argc, uint16 *argv) {
	// This function would normally enable the Windows menu item for
	// returning to the main menu. Ctrl+r will do this instead.
	// The original also had this shortcut.
}

void ASpit::xaenablemenureturn(uint16 argc, uint16 *argv) {
	// This function would normally enable the Windows menu item for
	// returning to the main menu. Ctrl+r will do this instead.
	// The original also had this shortcut.
}

void ASpit::xalaunchbrowser(uint16 argc, uint16 *argv) {
	// Well, we can't launch a browser for obvious reasons ;)
	// The original text is as follows (for reference):

	// If you have an auto-dial configured connection to the Internet,
	// please select YES below.
	//
	// America Online and CompuServe users may experience difficulty. If
	// you find that you are unable to connect, please quit the Riven
	// Demo, launch your browser and type in the following URL:
	//
	//     www.redorb.com/buyriven
	//
	// Would you like to attempt to make the connection?
	//
	// [YES] [NO]

	GUI::MessageDialog dialog(_("At this point, the Riven Demo would\n"
			                          "ask if you would like to open a web browser\n"
			                          "to bring you to the Red Orb store to buy\n"
			                          "the game. ScummVM cannot do that and\n"
			                          "the site no longer exists."));
	dialog.runModal();
}

void ASpit::xadisablemenuintro(uint16 argc, uint16 *argv) {
	// This function would normally enable the Windows menu item for
	// playing the intro. Ctrl+p will play the intro movies instead.
	// The original also had this shortcut.

	// Hide the "exit" button here
	_vm->_gfx->hideInventory();
}

void ASpit::xaenablemenuintro(uint16 argc, uint16 *argv) {
	// This function would normally enable the Windows menu item for
	// playing the intro. Ctrl+p will play the intro movies instead.
	// The original also had this shortcut.

	// Show the "exit" button here
	_vm->_gfx->showInventory();
}

void ASpit::xademoquit(uint16 argc, uint16 *argv) {
	// Exactly as it says on the tin. In the demo, this function quits.
	_vm->setGameOver();
}

void ASpit::xaexittomain(uint16 argc, uint16 *argv) {
	// One could potentially implement this function, but there would be no
	// point. This function is only used in the demo's aspit card 9 update
	// screen script. However, card 9 is not accessible from the game without
	// jumping to the card and there's nothing going on in the card so it
	// never gets called. There's also no card 9 in the full game, so the
	// functionality of this card was likely removed before release. The
	// demo executable references some other external commands relating to
	// setting and getting the volume, as well as drawing the volume. I'd
	// venture to guess that this would have been some sort of options card
	// replaced with the Windows/Mac API in the final product.
	//
	// Yeah, this function is just dummied and holds a big comment ;)
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
