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

#ifndef MYST_VARS_H
#define MYST_VARS_H

namespace Mohawk {

struct MystVarEntry {
	uint16 refNum;
	uint16 storage; // Used for Initial Value setting
	const char *description;
};

class MystVar {
public:
	MystVar(MohawkEngine_Myst *vm);
	~MystVar();

	// Only for use by Save/Load
	// All other code should use getVar() / setVar()
	void loadSetVar(uint16 stack, uint16 v, uint16 value);
	uint16 saveGetVar(uint16 stack, uint16 v);

	uint16 getVar(uint16 v);
	void setVar(uint16 v, uint16 value);

private:
	MohawkEngine_Myst *_vm;

	Common::Array<MystVarEntry> _unknown;
};

} // End of namespace Mohawk

#endif
