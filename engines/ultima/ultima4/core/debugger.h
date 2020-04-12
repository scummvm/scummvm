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

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/shared/engine/debugger.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Debugger base class
 */
class Debugger : public Shared::Debugger {
private:
	MapTile _horse, _ship, _balloon;
private:
	/**
	 * Prints a message to the console if it's active, or to the
	 * game screen if not
	 */
	void print(const char *fmt, ...);

	/**
	 * Summons a creature given by 'creatureName'. This can either be given
	 * as the creature's name, or the creature's id.  Once it finds the
	 * creature to be summoned, it calls gameSpawnCreature() to spawn it.
	 */
	void summonCreature(const Common::String &name);

	/**
	 * Destroy object at a given co-ordinate
	 */
	bool destroyAt(const Coords &coords);

	/**
	 * Returns a direction from a given string
	 */
	Direction directionFromName(const Common::String &dirStr);
private:
	/**
	 * Move the avatar in a given direction
	 */
	bool cmdMove(int argc, const char **argv);

private:
	/**
	 * Collision detection on/off
	 */
	bool cmd3d(int argc, const char **argv);

	/**
	 * Collision detection on/off
	 */
	bool cmdCollisions(int argc, const char **argv);

	/**
	 * Have all the companions join the party
	 */
	bool cmdCompanions(int argc, const char **argv);

	/**
	 * Destroy an object
	 */
	bool cmdDestroy(int argc, const char **argv);

	/**
	 * Jumps to a given dungeon
	 */
	bool cmdDungeon(int argc, const char **argv);

	/**
	 * All equipement
	 */
	bool cmdEquipment(int argc, const char **argv);

	/**
	 * Exit the current location
	 */
	bool cmdExit(int argc, const char **argv);

	/**
	 * Moongate teleportation
	 */
	bool cmdGate(int argc, const char **argv);

	/**
	 * Go to any specified location by name
	 */
	bool cmdGoto(int argc, const char **argv);

	/**
	 * Help.. sends the party to Lord British
	 */
	bool cmdHelp(int argc, const char **argv);

	/**
	 * Grant karma
	 */
	bool cmdKarma(int argc, const char **argv);

	/**
	 * Give all the items
	 */
	bool cmdItems(int argc, const char **argv);

	/**
	 * Displays the current location
	 */
	bool cmdLocation(int argc, const char **argv);

	/**
	 * Give all the mixtures
	 */
	bool cmdMixtures(int argc, const char **argv);

	/**
	 * Moon phase
	 */
	bool cmdMoon(int argc, const char **argv);

	/**
	 * Toggle opacity
	 */
	bool cmdOpacity(int argc, const char **argv);

	/**
	 * Peer
	 */
	bool cmdPeer(int argc, const char **argv);

	/**
	 * Give all the reagents
	 */
	bool cmdReagents(int argc, const char **argv);

	/**
	 * Full stats
	 */
	bool cmdStats(int argc, const char **argv);

	/**
	 * Summons a creature to fight
	 */
	bool cmdSummon(int argc, const char **argv);

	/**
	 * Returns the torch duration
	 */
	bool cmdTorch(int argc, const char **argv);

	/**
	 * Creates a given transport
	 */
	bool cmdTransport(int argc, const char **argv);

	/**
	 * Move up a floor
	 */
	bool cmdUp(int argc, const char **argv);

	/**
	 * Move down a floor
	 */
	bool cmdDown(int argc, const char **argv);

	/**
	 * Gives full virtue, or increments a specific virtue
	 */
	bool cmdVirtue(int argc, const char **argv);

	/**
	 * Set wind direction or locks the direction
	 */
	bool cmdWind(int argc, const char **argv);

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
