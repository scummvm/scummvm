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
#include "timeline.h"
#include "projexpl.h"
#include "group.h"


namespace DM {

const char *g417_baseSkillName[4] = {"FIGHTER", "NINJA", "PRIEST", "WIZARD"};

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
	for (uint16 i = 0; i < 4; ++i) {
		_g409_championPendingDamage[i] = 0;
		_g410_championPendingWounds[i] = 0;
	}

}

bool ChampionMan::f329_isLeaderHandObjectThrown(int16 side) {
	if (_g411_leaderIndex == kM1_ChampionNone) {
		return false;
	}
	return f328_isObjectThrown(_g411_leaderIndex, kM1_ChampionSlotLeaderHand, side);
}

bool ChampionMan::f328_isObjectThrown(uint16 champIndex, int16 slotIndex, int16 side) {
	int16 L0993_i_KineticEnergy;
	int16 L0994_i_Multiple;
#define AL0994_i_Experience L0994_i_Multiple
#define AL0994_i_Attack     L0994_i_Multiple
	int16 L0995_i_Multiple;
#define AL0995_i_WeaponKineticEnergy L0995_i_Multiple
#define AL0995_i_SkillLevel          L0995_i_Multiple
#define AL0995_i_StepEnergy          L0995_i_Multiple
	Thing L0996_T_Thing;
	Champion* L0997_ps_Champion = nullptr;
	WeaponInfo* L0998_ps_WeaponInfo;
	Thing L0999_T_ActionHandThing;
	bool L1000_B_ThrowingLeaderHandObject;


	L1000_B_ThrowingLeaderHandObject = false;
	if (slotIndex < 0) { /* Throw object in leader hand, which is temporarily placed in action hand */
		if (_g415_leaderEmptyHanded) {
			return false;
		}
		L0996_T_Thing = f298_getObjectRemovedFromLeaderHand();
		L0997_ps_Champion = &_gK71_champions[champIndex];
		L0999_T_ActionHandThing = L0997_ps_Champion->getSlot(k1_ChampionSlotActionHand);
		L0997_ps_Champion->setSlot(k1_ChampionSlotActionHand, L0996_T_Thing);
		slotIndex = k1_ChampionSlotActionHand;
		L1000_B_ThrowingLeaderHandObject = true;
	}
	L0993_i_KineticEnergy = f312_getStrength(champIndex, slotIndex);
	if (L1000_B_ThrowingLeaderHandObject) {
		L0997_ps_Champion->setSlot((ChampionSlot)slotIndex, L0999_T_ActionHandThing);
	} else {
		if ((L0996_T_Thing = f300_getObjectRemovedFromSlot(champIndex, slotIndex)) == Thing::_none) {
			return false;
		}
	}
	warning("MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
	f325_decrementStamine(champIndex, f305_getThrowingStaminaCost(L0996_T_Thing));
	f330_disableAction(champIndex, 4);
	AL0994_i_Experience = 8;
	AL0995_i_WeaponKineticEnergy = 1;
	if (L0996_T_Thing.getType() == k5_WeaponThingType) {
		AL0994_i_Experience += 4;
		L0998_ps_WeaponInfo = _vm->_dungeonMan->f158_getWeaponInfo(L0996_T_Thing);
		if (L0998_ps_WeaponInfo->_class <= k12_WeaponClassPoisinDart) {
			AL0994_i_Experience += (AL0995_i_WeaponKineticEnergy = L0998_ps_WeaponInfo->_kineticEnergy) >> 2;
		}
	}
	f304_addSkillExperience(champIndex, k10_ChampionSkillThrow, AL0994_i_Experience);
	L0993_i_KineticEnergy += AL0995_i_WeaponKineticEnergy;
	AL0995_i_SkillLevel = f303_getSkillLevel((ChampionIndex)champIndex, k10_ChampionSkillThrow);
	L0993_i_KineticEnergy += _vm->_rnd->getRandomNumber(16) + (L0993_i_KineticEnergy >> 1) + AL0995_i_SkillLevel;
	AL0994_i_Attack = f26_getBoundedValue((uint16)40, (uint16)((AL0995_i_SkillLevel << 3) + _vm->_rnd->getRandomNumber(31)), (uint16)200);
	AL0995_i_StepEnergy = MAX(5, 11 - AL0995_i_SkillLevel);
	_vm->_projexpl->f212_projectileCreate(L0996_T_Thing, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY,
							M21_normalizeModulo4(_vm->_dungeonMan->_g308_partyDir + side),
							_vm->_dungeonMan->_g308_partyDir, L0993_i_KineticEnergy, AL0994_i_Attack, AL0995_i_StepEnergy);
	_vm->_g311_projectileDisableMovementTicks = 4;
	_vm->_g312_lastProjectileDisabledMovementDirection = _vm->_dungeonMan->_g308_partyDir;
	f292_drawChampionState((ChampionIndex)champIndex);
	return true;
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

void ChampionMan::f299_applyModifiersToStatistics(Champion* champ, int16 slotIndex, int16 iconIndex, int16 modifierFactor, Thing thing) {
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
				_vm->_eventMan->f77_hideMouse();
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
			_vm->_eventMan->f77_hideMouse();
			objMan.f36_extractIconFromBitmap(iconIndex, objMan._g412_objectIconForMousePointer);
			_vm->_eventMan->f68_setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);
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
		_vm->_eventMan->f78_showMouse();
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
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			f296_drawChangedObjectIcons();
		} else if (isInventoryChampion && (slotIndex == k1_ChampionSlotActionHand) &&
			((iconIndex == k144_IconIndiceContainerChestClosed) || ((iconIndex >= k30_IconIndiceScrollOpen) && (iconIndex <= k31_IconIndiceScrollClosed)))) {
			champ->setAttributeFlag(k0x0800_ChampionAttributePanel, true);
		}

	} else if (slotIndex == k10_ChampionSlotNeck) {

		if ((iconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (iconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk*)rawObjPtr)->setChargeCount(1);
			_g407_party._magicalLightAmount += g39_LightPowerToLightAmount[2];
			_vm->_inventoryMan->f337_setDungeonViewPalette();
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

Thing ChampionMan::f298_getObjectRemovedFromLeaderHand() {
	Thing L0890_T_LeaderHandObject;


	_g415_leaderEmptyHanded = true;
	if ((L0890_T_LeaderHandObject = _g414_leaderHandObject) != Thing::_none) {
		_g414_leaderHandObject = Thing::_none;
		_g413_leaderHandObjectIconIndex = kM1_IconIndiceNone;
		_vm->_eventMan->f78_showMouse();
		_vm->_objectMan->f35_clearLeaderObjectName();
		_vm->_eventMan->f69_setMousePointer();
		_vm->_eventMan->f77_hideMouse();
		if (_g411_leaderIndex != kM1_ChampionNone) {
			_gK71_champions[_g411_leaderIndex]._load -= _vm->_dungeonMan->f140_getObjectWeight(L0890_T_LeaderHandObject);
			setFlag(_gK71_champions[_g411_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
			f292_drawChampionState(_g411_leaderIndex);
		}
	}
	return L0890_T_LeaderHandObject;
}

uint16 ChampionMan::f312_getStrength(int16 champIndex, int16 slotIndex) {
	int16 L0935_i_Strength;
	uint16 L0936_ui_Multiple;
#define AL0936_ui_ObjectWeight L0936_ui_Multiple
#define AL0936_ui_SkillLevel   L0936_ui_Multiple
	uint16 L0937_ui_Multiple;
#define AL0937_ui_OneSixteenthMaximumLoad L0937_ui_Multiple
#define AL0937_ui_Class                   L0937_ui_Multiple
	Thing L0938_T_Thing;
	Champion* L0939_ps_Champion;
	WeaponInfo* L0940_ps_WeaponInfo;
	int16 L0941_i_LoadThreshold;


	L0939_ps_Champion = &_gK71_champions[champIndex];
	L0935_i_Strength = _vm->_rnd->getRandomNumber(15) + L0939_ps_Champion->_statistics[k1_ChampionStatStrength][k1_ChampionStatCurrent];
	L0938_T_Thing = L0939_ps_Champion->_slots[slotIndex];
	if ((AL0936_ui_ObjectWeight = _vm->_dungeonMan->f140_getObjectWeight(L0938_T_Thing)) <= (AL0937_ui_OneSixteenthMaximumLoad = f309_getMaximumLoad(L0939_ps_Champion) >> 4)) {
		L0935_i_Strength += AL0936_ui_ObjectWeight - 12;
	} else {
		if (AL0936_ui_ObjectWeight <= (L0941_i_LoadThreshold = AL0937_ui_OneSixteenthMaximumLoad + ((AL0937_ui_OneSixteenthMaximumLoad - 12) >> 1))) {
			L0935_i_Strength += (AL0936_ui_ObjectWeight - AL0937_ui_OneSixteenthMaximumLoad) >> 1;
		} else {
			L0935_i_Strength -= (AL0936_ui_ObjectWeight - L0941_i_LoadThreshold) << 1;
		}
	}
	if (L0938_T_Thing.getType() == k5_WeaponThingType) {
		L0940_ps_WeaponInfo = _vm->_dungeonMan->f158_getWeaponInfo(L0938_T_Thing);
		L0935_i_Strength += L0940_ps_WeaponInfo->_strength;
		AL0936_ui_SkillLevel = 0;
		AL0937_ui_Class = L0940_ps_WeaponInfo->_class;
		if ((AL0937_ui_Class == k0_WeaponClassSwingWeapon) || (AL0937_ui_Class == k2_WeaponClassDaggerAndAxes)) {
			AL0936_ui_SkillLevel = f303_getSkillLevel(champIndex, k4_ChampionSkillSwing);
		}
		if ((AL0937_ui_Class != k0_WeaponClassSwingWeapon) && (AL0937_ui_Class < k16_WeaponClassFirstBow)) {
			AL0936_ui_SkillLevel += f303_getSkillLevel(champIndex, k10_ChampionSkillThrow);
		}
		if ((AL0937_ui_Class >= k16_WeaponClassFirstBow) && (AL0937_ui_Class < k112_WeaponClassFirstMagicWeapon)) {
			AL0936_ui_SkillLevel += f303_getSkillLevel(champIndex, k11_ChampionSkillShoot);
		}
		L0935_i_Strength += AL0936_ui_SkillLevel << 1;
	}
	L0935_i_Strength = f306_getStaminaAdjustedValue(L0939_ps_Champion, L0935_i_Strength);
	if (getFlag(L0939_ps_Champion->_wounds, (slotIndex == k0_ChampionSlotReadyHand) ? k0x0001_ChampionWoundReadHand : k0x0002_ChampionWoundActionHand)) {
		L0935_i_Strength >>= 1;
	}
	MAX(1, 2);
	return f26_getBoundedValue(0, L0935_i_Strength >> 1, 100);
}

Thing ChampionMan::f300_getObjectRemovedFromSlot(uint16 champIndex, uint16 slotIndex) {
	Thing L0894_T_Thing;
	int16 L0895_i_IconIndex;
	Champion* L0896_ps_Champion;
	Weapon* L0897_ps_Weapon;
	bool L0898_B_IsInventoryChampion;


	L0896_ps_Champion = &_gK71_champions[champIndex];
	if (slotIndex >= k30_ChampionSlotChest_1) {
		L0894_T_Thing = _vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1];
		_vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1] = Thing::_none;
	} else {
		L0894_T_Thing = L0896_ps_Champion->_slots[slotIndex];
		L0896_ps_Champion->_slots[slotIndex] = Thing::_none;
	}
	if (L0894_T_Thing == Thing::_none) {
		return Thing::_none;
	}
	L0898_B_IsInventoryChampion = (_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal);
	L0895_i_IconIndex = _vm->_objectMan->f33_getIconIndex(L0894_T_Thing);
	f299_applyModifiersToStatistics(L0896_ps_Champion, slotIndex, L0895_i_IconIndex, -1, L0894_T_Thing); /* Remove objet modifiers */
	L0897_ps_Weapon = (Weapon*)_vm->_dungeonMan->f156_getThingData(L0894_T_Thing);
	if (slotIndex == k10_ChampionSlotNeck) {
		if ((L0895_i_IconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (L0895_i_IconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk*)L0897_ps_Weapon)->setChargeCount(0);
			_g407_party._magicalLightAmount -= g39_LightPowerToLightAmount[2];
			_vm->_inventoryMan->f337_setDungeonViewPalette();
		} else {
			if ((L0895_i_IconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (L0895_i_IconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
				((Junk*)L0897_ps_Weapon)->setChargeCount(0);
			}
		}
	}
	f291_drawSlot(champIndex, slotIndex);
	if (L0898_B_IsInventoryChampion) {
		setFlag(L0896_ps_Champion->_attributes, k0x4000_ChampionAttributeViewport);
	}
	if (slotIndex < k2_ChampionSlotHead) {
		if (slotIndex == k1_ChampionSlotActionHand) {
			setFlag(L0896_ps_Champion->_attributes, k0x8000_ChampionAttributeActionHand);
			if (_g506_actingChampionOrdinal == _vm->M0_indexToOrdinal(champIndex)) {
				_vm->_menuMan->f388_clearActingChampion();
			}
			if ((L0895_i_IconIndex >= k30_IconIndiceScrollOpen) && (L0895_i_IconIndex <= k31_IconIndiceScrollClosed)) {
				((Scroll*)L0897_ps_Weapon)->setClosed(true);
				f296_drawChangedObjectIcons();
			}
		}
		if ((L0895_i_IconIndex >= k4_IconIndiceWeaponTorchUnlit) && (L0895_i_IconIndex <= k7_IconIndiceWeaponTorchLit)) {
			L0897_ps_Weapon->setLit(false);
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			f296_drawChangedObjectIcons();
		}
		if (L0898_B_IsInventoryChampion && (slotIndex == k1_ChampionSlotActionHand)) {
			if (L0895_i_IconIndex == k144_IconIndiceContainerChestClosed) {
				_vm->_inventoryMan->f334_closeChest();
				goto T0300011;
			}
			if ((L0895_i_IconIndex >= k30_IconIndiceScrollOpen) && (L0895_i_IconIndex <= k31_IconIndiceScrollClosed)) {
T0300011:
				setFlag(L0896_ps_Champion->_attributes, k0x0800_ChampionAttributePanel);
			}
		}
	}
	L0896_ps_Champion->_load -= _vm->_dungeonMan->f140_getObjectWeight(L0894_T_Thing);
	setFlag(L0896_ps_Champion->_attributes, k0x0200_ChampionAttributeLoad);
	return L0894_T_Thing;
}

void ChampionMan::f325_decrementStamine(int16 championIndex, int16 decrement) {
	int16 L0988_i_Stamina;
	Champion* L0989_ps_Champion;


	if (championIndex == kM1_ChampionNone) {
		return;
	}
	L0989_ps_Champion = &_gK71_champions[championIndex];
	if ((L0988_i_Stamina = (L0989_ps_Champion->_currStamina -= decrement)) <= 0) {
		L0989_ps_Champion->_currStamina = 0;
		f321_addPendingDamageAndWounds_getDamage(championIndex, (-L0988_i_Stamina) >> 1, k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
	} else {
		if (L0988_i_Stamina > L0989_ps_Champion->_maxStamina) {
			L0989_ps_Champion->_currStamina = L0989_ps_Champion->_maxStamina;
		}
	}
	setFlag(L0989_ps_Champion->_attributes, k0x0200_ChampionAttributeLoad | k0x0100_ChampionAttributeStatistics);
}

int16 ChampionMan::f321_addPendingDamageAndWounds_getDamage(int16 champIndex, int16 attack, int16 allowedWounds, uint16 attackType) {
	int16 L0976_i_Multiple;
#define AL0976_i_WoundIndex     L0976_i_Multiple
#define AL0976_i_WisdomFactor   L0976_i_Multiple
#define AL0976_i_AdjustedAttack L0976_i_Multiple
	uint16 L0977_ui_Defense;
	uint16 L0978_ui_WoundCount;
	Champion* L0979_ps_Champion;

	if (attack <= 0)
		return 0;

	L0979_ps_Champion = &_gK71_champions[champIndex];
	if (!L0979_ps_Champion->_currHealth) {
T0321004:
		return 0;
	}
	if (attackType != k0_attackType_NORMAL) {
		for (L0978_ui_WoundCount = 0, AL0976_i_WoundIndex = k0_ChampionSlotReadyHand, L0977_ui_Defense = 0; AL0976_i_WoundIndex <= k5_ChampionSlotFeet; AL0976_i_WoundIndex++) {
			if (allowedWounds & (1 << AL0976_i_WoundIndex)) {
				L0978_ui_WoundCount++;
				L0977_ui_Defense += f313_getWoundDefense(champIndex, AL0976_i_WoundIndex | ((attackType == k4_attackType_SHARP) ? k0x8000_maskUseSharpDefense : k0x0000_maskDoNotUseSharpDefense));
			}
		}
		if (L0978_ui_WoundCount) {
			L0977_ui_Defense /= L0978_ui_WoundCount;
		}
		switch (attackType) {
		case k6_attackType_PSYCHIC:
			if ((AL0976_i_WisdomFactor = 115 - L0979_ps_Champion->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent]) <= 0) {
				attack = 0;
			} else {
				attack = _vm->f30_getScaledProduct(attack, 6, AL0976_i_WisdomFactor);
			}
			goto T0321024;
		case k5_attackType_MAGIC:
			attack = f307_getStatisticAdjustedAttack(L0979_ps_Champion, k5_ChampionStatAntimagic, attack);
			attack -= _g407_party._spellShieldDefense;
			goto T0321024;
		case k1_attackType_FIRE:
			attack = f307_getStatisticAdjustedAttack(L0979_ps_Champion, k6_ChampionStatAntifire, attack);
			attack -= _g407_party._fireShieldDefense;
			break;
		case k2_attackType_SELF:
			L0977_ui_Defense >>= 1;
		case k3_attackType_BLUNT:
		case k4_attackType_SHARP:
		case k7_attackType_LIGHTNING:
			;
		}
		if (attack <= 0)
			goto T0321004;
		attack = _vm->f30_getScaledProduct(attack, 6, 130 - L0977_ui_Defense);
		/* BUG0_44 A champion may take much more damage than expected after a Black Flame attack or an impact
	with a Fireball projectile. If the party has a fire shield defense value higher than the fire attack value then the resulting intermediary
	attack value is negative and damage should be 0. However, the negative value is still used for further computations and the result may be a very
	high positive attack value which may kill a champion. This can occur only for k1_attackType_FIRE and if attack is negative before calling F0030_MAIN_GetScaledProduct */
T0321024:
		if (attack <= 0)
			goto T0321004;
		if (attack > (AL0976_i_AdjustedAttack = f307_getStatisticAdjustedAttack(L0979_ps_Champion, k4_ChampionStatVitality, _vm->_rnd->getRandomNumber(127) + 10))) { /* BUG0_45 This bug is not perceptible because of BUG0_41 that ignores Vitality while determining the probability of being wounded. However if it was fixed, the behavior would be the opposite of what it should: the higher the vitality of a champion, the lower the result of F0307_CHAMPION_GetStatisticAdjustedAttack and the more likely the champion could get wounded (because of more iterations in the loop below) */
			do {
				setFlag(*(uint16*)&_g410_championPendingWounds[champIndex], (1 << _vm->_rnd->getRandomNumber(7)) & allowedWounds);
			} while ((attack > (AL0976_i_AdjustedAttack <<= 1)) && AL0976_i_AdjustedAttack);
		}
		if (_g300_partyIsSleeping) {
			f314_wakeUp();
		}
	}
	_g409_championPendingDamage[champIndex] += attack;
	return attack;
}

int16 ChampionMan::f313_getWoundDefense(int16 champIndex, uint16 woundIndex) {
	static byte g50_woundDefenseFactor[6] = {5, 5, 4, 6, 3, 1}; // @ G0050_auc_Graphic562_WoundDefenseFactor

	int16 L0942_i_Multiple;
#define AL0942_i_SlotIndex    L0942_i_Multiple
#define AL0942_i_WoundDefense L0942_i_Multiple
	uint16 L0943_ui_ArmourShieldDefense;
	bool L0944_B_UseSharpDefense;
	Thing L0945_T_Thing;
	Champion* L0946_ps_Champion;
	ArmourInfo* L0947_ps_ArmourInfo;


	L0946_ps_Champion = &_gK71_champions[champIndex];
	if (L0944_B_UseSharpDefense = getFlag(woundIndex, k0x8000_maskUseSharpDefense)) {
		clearFlag(woundIndex, k0x8000_maskUseSharpDefense);
	}
	for (L0943_ui_ArmourShieldDefense = 0, AL0942_i_SlotIndex = k0_ChampionSlotReadyHand; AL0942_i_SlotIndex <= k1_ChampionSlotActionHand; AL0942_i_SlotIndex++) {
		if ((L0945_T_Thing = L0946_ps_Champion->_slots[AL0942_i_SlotIndex]).getType() == k6_ArmourThingType) {
			L0947_ps_ArmourInfo = (ArmourInfo*)_vm->_dungeonMan->f156_getThingData(L0945_T_Thing);
			L0947_ps_ArmourInfo = &g239_ArmourInfo[((Armour*)L0947_ps_ArmourInfo)->getType()];
			if (getFlag(L0947_ps_ArmourInfo->_attributes, k0x0080_ArmourAttributeIsAShield)) {
				L0943_ui_ArmourShieldDefense += ((f312_getStrength(champIndex, AL0942_i_SlotIndex) + _vm->_dungeonMan->f143_getArmourDefense(L0947_ps_ArmourInfo, L0944_B_UseSharpDefense)) * g50_woundDefenseFactor[woundIndex]) >> ((AL0942_i_SlotIndex == woundIndex) ? 4 : 5);
			}
		}
	}
	AL0942_i_WoundDefense = _vm->getRandomNumber((L0946_ps_Champion->_statistics[k4_ChampionStatVitality][k1_ChampionStatCurrent] >> 3) + 1);
	if (L0944_B_UseSharpDefense) {
		AL0942_i_WoundDefense >>= 1;
	}
	AL0942_i_WoundDefense += L0946_ps_Champion->_actionDefense + L0946_ps_Champion->_shieldDefense + _g407_party._shieldDefense + L0943_ui_ArmourShieldDefense;
	if ((woundIndex > k1_ChampionSlotActionHand) && ((L0945_T_Thing = L0946_ps_Champion->_slots[woundIndex]).getType() == k6_ArmourThingType)) {
		L0947_ps_ArmourInfo = (ArmourInfo*)_vm->_dungeonMan->f156_getThingData(L0945_T_Thing);
		AL0942_i_WoundDefense += _vm->_dungeonMan->f143_getArmourDefense(&g239_ArmourInfo[((Armour*)L0947_ps_ArmourInfo)->getType()], L0944_B_UseSharpDefense);
	}
	if (getFlag(L0946_ps_Champion->_wounds, 1 << woundIndex)) {
		AL0942_i_WoundDefense -= 8 + _vm->getRandomNumber(4);
	}
	if (_g300_partyIsSleeping) {
		AL0942_i_WoundDefense >>= 1;
	}
	return f26_getBoundedValue(0, AL0942_i_WoundDefense >> 1, 100);
}

uint16 ChampionMan::f307_getStatisticAdjustedAttack(Champion* champ, uint16 statIndex, uint16 attack) {
	int16 L0927_i_Factor;

	if ((L0927_i_Factor = 170 - champ->_statistics[statIndex][k1_ChampionStatCurrent]) < 16) { /* BUG0_41 The Antifire and Antimagic statistics are completely ignored. The Vitality statistic is ignored against poison and to determine the probability of being wounded. Vitality is still used normally to compute the defense against wounds and the speed of health regeneration. A bug in the Megamax C compiler produces wrong machine code for this statement. It always returns 0 for the current statistic value so that L0927_i_Factor = 170 in all cases */
		return attack >> 3;
	}
	return _vm->f30_getScaledProduct(attack, 7, L0927_i_Factor);
}

void ChampionMan::f314_wakeUp() {
	_vm->_g321_stopWaitingForPlayerInput = true;
	_g300_partyIsSleeping = false;
	_vm->waitMs(10);
	_vm->_displayMan->f98_drawFloorAndCeiling();
	_vm->_eventMan->_g441_primaryMouseInput = g447_PrimaryMouseInput_Interface;
	_vm->_eventMan->_g442_secondaryMouseInput = g448_SecondaryMouseInput_Movement;
	warning("MISSING CODE: set G0443_ps_PrimaryKeyboardInput");
	warning("MISSING CODE: G0444_ps_SecondaryKeyboardInput");
	_vm->_eventMan->f357_discardAllInput();
	_vm->_menuMan->f457_drawEnabledMenus();
}

int16 ChampionMan::f305_getThrowingStaminaCost(Thing thing) {
	int16 L0923_i_Weight;
	int16 L0924_i_StaminaCost;


	L0924_i_StaminaCost = f26_getBoundedValue((int16)1, L0923_i_Weight = _vm->_dungeonMan->f140_getObjectWeight(thing) >> 1, (int16)10);
	while ((L0923_i_Weight -= 10) > 0) {
		L0924_i_StaminaCost += L0923_i_Weight >> 1;
	}
	return L0924_i_StaminaCost;
}

void ChampionMan::f330_disableAction(uint16 champIndex, uint16 ticks) {
	int32 L1001_l_UpdatedEnableActionEventTime;
	int32 L1002_l_CurrentEnableActionEventTime;
	int16 L1003_i_EventIndex;
	Champion* L1004_ps_Champion;
	TimelineEvent L1005_s_Event;


	L1004_ps_Champion = &_gK71_champions[champIndex];
	L1001_l_UpdatedEnableActionEventTime = _vm->_g313_gameTime + ticks;
	L1005_s_Event._type = k11_TMEventTypeEnableChampionAction;
	L1005_s_Event._priority = champIndex;
	L1005_s_Event._B._slotOrdinal = 0;
	if ((L1003_i_EventIndex = L1004_ps_Champion->_enableActionEventIndex) >= 0) {
		L1002_l_CurrentEnableActionEventTime = M30_time(_vm->_timeline->_g370_events[L1003_i_EventIndex]._mapTime);
		if (L1001_l_UpdatedEnableActionEventTime >= L1002_l_CurrentEnableActionEventTime) {
			L1001_l_UpdatedEnableActionEventTime += (L1002_l_CurrentEnableActionEventTime - _vm->_g313_gameTime) >> 1;
		} else {
			L1001_l_UpdatedEnableActionEventTime = L1002_l_CurrentEnableActionEventTime + (ticks >> 1);
		}
		_vm->_timeline->f237_deleteEvent(L1003_i_EventIndex);
	} else {
		setFlag(L1004_ps_Champion->_attributes, k0x8000_ChampionAttributeActionHand | k0x0008_ChampionAttributeDisableAction);
		f292_drawChampionState((ChampionIndex)champIndex);
	}
	M33_setMapAndTime(L1005_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, L1001_l_UpdatedEnableActionEventTime);
	L1004_ps_Champion->_enableActionEventIndex = _vm->_timeline->f238_addEventGetEventIndex(&L1005_s_Event);
}

void ChampionMan::f304_addSkillExperience(uint16 champIndex, uint16 skillIndex, uint16 exp) {
#define AP0638_ui_SkillLevelAfter exp
#define AP0638_ui_ChampionColor   exp
	uint16 L0915_ui_Multiple;
#define AL0915_ui_MapDifficulty    L0915_ui_Multiple
#define AL0915_ui_SkillLevelBefore L0915_ui_Multiple
#define AL0915_ui_VitalityAmount   L0915_ui_Multiple
#define AL0915_ui_StaminaAmount    L0915_ui_Multiple
	uint16 L0916_ui_BaseSkillIndex;
	Skill* L0918_ps_Skill;
	Champion* L0919_ps_Champion;
	int16 L0920_i_MinorStatisticIncrease;
	int16 L0921_i_MajorStatisticIncrease;
	int16 L0922_i_BaseSkillLevel;


	warning("potaneitally dangerous cast of uint32 below");
	if ((skillIndex >= k4_ChampionSkillSwing) && (skillIndex <= k11_ChampionSkillShoot) && ((uint32)_vm->_projexpl->_g361_lastCreatureAttackTime < (_vm->_g313_gameTime - 150))) {
		exp >>= 1;
	}
	if (exp) {
		if (AL0915_ui_MapDifficulty = _vm->_dungeonMan->_g269_currMap->_difficulty) {
			exp *= AL0915_ui_MapDifficulty;
		}
		L0919_ps_Champion = &_gK71_champions[champIndex];
		if (skillIndex >= k4_ChampionSkillSwing) {
			L0916_ui_BaseSkillIndex = (skillIndex - k4_ChampionSkillSwing) >> 2;
		} else {
			L0916_ui_BaseSkillIndex = skillIndex;
		}
		AL0915_ui_SkillLevelBefore = f303_getSkillLevel(champIndex, L0916_ui_BaseSkillIndex | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience));
		warning("potentially dangerous cast of uint32 below");
		if ((skillIndex >= k4_ChampionSkillSwing) && ((uint32)_vm->_projexpl->_g361_lastCreatureAttackTime > (_vm->_g313_gameTime - 25))) {
			exp <<= 1;
		}
		L0918_ps_Skill = &L0919_ps_Champion->_skills[skillIndex];
		L0918_ps_Skill->_experience += exp;
		if (L0918_ps_Skill->_temporaryExperience < 32000) {
			L0918_ps_Skill->_temporaryExperience += f26_getBoundedValue(1, exp >> 3, 100);
		}
		L0918_ps_Skill = &L0919_ps_Champion->_skills[L0916_ui_BaseSkillIndex];
		if (skillIndex >= k4_ChampionSkillSwing) {
			L0918_ps_Skill->_experience += exp;
		}
		AP0638_ui_SkillLevelAfter = f303_getSkillLevel(champIndex, L0916_ui_BaseSkillIndex | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience));
		if (AP0638_ui_SkillLevelAfter > AL0915_ui_SkillLevelBefore) {
			L0922_i_BaseSkillLevel = AP0638_ui_SkillLevelAfter;
			L0920_i_MinorStatisticIncrease = _vm->getRandomNumber(2);
			L0921_i_MajorStatisticIncrease = 1 + _vm->getRandomNumber(2);
			AL0915_ui_VitalityAmount = _vm->getRandomNumber(2); /* For Priest skill, the amount is 0 or 1 for all skill levels */
			if (L0916_ui_BaseSkillIndex != k2_ChampionSkillPriest) {
				AL0915_ui_VitalityAmount &= AP0638_ui_SkillLevelAfter; /* For non Priest skills the amount is 0 for even skill levels. The amount is 0 or 1 for odd skill levels */
			}
			L0919_ps_Champion->_statistics[k4_ChampionStatVitality][k0_ChampionStatMaximum] += AL0915_ui_VitalityAmount;
			AL0915_ui_StaminaAmount = L0919_ps_Champion->_maxStamina;
			L0919_ps_Champion->_statistics[k6_ChampionStatAntifire][k0_ChampionStatMaximum] += _vm->getRandomNumber(2) & ~AP0638_ui_SkillLevelAfter; /* The amount is 0 for odd skill levels. The amount is 0 or 1 for even skill levels */
			switch (L0916_ui_BaseSkillIndex) {
			case k0_ChampionSkillFighter:
				AL0915_ui_StaminaAmount >>= 4;
				AP0638_ui_SkillLevelAfter *= 3;
				L0919_ps_Champion->_statistics[k1_ChampionStatStrength][k0_ChampionStatMaximum] += L0921_i_MajorStatisticIncrease;
				L0919_ps_Champion->_statistics[k2_ChampionStatDexterity][k0_ChampionStatMaximum] += L0920_i_MinorStatisticIncrease;
				break;
			case k1_ChampionSkillNinja:
				AL0915_ui_StaminaAmount /= 21;
				AP0638_ui_SkillLevelAfter <<= 1;
				L0919_ps_Champion->_statistics[k1_ChampionStatStrength][k0_ChampionStatMaximum] += L0920_i_MinorStatisticIncrease;
				L0919_ps_Champion->_statistics[k2_ChampionStatDexterity][k0_ChampionStatMaximum] += L0921_i_MajorStatisticIncrease;
				break;
			case k3_ChampionSkillWizard:
				AL0915_ui_StaminaAmount >>= 5;
				L0919_ps_Champion->_maxMana += AP0638_ui_SkillLevelAfter + (AP0638_ui_SkillLevelAfter >> 1);
				L0919_ps_Champion->_statistics[k3_ChampionStatWisdom][k0_ChampionStatMaximum] += L0921_i_MajorStatisticIncrease;
				goto T0304016;
			case k2_ChampionSkillPriest:
				AL0915_ui_StaminaAmount /= 25;
				L0919_ps_Champion->_maxMana += AP0638_ui_SkillLevelAfter;
				AP0638_ui_SkillLevelAfter += (AP0638_ui_SkillLevelAfter + 1) >> 1;
				L0919_ps_Champion->_statistics[k3_ChampionStatWisdom][k0_ChampionStatMaximum] += L0920_i_MinorStatisticIncrease;
T0304016:
				if ((L0919_ps_Champion->_maxMana += MIN(_vm->getRandomNumber(4), (uint16)(L0922_i_BaseSkillLevel - 1))) > 900) {
					L0919_ps_Champion->_maxMana = 900;
				}
				L0919_ps_Champion->_statistics[k5_ChampionStatAntimagic][k0_ChampionStatMaximum] += _vm->getRandomNumber(3);
			}
			if ((L0919_ps_Champion->_maxHealth += AP0638_ui_SkillLevelAfter + _vm->getRandomNumber((AP0638_ui_SkillLevelAfter >> 1) + 1)) > 999) {
				L0919_ps_Champion->_maxHealth = 999;
			}
			if ((L0919_ps_Champion->_maxStamina += AL0915_ui_StaminaAmount + _vm->getRandomNumber((AL0915_ui_StaminaAmount >> 1) + 1)) > 9999) {
				L0919_ps_Champion->_maxStamina = 9999;
			}
			setFlag(L0919_ps_Champion->_attributes, k0x0100_ChampionAttributeStatistics);
			f292_drawChampionState((ChampionIndex)champIndex);
			_vm->_textMan->f51_messageAreaPrintLineFeed();
			_vm->_textMan->f47_messageAreaPrintMessage((Color)(AP0638_ui_ChampionColor = g46_ChampionColor[champIndex]), L0919_ps_Champion->_name);
			// TODO: localization
			_vm->_textMan->f47_messageAreaPrintMessage((Color)AP0638_ui_ChampionColor, " JUST GAINED A ");
			_vm->_textMan->f47_messageAreaPrintMessage((Color)AP0638_ui_ChampionColor, g417_baseSkillName[L0916_ui_BaseSkillIndex]);
			_vm->_textMan->f47_messageAreaPrintMessage((Color)AP0638_ui_ChampionColor, " LEVEL!");
		}
	}
}

int16 ChampionMan::f324_damageAll_getDamagedChampionCount(uint16 attack, int16 wounds, int16 attackType) {
	int16 L0984_i_ChampionIndex;
	int16 L0985_i_RandomAttack;
	int16 L0986_i_DamagedChampionCount;

	attack -= (L0985_i_RandomAttack = (attack >> 3) + 1);
	L0985_i_RandomAttack <<= 1;
	for (L0986_i_DamagedChampionCount = 0, L0984_i_ChampionIndex = k0_ChampionFirst; L0984_i_ChampionIndex < _g305_partyChampionCount; L0984_i_ChampionIndex++) {
		if (f321_addPendingDamageAndWounds_getDamage(L0984_i_ChampionIndex, MAX(1, attack + _vm->getRandomNumber(L0985_i_RandomAttack)), wounds, attackType)) { /* Actual attack is attack +/- (attack / 8) */
			L0986_i_DamagedChampionCount++;
		}
	}
	return L0986_i_DamagedChampionCount;
}

int16 ChampionMan::f286_getTargetChampionIndex(int16 mapX, int16 mapY, uint16 cell) {
	uint16 L0838_ui_Counter;
	int16 L0839_i_ChampionIndex;
	signed char L0840_auc_OrderedCellsToAttack[4];


	if (_g305_partyChampionCount && (M38_distance(mapX, mapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY) <= 1)) {
		_vm->_groupMan->f229_setOrderedCellsToAttack(L0840_auc_OrderedCellsToAttack, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, mapX, mapY, cell);
		for (L0838_ui_Counter = 0; L0838_ui_Counter < 4; L0838_ui_Counter++) {
			if ((L0839_i_ChampionIndex = f285_getIndexInCell(L0840_auc_OrderedCellsToAttack[L0838_ui_Counter])) >= 0) {
				return L0839_i_ChampionIndex;
			}
		}
	}
	return kM1_ChampionNone;
}

int16 ChampionMan::f311_getDexterity(Champion* champ) {
	int16 L0934_i_Dexterity;


	L0934_i_Dexterity = _vm->getRandomNumber(8) + champ->_statistics[k2_ChampionStatDexterity][k1_ChampionStatCurrent];
	L0934_i_Dexterity -= ((int32)(L0934_i_Dexterity >> 1) * (int32)champ->_load) / f309_getMaximumLoad(champ);
	if (_g300_partyIsSleeping) {
		L0934_i_Dexterity >>= 1;
	}
	return f26_getBoundedValue(1 + _vm->getRandomNumber(8), L0934_i_Dexterity >> 1, 100 - _vm->getRandomNumber(8));
}

bool ChampionMan::f308_isLucky(Champion* champ, uint16 percentage) {
#define AP0646_ui_IsLucky percentage
	register unsigned char* L0928_puc_Statistic;


	if (_vm->getRandomNumber(2) && (_vm->getRandomNumber(100) > percentage)) {
		return true;
	}
	L0928_puc_Statistic = champ->_statistics[k0_ChampionStatLuck];
	AP0646_ui_IsLucky = (_vm->getRandomNumber(L0928_puc_Statistic[k1_ChampionStatCurrent]) > percentage);
	L0928_puc_Statistic[k1_ChampionStatCurrent] = f26_getBoundedValue((int32)L0928_puc_Statistic[k2_ChampionStatMinimum], (int32)L0928_puc_Statistic[k1_ChampionStatCurrent] + (AP0646_ui_IsLucky ? -2 : 2), (int32)L0928_puc_Statistic[k0_ChampionStatMaximum]);
	return AP0646_ui_IsLucky;
}

void ChampionMan::f322_championPoison(int16 champIndex, uint16 attack) {
	TimelineEvent L0980_s_Event;
	Champion* L0981_ps_Champion;


	if ((champIndex == kM1_ChampionNone) || (_vm->M0_indexToOrdinal(champIndex) == _g299_candidateChampionOrdinal)) {
		return;
	}
	L0981_ps_Champion = &_gK71_champions[champIndex];
	f321_addPendingDamageAndWounds_getDamage(champIndex, MAX(1, attack >> 6), k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
	setFlag(L0981_ps_Champion->_attributes, k0x0100_ChampionAttributeStatistics);
	if ((_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) && (_vm->_inventoryMan->_g424_panelContent == k0_PanelContentFoodWaterPoisoned)) {
		setFlag(L0981_ps_Champion->_attributes, k0x0800_ChampionAttributePanel);
	}
	if (--attack) {
		L0981_ps_Champion->_poisonEventCount++;
		L0980_s_Event._type = k75_TMEventTypePoisonChampion;
		L0980_s_Event._priority = champIndex;
		M33_setMapAndTime(L0980_s_Event._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + 36);
		L0980_s_Event._B._attack = attack;
		_vm->_timeline->f238_addEventGetEventIndex(&L0980_s_Event);
	}
	f292_drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::f284_setPartyDirection(int16 dir) {
	int16 L0833_i_ChampionIndex;
	int16 L0834_i_Delta;
	Champion* L0835_ps_Champion;


	if (dir == _vm->_dungeonMan->_g308_partyDir) {
		return;
	}
	if ((L0834_i_Delta = dir - _vm->_dungeonMan->_g308_partyDir) < 0) {
		L0834_i_Delta += 4;
	}
	L0835_ps_Champion = _gK71_champions;
	for (L0833_i_ChampionIndex = k0_ChampionFirst; L0833_i_ChampionIndex < _g305_partyChampionCount; L0833_i_ChampionIndex++) {
		L0835_ps_Champion->_cell = (ViewCell)M21_normalizeModulo4(L0835_ps_Champion->_cell + L0834_i_Delta);
		L0835_ps_Champion->_dir = (direction)M21_normalizeModulo4(L0835_ps_Champion->_dir + L0834_i_Delta);
		L0835_ps_Champion++;
	}
	_vm->_dungeonMan->_g308_partyDir = (direction)dir;
	f296_drawChangedObjectIcons();
}

void ChampionMan::f316_deleteScent(uint16 scentIndex) {
	uint16 L0953_ui_Count;

	if (L0953_ui_Count = --_g407_party._scentCount - scentIndex) {
		for (uint16 i = 0; i < L0953_ui_Count; ++i) {
			_g407_party._scents[scentIndex + i] = _g407_party._scents[scentIndex + i + 1];
			_g407_party._scentStrengths[scentIndex + i] = _g407_party._scentStrengths[scentIndex + i + 1];
		}
	}
	if (scentIndex < _g407_party._firstScentIndex) {
		_g407_party._firstScentIndex--;
	}
	if (scentIndex < _g407_party._lastScentIndex) {
		_g407_party._lastScentIndex--;
	}
}

void ChampionMan::f317_addScentStrength(int16 mapX, int16 mapY, int32 cycleCount) {
	int16 L0954_i_ScentIndex;
	bool L0955_B_Merge;
	bool L0956_B_CycleCountDefined;
	Scent* L0957_ps_Scent; /* BUG0_00 Useless code */
	Scent L0958_s_Scent; /* BUG0_00 Useless code */


	if (L0954_i_ScentIndex = _vm->_championMan->_g407_party._scentCount) {
		if (L0955_B_Merge = getFlag(cycleCount, k0x8000_mergeCycles)) {
			clearFlag(cycleCount, k0x8000_mergeCycles);
		}
		L0958_s_Scent.setMapX(mapX); /* BUG0_00 Useless code */
		L0958_s_Scent.setMapY(mapY); /* BUG0_00 Useless code */
		L0958_s_Scent.setMapIndex(_vm->_dungeonMan->_g272_currMapIndex); /* BUG0_00 Useless code */
		L0957_ps_Scent = _vm->_championMan->_g407_party._scents; /* BUG0_00 Useless code */
		L0956_B_CycleCountDefined = false;
		while (L0954_i_ScentIndex--) {
			if (&*L0957_ps_Scent++ == &L0958_s_Scent) {
				if (!L0956_B_CycleCountDefined) {
					L0956_B_CycleCountDefined = true;
					if (L0955_B_Merge) {
						cycleCount = MAX((int32)_vm->_championMan->_g407_party._scentStrengths[L0954_i_ScentIndex], cycleCount);
					} else {
						cycleCount = MIN(80, _vm->_championMan->_g407_party._scentStrengths[L0954_i_ScentIndex] + cycleCount);
					}
				}
				_vm->_championMan->_g407_party._scentStrengths[L0954_i_ScentIndex] = cycleCount;
			}
		}
	}
}

void ChampionMan::f297_putObjectInLeaderHand(Thing thing, bool setMousePointer) {
	if (thing == Thing::_none) {
		return;
	}
	_vm->_championMan->_g415_leaderEmptyHanded = false;
	_vm->_objectMan->f36_extractIconFromBitmap(_vm->_championMan->_g413_leaderHandObjectIconIndex = _vm->_objectMan->f33_getIconIndex(_vm->_championMan->_g414_leaderHandObject = thing), _vm->_objectMan->_g412_objectIconForMousePointer);
	_vm->_eventMan->f78_showMouse();
	_vm->_objectMan->f34_drawLeaderObjectName(thing);
	if (setMousePointer) {
		_vm->_g325_setMousePointerToObjectInMainLoop = true;
	} else {
		_vm->_eventMan->f68_setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);
	}
	_vm->_eventMan->f77_hideMouse();
	if (_vm->_championMan->_g411_leaderIndex != kM1_ChampionNone) {
		_vm->_championMan->_gK71_champions[_vm->_championMan->_g411_leaderIndex]._load += _vm->_dungeonMan->f140_getObjectWeight(thing);
		setFlag(_vm->_championMan->_gK71_champions[_vm->_championMan->_g411_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
		_vm->_championMan->f292_drawChampionState(_vm->_championMan->_g411_leaderIndex);
	}
}

ChampionIndex ChampionMan::f285_getIndexInCell(int16 cell) {
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
	_vm->_eventMan->f78_showMouse();

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
	_vm->_eventMan->f77_hideMouse();
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
	_vm->_eventMan->f78_showMouse();
}

uint16 ChampionMan::M26_championIconIndex(int16 val, direction dir) {
	return ((val + 4 - dir) & 0x3);
}

void ChampionMan::f290_drawHealthStaminaManaValues(Champion* champ) {
	f289_drawHealthOrStaminaOrManaValue(116, champ->_currHealth, champ->_maxHealth);
	f289_drawHealthOrStaminaOrManaValue(124, champ->_currStamina, champ->_maxStamina);
	f289_drawHealthOrStaminaOrManaValue(132, champ->_currMana, champ->_maxMana);
}

void ChampionMan::f291_drawSlot(uint16 champIndex, int16 slotIndex) {
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
		thing = champ->getSlot((ChampionSlot)slotIndex);
	}

	SlotBox *slotBox = &_vm->_objectMan->_g30_slotBoxes[slotBoxIndex];
	Box box;
	box._x1 = slotBox->_x - 1;
	box._y1 = slotBox->_y - 1;
	box._x2 = box._x1 + 17;
	box._y2 = box._y1 + 17;


	if (!isInventoryChamp) {
		_vm->_eventMan->f77_hideMouse();
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
		iconIndex = _vm->_objectMan->f33_getIconIndex(thing); // BUG0_35
		if (isInventoryChamp && (slotIndex == k1_ChampionSlotActionHand) && ((iconIndex == k144_IconIndiceContainerChestClosed) || (iconIndex == k30_IconIndiceScrollOpen))) {
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
		_vm->_eventMan->f78_showMouse();
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

uint16 ChampionMan::f303_getSkillLevel(int16 champIndex, int16 skillIndex) {
	if (_g300_partyIsSleeping)
		return 1;

	bool ignoreTempExp = skillIndex & k0x8000_IgnoreTemporaryExperience;
	bool ignoreObjModifiers = skillIndex & k0x4000_IgnoreObjectModifiers;
	skillIndex = (ChampionSkill)(skillIndex & ~(ignoreTempExp | ignoreObjModifiers));
	Champion *champ = &_gK71_champions[champIndex];
	Skill *skill = &champ->getSkill((ChampionSkill)skillIndex);
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


