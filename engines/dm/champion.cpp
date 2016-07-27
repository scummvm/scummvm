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
#include "movesens.h"


namespace DM {

uint16 g38_slotMasks[38] = { // @ G0038_ai_Graphic562_SlotMasks
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
	for (uint16 i = 0; i < 4; ++i) {
		_g409_championPendingDamage[i] = 0;
		_g410_championPendingWounds[i] = 0;
		_gK71_champions[i].resetToZero();
	}
	_g305_partyChampionCount = 0;
	_g303_partyDead = false;
	_g414_leaderHandObject = Thing(0);
	_g411_leaderIndex = kM1_ChampionNone;
	_g300_partyIsSleeping = false;
	_g506_actingChampionOrdinal = 0;
	_g413_leaderHandObjectIconIndex = (IconIndice)0;
	_g415_leaderEmptyHanded = false;
	_g407_party.resetToZero();
	_g514_magicCasterChampionIndex = kM1_ChampionNone;
	_g420_mousePointerHiddenToDrawChangedObjIconOnScreen = false;
}

bool ChampionMan::f329_isLeaderHandObjectThrown(int16 side) {
	if (_g411_leaderIndex == kM1_ChampionNone) {
		return false;
	}
	return f328_isObjectThrown(_g411_leaderIndex, kM1_ChampionSlotLeaderHand, side);
}

bool ChampionMan::f328_isObjectThrown(uint16 champIndex, int16 slotIndex, int16 side) {
	bool throwingLeaderHandObjectFl = false;
	Thing curThing;
	Champion *curChampion = nullptr;
	Thing actionHandThing;

	if (slotIndex < 0) { /* Throw object in leader hand, which is temporarily placed in action hand */
		if (_g415_leaderEmptyHanded)
			return false;

		curThing = f298_getObjectRemovedFromLeaderHand();
		curChampion = &_gK71_champions[champIndex];
		actionHandThing = curChampion->getSlot(k1_ChampionSlotActionHand);
		curChampion->setSlot(k1_ChampionSlotActionHand, curThing);
		slotIndex = k1_ChampionSlotActionHand;
		throwingLeaderHandObjectFl = true;
	}

	int16 kineticEnergy = f312_getStrength(champIndex, slotIndex);
	if (throwingLeaderHandObjectFl) {
		// In this case, curChampion and actionHandThing are set.
		curChampion->setSlot((ChampionSlot)slotIndex, actionHandThing);
	} else {
		curThing = f300_getObjectRemovedFromSlot(champIndex, slotIndex);
		if (curThing == Thing::_none)
			return false;
	}

	_vm->f064_SOUND_RequestPlay_CPSD(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, k1_soundModePlayIfPrioritized);
	f325_decrementStamina(champIndex, f305_getThrowingStaminaCost(curThing));
	f330_disableAction(champIndex, 4);
	int16 experience = 8;
	int16 weaponKineticEnergy = 1;
	if (curThing.getType() == k5_WeaponThingType) {
		experience += 4;
		WeaponInfo *curWeapon = _vm->_dungeonMan->f158_getWeaponInfo(curThing);
		if (curWeapon->_class <= k12_WeaponClassPoisinDart) {
			weaponKineticEnergy = curWeapon->_kineticEnergy;
			experience += weaponKineticEnergy >> 2;
		}
	}
	f304_addSkillExperience(champIndex, k10_ChampionSkillThrow, experience);
	kineticEnergy += weaponKineticEnergy;
	int16 skillLevel = f303_getSkillLevel((ChampionIndex)champIndex, k10_ChampionSkillThrow);
	kineticEnergy += _vm->getRandomNumber(16) + (kineticEnergy >> 1) + skillLevel;
	int16 attack = f26_getBoundedValue((uint16)40, (uint16)((skillLevel << 3) + _vm->getRandomNumber(32)), (uint16)200);
	int16 stepEnergy = MAX(5, 11 - skillLevel);
	_vm->_projexpl->f212_projectileCreate(curThing, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY,
										  M21_normalizeModulo4(_vm->_dungeonMan->_g308_partyDir + side),
										  _vm->_dungeonMan->_g308_partyDir, kineticEnergy, attack, stepEnergy);
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
	Common::String tmp = f288_getStringFromInteger(currVal, true, 3);
	_vm->_textMan->f52_printToViewport(55, posY, k13_ColorLightestGray, tmp.c_str());
	_vm->_textMan->f52_printToViewport(73, posY, k13_ColorLightestGray, "/");
	tmp = f288_getStringFromInteger(maxVal, true, 3);
	_vm->_textMan->f52_printToViewport(79, posY, k13_ColorLightestGray, tmp.c_str());
}

uint16 ChampionMan::M70_handSlotIndex(uint16 slotBoxIndex) {
	return slotBoxIndex & 0x1;
}

Common::String ChampionMan::f288_getStringFromInteger(uint16 val, bool padding, uint16 paddingCharCount) {
	Common::String valToStr = Common::String::format("%d", val);
	Common::String result;

	if (padding) {
		for (int16 i = 0, end = paddingCharCount - valToStr.size(); i < end; ++i)
			result += ' ';
	}

	return result += valToStr;
}

void ChampionMan::f299_applyModifiersToStatistics(Champion *champ, int16 slotIndex, int16 iconIndex, int16 modifierFactor, Thing thing) {
	int16 statIndex = k0_ChampionStatLuck;
	int16 modifier = 0;
	ThingType thingType = thing.getType();

	bool cursed = false;
	if (   ((thingType == k5_WeaponThingType) || (thingType == k6_ArmourThingType))
		&& (slotIndex >= k0_ChampionSlotReadyHand) && (slotIndex <= k12_ChampionSlotQuiverLine_1_1)) {
		if (thingType == k5_WeaponThingType) {
			Weapon *weapon = (Weapon *)_vm->_dungeonMan->f156_getThingData(thing);
			cursed = weapon->getCursed();
		} else {
			// k6_ArmourThingType
			Armour *armour = (Armour *)_vm->_dungeonMan->f156_getThingData(thing);
			cursed = armour->getCursed();
		}

		if (cursed) {
			statIndex = k0_ChampionStatLuck;
			modifier = -3;
		}
	}

	if (!cursed) {
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
					default:
						break;
					}
				}
			}
		} else if (slotIndex == k4_ChampionSlotLegs) {
			if (iconIndex == k142_IconIndiceArmourPowertowers) {
				statIndex = k1_ChampionStatStrength;
				modifier = 10;
			}
		} else if (slotIndex == k2_ChampionSlotHead) {
			switch (iconIndex) {
			case k104_IconIndiceArmourCrownOfNerra:
				statIndex = k3_ChampionStatWisdom;
				modifier = 10;
				break;
			case k140_IconIndiceArmourDexhelm:
				statIndex = k2_ChampionStatDexterity;
				modifier = 10;
				break;
			default:
				break;
			}
		} else if (slotIndex == k3_ChampionSlotTorso) {
			switch (iconIndex) {
			case k141_IconIndiceArmourFlamebain:
				statIndex = k6_ChampionStatAntifire;
				modifier = 12;
				break;
			case k81_IconIndiceArmourCloakOfNight:
				statIndex = k2_ChampionStatDexterity;
				modifier = 8;
				break;
			default:
				break;
			}
		} else if (slotIndex == k10_ChampionSlotNeck) {
			switch (iconIndex) {
			case k10_IconIndiceJunkJewelSymalUnequipped:
			case k11_IconIndiceJunkJewelSymalEquipped:
				statIndex = k5_ChampionStatAntimagic;
				modifier = 15;
				break;
			case k81_IconIndiceArmourCloakOfNight:
				statIndex = k2_ChampionStatDexterity;
				modifier = 8;
				break;
			case k122_IconIndiceJunkMoonstone:
				statIndex = k8_ChampionStatMana;
				modifier = 3;
				break;
			default:
				break;
			}
		}
	}

	if (modifier) {
		modifier *= modifierFactor;
		//statIndex is set when modifier is set
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
				((Scroll *)rawObjPtr)->setClosed(false);
				f296_drawChangedObjectIcons();
			}
		}

		if (iconIndex = k4_IconIndiceWeaponTorchUnlit) {
			((Weapon *)rawObjPtr)->setLit(true);
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			f296_drawChangedObjectIcons();
		} else if (isInventoryChampion && (slotIndex == k1_ChampionSlotActionHand) &&
			((iconIndex == k144_IconIndiceContainerChestClosed) || ((iconIndex >= k30_IconIndiceScrollOpen) && (iconIndex <= k31_IconIndiceScrollClosed)))) {
			champ->setAttributeFlag(k0x0800_ChampionAttributePanel, true);
		}
	} else if (slotIndex == k10_ChampionSlotNeck) {
		if ((iconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (iconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk *)rawObjPtr)->setChargeCount(1);
			_g407_party._magicalLightAmount += g39_LightPowerToLightAmount[2];
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			iconIndex = (IconIndice)(iconIndex + 1);
		} else if ((iconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (iconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
			((Junk *)rawObjPtr)->setChargeCount(1);
			iconIndex = (IconIndice)(iconIndex + 1);
		}
	}

	f291_drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		champ->setAttributeFlag(k0x4000_ChampionAttributeViewport, true);
}

int16 ChampionMan::f315_getScentOrdinal(int16 mapX, int16 mapY) {
	int16 scentIndex = _g407_party._scentCount;

	if (scentIndex) {
		Scent searchedScent;
		searchedScent.setMapX(mapX);
		searchedScent.setMapY(mapY);
		searchedScent.setMapIndex(_vm->_dungeonMan->_g272_currMapIndex);
		uint16 searchedScentRedEagle = searchedScent.toUint16();
		Scent *scent = &_g407_party._scents[scentIndex--];
		do {
			if ((*(--scent)).toUint16() == searchedScentRedEagle) {
				return _vm->M0_indexToOrdinal(scentIndex);
			}
		} while (scentIndex--);
	}
	return 0;
}

Thing ChampionMan::f298_getObjectRemovedFromLeaderHand() {
	_g415_leaderEmptyHanded = true;
	Thing leaderHandObject = _g414_leaderHandObject;

	if (leaderHandObject != Thing::_none) {
		_g414_leaderHandObject = Thing::_none;
		_g413_leaderHandObjectIconIndex = kM1_IconIndiceNone;
		_vm->_eventMan->f78_showMouse();
		_vm->_objectMan->f35_clearLeaderObjectName();
		_vm->_eventMan->f69_setMousePointer();
		_vm->_eventMan->f77_hideMouse();
		if (_g411_leaderIndex != kM1_ChampionNone) {
			_gK71_champions[_g411_leaderIndex]._load -= _vm->_dungeonMan->f140_getObjectWeight(leaderHandObject);
			setFlag(_gK71_champions[_g411_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
			f292_drawChampionState(_g411_leaderIndex);
		}
	}
	return leaderHandObject;
}

uint16 ChampionMan::f312_getStrength(int16 champIndex, int16 slotIndex) {
	Champion *curChampion = &_gK71_champions[champIndex];
	int16 strength = _vm->getRandomNumber(16) + curChampion->_statistics[k1_ChampionStatStrength][k1_ChampionStatCurrent];
	Thing curThing = curChampion->_slots[slotIndex];
	uint16 objectWeight = _vm->_dungeonMan->f140_getObjectWeight(curThing);
	uint16 oneSixteenthMaximumLoad = f309_getMaximumLoad(curChampion) >> 4;

	if (objectWeight <= oneSixteenthMaximumLoad) {
		strength += objectWeight - 12;
	} else {
		int16 loadThreshold = oneSixteenthMaximumLoad + ((oneSixteenthMaximumLoad - 12) >> 1);
		if (objectWeight <= loadThreshold) {
			strength += (objectWeight - oneSixteenthMaximumLoad) >> 1;
		} else {
			strength -= (objectWeight - loadThreshold) << 1;
		}
	}
	if (curThing.getType() == k5_WeaponThingType) {
		WeaponInfo *weaponInfo = _vm->_dungeonMan->f158_getWeaponInfo(curThing);
		strength += weaponInfo->_strength;
		uint16 skillLevel = 0;
		uint16 weaponClass = weaponInfo->_class;
		if ((weaponClass == k0_WeaponClassSwingWeapon) || (weaponClass == k2_WeaponClassDaggerAndAxes)) {
			skillLevel = f303_getSkillLevel(champIndex, k4_ChampionSkillSwing);
		}
		if ((weaponClass != k0_WeaponClassSwingWeapon) && (weaponClass < k16_WeaponClassFirstBow)) {
			skillLevel += f303_getSkillLevel(champIndex, k10_ChampionSkillThrow);
		}
		if ((weaponClass >= k16_WeaponClassFirstBow) && (weaponClass < k112_WeaponClassFirstMagicWeapon)) {
			skillLevel += f303_getSkillLevel(champIndex, k11_ChampionSkillShoot);
		}
		strength += skillLevel << 1;
	}
	strength = f306_getStaminaAdjustedValue(curChampion, strength);
	if (getFlag(curChampion->_wounds, (slotIndex == k0_ChampionSlotReadyHand) ? k0x0001_ChampionWoundReadHand : k0x0002_ChampionWoundActionHand)) {
		strength >>= 1;
	}
	return f26_getBoundedValue(0, strength >> 1, 100);
}

Thing ChampionMan::f300_getObjectRemovedFromSlot(uint16 champIndex, uint16 slotIndex) {
	Champion *curChampion = &_gK71_champions[champIndex];
	Thing curThing;

	if (slotIndex >= k30_ChampionSlotChest_1) {
		curThing = _vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1];
		_vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1] = Thing::_none;
	} else {
		curThing = curChampion->_slots[slotIndex];
		curChampion->_slots[slotIndex] = Thing::_none;
	}

	if (curThing == Thing::_none)
		return Thing::_none;

	bool isInventoryChampion = (_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal);
	int16 curIconIndex = _vm->_objectMan->f33_getIconIndex(curThing);
	// Remove object modifiers
	f299_applyModifiersToStatistics(curChampion, slotIndex, curIconIndex, -1, curThing);

	Weapon *curWeapon = (Weapon *)_vm->_dungeonMan->f156_getThingData(curThing);
	if (slotIndex == k10_ChampionSlotNeck) {
		if ((curIconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (curIconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk *)curWeapon)->setChargeCount(0);
			_g407_party._magicalLightAmount -= g39_LightPowerToLightAmount[2];
			_vm->_inventoryMan->f337_setDungeonViewPalette();
		} else if ((curIconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (curIconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
			((Junk *)curWeapon)->setChargeCount(0);
		}
	}

	f291_drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		setFlag(curChampion->_attributes, k0x4000_ChampionAttributeViewport);

	if (slotIndex < k2_ChampionSlotHead) {
		if (slotIndex == k1_ChampionSlotActionHand) {
			setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand);
			if (_g506_actingChampionOrdinal == _vm->M0_indexToOrdinal(champIndex))
				_vm->_menuMan->f388_clearActingChampion();

			if ((curIconIndex >= k30_IconIndiceScrollOpen) && (curIconIndex <= k31_IconIndiceScrollClosed)) {
				((Scroll *)curWeapon)->setClosed(true);
				f296_drawChangedObjectIcons();
			}
		}

		if ((curIconIndex >= k4_IconIndiceWeaponTorchUnlit) && (curIconIndex <= k7_IconIndiceWeaponTorchLit)) {
			curWeapon->setLit(false);
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			f296_drawChangedObjectIcons();
		}

		if (isInventoryChampion && (slotIndex == k1_ChampionSlotActionHand)) {
			switch (curIconIndex) {
			case k144_IconIndiceContainerChestClosed:
				_vm->_inventoryMan->f334_closeChest();
			// No break on purpose
			case k30_IconIndiceScrollOpen:
			case k31_IconIndiceScrollClosed:
				setFlag(curChampion->_attributes, k0x0800_ChampionAttributePanel);
				break;
			default:
				break;
			}
		}
	}
	curChampion->_load -= _vm->_dungeonMan->f140_getObjectWeight(curThing);
	setFlag(curChampion->_attributes, k0x0200_ChampionAttributeLoad);
	return curThing;
}

void ChampionMan::f325_decrementStamina(int16 championIndex, int16 decrement) {
	if (championIndex == kM1_ChampionNone)
		return;

	Champion *curChampion = &_gK71_champions[championIndex];
	curChampion->_currStamina -= decrement;

	int16 stamina = curChampion->_currStamina;
	if (stamina <= 0) {
		curChampion->_currStamina = 0;
		f321_addPendingDamageAndWounds_getDamage(championIndex, (-stamina) >> 1, k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
	} else if (stamina > curChampion->_maxStamina) {
		curChampion->_currStamina = curChampion->_maxStamina;
	}

	setFlag(curChampion->_attributes, k0x0200_ChampionAttributeLoad | k0x0100_ChampionAttributeStatistics);
}

int16 ChampionMan::f321_addPendingDamageAndWounds_getDamage(int16 champIndex, int16 attack, int16 allowedWounds, uint16 attackType) {
	if (attack <= 0)
		return 0;

	Champion *curChampion = &_gK71_champions[champIndex];
	if (!curChampion->_currHealth)
		return 0;

	bool skipScaling = false;
	if (attackType != k0_attackType_NORMAL) {
		uint16 defense = 0;
		uint16 woundCount = 0;
		for (int16 woundIndex = k0_ChampionSlotReadyHand; woundIndex <= k5_ChampionSlotFeet; woundIndex++) {
			if (allowedWounds & (1 << woundIndex)) {
				woundCount++;
				defense += f313_getWoundDefense(champIndex, woundIndex | ((attackType == k4_attackType_SHARP) ? k0x8000_maskUseSharpDefense : k0x0000_maskDoNotUseSharpDefense));
			}
		}
		if (woundCount)
			defense /= woundCount;

		switch (attackType) {
		case k6_attackType_PSYCHIC: 
			{
				int16 wisdomFactor = 115 - curChampion->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent];
				if (wisdomFactor <= 0) {
					attack = 0;
				} else {
					attack = _vm->f30_getScaledProduct(attack, 6, wisdomFactor);
				}

				skipScaling = true;
			}
			break;
		case k5_attackType_MAGIC:
			attack = f307_getStatisticAdjustedAttack(curChampion, k5_ChampionStatAntimagic, attack);
			attack -= _g407_party._spellShieldDefense;
			skipScaling = true;
			break;
		case k1_attackType_FIRE:
			attack = f307_getStatisticAdjustedAttack(curChampion, k6_ChampionStatAntifire, attack);
			attack -= _g407_party._fireShieldDefense;
			break;
		case k2_attackType_SELF:
			defense >>= 1;
			break;
		default:
			break;
		}

		if (!skipScaling) {
			if (attack <= 0)
				return 0;

			attack = _vm->f30_getScaledProduct(attack, 6, 130 - defense);
		}
		/* BUG0_44 
			A champion may take much more damage than expected after a Black Flame attack or an impact
			with a Fireball projectile. If the party has a fire shield defense value higher than the fire
			attack value then the resulting intermediary attack value is negative and damage should be 0. 
			However, the negative value is still used for further computations and the result may be a very
			high positive attack value which may kill a champion. This can occur only for k1_attackType_FIRE
			and if attack is negative before calling F0030_MAIN_GetScaledProduct
		*/

		if (attack <= 0)
			return 0;

		int16 adjustedAttack = f307_getStatisticAdjustedAttack(curChampion, k4_ChampionStatVitality, _vm->getRandomNumber(128) + 10);
		if (attack > adjustedAttack) {
		/* BUG0_45
			This bug is not perceptible because of BUG0_41 that ignores Vitality while determining the
			probability of being wounded. However if it was fixed, the behavior would be the opposite
			of what it should: the higher the vitality of a champion, the lower the result of
			F0307_CHAMPION_GetStatisticAdjustedAttack and the more likely the champion could get
			wounded (because of more iterations in the loop below)
		*/
			do {
				setFlag(*(uint16 *)&_g410_championPendingWounds[champIndex], (1 << _vm->getRandomNumber(8)) & allowedWounds);
			} while ((attack > (adjustedAttack <<= 1)) && adjustedAttack);
		}

		if (_g300_partyIsSleeping)
			f314_wakeUp();
	}
	_g409_championPendingDamage[champIndex] += attack;
	return attack;
}

int16 ChampionMan::f313_getWoundDefense(int16 champIndex, uint16 woundIndex) {
	static const byte woundDefenseFactor[6] = {5, 5, 4, 6, 3, 1}; // @ G0050_auc_Graphic562_WoundDefenseFactor

	Champion *curChampion = &_gK71_champions[champIndex];
	bool useSharpDefense = getFlag(woundIndex, k0x8000_maskUseSharpDefense);
	if (useSharpDefense)
		clearFlag(woundIndex, k0x8000_maskUseSharpDefense);

	uint16 armorShieldDefense = 0;
	for (int16 slotIndex = k0_ChampionSlotReadyHand; slotIndex <= k1_ChampionSlotActionHand; slotIndex++) {
		Thing curThing = curChampion->_slots[slotIndex];
		if (curThing.getType() == k6_ArmourThingType) {
			ArmourInfo *armorInfo = (ArmourInfo *)_vm->_dungeonMan->f156_getThingData(curThing);
			armorInfo = &g239_ArmourInfo[((Armour *)armorInfo)->getType()];
			if (getFlag(armorInfo->_attributes, k0x0080_ArmourAttributeIsAShield))
				armorShieldDefense += ((f312_getStrength(champIndex, slotIndex) + _vm->_dungeonMan->f143_getArmourDefense(armorInfo, useSharpDefense)) * woundDefenseFactor[woundIndex]) >> ((slotIndex == woundIndex) ? 4 : 5);
		}
	}

	int16 woundDefense = _vm->getRandomNumber((curChampion->_statistics[k4_ChampionStatVitality][k1_ChampionStatCurrent] >> 3) + 1);
	if (useSharpDefense)
		woundDefense >>= 1;

	woundDefense += curChampion->_actionDefense + curChampion->_shieldDefense + _g407_party._shieldDefense + armorShieldDefense;
	if (woundIndex > k1_ChampionSlotActionHand)  {
		Thing curThing = curChampion->_slots[woundIndex];
		if (curThing.getType() == k6_ArmourThingType) {
			ArmourInfo *armourInfo = (ArmourInfo *)_vm->_dungeonMan->f156_getThingData(curThing);
			woundDefense += _vm->_dungeonMan->f143_getArmourDefense(&g239_ArmourInfo[((Armour *)armourInfo)->getType()], useSharpDefense);
		}
	}

	if (getFlag(curChampion->_wounds, 1 << woundIndex))
		woundDefense -= 8 + _vm->getRandomNumber(4);

	if (_g300_partyIsSleeping)
		woundDefense >>= 1;

	return f26_getBoundedValue(0, woundDefense >> 1, 100);
}

uint16 ChampionMan::f307_getStatisticAdjustedAttack(Champion *champ, uint16 statIndex, uint16 attack) {
	int16 factor = 170 - champ->_statistics[statIndex][k1_ChampionStatCurrent];

	/* BUG0_41
		The Antifire and Antimagic statistics are completely ignored. The Vitality statistic is ignored
		against poison and to determine the probability of being wounded. Vitality is still used normally
		to compute the defense against wounds and the speed of health regeneration. A bug in the Megamax C
		compiler produces wrong machine code for this statement. It always returns 0 for the current statistic
		value so that factor = 170 in all cases
	*/
	if (factor < 16)
		return attack >> 3;

	return _vm->f30_getScaledProduct(attack, 7, factor);
}

void ChampionMan::f314_wakeUp() {
	_vm->_g321_stopWaitingForPlayerInput = true;
	_g300_partyIsSleeping = false;
	_vm->_g318_waitForInputMaxVerticalBlankCount = 10;
	_vm->f22_delay(10);
	_vm->_displayMan->f98_drawFloorAndCeiling();
	_vm->_eventMan->_g441_primaryMouseInput = g447_PrimaryMouseInput_Interface;
	_vm->_eventMan->_g442_secondaryMouseInput = g448_SecondaryMouseInput_Movement;
	_vm->_eventMan->_g443_primaryKeyboardInput = g458_primaryKeyboardInput_interface;
	_vm->_eventMan->_g444_secondaryKeyboardInput = g459_secondaryKeyboardInput_movement;
	_vm->_eventMan->f357_discardAllInput();
	_vm->_menuMan->f457_drawEnabledMenus();
}

int16 ChampionMan::f305_getThrowingStaminaCost(Thing thing) {
	int16 weight = _vm->_dungeonMan->f140_getObjectWeight(thing) >> 1;
	int16 staminaCost = f26_getBoundedValue<int16>(1, weight, 10);

	while ((weight -= 10) > 0)
		staminaCost += weight >> 1;

	return staminaCost;
}

void ChampionMan::f330_disableAction(uint16 champIndex, uint16 ticks) {
	Champion *curChampion = &_gK71_champions[champIndex];
	int32 updatedEnableActionEventTime = _vm->_g313_gameTime + ticks;

	TimelineEvent curEvent;
	curEvent._type = k11_TMEventTypeEnableChampionAction;
	curEvent._priority = champIndex;
	curEvent._B._slotOrdinal = 0;

	int16 eventIndex = curChampion->_enableActionEventIndex;
	if (eventIndex >= 0) {
		int32 currentEnableActionEventTime = M30_time(_vm->_timeline->_g370_events[eventIndex]._mapTime);
		if (updatedEnableActionEventTime >= currentEnableActionEventTime) {
			updatedEnableActionEventTime += (currentEnableActionEventTime - _vm->_g313_gameTime) >> 1;
		} else {
			updatedEnableActionEventTime = currentEnableActionEventTime + (ticks >> 1);
		}
		_vm->_timeline->f237_deleteEvent(eventIndex);
	} else {
		setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand | k0x0008_ChampionAttributeDisableAction);
		f292_drawChampionState((ChampionIndex)champIndex);
	}
	M33_setMapAndTime(curEvent._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, updatedEnableActionEventTime);
	curChampion->_enableActionEventIndex = _vm->_timeline->f238_addEventGetEventIndex(&curEvent);
}

void ChampionMan::f304_addSkillExperience(uint16 champIndex, uint16 skillIndex, uint16 exp) {
	if ((skillIndex >= k4_ChampionSkillSwing) && (skillIndex <= k11_ChampionSkillShoot) && (_vm->_projexpl->_g361_lastCreatureAttackTime < _vm->_g313_gameTime - 150))
		exp >>= 1;

	if (exp) {
		if (_vm->_dungeonMan->_g269_currMap->_difficulty)
			exp *= _vm->_dungeonMan->_g269_currMap->_difficulty;

		Champion *curChampion = &_gK71_champions[champIndex];
		uint16 baseSkillIndex;
		if (skillIndex >= k4_ChampionSkillSwing)
			baseSkillIndex = (skillIndex - k4_ChampionSkillSwing) >> 2;
		else
			baseSkillIndex = skillIndex;

		uint16 skillLevelBefore = f303_getSkillLevel(champIndex, baseSkillIndex | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience));

		if ((skillIndex >= k4_ChampionSkillSwing) && (_vm->_projexpl->_g361_lastCreatureAttackTime > _vm->_g313_gameTime - 25))
			exp <<= 1;

		Skill *curSkill = &curChampion->_skills[skillIndex];
		curSkill->_experience += exp;
		if (curSkill->_temporaryExperience < 32000)
			curSkill->_temporaryExperience += f26_getBoundedValue(1, exp >> 3, 100);

		curSkill = &curChampion->_skills[baseSkillIndex];
		if (skillIndex >= k4_ChampionSkillSwing)
			curSkill->_experience += exp;

		uint16 skillLevelAfter = f303_getSkillLevel(champIndex, baseSkillIndex | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience));
		if (skillLevelAfter > skillLevelBefore) {
			int16 newBaseSkillLevel = skillLevelAfter;
			int16 minorStatIncrease = _vm->getRandomNumber(2);
			int16 majorStatIncrease = 1 + _vm->getRandomNumber(2);
			uint16 vitalityAmount = _vm->getRandomNumber(2); /* For Priest skill, the amount is 0 or 1 for all skill levels */
			if (baseSkillIndex != k2_ChampionSkillPriest) {
				vitalityAmount &= skillLevelAfter; /* For non Priest skills the amount is 0 for even skill levels. The amount is 0 or 1 for odd skill levels */
			}
			curChampion->_statistics[k4_ChampionStatVitality][k0_ChampionStatMaximum] += vitalityAmount;
			uint16 staminaAmount = curChampion->_maxStamina;
			curChampion->_statistics[k6_ChampionStatAntifire][k0_ChampionStatMaximum] += _vm->getRandomNumber(2) & ~skillLevelAfter; /* The amount is 0 for odd skill levels. The amount is 0 or 1 for even skill levels */
			bool increaseManaFl = false;
			switch (baseSkillIndex) {
			case k0_ChampionSkillFighter:
				staminaAmount >>= 4;
				skillLevelAfter *= 3;
				curChampion->_statistics[k1_ChampionStatStrength][k0_ChampionStatMaximum] += majorStatIncrease;
				curChampion->_statistics[k2_ChampionStatDexterity][k0_ChampionStatMaximum] += minorStatIncrease;
				break;
			case k1_ChampionSkillNinja:
				staminaAmount /= 21;
				skillLevelAfter <<= 1;
				curChampion->_statistics[k1_ChampionStatStrength][k0_ChampionStatMaximum] += minorStatIncrease;
				curChampion->_statistics[k2_ChampionStatDexterity][k0_ChampionStatMaximum] += majorStatIncrease;
				break;
			case k3_ChampionSkillWizard:
				staminaAmount >>= 5;
				curChampion->_maxMana += skillLevelAfter + (skillLevelAfter >> 1);
				curChampion->_statistics[k3_ChampionStatWisdom][k0_ChampionStatMaximum] += majorStatIncrease;
				increaseManaFl = true;
				break;
			case k2_ChampionSkillPriest:
				staminaAmount /= 25;
				curChampion->_maxMana += skillLevelAfter;
				skillLevelAfter += (skillLevelAfter + 1) >> 1;
				curChampion->_statistics[k3_ChampionStatWisdom][k0_ChampionStatMaximum] += minorStatIncrease;
				increaseManaFl = true;
				break;
			default:
				break;
			}
			if (increaseManaFl) {
				if ((curChampion->_maxMana += MIN(_vm->getRandomNumber(4), (uint16)(newBaseSkillLevel - 1))) > 900)
					curChampion->_maxMana = 900;
				curChampion->_statistics[k5_ChampionStatAntimagic][k0_ChampionStatMaximum] += _vm->getRandomNumber(3);
			}

			if ((curChampion->_maxHealth += skillLevelAfter + _vm->getRandomNumber((skillLevelAfter >> 1) + 1)) > 999)
				curChampion->_maxHealth = 999;

			if ((curChampion->_maxStamina += staminaAmount + _vm->getRandomNumber((staminaAmount >> 1) + 1)) > 9999)
				curChampion->_maxStamina = 9999;

			setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
			f292_drawChampionState((ChampionIndex)champIndex);
			_vm->_textMan->f51_messageAreaPrintLineFeed();
			Color curChampionColor = g46_ChampionColor[champIndex];
			_vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, curChampion->_name);
			// TODO: localization
			_vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " JUST GAINED A ");
			_vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, g417_baseSkillName[baseSkillIndex]);
			_vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " LEVEL!");
		}
	}
}

int16 ChampionMan::f324_damageAll_getDamagedChampionCount(uint16 attack, int16 wounds, int16 attackType) {
	int16 randomMax = (attack >> 3) + 1;
	uint16 reducedAttack = attack - randomMax;
	randomMax <<= 1;

	int16 damagedChampionCount = 0;
	for (int16 championIndex = k0_ChampionFirst; championIndex < _g305_partyChampionCount; championIndex++) {
		// Actual attack is attack +/- (attack / 8)
		if (f321_addPendingDamageAndWounds_getDamage(championIndex, MAX(1, reducedAttack + _vm->getRandomNumber(randomMax)), wounds, attackType))
			damagedChampionCount++;
	}

	return damagedChampionCount;
}

int16 ChampionMan::f286_getTargetChampionIndex(int16 mapX, int16 mapY, uint16 cell) {
	if (_g305_partyChampionCount && (M38_distance(mapX, mapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY) <= 1)) {
		signed char orderedCellsToAttack[4];
		_vm->_groupMan->f229_setOrderedCellsToAttack(orderedCellsToAttack, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, mapX, mapY, cell);
		for (uint16 i = 0; i < 4; i++) {
			int16 championIndex = f285_getIndexInCell(orderedCellsToAttack[i]);
			if (championIndex >= 0)
				return championIndex;
		}
	}
	return kM1_ChampionNone;
}

int16 ChampionMan::f311_getDexterity(Champion* champ) {
	int16 dexterity = _vm->getRandomNumber(8) + champ->_statistics[k2_ChampionStatDexterity][k1_ChampionStatCurrent];
	dexterity -= ((int32)(dexterity >> 1) * (int32)champ->_load) / f309_getMaximumLoad(champ);
	if (_g300_partyIsSleeping)
		dexterity >>= 1;

	return f26_getBoundedValue(1 + _vm->getRandomNumber(8), dexterity >> 1, 100 - _vm->getRandomNumber(8));
}

bool ChampionMan::f308_isLucky(Champion* champ, uint16 percentage) {
	if (_vm->getRandomNumber(2) && (_vm->getRandomNumber(100) > percentage))
		return true;

	unsigned char *curStat = champ->_statistics[k0_ChampionStatLuck];
	bool isLucky = (_vm->getRandomNumber(curStat[k1_ChampionStatCurrent]) > percentage);
	curStat[k1_ChampionStatCurrent] = f26_getBoundedValue<char>(curStat[k2_ChampionStatMinimum], curStat[k1_ChampionStatCurrent] + (isLucky ? -2 : 2), curStat[k0_ChampionStatMaximum]);
	return isLucky;
}

void ChampionMan::f322_championPoison(int16 champIndex, uint16 attack) {
	if ((champIndex == kM1_ChampionNone) || (_vm->M0_indexToOrdinal(champIndex) == _g299_candidateChampionOrdinal))
		return;

	Champion *curChampion = &_gK71_champions[champIndex];
	f321_addPendingDamageAndWounds_getDamage(champIndex, MAX(1, attack >> 6), k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
	setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
	if ((_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) && (_vm->_inventoryMan->_g424_panelContent == k0_PanelContentFoodWaterPoisoned)) {
		setFlag(curChampion->_attributes, k0x0800_ChampionAttributePanel);
	}

	if (--attack) {
		curChampion->_poisonEventCount++;
		TimelineEvent newEvent;
		newEvent._type = k75_TMEventTypePoisonChampion;
		newEvent._priority = champIndex;
		M33_setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + 36);
		newEvent._B._attack = attack;
		_vm->_timeline->f238_addEventGetEventIndex(&newEvent);
	}

	f292_drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::f284_setPartyDirection(int16 dir) {
	if (dir == _vm->_dungeonMan->_g308_partyDir)
		return;

	int16 L0834_i_Delta = dir - _vm->_dungeonMan->_g308_partyDir;
	if (L0834_i_Delta < 0)
		L0834_i_Delta += 4;

	Champion *curChampion = _gK71_champions;
	for (int16 i = k0_ChampionFirst; i < _g305_partyChampionCount; i++) {
		curChampion->_cell = (ViewCell)M21_normalizeModulo4(curChampion->_cell + L0834_i_Delta);
		curChampion->_dir = (Direction)M21_normalizeModulo4(curChampion->_dir + L0834_i_Delta);
		curChampion++;
	}

	_vm->_dungeonMan->_g308_partyDir = (Direction)dir;
	f296_drawChangedObjectIcons();
}

void ChampionMan::f316_deleteScent(uint16 scentIndex) {
	uint16 count = --_g407_party._scentCount - scentIndex;

	if (count) {
		for (uint16 i = 0; i < count; ++i) {
			_g407_party._scents[scentIndex + i] = _g407_party._scents[scentIndex + i + 1];
			_g407_party._scentStrengths[scentIndex + i] = _g407_party._scentStrengths[scentIndex + i + 1];
		}
	}

	if (scentIndex < _g407_party._firstScentIndex)
		_g407_party._firstScentIndex--;

	if (scentIndex < _g407_party._lastScentIndex)
		_g407_party._lastScentIndex--;
}

void ChampionMan::f317_addScentStrength(int16 mapX, int16 mapY, int32 cycleCount) {
	int16 scentIndex = _vm->_championMan->_g407_party._scentCount;
	if (scentIndex) {
		bool mergeFl = getFlag(cycleCount, k0x8000_mergeCycles);
		if (mergeFl)
			clearFlag(cycleCount, k0x8000_mergeCycles);

		Scent newScent; /* BUG0_00 Useless code */
		newScent.setMapX(mapX); /* BUG0_00 Useless code */
		newScent.setMapY(mapY); /* BUG0_00 Useless code */
		newScent.setMapIndex(_vm->_dungeonMan->_g272_currMapIndex); /* BUG0_00 Useless code */

		Scent *curScent = _vm->_championMan->_g407_party._scents; /* BUG0_00 Useless code */
		bool cycleCountDefined = false;
		while (scentIndex--) {
			if (&*curScent++ == &newScent) {
				if (!cycleCountDefined) {
					cycleCountDefined = true;
					if (mergeFl) {
						cycleCount = MAX<int32>(_g407_party._scentStrengths[scentIndex], cycleCount);
					} else {
						cycleCount = MIN<int32>(80, _g407_party._scentStrengths[scentIndex] + cycleCount);
					}
				}
				_g407_party._scentStrengths[scentIndex] = cycleCount;
			}
		}
	}
}

void ChampionMan::f297_putObjectInLeaderHand(Thing thing, bool setMousePointer) {
	if (thing == Thing::_none)
		return;

	_g415_leaderEmptyHanded = false;
	_vm->_objectMan->f36_extractIconFromBitmap(_g413_leaderHandObjectIconIndex = _vm->_objectMan->f33_getIconIndex(_g414_leaderHandObject = thing), _vm->_objectMan->_g412_objectIconForMousePointer);
	_vm->_eventMan->f78_showMouse();
	_vm->_objectMan->f34_drawLeaderObjectName(thing);

	if (setMousePointer)
		_vm->_g325_setMousePointerToObjectInMainLoop = true;
	else
		_vm->_eventMan->f68_setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);

	_vm->_eventMan->f77_hideMouse();
	if (_g411_leaderIndex != kM1_ChampionNone) {
		_gK71_champions[_g411_leaderIndex]._load += _vm->_dungeonMan->f140_getObjectWeight(thing);
		setFlag(_gK71_champions[_g411_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
		f292_drawChampionState(_g411_leaderIndex);
	}
}

int16 ChampionMan::f310_getMovementTicks(Champion *champ) {
	uint16 maximumLoad = _vm->_championMan->f309_getMaximumLoad(champ);
	uint16 curLoad = champ->_load;
	uint16 woundTicks;
	int16 ticks;
	/* BUG0_72 - Fixed
		The party moves very slowly even though no champion 'Load' value is drawn in red.
		When the Load of a champion has exactly the maximum value he can carry then the Load
		is drawn in yellow but the speed is the same as when the champion is overloaded
		(when the Load is drawn in red). The comparison operator should be >= instead of >
	*/
	if (maximumLoad >= curLoad) {
		ticks = 2;
		if (((int32)curLoad << 3) > ((int32)maximumLoad * 5))
			ticks++;

		woundTicks = 1;
	} else {
		ticks = 4 + (((curLoad - maximumLoad) << 2) / maximumLoad);
		woundTicks = 2;
	}

	if (getFlag(champ->_wounds, k0x0020_ChampionWoundFeet))
		ticks += woundTicks;

	if (_vm->_objectMan->f33_getIconIndex(champ->_slots[k5_ChampionSlotFeet]) == k194_IconIndiceArmourBootOfSpeed)
		ticks--;

	return ticks;
}

bool ChampionMan::f294_isAmmunitionCompatibleWithWeapon(uint16 champIndex, uint16 weaponSlotIndex, uint16 ammunitionSlotIndex) {
	Champion *curChampion = &_vm->_championMan->_gK71_champions[champIndex];
	Thing curThing = curChampion->_slots[weaponSlotIndex];
	if (curThing.getType() != k5_WeaponThingType)
		return false;

	WeaponInfo *weaponInfo = _vm->_dungeonMan->f158_getWeaponInfo(curThing);
	int16 weaponClass = kM1_WeaponClassNone;

	if ((weaponInfo->_class >= k16_WeaponClassFirstBow) && (weaponInfo->_class <= k31_WeaponClassLastBow))
		weaponClass = k10_WeaponClassBowAmmunition;
	else if ((weaponInfo->_class >= k32_WeaponClassFirstSling) && (weaponInfo->_class <= k47_WeaponClassLastSling))
		weaponClass = k11_WeaponClassSlingAmmunition;

	if (weaponClass == kM1_WeaponClassNone)
		return false;

	curThing = curChampion->_slots[ammunitionSlotIndex];
	weaponInfo = _vm->_dungeonMan->f158_getWeaponInfo(curThing);
	return ((curThing.getType() == k5_WeaponThingType) && (weaponInfo->_class == weaponClass));
}

void ChampionMan::f293_drawAllChampionStates() {
	for (int16 i = k0_ChampionFirst; i < _vm->_championMan->_g305_partyChampionCount; i++)
		_vm->_championMan->f292_drawChampionState((ChampionIndex)i);
}

void ChampionMan::f283_viAltarRebirth(uint16 champIndex) {
	Champion *curChampion = &_vm->_championMan->_gK71_champions[champIndex];
	if (_vm->_championMan->f285_getIndexInCell(curChampion->_cell) != kM1_ChampionNone) {
		uint16 numCell = k0_CellNorthWest;
		while (_vm->_championMan->f285_getIndexInCell(numCell) != kM1_ChampionNone)
			numCell++;

		curChampion->_cell = (ViewCell)numCell;
	}
	
	uint16 maximumHealth = curChampion->_maxHealth;
	curChampion->_maxHealth = MAX(25, maximumHealth - (maximumHealth >> 6) - 1);
	curChampion->_currHealth = curChampion->_maxHealth >> 1;
	_vm->_menuMan->f393_drawSpellAreaControls(_vm->_championMan->_g514_magicCasterChampionIndex);
	curChampion->_dir = _vm->_dungeonMan->_g308_partyDir;
	setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand | k0x1000_ChampionAttributeStatusBox | k0x0400_ChampionAttributeIcon);
	_vm->_championMan->f292_drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::f302_processCommands28to65_clickOnSlotBox(uint16 slotBoxIndex) {
	uint16 champIndex;
	uint16 slotIndex;

	if (slotBoxIndex < k8_SlotBoxInventoryFirstSlot) {
		if (_g299_candidateChampionOrdinal)
			return;

		champIndex = slotBoxIndex >> 1;
		if ((champIndex >= _g305_partyChampionCount) || (_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) || !_gK71_champions[champIndex]._currHealth)
			return;

		slotIndex = M70_handSlotIndex(slotBoxIndex);
	} else {
		champIndex = _vm->M1_ordinalToIndex(_vm->_inventoryMan->_g432_inventoryChampionOrdinal);
		slotIndex = slotBoxIndex - k8_SlotBoxInventoryFirstSlot;
	}

	Thing leaderHandObject = _g414_leaderHandObject;
	Thing slotThing;
	if (slotIndex >= k30_ChampionSlotChest_1) {
		slotThing = _vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1];
	} else {
		slotThing = _gK71_champions[champIndex]._slots[slotIndex];
	}

	if ((slotThing == Thing::_none) && (leaderHandObject == Thing::_none))
		return;

	if ((leaderHandObject != Thing::_none) && (!(g237_ObjectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(leaderHandObject)]._allowedSlots & g38_slotMasks[slotIndex])))
		return;

	_vm->_eventMan->f78_showMouse();
	if (leaderHandObject != Thing::_none)
		f298_getObjectRemovedFromLeaderHand();

	if (slotThing != Thing::_none) {
		f300_getObjectRemovedFromSlot(champIndex, slotIndex);
		f297_putObjectInLeaderHand(slotThing, false);
	}

	if (leaderHandObject != Thing::_none)
		f301_addObjectInSlot((ChampionIndex)champIndex, leaderHandObject, (ChampionSlot)slotIndex);

	f292_drawChampionState((ChampionIndex)champIndex);
	_vm->_eventMan->f77_hideMouse();
}

bool ChampionMan::f327_isProjectileSpellCast(uint16 champIndex, Thing thing, int16 kineticEnergy, uint16 requiredManaAmount) {
	Champion *curChampion = &_vm->_championMan->_gK71_champions[champIndex];
	if (curChampion->_currMana < requiredManaAmount)
		return false;

	curChampion->_currMana -= requiredManaAmount;
	setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
	int16 stepEnergy = 10 - MIN(8, curChampion->_maxMana >> 3);
	if (kineticEnergy < (stepEnergy << 2)) {
		kineticEnergy += 3;
		stepEnergy--;
	}

	f326_championShootProjectile(curChampion, thing, kineticEnergy, 90, stepEnergy);
	return true; // fix BUG_01
}

void ChampionMan::f326_championShootProjectile(Champion* champ, Thing thing, int16 kineticEnergy, int16 attack, int16 stepEnergy) {
	Direction newDirection = champ->_dir;
	_vm->_projexpl->f212_projectileCreate(thing, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, M21_normalizeModulo4((((champ->_cell - newDirection + 1) & 0x0002) >> 1) + newDirection), newDirection, kineticEnergy, attack, stepEnergy);
	_vm->_g311_projectileDisableMovementTicks = 4;
	_vm->_g312_lastProjectileDisabledMovementDirection = newDirection;
}

void ChampionMan::f320_applyAndDrawPendingDamageAndWounds() {
	Champion *championPtr = _vm->_championMan->_gK71_champions;
	for (uint16 championIndex = k0_ChampionFirst; championIndex < _vm->_championMan->_g305_partyChampionCount; championIndex++, championPtr++) {
		int16 pendingWounds = _g410_championPendingWounds[championIndex];
		setFlag(championPtr->_wounds, pendingWounds);
		_g410_championPendingWounds[championIndex] = 0;
		uint16 pendingDamage = _g409_championPendingDamage[championIndex];
		if (!pendingDamage)
			continue;

		_g409_championPendingDamage[championIndex] = 0;
		int16 curHealth = championPtr->_currHealth;
		if (!curHealth)
			continue;

		curHealth -= pendingDamage;
		if (curHealth <= 0) {
			_vm->_championMan->f319_championKill(championIndex);
		} else {
			championPtr->_currHealth = curHealth;
			setFlag(championPtr->_attributes, k0x0100_ChampionAttributeStatistics);
			if (pendingWounds) {
				setFlag(championPtr->_attributes, k0x2000_ChampionAttributeWounds);
			}

			int16 textPosX = championIndex * k69_ChampionStatusBoxSpacing;
			int16 textPosY;

			Box blitBox;
			blitBox._y1 = 0;
			_vm->_eventMan->f78_showMouse();

			if (_vm->M0_indexToOrdinal(championIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
				blitBox._y2 = 28;
				blitBox._x1 = textPosX + 7;
				blitBox._x2 = blitBox._x1 + 31; /* Box is over the champion portrait in the status box */
				_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k16_damageToChampionBig), &blitBox, k16_byteWidth, k10_ColorFlesh, 29);
				// Check the number of digits and sets the position accordingly.
				if (pendingDamage < 10) // 1 digit
					textPosX += 21;
				else if (pendingDamage < 100)  // 2 digits
					textPosX += 18;
				else // 3 digits
					textPosX += 15;

				textPosY = 16;
			} else {
				blitBox._y2 = 6;
				blitBox._x1 = textPosX;
				blitBox._x2 = blitBox._x1 + 47; /* Box is over the champion name in the status box */
				_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k15_damageToChampionSmallIndice), &blitBox, k24_byteWidth, k10_ColorFlesh, 7);
				// Check the number of digits and sets the position accordingly.
				if (pendingDamage < 10) // 1 digit
					textPosX += 19;
				else if (pendingDamage < 100) // 2 digits
					textPosX += 16;
				else //3 digits
					textPosX += 13;

				textPosY = 5;
			}
			_vm->_textMan->f53_printToLogicalScreen(textPosX, textPosY, k15_ColorWhite, k8_ColorRed, _vm->_championMan->f288_getStringFromInteger(pendingDamage, false, 3).c_str());

			int16 eventIndex = championPtr->_hideDamageReceivedIndex;
			if (eventIndex == -1) {
				TimelineEvent newEvent;
				newEvent._type = k12_TMEventTypeHideDamageReceived;
				M33_setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + 5);
				newEvent._priority = championIndex;
				championPtr->_hideDamageReceivedIndex = _vm->_timeline->f238_addEventGetEventIndex(&newEvent);
			} else {
				TimelineEvent *curEvent = &_vm->_timeline->_g370_events[eventIndex];
				M33_setMapAndTime(curEvent->_mapTime, _vm->_dungeonMan->_g309_partyMapIndex, _vm->_g313_gameTime + 5);
				_vm->_timeline->f236_fixChronology(_vm->_timeline->f235_getIndex(eventIndex));
			}
			_vm->_championMan->f292_drawChampionState((ChampionIndex)championIndex);
			_vm->_eventMan->f77_hideMouse();
		}
	}
}

void ChampionMan::f319_championKill(uint16 champIndex) {
	Champion *curChampion = &_vm->_championMan->_gK71_champions[champIndex];
	curChampion->_currHealth = 0;
	setFlag(curChampion->_attributes, k0x1000_ChampionAttributeStatusBox);
	if (_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
		if (_vm->_g331_pressingEye) {
			_vm->_g331_pressingEye = false;
			_vm->_eventMan->_g597_ignoreMouseMovements = false;
			if (!_vm->_championMan->_g415_leaderEmptyHanded) {
				_vm->_objectMan->f34_drawLeaderObjectName(_vm->_championMan->_g414_leaderHandObject);
			}
			_vm->_eventMan->_g587_hideMousePointerRequestCount = 1;
			_vm->_eventMan->f77_hideMouse();
		} else if (_vm->_g333_pressingMouth) {
			_vm->_g333_pressingMouth = false;
			_vm->_eventMan->_g597_ignoreMouseMovements = false;
			_vm->_eventMan->_g587_hideMousePointerRequestCount = 1;
			_vm->_eventMan->f77_hideMouse();
		}
		_vm->_inventoryMan->f355_toggleInventory(k4_ChampionCloseInventory);
	}
	f318_dropAllObjects(champIndex);
	Thing unusedThing = _vm->_dungeonMan->f166_getUnusedThing(k0x8000_championBones | k10_JunkThingType);
	uint16 curCell = 0;
	if (unusedThing != Thing::_none) {
		Junk *L0966_ps_Junk = (Junk *)_vm->_dungeonMan->f156_getThingData(unusedThing);
		L0966_ps_Junk->setType(k5_JunkTypeBones);
		L0966_ps_Junk->setDoNotDiscard(true);
		L0966_ps_Junk->setChargeCount(champIndex);
		curCell = curChampion->_cell;
		_vm->_moveSens->f267_getMoveResult(M15_thingWithNewCell(unusedThing, curCell), kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
	}
	curChampion->_symbolStep = 0;
	curChampion->_symbols[0] = '\0';
	curChampion->_dir = _vm->_dungeonMan->_g308_partyDir;
	curChampion->_maximumDamageReceived = 0;
	uint16 curChampionIconIndex = _vm->_championMan->M26_championIconIndex(curCell, _vm->_dungeonMan->_g308_partyDir);
	if (_vm->M0_indexToOrdinal(curChampionIconIndex) == _vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap) {
		_vm->_eventMan->_g598_mousePointerBitmapUpdated = true;
		_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = _vm->M0_indexToOrdinal(kM1_ChampionNone);
		warning(false, "IGNORED CODE:G0592_B_BuildMousePointerScreenAreaRequested = true");
	}

	if (curChampion->_poisonEventCount)
		f323_unpoison(champIndex);

	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	_vm->_displayMan->D24_fillScreenBox(g54_BoxChampionIcons[curChampionIconIndex << 2], k0_ColorBlack);
	_vm->_championMan->f292_drawChampionState((ChampionIndex)champIndex);
	
	int16 aliveChampionIndex;
	for (aliveChampionIndex = k0_ChampionFirst, curChampion = _vm->_championMan->_gK71_champions; aliveChampionIndex < _vm->_championMan->_g305_partyChampionCount; aliveChampionIndex++, curChampion++) {
		if (curChampion->_currHealth)
			break;
	}

	if (aliveChampionIndex == _vm->_championMan->_g305_partyChampionCount) { /* BUG0_43 The game does not end if the last living champion in the party is killed while looking at a candidate champion in a portrait. The condition to end the game when the whole party is killed is not true because the code considers the candidate champion as alive (in the loop above) */
		_vm->_championMan->_g303_partyDead = true;
		return;
	}

	if (champIndex == _vm->_championMan->_g411_leaderIndex)
		_vm->_eventMan->f368_commandSetLeader((ChampionIndex)aliveChampionIndex);

	if (champIndex == _vm->_championMan->_g514_magicCasterChampionIndex)
		_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(aliveChampionIndex);
	else
		_vm->_menuMan->f393_drawSpellAreaControls(_vm->_championMan->_g514_magicCasterChampionIndex);
}

void ChampionMan::f318_dropAllObjects(uint16 champIndex) {
	static const int16 slotDropOrder[30] = {
		k5_ChampionSlotFeet,
		k4_ChampionSlotLegs,
		k9_ChampionSlotQuiverLine_2_2,
		k8_ChampionSlotQuiverLine_1_2,
		k7_ChampionSlotQuiverLine_2_1,
		k12_ChampionSlotQuiverLine_1_1,
		k6_ChampionSlotPouch_2,
		k11_ChampionSlotPouch_1,
		k3_ChampionSlotTorso,
		k13_ChampionSlotBackpackLine_1_1,
		k14_ChampionSlotBackpackLine_2_2,
		k15_ChampionSlotBackpackLine_2_3,
		k16_ChampionSlotBackpackLine_2_4,
		k17_ChampionSlotBackpackLine_2_5,
		k18_ChampionSlotBackpackLine_2_6,
		k19_ChampionSlotBackpackLine_2_7,
		k20_ChampionSlotBackpackLine_2_8,
		k21_ChampionSlotBackpackLine_2_9,
		k22_ChampionSlotBackpackLine_1_2,
		k23_ChampionSlotBackpackLine_1_3,
		k24_ChampionSlotBackpackLine_1_4,
		k25_ChampionSlotBackpackLine_1_5,
		k26_ChampionSlotBackpackLine_1_6,
		k27_ChampionSlotBackpackLine_1_7,
		k28_ChampionSlotBackpackLine_1_8,
		k29_ChampionSlotBackpackLine_1_9,
		k10_ChampionSlotNeck,
		k2_ChampionSlotHead,
		k0_ChampionSlotReadyHand,
		k1_ChampionSlotActionHand
	};

	uint16 curCell = _vm->_championMan->_gK71_champions[champIndex]._cell;
	for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++) {
		Thing curThing = f300_getObjectRemovedFromSlot(champIndex, slotDropOrder[slotIndex]);
		if (curThing != Thing::_none)
			_vm->_moveSens->f267_getMoveResult(M15_thingWithNewCell(curThing, curCell), kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
	}
}

void ChampionMan::f323_unpoison(int16 champIndex) {
	if (champIndex == kM1_ChampionNone)
		return;

	TimelineEvent *eventPtr = _vm->_timeline->_g370_events;
	for (uint16 eventIndex = 0; eventIndex < _vm->_timeline->_g369_eventMaxCount; eventPtr++, eventIndex++) {
		if ((eventPtr->_type == k75_TMEventTypePoisonChampion) && (eventPtr->_priority == champIndex))
			_vm->_timeline->f237_deleteEvent(eventIndex);
	}
	_vm->_championMan->_gK71_champions[champIndex]._poisonEventCount = 0;
}

void ChampionMan::f331_applyTimeEffects() {
	if (!_vm->_championMan->_g305_partyChampionCount)
		return;

	Scent checkScent;
	checkScent.setMapX(_vm->_dungeonMan->_g306_partyMapX);
	checkScent.setMapY(_vm->_dungeonMan->_g307_partyMapY);
	checkScent.setMapIndex(_vm->_dungeonMan->_g309_partyMapIndex);

	for (byte loopScentIndex = 0; loopScentIndex + 1 < _vm->_championMan->_g407_party._scentCount; loopScentIndex++) {
		if (&_vm->_championMan->_g407_party._scents[loopScentIndex] != &checkScent) {
			_vm->_championMan->_g407_party._scentStrengths[loopScentIndex] = MAX(0, _vm->_championMan->_g407_party._scentStrengths[loopScentIndex] - 1);
			if (!_vm->_championMan->_g407_party._scentStrengths[loopScentIndex] && !loopScentIndex) {
				f316_deleteScent(0);
				continue;
			}
		}
	}

	uint16 gameTime = _vm->_g313_gameTime & 0xFFFF;
	uint16 timeCriteria = (((gameTime & 0x0080) + ((gameTime & 0x0100) >> 2)) + ((gameTime & 0x0040) << 2)) >> 2;
	Champion *championPtr = _vm->_championMan->_gK71_champions;
	for (uint16 championIndex = k0_ChampionFirst; championIndex < _vm->_championMan->_g305_partyChampionCount; championIndex++, championPtr++) {
		if (championPtr->_currHealth && (_vm->M0_indexToOrdinal(championIndex) != _vm->_championMan->_g299_candidateChampionOrdinal)) {
			uint16 wizardSkillLevel = _vm->_championMan->f303_getSkillLevel(championIndex, k3_ChampionSkillWizard) + _vm->_championMan->f303_getSkillLevel(championIndex, k2_ChampionSkillPriest);
			if ((championPtr->_currMana < championPtr->_maxMana)
			&&  (timeCriteria < championPtr->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent] + wizardSkillLevel)) {
				int16 manaGain = championPtr->_maxMana / 40;
				if (_vm->_championMan->_g300_partyIsSleeping)
					manaGain <<= 1;

				manaGain++;
				f325_decrementStamina(championIndex, manaGain * MAX(7, 16 - wizardSkillLevel));
				championPtr->_currMana += MIN<int16>(manaGain, championPtr->_maxMana - championPtr->_currMana);
			} else if (championPtr->_currMana > championPtr->_maxMana)
				championPtr->_currMana--;

			for (int16 idx = k19_ChampionSkillWater; idx >= k0_ChampionSkillFighter; idx--) {
				if (championPtr->_skills[idx]._temporaryExperience > 0)
					championPtr->_skills[idx]._temporaryExperience--;
			}
			uint16 staminaGainCycleCount = 4;
			int16 staminaMagnitude = championPtr->_maxStamina;
			while (championPtr->_currStamina < (staminaMagnitude >>= 1))
				staminaGainCycleCount += 2;

			int16 staminaLoss = 0;
			int16 staminaAmount = f26_getBoundedValue(1, (championPtr->_maxStamina >> 8) - 1, 6);
			if (_vm->_championMan->_g300_partyIsSleeping)
				staminaAmount <<= 1;

			int32 compDelay = _vm->_g313_gameTime - _vm->_projexpl->_g362_lastPartyMovementTime;
			if (compDelay > 80) {
				staminaAmount++;
				if (compDelay > 250)
					staminaAmount++;
			}
			do {
				bool staminaAboveHalf = (staminaGainCycleCount <= 4);
				if (championPtr->_food < -512) {
					if (staminaAboveHalf) {
						staminaLoss += staminaAmount;
						championPtr->_food -= 2;
					}
				} else {
					if (championPtr->_food >= 0)
						staminaLoss -= staminaAmount;

					championPtr->_food -= staminaAboveHalf ? 2 : staminaGainCycleCount >> 1;
				}
				if (championPtr->_water < -512) {
					if (staminaAboveHalf) {
						staminaLoss += staminaAmount;
						championPtr->_water -= 1;
					}
				} else {
					if (championPtr->_water >= 0)
						staminaLoss -= staminaAmount;

					championPtr->_water -= staminaAboveHalf ? 1 : staminaGainCycleCount >> 2;
				}
			} while (--staminaGainCycleCount && ((championPtr->_currStamina - staminaLoss) < championPtr->_maxStamina));
			f325_decrementStamina(championIndex, staminaLoss);
			if (championPtr->_food < -1024)
				championPtr->_food = -1024;

			if (championPtr->_water < -1024)
				championPtr->_water = -1024;

			if ((championPtr->_currHealth < championPtr->_maxHealth) && (championPtr->_currStamina >= (championPtr->_maxStamina >> 2)) && (timeCriteria < (championPtr->_statistics[k4_ChampionStatVitality][k1_ChampionStatCurrent] + 12))) {
				int16 healthGain = (championPtr->_maxHealth >> 7) + 1;
				if (_vm->_championMan->_g300_partyIsSleeping)
					healthGain <<= 1;

				if (_vm->_objectMan->f33_getIconIndex(championPtr->_slots[k10_ChampionSlotNeck]) == k121_IconIndiceJunkEkkhardCross)
					healthGain += (healthGain >> 1) + 1;

				championPtr->_currHealth += MIN(healthGain, (int16)(championPtr->_maxHealth - championPtr->_currHealth));
			}
			if (!((int)_vm->_g313_gameTime & (_vm->_championMan->_g300_partyIsSleeping ? 63 : 255))) {
				for (uint16 i = k0_ChampionStatLuck; i <= k6_ChampionStatAntifire; i++) {
					byte *curStatistic = championPtr->_statistics[i];
					uint16 statisticMaximum = curStatistic[k0_ChampionStatMaximum];
					if (curStatistic[k1_ChampionStatCurrent] < statisticMaximum)
						curStatistic[k1_ChampionStatCurrent]++;
					else if (curStatistic[k1_ChampionStatCurrent] > statisticMaximum)
						curStatistic[k1_ChampionStatCurrent] -= curStatistic[k1_ChampionStatCurrent] / statisticMaximum;
				}
			}
			if (!_vm->_championMan->_g300_partyIsSleeping && (championPtr->_dir != _vm->_dungeonMan->_g308_partyDir) && (_vm->_projexpl->_g361_lastCreatureAttackTime + 60 < _vm->_g313_gameTime)) {
				championPtr->_dir = _vm->_dungeonMan->_g308_partyDir;
				championPtr->_maximumDamageReceived = 0;
				setFlag(championPtr->_attributes, k0x0400_ChampionAttributeIcon);
			}
			setFlag(championPtr->_attributes, k0x0100_ChampionAttributeStatistics);
			if (_vm->M0_indexToOrdinal(championIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
				if (_vm->_g333_pressingMouth || _vm->_g331_pressingEye || (_vm->_inventoryMan->_g424_panelContent == k0_PanelContentFoodWaterPoisoned)) {
					setFlag(championPtr->_attributes, k0x0800_ChampionAttributePanel);
				}
			}
		}
	}
	f293_drawAllChampionStates();
}

void ChampionMan::save2_PartyPart(Common::OutSaveFile* file) {
	for (uint16 i = 0; i < 4; ++i) {
		Champion *champ = &_gK71_champions[i];
		file->writeUint16BE(champ->_attributes);
		file->writeUint16BE(champ->_wounds);
		for (uint16 y = 0; y < 7; ++y)
			for (uint16 x = 0; x < 3; ++x)
				file->writeByte(champ->_statistics[y][x]);
		for (uint16 j = 0; j < 30; ++j)
			file->writeUint16BE(champ->_slots[j].toUint16());
		for (uint16 j = 0; j < 20; ++j) {
			file->writeSint16BE(champ->_skills[j]._temporaryExperience);
			file->writeSint32BE(champ->_skills[j]._experience);
		}
		for (uint16 j = 0; j < 8; ++j)
			file->writeByte(champ->_name[j]);
		for (uint16 j = 0; j < 20; ++j)
			file->writeByte(champ->_title[j]);
		file->writeUint16BE(champ->_dir);
		file->writeUint16BE(champ->_cell);
		file->writeUint16BE(champ->_actionIndex);
		file->writeUint16BE(champ->_symbolStep);
		for (uint16 j = 0; j < 5; ++j)
			file->writeByte(champ->_symbols[j]);
		file->writeUint16BE(champ->_directionMaximumDamageReceived);
		file->writeUint16BE(champ->_maximumDamageReceived);
		file->writeUint16BE(champ->_poisonEventCount);
		file->writeSint16BE(champ->_enableActionEventIndex);
		file->writeSint16BE(champ->_hideDamageReceivedIndex);
		file->writeSint16BE(champ->_currHealth);
		file->writeSint16BE(champ->_maxHealth);
		file->writeSint16BE(champ->_currStamina);
		file->writeSint16BE(champ->_maxStamina);
		file->writeSint16BE(champ->_currMana);
		file->writeSint16BE(champ->_maxMana);
		file->writeSint16BE(champ->_actionDefense);
		file->writeSint16BE(champ->_food);
		file->writeSint16BE(champ->_water);
		file->writeUint16BE(champ->_load);
		file->writeSint16BE(champ->_shieldDefense);
		for (uint16 j = 0; j < 928; ++j)
			file->writeByte(champ->_portrait[j]);
	}

	Party &party = _g407_party;
	file->writeSint16BE(party._magicalLightAmount);
	file->writeByte(party._event73Count_ThievesEye);
	file->writeByte(party._event79Count_Footprints);
	file->writeSint16BE(party._shieldDefense);
	file->writeSint16BE(party._fireShieldDefense);
	file->writeSint16BE(party._spellShieldDefense);
	file->writeByte(party._scentCount);
	file->writeByte(party._freezeLifeTicks);
	file->writeByte(party._firstScentIndex);
	file->writeByte(party._lastScentIndex);
	for (uint16 i = 0; i < 24; ++i)
		file->writeUint16BE(party._scents[i].toUint16());
	for (uint16 i = 0; i < 24; ++i)
		file->writeByte(party._scentStrengths[i]);
	file->writeByte(party._event71Count_Invisibility);
}

void ChampionMan::load2_PartyPart(Common::InSaveFile* file) {
	for (uint16 i = 0; i < 4; ++i) {
		Champion *champ = &_gK71_champions[i];
		champ->_attributes = file->readUint16BE();
		champ->_wounds = file->readUint16BE();
		for (uint16 y = 0; y < 7; ++y)
			for (uint16 x = 0; x < 3; ++x)
				champ->_statistics[y][x] = file->readByte();
		for (uint16 j = 0; j < 30; ++j)
			champ->_slots[j] = Thing(file->readUint16BE());
			for (uint16 j = 0; j < 20; ++j) {
				champ->_skills[j]._temporaryExperience = file->readSint16BE();
				champ->_skills[j]._experience = file->readSint32BE();
			}
		for (uint16 j = 0; j < 8; ++j)
			champ->_name[j] = file->readByte();
		for (uint16 j = 0; j < 20; ++j)
			champ->_title[j] = file->readByte();
		champ->_dir = (Direction)file->readUint16BE();
		champ->_cell = (ViewCell)file->readUint16BE();
		champ->_actionIndex = (ChampionAction)file->readUint16BE();
		champ->_symbolStep = file->readUint16BE();
		for (uint16 j = 0; j < 5; ++j)
			champ->_symbols[j] = file->readByte();
		champ->_directionMaximumDamageReceived = file->readUint16BE();
		champ->_maximumDamageReceived = file->readUint16BE();
		champ->_poisonEventCount = file->readUint16BE();
		champ->_enableActionEventIndex = file->readSint16BE();
		champ->_hideDamageReceivedIndex = file->readSint16BE();
		champ->_currHealth = file->readSint16BE();
		champ->_maxHealth = file->readSint16BE();
		champ->_currStamina = file->readSint16BE();
		champ->_maxStamina = file->readSint16BE();
		champ->_currMana = file->readSint16BE();
		champ->_maxMana = file->readSint16BE();
		champ->_actionDefense = file->readSint16BE();
		champ->_food = file->readSint16BE();
		champ->_water = file->readSint16BE();
		champ->_load = file->readUint16BE();
		champ->_shieldDefense = file->readSint16BE();
		for (uint16 j = 0; j < 928; ++j)
			champ->_portrait[j] = file->readByte();
	}

	Party &party = _g407_party;
	party._magicalLightAmount = file->readSint16BE();
	party._event73Count_ThievesEye = file->readByte();
	party._event79Count_Footprints = file->readByte();
	party._shieldDefense = file->readSint16BE();
	party._fireShieldDefense = file->readSint16BE();
	party._spellShieldDefense = file->readSint16BE();
	party._scentCount = file->readByte();
	party._freezeLifeTicks = file->readByte();
	party._firstScentIndex = file->readByte();
	party._lastScentIndex = file->readByte();
	for (uint16 i = 0; i < 24; ++i)
		party._scents[i] = Scent(file->readUint16BE());
	for (uint16 i = 0; i < 24; ++i)
		party._scentStrengths[i] = file->readByte();
	party._event71Count_Invisibility = file->readByte();
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
		Thing L0787_T_Thing;
		if ((L0787_T_Thing = _g414_leaderHandObject) == Thing::_none) {
			_g415_leaderEmptyHanded = true;
			_g413_leaderHandObjectIconIndex = kM1_IconIndiceNone;
			_vm->_eventMan->f69_setMousePointer();
		} else {
			f297_putObjectInLeaderHand(L0787_T_Thing, true); /* This call will add the weight of the leader hand object to the Load of the leader a first time */
		}
		Champion *L0788_ps_Champion = _gK71_champions;
		int16 L0785_i_ChampionIndex;
		for (L0785_i_ChampionIndex = k0_ChampionFirst; L0785_i_ChampionIndex < _g305_partyChampionCount; L0785_i_ChampionIndex++, L0788_ps_Champion++) {
			clearFlag(L0788_ps_Champion->_attributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand);
			setFlag(L0788_ps_Champion->_attributes, k0x8000_ChampionAttributeActionHand | k0x1000_ChampionAttributeStatusBox | k0x0400_ChampionAttributeIcon);
		}
		f293_drawAllChampionStates();
		if ((L0785_i_ChampionIndex = _g411_leaderIndex) != kM1_ChampionNone) {
			_g411_leaderIndex = kM1_ChampionNone;
			_vm->_eventMan->f368_commandSetLeader((ChampionIndex)L0785_i_ChampionIndex);
		}
		if ((L0785_i_ChampionIndex = _g514_magicCasterChampionIndex) != kM1_ChampionNone) {
			_g514_magicCasterChampionIndex = kM1_ChampionNone;
			_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(L0785_i_ChampionIndex);
		}
		return;
	}

	_g414_leaderHandObject = Thing::_none;
	_g413_leaderHandObjectIconIndex = kM1_IconIndiceNone;
	_g415_leaderEmptyHanded = true;
}

void ChampionMan::f280_addCandidateChampionToParty(uint16 championPortraitIndex) {
	if (!_vm->_championMan->_g415_leaderEmptyHanded)
		return;

	if (_vm->_championMan->_g305_partyChampionCount == 4)
		return;

	uint16 previousPartyChampionCount = _vm->_championMan->_g305_partyChampionCount;
	Champion *championPtr = &_vm->_championMan->_gK71_champions[previousPartyChampionCount];
	championPtr->resetToZero();
	// Strangerke - TODO: Check if the new code is possible to run on the older version (example: the portaits could be missing in the data)
	_vm->_displayMan->_g578_useByteBoxCoordinates = true;
	_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k26_ChampionPortraitsIndice), championPtr->_portrait, gBoxChampionPortrait, _vm->_championMan->M27_getChampionPortraitX(championPortraitIndex), _vm->_championMan->M28_getChampionPortraitY(championPortraitIndex), k128_byteWidth, k16_byteWidth, kM1_ColorNoTransparency);
	championPtr->_actionIndex = k255_ChampionActionNone;
	championPtr->_enableActionEventIndex = -1;
	championPtr->_hideDamageReceivedIndex = -1;
	championPtr->_dir = _vm->_dungeonMan->_g308_partyDir;
	uint16 viewCell = k0_ViewCellFronLeft;
	while (_vm->_championMan->f285_getIndexInCell(M21_normalizeModulo4(viewCell + _vm->_dungeonMan->_g308_partyDir)) != kM1_ChampionNone) {
		viewCell++;
	}
	championPtr->_cell = (ViewCell)M21_normalizeModulo4(viewCell + _vm->_dungeonMan->_g308_partyDir);
	championPtr->_attributes = k0x0400_ChampionAttributeIcon;
	championPtr->_directionMaximumDamageReceived = _vm->_dungeonMan->_g308_partyDir;
	championPtr->_food = 1500 + _vm->getRandomNumber(256);
	championPtr->_water = 1500 + _vm->getRandomNumber(256);
	for (int16 slotIdx = k0_ChampionSlotReadyHand; slotIdx < k30_ChampionSlotChest_1; slotIdx++)
		championPtr->_slots[slotIdx] = Thing::_none;

	Thing curThing = _vm->_dungeonMan->f161_getSquareFirstThing(_vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
	while (curThing.getType() != k2_TextstringType)
		curThing = _vm->_dungeonMan->f159_getNextThing(curThing);

	char L0807_ac_DecodedChampionText[77];
	char *decodedStringPtr = L0807_ac_DecodedChampionText;
	_vm->_dungeonMan->f168_decodeText(decodedStringPtr, curThing, (TextType)(k2_TextTypeScroll | k0x8000_DecodeEvenIfInvisible));

	uint16 charIdx = 0;
	char tmpChar;
	while ((tmpChar = *decodedStringPtr++) != '\n') { /* New line */
		championPtr->_name[charIdx++] = tmpChar;
	}

	championPtr->_name[charIdx] = '\0';
	charIdx = 0;
	bool championTitleCopiedFl = false;
	for (;;) { /*_Infinite loop_*/
		tmpChar = *decodedStringPtr++;
		if (tmpChar == '\n') { /* New line */
			if (championTitleCopiedFl)
				break;
			championTitleCopiedFl = true;
		} else {
			championPtr->_title[charIdx++] = tmpChar;
		}
	}
	championPtr->_title[charIdx] = '\0';
	if (*decodedStringPtr++ == 'M') {
		setFlag(championPtr->_attributes, k0x0010_ChampionAttributeMale);
	}
	decodedStringPtr++;
	championPtr->_currHealth = championPtr->_maxHealth = _vm->_championMan->f279_getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	championPtr->_currStamina = championPtr->_maxStamina = _vm->_championMan->f279_getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	championPtr->_currMana = championPtr->_maxMana = _vm->_championMan->f279_getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	decodedStringPtr++;
	for (int16 statIdx = k0_ChampionStatLuck; statIdx <= k6_ChampionStatAntifire; statIdx++) {
		championPtr->_statistics[statIdx][k2_ChampionStatMinimum] = 30;
		championPtr->_statistics[statIdx][k1_ChampionStatCurrent] = championPtr->_statistics[statIdx][k0_ChampionStatMaximum] = _vm->_championMan->f279_getDecodedValue(decodedStringPtr, 2);
		decodedStringPtr += 2;
	}
	championPtr->_statistics[k0_ChampionStatLuck][k2_ChampionStatMinimum] = 10;
	decodedStringPtr++;
	for (uint16 skillIdx = k4_ChampionSkillSwing; skillIdx <= k19_ChampionSkillWater; skillIdx++) {
		int skillValue = *decodedStringPtr++ - 'A';
		if (skillValue > 0)
			championPtr->_skills[skillIdx]._experience = 125L << skillValue;
	}
	for (uint16 skillIdx = k0_ChampionSkillFighter; skillIdx <= k3_ChampionSkillWizard; skillIdx++) {
		int32 baseSkillExperience = 0;
		int16 hiddenSkillIndex = (skillIdx + 1) << 2;
		for (uint16 hiddenIdx = 0; hiddenIdx < 4; hiddenIdx++)
			baseSkillExperience += championPtr->_skills[hiddenSkillIndex + hiddenIdx]._experience;

		championPtr->_skills[skillIdx]._experience = baseSkillExperience;
	}
	_vm->_championMan->_g299_candidateChampionOrdinal = previousPartyChampionCount + 1;
	if (++_vm->_championMan->_g305_partyChampionCount == 1) {
		_vm->_eventMan->f368_commandSetLeader(k0_ChampionFirst);
		_vm->_menuMan->_g508_refreshActionArea = true;
	} else {
		_vm->_menuMan->f388_clearActingChampion();
		_vm->_menuMan->f386_drawActionIcon((ChampionIndex)(_vm->_championMan->_g305_partyChampionCount - 1));
	}

	int16 curMapX = _vm->_dungeonMan->_g306_partyMapX;
	int16 curMapY = _vm->_dungeonMan->_g307_partyMapY;
	uint16 championObjectsCell = returnOppositeDir(_vm->_dungeonMan->_g308_partyDir);
	curMapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_g308_partyDir], curMapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_g308_partyDir];
	curThing = _vm->_dungeonMan->f161_getSquareFirstThing(curMapX, curMapY);
	int16 slotIdx = k13_ChampionSlotBackpackLine_1_1;
	while (curThing != Thing::_endOfList) {
		ThingType thingType = curThing.getType();
		if ((thingType > k3_SensorThingType) && (curThing.getCell() == championObjectsCell)) {
			int16 objectAllowedSlots = g237_ObjectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(curThing)]._allowedSlots;
			uint16 curSlotIndex;
			switch (thingType) {
			case k6_ArmourThingType: {
				bool skipCheck = false;
				for (curSlotIndex = k2_ChampionSlotHead; curSlotIndex <= k5_ChampionSlotFeet; curSlotIndex++) {
					if (objectAllowedSlots & gSlotMasks[curSlotIndex]) {
						skipCheck = true;
						break;
					}
				}

				if (skipCheck)
					break;

				if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;

				break;
				}
			case k5_WeaponThingType:
				if (championPtr->_slots[k1_ChampionSlotActionHand] == Thing::_none)
					curSlotIndex = k1_ChampionSlotActionHand;
				else if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;
				break;
			case k7_ScrollThingType:
			case k8_PotionThingType:
				if (championPtr->_slots[k11_ChampionSlotPouch_1] == Thing::_none)
					curSlotIndex = k11_ChampionSlotPouch_1;
				else if (championPtr->_slots[k6_ChampionSlotPouch_2] == Thing::_none)
					curSlotIndex = k6_ChampionSlotPouch_2;
				else if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;
				break;
			case k9_ContainerThingType:
			case k10_JunkThingType:
				if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;

				break;
			default:
				break;
			}

			while (championPtr->_slots[curSlotIndex] != Thing::_none) {
				if ((objectAllowedSlots & gSlotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;
			}
			_vm->_championMan->f301_addObjectInSlot((ChampionIndex)previousPartyChampionCount, curThing, (ChampionSlot)curSlotIndex);
		}
		curThing = _vm->_dungeonMan->f159_getNextThing(curThing);
	}
	_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)previousPartyChampionCount);
	_vm->_menuMan->f456_drawDisabledMenu();;
}

void ChampionMan::f287_drawChampionBarGraphs(ChampionIndex champIndex) {
	int16 barGraphHeights[3];
	Champion *champ = &_vm->_championMan->_gK71_champions[champIndex];
	int16 barGraphIdx = 0;
	if (champ->_currHealth > 0) {
		int32 barGraphHeight = (((int32)champ->_currHealth << 10) * 25) / champ->_maxHealth;
		barGraphHeights[barGraphIdx++] = (barGraphHeight >> 10) + ((barGraphHeight & 0x000003FF) ? 1 : 0);
	} else
		barGraphHeights[barGraphIdx++] = 0;

	if (champ->_currStamina > 0) {
		int32 barGraphHeight = (((int32)champ->_currStamina << 10) * 25) / champ->_maxStamina;
		barGraphHeights[barGraphIdx++] = (barGraphHeight >> 10) + ((barGraphHeight & 0x000003FF) ? 1 : 0);
	} else
		barGraphHeights[barGraphIdx++] = 0;

	if (champ->_currMana > 0) {
		if (champ->_currMana > champ->_maxMana)
			barGraphHeights[barGraphIdx] = 25;
		else {
			int32 barGraphHeight = (((int32)champ->_currMana << 10) * 25) / champ->_maxMana;
			barGraphHeights[barGraphIdx] = (barGraphHeight >> 10) + ((barGraphHeight & 0x000003FF) ? 1 : 0);
		}
	} else {
		barGraphHeights[barGraphIdx] = 0;
	}
	_vm->_eventMan->f78_showMouse();

	// Strangerke - TO CHECK: if portraits, maybe the old (assembly) code is required for older versions
	Box box;
	box._x1 = champIndex * k69_ChampionStatusBoxSpacing + 46;
	box._x2 = box._x1 + 3;
	box._y1 = 2;
	box._y2 = 26;
	for (int16 barGraphIndex = 0; barGraphIndex < 3; barGraphIndex++) {
		int16 barGraphHeight = barGraphHeights[barGraphIndex];
		if (barGraphHeight < 25) {
			box._y1 = 2;
			box._y2 = 27 - barGraphHeight;
			_vm->_displayMan->D24_fillScreenBox(box, k12_ColorDarkestGray);
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
		warning(false, "Possible undefined behavior in the original code");
		val /= 2;
		return val + ((uint32)val * (uint32)currStamina) / halfMaxStamina;
	}
	return val;
}

uint16 ChampionMan::f309_getMaximumLoad(Champion *champ) {
	uint16 maximumLoad = champ->getStatistic(k1_ChampionStatStrength, k1_ChampionStatCurrent) * 8 + 100;
	maximumLoad = f306_getStaminaAdjustedValue(champ, maximumLoad);
	int16 wounds = champ->getWounds();
	if (wounds)
		maximumLoad -= maximumLoad >> (champ->getWoundsFlag(k0x0010_ChampionWoundLegs) ? 2 : 3);

	if (_vm->_objectMan->f33_getIconIndex(champ->getSlot(k5_ChampionSlotFeet)) == k119_IconIndiceArmourElvenBoots)
		maximumLoad += maximumLoad * 16;

	maximumLoad += 9;
	maximumLoad -= maximumLoad % 10;
	return maximumLoad;
}

void ChampionMan::f292_drawChampionState(ChampionIndex champIndex) {
	uint16 L0862_ui_ChampionAttributes;
	bool L0863_B_IsInventoryChampion;
	int16 L0864_i_Multiple;
#define AL0864_i_BorderCount       L0864_i_Multiple
#define AL0864_i_ColorIndex        L0864_i_Multiple
#define AL0864_i_Load              L0864_i_Multiple
#define AL0864_i_ChampionIconIndex L0864_i_Multiple
#define AL0864_i_StatisticIndex    L0864_i_Multiple
#define AL0864_i_SlotIndex         L0864_i_Multiple
	Champion* L0865_ps_Champion;
	char* L0866_pc_ChampionName;
	char L0867_c_ChampionTitleFirstCharacter;
	int16 L0868_i_ChampionStatusBoxX;
	int16 L0869_i_ChampionTitleX;
	int16 L0870_i_Multiple;
#define AL0870_i_NativeBitmapIndex L0870_i_Multiple
#define AL0870_i_Color             L0870_i_Multiple
	Box L0871_s_Box;
	int16 L0872_ai_NativeBitmapIndices[3];


	L0868_i_ChampionStatusBoxX = champIndex * k69_ChampionStatusBoxSpacing;
	L0865_ps_Champion = &_gK71_champions[champIndex];
	L0862_ui_ChampionAttributes = L0865_ps_Champion->_attributes;
	if (!getFlag(L0862_ui_ChampionAttributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand)) {
		return;
	}
	L0863_B_IsInventoryChampion = (_vm->M0_indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal);
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	_vm->_eventMan->f78_showMouse();
	if (getFlag(L0862_ui_ChampionAttributes, k0x1000_ChampionAttributeStatusBox)) {
		L0871_s_Box._y1 = 0;
		L0871_s_Box._y2 = 28;
		L0871_s_Box._x2 = (L0871_s_Box._x1 = L0868_i_ChampionStatusBoxX) + 66;
		if (L0865_ps_Champion->_currHealth) {
			_vm->_displayMan->D24_fillScreenBox(L0871_s_Box, k12_ColorDarkestGray);
			for (uint16 i = 0; i < 3; ++i)
				L0872_ai_NativeBitmapIndices[i] = 0;
			AL0864_i_BorderCount = 0;
			if (_g407_party._fireShieldDefense > 0) {
				L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount++] = k38_BorderPartyFireshieldIndice;
			}
			if (_g407_party._spellShieldDefense > 0) {
				L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount++] = k39_BorderPartySpellshieldIndice;
			}
			if ((_g407_party._shieldDefense > 0) || L0865_ps_Champion->_shieldDefense) {
				L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount++] = k37_BorderPartyShieldIndice;
			}
			while (AL0864_i_BorderCount--) {
				_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount]), &L0871_s_Box, k40_byteWidth, k10_ColorFlesh, 29);
			}
			if (L0863_B_IsInventoryChampion) {
				_vm->_inventoryMan->f354_drawStatusBoxPortrait(champIndex);
				setFlag(L0862_ui_ChampionAttributes, k0x0100_ChampionAttributeStatistics);
			} else {
				setFlag(L0862_ui_ChampionAttributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x2000_ChampionAttributeWounds | k0x8000_ChampionAttributeActionHand);
			}
		} else {
			_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k8_StatusBoxDeadChampion), &L0871_s_Box, k40_byteWidth, kM1_ColorNoTransparency, 29);
			_vm->_textMan->f53_printToLogicalScreen(L0868_i_ChampionStatusBoxX + 1, 5, k13_ColorLightestGray, k1_ColorDarkGary, L0865_ps_Champion->_name);
			_vm->_menuMan->f386_drawActionIcon(champIndex);
			goto T0292042;
		}
	}
	if (!(L0865_ps_Champion->_currHealth))
		goto T0292042;
	if (getFlag(L0862_ui_ChampionAttributes, k0x0080_ChampionAttributeNameTitle)) {
		AL0864_i_ColorIndex = (champIndex == _g411_leaderIndex) ? k9_ColorGold : k13_ColorLightestGray;
		if (L0863_B_IsInventoryChampion) {
			_vm->_textMan->f52_printToViewport(3, 7, (Color)AL0864_i_ColorIndex, L0866_pc_ChampionName = L0865_ps_Champion->_name);
			L0869_i_ChampionTitleX = 6 * strlen(L0866_pc_ChampionName) + 3;
			L0867_c_ChampionTitleFirstCharacter = L0865_ps_Champion->_title[0];
			if ((L0867_c_ChampionTitleFirstCharacter != ',') && (L0867_c_ChampionTitleFirstCharacter != ';') && (L0867_c_ChampionTitleFirstCharacter != '-')) {
				L0869_i_ChampionTitleX += 6;
			}
			_vm->_textMan->f52_printToViewport(L0869_i_ChampionTitleX, 7, (Color)AL0864_i_ColorIndex, L0865_ps_Champion->_title);
			setFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport);
		} else {
			L0871_s_Box._y1 = 0;
			L0871_s_Box._y2 = 6;
			L0871_s_Box._x2 = (L0871_s_Box._x1 = L0868_i_ChampionStatusBoxX) + 42;
			_vm->_displayMan->D24_fillScreenBox(L0871_s_Box, k1_ColorDarkGary);
			_vm->_textMan->f53_printToLogicalScreen(L0868_i_ChampionStatusBoxX + 1, 5, (Color)AL0864_i_ColorIndex, k1_ColorDarkGary, L0865_ps_Champion->_name);
		}
	}
	if (getFlag(L0862_ui_ChampionAttributes, k0x0100_ChampionAttributeStatistics)) {
		f287_drawChampionBarGraphs(champIndex);
		if (L0863_B_IsInventoryChampion) {
			f290_drawHealthStaminaManaValues(L0865_ps_Champion);
			if ((L0865_ps_Champion->_food < 0) || (L0865_ps_Champion->_water < 0) || (L0865_ps_Champion->_poisonEventCount)) {
				AL0870_i_NativeBitmapIndex = k34_SlotBoxWoundedIndice;
			} else {
				AL0870_i_NativeBitmapIndex = k33_SlotBoxNormalIndice;
			}
			_vm->_displayMan->f20_blitToViewport(_vm->_displayMan->f489_getNativeBitmapOrGraphic(AL0870_i_NativeBitmapIndex), gBoxMouth, k16_byteWidth, k12_ColorDarkestGray, 18);
			AL0870_i_NativeBitmapIndex = k33_SlotBoxNormalIndice;
			for (AL0864_i_StatisticIndex = k1_ChampionStatStrength; AL0864_i_StatisticIndex <= k6_ChampionStatAntifire; AL0864_i_StatisticIndex++) {
				if ((L0865_ps_Champion->_statistics[AL0864_i_StatisticIndex][k1_ChampionStatCurrent] < L0865_ps_Champion->_statistics[AL0864_i_StatisticIndex][k0_ChampionStatMaximum])) {
					AL0870_i_NativeBitmapIndex = k34_SlotBoxWoundedIndice;
					break;
				}
			}
			_vm->_displayMan->f20_blitToViewport(_vm->_displayMan->f489_getNativeBitmapOrGraphic(AL0870_i_NativeBitmapIndex), gBoxEye, k16_byteWidth, k12_ColorDarkestGray, 18);
			setFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport);
		}
	}
	if (getFlag(L0862_ui_ChampionAttributes, k0x2000_ChampionAttributeWounds)) {
		for (AL0864_i_SlotIndex = L0863_B_IsInventoryChampion ? k5_ChampionSlotFeet : k1_ChampionSlotActionHand; AL0864_i_SlotIndex >= k0_ChampionSlotReadyHand; AL0864_i_SlotIndex--) {
			f291_drawSlot(champIndex, AL0864_i_SlotIndex);
		}
		if (L0863_B_IsInventoryChampion) {
			setFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport);
		}
	}
	if (getFlag(L0862_ui_ChampionAttributes, k0x0200_ChampionAttributeLoad) && L0863_B_IsInventoryChampion) {
		if (L0865_ps_Champion->_load > (AL0864_i_Load = f309_getMaximumLoad(L0865_ps_Champion))) {
			AL0870_i_Color = k8_ColorRed;
		} else {
			if (((long)L0865_ps_Champion->_load << 3) > ((long)AL0864_i_Load * 5)) {
				AL0870_i_Color = k11_ColorYellow;
			} else {
				AL0870_i_Color = k13_ColorLightestGray;
			}
		}
		_vm->_textMan->f52_printToViewport(104, 132, (Color)AL0870_i_Color, "LOAD ");
		AL0864_i_Load = L0865_ps_Champion->_load / 10;
		strcpy(_vm->_g353_stringBuildBuffer, f288_getStringFromInteger(AL0864_i_Load, true, 3).c_str());
		strcat(_vm->_g353_stringBuildBuffer, ".");
		AL0864_i_Load = L0865_ps_Champion->_load - (AL0864_i_Load * 10);
		strcat(_vm->_g353_stringBuildBuffer, f288_getStringFromInteger(AL0864_i_Load, false, 1).c_str());
		strcat(_vm->_g353_stringBuildBuffer, "/");
		AL0864_i_Load = (f309_getMaximumLoad(L0865_ps_Champion) + 5) / 10;
		strcat(_vm->_g353_stringBuildBuffer, f288_getStringFromInteger(AL0864_i_Load, true, 3).c_str());
		strcat(_vm->_g353_stringBuildBuffer, " KG");
		_vm->_textMan->f52_printToViewport(148, 132, (Color)AL0870_i_Color, _vm->_g353_stringBuildBuffer);
		setFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport);
	}
	AL0864_i_ChampionIconIndex = M26_championIconIndex(L0865_ps_Champion->_cell, _vm->_dungeonMan->_g308_partyDir);
	if (getFlag(L0862_ui_ChampionAttributes, k0x0400_ChampionAttributeIcon) && (_vm->_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap != _vm->M0_indexToOrdinal(AL0864_i_ChampionIconIndex))) {
		_vm->_displayMan->D24_fillScreenBox(g54_BoxChampionIcons[AL0864_i_ChampionIconIndex], g46_ChampionColor[champIndex]);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->f489_getNativeBitmapOrGraphic(k28_ChampionIcons), _vm->_displayMan->_g348_bitmapScreen, g54_BoxChampionIcons[AL0864_i_ChampionIconIndex], _vm->_championMan->M26_championIconIndex(L0865_ps_Champion->_dir, _vm->_dungeonMan->_g308_partyDir) * 19, 0, k40_byteWidth, k160_byteWidthScreen, k12_ColorDarkestGray, 14, k200_heightScreen);
	}
	if (getFlag(L0862_ui_ChampionAttributes, k0x0800_ChampionAttributePanel) && L0863_B_IsInventoryChampion) {
		if (_vm->_g333_pressingMouth) {
			_vm->_inventoryMan->f345_drawPanelFoodWaterPoisoned();
		} else {
			if (_vm->_g331_pressingEye) {
				if (_g415_leaderEmptyHanded) {
					_vm->_inventoryMan->f351_drawChampionSkillsAndStatistics();
				}
			} else {
				_vm->_inventoryMan->f347_drawPanel();
			}
		}
		setFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport);
	}
	if (getFlag(L0862_ui_ChampionAttributes, k0x8000_ChampionAttributeActionHand)) {
		f291_drawSlot(champIndex, k1_ChampionSlotActionHand);
		_vm->_menuMan->f386_drawActionIcon(champIndex);
		if (L0863_B_IsInventoryChampion) {
			setFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport);
		}
	}
	if (getFlag(L0862_ui_ChampionAttributes, k0x4000_ChampionAttributeViewport)) {
		_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
	}
T0292042:
	clearFlag(L0865_ps_Champion->_attributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand);
	_vm->_eventMan->f77_hideMouse();
}

uint16 ChampionMan::M26_championIconIndex(int16 val, Direction dir) {
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
	warning(false, "STUB METHOD: Champion::renameChampion, F0281_CHAMPION_Rename");

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

uint16 ChampionMan::f303_getSkillLevel(int16 champIndex, uint16 skillIndex) {
	if (_vm->_championMan->_g300_partyIsSleeping) {
		return 1;
	}
	bool ignoreTmpExp = getFlag(skillIndex, k0x8000_IgnoreTemporaryExperience);
	bool ignoreObjModifiers = getFlag(skillIndex, k0x4000_IgnoreObjectModifiers);
	clearFlag(skillIndex, k0x8000_IgnoreTemporaryExperience | k0x4000_IgnoreObjectModifiers);
	Champion *champ = &_vm->_championMan->_gK71_champions[champIndex];
	Skill *skill = &champ->_skills[skillIndex];
	int32 exp = skill->_experience;
	if (!ignoreTmpExp) {
		exp += skill->_temporaryExperience;
	}
	if (skillIndex > k3_ChampionSkillWizard) { /* Hidden skill */
		skill = &champ->_skills[(skillIndex - k4_ChampionSkillSwing) >> 2];
		exp += skill->_experience; /* Add experience in the base skill */
		if (!ignoreTmpExp) {
			exp += skill->_temporaryExperience;
		}
		exp >>= 1; /* Halve experience to get average of base skill + hidden skill experience */
	}
	int16 skillLevel = 1;
	while (exp >= 500) {
		exp >>= 1;
		skillLevel++;
	}
	if (!ignoreObjModifiers) {
		int16 actionHandIconIndex;
		if ((actionHandIconIndex = _vm->_objectMan->f33_getIconIndex(champ->_slots[k1_ChampionSlotActionHand])) == k27_IconIndiceWeaponTheFirestaff) {
			skillLevel++;
		} else {
			if (actionHandIconIndex == k28_IconIndiceWeaponTheFirestaffComplete) {
				skillLevel += 2;
			}
		}
		int16 neckIconIndex = _vm->_objectMan->f33_getIconIndex(champ->_slots[k10_ChampionSlotNeck]);
		switch (skillIndex) {
		case k3_ChampionSkillWizard:
			if (neckIconIndex == k124_IconIndiceJunkPendantFeral) {
				skillLevel += 1;
			}
			break;
		case k15_ChampionSkillDefend:
			if (neckIconIndex == k121_IconIndiceJunkEkkhardCross) {
				skillLevel += 1;
			}
			break;
		case k13_ChampionSkillHeal:
			if ((neckIconIndex == k120_IconIndiceJunkGemOfAges) || (actionHandIconIndex == k66_IconIndiceWeaponSceptreOfLyf)) { /* The skill modifiers of these two objects are not cumulative */
				skillLevel += 1;
			}
			break;
		case k14_ChampionSkillInfluence:
			if (neckIconIndex == k122_IconIndiceJunkMoonstone) {
				skillLevel += 1;
			}
		}
	}
	return skillLevel;
}

}


