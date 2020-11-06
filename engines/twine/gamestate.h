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

#include "common/savefile.h"
#include "common/scummsys.h"
#include "twine/actor.h"
#include "twine/menu.h"
#include "twine/holomap.h"

namespace TwinE {

#define NUM_GAME_FLAGS 255
#define NUM_INVENTORY_ITEMS 28

/**
 * This gameflag indicates that the inventory items are taken from Twinson because he went to jail
 */
#define GAMEFLAG_INVENTORY_DISABLED 70

enum InventoryItems {
	kiHolomap = 0,
	kiMagicBall = 1,
	kiUseSabre = 2,
	kiGawleysHorn = 3,
	kiTunic = 4,
	kiBookOfBu = 5,
	kSendellsMedallion = 6,
	kFlaskOfClearWater = 7,
	kRedCard = 8,
	kBlueCard = 9,
	kIDCard = 10,
	kMrMiesPass = 11,
	kiProtoPack = 12,
	kSnowboard = 13,
	kiPinguin = 14,
	kGasItem = 15,
	kPirateFlag = 16,
	kMagicFlute = 17,
	kSpaceGuitar = 18,
	kHairDryer = 19,
	kAncesteralKey = 20,
	kBottleOfSyrup = 21,
	kEmptyBottle = 22,
	kFerryTicket = 23,
	kKeypad = 24,
	kCoffeeCan  = 25,
	kiBonusList = 26,
	kiCloverLeaf = 27,
	MaxInventoryItems = 28
};

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

	void initSceneVars();
	void initHeroVars();

public:
	GameState(TwinEEngine *engine);

	/**
	 * LBA engine game flags to save quest states
	 *
	 * 0-27: inventory related
	 * 28-199: story related
	 * 200-255: video related
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
	uint8 gameFlags[256];

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
	bool usingSabre = false;

	/** Inventory used flags */
	uint8 inventoryFlags[NUM_INVENTORY_ITEMS];

	uint8 holomapFlags[NUM_LOCATIONS]; // GV14

	char playerName[30];

	int32 gameChoices[10];         // inGameMenuData
	int32 numChoices = 0;          // numOfOptionsInChoice
	MenuSettings gameChoicesSettings; // choiceTab -  same structure as menu settings
	int32 choiceAnswer = 0;        // inGameMenuAnswer

	/** Initialize all engine variables */
	void initEngineVars();

	/** Initialize engine 3D projections */
	void initEngineProjections();

	void processFoundItem(int32 item);

	bool loadGame(Common::SeekableReadStream *file);
	bool saveGame(Common::WriteStream *file);

	void processGameChoices(int32 choiceIdx);

	void processGameoverAnimation();
};

} // namespace TwinE

#endif
