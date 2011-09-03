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

namespace Myst3 {

Script::Script(Myst3Engine *vm):
	_vm(vm) {
}

Script::~Script() {
	// TODO Auto-generated destructor stub
}

void Script::run(Common::Array<Opcode> *script) {
	debug("Script start %p", (void *) script);

	for (uint i = 0; i < script->size(); i++) {
		runOp(&script->operator[](i));
	}

	debug("Script stop %p ", (void *) script);
}

void Script::runOp(Opcode *op) {
	debug("opcode %d", op->op);

	for (uint i = 0; i < op->args.size(); i++) {
		debug("\targ %d: %d", i, op->args[i]);
	}

	switch (op->op) {
	case 138:
		_vm->goToNode(op->args[0]);
		break;
	case 139:
		_vm->goToNode(op->args[1], op->args[0]);
		break;
	default:
		break;
	}
}

} /* namespace Myst3 */
