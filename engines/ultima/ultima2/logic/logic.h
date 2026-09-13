/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA2_LOGIC_LOGIC_H
#define ULTIMA2_LOGIC_LOGIC_H

#include "ultima/ultima2/data/data.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

/**
 * Base class for gameplay handling logic - one virtual method per player
 * command letter, with a default fallback body for each. Concrete
 * subclasses per mode (overworld, city/castle, dungeon, space) override
 * only the commands that differ there.
 */
class Logic {
protected:
	/**
	 * Dispatches some text to be shown in the Commands window
	 */
	void writeString(const Common::String &msg);
	void writeString(const char *format, ...);

	/**
	 * Triggers a prompt display in the Commands window
	 */
	void prompt();

	/**
	 * Handles the player dying - clears their resources and returns to
	 * the title screen
	 */
	void playerDied();

	/**
	 * Alerts nearby town guards after an attack or theft. A no-op outside
	 * of towns/castles/villages (_mapNum2 == 0), which is the only mode
	 * implemented so far
	 */
	void alertTownGuards();

	/**
	 * Signal the map to redraw
	 */
	void redrawMap();

	/**
	 * Signal the stats to redraw
	 */
	void redrawStats();

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber);
	int getRandomNumber(int maxNumber);

	/**
	 * Returns the next byte from the game's random number generator
	 */
	byte randByte();

	/**
	 * Has the currently focused map view briefly flash the attack sprite
	 * over a map position - a hit indicator, pausing before returning.
	 * See AttackTileMessage
	 */
	void showAttackTile(int x, int y);

	/**
	 * Plays a sound effect. Currently a no-op stub, mirroring ultima1's
	 * own unimplemented playFX
	 */
	void playFX(int num);

	/**
	 * Handles end of turn logic
	 */
	virtual void endOfTurn();

	/**
	 * Handles updating creatures/NPCs
	 */
	virtual void updateCreatures() {
	}

	/*--- Fallback player command handlers. Each returns true if end of
	 * turn processing should happen immediately afterward, or false if
	 * that's being deferred - typically because a sub-dialog was opened
	 * that will only complete the action once it closes ---*/

	virtual bool move(Data::Direction dir) {
		return false;
	}
	virtual bool board();
	virtual bool cast();
	virtual bool descend();
	virtual bool enter();
	virtual bool fire();
	virtual bool get();
	virtual bool hyper();
	virtual bool igniteTorch();
	virtual bool jump();
	virtual bool klimb();
	virtual bool launch();
	virtual bool magic();
	virtual bool negateTime();
	virtual bool offer();
	virtual bool pass();
	virtual bool quit();
	virtual bool ready();
	virtual bool steal();
	virtual bool transact();
	virtual bool unlock();
	virtual bool view();
	virtual bool wearArmor();
	virtual bool xit();
	virtual bool yell();
	virtual bool zstats();

	/*---------------------------------------*/
public:
	virtual ~Logic() {
	}

	virtual void action(int action);
	virtual void keypress(const Shared::Messages::KeypressMessage &msg);
	virtual void entering() {
	}

	/**
	 * Attacks in the given direction, or - if unspecified - starts the
	 * Direction interaction to prompt for one. Public since the Direction
	 * interaction calls back into it directly once a direction is chosen
	 */
	virtual bool attack(Data::Direction dir = Data::DIR_UNSPECIFIED);

	/**
	 * Called once per game frame for any logic updates
	 */
	virtual void tick() {
	}

	/**
	 * Resumes end of turn processing after a command deferred it (returned
	 * false from action()) to run some sub-interaction first
	 */
	void resumeTurn() {
		endOfTurn();
		prompt();
	}
};

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima

#endif
