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

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "common/array.h"

namespace Myst3 {

class Myst3Engine;
struct Opcode;

#define DECLARE_OPCODE(x) void x(const Opcode &cmd)

class Script {
public:
	Script(Myst3Engine *vm);
	virtual ~Script();

	void run(Common::Array<Opcode> *script);
	const Common::String describeCommand(uint16 op);

private:
	typedef void (Script::*CommandProc)(const Opcode &cmd);

	struct Command {
		Command() {}
		Command(uint16 o, CommandProc p, const char *d) : op(o), proc(p), desc(d) {}

		uint16 op;
		CommandProc proc;
		const char *desc;
	};

	Myst3Engine *_vm;
	Common::Array<Command> _commands;

	void runOp(const Opcode &op);

	DECLARE_OPCODE(goToNode);
	DECLARE_OPCODE(goToRoomNode);
};

} /* namespace Myst3 */
#endif /* SCRIPT_H_ */
