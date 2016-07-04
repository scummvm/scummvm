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
Box g54_BoxChampionIcons[4] = {
	Box(281, 299,  0, 13),
	Box(301, 319,  0, 13),
	Box(301, 319, 15, 28),
	Box(281, 299, 15, 28)};
Color g46_ChampionColor[4] = {(Color)7, (Color)11, (Color)8, (Color)14};

int16 g39_LightPowerToLightAmount[16] = {0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100};

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
	_g411_leaderIndex = kM1_ChampionNone;

	_g303_partyDead = false;
	_g300_partyIsSleeping = false;
	_g413_leaderHandObjectIconIndex = kM1_IconIndiceNone;
	_g415_leaderEmptyHanded = true;
	_g514_magicCasterChampionIndex = kM1_ChampionNone;
}

uint16 ChampionMan::M27_getChampionPortraitX(uint16 index) {
	return ((index) & 0x7) << 5;
}

uint16 ChampionMan::M28_getChampionPortraitY(uint16 index) {
	return ((index) >> 3) * 29;
}

int16 ChampionMan::f279_getDecodedValue(char *string, uint16 characterCount) {
	int val = 0;
	for (uint16 i = 0; i < characterCount; ++i) {
		val = (val << 4) + (string[i] - 'A');
	}
	return val;
}

void ChampionMan::f289_drawHealthOrStaminaOrManaValue(int16 posY, int16 currVal, int16 maxVal) {
	Common::String tmp = f288_getStringFromInteger(currVal, true, 3).c_str();
	_vm->_textMan->f52_printToViewport(55, posY, k13_ColorLightestGray, tmp.c_str());
	_vm->_textMan->f52_printToViewport(73, posY, k13_ColorLightestGray, "/");
	tmp = f288_getStringFromInteger(maxVal, true, 3);
	_vm->_textMan->f52_printToViewport(79, posY, k13_ColorLightestGray, tmp.c_str());
}

uint16 ChampionMan::M70_handSlotIndex(uint16 slotBoxIndex) {
	return slotBoxIndex & 0x1;
}

Common::String ChampionMan::f288_getStringFromInteger(uint16 val, bool padding, uint16 paddingCharCount) {
	using namespace Common;
	String valToStr = String::format("%d", val);
	String result;
	for (int16 i = 0, end = paddingCharCount - valToStr.size(); i < end; ++i)
		result += ' ';

	return result += valToStr;
}

void ChampionMan::f299_applyModifiersToStatistics(Champion* champ, ChampionSlot slotIndex, IconIndice iconIndex, int16 modifierFactor, Thing thing) {
	int16 statIndex;
	int16 modifier = 0;
	ThingType thingType = thing.getType();
	if (((thingType == k5_WeaponThingType) || (thingType == k6_ArmourThingType))
		&& (slotIndex >= k0_ChampionSlotReadyHand)
		&& (slotIndex <= k12_ChampionSlotQuiverLine_1_1)) {
		Weapon *weapon = (Weapon*)_vm->_dungeonMan->f156_getThingData(thing);
		Armour *armour = (Armour*)_vm->_dungeonMan->f156_getThingData(thing);
		if (((thingType == k5_WeaponThingType) && weapon->getCursed())
			|| ((thingType == k6_ArmourThingType) && armour->getCursed())) {
			statIndex = k0_ChampionStatLuck;
			modifier = -3;
			goto T0299044_ApplyModifier;
		}
	}

	statIndex = (ChampionStatisticType)thingType; // variable sharing

	if ((iconIndex == k137_IconIndiceJunkRabbitsFoot) && (slotIndex < k30_ChampionSlotChest_1)) {
		statIndex = k0_ChampionStatLuck;
		modifier = 10;
	} else if (slotIndex == k1_ChampionSlotActionHand) {

		if (iconIndex == k45_IconIndiceWeaponMaceOfOrder) {
			statIndex = k1_ChampionStatStrength;
			modifier = 5;
		} else {

			statIndex = k8_ChampionStatMana;
			if ((iconIndex >= k20_IconIndiceWeaponStaffOfClawsEmpty) && (iconIndex <= k22_IconIndiceWeaponStaffOfClawsFull)) {
				modifier = 4;
			} else if ((iconIndex >= k58_IconIndiceWeaponStaff) && (iconIndex <= k66_IconIndiceWeaponSceptreOfLyf)) {
				switch (iconIndex) {
				case k58_IconIndiceWeaponStaff:
					modifier = 2;
					break;
				case k59_IconIndiceWeaponWand:
					modifier = 1;
					break;
				case k60_IconIndiceWeaponTeowand:
					modifier = 6;
					break;
				case k61_IconIndiceWeaponYewStaff:
					modifier = 4;
					break;
				case k62_IconIndiceWeaponStaffOfManarStaffOfIrra:
					modifier = 10;
					break;
				case k63_IconIndiceWeaponSnakeStaffCrossOfNeta:
					modifier = 8;
					break;
				case k64_IconIndiceWeaponTheConduitSerpentStaff:
					modifier = 16;
					break;
				case k65_IconIndiceWeaponDragonSpit:
					modifier = 7;
					break;
				case k66_IconIndiceWeaponSceptreOfLyf:
					modifier = 5;
					break;
				}
			} else {
				switch (iconIndex) {
				case k38_IconIndiceWeaponDeltaSideSplitter:
					modifier = 1;
					break;
				case k41_IconIndiceWeaponTheInquisitorDragonFang:
					modifier = 2;
					break;
				case k40_IconIndiceWeaponVorpalBlade:
					modifier = 4;
					break;
				}
			} // end of else

		}

	} else if (slotIndex == k4_ChampionSlotLegs) {

		if (iconIndex == k142_IconIndiceArmourPowertowers) {
			statIndex = k1_ChampionStatStrength;
			modifier = 10;
		}

	} else if (slotIndex == k2_ChampionSlotHead) {

		if (iconIndex == k104_IconIndiceArmourCrownOfNerra) {
			statIndex = k3_ChampionStatWisdom;
			modifier = 10;
		} else if (iconIndex == k140_IconIndiceArmourDexhelm) {
			statIndex = k2_ChampionStatDexterity;
			modifier = 10;
		}

	} else if (slotIndex == k3_ChampionSlotTorso) {

		if (iconIndex == k141_IconIndiceArmourFlamebain) {
			statIndex = k6_ChampionStatAntifire;
			modifier = 12;
		} else if (iconIndex == k81_IconIndiceArmourCloakOfNight) {
			statIndex = k2_ChampionStatDexterity;
			modifier = 8;
		}

	} else if (slotIndex == k10_ChampionSlotNeck) {

		if ((iconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (iconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
			statIndex = k5_ChampionStatAntimagic;
			modifier = 15;
		} else if (iconIndex == k81_IconIndiceArmourCloakOfNight) {
			statIndex = k2_ChampionStatDexterity;
			modifier = 8;
		} else if (iconIndex == k122_IconIndiceJunkMoonstone) {
			statIndex = k8_ChampionStatMana;
			modifier = 3;
		}

	}

T0299044_ApplyModifier:
	if (modifier) {
		modifier *= modifierFactor;
		if (statIndex == k8_ChampionStatMana) {
			champ->_maxMana += modifier;
		} else if (statIndex < k6_ChampionStatAntifire + 1) {
			for (uint16 statValIndex = k0_ChampionStatMaximum; statValIndex <= k2_ChampionStatMinimum; ++statValIndex) {
				champ->getStatistic((ChampionStatisticType)statIndex, (ChampionStatisticValue)statValIndex) += modifier;
				warning("BUG0_38");
			}
		}
	}

}

bool ChampionMan::f295_hasObjectIconInSlotBoxChanged(int16 slotBoxIndex, Thing thing) {
	ObjectMan &objMan = *_vm->_objectMan;

	IconIndice currIconIndex = objMan.f39_getIconIndexInSlotBox(slotBoxIndex);
	if (((currIconIndex < k32_IconIndiceWeaponDagger) && (currIconIndex >= k0_IconIndiceJunkCompassNorth))
		|| ((currIconIndex >= k148_IconIndicePotionMaPotionMonPotion) && (currIconIndex <= k163_IconIndicePotionWaterFlask))
		|| (currIconIndex == k195_IconIndicePotionEmptyFlask)) {
		IconIndice newIconIndex = objMan.f33_getIconIndex(thing);
		if (newIconIndex != currIconIndex) {
			if ((slotBoxIndex < k8_SlotBoxInventoryFirstSlot) && !_g420_mousePointerHiddenToDrawChangedObjIconOnScreen) {
				_g420_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
				warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
			}
			objMan.f38_drawIconInSlotBox(slotBoxIndex, newIconIndex);
			return true;
		}
	}

	return false;
}

void ChampionMan::f296_drawChangedObjectIcons() {
	InventoryMan &invMan = *_vm->_inventoryMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;

	uint16 invChampOrdinal = invMan._g432_inventoryChampionOrdinal;
	if (_g299_candidateChampionOrdinal && !invChampOrdinal)
		return;

	_g420_mousePointerHiddenToDrawChangedObjIconOnScreen = false;
	IconIndice leaderHandObjIconIndex = _g413_leaderHandObjectIconIndex;

	if (((leaderHandObjIconIndex < k32_IconIndiceWeaponDagger) && (leaderHandObjIconIndex >= k0_IconIndiceJunkCompassNorth))	// < instead of <= is correct
		|| ((leaderHandObjIconIndex >= k148_IconIndicePotionMaPotionMonPotion) && (leaderHandObjIconIndex <= k163_IconIndicePotionWaterFlask))
		|| (leaderHandObjIconIndex == k195_IconIndicePotionEmptyFlask)) {
		IconIndice iconIndex = objMan.f33_getIconIndex(_g414_leaderHandObject);
		if (iconIndex != leaderHandObjIconIndex) {
			_g420_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
			warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
			objMan.f36_extractIconFromBitmap(iconIndex, objMan._g412_objectIconForMousePointer);
			warning("MISSING CODE: F0068_MOUSE_SetPointerToObject");
			_g413_leaderHandObjectIconIndex = iconIndex;
			objMan.f34_drawLeaderObjectName(_g414_leaderHandObject);
		}
	}

	for (uint16 slotBoxIndex = 0; slotBoxIndex < (_g305_partyChampionCount * 2); ++slotBoxIndex) {
		int16 champIndex = slotBoxIndex >> 1;
		if (invChampOrdinal == _vm->M0_indexToOrdinal(champIndex))
			continue;

		if (f295_hasObjectIconInSlotBoxChanged(slotBoxIndex, _gK71_champions[champIndex].getSlot((ChampionSlot)M70_handSlotIndex(slotBoxIndex)))
			&& (M70_handSlotIndex(slotBoxIndex) == k1_ChampionSlotActionHand)) {

			menuMan.f386_drawActionIcon((ChampionIndex)champIndex);
		}
	}

	if (invChampOrdinal) {
		Champion *champ = &_gK71_champions[_vm->M1_ordinalToIndex(invChampOrdinal)];
		Thing *thing = &champ->getSlot(k0_ChampionSlotReadyHand);
		uint16 drawViewport = 0;

		for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++, thing++) {
			uint16 objIconChanged = f295_hasObjectIconInSlotBoxChanged(slotIndex + k8_SlotBoxInventoryFirstSlot, *thing) ? 1 : 0;
			drawViewport |= objIconChanged;
			if (objIconChanged && (slotIndex == k1_ChampionSlotActionHand)) {
				menuMan.f386_drawActionIcon((ChampionIndex)_vm->M1_ordinalToIndex(invChampOrdinal));
			}
		}

		if (invMan._g424_panelContent = k4_PanelContentChest) {
			thing = invMan._g425_chestSlots;
			for (int16 slotIndex = 0; slotIndex < 8; ++slotIndex, thing++) {
				drawViewport |= (f295_hasObjectIconInSlotBoxChanged(slotIndex + k38_SlotBoxChestFirstSlot, *thing) ? 1 : 0);
			}
		}

		if (drawViewport) {
			champ->setAttributeFlag(k0x4000_ChampionAttributeViewport, true);
			f292_drawChampionState((ChampionIndex)_vm->M1_ordinalToIndex(invChampOrdinal));
		}
	}

	if (_g420_mousePointerHiddenToDrawChangedObjIconOnScreen)
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

void ChampionMan::f301_addObjectInSlot(ChampionIndex champIndex, Thing thing, ChampionSlot slotIndex) {
	InventoryMan &invMan = *_vm->_inventoryMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;

	if (thing == Thing::_none)
		return;

	Champion *champ = &_gK71_champions[champIndex];

	if (slotIndex >= k30_ChampionSlotChest_1) {
		invMan._g425_chestSlots[slotIndex - k30_ChampionSlotChest_1] = thing;
	} else {
		champ->setSlot(slotIndex, thing);
	}

	champ->_load += dunMan.f140_getObjectWeight(thing);
	champ->setAttributeFlag(k0x0200_ChampionAttributeLoad, true);
	IconIndice iconIndex = objMan.f33_getIconIndex(thing);
	bool isInventoryChampion = (_vm->M0_indexToOrdinal(champIndex) == invMan._g432_inventoryChampionOrdinal);
	f299_applyModifiersToStatistics(champ, slotIndex, iconIndex, 1, thing);
	uint16 *rawObjPtr = dunMan.f156_getThingData(thing);

	if (slotIndex < k2_ChampionSlotHead) {

		if (slotIndex == k1_ChampionSlotActionHand) {
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			if (_g506_actingChampionOrdinal == _vm->M0_indexToOrdinal(champIndex))
				menuMan.f388_clearActingChampion();

			if ((iconIndex >= k30_IconIndiceScrollOpen) && (iconIndex <= k31_IconIndiceScrollClosed)) {
				((Scroll*)rawObjPtr)->setClosed(false);
				f296_drawChangedObjectIcons();
			}
		}

		if (iconIndex = k4_IconIndiceWeaponTorchUnlit) {
			((Weapon*)rawObjPtr)->setLit(true);
			warning("MISSING CODE: F0337_INVENTORY_SetDungeonViewPalette");
			f296_drawChangedObjectIcons();
		} else if (isInventoryChampion && (slotIndex == k1_ChampionSlotActionHand) &&
			((iconIndex == k144_IconIndiceContainerChestClosed) || ((iconIndex >= k30_IconIndiceScrollOpen) && (iconIndex <= k31_IconIndiceScrollClosed)))) {
			champ->setAttributeFlag(k0x0800_ChampionAttributePanel, true);
		}

	} else if (slotIndex == k10_ChampionSlotNeck) {

		if ((iconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (iconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk*)rawObjPtr)->setChargeCount(1);
			_g407_party._magicalLightAmount += g39_LightPowerToLightAmount[2];
			warning("MISSING CODE: F0337_INVENTORY_SetDungeonViewPalette");
			iconIndex = (IconIndice)(iconIndex + 1);
		} else if ((iconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (iconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
			((Junk*)rawObjPtr)->setChargeCount(1);
			iconIndex = (IconIndice)(iconIndex + 1);
		}

	}

	f291_drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		champ->setAttributeFlag(k0x4000_ChampionAttributeViewport, true);
}

int16 ChampionMan::f315_getScentOrdinal(int16 mapX, int16 mapY) {
	uint16 searchedScentRedEagle;
	int16 scentIndex;
	Scent* scent;
	Scent searchedScent;


	if (scentIndex = _g407_party._scentCount) {
		searchedScent.setMapX(mapX);
		searchedScent.setMapY(mapY);
		searchedScent.setMapIndex(_vm->_dungeonMan->_g272_currMapIndex);
		searchedScentRedEagle = searchedScent.toUint16();
		scent = &_g407_party._scents[scentIndex--];
		do {
			if ((*(--scent)).toUint16() == searchedScentRedEagle) {
				return _vm->M0_indexToOrdinal(scentIndex);
			}
		} while (scentIndex--);
	}
	return 0;
}

ChampionIndex ChampionMan::f285_getIndexInCell(ViewCell cell) {
	for (uint16 i = 0; i < _g305_partyChampionCount; ++i) {
		if ((_gK71_champions[i]._cell == cell) && _gK71_champions[i]._currHealth)
			return (ChampionIndex)i;
	}

	return kM1_ChampionNone;
}

void ChampionMan::f278_resetDataToStartGame() {
	if (!_vm->_g298_newGame) {
		warning("MISSING CODE: stuff for resetting for loaded games");
		assert(false);
	}

	_g414_leaderHandObject = Thing::_none;
	_g413_leaderHandObjectIconIndex = kM1_IconIndiceNone;
	_g415_leaderEmptyHanded = true;
}


void ChampionMan::f280_addCandidateChampionToParty(uint16 championPortraitIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;

	if (!_g415_leaderEmptyHanded || _g305_partyChampionCount == 4)
		return;

	uint16 prevChampCount = _g305_partyChampionCount;
	Champion *champ = &_gK71_champions[prevChampCount];
	champ->resetToZero();
	dispMan._g578_useByteBoxCoordinates = true;
	{ // limit destBox scope
		Box &destBox = gBoxChampionPortrait;
		dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k26_ChampionPortraitsIndice), champ->_portrait,
								  destBox, M27_getChampionPortraitX(championPortraitIndex), M28_getChampionPortraitY(championPortraitIndex), 128, 16, k255_ColorNoTransparency);
	}

	champ->_actionIndex = k255_ChampionActionNone;
	champ->_enableActionEventIndex = -1;
	champ->_hideDamageReceivedIndex = -1;
	champ->_dir = dunMan._g308_partyDir;
	ViewCell AL_0_viewCell = k0_ViewCellFronLeft;
	while (f285_getIndexInCell((ViewCell)((AL_0_viewCell + dunMan._g308_partyDir) & 3)) != kM1_ChampionNone)
		AL_0_viewCell = (ViewCell)(AL_0_viewCell + 1);
	champ->_cell = (ViewCell)((AL_0_viewCell + dunMan._g308_partyDir) & 3);
	champ->clearAttributes(k0x0400_ChampionAttributeIcon);
	champ->_directionMaximumDamageReceived = dunMan._g308_partyDir;
	champ->_food = 1500 + _vm->_rnd->getRandomNumber(256);
	champ->_water = 1500 + _vm->_rnd->getRandomNumber(256);
	int16 AL_0_slotIndex_Red;
	for (AL_0_slotIndex_Red = k0_ChampionSlotReadyHand; AL_0_slotIndex_Red < k30_ChampionSlotChest_1; ++AL_0_slotIndex_Red) {
		champ->setSlot((ChampionSlot)AL_0_slotIndex_Red, Thing::_none);
	}
	Thing thing = dunMan.f161_getSquareFirstThing(dunMan._g306_partyMapX, dunMan._g307_partyMapY);
	while (thing.getType() != k2_TextstringType) {
		thing = dunMan.f159_getNextThing(thing);
	}
	char decodedChampionText[77];
	char* character_Green = decodedChampionText;
	dunMan.f168_decodeText(character_Green, thing, (TextType)(k2_TextTypeScroll | k0x8000_DecodeEvenIfInvisible));
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
		champ->setAttributeFlag(k0x0010_ChampionAttributeMale, true);
	}
	character_Green++;
	champ->_currHealth = champ->_maxHealth = f279_getDecodedValue(character_Green, 4);
	character_Green += 4;
	champ->_currStamina = champ->_maxStamina = f279_getDecodedValue(character_Green, 4);
	character_Green += 4;
	champ->_currMana = champ->_maxMana = f279_getDecodedValue(character_Green, 4);
	character_Green += 4;
	character_Green++;

	int16 AL_0_statisticIndex;
	for (AL_0_statisticIndex = k0_ChampionStatLuck; AL_0_statisticIndex <= k6_ChampionStatAntifire; ++AL_0_statisticIndex) {
		champ->setStatistic((ChampionStatisticType)AL_0_statisticIndex, k2_ChampionStatMinimum, 30);
		uint16 currMaxVal = f279_getDecodedValue(character_Green, 2);
		champ->setStatistic((ChampionStatisticType)AL_0_statisticIndex, k1_ChampionStatCurrent, currMaxVal);
		champ->setStatistic((ChampionStatisticType)AL_0_statisticIndex, k0_ChampionStatMaximum, currMaxVal);
		character_Green += 2;
	}

	champ->setStatistic(k0_ChampionStatLuck, k2_ChampionStatMinimum, 10);
	character_Green++;

	int16 AL_0_skillIndex;
	int16 AL_2_skillValue;
	for (AL_0_skillIndex = k4_ChampionSkillSwing; AL_0_skillIndex <= k19_ChampionSkillWater; AL_0_skillIndex++) {
		if ((AL_2_skillValue = *character_Green++ - 'A') > 0) {
			champ->setSkillExp((ChampionSkill)AL_0_skillIndex, 125L << AL_2_skillValue);
		}
	}

	for (AL_0_skillIndex = k0_ChampionSkillFighter; AL_0_skillIndex <= k3_ChampionSkillWizard; ++AL_0_skillIndex) {
		int32 baseSkillExp = 0;
		int16 hiddenSkillIndex = (AL_0_skillIndex + 1) << 2;
		for (uint16 AL_4_hiddenSkillCounter = 0; AL_4_hiddenSkillCounter < 4; ++AL_4_hiddenSkillCounter) {
			baseSkillExp += champ->getSkill((ChampionSkill)(hiddenSkillIndex + AL_4_hiddenSkillCounter))._experience;
		}
		champ->setSkillExp((ChampionSkill)AL_0_skillIndex, baseSkillExp);
	}

	_g299_candidateChampionOrdinal = prevChampCount + 1;
	if (++_g305_partyChampionCount == 1) {
		_vm->_eventMan->f368_commandSetLeader(k0_ChampionFirst);
		_vm->_menuMan->_g508_refreshActionArea = true;
	} else {
		_vm->_menuMan->f388_clearActingChampion();
		_vm->_menuMan->f386_drawActionIcon((ChampionIndex)(_g305_partyChampionCount - 1));
	}

	int16 mapX = _vm->_dungeonMan->_g306_partyMapX;
	int16 mapY = _vm->_dungeonMan->_g307_partyMapY;

	uint16 championObjectsCell = returnOppositeDir((direction)(dunMan._g308_partyDir));
	mapX += _vm->_dirIntoStepCountEast[dunMan._g308_partyDir];
	mapY += _vm->_dirIntoStepCountNorth[dunMan._g308_partyDir];
	thing = dunMan.f161_getSquareFirstThing(mapX, mapY);
	AL_0_slotIndex_Red = k13_ChampionSlotBackpackLine_1_1;
	uint16 slotIndex_Green;
	while (thing != Thing::_endOfList) {
		ThingType AL_2_thingType = thing.getType();
		if ((AL_2_thingType > k3_SensorThingType) && (thing.getCell() == championObjectsCell)) {
			int16 objectAllowedSlots = g237_ObjectInfo[dunMan.f141_getObjectInfoIndex(thing)].getAllowedSlots();
			switch (AL_2_thingType) {
			case k6_ArmourThingType:
				for (slotIndex_Green = k2_ChampionSlotHead; slotIndex_Green <= k5_ChampionSlotFeet; slotIndex_Green++) {
					if (objectAllowedSlots & gSlotMasks[slotIndex_Green])
						goto T0280048;
				}
				if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (champ->getSlot(k10_ChampionSlotNeck) == Thing::_none)) {
					slotIndex_Green = k10_ChampionSlotNeck;
				} else {
					goto T0280046;
				}
				break;
			case k5_WeaponThingType:
				if (champ->getSlot(k1_ChampionSlotActionHand) == Thing::_none) {
					slotIndex_Green = k1_ChampionSlotActionHand;
				} else {
					goto T0280046;
				}
				break;
			case k7_ScrollThingType:
			case k8_PotionThingType:
				if (champ->getSlot(k11_ChampionSlotPouch_1) == Thing::_none) {
					slotIndex_Green = k11_ChampionSlotPouch_1;
				} else if (champ->getSlot(k6_ChampionSlotPouch_2) == Thing::_none) {
					slotIndex_Green = k6_ChampionSlotPouch_2;
				} else {
					goto T0280046;
				}
				break;
			case k9_ContainerThingType:
			case k10_JunkThingType:
T0280046:
				if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (champ->getSlot(k10_ChampionSlotNeck) == Thing::_none)) {
					slotIndex_Green = k10_ChampionSlotNeck;
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
			f301_addObjectInSlot((ChampionIndex)prevChampCount, thing, (ChampionSlot)slotIndex_Green);
		}
		thing = dunMan.f159_getNextThing(thing);
	}

	_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)prevChampCount);
	_vm->_menuMan->f456_drawDisabledMenu();
}

void ChampionMan::f287_drawChampionBarGraphs(ChampionIndex champIndex) {

	Champion *curChampion = &_gK71_champions[champIndex];
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
	box._x1 = champIndex * k69_ChampionStatusBoxSpacing + 46;
	box._x2 = box._x1 + 3;
	box._y1 = 2;
	box._y2 = 26;

	for (int16 AL_0_barGraphIndex = 0; AL_0_barGraphIndex < 3; AL_0_barGraphIndex++) {
		int16 barGraphHeight = barGraphHeightArray[AL_0_barGraphIndex];
		if (barGraphHeight < 25) {
			box._y1 = 2;
			box._y1 = 27 - barGraphHeight;
			_vm->_displayMan->D24_fillScreenBox(box, g46_ChampionColor[champIndex]);
		}
		if (barGraphHeight) {
			box._y1 = 27 - barGraphHeight;
			box._y2 = 26;
			_vm->_displayMan->D24_fillScreenBox(box, g46_ChampionColor[champIndex]);
		}
		box._x1 += 7;
		box._x2 += 7;
	}
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}


uint16 ChampionMan::f306_getStaminaAdjustedValue(Champion *champ, int16 val) {
	int16 currStamina = champ->_currStamina;
	int16 halfMaxStamina = champ->_maxStamina / 2;
	if (currStamina < halfMaxStamina) {
		warning("Possible undefined behavior in the original code");
		val /= 2;
		return val + ((uint32)val * (uint32)currStamina) / halfMaxStamina;
	}
	return val;
}

uint16 ChampionMan::f309_getMaximumLoad(Champion *champ) {
	uint16 maximumLoad = champ->getStatistic(k1_ChampionStatStrength, k1_ChampionStatCurrent) * 8 + 100;
	maximumLoad = f306_getStaminaAdjustedValue(champ, maximumLoad);
	int16 wounds = champ->getWounds();
	if (wounds) {
		maximumLoad -= maximumLoad >> (champ->getWoundsFlag(k0x0010_ChampionWoundLegs) ? 2 : 3);
	}
	if (_vm->_objectMan->f33_getIconIndex(champ->getSlot(k5_ChampionSlotFeet)) == k119_IconIndiceArmourElvenBoots) {
		maximumLoad += maximumLoad * 16;
	}
	maximumLoad += 9;
	maximumLoad -= maximumLoad % 10;
	return maximumLoad;
}

void ChampionMan::f292_drawChampionState(ChampionIndex champIndex) {
	InventoryMan &invMan = *_vm->_inventoryMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	MenuMan &menuMan = *_vm->_menuMan;
	EventManager &eventMan = *_vm->_eventMan;

	Box box;
	int16 champStatusBoxX = champIndex * k69_ChampionStatusBoxSpacing;
	Champion *champ = &_gK71_champions[champIndex];
	uint16 champAttributes = champ->getAttributes();
	if (!((champAttributes) & (k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon |
							   k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport |
							   k0x8000_ChampionAttributeActionHand))) {
		return;
	}
	bool isInventoryChamp = (_vm->M0_indexToOrdinal(champIndex) == invMan._g432_inventoryChampionOrdinal);
	dispMan._g578_useByteBoxCoordinates = false;
	if (champAttributes & k0x1000_ChampionAttributeStatusBox) {
		box._y1 = 0;
		box._y2 = 28;
		box._x1 = champStatusBoxX;
		box._x2 = box._x1 + 66;
		if (champ->_currHealth) {
			dispMan.D24_fillScreenBox(box, k12_ColorDarkestGray);
			int16 nativeBitmapIndices[3];
			for (int16 i = 0; i < 3; ++i)
				nativeBitmapIndices[i] = 0;
			int16 AL_0_borderCount = 0;
			if (_g407_party._fireShieldDefense > 0)
				nativeBitmapIndices[AL_0_borderCount++] = k38_BorderPartyFireshieldIndice;
			if (_g407_party._spellShieldDefense > 0)
				nativeBitmapIndices[AL_0_borderCount++] = k39_BorderPartySpellshieldIndice;
			if (_g407_party._shieldDefense > 0)
				nativeBitmapIndices[AL_0_borderCount++] = k37_BorderPartyShieldIndice;
			while (AL_0_borderCount--) {
				dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(nativeBitmapIndices[AL_0_borderCount]),
										  dispMan._g348_bitmapScreen, box, 0, 0, 40, k160_byteWidthScreen, k10_ColorFlesh);
			}
			if (isInventoryChamp) {
				invMan.f354_drawStatusBoxPortrait(champIndex);
				champAttributes |= k0x0100_ChampionAttributeStatistics;
			} else {
				champAttributes |= (k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x2000_ChampionAttributeWounds | k0x8000_ChampionAttributeActionHand);
			}
		} else {
			dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k8_StatusBoxDeadChampion), dispMan._g348_bitmapScreen,
									  box, 0, 0, 40, k160_byteWidthScreen, k255_ColorNoTransparency);
			_vm->_textMan->f53_printToLogicalScreen(champStatusBoxX + 1, 5, k13_ColorLightestGray, k1_ColorDarkGary, champ->_name);
			menuMan.f386_drawActionIcon(champIndex);
			goto T0292042_green;
		}
	}

	if (!champ->_currHealth)
		goto T0292042_green;

	if (champAttributes & k0x0080_ChampionAttributeNameTitle) {
		Color AL_0_colorIndex = (champIndex == _g411_leaderIndex) ? k9_ColorGold : k13_ColorLightestGray; // unused because of missing functions
		if (isInventoryChamp) {
			char *champName = champ->_name;
			_vm->_textMan->f52_printToViewport(3, 7, AL_0_colorIndex, champName);
			int16 champTitleX = 6 * strlen(champName) + 3;
			char champTitleFirstChar = champ->_title[0];
			if ((champTitleFirstChar != ',') && (champTitleFirstChar != ';') && (champTitleFirstChar != '-')) {
				champTitleX += 6;
			}
			_vm->_textMan->f52_printToViewport(champTitleX, 7, AL_0_colorIndex, champ->_title);
			champAttributes |= k0x4000_ChampionAttributeViewport;
		} else {
			box._y1 = 0;
			box._y2 = 6;
			box._x1 = champStatusBoxX;
			box._x2 = box._x1 + 42;
			dispMan.D24_fillScreenBox(box, k1_ColorDarkGary);
			_vm->_textMan->f53_printToLogicalScreen(champStatusBoxX + 1, 5, AL_0_colorIndex, k1_ColorDarkGary, champ->_name);
		}
	}

	if (champAttributes & k0x0100_ChampionAttributeStatistics) {
		f287_drawChampionBarGraphs(champIndex);
		if (isInventoryChamp) {
			f290_drawHealthStaminaManaValues(champ);
			int16 AL_2_nativeBitmapIndex;
			if ((champ->_food < 0) || (champ->_water < 0) || (champ->_poisonEventCount)) {
				AL_2_nativeBitmapIndex = k34_SlotBoxWoundedIndice;
			} else {
				AL_2_nativeBitmapIndex = k33_SlotBoxNormalIndice;
			}
			dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(AL_2_nativeBitmapIndex), dispMan._g296_bitmapViewport,
									  gBoxMouth, 0, 0, 16, k112_byteWidthViewport, k12_ColorDarkestGray);
			AL_2_nativeBitmapIndex = k33_SlotBoxNormalIndice;
			for (int16 AL_0_statisticIndex = k1_ChampionStatStrength; AL_0_statisticIndex <= k6_ChampionStatAntifire; AL_0_statisticIndex++) {
				if (champ->getStatistic((ChampionStatisticType)AL_0_statisticIndex, k1_ChampionStatCurrent)
					< champ->getStatistic((ChampionStatisticType)AL_0_statisticIndex, k0_ChampionStatMaximum)) {
					AL_2_nativeBitmapIndex = k34_SlotBoxWoundedIndice;
					break;
				}
			}
			dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(AL_2_nativeBitmapIndex), dispMan._g296_bitmapViewport, gBoxEye,
									  0, 0, 16, k112_byteWidthViewport, k12_ColorDarkestGray);
			champAttributes |= k0x4000_ChampionAttributeViewport;
		}
	}

	if (champAttributes & k0x2000_ChampionAttributeWounds) {
		for (int16 AL_0_slotIndex = isInventoryChamp ? k5_ChampionSlotFeet : k1_ChampionSlotActionHand; AL_0_slotIndex >= k0_ChampionSlotReadyHand; AL_0_slotIndex--) {
			f291_drawSlot(champIndex, (ChampionSlot)AL_0_slotIndex);
		}
		if (isInventoryChamp) {
			champAttributes |= k0x4000_ChampionAttributeViewport;
		}
	}

	if ((champAttributes & k0x0200_ChampionAttributeLoad) && isInventoryChamp) {
		Color loadColor;
		int16 champMaxLoad = f309_getMaximumLoad(champ);
		if (champ->_load > champMaxLoad) {
			loadColor = k8_ColorRed;
		} else if (((int32)champ->_load) * 8 > ((int32)champMaxLoad) * 5) {
			loadColor = k11_ColorYellow;
		} else {
			loadColor = k13_ColorLightestGray;
		}
		_vm->_textMan->f52_printToViewport(104, 132, loadColor, "LOAD ");

		int16 loadTmp = champ->_load / 10;
		Common::String str = f288_getStringFromInteger(loadTmp, true, 3);
		str += '.';
		loadTmp = champ->_load - (loadTmp * 10);
		str += f288_getStringFromInteger(loadTmp, false, 1);
		str += '/';
		loadTmp = (f309_getMaximumLoad(champ) + 5) / 10;
		str += "KG";
		_vm->_textMan->f52_printToViewport(148, 132, loadColor, str.c_str());
		champAttributes |= k0x4000_ChampionAttributeViewport;
	}

	{ // block so goto won't skip AL_0_championIconIndex initialization 
		int16 AL_0_championIconIndex = M26_championIconIndex(champ->_cell, _vm->_dungeonMan->_g308_partyDir);

		if ((champAttributes & k28_ChampionIcons) && (eventMan._g599_useChampionIconOrdinalAsMousePointerBitmap != _vm->M0_indexToOrdinal(AL_0_championIconIndex))) {
			dispMan.D24_fillScreenBox(g54_BoxChampionIcons[AL_0_championIconIndex], g46_ChampionColor[champIndex]);
			dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k28_ChampionIcons),
									  dispMan._g348_bitmapScreen,
									  g54_BoxChampionIcons[AL_0_championIconIndex << 2],
									  M26_championIconIndex(champ->_dir, _vm->_dungeonMan->_g308_partyDir) * 19, 0,
									  40, k160_byteWidthScreen, k12_ColorDarkestGray);
		}
	}

	if ((champAttributes & k0x0800_ChampionAttributePanel) && isInventoryChamp) {
		if (_vm->_g333_pressingMouth) {
			invMan.f345_drawPanelFoodWaterPoisoned();
		} else if (_vm->_g331_pressingEye) {
			if (_g415_leaderEmptyHanded) {
				warning("MISSING CODE: F0351_INVENTORY_DrawChampionSkillsAndStatistics");
			}
		} else {
			invMan.f347_drawPanel();
		}
		champAttributes |= k0x4000_ChampionAttributeViewport;
	}

	if (champAttributes & k0x8000_ChampionAttributeActionHand) {
		f291_drawSlot(champIndex, k1_ChampionSlotActionHand);
		menuMan.f386_drawActionIcon(champIndex);
		if (isInventoryChamp) {
			champAttributes |= k0x4000_ChampionAttributeViewport;
		}
	}

	if (champAttributes & k0x4000_ChampionAttributeViewport) {
		dispMan.f97_drawViewport(k0_viewportNotDungeonView);
	}


T0292042_green:
	champ->setAttributeFlag((ChampionAttribute)(k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon |
												k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport |
												k0x8000_ChampionAttributeActionHand), false);
	warning("MISSING CODE: F0078_MOUSE_ShowPointer");
}

uint16 ChampionMan::M26_championIconIndex(int16 val, direction dir) {
	return ((val + 4 - dir) & 0x3);
}

void ChampionMan::f290_drawHealthStaminaManaValues(Champion* champ) {
	f289_drawHealthOrStaminaOrManaValue(116, champ->_currHealth, champ->_maxHealth);
	f289_drawHealthOrStaminaOrManaValue(124, champ->_currStamina, champ->_maxStamina);
	f289_drawHealthOrStaminaOrManaValue(132, champ->_currMana, champ->_maxMana);
}

void ChampionMan::f291_drawSlot(uint16 champIndex, ChampionSlot slotIndex) {
	int16 nativeBitmapIndex = -1;
	Champion *champ = &_gK71_champions[champIndex];
	bool isInventoryChamp = (_vm->_inventoryMan->_g432_inventoryChampionOrdinal == _vm->M0_indexToOrdinal(champIndex));

	uint16 slotBoxIndex;
	if (!isInventoryChamp) {  /* If drawing a slot for a champion other than the champion whose inventory is open */
		if ((slotIndex > k1_ChampionSlotActionHand) || (_g299_candidateChampionOrdinal == _vm->M0_indexToOrdinal(champIndex))) {
			return;
		}
		slotBoxIndex = (champIndex << 1) + slotIndex;
	} else {
		slotBoxIndex = k8_SlotBoxInventoryFirstSlot + slotIndex;
	}

	Thing thing;
	if (slotIndex >= k30_ChampionSlotChest_1) {
		thing = _vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1];
	} else {
		thing = champ->getSlot(slotIndex);
	}

	SlotBox *slotBox = &_vm->_objectMan->_g30_slotBoxes[slotBoxIndex];
	Box box;
	box._x1 = slotBox->_x - 1;
	box._y1 = slotBox->_y - 1;
	box._x2 = box._x1 + 17;
	box._y2 = box._y1 + 17;


	if (!isInventoryChamp) {
		warning("MISSING CODE: F0077_MOUSE_HidePointer_CPSE");
	}

	int16 iconIndex;
	if (thing == Thing::_none) {
		if (slotIndex <= k5_ChampionSlotFeet) {
			iconIndex = k212_IconIndiceReadyHand + (slotIndex << 1);
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex))) {
				iconIndex++;
				nativeBitmapIndex = k34_SlotBoxWoundedIndice;
			} else {
				nativeBitmapIndex = k33_SlotBoxNormalIndice;
			}
		} else {
			if ((slotIndex >= k10_ChampionSlotNeck) && (slotIndex <= k13_ChampionSlotBackpackLine_1_1)) {
				iconIndex = k208_IconIndiceNeck + (slotIndex - k10_ChampionSlotNeck);
			} else {
				iconIndex = k204_IconIndiceEmptyBox;
			}
		}
	} else {
		warning("BUG0_35");
		iconIndex = _vm->_objectMan->f33_getIconIndex(thing); // BUG0_35
		if (isInventoryChamp && (slotIndex == k1_ChampionSlotActionHand) && ((iconIndex == k144_IconIndiceContainerChestClosed) || (iconIndex == k30_IconIndiceScrollOpen))) {
			warning("BUG2_00");
			iconIndex++;
		} // BUG2_00
		if (slotIndex <= k5_ChampionSlotFeet) {
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex))) {
				nativeBitmapIndex = k34_SlotBoxWoundedIndice;
			} else {
				nativeBitmapIndex = k33_SlotBoxNormalIndice;
			}
		}
	}

	if ((slotIndex == k1_ChampionSlotActionHand) && (_vm->M0_indexToOrdinal(champIndex) == _g506_actingChampionOrdinal)) {
		nativeBitmapIndex = k35_SlotBoxActingHandIndice;
	}

	if (nativeBitmapIndex != -1) {
		_vm->_displayMan->_g578_useByteBoxCoordinates = false;
		if (isInventoryChamp) {
			_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->f489_getNativeBitmapOrGraphic(nativeBitmapIndex),
												_vm->_displayMan->_g296_bitmapViewport, box, 0, 0, 16, k112_byteWidthViewport, k12_ColorDarkestGray);
		} else {
			_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->f489_getNativeBitmapOrGraphic(nativeBitmapIndex),
												_vm->_displayMan->_g348_bitmapScreen, box, 0, 0, 16, k160_byteWidthScreen, k12_ColorDarkestGray);
		}
	}

	_vm->_objectMan->f38_drawIconInSlotBox(slotBoxIndex, iconIndex);

	if (!isInventoryChamp) {
		warning("MISSING CODE: F0078_MOUSE_ShowPointer");
	}
}

void ChampionMan::f281_renameChampion(Champion* champ) {
	warning("STUB METHOD: Champion::renameChampion, F0281_CHAMPION_Rename");

	DisplayMan &dispMan = *_vm->_displayMan;
	TextMan &textMan = *_vm->_textMan;

	Box box;
	box._y1 = 3;
	box._y2 = 8;
	box._x1 = 3;
	box._x2 = box._x1 + 167;

	dispMan.f135_fillBoxBitmap(dispMan._g296_bitmapViewport, box, k12_ColorDarkestGray, k112_byteWidthViewport, k136_heightViewport);
	dispMan.f132_blitToBitmap(dispMan.f489_getNativeBitmapOrGraphic(k27_PanelRenameChampionIndice), dispMan._g296_bitmapViewport, g32_BoxPanel,
							  0, 0, 72, k112_byteWidthViewport, k4_ColorCyan);
	textMan.f52_printToViewport(177, 58, k13_ColorLightestGray, "_______");
	textMan.f52_printToViewport(105, 76, k13_ColorLightestGray, "___________________");
	Common::Point clickPos;
	static Box okButtonBox(197, 215, 147, 155);
	for (;;) {
		_vm->_eventMan->processInput();
		if (_vm->_eventMan->f360_hasPendingClick(clickPos, k1_LeftMouseButton) && okButtonBox.isPointInside(clickPos)) {
			return;
		}
		dispMan.f97_drawViewport(k0_viewportNotDungeonView);
		dispMan.updateScreen();
	}
}

uint16 ChampionMan::f303_getSkillLevel(ChampionIndex champIndex, ChampionSkill skillIndex) {
	if (_g300_partyIsSleeping)
		return 1;

	bool ignoreTempExp = skillIndex & k0x8000_IgnoreTemporaryExperience;
	bool ignoreObjModifiers = skillIndex & k0x4000_IgnoreObjectModifiers;
	skillIndex = (ChampionSkill)(skillIndex & ~(ignoreTempExp | ignoreObjModifiers));
	Champion *champ = &_gK71_champions[champIndex];
	Skill *skill = &champ->getSkill(skillIndex);
	int32 experience = skill->_experience;

	if (!ignoreTempExp)
		experience += skill->_temporaryExperience;

	if (skillIndex > k3_ChampionSkillWizard) { // hidden skill
		skill = &champ->getSkill((ChampionSkill)((skillIndex - k4_ChampionSkillSwing) / 4));
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
		IconIndice actionHandIconIndex = _vm->_objectMan->f33_getIconIndex(champ->getSlot(k1_ChampionSlotActionHand));
		if (actionHandIconIndex == k27_IconIndiceWeaponTheFirestaff) {
			skillLevel++;
		} else if (actionHandIconIndex == k28_IconIndiceWeaponTheFirestaffComplete) {
			skillLevel += 2;
		}

		IconIndice neckIconIndice = _vm->_objectMan->f33_getIconIndex(champ->getSlot(k10_ChampionSlotNeck));
		switch (skillIndex) {
		case k3_ChampionSkillWizard:
			if (neckIconIndice == k124_IconIndiceJunkPendantFeral)
				skillLevel++;
			break;
		case k15_ChampionSkillDefend:
			if (neckIconIndice == k121_IconIndiceJunkEkkhardCross)
				skillLevel++;
			break;
		case k13_ChampionSkillHeal:
			// these two are not cummulative
			if ((neckIconIndice == k120_IconIndiceJunkGemOfAges) || (neckIconIndice == k66_IconIndiceWeaponSceptreOfLyf))
				skillLevel++;
			break;
		case k14_ChampionSkillInfluence:
			if (neckIconIndice == k122_IconIndiceJunkMoonstone)
				skillLevel++;
			break;
		}
	}
	return skillLevel;
}

}


