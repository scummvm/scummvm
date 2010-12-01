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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/credits.h"

#include "gui/message.h"

namespace Mohawk {

// NOTE: Credits Start Card is 10000

MystScriptParser_Credits::MystScriptParser_Credits(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

MystScriptParser_Credits::~MystScriptParser_Credits() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Credits::x, #x))

void MystScriptParser_Credits::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_quit);

	// "Init" Opcodes
	OPCODE(200, o_runCredits);
}

#undef OPCODE

void MystScriptParser_Credits::disablePersistentScripts() {
	_creditsRunning = false;
	_creditsVar = 0;
	_baseImageId = 0;
	_lastCardTime = 0;
}

void MystScriptParser_Credits::runPersistentScripts() {
	if (!_creditsRunning)
		return;

	uint16 curImageIndex = _vm->_varStore->getVar(_creditsVar);

	if (_vm->_system->getMillis() - _lastCardTime >= 7 * 1000) {
		// After the 6th image has shown, it's time to quit
		if (curImageIndex == 7)
			_vm->_system->quit();

		// Note: The modulus by 6 is because the 6th image is the one at imageBaseId
		_vm->_gfx->copyImageToScreen(_baseImageId + curImageIndex % 6, Common::Rect(0, 0, 544, 333));
		_vm->_gfx->updateScreen();

		_vm->_varStore->setVar(_creditsVar, curImageIndex + 1);
		_lastCardTime = _vm->_system->getMillis();
	}
}

void MystScriptParser_Credits::o_quit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->_system->quit();
}

void MystScriptParser_Credits::o_runCredits(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Activate the credits
	_creditsRunning = true;
	_creditsVar = var;
	_baseImageId = _vm->getCurCard();
	_lastCardTime = _vm->_system->getMillis();

	_vm->_varStore->setVar(var, 1);
}

} // End of namespace Mohawk
