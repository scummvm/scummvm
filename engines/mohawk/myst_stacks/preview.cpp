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
 */

#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/preview.h"

#include "common/system.h"
#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Preview::Preview(MohawkEngine_Myst *vm) : Myst(vm) {
	setupOpcodes();
}

Preview::~Preview() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Preview::x, #x))

#define OVERRIDE_OPCODE(opcode, x) \
	for (uint32 i = 0; i < _opcodes.size(); i++) \
		if (_opcodes[i]->op == opcode) { \
			_opcodes[i]->proc = (OpcodeProcMyst) &Preview::x; \
			_opcodes[i]->desc = #x; \
			break; \
		}

void Preview::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OVERRIDE_OPCODE(196, opcode_196);
	OVERRIDE_OPCODE(197, opcode_197);
	OVERRIDE_OPCODE(198, opcode_198);
	OVERRIDE_OPCODE(199, opcode_199);

	// "Init" Opcodes
	OPCODE(298, opcode_298);
	OPCODE(299, opcode_299);
}

#undef OPCODE
#undef OVERRIDE_OPCODE

void Preview::opcode_196(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card ...
	// TODO: Finish Implementation...
	// Voice Over and Card Advance?
}

void Preview::opcode_197(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card ...
	// TODO: Finish Implementation...
	// Voice Over and Card Advance?
}

// TODO: Merge with Opcode 42?
void Preview::opcode_198(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Nuh-uh! No leaving the library in the demo!
		GUI::MessageDialog dialog("You can't leave the library in the demo.");
		dialog.runModal();
	} else
		unknown(op, var, argc, argv);
}

void Preview::opcode_199(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card ...
	// TODO: Finish Implementation...
	// Voice Over and Card Advance?
}

void Preview::opcode_298(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 3000 (Closed Myst Book)
	// TODO: Fill in logic.
	// Start Voice Over... which controls book opening
	_vm->_sound->replaceSoundMyst(3001);

	// then link to Myst - Trigger of Hotspot? then opcode 199/196/197 for voice over continue?
	// TODO: Sync Voice and Actions to Original
	// TODO: Flash Library Red
	// TODO: Move to run process based delay to prevent
	//       blocking...
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(20 * 1000);

	for (uint16 imageId = 3001; imageId <= 3012; imageId++) {
		_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(5 * 1000);
	}
}

void Preview::opcode_299(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 3002 (Myst Island Overview)
	// TODO: Fill in logic.
	// Zoom into Island?
	// On this card is a Type 8 controlled by Var 0, which
	// can change the Myst Library to Red..
}

} // End of namespace MystStacks
} // End of namespace Mohawk
