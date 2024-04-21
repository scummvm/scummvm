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

#ifdef ENABLE_MADSV2

#ifndef MADS_GAME_FOREST_H
#define MADS_GAME_FOREST_H

#include "mads/game.h"
#include "mads/globals.h"
#include "mads/forest/globals_forest.h"

namespace MADS {

namespace Forest {

// TODO: Adapt for Forest's difficulty setting
enum StoryMode { STORYMODE_NAUGHTY = 1, STORYMODE_NICE = 2 };

enum InventoryObject {
	OBJ_NONE = -1,
	OBJ_GEARS = 0,
	OBJ_RUBBER_BAND = 1,
	OBJ_FEATHER = 2,
	OBJ_NEEDLE = 3,
	OBJ_LILY_PAD = 4,
	OBJ_PEBBLES = 5,
	OBJ_REEDS = 6,
	OBJ_STICKS = 7,
	OBJ_TWINE = 8,
	OBJ_VINE_WEED = 9,
	OBJ_WEB = 10,
	OBJ_WOOD = 11,
	OBJ_LEAVES = 12,
	OBJ_WRENCH = 13,
	OBJ_Y_STICK = 14,
	OBJ_FORKED_STICK = 15
};

class GameForest : public Game {
	friend class Game;
protected:
	GameForest(MADSEngine *vm);

	void startGame() override;

	void initializeGlobals() override;

	void setSectionHandler() override;

	void checkShowDialog() override;
public:
	ForestGlobals _globals;
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

} // End of namespace Forest

} // End of namespace MADS

#endif /* MADS_GAME_FOREST_H */

#endif
