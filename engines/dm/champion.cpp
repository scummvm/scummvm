#include "champion.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "inventory.h"


namespace DM {

Box gBoxMouth = Box(55, 72, 12, 29); // @ G0048_s_Graphic562_Box_Mouth 
Box gBoxEye = Box(11, 28, 12, 29); // @ G0049_s_Graphic562_Box_Eye 
Box gBoxChampionIcons[4] = { // @ G0054_ai_Graphic562_Box_ChampionIcons
	Box(281, 299,  0, 13),
	Box(301, 319,  0, 13),
	Box(301, 319, 15, 28),
	Box(281, 299, 15, 28)};
Color gChampionColor[4] = {(Color)7, (Color)11, (Color)8, (Color)14}; // @ G0046_auc_Graphic562_ChampionColor

uint16 gSlotMasks[38] = {  // @ G0038_ai_Graphic562_SlotMasks
	/* 30 for champion inventory, 8 for chest */
	0xFFFF,   /* Ready Hand       Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Action Hand      Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0x0002,   /* Head             Head */
	0x0008,   /* Torso            Torso */
	0x0010,   /* Legs             Legs */
	0x0020,   /* Feet             Feet */
	0x0100,   /* Pouch 2          Pouch */
	0x0080,   /* Quiver Line2 1   Quiver 2 */
	0x0080,   /* Quiver Line1 2   Quiver 2 */
	0x0080,   /* Quiver Line2 2   Quiver 2 */
	0x0004,   /* Neck             Neck */
	0x0100,   /* Pouch 1          Pouch */
	0x0040,   /* Quiver Line1 1   Quiver 1 */
	0xFFFF,   /* Backpack Line1 1 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 2 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 3 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 4 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 5 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 6 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 7 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 8 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line2 9 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 2 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 3 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 4 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 5 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 6 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 7 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 8 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0xFFFF,   /* Backpack Line1 9 Mouth/Head/Neck/Torso/Legs/Feet/Quiver 1/Quiver 2/Pouch/Hands/Chest */
	0x0400,   /* Chest 1          Chest */
	0x0400,   /* Chest 2          Chest */
	0x0400,   /* Chest 3          Chest */
	0x0400,   /* Chest 4          Chest */
	0x0400,   /* Chest 5          Chest */
	0x0400,   /* Chest 6          Chest */
	0x0400,   /* Chest 7          Chest */
	0x0400}; /* Chest 8          Chest */

Box gBoxChampionPortrait = Box(0, 31, 0, 28); // @ G0047_s_Graphic562_Box_ChampionPortrait 

ChampionMan::ChampionMan(DMEngine *vm) : _vm(vm) {
	_leaderIndex = kChampionNone;
}

uint16 ChampionMan::getChampionPortraitX(uint16 index) {
	return ((index) & 0x7) << 5;
}

uint16 ChampionMan::getChampionPortraitY(uint16 index) {
	return ((index) >> 3) * 29;
}

int16 ChampionMan::getDecodedValue(char *string, uint16 characterCount) {
	int val = 0;
	for (uint16 i = 0; i < characterCount; ++i) {
		val = (val << 4) + (string[i] - 'A');
	}
	return val;
}

ChampionIndex ChampionMan::getIndexInCell(ViewCell cell) {
	for (uint16 i = 0; i < _partyChampionCount; ++i) {
		if ((_champions[i]._cell == cell) && _champions[i]._currHealth)
			return (ChampionIndex)i;
	}

	return kChampionNone;
}

void ChampionMan::resetDataToStartGame() {
	if (!_vm->_dungeonMan->_messages._newGame) {
		warning("MISSING CODE: stuff for resetting for loaded games");
		assert(false);
	}

	_leaderHand = Thing::_thingNone;
	_leaderHandObjectIconIndex = kIconIndiceNone;
	_leaderEmptyHanded = true;
}


void ChampionMan::addCandidateChampionToParty(uint16 championPortraitIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;

	if (!_leaderEmptyHanded || _partyChampionCount == 4)
		return;

	uint16 prevChampCount = _partyChampionCount;
	Champion *champ = &_champions[prevChampCount];
	champ->resetToZero();
	dispMan._useByteBoxCoordinates = true;
	{ // limit destBox scope
		Box &destBox = gBoxChampionPortrait;
		dispMan.blitToBitmap(dispMan.getBitmap(kChampionPortraitsIndice), 256, getChampionPortraitX(championPortraitIndex), getChampionPortraitY(championPortraitIndex),
							 champ->_portrait, 32, destBox._x1, destBox._x2, destBox._y1, destBox._y2, kColorNoTransparency);
	}

	champ->_actionIndex = kChampionActionNone;
	champ->_enableActionEventIndex = -1;
	champ->_hideDamageReceivedIndex = -1;
	champ->_dir = dunMan._currMap._partyDir;
	ViewCell AL_0_viewCell = kViewCellFronLeft;
	while (getIndexInCell((ViewCell)((AL_0_viewCell + dunMan._currMap._partyDir) & 3)) != kChampionNone)
		AL_0_viewCell = (ViewCell)(AL_0_viewCell + 1);
	champ->_cell = (ViewCell)((AL_0_viewCell + dunMan._currMap._partyDir) & 3);
	champ->clearAttributes(kChampionAttributeIcon);
	champ->_directionMaximumDamageReceived = dunMan._currMap._partyDir;
	champ->_food = 1500 + _vm->_rnd->getRandomNumber(256);
	champ->_water = 1500 + _vm->_rnd->getRandomNumber(256);
	int16 AL_0_slotIndex_Red;
	for (AL_0_slotIndex_Red = kChampionSlotReadyHand; AL_0_slotIndex_Red < kChampionSlotChest_1; ++AL_0_slotIndex_Red) {
		champ->setSlot((ChampionSlot)AL_0_slotIndex_Red, Thing::_thingNone);
	}
	Thing thing = dunMan.getSquareFirstThing(dunMan._currMap._partyPosX, dunMan._currMap._partyPosY);
	while (thing.getType() != kTextstringType) {
		thing = dunMan.getNextThing(thing);
	}
	char decodedChampionText[77];
	char* character_Green = decodedChampionText;
	dunMan.decodeText(character_Green, thing, (TextType)(kTextTypeScroll | kDecodeEvenIfInvisible));
	int16 AL_0_characterIndex = 0;
	uint16 AL_2_character;
	while ((AL_2_character = *character_Green++) != '\n') {
		champ->_name[AL_0_characterIndex++] = AL_2_character;
	}
	champ->_name[AL_0_characterIndex] = '\0';
	AL_0_characterIndex = 0;
	bool AL_4_champTitleCopied = false;
	for (;;) { // infinite
		AL_2_character = *character_Green++;
		if (AL_2_character == '\n') {
			if (AL_4_champTitleCopied)
				break;
			AL_4_champTitleCopied = true;
		} else {
			champ->_title[AL_0_characterIndex++] = AL_2_character;
		}
	}
	champ->_title[AL_0_characterIndex] = '\0';
	if (*character_Green++ == 'M') {
		champ->setAttributeFlag(kChampionAttributeMale, true);
	}
	character_Green++;
	champ->_currHealth = champ->_maxHealth = getDecodedValue(character_Green, 4);
	character_Green += 4;
	champ->_currStamina = champ->_maxStamina = getDecodedValue(character_Green, 4);
	character_Green += 4;
	character_Green++;

	int16 AL_0_statisticIndex;
	for (AL_0_statisticIndex = kChampionStatLuck; AL_0_statisticIndex <= kChampionStatAntifire; ++AL_0_statisticIndex) {
		champ->setStatistic((ChampionStatisticType)AL_0_statisticIndex, kChampionStatMinimum, 30);
		uint16 currMaxVal = getDecodedValue(character_Green, 2);
		champ->setStatistic((ChampionStatisticType)AL_0_statisticIndex, kChampionStatCurrent, currMaxVal);
		champ->setStatistic((ChampionStatisticType)AL_0_statisticIndex, kChampionStatMaximum, currMaxVal);
		character_Green += 2;
	}

	champ->setStatistic(kChampionStatLuck, kChampionStatMinimum, 10);
	character_Green++;

	int16 AL_0_skillIndex;
	int16 AL_2_skillValue;
	for (AL_0_skillIndex = kChampionSkillSwing; AL_0_skillIndex <= kChampionSkillWater; AL_0_skillIndex++) {
		if ((AL_2_skillValue = *character_Green++ - 'A') > 0) {
			champ->setSkillExp((ChampionSkill)AL_0_skillIndex, 125L << AL_2_skillValue);
		}
	}

	for (AL_0_skillIndex = kChampionSkillFighter; AL_0_skillIndex <= kChampionSkillWizard; ++AL_0_skillIndex) {
		int32 baseSkillExp = 0;
		int16 hiddenSkillIndex = (AL_0_skillIndex + 1) << 2;
		for (uint16 AL_4_hiddenSkillCounter = 0; AL_4_hiddenSkillCounter < 4; ++AL_4_hiddenSkillCounter) {
			baseSkillExp += champ->getSkill((ChampionSkill)(hiddenSkillIndex + AL_4_hiddenSkillCounter))._experience;
		}
		champ->setSkillExp((ChampionSkill)AL_0_skillIndex, baseSkillExp);
	}

	_candidateChampionOrdinal = prevChampCount + 1;
	if (++_partyChampionCount == 1) {
		_vm->_eventMan->commandSetLeader(kChampionFirst);
		_vm->_menuMan->_shouldRefreshActionArea = true;
	} else {
		_vm->_menuMan->clearActingChampion();
		_vm->_menuMan->drawActionIcon((ChampionIndex)(_partyChampionCount - 1));
	}

	int16 mapX = _vm->_dungeonMan->_currMap._partyPosX;
	int16 mapY = _vm->_dungeonMan->_currMap._partyPosY;

	uint16 championObjectsCell = returnOppositeDir((direction)(dunMan._currMap._partyDir));
	mapX += _dirIntoStepCountEast[dunMan._currMap._partyDir];
	mapY += _dirIntoStepCountNorth[dunMan._currMap._partyDir];
	thing = dunMan.getSquareFirstThing(mapX, mapY);
	AL_0_slotIndex_Red = kChampionSlotBackpackLine_1_1;
	uint16 slotIndex_Green;
	while (thing != Thing::_thingEndOfList) {
		ThingType AL_2_thingType = thing.getType();
		if ((AL_2_thingType > kSensorThingType) && (thing.getCell() == championObjectsCell)) {
			int16 objectAllowedSlots = gObjectInfo[dunMan.getObjectInfoIndex(thing)].getAllowedSlots();
			switch (AL_2_thingType) {
			case kArmourThingType:
				for (slotIndex_Green = kChampionSlotHead; slotIndex_Green <= kChampionSlotFeet; slotIndex_Green++) {
					if (objectAllowedSlots & gSlotMasks[slotIndex_Green])
						goto T0280048;
				}
				if ((objectAllowedSlots & gSlotMasks[kChampionSlotNeck]) && (champ->getSlot(kChampionSlotNeck) == Thing::_thingNone)) {
					slotIndex_Green = kChampionSlotNeck;
				} else {
					goto T0280046;
				}
				break;
			case kWeaponThingType:
				if (champ->getSlot(kChampionSlotActionHand) == Thing::_thingNone) {
					slotIndex_Green = kChampionSlotActionHand;
				} else {
					goto T0280046;
				}
				break;
			case kScrollThingType:
			case kPotionThingType:
				if (champ->getSlot(kChampionSlotPouch_1) == Thing::_thingNone) {
					slotIndex_Green = kChampionSlotPouch_1;
				} else if (champ->getSlot(kChampionSlotPouch_2) == Thing::_thingNone) {
					slotIndex_Green = kChampionSlotPouch_2;
				} else {
					goto T0280046;
				}
				break;
			case kContainerThingType:
			case kJunkThingType:
T0280046:
				if ((objectAllowedSlots & gSlotMasks[kChampionSlotNeck]) && (champ->getSlot(kChampionSlotNeck) == Thing::_thingNone)) {
					slotIndex_Green = kChampionSlotNeck;
				} else {
					slotIndex_Green = AL_0_slotIndex_Red++;
				}
				break;
			}
T0280048:
			if (champ->getSlot((ChampionSlot)slotIndex_Green) != Thing::_thingNone) {
				goto T0280046;
			}
			warning("MISSING CODE: F0301_CHAMPION_AddObjectInSlot");
		}
		thing = dunMan.getNextThing(thing);
	}

	_vm->_inventoryMan->toggleInventory((ChampionIndex)prevChampCount);
	_vm->_menuMan->drawDisabledMenu();
}

void ChampionMan::drawChampionBarGraphs(ChampionIndex champIndex) {

	Champion *AL_6_champion = &_champions[champIndex];
	int16 AL_2_barGraphIndex = 0;
	int16 barGraphHeightArray[3];

	if (AL_6_champion->_currHealth > 0) {
		uint32 AL_4_barGraphHeight = (((uint32)(AL_6_champion->_currHealth) << 10) * 25) / AL_6_champion->_maxHealth;
		if (AL_4_barGraphHeight & 0x3FF) {
			barGraphHeightArray[AL_2_barGraphIndex++] = (AL_4_barGraphHeight >> 10) + 1;
		} else {
			barGraphHeightArray[AL_2_barGraphIndex++] = (AL_4_barGraphHeight >> 10);
		}
	} else {
		barGraphHeightArray[AL_2_barGraphIndex++] = 0;
	}

	if (AL_6_champion->_currStamina > 0) {
		uint32 AL_4_barGraphHeight = (((uint32)(AL_6_champion->_currStamina) << 10) * 25) / AL_6_champion->_maxStamina;
		if (AL_4_barGraphHeight & 0x3FF) {
			barGraphHeightArray[AL_2_barGraphIndex++] = (AL_4_barGraphHeight >> 10) + 1;
		} else {
			barGraphHeightArray[AL_2_barGraphIndex++] = (AL_4_barGraphHeight >> 10);
		}
	} else {
		barGraphHeightArray[AL_2_barGraphIndex++] = 0;
	}

	if (AL_6_champion->_currMana > 0) {
		uint32 AL_4_barGraphHeight = (((uint32)(AL_6_champion->_currMana) << 10) * 25) / AL_6_champion->_maxMana;
		if (AL_4_barGraphHeight & 0x3FF) {
			barGraphHeightArray[AL_2_barGraphIndex++] = (AL_4_barGraphHeight >> 10) + 1;
		} else {
			barGraphHeightArray[AL_2_barGraphIndex++] = (AL_4_barGraphHeight >> 10);
		}
	} else {
		barGraphHeightArray[AL_2_barGraphIndex++] = 0;
	}
	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");

	Box box;
	box._x1 = champIndex * kChampionStatusBoxSpacing + 46;
	box._x2 = box._x1 + 3;
	box._y1 = 2;
	box._y2 = 26;

	for (int16 AL_0_barGraphIndex = 0; AL_0_barGraphIndex < 3; AL_0_barGraphIndex++) {
		int16 AL_2_barGraphHeight = barGraphHeightArray[AL_0_barGraphIndex];
		if (AL_2_barGraphHeight < 25) {
			box._y1 = 2;
			box._y1 = 27 - AL_2_barGraphHeight;
			_vm->_displayMan->clearScreenBox(gChampionColor[champIndex], box);
		}
		if (AL_2_barGraphHeight) {
			box._y1 = 27 - AL_2_barGraphHeight;
			box._y2 = 26;
			_vm->_displayMan->clearScreenBox(gChampionColor[champIndex], box);
		}
		box._x1 += 7;
		box._x2 += 7;
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

}