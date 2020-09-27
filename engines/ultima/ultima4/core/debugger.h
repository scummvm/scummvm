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
#include "ultima/ultima4/core/debugger_actions.h"
#include "ultima/shared/engine/debugger.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Debugger base class
 */
class Debugger : public Shared::Debugger, public DebuggerActions {
private:
	MapTile _horse, _ship, _balloon;
	bool _dontEndTurn;
protected:
	/**
	 * Returns true if the debugger is active
	 */
	bool isDebuggerActive() const override {
		return isActive();
	}

	/**
	 * Process the given command line.
	 * Returns true if and only if argv[0] is a known command and was
	 * handled, false otherwise.
	 */
	bool handleCommand(int argc, const char **argv, bool &keepRunning) override;

	/**
	 * Prints a message to the console if it's active, or to the
	 * game screen if not
	 */
	void print(const char *fmt, ...) override;

	/**
	 * Prints a message to the console if it's active, or to the
	 * game screen if not, with no newline
	 */
	void printN(const char *fmt, ...) override;

	/**
	 * Prompts for input, but only if debugger isn't running
	 */
	void prompt() override;

	/**
	 * Gets the direction for an action
	 */
	Direction getDirection(int argc, const char **argv);

	/**
	 * Used by methods so that when they're triggered by a keybinding
	 * action, stops the turn from being finished when they're done
	 */
	void dontEndTurn() {
		_dontEndTurn = true;
	}
private:
	/**
	 * Move the avatar in a given direction
	 */
	bool cmdMove(int argc, const char **argv);

	/**
	 * Attack
	 */
	bool cmdAttack(int argc, const char **argv);

	/**
	 * Board transport
	 */
	bool cmdBoard(int argc, const char **argv);

	/**
	 * Cast spell
	 */
	bool cmdCastSpell(int argc, const char **argv);

	/**
	 * Climb
	 */
	bool cmdClimb(int argc, const char **argv);

	/**
	 * Descend
	 */
	bool cmdDescend(int argc, const char **argv);

	/**
	 * Enter location
	 */
	bool cmdEnter(int argc, const char **argv);

	/**
	 * Exit
	 */
	bool cmdExit(int argc, const char **argv);

	/**
	 * Fire
	 */
	bool cmdFire(int argc, const char **argv);

	/**
	 * Get chest
	 */
	bool cmdGetChest(int argc, const char **argv);

	/**
	 * Hole Up & Camp
	 */
	bool cmdCamp(int argc, const char **argv);

	/**
	 * Ignite Torch
	 */
	bool cmdIgnite(int argc, const char **argv);

	/**
	 * Generic interaction
	 */
	bool cmdInteract(int argc, const char **argv);

	/**
	 * Jimmy lock
	 */
	bool cmdJimmy(int argc, const char **argv);

	/**
	 * Locate position
	 */
	bool cmdLocate(int argc, const char **argv);

	/**
	 * Mix reagents
	 */
	bool cmdMixReagents(int argc, const char **argv);

	/**
	 * Exchanges the position of two players in the party.  Prompts the
	 * user for the player numbers.
	 */
	bool cmdNewOrder(int argc, const char **argv);

	/**
	 * Open door
	 */
	bool cmdOpenDoor(int argc, const char **argv);

	/**
	 * Specifies a particular party number
	 */
	bool cmdParty(int argc, const char **argv);

	/**
	 * Pass turn
	 */
	bool cmdPass(int argc, const char **argv);

	/**
	 * Peer
	 */
	bool cmdPeer(int argc, const char **argv);

	/**
	 * Save and quit
	 */
	bool cmdQuitAndSave(int argc, const char **argv);

	/**
	 * Readies a weapon for a player.  Prompts for the player and/or the
	 * weapon if not provided.
	 */
	bool cmdReadyWeapon(int argc, const char **argv);

	/**
	 * Search
	 */
	bool cmdSearch(int argc, const char **argv);

	/**
	 * Speed up, down, or normal
	 */
	bool cmdSpeed(int argc, const char **argv);

	/**
	 * Combat speed up, down, or normal
	 */
	bool cmdCombatSpeed(int argc, const char **argv);


	/**
	 * Show character stats
	 */
	bool cmdStats(int argc, const char **argv);

	/**
	 * Talk
	 */
	bool cmdTalk(int argc, const char **argv);

	/**
	 * Use
	 */
	bool cmdUse(int argc, const char **argv);

	/**
	 * Changes a player's armor
	 */
	bool cmdWearArmor(int argc, const char **argv);

	/**
	 * Yell
	 */
	bool cmdYell(int argc, const char **argv);
private:
	/**
	 * Collision detection on/off
	 */
	bool cmd3d(int argc, const char **argv);

	/**
	 * Teleports to the Abyss final altar
	 */
	bool cmdAbyss(int argc, const char **argv);

	/**
	 * Collision detection on/off
	 */
	bool cmdCollisions(int argc, const char **argv);

	/**
	 * Have all the companions join the party
	 */
	bool cmdCompanions(int argc, const char **argv);

	/**
	 * Toggle whether combat occurs
	 */
	bool cmdCombat(int argc, const char **argv);

	/**
	 * Destroy an object
	 */
	bool cmdDestroy(int argc, const char **argv);

	/**
	 * Destroy all creatures
	 */
	bool cmdDestroyCreatures(int argc, const char **argv);

	/**
	 * Jumps to a given dungeon
	 */
	bool cmdDungeon(int argc, const char **argv);

	/**
	 * Flee from combat
	 */
	bool cmdFlee(int argc, const char **argv);

	/**
	 * All equipement
	 */
	bool cmdEquipment(int argc, const char **argv);

	/**
	 * Full stats
	 */
	bool cmdFullStats(int argc, const char **argv);

	/**
	 * Toggle hunger on or off
	 */
	bool cmdHunger(int argc, const char **argv);

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
	bool cmdLorddBritish(int argc, const char **argv);

	/**
	 * Grant karma
	 */
	bool cmdKarma(int argc, const char **argv);

	/**
	 * Give all the items
	 */
	bool cmdItems(int argc, const char **argv);

	/**
	 * Leave the current location
	 */
	bool cmdLeave(int argc, const char **argv);

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
	 * Toggle overhead view
	 */
	bool cmdOverhead(int argc, const char **argv);

	/**
	 * Give all the reagents
	 */
	bool cmdReagents(int argc, const char **argv);

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

	/**
	 * Lists the triggers in a dungeon room
	 */
	bool cmdListTriggers(int argc, const char **argv);
public:
	bool _collisionOverride;
	bool _disableHunger;
	bool _disableCombat;
public:
	Debugger();
	~Debugger() override;

	/**
	 * Gets a chest.
	 * If the default -2 is used, it bypasses prompting for a
	 * user. Otherwise, a non-negative player number is expected
	 */
	void getChest(int player = -2);
};

extern Debugger *g_debugger;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
