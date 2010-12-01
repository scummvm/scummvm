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
#include "mohawk/myst_stacks/makingof.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_MakingOf::MystScriptParser_MakingOf(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

MystScriptParser_MakingOf::~MystScriptParser_MakingOf() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_MakingOf::x, #x))

void MystScriptParser_MakingOf::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_quit);
}

#undef OPCODE

void MystScriptParser_MakingOf::disablePersistentScripts() {
}

void MystScriptParser_MakingOf::runPersistentScripts() {
}

void MystScriptParser_MakingOf::o_quit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->_system->quit();
}

} // End of namespace Mohawk
