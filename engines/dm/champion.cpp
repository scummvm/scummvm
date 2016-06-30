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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "champion.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "inventory.h"
#include "objectman.h"
#include "text.h"


namespace DM {

Box gBoxMouth = Box(55, 72, 12, 29); // @ G0048_s_Graphic562_Box_Mouth 
Box gBoxEye = Box(11, 28, 12, 29); // @ G0049_s_Graphic562_Box_Eye 
Box gBoxChampionIcons[4] = {
	Box(281, 299,  0, 13),
	Box(301, 319,  0, 13),
	Box(301, 319, 15, 28),
	Box(281, 299, 15, 28)};
Color gChampionColor[4] = {(Color)7, (Color)11, (Color)8, (Color)14};

int16 gLightPowerToLightAmount[16] = {0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100};

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

	_partyDead = false;
	_partyIsSleeping = false;
	_leaderHandObjectIconIndex = kIconIndiceNone;
	_leaderEmptyHanded = true;
	_magicCasterChampionIndex = kChampionNone;
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

void ChampionMan::drawHealthOrStaminaOrManaValue(int16 posY, int16 currVal, int16 maxVal) {
	Common::String tmp = getStringFromInteger(currVal, true, 3).c_str();
	_vm->_textMan->printToViewport(55, posY, kColorLightestGray, tmp.c_str());
	_vm->_textMan->printToViewport(73, posY, kColorLightestGray, "/");
	tmp = getStringFromInteger(maxVal, true, 3);
	_vm->_textMan->printToViewport(79, posY, kColorLightestGray, tmp.c_str());
}

uint16 ChampionMan::handSlotIndex(uint16 slotBoxIndex) {
	return slotBoxIndex & 0x1;
}

Common::String ChampionMan::getStringFromInteger(uint16 val, bool padding, uint16 paddingCharCount) {
	using namespace Common;
	String valToStr = String::format("%d", val);
	String result;
	for (int16 i = 0, end = paddingCharCount - valToStr.size(); i < end; ++i)
		result += ' ';

	return result += valToStr;
}

void ChampionMan::applyModifiersToStatistics(Champion* champ, ChampionSlot slotIndex, IconIndice iconIndex, int16 modifierFactor, Thing thing) {
	int16 statIndex;
	int16 modifier = 0;
	ThingType thingType = thing.getType();
	if (((thingType == kWeaponThingType) || (thingType == kArmourThingType))
		&& (slotIndex >= kChampionSlotReadyHand)
		&& (slotIndex <= kChampionSlotQuiverLine_1_1)) {
		Weapon *weapon = (Weapon*)_vm->_dungeonMan->getThingData(thing);
		Armour *armour = (Armour*)_vm->_dungeonMan->getThingData(thing);
		if (((thingType == kWeaponThingType) && weapon->getCursed())
			|| ((thingType == kArmourThingType) && armour->getCursed())) {
			statIndex = kChampionStatLuck;
			modifier = -3;
			goto T0299044_ApplyModifier;
		}
	}

	statIndex = (ChampionStatisticType)thingType; // variable sharing

	if ((iconIndex == kIconIndiceJunkRabbitsFoot) && (slotIndex < kChampionSlotChest_1)) {
		statIndex = kChampionStatLuck;
		modifier = 10;
	} else if (slotIndex == kChampionSlotActionHand) {

		if (iconIndex == kIconIndiceWeaponMaceOfOrder) {
			statIndex = kChampionStatStrength;
			modifier = 5;
		} else {

			statIndex = kChampionStatMana;
			if ((iconIndex >= kIconIndiceWeaponStaffOfClawsEmpty) && (iconIndex <= kIconIndiceWeaponStaffOfClawsFull)) {
				modifier = 4;
			} else if ((iconIndex >= kIconIndiceWeaponStaff) && (iconIndex <= kIconIndiceWeaponSceptreOfLyf)) {
				switch (iconIndex) {
				case kIconIndiceWeaponStaff:
					modifier = 2;
					break;
				case kIconIndiceWeaponWand:
					modifier = 1;
					break;
				case kIconIndiceWeaponTeowand:
					modifier = 6;
					break;
				case kIconIndiceWeaponYewStaff:
					modifier = 4;
					break;
				case kIconIndiceWeaponStaffOfManarStaffOfIrra:
					modifier = 10;
					break;
				case kIconIndiceWeaponSnakeStaffCrossOfNeta:
					modifier = 8;
					break;
				case kIconIndiceWeaponTheConduitSerpentStaff:
					modifier = 16;
					break;
				case kIconIndiceWeaponDragonSpit:
					modifier = 7;
					break;
				case kIconIndiceWeaponSceptreOfLyf:
					modifier = 5;
					break;
				}
			} else {
				switch (iconIndex) {
				case kIconIndiceWeaponDeltaSideSplitter:
					modifier = 1;
					break;
				case kIconIndiceWeaponTheInquisitorDragonFang:
					modifier = 2;
					break;
				case kIconIndiceWeaponVorpalBlade:
					modifier = 4;
					break;
				}
			} // end of else

		}

	} else if (slotIndex == kChampionSlotLegs) {

		if (iconIndex == kIconIndiceArmourPowertowers) {
			statIndex = kChampionStatStrength;
			modifier = 10;
		}

	} else if (slotIndex == kChampionSlotHead) {

		if (iconIndex == kIconIndiceArmourCrownOfNerra) {
			statIndex = kChampionStatWisdom;
			modifier = 10;
		} else if (iconIndex == kIconIndiceArmourDexhelm) {
			statIndex = kChampionStatDexterity;
			modifier = 10;
		}

	} else if (slotIndex == kChampionSlotTorso) {

		if (iconIndex == kIconIndiceArmourFlamebain) {
			statIndex = kChampionStatAntifire;
			modifier = 12;
		} else if (iconIndex == kIconIndiceArmourCloakOfNight) {
			statIndex = kChampionStatDexterity;
			modifier = 8;
		}

	} else if (slotIndex == kChampionSlotNeck) {

		if ((iconIndex >= kIconIndiceJunkJewelSymalUnequipped) && (iconIndex <= kIconIndiceJunkJewelSymalEquipped)) {
			statIndex = kChampionStatAntimagic;
			modifier = 15;
		} else if (iconIndex == kIconIndiceArmourCloakOfNight) {
			statIndex = kChampionStatDexterity;
			modifier = 8;
		} else if (iconIndex == kIconIndiceJunkMoonstone) {
			statIndex = kChampionStatMana;
			modifier = 3;
		}

	}

T0299044_ApplyModifier:
	if (modifier) {
		modifier *= modifierFactor;
		if (statIndex == kChampionStatMana) {
			champ->_maxMana += modifier;
		} else if (statIndex < kChampionStatAntifire + 1) {
			for (uint16 statValIndex = kChampionStatMaximum; statValIndex <= kChampionStatMinimum; ++statValIndex) {
				champ->getStatistic((ChampionStatisticType)statIndex, (ChampionStatisticValue)statValIndex) += modifier;
				warning("BUG0_38");
			}
		}
	}

}

bool ChampionMan::hasObjectIconInSlotBoxChanged(int16 slotBoxIndex, Thing thing) {
	ObjectMan &objMan = *_vm->_objectMan;

	IconIndice currIconIndex = objMan.getIconIndexInSlotBox(slotBoxIndex);
	if (((currIconIndex < kIconIndiceWeaponDagger) && (currIconIndex >= kIconIndiceJunkCompassNorth))
		|| ((currIconIndex >= kIconIndicePotionMaPotionMonPotion) && (currIconIndex <= kIconIndicePotionWaterFlask))
		|| (currIconIndex == kIconIndicePotionEmptyFlask)) {
		IconIndice newIconIndex = objMan.getIconIndex(thing);
		if (newIconIndex != currIconIndex) {
			if ((slotBoxIndex < kSlotBoxInventoryFirstSlot) && !_mousePointerHiddenToDrawChangedObjIconOnScreen) {
				_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
				warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
			}
			objMan.drawIconInSlotBox(slotBoxIndex, newIconIndex);
			return true;
		}
	}

	return false;
}

void ChampionMan::drawChangedObjectIcons() {
	InventoryMan &invMan = *_vm->_inventoryMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;

	uint16 invChampOrdinal = invMan._inventoryChampionOrdinal;
	if (_candidateChampionOrdinal && !invChampOrdinal)
		return;

	_mousePointerHiddenToDrawChangedObjIconOnScreen = false;
	IconIndice leaderHandObjIconIndex = _leaderHandObjectIconIndex;

	if (((leaderHandObjIconIndex < kIconIndiceWeaponDagger) && (leaderHandObjIconIndex >= kIconIndiceJunkCompassNorth))	// < instead of <= is correct
		|| ((leaderHandObjIconIndex >= kIconIndicePotionMaPotionMonPotion) && (leaderHandObjIconIndex <= kIconIndicePotionWaterFlask))
		|| (leaderHandObjIconIndex == kIconIndicePotionEmptyFlask)) {
		IconIndice iconIndex = objMan.getIconIndex(_leaderHandObject);
		if (iconIndex != leaderHandObjIconIndex) {
			_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
			warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
			objMan.extractIconFromBitmap(iconIndex, objMan._objectIconForMousePointer);
			warning("MISSING CODE: F0068_MOUSE_SetPointerToObject");
			_leaderHandObjectIconIndex = iconIndex;
			objMan.drawLeaderObjectName(_leaderHandObject);
		}
	}

	for (uint16 slotBoxIndex = 0; slotBoxIndex < (_partyChampionCount * 2); ++slotBoxIndex) {
		int16 champIndex = slotBoxIndex >> 1;
		if (invChampOrdinal == _vm->indexToOrdinal(champIndex))
			continue;

		if (hasObjectIconInSlotBoxChanged(slotBoxIndex, _champions[champIndex].getSlot((ChampionSlot)handSlotIndex(slotBoxIndex)))
			&& (handSlotIndex(slotBoxIndex) == kChampionSlotActionHand)) {

			menuMan.drawActionIcon((ChampionIndex)champIndex);
		}
	}

	if (invChampOrdinal) {
		Champion *champ = &_champions[_vm->ordinalToIndex(invChampOrdinal)];
		Thing *thing = &champ->getSlot(kChampionSlotReadyHand);
		uint16 drawViewport = 0;

		for (uint16 slotIndex = kChampionSlotReadyHand; slotIndex < kChampionSlotChest_1; slotIndex++, thing++) {
			uint16 objIconChanged = hasObjectIconInSlotBoxChanged(slotIndex + kSlotBoxInventoryFirstSlot, *thing) ? 1 : 0;
			drawViewport |= objIconChanged;
			if (objIconChanged && (slotIndex == kChampionSlotActionHand)) {
				menuMan.drawActionIcon((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
			}
		}

		if (invMan._panelContent = kPanelContentChest) {
			thing = invMan._chestSlots;
			for (int16 slotIndex = 0; slotIndex < 8; ++slotIndex, thing++) {
				drawViewport |= (hasObjectIconInSlotBoxChanged(slotIndex + kSlotBoxChestFirstSlot, *thing) ? 1 : 0);
			}
		}

		if (drawViewport) {
			champ->setAttributeFlag(kChampionAttributeViewport, true);
			drawChampionState((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
		}
	}

	if (_mousePointerHiddenToDrawChangedObjIconOnScreen)
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

void ChampionMan::addObjectInSlot(ChampionIndex champIndex, Thing thing, ChampionSlot slotIndex) {
	InventoryMan &invMan = *_vm->_inventoryMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;

	if (thing == Thing::_none)
		return;

	Champion *champ = &_champions[champIndex];

	if (slotIndex >= kChampionSlotChest_1) {
		invMan._chestSlots[slotIndex - kChampionSlotChest_1] = thing;
	} else {
		champ->setSlot(slotIndex, thing);
	}

	champ->_load += dunMan.getObjectWeight(thing);
	champ->setAttributeFlag(kChampionAttributeLoad, true);
	IconIndice iconIndex = objMan.getIconIndex(thing);
	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == invMan._inventoryChampionOrdinal);
	applyModifiersToStatistics(champ, slotIndex, iconIndex, 1, thing);
	uint16 *rawObjPtr = dunMan.getThingData(thing);

	if (slotIndex < kChampionSlotHead) {

		if (slotIndex == kChampionSlotActionHand) {
			champ->setAttributeFlag(kChampionAttributeActionHand, true);
			if (_actingChampionOrdinal == _vm->indexToOrdinal(champIndex))
				menuMan.clearActingChampion();

			if ((iconIndex >= kIconIndiceScrollOpen) && (iconIndex <= kIconIndiceScrollClosed)) {
				((Scroll*)rawObjPtr)->setClosed(false);
				drawChangedObjectIcons();
			}
		}

		if (iconIndex = kIconIndiceWeaponTorchUnlit) {
			((Weapon*)rawObjPtr)->setLit(true);
			warning("MISSING CODE: F0337_INVENTORY_SetDungeonViewPalette");
			drawChangedObjectIcons();
		} else if (isInventoryChampion && (slotIndex == kChampionSlotActionHand) &&
			((iconIndex == kIconIndiceContainerChestClosed) || ((iconIndex >= kIconIndiceScrollOpen) && (iconIndex <= kIconIndiceScrollClosed)))) {
			champ->setAttributeFlag(kChampionAttributePanel, true);
		}

	} else if (slotIndex == kChampionSlotNeck) {

		if ((iconIndex >= kIconIndiceJunkIllumuletUnequipped) && (iconIndex <= kIconIndiceJunkIllumuletEquipped)) {
			((Junk*)rawObjPtr)->setChargeCount(1);
			_party._magicalLightAmount += gLightPowerToLightAmount[2];
			warning("MISSING CODE: F0337_INVENTORY_SetDungeonViewPalette");
			iconIndex = (IconIndice) (iconIndex + 1);
		} else if ((iconIndex >= kIconIndiceJunkJewelSymalUnequipped) && (iconIndex <= kIconIndiceJunkJewelSymalEquipped)) {
			((Junk*)rawObjPtr)->setChargeCount(1);
			iconIndex = (IconIndice) (iconIndex + 1);
		}

	}

	drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		champ->setAttributeFlag(kChampionAttributeViewport, true);
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

	_leaderHandObject = Thing::_none;
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
		champ->setSlot((ChampionSlot)AL_0_slotIndex_Red, Thing::_none);
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
	champ->_currMana = champ->_maxMana = getDecodedValue(character_Green, 4);
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
		_vm->_menuMan->_refreshActionArea = true;
	} else {
		_vm->_menuMan->clearActingChampion();
		_vm->_menuMan->drawActionIcon((ChampionIndex)(_partyChampionCount - 1));
	}

	int16 mapX = _vm->_dungeonMan->_currMap._partyPosX;
	int16 mapY = _vm->_dungeonMan->_currMap._partyPosY;

	uint16 championObjectsCell = returnOppositeDir((direction)(dunMan._currMap._partyDir));
	mapX += _vm->_dirIntoStepCountEast[dunMan._currMap._partyDir];
	mapY += _vm->_dirIntoStepCountNorth[dunMan._currMap._partyDir];
	thing = dunMan.getSquareFirstThing(mapX, mapY);
	AL_0_slotIndex_Red = kChampionSlotBackpackLine_1_1;
	uint16 slotIndex_Green;
	while (thing != Thing::_endOfList) {
		ThingType AL_2_thingType = thing.getType();
		if ((AL_2_thingType > kSensorThingType) && (thing.getCell() == championObjectsCell)) {
			int16 objectAllowedSlots = gObjectInfo[dunMan.getObjectInfoIndex(thing)].getAllowedSlots();
			switch (AL_2_thingType) {
			case kArmourThingType:
				for (slotIndex_Green = kChampionSlotHead; slotIndex_Green <= kChampionSlotFeet; slotIndex_Green++) {
					if (objectAllowedSlots & gSlotMasks[slotIndex_Green])
						goto T0280048;
				}
				if ((objectAllowedSlots & gSlotMasks[kChampionSlotNeck]) && (champ->getSlot(kChampionSlotNeck) == Thing::_none)) {
					slotIndex_Green = kChampionSlotNeck;
				} else {
					goto T0280046;
				}
				break;
			case kWeaponThingType:
				if (champ->getSlot(kChampionSlotActionHand) == Thing::_none) {
					slotIndex_Green = kChampionSlotActionHand;
				} else {
					goto T0280046;
				}
				break;
			case kScrollThingType:
			case kPotionThingType:
				if (champ->getSlot(kChampionSlotPouch_1) == Thing::_none) {
					slotIndex_Green = kChampionSlotPouch_1;
				} else if (champ->getSlot(kChampionSlotPouch_2) == Thing::_none) {
					slotIndex_Green = kChampionSlotPouch_2;
				} else {
					goto T0280046;
				}
				break;
			case kContainerThingType:
			case kJunkThingType:
T0280046:
				if ((objectAllowedSlots & gSlotMasks[kChampionSlotNeck]) && (champ->getSlot(kChampionSlotNeck) == Thing::_none)) {
					slotIndex_Green = kChampionSlotNeck;
				} else {
					slotIndex_Green = AL_0_slotIndex_Red++;
				}
				break;

			default:
				break;
			}
T0280048:
			if (champ->getSlot((ChampionSlot)slotIndex_Green) != Thing::_none) {
				goto T0280046;
			}
			addObjectInSlot((ChampionIndex)prevChampCount, thing, (ChampionSlot)slotIndex_Green);
		}
		thing = dunMan.getNextThing(thing);
	}

	_vm->_inventoryMan->toggleInventory((ChampionIndex)prevChampCount);
	_vm->_menuMan->drawDisabledMenu();
}

void ChampionMan::drawChampionBarGraphs(ChampionIndex champIndex) {

	Champion *curChampion = &_champions[champIndex];
	int16 barGraphIndex = 0;
	int16 barGraphHeightArray[3];

	if (curChampion->_currHealth > 0) {
		uint32 barGraphHeight = (((uint32)(curChampion->_currHealth) << 10) * 25) / curChampion->_maxHealth;
		if (barGraphHeight & 0x3FF) {
			barGraphHeightArray[barGraphIndex++] = (barGraphHeight >> 10) + 1;
		} else {
			barGraphHeightArray[barGraphIndex++] = (barGraphHeight >> 10);
		}
	} else {
		barGraphHeightArray[barGraphIndex++] = 0;
	}

	if (curChampion->_currStamina > 0) {
		uint32 barGraphHeight = (((uint32)(curChampion->_currStamina) << 10) * 25) / curChampion->_maxStamina;
		if (barGraphHeight & 0x3FF) {
			barGraphHeightArray[barGraphIndex++] = (barGraphHeight >> 10) + 1;
		} else {
			barGraphHeightArray[barGraphIndex++] = (barGraphHeight >> 10);
		}
	} else {
		barGraphHeightArray[barGraphIndex++] = 0;
	}

	if (curChampion->_currMana > 0) {
		uint32 barGraphHeight = (((uint32)(curChampion->_currMana) << 10) * 25) / curChampion->_maxMana;
		if (barGraphHeight & 0x3FF) {
			barGraphHeightArray[barGraphIndex++] = (barGraphHeight >> 10) + 1;
		} else {
			barGraphHeightArray[barGraphIndex++] = (barGraphHeight >> 10);
		}
	} else {
		barGraphHeightArray[barGraphIndex++] = 0;
	}
	warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");

	Box box;
	box._x1 = champIndex * kChampionStatusBoxSpacing + 46;
	box._x2 = box._x1 + 3 + 1;
	box._y1 = 2;
	box._y2 = 26 + 1;

	for (int16 AL_0_barGraphIndex = 0; AL_0_barGraphIndex < 3; AL_0_barGraphIndex++) {
		int16 barGraphHeight = barGraphHeightArray[AL_0_barGraphIndex];
		if (barGraphHeight < 25) {
			box._y1 = 2;
			box._y1 = 27 - barGraphHeight + 1;
			_vm->_displayMan->clearScreenBox(gChampionColor[champIndex], box);
		}
		if (barGraphHeight) {
			box._y1 = 27 - barGraphHeight;
			box._y2 = 26 + 1;
			_vm->_displayMan->clearScreenBox(gChampionColor[champIndex], box);
		}
		box._x1 += 7;
		box._x2 += 7;
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}


uint16 ChampionMan::getStaminaAdjustedValue(Champion *champ, int16 val) {
	int16 currStamina = champ->_currStamina;
	int16 halfMaxStamina = champ->_maxStamina / 2;
	if (currStamina < halfMaxStamina) {
		warning("Possible undefined behavior in the original code");
		val /= 2;
		return val + ((uint32)val * (uint32)currStamina) / halfMaxStamina;
	}
	return val;
}

uint16 ChampionMan::getMaximumLoad(Champion *champ) {
	uint16 maximumLoad = champ->getStatistic(kChampionStatStrength, kChampionStatCurrent) * 8 + 100;
	maximumLoad = getStaminaAdjustedValue(champ, maximumLoad);
	int16 wounds = champ->getWounds();
	if (wounds) {
		maximumLoad -= maximumLoad >> (champ->getWoundsFlag(kChampionWoundLegs) ? 2 : 3);
	}
	if (_vm->_objectMan->getIconIndex(champ->getSlot(kChampionSlotFeet)) == kIconIndiceArmourElvenBoots) {
		maximumLoad += maximumLoad * 16;
	}
	maximumLoad += 9;
	maximumLoad -= maximumLoad % 10;
	return maximumLoad;
}

void ChampionMan::drawChampionState(ChampionIndex champIndex) {
	InventoryMan &invMan = *_vm->_inventoryMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	MenuMan &menuMan = *_vm->_menuMan;
	EventManager &eventMan = *_vm->_eventMan;

	Box box;
	int16 champStatusBoxX = champIndex * kChampionStatusBoxSpacing;
	Champion *champ = &_champions[champIndex];
	uint16 champAttributes = champ->getAttributes();
	if (!((champAttributes) & (kChampionAttributeNameTitle | kChampionAttributeStatistics | kChampionAttributeLoad | kChampionAttributeIcon |
							   kChampionAttributePanel | kChampionAttributeStatusBox | kChampionAttributeWounds | kChampionAttributeViewport |
							   kChampionAttributeActionHand))) {
		return;
	}
	bool isInventoryChamp = (_vm->indexToOrdinal(champIndex) == invMan._inventoryChampionOrdinal);
	dispMan._useByteBoxCoordinates = false;
	if (champAttributes & kChampionAttributeStatusBox) {
		box._y1 = 0;
		box._y2 = 28 + 1;
		box._x1 = champStatusBoxX;
		box._x2 = box._x1 + 66 + 1;
		if (champ->_currHealth) {
			dispMan.clearScreenBox(kColorDarkestGray, box);
			int16 nativeBitmapIndices[3];
			for (int16 i = 0; i < 3; ++i)
				nativeBitmapIndices[i] = 0;
			int16 AL_0_borderCount = 0;
			if (_party._fireShieldDefense > 0)
				nativeBitmapIndices[AL_0_borderCount++] = kBorderPartyFireshieldIndice;
			if (_party._spellShieldDefense > 0)
				nativeBitmapIndices[AL_0_borderCount++] = kBorderPartySpellshieldIndice;
			if (_party._shieldDefense > 0)
				nativeBitmapIndices[AL_0_borderCount++] = kBorderPartyShieldIndice;
			while (AL_0_borderCount--) {
				dispMan.blitToScreen(dispMan.getBitmap(nativeBitmapIndices[AL_0_borderCount]), 80, 0, 0, box, kColorFlesh);
			}
			if (isInventoryChamp) {
				invMan.drawStatusBoxPortrait(champIndex);
				champAttributes |= kChampionAttributeStatistics;
			} else {
				champAttributes |= (kChampionAttributeNameTitle | kChampionAttributeStatistics | kChampionAttributeWounds | kChampionAttributeActionHand);
			}
		} else {
			dispMan.blitToScreen(dispMan.getBitmap(kStatusBoxDeadChampion), 80, 0, 0, box, kColorNoTransparency);
			_vm->_textMan->printTextToScreen(champStatusBoxX + 1, 5, kColorLightestGray, kColorDarkGary, champ->_name);
			menuMan.drawActionIcon(champIndex);
			goto T0292042_green;
		}
	}

	if (!champ->_currHealth)
		goto T0292042_green;

	if (champAttributes & kChampionAttributeNameTitle) {
		Color AL_0_colorIndex = (champIndex == _leaderIndex) ? kColorGold : kColorLightestGray; // unused because of missing functions
		if (isInventoryChamp) {
			char *champName = champ->_name;
			_vm->_textMan->printToViewport(3, 7, AL_0_colorIndex, champName);
			int16 champTitleX = 6 * strlen(champName) + 3;
			char champTitleFirstChar = champ->_title[0];
			if ((champTitleFirstChar != ',') && (champTitleFirstChar != ';') && (champTitleFirstChar != '-')) {
				champTitleX += 6;
			}
			_vm->_textMan->printToViewport(champTitleX, 7, AL_0_colorIndex, champ->_title);
			champAttributes |= kChampionAttributeViewport;
		} else {
			box._y1 = 0;
			box._y2 = 6 + 1;
			box._x1 = champStatusBoxX;
			box._x2 = box._x1 + 42 + 1;
			dispMan.clearScreenBox(kColorDarkGary, box);
			_vm->_textMan->printTextToScreen(champStatusBoxX + 1, 5, AL_0_colorIndex, kColorDarkGary, champ->_name);
		}
	}

	if (champAttributes & kChampionAttributeStatistics) {
		drawChampionBarGraphs(champIndex);
		if (isInventoryChamp) {
			drawHealthStaminaManaValues(champ);
			int16 AL_2_nativeBitmapIndex;
			if ((champ->_food < 0) || (champ->_water < 0) || (champ->_poisonEventCount)) {
				AL_2_nativeBitmapIndex = kSlotBoxWoundedIndice;
			} else {
				AL_2_nativeBitmapIndex = kSlotBoxNormalIndice;
			}
			dispMan.blitToScreen(dispMan.getBitmap(AL_2_nativeBitmapIndex), 32, 0, 0, gBoxMouth, kColorDarkestGray, gDungeonViewport);
			AL_2_nativeBitmapIndex = kSlotBoxNormalIndice;
			for (int16 AL_0_statisticIndex = kChampionStatStrength; AL_0_statisticIndex <= kChampionStatAntifire; AL_0_statisticIndex++) {
				if (champ->getStatistic((ChampionStatisticType)AL_0_statisticIndex, kChampionStatCurrent)
					< champ->getStatistic((ChampionStatisticType)AL_0_statisticIndex, kChampionStatMaximum)) {
					AL_2_nativeBitmapIndex = kSlotBoxWoundedIndice;
					break;
				}
			}
			dispMan.blitToScreen(dispMan.getBitmap(AL_2_nativeBitmapIndex), 32, 0, 0, gBoxEye, kColorDarkestGray, gDungeonViewport);
			champAttributes |= kChampionAttributeViewport;
		}
	}

	if (champAttributes & kChampionAttributeWounds) {
		for (int16 AL_0_slotIndex = isInventoryChamp ? kChampionSlotFeet : kChampionSlotActionHand; AL_0_slotIndex >= kChampionSlotReadyHand; AL_0_slotIndex--) {
			drawSlot(champIndex, (ChampionSlot)AL_0_slotIndex);
		}
		if (isInventoryChamp) {
			champAttributes |= kChampionAttributeViewport;
		}
	}

	if ((champAttributes & kChampionAttributeLoad) && isInventoryChamp) {
		Color loadColor;
		int16 champMaxLoad = getMaximumLoad(champ);
		if (champ->_load > champMaxLoad) {
			loadColor = kColorRed;
		} else if (((int32)champ->_load) * 8 > ((int32)champMaxLoad) * 5) {
			loadColor = kColorYellow;
		} else {
			loadColor = kColorLightestGray;
		}
		_vm->_textMan->printToViewport(104, 132, loadColor, "LOAD ");

		int16 loadTmp = champ->_load / 10;
		Common::String str = getStringFromInteger(loadTmp, true, 3);
		str += '.';
		loadTmp = champ->_load - (loadTmp * 10);
		str += getStringFromInteger(loadTmp, false, 1);
		str += '/';
		loadTmp = (getMaximumLoad(champ) + 5) / 10;
		str += "KG";
		_vm->_textMan->printToViewport(148, 132, loadColor, str.c_str());
		champAttributes |= kChampionAttributeViewport;
	}

	{ // block so goto won't skip AL_0_championIconIndex initialization 
		int16 AL_0_championIconIndex = championIconIndex(champ->_cell, _vm->_dungeonMan->_currMap._partyDir);

		if ((champAttributes & kChampionIcons) && (eventMan._useChampionIconOrdinalAsMousePointerBitmap != _vm->indexToOrdinal(AL_0_championIconIndex))) {
			dispMan.clearScreenBox(gChampionColor[champIndex], gBoxChampionIcons[AL_0_championIconIndex]);
			dispMan.blitToScreen(dispMan.getBitmap(kChampionIcons), 80, championIconIndex(champ->_dir, _vm->_dungeonMan->_currMap._partyDir) * 19, 0,
								 gBoxChampionIcons[AL_0_championIconIndex], kColorDarkestGray);
		}
	}

	if ((champAttributes & kChampionAttributePanel) && isInventoryChamp) {
		if (_vm->_pressingMouth) {
			invMan.drawPanelFoodWaterPoisoned();
		} else if (_vm->_pressingEye) {
			if (_leaderEmptyHanded) {
				warning("MISSING CODE: F0351_INVENTORY_DrawChampionSkillsAndStatistics");
			}
		} else {
			invMan.drawPanel();
		}
		champAttributes |= kChampionAttributeViewport;
	}

	if (champAttributes & kChampionAttributeActionHand) {
		drawSlot(champIndex, kChampionSlotActionHand);
		menuMan.drawActionIcon(champIndex);
		if (isInventoryChamp) {
			champAttributes |= kChampionAttributeViewport;
		}
	}

	if (champAttributes & kChampionAttributeViewport) {
		warning("MISSGIN CODE: F0097_DUNGEONVIEW_DrawViewport");
	}


T0292042_green:
	champ->setAttributeFlag((ChampionAttribute)(kChampionAttributeNameTitle | kChampionAttributeStatistics | kChampionAttributeLoad | kChampionAttributeIcon |
												kChampionAttributePanel | kChampionAttributeStatusBox | kChampionAttributeWounds | kChampionAttributeViewport |
												kChampionAttributeActionHand), false);
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

uint16 ChampionMan::championIconIndex(int16 val, direction dir) {
	return ((val + 4 - dir) & 0x3);
}

void ChampionMan::drawHealthStaminaManaValues(Champion* champ) {
	drawHealthOrStaminaOrManaValue(116, champ->_currHealth, champ->_maxHealth);
	drawHealthOrStaminaOrManaValue(124, champ->_currStamina, champ->_maxStamina);
	drawHealthOrStaminaOrManaValue(132, champ->_currMana, champ->_maxMana);
}

void ChampionMan::drawSlot(uint16 champIndex, ChampionSlot slotIndex) {
	int16 nativeBitmapIndex = -1;
	Champion *champ = &_champions[champIndex];
	bool isInventoryChamp = (_vm->_inventoryMan->_inventoryChampionOrdinal == _vm->indexToOrdinal(champIndex));

	uint16 slotBoxIndex;
	if (!isInventoryChamp) {  /* If drawing a slot for a champion other than the champion whose inventory is open */
		if ((slotIndex > kChampionSlotActionHand) || (_candidateChampionOrdinal == _vm->indexToOrdinal(champIndex))) {
			return;
		}
		slotBoxIndex = (champIndex << 1) + slotIndex;
	} else {
		slotBoxIndex = kSlotBoxInventoryFirstSlot + slotIndex;
	}

	Thing thing;
	if (slotIndex >= kChampionSlotChest_1) {
		thing = _vm->_inventoryMan->_chestSlots[slotIndex - kChampionSlotChest_1];
	} else {
		thing = champ->getSlot(slotIndex);
	}

	SlotBox *slotBox = &_vm->_objectMan->_slotBoxes[slotBoxIndex];
	Box box;
	box._x1 = slotBox->_x - 1;
	box._y1 = slotBox->_y - 1;
	box._x2 = box._x1 + 17 + 1;
	box._y2 = box._y1 + 17 + 1;


	if (!isInventoryChamp) {
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	}

	int16 iconIndex;
	if (thing == Thing::_none) {
		if (slotIndex <= kChampionSlotFeet) {
			iconIndex = kIconIndiceReadyHand + (slotIndex << 1);
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex))) {
				iconIndex++;
				nativeBitmapIndex = kSlotBoxWoundedIndice;
			} else {
				nativeBitmapIndex = kSlotBoxNormalIndice;
			}
		} else {
			if ((slotIndex >= kChampionSlotNeck) && (slotIndex <= kChampionSlotBackpackLine_1_1)) {
				iconIndex = kIconIndiceNeck + (slotIndex - kChampionSlotNeck);
			} else {
				iconIndex = kIconIndiceEmptyBox;
			}
		}
	} else {
		warning("BUG0_35");
		iconIndex = _vm->_objectMan->getIconIndex(thing); // BUG0_35
		if (isInventoryChamp && (slotIndex == kChampionSlotActionHand) && ((iconIndex == kIconIndiceContainerChestClosed) || (iconIndex == kIconIndiceScrollOpen))) {
			warning("BUG2_00");
			iconIndex++;
		} // BUG2_00
		if (slotIndex <= kChampionSlotFeet) {
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex))) {
				nativeBitmapIndex = kSlotBoxWoundedIndice;
			} else {
				nativeBitmapIndex = kSlotBoxNormalIndice;
			}
		}
	}

	if ((slotIndex == kChampionSlotActionHand) && (_vm->indexToOrdinal(champIndex) == _actingChampionOrdinal)) {
		nativeBitmapIndex = kSlotBoxActingHandIndice;
	}

	if (nativeBitmapIndex != -1) {
		_vm->_displayMan->_useByteBoxCoordinates = false;
		_vm->_displayMan->blitToScreen(_vm->_displayMan->getBitmap(nativeBitmapIndex), 32, 0, 0,
									   box, kColorDarkestGray, isInventoryChamp ? gDungeonViewport : gDefultViewPort);
	}

	_vm->_objectMan->drawIconInSlotBox(slotBoxIndex, iconIndex);

	if (!isInventoryChamp) {
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	}
}

void ChampionMan::renameChampion(Champion* champ) {
	warning("STUB METHOD: Champion::renameChampion, F0281_CHAMPION_Rename");

	DisplayMan &dispMan = *_vm->_displayMan;
	TextMan &textMan = *_vm->_textMan;

	Box box;
	box._y1 = 3;
	box._y2 = 8 + 1;
	box._x1 = 3;
	box._x2 = box._x1 + 167;

	dispMan.clearScreenBox(kColorDarkestGray, box, gDungeonViewport);
	dispMan.blitToScreen(dispMan.getBitmap(kPanelRenameChampionIndice), 144, 0, 0, gBoxPanel, kColorCyan, gDungeonViewport);
	textMan.printToViewport(177, 58, kColorLightestGray, "_______");
	textMan.printToViewport(105, 76, kColorLightestGray, "___________________");
	Common::Point clickPos;
	static Box okButtonBox(197, 215, 147, 155); // inclusive boundaries, constructor adds +1
	for (;;) {
		_vm->_eventMan->processInput();
		if (_vm->_eventMan->hasPendingClick(clickPos, kLeftMouseButton) && okButtonBox.isPointInside(clickPos)) {
			return;
		}
		dispMan.updateScreen();
	}
}

uint16 ChampionMan::getSkillLevel(ChampionIndex champIndex, ChampionSkill skillIndex) {
	if (_partyIsSleeping)
		return 1;

	bool ignoreTempExp = skillIndex & kIgnoreTemporaryExperience;
	bool ignoreObjModifiers = skillIndex & kIgnoreObjectModifiers;
	skillIndex = (ChampionSkill)(skillIndex & ~(ignoreTempExp | ignoreObjModifiers));
	Champion *champ = &_champions[champIndex];
	Skill *skill = &champ->getSkill(skillIndex);
	int32 experience = skill->_experience;

	if (!ignoreTempExp)
		experience += skill->_temporaryExperience;

	if (skillIndex > kChampionSkillWizard) { // hidden skill
		skill = &champ->getSkill((ChampionSkill)((skillIndex - kChampionSkillSwing) / 4));
		experience += skill->_experience; // add exp to the base skill
		if (!ignoreTempExp)
			experience += skill->_temporaryExperience;

		experience /= 2; // halve the exp to get avarage of base skill + hidden skill exp
	}

	int16 skillLevel = 1;
	while (experience >= 500) {
		experience /= 2;
		skillLevel++;
	}

	if (!ignoreObjModifiers) {
		IconIndice actionHandIconIndex = _vm->_objectMan->getIconIndex(champ->getSlot(kChampionSlotActionHand));
		if (actionHandIconIndex == kIconIndiceWeaponTheFirestaff) {
			skillLevel++;
		} else if (actionHandIconIndex == kIconIndiceWeaponTheFirestaffComplete) {
			skillLevel += 2;
		}

		IconIndice neckIconIndice = _vm->_objectMan->getIconIndex(champ->getSlot(kChampionSlotNeck));
		switch (skillIndex) {
		case kChampionSkillWizard:
			if (neckIconIndice == kIconIndiceJunkPendantFeral)
				skillLevel++;
			break;
		case kChampionSkillDefend:
			if (neckIconIndice == kIconIndiceJunkEkkhardCross)
				skillLevel++;
			break;
		case kChampionSkillHeal:
			// these two are not cummulative
			if ((neckIconIndice == kIconIndiceJunkGemOfAges) || (neckIconIndice == kIconIndiceWeaponSceptreOfLyf))
				skillLevel++;
			break;
		case kChampionSkillInfluence:
			if (neckIconIndice == kIconIndiceJunkMoonstone)
				skillLevel++;
			break;
		}
	}
	return skillLevel;
}

}


