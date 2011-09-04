/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/myst3/myst3.h"
#include "engines/myst3/script.h"
#include "engines/myst3/hotspot.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Script::Script(Myst3Engine *vm):
	_vm(vm) {
#define OPCODE(op, x) _commands.push_back(Command(op, &Script::x, #x))

	OPCODE(53, varSetValue);
	OPCODE(138, goToNode);
	OPCODE(139, goToRoomNode);
	OPCODE(187, runScriptsFromNode);

#undef OPCODE
}

Script::~Script() {
}

void Script::run(Common::Array<Opcode> *script) {
	debugC(kDebugScript, "Script start %p", (void *) script);

	for (uint i = 0; i < script->size(); i++) {
		runOp(script->operator[](i));
	}

	debugC(kDebugScript, "Script stop %p ", (void *) script);
}

void Script::runOp(const Opcode &op) {
	bool ranOpcode = false;

	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op == op.op) {
			(this->*(_commands[i].proc)) (op);
			ranOpcode = true;
			break;
		}

	if (!ranOpcode)
		warning("Trying to run invalid opcode %d", op.op);
}

const Common::String Script::describeCommand(uint16 op) {
	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op == op)
			return Common::String::format("%d, %s", op, _commands[i].desc);

	return Common::String::format("%d", op);
}

void Script::varSetValue(const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->set(cmd.args[0], cmd.args[1]);
}

void Script::goToNode(const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0]);
}

void Script::goToRoomNode(const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to room %d, node %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->goToNode(cmd.args[1], cmd.args[0]);
}

void Script::runScriptsFromNode(const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run scripts from node %d", cmd.op, cmd.args[0]);

	warning("Unimplemented opcode %d", cmd.op);
}

} /* namespace Myst3 */
