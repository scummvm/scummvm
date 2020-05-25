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

#ifndef ULTIMA4_CONTROLLERS_GAME_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_GAME_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/views/tileview.h"

namespace Ultima {
namespace Ultima4 {

/**
 * The main game controller that handles basic game flow and keypresses.
 *
 * @todo
 *  <ul>
 *      <li>separate the dungeon specific stuff into another class (subclass?)</li>
 *  </ul>
 */
class GameController : public Controller, public Observer<Party *, PartyEvent &>, public Observer<Location *, MoveEvent &>,
	public TurnCompleter {
private:
	/**
	 * Handles feedback after avatar moved during normal 3rd-person view.
	 */
	void avatarMoved(MoveEvent &event);

	/**
	 * Handles feedback after moving the avatar in the 3-d dungeon view.
	 */
	void avatarMovedInDungeon(MoveEvent &event);

	/**
	 * Removes creatures from the current map if they are too far away from the avatar
	 */
	void creatureCleanup();

	/**
	 * Handles trolls under bridges
	 */
	void checkBridgeTrolls();

	/**
	 * Checks creature conditions and spawns new creatures if necessary
	 */
	void checkRandomCreatures();

	/**
	 * Checks for valid conditions and handles
	 * special creatures guarding the entrance to the
	 * abyss and to the shrine of spirituality
	 */
	void checkSpecialCreatures(Direction dir);

	/**
	 * Checks for and handles when the avatar steps on a moongate
	 */
	bool checkMoongates();

	/**
	 * Creates the balloon near Hythloth, but only if the balloon doesn't already exists somewhere
	 */
	bool createBalloon(Map *map);

	/**
	 * Attempts to attack a creature at map coordinates x,y.  If no
	 * creature is present at that point, zero is returned.
	 */
	bool attackAt(const Coords &coords);
public:
	/**
	 * Show an attack flash at x, y on the current map.
	 * This is used for 'being hit' or 'being missed'
	 * by weapons, cannon fire, spells, etc.
	 */
	static void flashTile(const Coords &coords, MapTile tile, int timeFactor);

	static void flashTile(const Coords &coords, const Common::String &tilename, int timeFactor);
	static void doScreenAnimationsWhilePausing(int timeFactor);
public:
	TileView _mapArea;
	bool _paused;
	int _pausedTimer;
public:
	GameController();

	/* controller functions */

	/**
	 * Called when a controller is made active
	 */
	void setActive() override;

	/**
	 * Keybinder actions
	 */
	void keybinder(KeybindingAction action) override;

	/**
	 * Mouse button was pressed
	 */
	bool mousePressed(const Common::Point &mousePos) override;

	/**
	 * This function is called every quarter second.
	 */
	void timerFired() override;

	/* main game functions */
	void init();
	void initScreen();
	void initScreenWithoutReloadingState();
	void setMap(Map *map, bool saveLocation, const Portal *portal, TurnCompleter *turnCompleter = nullptr);

	/**
	 * Exits the current map and location and returns to its parent location
	 * This restores all relevant information from the previous location,
	 * such as the map, map position, etc. (such as exiting a city)
	 **/
	int exitToParentMap();

	/**
	 * Terminates a game turn.  This performs the post-turn housekeeping
	 * tasks like adjusting the party's food, incrementing the number of
	 * moves, etc.
	 */
	void finishTurn() override;

	/**
	 * Provide feedback to user after a party event happens.
	 */
	void update(Party *party, PartyEvent &event) override;

	/**
	 * Provide feedback to user after a movement event happens.
	 */
	void update(Location *location, MoveEvent &event) override;

	/**
	 * Initializes the moon state according to the savegame file. This method of
	 * initializing the moons (rather than just setting them directly) is necessary
	 * to make sure trammel and felucca stay in sync
	 */
	void initMoons();

	/**
	 * Updates the phases of the moons and shows
	 * the visual moongates on the map, if desired
	 */
	void updateMoons(bool showmoongates);

	void attack(Direction dir = DIR_NONE);
};

extern GameController *g_game;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
