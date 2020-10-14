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

#ifndef TWINE_GAMESTATE_H
#define TWINE_GAMESTATE_H

#include "common/scummsys.h"
#include "twine/actor.h"

namespace TwinE {

#define NUM_GAME_FLAGS 255
#define NUM_INVENTORY_ITEMS 28

#define GAMEFLAG_INVENTORY_DISABLED 70

enum InventoryItems {
	kiHolomap = 0,
	kiMagicBall = 1,
	kiUseSabre = 2,
	kiTunic = 4,
	kiBookOfBu = 5,
	kiProtoPack = 12,
	kiPinguin = 14,
	kiBonusList = 26,
	kiCloverLeaf = 27
};

/** Magicball strength*/
enum MagicballStrengthType {
	kNoBallStrenght = 2,
	kYellowBallStrenght = 3,
	kGreenBallStrenght = 4,
	kRedBallStrenght = 6,
	kFireBallStrength = 8
};

class TwinEEngine;

class GameState {
private:
	TwinEEngine *_engine;

	void initSceneVars();
	void initHeroVars();

public:
	GameState(TwinEEngine *engine);

	/** LBA engine game flags to save quest states */
	// TODO: why not NUM_GAME_FLAGS?
	uint8 gameFlags[256]{0};

	/** LBA engine chapter */
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

	/** Its using FunFrock Sabre */
	int16 usingSabre = 0;

	/** Inventory used flags */
	uint8 inventoryFlags[NUM_INVENTORY_ITEMS]{0};

	uint8 holomapFlags[150]{0}; // GV14

	char playerName[30] = "";

	int32 gameChoices[10]{0};         // inGameMenuData
	int32 numChoices = 0;             // numOfOptionsInChoice
	int16 gameChoicesSettings[18]{0}; // choiceTab -  same structure as menu settings
	int32 choiceAnswer = 0;           // inGameMenuAnswer

	/** Initialize all engine variables */
	void initEngineVars();

	/** Initialize engine 3D projections */
	void initEngineProjections();

	void processFoundItem(int32 item);

	bool loadGame();
	bool saveGame();

	void processGameChoices(int32 choiceIdx);

	void processGameoverAnimation();
};

} // namespace TwinE

#endif
