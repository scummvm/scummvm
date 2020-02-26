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

	/**
	 * Casts a spell
	 */
	bool cmdSpell(int argc, const char **argv);

	/**
	 * Gives all the characters a full spellbook
	 */
	bool cmdSpells(int argc, const char **argv);

	/**
	 * Dumps a resource to a file
	 */
	bool cmdDump(int argc, const char **argv);

	/**
	 * Gives gold to the party or bank
	 */
	bool cmdGold(int argc, const char **argv);

	/**
	 * Gives gems to the party or bank
	 */
	bool cmdGems(int argc, const char **argv);

	/**
	 * Jumps to a given map, and optionally a given position
	 */
	bool cmdMap(int argc, const char **argv);

	/**
	 * Changes the party's position in the current map
	 */
	bool cmdPos(int argc, const char **argv);

	/**
	 * Flags whether to make the party invincible
	 */
	bool cmdInvincible(int argc, const char **argv);

	/**
	 * Flags whether to make the party super-strength attacks
	 */
	bool cmdSuperStrength(int argc, const char **argv);

	/**
	 * Flags whether to make the party invincible
	 */
	bool cmdIntangible(int argc, const char **argv);
public:
	bool _invincible;
	bool _intangible;
	bool _superStrength;
public:
	Debugger(XeenEngine *vm);

	/**
	 * Updates the debugger
	 */
	virtual void onFrame() override;
};

} // End of namespace Xeen

#endif	/* XEEN_DEBUGGER_H */
