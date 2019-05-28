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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/slides.h"

#include "common/system.h"

namespace Mohawk {
namespace MystStacks {

Slides::Slides(MohawkEngine_Myst *vm) :
		MystScriptParser(vm, kDemoSlidesStack) {
	setupOpcodes();

	_vm->_cursor->hideCursor();

	_cardSwapEnabled = false;
	_nextCardID = 0;
	_nextCardTime = 0;
}

Slides::~Slides() {
}

void Slides::setupOpcodes() {
	// "Stack-Specific" Opcodes
	REGISTER_OPCODE(100, Slides, o_returnToMenu);

	// "Init" Opcodes
	REGISTER_OPCODE(200, Slides, o_setCardSwap);
}

void Slides::disablePersistentScripts() {
	_cardSwapEnabled = false;
}

void Slides::runPersistentScripts() {
	if (_cardSwapEnabled) {
		// Used on Cards...
		if (_vm->getTotalPlayTime() > _nextCardTime) {
			_vm->_gfx->fadeToBlack();
			_vm->changeToCard(_nextCardID, kNoTransition);
			_vm->_gfx->fadeFromBlack();
		}
	}
}

void Slides::o_returnToMenu(uint16 var, const ArgumentsArray &args) {
	// Go to the information screens of the menu
	_vm->changeToStack(kDemoStack, 2002, 0, 0);
}

void Slides::o_setCardSwap(uint16 var, const ArgumentsArray &args) {
	_nextCardID = args[0];

	_nextCardTime = _vm->getTotalPlayTime() + 5000;
	_cardSwapEnabled = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
