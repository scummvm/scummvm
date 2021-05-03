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

#ifndef TWINE_SCENE_GAMESTATE_H
#define TWINE_SCENE_GAMESTATE_H

#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "twine/holomap.h"
#include "twine/menu/menu.h"
#include "twine/scene/actor.h"

namespace TwinE {

/** Magicball strength*/
enum MagicballStrengthType {
	kNoBallStrength = 2,
	kYellowBallStrength = 3,
	kGreenBallStrength = 4,
	kRedBallStrength = 6,
	kFireBallStrength = 8
};

class TwinEEngine;

class GameState {
private:
	TwinEEngine *_engine;

	void initGameStateVars();
	void initHeroVars();

	MenuSettings _gameChoicesSettings;

	/**
	 * LBA engine game flags to save quest states
	 *
	 * 0-27: inventory related
	 * 28-158: story related
	 * 159..199: unused
	 * 200-219: video related
	 * 220..255: unused
	 *
	 * 35: If 0, a zommed sequence of opening the ventilation shaft will be played when Twinsen escapes
	 * his house after arresting Zoe. Set to 1 after the sequence (also if Twinsen is killed during the arrest).
	 * 47: Value of 1 indicates that Twinsen has opened the door to the Citadel Island Tavern's basement.
	 * The door will be always open from now on.
	 * 70: Set to 1 if inventory items are taken from Twinsen when he goes to jail (inventory is empty),
	 * set to 0 after he gets back his stuff.
	 * 92: Set to 1 if the green grobo in the Citadel Island Tavern has told Twinsen about taking Zoe to the
	 * port and leaving for another island.
	 * 107: Set to 1 after Twinsen kills yellow groboclone in the Citadel Island Tavern (after the Tavern has
	 * been closed down). Makes the Tavern open again and groboclone not appear any more.
	 */
	// TODO: why not NUM_GAME_FLAGS?
	uint8 _gameStateFlags[256];

public:
	GameState(TwinEEngine *engine);

	inline bool inventoryDisabled() const {
		return hasGameFlag(GAMEFLAG_INVENTORY_DISABLED) != 0;
	}

	inline bool hasOpenedFunfrocksSafe() const {
		return hasGameFlag(30) != 0;
	}

	inline bool hasItem(InventoryItems item) const {
		return hasGameFlag(item) != 0;
	}

	inline void giveItem(InventoryItems item) {
		setGameFlag(item, 1);
	}

	inline void removeItem(InventoryItems item) {
		setGameFlag(item, 0);
	}

	void clearGameFlags() {
		debug(2, "Clear all gameStateFlags");
		Common::fill(&_gameStateFlags[0], &_gameStateFlags[NUM_GAME_FLAGS], 0);
	}

	inline uint8 hasGameFlag(uint8 index) const {
		debug(6, "Query gameStateFlags[%u]=%u", index, _gameStateFlags[index]);
		return _gameStateFlags[index];
	}

	void setGameFlag(uint8 index, uint8 value);

	/**
	 * LBA engine chapter
	 *  0: Inprisoned
	 *  1: Escape from the citadel
	 *  2: Zoe got captured
	 *  3: - looking for a young girl
	 *  4: - looking for a "friend"
	 *  5: The legend of Sendell
	 *  6: The book of Bu
	 *  7: Pirate LeBorne
	 *  8: - "good day"
	 *  9: - "good day"
	 * 10: - ?? nothing
	 * 11: - ?? nothing
	 * 12: - ?? nothing
	 * 13: - looking for plans
	 * 14: - still looking for plans
	 * 15: The final showdown - "good day"
	 */
	int16 gameChapter = 0;

	/** Magic ball type index */
	int16 magicBallIdx = 0;
	/** Magic ball num bounce */
	int16 magicBallNumBounce = 0;
	/** Magic ball auxiliar bounce number */
	int16 magicBallAuxBounce = 0; // magicBallParam
	/** Magic level index */
	int16 magicLevelIdx = 0;

	/** Store the number of inventory keys */
	int16 inventoryNumKeys = 0;
	/** Store the number of inventory kashes */
	int16 inventoryNumKashes = 0;
	/** Store the number of inventory clover leafs boxes */
	int16 inventoryNumLeafsBox = 0;
	/** Store the number of inventory clover leafs */
	int16 inventoryNumLeafs = 0;
	/** Store the number of inventory magic points */
	int16 inventoryMagicPoints = 0;
	/** Store the number of gas */
	int16 inventoryNumGas = 0;

	int16 setKeys(int16 value);
	int16 setGas(int16 value);
	int16 setLeafs(int16 value);
	int16 setKashes(int16 value);
	int16 setMagicPoints(int16 val);
	int16 setMaxMagicPoints();
	int16 setLeafBoxes(int16 val);

	void addGas(int16 value);
	void addKeys(int16 val);
	void addKashes(int16 val);
	void addMagicPoints(int16 val);
	void addLeafs(int16 val);
	void addLeafBoxes(int16 val);

	/** Its using FunFrock Sabre */
	bool usingSabre = false;

	/**
	 * Inventory used flags
	 * 0 means never used, 1 means already used and automatic re-use
	 */
	uint8 inventoryFlags[NUM_INVENTORY_ITEMS];

	uint8 holomapFlags[NUM_LOCATIONS]; // GV14

	char sceneName[30] {};

	TextId gameChoices[10];  // inGameMenuData
	int32 numChoices = 0;   // numOfOptionsInChoice
	TextId choiceAnswer = TextId::kNone; // inGameMenuAnswer

	/** Initialize all engine variables */
	void initEngineVars();

	/** Initialize engine 3D projections */
	void initEngineProjections();

	void processFoundItem(InventoryItems item);

	void giveUp();
	bool loadGame(Common::SeekableReadStream *file);
	bool saveGame(Common::WriteStream *file);

	void processGameChoices(TextId choiceIdx);

	void processGameoverAnimation();
};

} // namespace TwinE

#endif
