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

#define OPCODE(op, x) { op, &MystScriptParser::x, #x }
#define SPECIFIC_OPCODE(op, x) { op, (OpcodeProcMyst) &MystScriptParser_Credits::x, #x }

MystScriptParser_Credits::MystScriptParser_Credits(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
	_invokingResource = NULL;
}

MystScriptParser_Credits::~MystScriptParser_Credits() {
}

void MystScriptParser_Credits::setupOpcodes() {
	// "invalid" opcodes do not exist or have not been observed
	// "unknown" opcodes exist, but their meaning is unknown

	static const MystOpcode creditsOpcodes[] = {
		// "Standard" Opcodes
		OPCODE(0, o_toggleVar),
		OPCODE(1, o_setVar),
		OPCODE(2, o_changeCardSwitch),
		OPCODE(3, o_takePage),
		OPCODE(4, o_redrawCard),
		// TODO: Opcode 5 Not Present
		OPCODE(6, o_goToDest),
		OPCODE(7, o_goToDest),
		OPCODE(8, o_goToDest),
		OPCODE(9, o_triggerMovie),
		OPCODE(10, o_toggleVarNoRedraw),
		// TODO: Opcode 10 to 11 Not Present
		OPCODE(12, o_changeCardSwitch),
		OPCODE(13, o_changeCardSwitch),
		OPCODE(14, o_drawAreaState),
		OPCODE(15, o_redrawAreaForVar),
		OPCODE(16, o_changeCardDirectional),
		OPCODE(17, o_changeCardPush),
		OPCODE(18, o_changeCardPop),
		OPCODE(19, o_enableAreas),
		OPCODE(20, o_disableAreas),
		OPCODE(21, o_directionalUpdate),
		OPCODE(22, o_goToDest),
		OPCODE(23, o_toggleAreasActivation),
		OPCODE(24, o_playSound),
		// TODO: Opcode 25 Not Present
		OPCODE(26, o_stopSoundBackground),
		OPCODE(27, o_playSoundBlocking),
		OPCODE(28, o_restoreDefaultRect),
		OPCODE(29, o_blitRect),
		OPCODE(30, o_changeSound),
		OPCODE(31, o_soundPlaySwitch),
		OPCODE(32, o_soundResumeBackground),
		OPCODE(33, o_blitRect),
		OPCODE(34, o_changeCard),
		OPCODE(35, o_drawImageChangeCard),
		OPCODE(36, o_changeMainCursor),
		OPCODE(37, o_hideCursor),
		OPCODE(38, o_showCursor),
		OPCODE(39, o_delay),
		OPCODE(40, o_changeStack),
		OPCODE(41, o_changeCardPlaySoundDirectional),
		OPCODE(42, o_directionalUpdatePlaySound),
		OPCODE(43, o_saveMainCursor),
		OPCODE(44, o_restoreMainCursor),
		// TODO: Opcode 45 Not Present
		OPCODE(46, o_soundWaitStop),
		// TODO: Opcodes 47 to 99 Not Present

		// "Stack-Specific" Opcodes
		SPECIFIC_OPCODE(100, o_quit),

		// "Init" Opcodes
		SPECIFIC_OPCODE(200, o_runCredits),

		OPCODE(0xFFFF, NOP)
	};

	_opcodes = creditsOpcodes;
	_opcodeCount = ARRAYSIZE(creditsOpcodes);
}

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
