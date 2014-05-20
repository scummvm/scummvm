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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_NEBULAR_SCENES_H
#define MADS_NEBULAR_SCENES_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/globals_nebular.h"


namespace MADS {

namespace Nebular {

enum {
	SEX_MALE = 0, SEX_UNKNOWN = 1, SEX_FEMALE = 2
};

enum Verb {
	VERB_ACTIVATE       = 0x00E,
	VERB_CAST           = 0x03D,
	VERB_CLIMB_DOWN     = 0x04E,
	VERB_CLIMB_THROUGH  = 0x04F,
	VERB_CLIMB_UP       = 0x050,
	VERB_DIVE_INTO      = 0x06D,
	VERB_EAT            = 0x075,
	VERB_EXAMINE        = 0x07D,
	VERB_HOSE_DOWN      = 0x0A6,
	VERB_IGNITE         = 0x0B4,
	VERB_INFLATE        = 0x0B5,
	VERB_INSERT         = 0x0B6,
	VERB_INSPECT        = 0x0B7,
	VERB_HURL           = 0x0A9,
	VERB_LOOK_AT        = 0x0D1,
	VERB_LOOK_IN        = 0x0D2,
	VERB_LOOK_THROUGH   = 0x0D3,
	VERB_PEER_THROUGH   = 0x103,
	VERB_PLAY           = 0x112,
	VERB_PRESS          = 0x11A,
	VERB_PRY            = 0x11C,
	VERB_READ           = 0x11F,
	VERB_SHOOT          = 0x13A,
	VERB_SIT_IN         = 0x13F,
	VERB_SMELL          = 0x147,
	VERB_STARE_AT       = 0x155,
	VERB_SWIM_INTO      = 0x15A,
	VERB_SWIM_THROUGH   = 0x15B,
	VERB_SWIM_TO        = 0x15C,
	VERB_SWIM_TOWARDS   = 0x15D,
	VERB_SWIM_UNDER     = 0x15E,
	VERB_UNLOCK         = 0x17B,
	VERB_WALK_INSIDE    = 0x188,
	VERB_WALK_THROUGH   = 0x18B,
	VERB_WALK_TOWARDS   = 0x18C,
	VERB_WALK_DOWN      = 0x1AD,
	VERB_LEAVE          = 0x1CD,
	VERB_EXIT_FROM      = 0x1CE,
	VERB_USE            = 0x20C,
	VERB_SIT_AT         = 0x21F,
	VERB_WALK_UP        = 0x227,
	VERB_WALK_INTO      = 0x242,
	VERB_EXIT           = 0x298,
	VERB_WALK_ONTO      = 0x2B5,
	VERB_RETURN_TO      = 0x2D5,
	VERB_CLIMB_INTO     = 0x2F7,
	VERB_STEP_INTO      = 0x2F9,
	VERB_CRAWL_TO       = 0x2FB,
	VERB_SIT_ON         = 0x30B,
	VERB_WALK_ALONG     = 0x312,
	VERB_WALK           = 0x32F,
	VERB_REFLECT        = 0x365,
	VERB_GET_INTO       = 0x36A,
	VERB_ENTER          = 0x3B8,
	VERB_INSTALL        = 0x474,
	VERB_REMOVE         = 0x476
};

enum Noun {
	NOUN_BIG_LEAVES     = 0x23,
	NOUN_BINOCULARS     = 0x27,
	NOUN_BLOWGUN        = 0x29,
	NOUN_BOMB           = 0x2A,
	NOUN_BOMBS          = 0x2B,
	NOUN_BONE           = 0x2C,
	NOUN_BONES          = 0x2D,
	NOUN_BURGER         = 0x35,
	NOUN_CHAIR          = 0x47,
	NOUN_CHICKEN        = 0x49,
	NOUN_CHICKEN_BOMB   = 0x4A,
	NOUN_CONTROL_PANEL  = 0x59,
	NOUN_DEAD_FISH		= 0x65,
	NOUN_DOOR			= 0x6E,
	NOUN_DRAWER         = 0x71,
	NOUN_ENTER_KEY      = 0x7A,
	NOUN_FISHING_LINE	= 0x87,
	NOUN_FISHING_ROD    = 0x88,
	NOUN_FRONT_WINDOW	= 0x8E,
	NOUN_FUZZY_DICE		= 0x91,
	NOUN_HOTPANTS		= 0x0A7,
	NOUN_HULL			= 0x0A8,
	NOUN_ID_CARD        = 0x0B3,
	NOUN_JUNGLE			= 0x0B8,
	NOUN_KEYPAD         = 0x0C4,
	NOUN_LADDER         = 0x0C7,
	NOUN_LIFE_SUPPORT_SECTION  = 0x0CC,
	NOUN_LOG			= 0x0D0,
	NOUN_MONKEY			= 0x0E3,
	NOUN_OUTER_HULL		= 0x0F8,
	NOUN_OUTSIDE		= 0x0F9,
	NOUN_PIRANHA        = 0x10D,
	NOUN_PLANT_STALK	= 0x10F,
	NOUN_REFRIGERATOR	= 0x122,
	NOUN_ROBO_KITCHEN	= 0x127,
	NOUN_ROCK           = 0x128,
	NOUN_ROCKS          = 0x129,
	NOUN_SHIELD_ACCESS_PANEL  = 0x135,
	NOUN_SHIELD_MODULATOR	= 0x137,
	NOUN_SKULL          = 0x140,
	NOUN_TELEPORTER     = 0x16C,
	NOUN_BROKEN_LADDER  = 0x1C9,
	NOUN_STUFFED_FISH	= 0x157,
	NOUN_15F            = 0x15F,
	NOUN_TIMEBOMB       = 0x171,
	NOUN_TWINKIFRUIT    = 0x17A,
	NOUN_VIEW_SCREEN    = 0x180,
	NOUN_VIEWPORT       = 0x181,
	NOUN_DEEP_PIT       = 0x19E,
	NOUN_HUGE_LEGS      = 0x1A8,
	NOUN_LEAF_COVERED_PIT  = 0x1A9,
	NOUN_PILE_OF_LEAVES = 0x1AA,
	NOUN_STRANGE_DEVICE = 0x1B6,
	NOUN_CAPTIVE_CREATURE = 0x1C3,
	NOUN_DISPLAY        = 0x1CC,
	NOUN_DEVICE         = 0x1CF,
	NOUN_0_KEY          = 0x1D0,
	NOUN_1_KEY          = 0x1D1,
	NOUN_2_KEY          = 0x1D2,
	NOUN_3_KEY          = 0x1D3,
	NOUN_4_KEY          = 0x1D4,
	NOUN_5_KEY          = 0x1D5,
	NOUN_6_KEY          = 0x1D6,
	NOUN_7_KEY          = 0x1D7,
	NOUN_8_KEY          = 0x1D8,
	NOUN_9_KEY          = 0x1D9,
	NOUN_FROWN_KEY      = 0x1DA,
	NOUN_SMILE_KEY      = 0x1DB,
	NOUN_NATIVE_WOMAN	= 0x1DC,
	NOUN_PLATFORM       = 0x22C,
	NOUN_ALCOHOL        = 0x310,
	NOUN_SUBMERGED_CITY = 0x313,
	NOUN_CEMENT_PYLON   = 0x316,
	NOUN_ELEVATOR       = 0x317,
	NOUN_BUILDING       = 0x323,
	NOUN_LASER_BEAM     = 0x343,
	NOUN_BOAT           = 0x345,
	NOUN_CEMENT_BLOCK   = 0x38E,
	NOUN_CITY           = 0x38F,
	NOUN_DOLLOP			= 0x3AC,
	NOUN_DROP			= 0x3AD,
	NOUN_DASH			= 0x3AE,
	NOUN_SPLASH			= 0x3AF,
	NOUN_HOOK           = 0x467,
	NOUN_BIRDS          = 0x487,
	NOUN_WEST_END_OF_PLATFORM = 0x4A9,
	NOUN_EAST_END_OF_PLATFORM = 0x4AA
};

class SceneFactory {
public:
	static SceneLogic *createScene(MADSEngine *vm);
};

/**
 * Specialized base class for Rex Nebular game scenes
 */
class NebularScene : public SceneLogic {
protected:
	NebularGlobals &_globals;
	GameNebular &_game;
	MADSAction &_action;

	/**
	 * Forms an animation resource name
	 */
	Common::String formAnimName(char sepChar, int suffixNum);

	/**
	 * Plays appropriate sound for entering various rooms
	 */
	void lowRoomsEntrySound();
public:
	/**
	 * Constructor
	 */
	NebularScene(MADSEngine *vm);

	void sub7178C();
};

class SceneInfoNebular : public SceneInfo {
	friend class SceneInfo;
protected:
	virtual void loadCodes(MSurface &depthSurface, int variant);

	virtual void loadCodes(MSurface &depthSurface, Common::SeekableReadStream *stream);

	/**
	* Constructor
	*/
	SceneInfoNebular(MADSEngine *vm) : SceneInfo(vm) {}
};

class SceneTeleporter : public NebularScene {
protected:
	int _buttonTyped;
	int _curCode;
	int _digitCount;
	int _curMessageId;
	int _handSpriteId;
	int _handSequenceId;
	int _finishedCodeCounter;
	int _meteorologistNextPlace;
	int _meteorologistCurPlace;
	int _teleporterSceneId;
	Common::String _msgText;

	int teleporterAddress(int code, bool working);

	void teleporterHandleKey();
	Common::Point teleporterComputeLocation();
	void teleporterEnter();
	bool teleporterActions();
	void teleporterStep();

protected:
	/**
	* Constructor
	*/
	SceneTeleporter(MADSEngine *vm) : NebularScene(vm) {}
};

} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES_H */
