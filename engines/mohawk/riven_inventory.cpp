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

#include "mohawk/riven_inventory.h"

#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_stack.h"

namespace Mohawk {

RivenInventory::RivenInventory(MohawkEngine_Riven *vm) :
		_vm(vm),
		_inventoryDrawn(false),
		_forceVisible(false),
		_forceHidden(false) {

	_atrusJournalRect1 = Common::Rect(295, 402, 313, 426);
	_atrusJournalRect2 = Common::Rect(259, 402, 278, 426);
	_cathJournalRect2 = Common::Rect(328, 408, 348, 419);
	_atrusJournalRect3 = Common::Rect(222, 402, 240, 426);
	_cathJournalRect3 = Common::Rect(291, 408, 311, 419);
	_trapBookRect3 = Common::Rect(363, 396, 386, 432);
	_demoExitRect = Common::Rect(291, 408, 317, 419);
}

RivenInventory::~RivenInventory() {

}

void RivenInventory::draw() {
	// Clear the inventory area
	clearArea();

	// Draw the demo's exit button
	if (_vm->isGameVariant(GF_DEMO)) {
		// extras.mhk tBMP 101 contains "EXIT" instead of Atrus' journal in the demo!
		// The demo's extras.mhk contains all the other inventory/marble/credits image
		// but has hacked tBMP 101 with "EXIT". *sigh*
		_vm->_gfx->drawExtrasImageToScreen(101, _demoExitRect);
	} else {
		// There are three books and three vars. We have three different
		// combinations. At the start you have just Atrus' journal. Later,
		// you get Catherine's journal and the trap book. Near the end,
		// you lose the trap book and have just the two journals.

		bool hasCathBook = _vm->_vars["rrebel"] == 5 || _vm->_vars["rrebel"] == 6;
		bool hasTrapBook = _vm->_vars["atrapbook"] == 1;

		if (!hasCathBook) {
			_vm->_gfx->drawExtrasImageToScreen(101, _atrusJournalRect1);
		} else if (!hasTrapBook) {
			_vm->_gfx->drawExtrasImageToScreen(101, _atrusJournalRect2);
			_vm->_gfx->drawExtrasImageToScreen(102, _cathJournalRect2);
		} else {
			_vm->_gfx->drawExtrasImageToScreen(101, _atrusJournalRect3);
			_vm->_gfx->drawExtrasImageToScreen(102, _cathJournalRect3);
			_vm->_gfx->drawExtrasImageToScreen(100, _trapBookRect3);
		}
	}
}

void RivenInventory::clearArea() {
	// Clear the inventory area
	static const Common::Rect inventoryRect = Common::Rect(0, 392, 608, 436);

	// Lock the screen
	Graphics::Surface *screen = _vm->_system->lockScreen();

	// Fill the inventory area with black
	screen->fillRect(inventoryRect, screen->format.RGBToColor(0, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenInventory::checkClick(const Common::Point &mousePos) {
	if (!isVisible()) {
		return; // Don't even bother.
	}

	// In the demo, check if we've clicked the exit button
	if (_vm->isGameVariant(GF_DEMO)) {
		if (_demoExitRect.contains(mousePos)) {
			if (_vm->getStack()->getId() == kStackAspit && _vm->getCard()->getId() == 1) {
				// From the main menu, go to the "quit" screen
				_vm->changeToCard(12);
			} else if (_vm->getStack()->getId() == kStackAspit && _vm->getCard()->getId() == 12) {
				// From the "quit" screen, just quit
				_vm->setGameEnded();
			} else {
				// Otherwise, return to the main menu
				if (_vm->getStack()->getId() != kStackAspit)
					_vm->changeToStack(kStackAspit);
				_vm->changeToCard(1);
			}
		}
		return;
	}

	// No inventory shown on aspit
	if (_vm->getStack()->getId() == kStackAspit)
		return;

	// Set the return stack/card id's.
	_vm->_vars["returnstackid"] = _vm->getStack()->getId();
	_vm->_vars["returncardid"] = _vm->getStack()->getCardGlobalId(_vm->getCard()->getId());

	// See RivenGraphics::show() for an explanation
	// of the variables' meanings.
	bool hasCathBook = _vm->_vars["rrebel"] == 5 || _vm->_vars["rrebel"] == 6;
	bool hasTrapBook = _vm->_vars["atrapbook"] == 1;

	// Go to the book if a hotspot contains the mouse
	if (!hasCathBook) {
		if (_atrusJournalRect1.contains(mousePos)) {
			_vm->changeToStack(kStackAspit);
			_vm->changeToCard(5);
		}
	} else if (!hasTrapBook) {
		if (_atrusJournalRect2.contains(mousePos)) {
			_vm->changeToStack(kStackAspit);
			_vm->changeToCard(5);
		} else if (_cathJournalRect2.contains(mousePos)) {
			_vm->changeToStack(kStackAspit);
			_vm->changeToCard(6);
		}
	} else {
		if (_atrusJournalRect3.contains(mousePos)) {
			_vm->changeToStack(kStackAspit);
			_vm->changeToCard(5);
		} else if (_cathJournalRect3.contains(mousePos)) {
			_vm->changeToStack(kStackAspit);
			_vm->changeToCard(6);
		} else if (_trapBookRect3.contains(mousePos)) {
			_vm->changeToStack(kStackAspit);
			_vm->changeToCard(7);
		}
	}
}

void RivenInventory::backFromItemScript() const {
	RivenScriptPtr stopSoundScript = _vm->_scriptMan->createScriptFromData(1, kRivenCommandStopSound, 1, 1);
	_vm->_scriptMan->runScript(stopSoundScript, false);

	uint16 backStackId = _vm->_vars["returnstackid"];
	uint32 backCardId = _vm->_vars["returncardid"];

	// Return to where we were before entering the book
	RivenCommand *back = new RivenStackChangeCommand(_vm, backStackId, backCardId, true, false);
	RivenScriptPtr backScript = _vm->_scriptMan->createScriptWithCommand(back);
	_vm->_scriptMan->runScript(backScript, true);
}

bool RivenInventory::isVisible() const {
	if (_forceVisible) {
		return true;
	}

	if (_forceHidden) {
		return false;
	}

	if (_vm->isGameVariant(GF_DEMO)) {
		// The inventory is always visible in the demo
		return true;
	}

	// We don't want to show the inventory on setup screens or in other journals.
	if (_vm->getStack()->getId() == kStackAspit)
		return false;

	// We don't want to show the inventory while scripts are running
	if (_vm->_scriptMan->runningQueuedScripts())
		return false;

	Common::Point mouse = _vm->getStack()->getMousePosition();
	return mouse.y >= 392;
}

void RivenInventory::onFrame() {
	bool visible = isVisible();

	if (visible && !_inventoryDrawn) {
		draw();
		_inventoryDrawn = true;
	} else if (!visible && _inventoryDrawn) {
		clearArea();
		_inventoryDrawn = false;
	}
}

void RivenInventory::forceVisible(bool visible) {
	_forceVisible = visible;
}

void RivenInventory::forceHidden(bool hidden) {
	_forceHidden = hidden;
}

} // End of namespace Mohawk
