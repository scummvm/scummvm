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

#ifndef MYST_SCRIPTS_PREVIEW_H
#define MYST_SCRIPTS_PREVIEW_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_stacks/myst.h"

namespace Mohawk {

struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Preview : public Myst {
public:
	Preview(MohawkEngine_Myst *vm);
	~Preview();

private:
	void setupOpcodes();

	DECLARE_OPCODE(opcode_196);
	DECLARE_OPCODE(opcode_197);
	DECLARE_OPCODE(opcode_198);
	DECLARE_OPCODE(opcode_199);

	DECLARE_OPCODE(opcode_298);
	DECLARE_OPCODE(opcode_299);
};

} // End of namespace MystStacks
} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
