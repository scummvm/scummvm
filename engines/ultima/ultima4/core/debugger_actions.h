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

#ifndef ULTIMA4_CORE_DEBUGGER_ACTIONS_H
#define ULTIMA4_CORE_DEBUGGER_ACTIONS_H

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/shared/engine/debugger.h"

namespace Ultima {
namespace Ultima4 {

/**
 * This is a secondary class inherited by the Debugger class
 * that contains various support methods for implementing the
 * different actions players can take in the game
 */
class DebuggerActions {
private:
	/**
	 * Executes the current conversation until it is done.
	 */
	void talkRunConversation(Conversation &conv, Person *talker, bool showPrompt);

	/**
	 * Check the levels of each party member while talking to Lord British
	 */
	void gameLordBritishCheckLevels();
protected:
	/**
	 * Returns true if the debugger is active
	 */
	virtual bool isDebuggerActive() const = 0;

	/**
	 * Prints a message to the console if it's active, or to the
	 * game screen if not
	 */
	virtual void print(const char *fmt, ...) = 0;

	/**
	 * Prints a message to the console if it's active, or to the
	 * game screen if not
	 */
	virtual void printN(const char *fmt, ...) = 0;

	/**
	 * Prompts for input, but only if debugger isn't running
	 */
	virtual void prompt() = 0;

	/**
	 * Returns true if combat is currently active
	 */
	bool isCombat() const;

	/**
	 * Returns currently focused character in combat mode
	 */
	int getCombatFocus() const;
public:
	virtual ~DebuggerActions() {}

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

	/**
	 * Called by getChest() to handle possible traps on chests
	 **/
	bool getChestTrapHandler(int player);

	/**
	 * Attempts to jimmy a locked door at map coordinates x,y.  The locked
	 * door is replaced by a permanent annotation of an unlocked door
	 * tile.
	 */
	bool jimmyAt(const Coords &coords);

	/**
	 * Prompts for spell reagents to mix in the traditional Ultima IV
	 * style.
	 */
	bool mixReagentsForSpellU4(int spell);

	/**
	 * Prompts for spell reagents to mix with an Ultima V-like menu.
	 */
	bool mixReagentsForSpellU5(int spell);

	bool gameSpellMixHowMany(int spell, int num, Ingredients *ingredients);

	/**
	 * Attempts to open a door at map coordinates x,y.  The door is
	 * replaced by a temporary annotation of a floor tile for 4 turns.
	 */
	bool openAt(const Coords &coords);

	void gameCastSpell(uint spell, int caster, int param);


	/**
	 * Begins a conversation with the NPC at map coordinates x,y.  If no
	 * NPC is present at that point, zero is returned.
	 */
	bool talkAt(const Coords &coords);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
