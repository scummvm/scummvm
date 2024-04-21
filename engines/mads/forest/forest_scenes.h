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

#ifndef MADS_FOREST_SCENES_H
#define MADS_FOREST_SCENES_H

#include "mads/scene.h"
#include "mads/forest/game_forest.h"
#include "mads/forest/globals_forest.h"

namespace MADS {

namespace Forest {

enum Verb {
	VERB_LOOK = 0x3,
	VERB_TAKE = 0x4,
	VERB_PUSH = 0x5,
	VERB_OPEN = 0x6,
	VERB_PUT = 0x7,
	VERB_TALK_TO = 0x8,
	VERB_GIVE = 0x9,
	VERB_PULL = 0xa,
	VERB_CLOSE = 0xb,
	VERB_THROW = 0xc,
	VERB_WALK_TO = 0xd,
	VERB_ATTACK = 0x43,
	VERB_CARVE_UP = 0x4c,
	VERB_CLICK_ON = 0x4e,
	VERB_CLIMB_DOWN = 0x50,
	VERB_CLIMB_UP = 0x51,
	VERB_DOWN_PAGE = 0x5a,
	VERB_EAT = 0x5b,
	VERB_INVOKE = 0x6c,
	VERB_LOOK_AT = 0x72,
	VERB_MAKE_NOISE = 0x73,
	VERB_POLISH = 0x80,
	VERB_RUB = 0x88,
	VERB_THRUST = 0x98,
	VERB_TIE = 0x99,
	VERB_UP_PAGE = 0x9c,
	VERB_USE = 0x9d,
	VERB_WALK = 0x9f,
	VERB_WALK_DOWN = 0xa0,
	VERB_WEAR = 0xa1
};

enum Noun {
	NOUN_GAME = 0x1,
	NOUN_QSAVE = 0x2,
	NOUN_OWL_TREE = 0x40,
	NOUN_ABI_BUBBLE = 0x41,
	NOUN_ALL_BUBBLE = 0x42,
	NOUN_BEDROOM = 0x44,
	NOUN_BIRD_FIGURINE = 0x45,
	NOUN_BIRDCALL = 0x46,
	NOUN_BOOK = 0x47,
	NOUN_BOOKCASE = 0x48,
	NOUN_BOOKS = 0x49,
	NOUN_BOOKSHELF = 0x4a,
	NOUN_BUSH = 0x4b,
	NOUN_CHICORY = 0x4d,
	NOUN_CLIFF = 0x4f,
	NOUN_COMFREY = 0x52,
	NOUN_DANDELION = 0x53,
	NOUN_DOOR = 0x54,
	NOUN_DOOR_1 = 0x55,
	NOUN_DOOR_2 = 0x56,
	NOUN_DOOR_3 = 0x57,
	NOUN_DOOR_4 = 0x58,
	NOUN_DOOR_5 = 0x59,
	NOUN_EDGE_OF_CLIFF = 0x5c,
	NOUN_ELM_LEAVES = 0x5d,
	NOUN_EXIT_JOURNAL = 0x5e,
	NOUN_EYEBRIGHT = 0x5f,
	NOUN_FEATHER = 0x60,
	NOUN_FIVE = 0x61,
	NOUN_FLOOR = 0x62,
	NOUN_FLOWERS = 0x63,
	NOUN_FOREST = 0x64,
	NOUN_FORKED_STICK = 0x65,
	NOUN_FOUR = 0x66,
	NOUN_FOXGLOVE = 0x67,
	NOUN_FROG = 0x68,
	NOUN_GEARS = 0x69,
	NOUN_GROUND = 0x6a,
	NOUN_HOLE = 0x6b,
	NOUN_IVY_LEAF = 0x6d,
	NOUN_JOURNAL = 0x6e,
	NOUN_LABORATORY = 0x6f,
	NOUN_LEAVES = 0x70,
	NOUN_LILY_PAD = 0x71,
	NOUN_MAP = 0x74,
	NOUN_MARSH = 0x75,
	NOUN_MINT = 0x76,
	NOUN_MOSS = 0x77,
	NOUN_MUSHROOM = 0x78,
	NOUN_NEEDLE = 0x79,
	NOUN_NOTHING = 0x7a,
	NOUN_ONE = 0x7b,
	NOUN_OVERHANGING_GRASS = 0x7c,
	NOUN_PEBBLES = 0x7d,
	NOUN_PICK_UP = 0x7e,
	NOUN_PLANT = 0x7f,
	NOUN_POND = 0x81,
	NOUN_POSTER = 0x82,
	NOUN_PRIMROSE = 0x83,
	NOUN_REEDS = 0x84,
	NOUN_RIVER = 0x85,
	NOUN_ROCKS = 0x86,
	NOUN_ROPE = 0x87,
	NOUN_RUBBER_BAND = 0x89,
	NOUN_SANCTUARY_WOODS = 0x8a,
	NOUN_SHIELDSTONE = 0x8b,
	NOUN_SIGNET_RING = 0x8c,
	NOUN_SIX = 0x8d,
	NOUN_SNAPDRAGON = 0x8e,
	NOUN_SPIDER_SILK = 0x8f,
	NOUN_STAIRS = 0x90,
	NOUN_STICKS = 0x91,
	NOUN_SUNFLOWER = 0x92,
	NOUN_SWORD = 0x93,
	NOUN_TABLE = 0x94,
	NOUN_TELESCOPE = 0x95,
	NOUN_THISTLE = 0x96,
	NOUN_THREE = 0x97,
	NOUN_TWINE = 0x9a,
	NOUN_TWO = 0x9b,
	NOUN_VINE_WEED = 0x9e,
	NOUN_WEASEL = 0xa2,
	NOUN_WEB = 0xa3,
	NOUN_WOOD = 0xa4,
	NOUN_WOODS = 0xa5,
	NOUN_WRENCH = 0xa6,
	NOUN_Y_STICK = 0xa7,
	NOUN_ABIGAIL = 0xa8,
	NOUN_EDGAR = 0xa9,
	NOUN_RUSSEL = 0xaa,
	NOUN_SNAPDRAGON2 = 0xab,
	NOUN_LUNGWORT = 0xac,
	NOUN_GRASS = 0xad,
	NOUN_FWT = 0xae,
	NOUN_DRAGON1 = 0xaf,
	NOUN_DRAGON2 = 0xb0,
	NOUN_NEST = 0xb1,
	NOUN_PAINT_CAN = 0xb2,
	NOUN_DAM = 0xb3,
	NOUN_BROWN = 0xb4,
	NOUN_TURTLE = 0xb5,
	NOUN_DRAGONFLY = 0xb6,
	NOUN_FENWICK = 0xb7,
	NOUN_BLUE_BIRD = 0xb8,
	NOUN_BLACK_BIRD = 0xb9,
	NOUN_LEVER = 0xba,
	NOUN_ROCK = 0xbb,
	NOUN_USED_IT = 0xbc,
	NOUN_TAIL = 0xbd
};

class SceneFactory {
public:
	static SceneLogic *createScene(MADSEngine *vm);
};

/**
 * Specialized base class for Forest game scenes
 */
class ForestScene : public SceneLogic {
protected:
	ForestGlobals &_globals;
	GameForest &_game;
	MADSAction &_action;

	/**
	 * Forms an animation resource name
	 */
	Common::Path formAnimName(char sepChar, int suffixNum);

	/**
	 * Plays appropriate sound for entering varous rooms
	 */
	void lowRoomsEntrySound();
public:
	/**
	 * Constructor
	 */
	ForestScene(MADSEngine *vm);
};

class SceneInfoForest : public SceneInfo {
	friend class SceneInfo;
protected:
	void loadCodes(BaseSurface &depthSurface, int variant) override;

	void loadCodes(BaseSurface &depthSurface, Common::SeekableReadStream *stream) override;

	/**
	* Constructor
	*/
	SceneInfoForest(MADSEngine *vm) : SceneInfo(vm) {}
};

// TODO: Temporary, remove once implemented properly
class DummyScene : public ForestScene {
public:
	DummyScene(MADSEngine *vm) : ForestScene(vm) {
		warning("Unimplemented scene");
	}

	void setup() override {}
	void enter() override {}
	void actions() override {}
};

} // End of namespace Forest

} // End of namespace MADS

#endif /* MADS_FOREST_SCENES_H */

#endif
