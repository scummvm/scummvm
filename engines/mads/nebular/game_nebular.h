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

#ifndef MADS_GAME_NEBULAR_H
#define MADS_GAME_NEBULAR_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/globals.h"
#include "mads/nebular/globals_nebular.h"

namespace MADS {

namespace Nebular {

enum StoryMode { STORYMODE_NAUGHTY = 1, STORYMODE_NICE = 2 };

enum Difficulty {
	DIFFICULTY_HARD = 1, DIFFICULTY_MEDIUM = 2, DIFFICULTY_EASY = 3
};


enum ProtectionResult {
	PROTECTION_SUCCEED = 0, PROTECTION_FAIL = 1, PROTECTION_ESCAPE = 2
};

enum InventoryObject {
	OBJ_NONE = -1,
	OBJ_BINOCULARS = 0,
	OBJ_BURGER = 1,
	OBJ_DEAD_FISH = 2,
	OBJ_STUFFED_FISH = 3,
	OBJ_REBREATHER = 4,
	OBJ_TIMER_MODULE = 5,
	OBJ_BIG_LEAVES = 6,
	OBJ_POISON_DARTS = 7,
	OBJ_PLANT_STALK = 8,
	OBJ_BLOWGUN = 9,
	OBJ_TWINKIFRUIT = 10,
	OBJ_BONE = 11,
	OBJ_CHICKEN = 12,
	OBJ_SCALPEL = 13,
	OBJ_AUDIO_TAPE = 14,
	OBJ_CREDIT_CHIP = 15,
	OBJ_SECURITY_CARD = 16,
	OBJ_CHARGE_CASES = 17,
	OBJ_ESTROTOXIN = 18,
	OBJ_BOMB = 19,
	OBJ_TIMEBOMB = 20,
	OBJ_REPAIR_LIST = 21,
	OBJ_ALIEN_LIQUOR = 22,
	OBJ_TARGET_MODULE = 23,
	OBJ_SHIELD_MODULATOR = 24,
	OBJ_TAPE_PLAYER = 25,
	OBJ_PHONE_CELLS = 26,
	OBJ_PENLIGHT = 27,
	OBJ_DURAFAIL_CELLS = 28,
	OBJ_FAKE_ID = 29,
	OBJ_ID_CARD = 30,
	OBJ_POLYCEMENT = 31,
	OBJ_FISHING_ROD = 32,
	OBJ_FISHING_LINE = 33,
	OBJ_PADLOCK_KEY = 34,
	OBJ_DOOR_KEY = 35,
	OBJ_REARVIEW_MIRROR = 36,
	OBJ_COMPACT_CASE = 37,
	OBJ_DETONATORS = 39,
	OBJ_BOTTLE = 40,
	OBJ_CHICKEN_BOMB = 41,
	OBJ_VASE = 42,
	OBJ_REMOTE = 43,
	OBJ_COMPUTER_GAME = 44,
	OBJ_PHONE_HANDSET = 45,
	OBJ_BONES = 46,
	OBJ_GUARDS_ARM = 47,
	OBJ_LOG = 48,
	OBJ_BOMBS = 49,
	OBJ_NOTE = 50,
	OBJ_COMBINATION = 51,
	OBJ_FORMALDEHYDE = 52,
	OBJ_PETROX = 53,
	OBJ_LECITHIN = 54
};

class GameNebular : public Game {
	friend class Game;
private:
	ProtectionResult checkCopyProtection();
protected:
	GameNebular(MADSEngine *vm);

	void startGame() override;

	void initializeGlobals() override;

	void setSectionHandler() override;

	void checkShowDialog() override;
public:
	NebularGlobals _globals;
	StoryMode _storyMode;
	Difficulty _difficulty;

	Globals &globals() override { return _globals; }

	void doObjectAction() override;

	void showRecipe();

	void unhandledAction() override;

	void step() override;

	void synchronize(Common::Serializer &s, bool phase1) override;

	void setNaughtyMode(bool naughtyMode) override { _storyMode = naughtyMode ? STORYMODE_NAUGHTY : STORYMODE_NICE; }
	bool getNaughtyMode() const override { return _storyMode == STORYMODE_NAUGHTY; }
};

// Section handlers aren't needed in ScummVM implementation
class Section1Handler : public SectionHandler {
public:
	Section1Handler(MADSEngine *vm) : SectionHandler(vm) {}

	void preLoadSection() override {}
	void sectionPtr2() override {}
	void postLoadSection() override {}
};

typedef Section1Handler Section2Handler;
typedef Section1Handler Section3Handler;
typedef Section1Handler Section4Handler;
typedef Section1Handler Section5Handler;
typedef Section1Handler Section6Handler;
typedef Section1Handler Section7Handler;
typedef Section1Handler Section8Handler;

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_GAME_NEBULAR_H */
