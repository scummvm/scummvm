
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

#ifndef ULTIMA2_LOGIC_H
#define ULTIMA2_LOGIC_H

#include "common/events.h"
#include "common/ptr.h"
#include "ultima/ultima1/data/map.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

class DirectionLogic;

/**
 * Base class for gameplay handling logic used by the Commands class to respond to player actions
 * or other miscellaneous keypresses.
 */
class Logic {
	friend class DirectionLogic;
private:
	void combatDir(Data::Direction direction, int effect);

protected:
	// Per-point-of-strength/readied-item scaling factor used by combatDir's
	// weapon-attack strike roll - 8 for the main game, but MondainLogic
	// overrides it to 3 to match the Mondain encounter's own weaker curve
	int _weaponDamageScale = 8;


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
	 * Play a sound effect
	 */
	void playFX(int num);

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber);
	int getRandomNumber(int maxNumber);

	/**
	 * Signal the map to redraw
	 */
	void redrawMap();

	/**
	 * Has the currently focused map view briefly flash a tile on-screen at
	 * a given map position - e.g. a projectile flying towards the player,
	 * or a hit flash - pausing before returning. Used for combat
	 * animations; see AttackTileMessage
	 */
	void showAttackTile(int x, int y, int tileId);

	/**
	 * Signal the stats to redraw
	 */
	void redrawStats();

	/**
	 * Triggers combat action
	 */
	void combat(Data::Direction direction, int effect);

	/**
	 * Attack with a weapon
	 */
	virtual bool attack(Data::Direction dir);

	/**
	 * Handles end of turn logic
	 */
	virtual void endOfTurn();

	/**
	 * Handles updating creatures/NPCs
	 */
	virtual void updateCreatures() {
	}

	/**
	 * Cast spell direction chosen for overworld spells
	 */
	virtual void castSpellAttack(Data::Direction dir) {
	}

	/**
	 * Resolves a weapon/cannon-fire/spell attack in a given direction -
	 * rolls to hit, then applies damage or kills whatever's found there.
	 * The city/castle half (CityCastleLogic) attacks an NPC widget; the
	 * overworld half (OverworldLogic) attacks a monster entity - dungeons
	 * have their own separate attack handling and don't use this
	 * @param dir			Direction to attack in
	 * @param effectNum		Sound effect Id to play on a successful hit
	 * @param maxDistance	How far away a target can be and still be hit
	 * @param strike		Amount of damage to inflict
	 * @param hitChance		1-100 threshold a to-hit roll must be under
	 * @param tileId		Tile Id shown for the weapon/projectile effect
	 */
	virtual void damage(Data::Direction dir, int effectNum, int maxDistance, int strike, int hitChance, int tileId) {
	}

	/*--- Fallback player action handlers. Each returns true if end of
	 * turn processing should happen immediately afterward, or false if
	 * that's being deferred - typically because a sub-dialog was opened
	 * that will only complete the action once it closes ---*/

	virtual bool board();
	virtual bool cast();
	virtual bool climb();
	virtual bool drop();
	virtual bool enter();
	virtual bool fire();
	virtual bool get();
	virtual bool hyperjump();
	virtual bool inform();
	virtual bool noise();
	virtual bool open();
	virtual bool quit();
	virtual bool ready();
	virtual bool steal();
	virtual bool transact();
	virtual bool unlock();
	virtual bool view();
	virtual bool xit();
	virtual bool zstats();
	virtual bool move(Data::Direction dir) {
		return false;
	}
	virtual bool pass();

	/*---------------------------------------*/
public:
	virtual ~Logic() {
	}

	virtual void action(int action);
	virtual void keypress(Common::KeyCode keycode);
	virtual void entering() {
	}

	/**
	 * Called once per game frame for any logic updates
	 */
	virtual void tick() {}
};

/**
 * Bridging class used to receive back the selected direction from the Direction view, and then
 * dispatch it for either spell or weapon attack in the given direction.
 */
class DirectionLogic : public Logic {
public:
	enum Mode {
		SPELL, WEAPON, FIRE
	};
private:
	Common::SharedPtr<Logic> _oldLogic;
	Mode _mode;

public:
	DirectionLogic(Mode mode);
	~DirectionLogic() override {
	}
	void action(int action) override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
