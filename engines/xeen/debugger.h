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

#ifndef XEEN_DEBUGGER_H
#define XEEN_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Xeen {

class XeenEngine;

class Debugger : public GUI::Debugger {
private:
	XeenEngine *_vm;
	int _spellId;

	bool cmdSpell(int argc, const char **argv);
	bool cmdDump(int argc, const char **argv);
	bool cmdGold(int argc, const char **argv);
	bool cmdGems(int argc, const char **argv);
public:
	Debugger(XeenEngine *vm);

	void update();
};

} // End of namespace Xeen

#endif	/* XEEN_DEBUGGER_H */
