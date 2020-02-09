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

#ifndef MADS_GAME_DRAGONSPHERE_H
#define MADS_GAME_DRAGONSPHERE_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/globals.h"
#include "mads/dragonsphere/globals_dragonsphere.h"

namespace MADS {

namespace Dragonsphere {

// TODO: Adapt for Dragonsphere's difficulty setting
enum StoryMode { STORYMODE_NAUGHTY = 1, STORYMODE_NICE = 2 };

enum InventoryObject {
	OBJ_NONE = -1,
	OBJ_SIGNET_RING = 0,
	OBJ_BIRD_FIGURINE = 1,
	OBJ_BIRDCALL = 2,
	OBJ_SHIELDSTONE = 3,
	OBJ_SWORD = 4,
	OBJ_GOBLET = 5,
	OBJ_BONE = 6,
	OBJ_FRUIT = 7,
	OBJ_DOLL = 8,
	OBJ_POLYSTONE = 9,
	OBJ_RED_STONE = 10,
	OBJ_YELLOW_STONE = 11,
	OBJ_BLUE_STONE = 12,
	OBJ_KEY_CROWN = 13,
	OBJ_DATES = 14,
	OBJ_STATUE = 15,
	OBJ_FLIES = 16,
	OBJ_SOUL_EGG = 17,
	OBJ_MAGIC_BELT = 18,
	OBJ_AMULET = 19,
	OBJ_MUD = 20,
	OBJ_FEATHERS = 21,
	OBJ_TORCH = 22,
	OBJ_FLASK = 23,
	OBJ_FLASK_OF_ACID = 24,
	OBJ_ROPE = 25,
	OBJ_VORTEX_STONE = 26,
	OBJ_DEAD_RAT = 27,
	OBJ_MAP = 28,
	OBJ_CRYSTAL_BALL = 29,
	OBJ_BLACK_SPHERE = 30,
	OBJ_SOPORIFIC = 31,
	OBJ_SHIFTER_RING = 32,
	OBJ_SPIRIT_BUNDLE = 33,
	OBJ_PARTIAL_BUNDLE = 34,
	OBJ_RATSICLE = 35,
	OBJ_TENTACLE_PARTS = 36,
	OBJ_TELEPORT_DOOR = 37,
	OBJ_RARE_COIN = 38,
	OBJ_CRYSTAL_FLOWER = 39,
	OBJ_DIAMOND_DUST = 40,
	OBJ_RUBY_RING = 41,
	OBJ_GOLD_NUGGET = 42,
	OBJ_MAGIC_MUSIC_BOX = 43,
	OBJ_EMERALD = 44,
	OBJ_PARCHMENT = 45,
	OBJ_GAME = 46,
	OBJ_GAME2 = 47,
	OBJ_NEW_BUNDLE = 48
};

class GameDragonsphere : public Game {
	friend class Game;
protected:
	GameDragonsphere(MADSEngine *vm);

	void startGame() override;

	void initializeGlobals() override;

	void setSectionHandler() override;

	void checkShowDialog() override;
public:
	DragonsphereGlobals _globals;
	StoryMode _storyMode;

	Globals &globals() override { return _globals; }

	void doObjectAction() override;

	void unhandledAction() override;

	void step() override;

	void synchronize(Common::Serializer &s, bool phase1) override;
};


class Section1Handler : public SectionHandler {
public:
	Section1Handler(MADSEngine *vm) : SectionHandler(vm) {}

	// TODO: Properly implement handler methods
	void preLoadSection() override {}
	void sectionPtr2() override {}
	void postLoadSection() override {}
};

// TODO: Properly implement handler classes
typedef Section1Handler Section2Handler;
typedef Section1Handler Section3Handler;
typedef Section1Handler Section4Handler;
typedef Section1Handler Section5Handler;
typedef Section1Handler Section6Handler;
typedef Section1Handler Section7Handler;
typedef Section1Handler Section8Handler;

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_GAME_DRAGONSPHERE_H */
