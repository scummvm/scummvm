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

#ifndef ULTIMA4_CORE_DEBUGGER_H
#define ULTIMA4_CORE_DEBUGGER_H

#include "ultima/shared/engine/debugger.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Debugger base class
 */
class Debugger : public Shared::Debugger {
private:
	/**
	 * Prints a message to the console if it's active, or to the
	 * game screen if not
	 */
	void print(const char *fmt, ...);

	/**
	 * Collision detection on/off
	 */
	bool cmdCollisions(int argc, const char **argv);

	/**
	 * All equipement
	 */
	bool cmdEquipment(int argc, const char **argv);

	/**
	 * Moongate teleportation
	 */
	bool cmdGate(int argc, const char **argv);

	/**
	 * Go to any specified location by name
	 */
	bool cmdGoto(int argc, const char **argv);

	/**
	 * Moon phase
	 */
	bool cmdMoon(int argc, const char **argv);

	/**
	 * Full stats
	 */
	bool cmdStats(int argc, const char **argv);
public:
	bool _collisionOverride;
public:
	Debugger();
	~Debugger() override;
};

extern Debugger *g_debugger;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
