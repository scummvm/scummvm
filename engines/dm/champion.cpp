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
#include "sounds.h"


namespace DM {

void Champion::resetToZero() {
	for (int16 i = 0; i < 30; ++i)
		_slots[i] = Thing::_none;
	for (int16 i = 0; i < 20; ++i)
		_skills[i].resetToZero();
	_attributes = _wounds = 0;
	memset(_statistics, 0, 7 * 3);
	memset(_name, '\0', 8);
	memset(_title, '\0', 20);
	_dir = kDirNorth;
	_cell = k0_ViewCellFronLeft;
	_actionIndex = k0_ChampionActionN;
	_symbolStep = 0;
	memset(_symbols, '\0', 5);
	_directionMaximumDamageReceived = _maximumDamageReceived = _poisonEventCount = _enableActionEventIndex = 0;
	_hideDamageReceivedIndex = _currHealth = _maxHealth = _currStamina = _maxStamina = _currMana = _maxMana = 0;
	_actionDefense = _food = _water = _load = _shieldDefense = 0;
	memset(_portrait, 0, 464);
}

void Champion::setWoundsFlag(ChampionWound flag, bool value) {
	if (value)
		_wounds |= flag;
	else
		_wounds &= ~flag;
}

void Champion::setAttributeFlag(ChampionAttribute flag, bool value) {
	if (value)
		_attributes |= flag;
	else
		_attributes &= ~flag;
}

void ChampionMan::initConstants() {
	static const char *g417_baseSkillName_EN_ANY[4] = {"FIGHTER", "NINJA", "PRIEST", "WIZARD"};
	static const char *g417_baseSkillName_DE_DEU[4] = {"KAEMPFER", "NINJA", "PRIESTER", "MAGIER"};
	static const char *g417_baseSkillName_FR_FRA[4] = {"GUERRIER", "NINJA", "PRETRE", "SORCIER"};
	static Box boxChampionIcons[4] = {
		Box(281, 299,  0, 13),
		Box(301, 319,  0, 13),
		Box(301, 319, 15, 28),
		Box(281, 299, 15, 28)
	};

	static Color championColor[4] = {(Color)7, (Color)11, (Color)8, (Color)14};
	int16 lightPowerToLightAmount[16] = {0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100};
	uint16 slotMasks[38] = {  // @ G0038_ai_Graphic562_SlotMasks
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
		0x0400    /* Chest 8          Chest */
	};

	_boxChampionPortrait = Box(0, 31, 0, 28); // @ G0047_s_Graphic562_Box_ChampionPortrait 

	const char **g417_byLanguage;
	switch (_vm->getGameLanguage()) { // localized
	case Common::EN_ANY: 
		g417_byLanguage = g417_baseSkillName_EN_ANY;
		break;
	case Common::DE_DEU:
		g417_byLanguage = g417_baseSkillName_DE_DEU;
		break;
	case Common::FR_FRA:
		g417_byLanguage = g417_baseSkillName_FR_FRA;
		break;
	default:
		error("Unexpected language used");
	}

	for (int i = 0; i < 4; ++i) {
		_baseSkillName[i] = g417_byLanguage[i];
		_championColor[i] = championColor[i];
		_boxChampionIcons[i] = boxChampionIcons[i];
	}

	for (int i = 0; i < 16; i++)
		_lightPowerToLightAmount[i] = lightPowerToLightAmount[i];

	for (int i = 0; i < 38; i++)
		_slotMasks[i] = slotMasks[i];
}

ChampionMan::ChampionMan(DMEngine *vm) : _vm(vm) {
	for (uint16 i = 0; i < 4; ++i) {
		_championPendingDamage[i] = 0;
		_championPendingWounds[i] = 0;
		_champions[i].resetToZero();
	}
	_partyChampionCount = 0;
	_partyDead = false;
	_leaderHandObject = Thing(0);
	_leaderIndex = kM1_ChampionNone;
	_candidateChampionOrdinal = 0;
	_partyIsSleeping = false;
	_actingChampionOrdinal = 0;
	_leaderHandObjectIconIndex = (IconIndice)0;
	_leaderEmptyHanded = false;
	_party.resetToZero();
	_magicCasterChampionIndex = kM1_ChampionNone;
	_mousePointerHiddenToDrawChangedObjIconOnScreen = false;

	initConstants();
}

bool ChampionMan::isLeaderHandObjectThrown(int16 side) {
	if (_leaderIndex == kM1_ChampionNone)
		return false;

	return isObjectThrown(_leaderIndex, kM1_ChampionSlotLeaderHand, side);
}

bool ChampionMan::isObjectThrown(uint16 champIndex, int16 slotIndex, int16 side) {
	bool throwingLeaderHandObjectFl = false;
	Thing curThing;
	Champion *curChampion = nullptr;
	Thing actionHandThing;

	if (slotIndex < 0) { /* Throw object in leader hand, which is temporarily placed in action hand */
		if (_leaderEmptyHanded)
			return false;

		curThing = getObjectRemovedFromLeaderHand();
		curChampion = &_champions[champIndex];
		actionHandThing = curChampion->getSlot(k1_ChampionSlotActionHand);
		curChampion->setSlot(k1_ChampionSlotActionHand, curThing);
		slotIndex = k1_ChampionSlotActionHand;
		throwingLeaderHandObjectFl = true;
	}

	int16 kineticEnergy = getStrength(champIndex, slotIndex);
	if (throwingLeaderHandObjectFl) {
		// In this case, curChampion and actionHandThing are set.
		curChampion->setSlot((ChampionSlot)slotIndex, actionHandThing);
	} else {
		curThing = getObjectRemovedFromSlot(champIndex, slotIndex);
		if (curThing == Thing::_none)
			return false;
	}

	_vm->_sound->f064_SOUND_RequestPlay_CPSD(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);
	decrementStamina(champIndex, getThrowingStaminaCost(curThing));
	disableAction(champIndex, 4);
	int16 experience = 8;
	int16 weaponKineticEnergy = 1;
	if (curThing.getType() == k5_WeaponThingType) {
		experience += 4;
		WeaponInfo *curWeapon = _vm->_dungeonMan->getWeaponInfo(curThing);
		if (curWeapon->_class <= k12_WeaponClassPoisinDart) {
			weaponKineticEnergy = curWeapon->_kineticEnergy;
			experience += weaponKineticEnergy >> 2;
		}
	}
	addSkillExperience(champIndex, k10_ChampionSkillThrow, experience);
	kineticEnergy += weaponKineticEnergy;
	int16 skillLevel = getSkillLevel((ChampionIndex)champIndex, k10_ChampionSkillThrow);
	kineticEnergy += _vm->getRandomNumber(16) + (kineticEnergy >> 1) + skillLevel;
	int16 attack = getBoundedValue((uint16)40, (uint16)((skillLevel << 3) + _vm->getRandomNumber(32)), (uint16)200);
	int16 stepEnergy = MAX(5, 11 - skillLevel);
	_vm->_projexpl->f212_projectileCreate(curThing, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY,
										  normalizeModulo4(_vm->_dungeonMan->_partyDir + side),
										  _vm->_dungeonMan->_partyDir, kineticEnergy, attack, stepEnergy);
	_vm->_projectileDisableMovementTicks = 4;
	_vm->_lastProjectileDisabledMovementDirection = _vm->_dungeonMan->_partyDir;
	drawChampionState((ChampionIndex)champIndex);
	return true;
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
	Common::String tmp = getStringFromInteger(currVal, true, 3);
	_vm->_textMan->f52_printToViewport(55, posY, k13_ColorLightestGray, tmp.c_str());
	_vm->_textMan->f52_printToViewport(73, posY, k13_ColorLightestGray, "/");
	tmp = getStringFromInteger(maxVal, true, 3);
	_vm->_textMan->f52_printToViewport(79, posY, k13_ColorLightestGray, tmp.c_str());
}

uint16 ChampionMan::getHandSlotIndex(uint16 slotBoxIndex) {
	return slotBoxIndex & 0x1;
}


Common::String ChampionMan::getStringFromInteger(uint16 val, bool padding, uint16 paddingCharCount) {
	Common::String valToStr = Common::String::format("%d", val);
	Common::String result;

	if (padding) {
		for (int16 i = 0, end = paddingCharCount - valToStr.size(); i < end; ++i)
			result += ' ';
	}

	return result += valToStr;
}

void ChampionMan::applyModifiersToStatistics(Champion *champ, int16 slotIndex, int16 iconIndex, int16 modifierFactor, Thing thing) {
	int16 statIndex = k0_ChampionStatLuck;
	int16 modifier = 0;
	ThingType thingType = thing.getType();

	bool cursed = false;
	if (((thingType == k5_WeaponThingType) || (thingType == k6_ArmourThingType))
		&& (slotIndex >= k0_ChampionSlotReadyHand) && (slotIndex <= k12_ChampionSlotQuiverLine_1_1)) {
		if (thingType == k5_WeaponThingType) {
			Weapon *weapon = (Weapon *)_vm->_dungeonMan->getThingData(thing);
			cursed = weapon->getCursed();
		} else {
			// k6_ArmourThingType
			Armour *armour = (Armour *)_vm->_dungeonMan->getThingData(thing);
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

bool ChampionMan::hasObjectIconInSlotBoxChanged(int16 slotBoxIndex, Thing thing) {
	ObjectMan &objMan = *_vm->_objectMan;

	IconIndice currIconIndex = objMan.f39_getIconIndexInSlotBox(slotBoxIndex);
	if (((currIconIndex < k32_IconIndiceWeaponDagger) && (currIconIndex >= k0_IconIndiceJunkCompassNorth))
		|| ((currIconIndex >= k148_IconIndicePotionMaPotionMonPotion) && (currIconIndex <= k163_IconIndicePotionWaterFlask))
		|| (currIconIndex == k195_IconIndicePotionEmptyFlask)) {
		IconIndice newIconIndex = objMan.f33_getIconIndex(thing);
		if (newIconIndex != currIconIndex) {
			if ((slotBoxIndex < k8_SlotBoxInventoryFirstSlot) && !_mousePointerHiddenToDrawChangedObjIconOnScreen) {
				_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
				_vm->_eventMan->hideMouse();
			}
			objMan.f38_drawIconInSlotBox(slotBoxIndex, newIconIndex);
			return true;
		}
	}

	return false;
}

void ChampionMan::drawChangedObjectIcons() {
	InventoryMan &invMan = *_vm->_inventoryMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;

	uint16 invChampOrdinal = invMan._g432_inventoryChampionOrdinal;
	if (_candidateChampionOrdinal && !invChampOrdinal)
		return;

	_mousePointerHiddenToDrawChangedObjIconOnScreen = false;
	IconIndice leaderHandObjIconIndex = _leaderHandObjectIconIndex;

	if (((leaderHandObjIconIndex < k32_IconIndiceWeaponDagger) && (leaderHandObjIconIndex >= k0_IconIndiceJunkCompassNorth))	// < instead of <= is correct
		|| ((leaderHandObjIconIndex >= k148_IconIndicePotionMaPotionMonPotion) && (leaderHandObjIconIndex <= k163_IconIndicePotionWaterFlask))
		|| (leaderHandObjIconIndex == k195_IconIndicePotionEmptyFlask)) {
		IconIndice iconIndex = objMan.f33_getIconIndex(_leaderHandObject);
		if (iconIndex != leaderHandObjIconIndex) {
			_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
			_vm->_eventMan->hideMouse();
			objMan.f36_extractIconFromBitmap(iconIndex, objMan._g412_objectIconForMousePointer);
			_vm->_eventMan->setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);
			_leaderHandObjectIconIndex = iconIndex;
			objMan.f34_drawLeaderObjectName(_leaderHandObject);
		}
	}

	for (uint16 slotBoxIndex = 0; slotBoxIndex < (_partyChampionCount * 2); ++slotBoxIndex) {
		int16 champIndex = slotBoxIndex >> 1;
		if (invChampOrdinal == _vm->indexToOrdinal(champIndex))
			continue;

		if (hasObjectIconInSlotBoxChanged(slotBoxIndex, _champions[champIndex].getSlot((ChampionSlot)getHandSlotIndex(slotBoxIndex)))
			&& (getHandSlotIndex(slotBoxIndex) == k1_ChampionSlotActionHand)) {

			menuMan.f386_drawActionIcon((ChampionIndex)champIndex);
		}
	}

	if (invChampOrdinal) {
		Champion *champ = &_champions[_vm->ordinalToIndex(invChampOrdinal)];
		Thing *thing = &champ->getSlot(k0_ChampionSlotReadyHand);
		uint16 drawViewport = 0;

		for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++, thing++) {
			uint16 objIconChanged = hasObjectIconInSlotBoxChanged(slotIndex + k8_SlotBoxInventoryFirstSlot, *thing) ? 1 : 0;
			drawViewport |= objIconChanged;
			if (objIconChanged && (slotIndex == k1_ChampionSlotActionHand)) {
				menuMan.f386_drawActionIcon((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
			}
		}

		if (invMan._g424_panelContent == k4_PanelContentChest) {
			thing = invMan._g425_chestSlots;
			for (int16 slotIndex = 0; slotIndex < 8; ++slotIndex, thing++) {
				drawViewport |= (hasObjectIconInSlotBoxChanged(slotIndex + k38_SlotBoxChestFirstSlot, *thing) ? 1 : 0);
			}
		}

		if (drawViewport) {
			champ->setAttributeFlag(k0x4000_ChampionAttributeViewport, true);
			drawChampionState((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
		}
	}

	if (_mousePointerHiddenToDrawChangedObjIconOnScreen)
		_vm->_eventMan->showMouse();
}

void ChampionMan::addObjectInSlot(ChampionIndex champIndex, Thing thing, ChampionSlot slotIndex) {
	InventoryMan &invMan = *_vm->_inventoryMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;

	if (thing == Thing::_none)
		return;

	Champion *champ = &_champions[champIndex];

	if (slotIndex >= k30_ChampionSlotChest_1) {
		invMan._g425_chestSlots[slotIndex - k30_ChampionSlotChest_1] = thing;
	} else {
		champ->setSlot(slotIndex, thing);
	}

	champ->_load += dunMan.getObjectWeight(thing);
	champ->setAttributeFlag(k0x0200_ChampionAttributeLoad, true);
	IconIndice iconIndex = objMan.f33_getIconIndex(thing);
	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == invMan._g432_inventoryChampionOrdinal);
	applyModifiersToStatistics(champ, slotIndex, iconIndex, 1, thing);
	uint16 *rawObjPtr = dunMan.getThingData(thing);

	if (slotIndex < k2_ChampionSlotHead) {
		if (slotIndex == k1_ChampionSlotActionHand) {
			champ->setAttributeFlag(k0x8000_ChampionAttributeActionHand, true);
			if (_actingChampionOrdinal == _vm->indexToOrdinal(champIndex))
				menuMan.f388_clearActingChampion();

			if ((iconIndex >= k30_IconIndiceScrollOpen) && (iconIndex <= k31_IconIndiceScrollClosed)) {
				((Scroll *)rawObjPtr)->setClosed(false);
				drawChangedObjectIcons();
			}
		}

		if (iconIndex == k4_IconIndiceWeaponTorchUnlit) {
			((Weapon *)rawObjPtr)->setLit(true);
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			drawChangedObjectIcons();
		} else if (isInventoryChampion && (slotIndex == k1_ChampionSlotActionHand) &&
			((iconIndex == k144_IconIndiceContainerChestClosed) || ((iconIndex >= k30_IconIndiceScrollOpen) && (iconIndex <= k31_IconIndiceScrollClosed)))) {
			champ->setAttributeFlag(k0x0800_ChampionAttributePanel, true);
		}
	} else if (slotIndex == k10_ChampionSlotNeck) {
		if ((iconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (iconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk *)rawObjPtr)->setChargeCount(1);
			_party._magicalLightAmount += _lightPowerToLightAmount[2];
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			iconIndex = (IconIndice)(iconIndex + 1);
		} else if ((iconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (iconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
			((Junk *)rawObjPtr)->setChargeCount(1);
			iconIndex = (IconIndice)(iconIndex + 1);
		}
	}

	drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		champ->setAttributeFlag(k0x4000_ChampionAttributeViewport, true);
}

int16 ChampionMan::getScentOrdinal(int16 mapX, int16 mapY) {
	int16 scentIndex = _party._scentCount;

	if (scentIndex) {
		Scent searchedScent;
		searchedScent.setMapX(mapX);
		searchedScent.setMapY(mapY);
		searchedScent.setMapIndex(_vm->_dungeonMan->_currMapIndex);
		uint16 searchedScentRedEagle = searchedScent.toUint16();
		Scent *scent = &_party._scents[scentIndex--];
		do {
			if ((*(--scent)).toUint16() == searchedScentRedEagle) {
				return _vm->indexToOrdinal(scentIndex);
			}
		} while (scentIndex--);
	}
	return 0;
}

Thing ChampionMan::getObjectRemovedFromLeaderHand() {
	_leaderEmptyHanded = true;
	Thing leaderHandObject = _leaderHandObject;

	if (leaderHandObject != Thing::_none) {
		_leaderHandObject = Thing::_none;
		_leaderHandObjectIconIndex = kM1_IconIndiceNone;
		_vm->_eventMan->showMouse();
		_vm->_objectMan->f35_clearLeaderObjectName();
		_vm->_eventMan->setMousePointer();
		_vm->_eventMan->hideMouse();
		if (_leaderIndex != kM1_ChampionNone) {
			_champions[_leaderIndex]._load -= _vm->_dungeonMan->getObjectWeight(leaderHandObject);
			setFlag(_champions[_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
			drawChampionState(_leaderIndex);
		}
	}
	return leaderHandObject;
}

uint16 ChampionMan::getStrength(int16 champIndex, int16 slotIndex) {
	Champion *curChampion = &_champions[champIndex];
	int16 strength = _vm->getRandomNumber(16) + curChampion->_statistics[k1_ChampionStatStrength][k1_ChampionStatCurrent];
	Thing curThing = curChampion->_slots[slotIndex];
	uint16 objectWeight = _vm->_dungeonMan->getObjectWeight(curThing);
	uint16 oneSixteenthMaximumLoad = getMaximumLoad(curChampion) >> 4;

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
		WeaponInfo *weaponInfo = _vm->_dungeonMan->getWeaponInfo(curThing);
		strength += weaponInfo->_strength;
		uint16 skillLevel = 0;
		uint16 weaponClass = weaponInfo->_class;
		if ((weaponClass == k0_WeaponClassSwingWeapon) || (weaponClass == k2_WeaponClassDaggerAndAxes)) {
			skillLevel = getSkillLevel(champIndex, k4_ChampionSkillSwing);
		}
		if ((weaponClass != k0_WeaponClassSwingWeapon) && (weaponClass < k16_WeaponClassFirstBow)) {
			skillLevel += getSkillLevel(champIndex, k10_ChampionSkillThrow);
		}
		if ((weaponClass >= k16_WeaponClassFirstBow) && (weaponClass < k112_WeaponClassFirstMagicWeapon)) {
			skillLevel += getSkillLevel(champIndex, k11_ChampionSkillShoot);
		}
		strength += skillLevel << 1;
	}
	strength = getStaminaAdjustedValue(curChampion, strength);
	if (getFlag(curChampion->_wounds, (slotIndex == k0_ChampionSlotReadyHand) ? k0x0001_ChampionWoundReadHand : k0x0002_ChampionWoundActionHand)) {
		strength >>= 1;
	}
	return getBoundedValue(0, strength >> 1, 100);
}

Thing ChampionMan::getObjectRemovedFromSlot(uint16 champIndex, uint16 slotIndex) {
	Champion *curChampion = &_champions[champIndex];
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

	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal);
	int16 curIconIndex = _vm->_objectMan->f33_getIconIndex(curThing);
	// Remove object modifiers
	applyModifiersToStatistics(curChampion, slotIndex, curIconIndex, -1, curThing);

	Weapon *curWeapon = (Weapon *)_vm->_dungeonMan->getThingData(curThing);
	if (slotIndex == k10_ChampionSlotNeck) {
		if ((curIconIndex >= k12_IconIndiceJunkIllumuletUnequipped) && (curIconIndex <= k13_IconIndiceJunkIllumuletEquipped)) {
			((Junk *)curWeapon)->setChargeCount(0);
			_party._magicalLightAmount -= _lightPowerToLightAmount[2];
			_vm->_inventoryMan->f337_setDungeonViewPalette();
		} else if ((curIconIndex >= k10_IconIndiceJunkJewelSymalUnequipped) && (curIconIndex <= k11_IconIndiceJunkJewelSymalEquipped)) {
			((Junk *)curWeapon)->setChargeCount(0);
		}
	}

	drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		setFlag(curChampion->_attributes, k0x4000_ChampionAttributeViewport);

	if (slotIndex < k2_ChampionSlotHead) {
		if (slotIndex == k1_ChampionSlotActionHand) {
			setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand);
			if (_actingChampionOrdinal == _vm->indexToOrdinal(champIndex))
				_vm->_menuMan->f388_clearActingChampion();

			if ((curIconIndex >= k30_IconIndiceScrollOpen) && (curIconIndex <= k31_IconIndiceScrollClosed)) {
				((Scroll *)curWeapon)->setClosed(true);
				drawChangedObjectIcons();
			}
		}

		if ((curIconIndex >= k4_IconIndiceWeaponTorchUnlit) && (curIconIndex <= k7_IconIndiceWeaponTorchLit)) {
			curWeapon->setLit(false);
			_vm->_inventoryMan->f337_setDungeonViewPalette();
			drawChangedObjectIcons();
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
	curChampion->_load -= _vm->_dungeonMan->getObjectWeight(curThing);
	setFlag(curChampion->_attributes, k0x0200_ChampionAttributeLoad);
	return curThing;
}

void ChampionMan::decrementStamina(int16 championIndex, int16 decrement) {
	if (championIndex == kM1_ChampionNone)
		return;

	Champion *curChampion = &_champions[championIndex];
	curChampion->_currStamina -= decrement;

	int16 stamina = curChampion->_currStamina;
	if (stamina <= 0) {
		curChampion->_currStamina = 0;
		addPendingDamageAndWounds_getDamage(championIndex, (-stamina) >> 1, k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
	} else if (stamina > curChampion->_maxStamina) {
		curChampion->_currStamina = curChampion->_maxStamina;
	}

	setFlag(curChampion->_attributes, k0x0200_ChampionAttributeLoad | k0x0100_ChampionAttributeStatistics);
}

int16 ChampionMan::addPendingDamageAndWounds_getDamage(int16 champIndex, int16 attack, int16 allowedWounds, uint16 attackType) {
	if (attack <= 0)
		return 0;

	Champion *curChampion = &_champions[champIndex];
	if (!curChampion->_currHealth)
		return 0;

	if (attackType != k0_attackType_NORMAL) {
		uint16 defense = 0;
		uint16 woundCount = 0;
		for (int16 woundIndex = k0_ChampionSlotReadyHand; woundIndex <= k5_ChampionSlotFeet; woundIndex++) {
			if (allowedWounds & (1 << woundIndex)) {
				woundCount++;
				defense += getWoundDefense(champIndex, woundIndex | ((attackType == k4_attackType_SHARP) ? k0x8000_maskUseSharpDefense : k0x0000_maskDoNotUseSharpDefense));
			}
		}
		if (woundCount)
			defense /= woundCount;

		bool skipScaling = false;
		switch (attackType) {
		case k6_attackType_PSYCHIC:
		{
			int16 wisdomFactor = 115 - curChampion->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent];
			if (wisdomFactor <= 0) {
				attack = 0;
			} else {
				attack = _vm->getScaledProduct(attack, 6, wisdomFactor);
			}

			skipScaling = true;
		}
		break;
		case k5_attackType_MAGIC:
			attack = getStatisticAdjustedAttack(curChampion, k5_ChampionStatAntimagic, attack);
			attack -= _party._spellShieldDefense;
			skipScaling = true;
			break;
		case k1_attackType_FIRE:
			attack = getStatisticAdjustedAttack(curChampion, k6_ChampionStatAntifire, attack);
			attack -= _party._fireShieldDefense;
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

			attack = _vm->getScaledProduct(attack, 6, 130 - defense);
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

		int16 adjustedAttack = getStatisticAdjustedAttack(curChampion, k4_ChampionStatVitality, _vm->getRandomNumber(128) + 10);
		if (attack > adjustedAttack) {
		/* BUG0_45
			This bug is not perceptible because of BUG0_41 that ignores Vitality while determining the
			probability of being wounded. However if it was fixed, the behavior would be the opposite
			of what it should: the higher the vitality of a champion, the lower the result of
			F0307_CHAMPION_GetStatisticAdjustedAttack and the more likely the champion could get
			wounded (because of more iterations in the loop below)
		*/
			do {
				setFlag(*(uint16 *)&_championPendingWounds[champIndex], (1 << _vm->getRandomNumber(8)) & allowedWounds);
			} while ((attack > (adjustedAttack <<= 1)) && adjustedAttack);
		}

		if (_partyIsSleeping)
			wakeUp();
	}
	_championPendingDamage[champIndex] += attack;
	return attack;
}

int16 ChampionMan::getWoundDefense(int16 champIndex, uint16 woundIndex) {
	static const byte woundDefenseFactor[6] = {5, 5, 4, 6, 3, 1}; // @ G0050_auc_Graphic562_WoundDefenseFactor

	Champion *curChampion = &_champions[champIndex];
	bool useSharpDefense = getFlag(woundIndex, k0x8000_maskUseSharpDefense);
	if (useSharpDefense)
		clearFlag(woundIndex, k0x8000_maskUseSharpDefense);

	uint16 armorShieldDefense = 0;
	for (int16 slotIndex = k0_ChampionSlotReadyHand; slotIndex <= k1_ChampionSlotActionHand; slotIndex++) {
		Thing curThing = curChampion->_slots[slotIndex];
		if (curThing.getType() == k6_ArmourThingType) {
			ArmourInfo *armorInfo = (ArmourInfo *)_vm->_dungeonMan->getThingData(curThing);
			armorInfo = &_vm->_dungeonMan->_armourInfo[((Armour *)armorInfo)->getType()];
			if (getFlag(armorInfo->_attributes, k0x0080_ArmourAttributeIsAShield))
				armorShieldDefense += ((getStrength(champIndex, slotIndex) + _vm->_dungeonMan->getArmourDefense(armorInfo, useSharpDefense)) * woundDefenseFactor[woundIndex]) >> ((slotIndex == woundIndex) ? 4 : 5);
		}
	}

	int16 woundDefense = _vm->getRandomNumber((curChampion->_statistics[k4_ChampionStatVitality][k1_ChampionStatCurrent] >> 3) + 1);
	if (useSharpDefense)
		woundDefense >>= 1;

	woundDefense += curChampion->_actionDefense + curChampion->_shieldDefense + _party._shieldDefense + armorShieldDefense;
	if (woundIndex > k1_ChampionSlotActionHand) {
		Thing curThing = curChampion->_slots[woundIndex];
		if (curThing.getType() == k6_ArmourThingType) {
			ArmourInfo *armourInfo = (ArmourInfo *)_vm->_dungeonMan->getThingData(curThing);
			woundDefense += _vm->_dungeonMan->getArmourDefense(&_vm->_dungeonMan->_armourInfo[((Armour *)armourInfo)->getType()], useSharpDefense);
		}
	}

	if (getFlag(curChampion->_wounds, 1 << woundIndex))
		woundDefense -= 8 + _vm->getRandomNumber(4);

	if (_partyIsSleeping)
		woundDefense >>= 1;

	return getBoundedValue(0, woundDefense >> 1, 100);
}

uint16 ChampionMan::getStatisticAdjustedAttack(Champion *champ, uint16 statIndex, uint16 attack) {
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

	return _vm->getScaledProduct(attack, 7, factor);
}

void ChampionMan::wakeUp() {
	_vm->_stopWaitingForPlayerInput = true;
	_partyIsSleeping = false;
	_vm->_waitForInputMaxVerticalBlankCount = 10;
	_vm->delay(10);
	_vm->_displayMan->drawFloorAndCeiling();
	_vm->_eventMan->_primaryMouseInput = _vm->_eventMan->_primaryMouseInputInterface;
	_vm->_eventMan->_secondaryMouseInput = _vm->_eventMan->_secondaryMouseInputMovement;
	_vm->_eventMan->_primaryKeyboardInput = _vm->_eventMan->_primaryKeyboardInputInterface;
	_vm->_eventMan->_secondaryKeyboardInput = _vm->_eventMan->_secondaryKeyboardInputMovement;
	_vm->_eventMan->discardAllInput();
	_vm->_menuMan->f457_drawEnabledMenus();
}

int16 ChampionMan::getThrowingStaminaCost(Thing thing) {
	int16 weight = _vm->_dungeonMan->getObjectWeight(thing) >> 1;
	int16 staminaCost = getBoundedValue<int16>(1, weight, 10);

	while ((weight -= 10) > 0)
		staminaCost += weight >> 1;

	return staminaCost;
}

void ChampionMan::disableAction(uint16 champIndex, uint16 ticks) {
	Champion *curChampion = &_champions[champIndex];
	int32 updatedEnableActionEventTime = _vm->_gameTime + ticks;

	TimelineEvent curEvent;
	curEvent._type = k11_TMEventTypeEnableChampionAction;
	curEvent._priority = champIndex;
	curEvent._B._slotOrdinal = 0;

	int16 eventIndex = curChampion->_enableActionEventIndex;
	if (eventIndex >= 0) {
		int32 currentEnableActionEventTime = filterTime(_vm->_timeline->_g370_events[eventIndex]._mapTime);
		if (updatedEnableActionEventTime >= currentEnableActionEventTime) {
			updatedEnableActionEventTime += (currentEnableActionEventTime - _vm->_gameTime) >> 1;
		} else {
			updatedEnableActionEventTime = currentEnableActionEventTime + (ticks >> 1);
		}
		_vm->_timeline->f237_deleteEvent(eventIndex);
	} else {
		setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand | k0x0008_ChampionAttributeDisableAction);
		drawChampionState((ChampionIndex)champIndex);
	}
	setMapAndTime(curEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, updatedEnableActionEventTime);
	curChampion->_enableActionEventIndex = _vm->_timeline->f238_addEventGetEventIndex(&curEvent);
}

void ChampionMan::addSkillExperience(uint16 champIndex, uint16 skillIndex, uint16 exp) {
	if ((skillIndex >= k4_ChampionSkillSwing) && (skillIndex <= k11_ChampionSkillShoot) && (_vm->_projexpl->_g361_lastCreatureAttackTime < _vm->_gameTime - 150))
		exp >>= 1;

	if (exp) {
		if (_vm->_dungeonMan->_currMap->_difficulty)
			exp *= _vm->_dungeonMan->_currMap->_difficulty;

		Champion *curChampion = &_champions[champIndex];
		uint16 baseSkillIndex;
		if (skillIndex >= k4_ChampionSkillSwing)
			baseSkillIndex = (skillIndex - k4_ChampionSkillSwing) >> 2;
		else
			baseSkillIndex = skillIndex;

		uint16 skillLevelBefore = getSkillLevel(champIndex, baseSkillIndex | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience));

		if ((skillIndex >= k4_ChampionSkillSwing) && (_vm->_projexpl->_g361_lastCreatureAttackTime > _vm->_gameTime - 25))
			exp <<= 1;

		Skill *curSkill = &curChampion->_skills[skillIndex];
		curSkill->_experience += exp;
		if (curSkill->_temporaryExperience < 32000)
			curSkill->_temporaryExperience += getBoundedValue(1, exp >> 3, 100);

		curSkill = &curChampion->_skills[baseSkillIndex];
		if (skillIndex >= k4_ChampionSkillSwing)
			curSkill->_experience += exp;

		uint16 skillLevelAfter = getSkillLevel(champIndex, baseSkillIndex | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience));
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
			drawChampionState((ChampionIndex)champIndex);
			_vm->_textMan->f51_messageAreaPrintLineFeed();
			Color curChampionColor = _championColor[champIndex];
			_vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, curChampion->_name);

			switch (_vm->getGameLanguage()) { // localized
			default:
			case Common::EN_ANY: _vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " JUST GAINED A "); break;
			case Common::DE_DEU: _vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " HAT SOEBEN STUFE"); break;
			case Common::FR_FRA: _vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " VIENT DE DEVENIR "); break;
			}

			_vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, _baseSkillName[baseSkillIndex]);

			switch (_vm->getGameLanguage()) { // localized
			default:
			case Common::EN_ANY: _vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, "!"); break;
			case Common::DE_DEU: _vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " LEVEL!"); break;
			case Common::FR_FRA: _vm->_textMan->f47_messageAreaPrintMessage(curChampionColor, " ERREICHT!"); break;
			}
		}
	}
}

int16 ChampionMan::getDamagedChampionCount(uint16 attack, int16 wounds, int16 attackType) {
	int16 randomMax = (attack >> 3) + 1;
	uint16 reducedAttack = attack - randomMax;
	randomMax <<= 1;

	int16 damagedChampionCount = 0;
	for (int16 championIndex = k0_ChampionFirst; championIndex < _partyChampionCount; championIndex++) {
		// Actual attack is attack +/- (attack / 8)
		if (addPendingDamageAndWounds_getDamage(championIndex, MAX(1, reducedAttack + _vm->getRandomNumber(randomMax)), wounds, attackType))
			damagedChampionCount++;
	}

	return damagedChampionCount;
}

int16 ChampionMan::getTargetChampionIndex(int16 mapX, int16 mapY, uint16 cell) {
	if (_partyChampionCount && (getDistance(mapX, mapY, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY) <= 1)) {
		signed char orderedCellsToAttack[4];
		_vm->_groupMan->f229_setOrderedCellsToAttack(orderedCellsToAttack, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, mapX, mapY, cell);
		for (uint16 i = 0; i < 4; i++) {
			int16 championIndex = getIndexInCell(orderedCellsToAttack[i]);
			if (championIndex >= 0)
				return championIndex;
		}
	}
	return kM1_ChampionNone;
}

int16 ChampionMan::getDexterity(Champion* champ) {
	int16 dexterity = _vm->getRandomNumber(8) + champ->_statistics[k2_ChampionStatDexterity][k1_ChampionStatCurrent];
	dexterity -= ((int32)(dexterity >> 1) * (int32)champ->_load) / getMaximumLoad(champ);
	if (_partyIsSleeping)
		dexterity >>= 1;

	return getBoundedValue(1 + _vm->getRandomNumber(8), dexterity >> 1, 100 - _vm->getRandomNumber(8));
}

bool ChampionMan::isLucky(Champion* champ, uint16 percentage) {
	if (_vm->getRandomNumber(2) && (_vm->getRandomNumber(100) > percentage))
		return true;

	unsigned char *curStat = champ->_statistics[k0_ChampionStatLuck];
	bool isLucky = (_vm->getRandomNumber(curStat[k1_ChampionStatCurrent]) > percentage);
	curStat[k1_ChampionStatCurrent] = getBoundedValue<char>(curStat[k2_ChampionStatMinimum], curStat[k1_ChampionStatCurrent] + (isLucky ? -2 : 2), curStat[k0_ChampionStatMaximum]);
	return isLucky;
}

void ChampionMan::championPoison(int16 champIndex, uint16 attack) {
	if ((champIndex == kM1_ChampionNone) || (_vm->indexToOrdinal(champIndex) == _candidateChampionOrdinal))
		return;

	Champion *curChampion = &_champions[champIndex];
	addPendingDamageAndWounds_getDamage(champIndex, MAX(1, attack >> 6), k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
	setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
	if ((_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) && (_vm->_inventoryMan->_g424_panelContent == k0_PanelContentFoodWaterPoisoned)) {
		setFlag(curChampion->_attributes, k0x0800_ChampionAttributePanel);
	}

	if (--attack) {
		curChampion->_poisonEventCount++;
		TimelineEvent newEvent;
		newEvent._type = k75_TMEventTypePoisonChampion;
		newEvent._priority = champIndex;
		setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 36);
		newEvent._B._attack = attack;
		_vm->_timeline->f238_addEventGetEventIndex(&newEvent);
	}

	drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::setPartyDirection(int16 dir) {
	if (dir == _vm->_dungeonMan->_partyDir)
		return;

	int16 L0834_i_Delta = dir - _vm->_dungeonMan->_partyDir;
	if (L0834_i_Delta < 0)
		L0834_i_Delta += 4;

	Champion *curChampion = _champions;
	for (int16 i = k0_ChampionFirst; i < _partyChampionCount; i++) {
		curChampion->_cell = (ViewCell)normalizeModulo4(curChampion->_cell + L0834_i_Delta);
		curChampion->_dir = (Direction)normalizeModulo4(curChampion->_dir + L0834_i_Delta);
		curChampion++;
	}

	_vm->_dungeonMan->_partyDir = (Direction)dir;
	drawChangedObjectIcons();
}

void ChampionMan::deleteScent(uint16 scentIndex) {
	uint16 count = --_party._scentCount - scentIndex;

	if (count) {
		for (uint16 i = 0; i < count; ++i) {
			_party._scents[scentIndex + i] = _party._scents[scentIndex + i + 1];
			_party._scentStrengths[scentIndex + i] = _party._scentStrengths[scentIndex + i + 1];
		}
	}

	if (scentIndex < _party._firstScentIndex)
		_party._firstScentIndex--;

	if (scentIndex < _party._lastScentIndex)
		_party._lastScentIndex--;
}

void ChampionMan::addScentStrength(int16 mapX, int16 mapY, int32 cycleCount) {
	int16 scentIndex = _party._scentCount;
	if (scentIndex) {
		bool mergeFl = getFlag(cycleCount, k0x8000_mergeCycles);
		if (mergeFl)
			clearFlag(cycleCount, k0x8000_mergeCycles);

		Scent newScent; /* BUG0_00 Useless code */
		newScent.setMapX(mapX); /* BUG0_00 Useless code */
		newScent.setMapY(mapY); /* BUG0_00 Useless code */
		newScent.setMapIndex(_vm->_dungeonMan->_currMapIndex); /* BUG0_00 Useless code */

		Scent *curScent = _party._scents; /* BUG0_00 Useless code */
		bool cycleCountDefined = false;
		while (scentIndex--) {
			if (&*curScent++ == &newScent) {
				if (!cycleCountDefined) {
					cycleCountDefined = true;
					if (mergeFl) {
						cycleCount = MAX<int32>(_party._scentStrengths[scentIndex], cycleCount);
					} else {
						cycleCount = MIN<int32>(80, _party._scentStrengths[scentIndex] + cycleCount);
					}
				}
				_party._scentStrengths[scentIndex] = cycleCount;
			}
		}
	}
}

void ChampionMan::putObjectInLeaderHand(Thing thing, bool setMousePointer) {
	if (thing == Thing::_none)
		return;

	_leaderEmptyHanded = false;
	_vm->_objectMan->f36_extractIconFromBitmap(_leaderHandObjectIconIndex = _vm->_objectMan->f33_getIconIndex(_leaderHandObject = thing), _vm->_objectMan->_g412_objectIconForMousePointer);
	_vm->_eventMan->showMouse();
	_vm->_objectMan->f34_drawLeaderObjectName(thing);

	if (setMousePointer)
		_vm->_setMousePointerToObjectInMainLoop = true;
	else
		_vm->_eventMan->setPointerToObject(_vm->_objectMan->_g412_objectIconForMousePointer);

	_vm->_eventMan->hideMouse();
	if (_leaderIndex != kM1_ChampionNone) {
		_champions[_leaderIndex]._load += _vm->_dungeonMan->getObjectWeight(thing);
		setFlag(_champions[_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
		drawChampionState(_leaderIndex);
	}
}

int16 ChampionMan::getMovementTicks(Champion *champ) {
	uint16 maximumLoad = getMaximumLoad(champ);
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

bool ChampionMan::isAmmunitionCompatibleWithWeapon(uint16 champIndex, uint16 weaponSlotIndex, uint16 ammunitionSlotIndex) {
	Champion *curChampion = &_champions[champIndex];
	Thing curThing = curChampion->_slots[weaponSlotIndex];
	if (curThing.getType() != k5_WeaponThingType)
		return false;

	WeaponInfo *weaponInfo = _vm->_dungeonMan->getWeaponInfo(curThing);
	int16 weaponClass = kM1_WeaponClassNone;

	if ((weaponInfo->_class >= k16_WeaponClassFirstBow) && (weaponInfo->_class <= k31_WeaponClassLastBow))
		weaponClass = k10_WeaponClassBowAmmunition;
	else if ((weaponInfo->_class >= k32_WeaponClassFirstSling) && (weaponInfo->_class <= k47_WeaponClassLastSling))
		weaponClass = k11_WeaponClassSlingAmmunition;

	if (weaponClass == kM1_WeaponClassNone)
		return false;

	curThing = curChampion->_slots[ammunitionSlotIndex];
	weaponInfo = _vm->_dungeonMan->getWeaponInfo(curThing);
	return ((curThing.getType() == k5_WeaponThingType) && (weaponInfo->_class == weaponClass));
}

void ChampionMan::drawAllChampionStates() {
	for (int16 i = k0_ChampionFirst; i < _partyChampionCount; i++)
		drawChampionState((ChampionIndex)i);
}

void ChampionMan::viAltarRebirth(uint16 champIndex) {
	Champion *curChampion = &_champions[champIndex];
	if (getIndexInCell(curChampion->_cell) != kM1_ChampionNone) {
		uint16 numCell = k0_CellNorthWest;
		while (getIndexInCell(numCell) != kM1_ChampionNone)
			numCell++;

		curChampion->_cell = (ViewCell)numCell;
	}

	uint16 maximumHealth = curChampion->_maxHealth;
	curChampion->_maxHealth = MAX(25, maximumHealth - (maximumHealth >> 6) - 1);
	curChampion->_currHealth = curChampion->_maxHealth >> 1;
	_vm->_menuMan->f393_drawSpellAreaControls(_magicCasterChampionIndex);
	curChampion->_dir = _vm->_dungeonMan->_partyDir;
	setFlag(curChampion->_attributes, k0x8000_ChampionAttributeActionHand | k0x1000_ChampionAttributeStatusBox | k0x0400_ChampionAttributeIcon);
	drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::clickOnSlotBox(uint16 slotBoxIndex) {
	uint16 champIndex;
	uint16 slotIndex;

	if (slotBoxIndex < k8_SlotBoxInventoryFirstSlot) {
		if (_candidateChampionOrdinal)
			return;

		champIndex = slotBoxIndex >> 1;
		if ((champIndex >= _partyChampionCount) || (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) || !_champions[champIndex]._currHealth)
			return;

		slotIndex = getHandSlotIndex(slotBoxIndex);
	} else {
		champIndex = _vm->ordinalToIndex(_vm->_inventoryMan->_g432_inventoryChampionOrdinal);
		slotIndex = slotBoxIndex - k8_SlotBoxInventoryFirstSlot;
	}

	Thing leaderHandObject = _leaderHandObject;
	Thing slotThing;
	if (slotIndex >= k30_ChampionSlotChest_1) {
		slotThing = _vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1];
	} else {
		slotThing = _champions[champIndex]._slots[slotIndex];
	}

	if ((slotThing == Thing::_none) && (leaderHandObject == Thing::_none))
		return;

	if ((leaderHandObject != Thing::_none) && (!(_vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->getObjectInfoIndex(leaderHandObject)]._allowedSlots & _slotMasks[slotIndex])))
		return;

	_vm->_eventMan->showMouse();
	if (leaderHandObject != Thing::_none)
		getObjectRemovedFromLeaderHand();

	if (slotThing != Thing::_none) {
		getObjectRemovedFromSlot(champIndex, slotIndex);
		putObjectInLeaderHand(slotThing, false);
	}

	if (leaderHandObject != Thing::_none)
		addObjectInSlot((ChampionIndex)champIndex, leaderHandObject, (ChampionSlot)slotIndex);

	drawChampionState((ChampionIndex)champIndex);
	_vm->_eventMan->hideMouse();
}

bool ChampionMan::isProjectileSpellCast(uint16 champIndex, Thing thing, int16 kineticEnergy, uint16 requiredManaAmount) {
	Champion *curChampion = &_champions[champIndex];
	if (curChampion->_currMana < requiredManaAmount)
		return false;

	curChampion->_currMana -= requiredManaAmount;
	setFlag(curChampion->_attributes, k0x0100_ChampionAttributeStatistics);
	int16 stepEnergy = 10 - MIN(8, curChampion->_maxMana >> 3);
	if (kineticEnergy < (stepEnergy << 2)) {
		kineticEnergy += 3;
		stepEnergy--;
	}

	championShootProjectile(curChampion, thing, kineticEnergy, 90, stepEnergy);
	return true; // fix BUG_01
}

void ChampionMan::championShootProjectile(Champion* champ, Thing thing, int16 kineticEnergy, int16 attack, int16 stepEnergy) {
	Direction newDirection = champ->_dir;
	_vm->_projexpl->f212_projectileCreate(thing, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, normalizeModulo4((((champ->_cell - newDirection + 1) & 0x0002) >> 1) + newDirection), newDirection, kineticEnergy, attack, stepEnergy);
	_vm->_projectileDisableMovementTicks = 4;
	_vm->_lastProjectileDisabledMovementDirection = newDirection;
}

void ChampionMan::applyAndDrawPendingDamageAndWounds() {
	Champion *championPtr = _champions;
	for (uint16 championIndex = k0_ChampionFirst; championIndex < _partyChampionCount; championIndex++, championPtr++) {
		int16 pendingWounds = _championPendingWounds[championIndex];
		setFlag(championPtr->_wounds, pendingWounds);
		_championPendingWounds[championIndex] = 0;
		uint16 pendingDamage = _championPendingDamage[championIndex];
		if (!pendingDamage)
			continue;

		_championPendingDamage[championIndex] = 0;
		int16 curHealth = championPtr->_currHealth;
		if (!curHealth)
			continue;

		// DEBUG CODE
		if (_vm->_console->_debugGodmodeHP == false)
			curHealth -= pendingDamage;

		if (curHealth <= 0) {
			championKill(championIndex);
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
			_vm->_eventMan->showMouse();

			if (_vm->indexToOrdinal(championIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
				blitBox._y2 = 28;
				blitBox._x1 = textPosX + 7;
				blitBox._x2 = blitBox._x1 + 31; /* Box is over the champion portrait in the status box */
				_vm->_displayMan->blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(k16_damageToChampionBig), &blitBox, k16_byteWidth, k10_ColorFlesh, 29);
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
				_vm->_displayMan->blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(k15_damageToChampionSmallIndice), &blitBox, k24_byteWidth, k10_ColorFlesh, 7);
				// Check the number of digits and sets the position accordingly.
				if (pendingDamage < 10) // 1 digit
					textPosX += 19;
				else if (pendingDamage < 100) // 2 digits
					textPosX += 16;
				else //3 digits
					textPosX += 13;

				textPosY = 5;
			}
			_vm->_textMan->f53_printToLogicalScreen(textPosX, textPosY, k15_ColorWhite, k8_ColorRed, getStringFromInteger(pendingDamage, false, 3).c_str());

			int16 eventIndex = championPtr->_hideDamageReceivedIndex;
			if (eventIndex == -1) {
				TimelineEvent newEvent;
				newEvent._type = k12_TMEventTypeHideDamageReceived;
				setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 5);
				newEvent._priority = championIndex;
				championPtr->_hideDamageReceivedIndex = _vm->_timeline->f238_addEventGetEventIndex(&newEvent);
			} else {
				TimelineEvent *curEvent = &_vm->_timeline->_g370_events[eventIndex];
				setMapAndTime(curEvent->_mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 5);
				_vm->_timeline->f236_fixChronology(_vm->_timeline->f235_getIndex(eventIndex));
			}
			drawChampionState((ChampionIndex)championIndex);
			_vm->_eventMan->hideMouse();
		}
	}
}

void ChampionMan::championKill(uint16 champIndex) {
	Champion *curChampion = &_champions[champIndex];
	curChampion->_currHealth = 0;
	setFlag(curChampion->_attributes, k0x1000_ChampionAttributeStatusBox);
	if (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
		if (_vm->_pressingEye) {
			_vm->_pressingEye = false;
			_vm->_eventMan->_ignoreMouseMovements = false;
			if (!_leaderEmptyHanded) {
				_vm->_objectMan->f34_drawLeaderObjectName(_leaderHandObject);
			}
			_vm->_eventMan->_hideMousePointerRequestCount = 1;
			_vm->_eventMan->hideMouse();
		} else if (_vm->_pressingMouth) {
			_vm->_pressingMouth = false;
			_vm->_eventMan->_ignoreMouseMovements = false;
			_vm->_eventMan->_hideMousePointerRequestCount = 1;
			_vm->_eventMan->hideMouse();
		}
		_vm->_inventoryMan->f355_toggleInventory(k4_ChampionCloseInventory);
	}
	dropAllObjects(champIndex);
	Thing unusedThing = _vm->_dungeonMan->getUnusedThing(k0x8000_championBones | k10_JunkThingType);
	uint16 curCell = 0;
	if (unusedThing != Thing::_none) {
		Junk *L0966_ps_Junk = (Junk *)_vm->_dungeonMan->getThingData(unusedThing);
		L0966_ps_Junk->setType(k5_JunkTypeBones);
		L0966_ps_Junk->setDoNotDiscard(true);
		L0966_ps_Junk->setChargeCount(champIndex);
		curCell = curChampion->_cell;
		_vm->_moveSens->f267_getMoveResult(thingWithNewCell(unusedThing, curCell), kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY);
	}
	curChampion->_symbolStep = 0;
	curChampion->_symbols[0] = '\0';
	curChampion->_dir = _vm->_dungeonMan->_partyDir;
	curChampion->_maximumDamageReceived = 0;
	uint16 curChampionIconIndex = getChampionIconIndex(curCell, _vm->_dungeonMan->_partyDir);
	if (_vm->indexToOrdinal(curChampionIconIndex) == _vm->_eventMan->_useChampionIconOrdinalAsMousePointerBitmap) {
		_vm->_eventMan->_mousePointerBitmapUpdated = true;
		_vm->_eventMan->_useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(kM1_ChampionNone);
	}

	if (curChampion->_poisonEventCount)
		unpoison(champIndex);

	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_displayMan->fillScreenBox(_boxChampionIcons[curChampionIconIndex], k0_ColorBlack);
	drawChampionState((ChampionIndex)champIndex);

	int16 aliveChampionIndex;
	for (aliveChampionIndex = k0_ChampionFirst, curChampion = _champions; aliveChampionIndex < _partyChampionCount; aliveChampionIndex++, curChampion++) {
		if (curChampion->_currHealth)
			break;
	}

	if (aliveChampionIndex == _partyChampionCount) { /* BUG0_43 The game does not end if the last living champion in the party is killed while looking at a candidate champion in a portrait. The condition to end the game when the whole party is killed is not true because the code considers the candidate champion as alive (in the loop above) */
		_partyDead = true;
		return;
	}

	if (champIndex == _leaderIndex)
		_vm->_eventMan->commandSetLeader((ChampionIndex)aliveChampionIndex);

	if (champIndex == _magicCasterChampionIndex)
		_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(aliveChampionIndex);
	else
		_vm->_menuMan->f393_drawSpellAreaControls(_magicCasterChampionIndex);
}

void ChampionMan::dropAllObjects(uint16 champIndex) {
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

	uint16 curCell = _champions[champIndex]._cell;
	for (uint16 slotIndex = k0_ChampionSlotReadyHand; slotIndex < k30_ChampionSlotChest_1; slotIndex++) {
		Thing curThing = getObjectRemovedFromSlot(champIndex, slotDropOrder[slotIndex]);
		if (curThing != Thing::_none)
			_vm->_moveSens->f267_getMoveResult(thingWithNewCell(curThing, curCell), kM1_MapXNotOnASquare, 0, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY);
	}
}

void ChampionMan::unpoison(int16 champIndex) {
	if (champIndex == kM1_ChampionNone)
		return;

	TimelineEvent *eventPtr = _vm->_timeline->_g370_events;
	for (uint16 eventIndex = 0; eventIndex < _vm->_timeline->_g369_eventMaxCount; eventPtr++, eventIndex++) {
		if ((eventPtr->_type == k75_TMEventTypePoisonChampion) && (eventPtr->_priority == champIndex))
			_vm->_timeline->f237_deleteEvent(eventIndex);
	}
	_champions[champIndex]._poisonEventCount = 0;
}

void ChampionMan::applyTimeEffects() {
	if (!_partyChampionCount)
		return;

	Scent checkScent;
	checkScent.setMapX(_vm->_dungeonMan->_partyMapX);
	checkScent.setMapY(_vm->_dungeonMan->_partyMapY);
	checkScent.setMapIndex(_vm->_dungeonMan->_partyMapIndex);

	for (byte loopScentIndex = 0; loopScentIndex + 1 < _party._scentCount; loopScentIndex++) {
		if (&_party._scents[loopScentIndex] != &checkScent) {
			_party._scentStrengths[loopScentIndex] = MAX(0, _party._scentStrengths[loopScentIndex] - 1);
			if (!_party._scentStrengths[loopScentIndex] && !loopScentIndex) {
				deleteScent(0);
				continue;
			}
		}
	}

	uint16 gameTime = _vm->_gameTime & 0xFFFF;
	uint16 timeCriteria = (((gameTime & 0x0080) + ((gameTime & 0x0100) >> 2)) + ((gameTime & 0x0040) << 2)) >> 2;
	Champion *championPtr = _champions;
	for (uint16 championIndex = k0_ChampionFirst; championIndex < _partyChampionCount; championIndex++, championPtr++) {
		if (championPtr->_currHealth && (_vm->indexToOrdinal(championIndex) != _candidateChampionOrdinal)) {
			uint16 wizardSkillLevel = getSkillLevel(championIndex, k3_ChampionSkillWizard) + getSkillLevel(championIndex, k2_ChampionSkillPriest);
			if ((championPtr->_currMana < championPtr->_maxMana)
				&& (timeCriteria < championPtr->_statistics[k3_ChampionStatWisdom][k1_ChampionStatCurrent] + wizardSkillLevel)) {
				int16 manaGain = championPtr->_maxMana / 40;
				if (_partyIsSleeping)
					manaGain <<= 1;

				manaGain++;
				decrementStamina(championIndex, manaGain * MAX(7, 16 - wizardSkillLevel));
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
			int16 staminaAmount = getBoundedValue(1, (championPtr->_maxStamina >> 8) - 1, 6);
			if (_partyIsSleeping)
				staminaAmount <<= 1;

			int32 compDelay = _vm->_gameTime - _vm->_projexpl->_g362_lastPartyMovementTime;
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
			decrementStamina(championIndex, staminaLoss);
			if (championPtr->_food < -1024)
				championPtr->_food = -1024;

			if (championPtr->_water < -1024)
				championPtr->_water = -1024;

			if ((championPtr->_currHealth < championPtr->_maxHealth) && (championPtr->_currStamina >= (championPtr->_maxStamina >> 2)) && (timeCriteria < (championPtr->_statistics[k4_ChampionStatVitality][k1_ChampionStatCurrent] + 12))) {
				int16 healthGain = (championPtr->_maxHealth >> 7) + 1;
				if (_partyIsSleeping)
					healthGain <<= 1;

				if (_vm->_objectMan->f33_getIconIndex(championPtr->_slots[k10_ChampionSlotNeck]) == k121_IconIndiceJunkEkkhardCross)
					healthGain += (healthGain >> 1) + 1;

				championPtr->_currHealth += MIN(healthGain, (int16)(championPtr->_maxHealth - championPtr->_currHealth));
			}
			if (!((int)_vm->_gameTime & (_partyIsSleeping ? 63 : 255))) {
				for (uint16 i = k0_ChampionStatLuck; i <= k6_ChampionStatAntifire; i++) {
					byte *curStatistic = championPtr->_statistics[i];
					uint16 statisticMaximum = curStatistic[k0_ChampionStatMaximum];
					if (curStatistic[k1_ChampionStatCurrent] < statisticMaximum)
						curStatistic[k1_ChampionStatCurrent]++;
					else if (curStatistic[k1_ChampionStatCurrent] > statisticMaximum)
						curStatistic[k1_ChampionStatCurrent] -= curStatistic[k1_ChampionStatCurrent] / statisticMaximum;
				}
			}
			if (!_partyIsSleeping && (championPtr->_dir != _vm->_dungeonMan->_partyDir) && (_vm->_projexpl->_g361_lastCreatureAttackTime + 60 < _vm->_gameTime)) {
				championPtr->_dir = _vm->_dungeonMan->_partyDir;
				championPtr->_maximumDamageReceived = 0;
				setFlag(championPtr->_attributes, k0x0400_ChampionAttributeIcon);
			}
			setFlag(championPtr->_attributes, k0x0100_ChampionAttributeStatistics);
			if (_vm->indexToOrdinal(championIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal) {
				if (_vm->_pressingMouth || _vm->_pressingEye || (_vm->_inventoryMan->_g424_panelContent == k0_PanelContentFoodWaterPoisoned)) {
					setFlag(championPtr->_attributes, k0x0800_ChampionAttributePanel);
				}
			}
		}
	}
	drawAllChampionStates();
}

void ChampionMan::savePartyPart2(Common::OutSaveFile* file) {
	for (uint16 i = 0; i < 4; ++i) {
		Champion *champ = &_champions[i];
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

	Party &party = _party;
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

void ChampionMan::loadPartyPart2(Common::InSaveFile* file) {
	for (uint16 i = 0; i < 4; ++i) {
		Champion *champ = &_champions[i];
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

	Party &party = _party;
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

ChampionIndex ChampionMan::getIndexInCell(int16 cell) {
	for (uint16 i = 0; i < _partyChampionCount; ++i) {
		if ((_champions[i]._cell == cell) && _champions[i]._currHealth)
			return (ChampionIndex)i;
	}

	return kM1_ChampionNone;
}

void ChampionMan::resetDataToStartGame() {
	if (!_vm->_newGameFl) {
		Thing L0787_T_Thing;
		if ((L0787_T_Thing = _leaderHandObject) == Thing::_none) {
			_leaderEmptyHanded = true;
			_leaderHandObjectIconIndex = kM1_IconIndiceNone;
			_vm->_eventMan->setMousePointer();
		} else {
			putObjectInLeaderHand(L0787_T_Thing, true); /* This call will add the weight of the leader hand object to the Load of the leader a first time */
		}
		Champion *L0788_ps_Champion = _champions;
		int16 L0785_i_ChampionIndex;
		for (L0785_i_ChampionIndex = k0_ChampionFirst; L0785_i_ChampionIndex < _partyChampionCount; L0785_i_ChampionIndex++, L0788_ps_Champion++) {
			clearFlag(L0788_ps_Champion->_attributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand);
			setFlag(L0788_ps_Champion->_attributes, k0x8000_ChampionAttributeActionHand | k0x1000_ChampionAttributeStatusBox | k0x0400_ChampionAttributeIcon);
		}
		drawAllChampionStates();
		if ((L0785_i_ChampionIndex = _leaderIndex) != kM1_ChampionNone) {
			_leaderIndex = kM1_ChampionNone;
			_vm->_eventMan->commandSetLeader((ChampionIndex)L0785_i_ChampionIndex);
		}
		if ((L0785_i_ChampionIndex = _magicCasterChampionIndex) != kM1_ChampionNone) {
			_magicCasterChampionIndex = kM1_ChampionNone;
			_vm->_menuMan->f394_setMagicCasterAndDrawSpellArea(L0785_i_ChampionIndex);
		}
		return;
	}

	_leaderHandObject = Thing::_none;
	_leaderHandObjectIconIndex = kM1_IconIndiceNone;
	_leaderEmptyHanded = true;
}

void ChampionMan::addCandidateChampionToParty(uint16 championPortraitIndex) {
	if (!_leaderEmptyHanded)
		return;

	if (_partyChampionCount == 4)
		return;

	uint16 previousPartyChampionCount = _partyChampionCount;
	Champion *championPtr = &_champions[previousPartyChampionCount];
	championPtr->resetToZero();
	// Strangerke - TODO: Check if the new code is possible to run on the older version (example: the portraits could be missing in the data)
	_vm->_displayMan->_useByteBoxCoordinates = true;
	_vm->_displayMan->blitToBitmap(_vm->_displayMan->getNativeBitmapOrGraphic(k26_ChampionPortraitsIndice), championPtr->_portrait, _boxChampionPortrait, getChampionPortraitX(championPortraitIndex), getChampionPortraitY(championPortraitIndex), k128_byteWidth, k16_byteWidth, kM1_ColorNoTransparency, 87, 29);
	championPtr->_actionIndex = k255_ChampionActionNone;
	championPtr->_enableActionEventIndex = -1;
	championPtr->_hideDamageReceivedIndex = -1;
	championPtr->_dir = _vm->_dungeonMan->_partyDir;
	uint16 viewCell = k0_ViewCellFronLeft;
	while (getIndexInCell(normalizeModulo4(viewCell + _vm->_dungeonMan->_partyDir)) != kM1_ChampionNone)
		viewCell++;

	championPtr->_cell = (ViewCell)normalizeModulo4(viewCell + _vm->_dungeonMan->_partyDir);
	championPtr->_attributes = k0x0400_ChampionAttributeIcon;
	championPtr->_directionMaximumDamageReceived = _vm->_dungeonMan->_partyDir;
	championPtr->_food = 1500 + _vm->getRandomNumber(256);
	championPtr->_water = 1500 + _vm->getRandomNumber(256);
	for (int16 slotIdx = k0_ChampionSlotReadyHand; slotIdx < k30_ChampionSlotChest_1; slotIdx++)
		championPtr->_slots[slotIdx] = Thing::_none;

	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY);
	while (curThing.getType() != k2_TextstringType)
		curThing = _vm->_dungeonMan->getNextThing(curThing);

	char L0807_ac_DecodedChampionText[77];
	char *decodedStringPtr = L0807_ac_DecodedChampionText;
	_vm->_dungeonMan->decodeText(decodedStringPtr, curThing, (TextType)(k2_TextTypeScroll | k0x8000_DecodeEvenIfInvisible));

	uint16 charIdx = 0;
	char tmpChar;
	while ((tmpChar = *decodedStringPtr++) != '\n')
		championPtr->_name[charIdx++] = tmpChar;

	championPtr->_name[charIdx] = '\0';
	charIdx = 0;
	bool championTitleCopiedFl = false;
	for (;;) { /*_Infinite loop_*/
		tmpChar = *decodedStringPtr++;
		if (tmpChar == '\n') { /* New line */
			if (championTitleCopiedFl)
				break;
			championTitleCopiedFl = true;
		} else
			championPtr->_title[charIdx++] = tmpChar;
	}
	championPtr->_title[charIdx] = '\0';
	if (*decodedStringPtr++ == 'M')
		setFlag(championPtr->_attributes, k0x0010_ChampionAttributeMale);

	decodedStringPtr++;
	championPtr->_currHealth = championPtr->_maxHealth = getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	championPtr->_currStamina = championPtr->_maxStamina = getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	championPtr->_currMana = championPtr->_maxMana = getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	decodedStringPtr++;
	for (int16 statIdx = k0_ChampionStatLuck; statIdx <= k6_ChampionStatAntifire; statIdx++) {
		championPtr->_statistics[statIdx][k2_ChampionStatMinimum] = 30;
		championPtr->_statistics[statIdx][k1_ChampionStatCurrent] = championPtr->_statistics[statIdx][k0_ChampionStatMaximum] = getDecodedValue(decodedStringPtr, 2);
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
	_candidateChampionOrdinal = previousPartyChampionCount + 1;
	if (++_partyChampionCount == 1) {
		_vm->_eventMan->commandSetLeader(k0_ChampionFirst);
		_vm->_menuMan->_g508_refreshActionArea = true;
	} else {
		_vm->_menuMan->f388_clearActingChampion();
		_vm->_menuMan->f386_drawActionIcon((ChampionIndex)(_partyChampionCount - 1));
	}

	int16 curMapX = _vm->_dungeonMan->_partyMapX;
	int16 curMapY = _vm->_dungeonMan->_partyMapY;
	uint16 championObjectsCell = returnOppositeDir(_vm->_dungeonMan->_partyDir);
	curMapX += _vm->_dirIntoStepCountEast[_vm->_dungeonMan->_partyDir], curMapY += _vm->_dirIntoStepCountNorth[_vm->_dungeonMan->_partyDir];
	curThing = _vm->_dungeonMan->getSquareFirstThing(curMapX, curMapY);
	int16 slotIdx = k13_ChampionSlotBackpackLine_1_1;
	while (curThing != Thing::_endOfList) {
		ThingType thingType = curThing.getType();
		if ((thingType > k3_SensorThingType) && (curThing.getCell() == championObjectsCell)) {
			int16 objectAllowedSlots = _vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->getObjectInfoIndex(curThing)]._allowedSlots;
			uint16 curSlotIndex = k0_ChampionSlotReadyHand;
			switch (thingType) {
			case k6_ArmourThingType: {
				bool skipCheck = false;
				for (curSlotIndex = k2_ChampionSlotHead; curSlotIndex <= k5_ChampionSlotFeet; curSlotIndex++) {
					if (objectAllowedSlots & _slotMasks[curSlotIndex]) {
						skipCheck = true;
						break;
					}
				}

				if (skipCheck)
					break;

				if ((objectAllowedSlots & _slotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;

				break;
			}
			case k5_WeaponThingType:
				if (championPtr->_slots[k1_ChampionSlotActionHand] == Thing::_none)
					curSlotIndex = k1_ChampionSlotActionHand;
				else if ((objectAllowedSlots & _slotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
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
				else if ((objectAllowedSlots & _slotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;
				break;
			case k9_ContainerThingType:
			case k10_JunkThingType:
				if ((objectAllowedSlots & _slotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;

				break;
			default:
				break;
			}

			while (championPtr->_slots[curSlotIndex] != Thing::_none) {
				if ((objectAllowedSlots & _slotMasks[k10_ChampionSlotNeck]) && (championPtr->_slots[k10_ChampionSlotNeck] == Thing::_none))
					curSlotIndex = k10_ChampionSlotNeck;
				else
					curSlotIndex = slotIdx++;
			}
			addObjectInSlot((ChampionIndex)previousPartyChampionCount, curThing, (ChampionSlot)curSlotIndex);
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
	_vm->_inventoryMan->f355_toggleInventory((ChampionIndex)previousPartyChampionCount);
	_vm->_menuMan->f456_drawDisabledMenu();;
}

void ChampionMan::drawChampionBarGraphs(ChampionIndex champIndex) {
	int16 barGraphHeights[3];
	Champion *champ = &_champions[champIndex];
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
	_vm->_eventMan->showMouse();

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
			_vm->_displayMan->fillScreenBox(box, k12_ColorDarkestGray);
		}
		if (barGraphHeight) {
			box._y1 = 27 - barGraphHeight;
			box._y2 = 26;
			_vm->_displayMan->fillScreenBox(box, _championColor[champIndex]);
		}
		box._x1 += 7;
		box._x2 += 7;
	}
	_vm->_eventMan->hideMouse();
}


uint16 ChampionMan::getStaminaAdjustedValue(Champion *champ, int16 val) {
	int16 currStamina = champ->_currStamina;
	int16 halfMaxStamina = champ->_maxStamina / 2;
	if (currStamina < halfMaxStamina) {
		val /= 2;
		return val + ((uint32)val * (uint32)currStamina) / halfMaxStamina;
	}
	return val;
}

uint16 ChampionMan::getMaximumLoad(Champion *champ) {
	uint16 maximumLoad = champ->getStatistic(k1_ChampionStatStrength, k1_ChampionStatCurrent) * 8 + 100;
	maximumLoad = getStaminaAdjustedValue(champ, maximumLoad);
	int16 wounds = champ->getWounds();
	if (wounds)
		maximumLoad -= maximumLoad >> (champ->getWoundsFlag(k0x0010_ChampionWoundLegs) ? 2 : 3);

	if (_vm->_objectMan->f33_getIconIndex(champ->getSlot(k5_ChampionSlotFeet)) == k119_IconIndiceArmourElvenBoots)
		maximumLoad += maximumLoad * 16;

	maximumLoad += 9;
	maximumLoad -= maximumLoad % 10;
	return maximumLoad;
}

void ChampionMan::drawChampionState(ChampionIndex champIndex) {
	static Box boxMouth = Box(55, 72, 12, 29); // @ G0048_s_Graphic562_Box_Mouth 
	static Box boxEye = Box(11, 28, 12, 29); // @ G0049_s_Graphic562_Box_Eye 

	int16 championStatusBoxX = champIndex * k69_ChampionStatusBoxSpacing;
	Champion *curChampion = &_champions[champIndex];
	uint16 championAttributes = curChampion->_attributes;
	if (!getFlag(championAttributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand))
		return;

	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == _vm->_inventoryMan->_g432_inventoryChampionOrdinal);
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_eventMan->showMouse();
	if (getFlag(championAttributes, k0x1000_ChampionAttributeStatusBox)) {
		Box box;
		box._y1 = 0;
		box._y2 = 28;
		box._x1 = championStatusBoxX;
		box._x2 = box._x1 + 66;
		if (curChampion->_currHealth) {
			_vm->_displayMan->fillScreenBox(box, k12_ColorDarkestGray);
			int16 nativeBitmapIndices[3];
			for (uint16 i = 0; i < 3; ++i)
				nativeBitmapIndices[i] = 0;

			uint16 borderCount = 0;
			if (_party._fireShieldDefense > 0)
				nativeBitmapIndices[borderCount++] = k38_BorderPartyFireshieldIndice;

			if (_party._spellShieldDefense > 0)
				nativeBitmapIndices[borderCount++] = k39_BorderPartySpellshieldIndice;

			if ((_party._shieldDefense > 0) || curChampion->_shieldDefense)
				nativeBitmapIndices[borderCount++] = k37_BorderPartyShieldIndice;

			while (borderCount--)
				_vm->_displayMan->blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(nativeBitmapIndices[borderCount]), &box, k40_byteWidth, k10_ColorFlesh, 29);

			if (isInventoryChampion) {
				_vm->_inventoryMan->f354_drawStatusBoxPortrait(champIndex);
				setFlag(championAttributes, k0x0100_ChampionAttributeStatistics);
			} else
				setFlag(championAttributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x2000_ChampionAttributeWounds | k0x8000_ChampionAttributeActionHand);
		} else {
			_vm->_displayMan->blitToScreen(_vm->_displayMan->getNativeBitmapOrGraphic(k8_StatusBoxDeadChampion), &box, k40_byteWidth, kM1_ColorNoTransparency, 29);
			_vm->_textMan->f53_printToLogicalScreen(championStatusBoxX + 1, 5, k13_ColorLightestGray, k1_ColorDarkGary, curChampion->_name);
			_vm->_menuMan->f386_drawActionIcon(champIndex);

			clearFlag(curChampion->_attributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand);
			_vm->_eventMan->hideMouse();
			return;
		}
	}
	if (!(curChampion->_currHealth)) {
		clearFlag(curChampion->_attributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand);
		_vm->_eventMan->hideMouse();
		return;
	}

	if (getFlag(championAttributes, k0x0080_ChampionAttributeNameTitle)) {
		Color nameColor = (champIndex == _leaderIndex) ? k9_ColorGold : k13_ColorLightestGray;
		if (isInventoryChampion) {
			char *championName = curChampion->_name;
			_vm->_textMan->f52_printToViewport(3, 7, nameColor, championName);
			int16 championTitleX = 6 * strlen(championName) + 3;
			char titleFirstCharacter = curChampion->_title[0];
			if ((titleFirstCharacter != ',') && (titleFirstCharacter != ';') && (titleFirstCharacter != '-'))
				championTitleX += 6;

			_vm->_textMan->f52_printToViewport(championTitleX, 7, nameColor, curChampion->_title);
			setFlag(championAttributes, k0x4000_ChampionAttributeViewport);
		} else {
			Box box;
			box._y1 = 0;
			box._y2 = 6;
			box._x1 = championStatusBoxX;
			box._x2 = box._x1 + 42;
			_vm->_displayMan->fillScreenBox(box, k1_ColorDarkGary);
			_vm->_textMan->f53_printToLogicalScreen(championStatusBoxX + 1, 5, nameColor, k1_ColorDarkGary, curChampion->_name);
		}
	}
	if (getFlag(championAttributes, k0x0100_ChampionAttributeStatistics)) {
		drawChampionBarGraphs(champIndex);
		if (isInventoryChampion) {
			drawHealthStaminaManaValues(curChampion);
			int16 nativeBitmapIndex;
			if ((curChampion->_food < 0) || (curChampion->_water < 0) || (curChampion->_poisonEventCount))
				nativeBitmapIndex = k34_SlotBoxWoundedIndice;
			else
				nativeBitmapIndex = k33_SlotBoxNormalIndice;

			_vm->_displayMan->blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(nativeBitmapIndex), boxMouth, k16_byteWidth, k12_ColorDarkestGray, 18);
			nativeBitmapIndex = k33_SlotBoxNormalIndice;
			for (int i = k1_ChampionStatStrength; i <= k6_ChampionStatAntifire; i++) {
				if ((curChampion->_statistics[i][k1_ChampionStatCurrent] < curChampion->_statistics[i][k0_ChampionStatMaximum])) {
					nativeBitmapIndex = k34_SlotBoxWoundedIndice;
					break;
				}
			}
			_vm->_displayMan->blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(nativeBitmapIndex), boxEye, k16_byteWidth, k12_ColorDarkestGray, 18);
			setFlag(championAttributes, k0x4000_ChampionAttributeViewport);
		}
	}
	if (getFlag(championAttributes, k0x2000_ChampionAttributeWounds)) {
		for (int i = isInventoryChampion ? k5_ChampionSlotFeet : k1_ChampionSlotActionHand; i >= k0_ChampionSlotReadyHand; i--)
			drawSlot(champIndex, i);

		if (isInventoryChampion)
			setFlag(championAttributes, k0x4000_ChampionAttributeViewport);
	}
	if (getFlag(championAttributes, k0x0200_ChampionAttributeLoad) && isInventoryChampion) {
		uint16 maxLoad = getMaximumLoad(curChampion);
		Color loadColor;
		if (curChampion->_load > maxLoad)
			loadColor = k8_ColorRed;
		else if (((long)curChampion->_load << 3) > ((long)maxLoad * 5))
			loadColor = k11_ColorYellow;
		else
			loadColor = k13_ColorLightestGray;

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY: _vm->_textMan->f52_printToViewport(104, 132, loadColor, "LOAD "); break;
		case Common::DE_DEU: _vm->_textMan->f52_printToViewport(104, 132, loadColor, "LAST "); break;
		case Common::FR_FRA: _vm->_textMan->f52_printToViewport(104, 132, loadColor, "CHARGE "); break;
		}

		maxLoad = curChampion->_load / 10;
		strcpy(_vm->_stringBuildBuffer, getStringFromInteger(maxLoad, true, 3).c_str());

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY: strcat(_vm->_stringBuildBuffer, "."); break;
		case Common::DE_DEU: strcat(_vm->_stringBuildBuffer, ","); break;
		case Common::FR_FRA: strcat(_vm->_stringBuildBuffer, "KG,"); break;
		}

		maxLoad = curChampion->_load - (maxLoad * 10);
		strcat(_vm->_stringBuildBuffer, getStringFromInteger(maxLoad, false, 1).c_str());
		strcat(_vm->_stringBuildBuffer, "/");
		maxLoad = (getMaximumLoad(curChampion) + 5) / 10;
		strcat(_vm->_stringBuildBuffer, getStringFromInteger(maxLoad, true, 3).c_str());
		strcat(_vm->_stringBuildBuffer, " KG");
		_vm->_textMan->f52_printToViewport(148, 132, loadColor, _vm->_stringBuildBuffer);
		setFlag(championAttributes, k0x4000_ChampionAttributeViewport);
	}
	uint16 championIconIndex = getChampionIconIndex(curChampion->_cell, _vm->_dungeonMan->_partyDir);
	if (getFlag(championAttributes, k0x0400_ChampionAttributeIcon) && (_vm->_eventMan->_useChampionIconOrdinalAsMousePointerBitmap != _vm->indexToOrdinal(championIconIndex))) {
		_vm->_displayMan->fillScreenBox(_boxChampionIcons[championIconIndex], _championColor[champIndex]);
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->getNativeBitmapOrGraphic(k28_ChampionIcons), _vm->_displayMan->_bitmapScreen, _boxChampionIcons[championIconIndex], getChampionIconIndex(curChampion->_dir, _vm->_dungeonMan->_partyDir) * 19, 0, k40_byteWidth, k160_byteWidthScreen, k12_ColorDarkestGray, 14, k200_heightScreen);
	}
	if (getFlag(championAttributes, k0x0800_ChampionAttributePanel) && isInventoryChampion) {
		if (_vm->_pressingMouth)
			_vm->_inventoryMan->f345_drawPanelFoodWaterPoisoned();
		else if (_vm->_pressingEye) {
			if (_leaderEmptyHanded)
				_vm->_inventoryMan->f351_drawChampionSkillsAndStatistics();
		} else
			_vm->_inventoryMan->f347_drawPanel();

		setFlag(championAttributes, k0x4000_ChampionAttributeViewport);
	}
	if (getFlag(championAttributes, k0x8000_ChampionAttributeActionHand)) {
		drawSlot(champIndex, k1_ChampionSlotActionHand);
		_vm->_menuMan->f386_drawActionIcon(champIndex);
		if (isInventoryChampion)
			setFlag(championAttributes, k0x4000_ChampionAttributeViewport);
	}
	if (getFlag(championAttributes, k0x4000_ChampionAttributeViewport))
		_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);

	clearFlag(curChampion->_attributes, k0x0080_ChampionAttributeNameTitle | k0x0100_ChampionAttributeStatistics | k0x0200_ChampionAttributeLoad | k0x0400_ChampionAttributeIcon | k0x0800_ChampionAttributePanel | k0x1000_ChampionAttributeStatusBox | k0x2000_ChampionAttributeWounds | k0x4000_ChampionAttributeViewport | k0x8000_ChampionAttributeActionHand);
	_vm->_eventMan->hideMouse();
}

uint16 ChampionMan::getChampionIconIndex(int16 val, Direction dir) {
	return ((val + 4 - dir) & 0x3);
}

void ChampionMan::drawHealthStaminaManaValues(Champion* champ) {
	drawHealthOrStaminaOrManaValue(116, champ->_currHealth, champ->_maxHealth);
	drawHealthOrStaminaOrManaValue(124, champ->_currStamina, champ->_maxStamina);
	drawHealthOrStaminaOrManaValue(132, champ->_currMana, champ->_maxMana);
}

void ChampionMan::drawSlot(uint16 champIndex, int16 slotIndex) {
	int16 nativeBitmapIndex = -1;
	Champion *champ = &_champions[champIndex];
	bool isInventoryChamp = (_vm->_inventoryMan->_g432_inventoryChampionOrdinal == _vm->indexToOrdinal(champIndex));

	uint16 slotBoxIndex;
	if (!isInventoryChamp) {
		// If drawing a slot for a champion other than the champion whose inventory is open
		if ((slotIndex > k1_ChampionSlotActionHand) || (_candidateChampionOrdinal == _vm->indexToOrdinal(champIndex)))
			return;
		slotBoxIndex = (champIndex << 1) + slotIndex;
	} else
		slotBoxIndex = k8_SlotBoxInventoryFirstSlot + slotIndex;

	Thing thing;
	if (slotIndex >= k30_ChampionSlotChest_1)
		thing = _vm->_inventoryMan->_g425_chestSlots[slotIndex - k30_ChampionSlotChest_1];
	else
		thing = champ->getSlot((ChampionSlot)slotIndex);

	SlotBox *slotBox = &_vm->_objectMan->_g30_slotBoxes[slotBoxIndex];
	Box box;
	box._x1 = slotBox->_x - 1;
	box._y1 = slotBox->_y - 1;
	box._x2 = box._x1 + 17;
	box._y2 = box._y1 + 17;

	if (!isInventoryChamp)
		_vm->_eventMan->hideMouse();

	int16 iconIndex;
	if (thing == Thing::_none) {
		if (slotIndex <= k5_ChampionSlotFeet) {
			iconIndex = k212_IconIndiceReadyHand + (slotIndex << 1);
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex))) {
				iconIndex++;
				nativeBitmapIndex = k34_SlotBoxWoundedIndice;
			} else
				nativeBitmapIndex = k33_SlotBoxNormalIndice;
		} else {
			if ((slotIndex >= k10_ChampionSlotNeck) && (slotIndex <= k13_ChampionSlotBackpackLine_1_1))
				iconIndex = k208_IconIndiceNeck + (slotIndex - k10_ChampionSlotNeck);
			else
				iconIndex = k204_IconIndiceEmptyBox;
		}
	} else {
		iconIndex = _vm->_objectMan->f33_getIconIndex(thing); // BUG0_35
		if (isInventoryChamp && (slotIndex == k1_ChampionSlotActionHand) && ((iconIndex == k144_IconIndiceContainerChestClosed) || (iconIndex == k30_IconIndiceScrollOpen))) {
			iconIndex++;
		} // BUG2_00
		if (slotIndex <= k5_ChampionSlotFeet) {
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex)))
				nativeBitmapIndex = k34_SlotBoxWoundedIndice;
			else
				nativeBitmapIndex = k33_SlotBoxNormalIndice;
		}
	}

	if ((slotIndex == k1_ChampionSlotActionHand) && (_vm->indexToOrdinal(champIndex) == _actingChampionOrdinal))
		nativeBitmapIndex = k35_SlotBoxActingHandIndice;

	if (nativeBitmapIndex != -1) {
		_vm->_displayMan->_useByteBoxCoordinates = false;
		if (isInventoryChamp) {
			_vm->_displayMan->blitToBitmap(_vm->_displayMan->getNativeBitmapOrGraphic(nativeBitmapIndex),
												_vm->_displayMan->_bitmapViewport, box, 0, 0, 16, k112_byteWidthViewport,
												k12_ColorDarkestGray, _vm->_displayMan->getPixelHeight(nativeBitmapIndex), k136_heightViewport);
		} else {
			_vm->_displayMan->blitToBitmap(_vm->_displayMan->getNativeBitmapOrGraphic(nativeBitmapIndex),
												_vm->_displayMan->_bitmapScreen, box, 0, 0, 16, k160_byteWidthScreen,
												k12_ColorDarkestGray, _vm->_displayMan->getPixelHeight(nativeBitmapIndex), k136_heightViewport);
		}
	}

	_vm->_objectMan->f38_drawIconInSlotBox(slotBoxIndex, iconIndex);

	if (!isInventoryChamp)
		_vm->_eventMan->showMouse();
}

void ChampionMan::renameChampion(Champion* champ) {
#define k1_RENAME_CHAMPION_NAME 1
#define k2_RENAME_CHAMPION_TITLE 2
	static const char underscoreCharacterString[2] = "_";
	static char renameChampionInputCharacterString[2] = " ";
	static const char reincarnateSpecialCharacters[6] = {',', '.', ';', ':', ' '};

	Box displayBox;
	displayBox._y1 = 3;
	displayBox._y2 = 8;
	displayBox._x1 = 3;
	displayBox._x2 = displayBox._x1 + 167;

	_vm->_displayMan->fillBoxBitmap(_vm->_displayMan->_bitmapViewport, displayBox, k12_ColorDarkestGray, k112_byteWidthViewport, k136_heightViewport);
	_vm->_displayMan->blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k27_PanelRenameChampionIndice), _vm->_inventoryMan->g32_BoxPanel, k72_byteWidth, k4_ColorCyan, 73);
	_vm->_textMan->f52_printToViewport(177, 58, k13_ColorLightestGray, "_______");
	_vm->_textMan->f52_printToViewport(105, 76, k13_ColorLightestGray, "___________________");
	_vm->_eventMan->showMouse();
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
	_vm->_eventMan->setMousePointerToNormal(k0_pointerArrow);
	_vm->_eventMan->hideMouse();
	uint16 curCharacterIndex = 0;
	champ->_name[curCharacterIndex] = '\0';
	champ->_title[0] = '\0';
	int16 renamedChampionStringMode = k1_RENAME_CHAMPION_NAME;
	char *renamedChampionString = champ->_name;
	int16 textPosX = 177;
	int16 textPosY = 91;

	for (;;) { /*_Infinite loop_*/
		bool championTitleIsFull = ((renamedChampionStringMode == k2_RENAME_CHAMPION_TITLE) && (curCharacterIndex == 19));
		if (!championTitleIsFull) {
			_vm->_eventMan->showMouse();
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, k9_ColorGold, k12_ColorDarkestGray, underscoreCharacterString, k200_heightScreen);
			_vm->_eventMan->hideMouse();
		}

		int16 curCharacter = 256;
		while (curCharacter == 256) {
			Common::Event event;
			Common::EventType eventType = _vm->_eventMan->processInput(&event, &event);
			_vm->_displayMan->updateScreen();
			if (_vm->_engineShouldQuit)
				return;
			_vm->_displayMan->updateScreen();
				//_vm->f22_delay(1);

			if (eventType == Common::EVENT_LBUTTONDOWN) {
				// If left mouse button status has changed

				Common::Point mousePos = _vm->_eventMan->getMousePos();
				if ((renamedChampionStringMode == k2_RENAME_CHAMPION_TITLE || (curCharacterIndex > 0)) && (mousePos.x >= 197) && (mousePos.x <= 215) && (mousePos.y >= 147) && (mousePos.y <= 155)) { /* Coordinates of 'OK' button */
					int16 characterIndexBackup = curCharacterIndex;
					char L0821_ac_ChampionNameBackupString[8];
					renamedChampionString = champ->_name;
					strcpy(L0821_ac_ChampionNameBackupString, renamedChampionString);
					curCharacterIndex = strlen(renamedChampionString);
					// Replace space characters on the right of the champion name by '\0' characters
					while (renamedChampionString[--curCharacterIndex] == ' ')
						renamedChampionString[curCharacterIndex] = '\0';

					bool found = false;
					for (uint16 idx = k0_ChampionFirst; idx < _partyChampionCount - 1; idx++) {
						if (!strcmp(_champions[idx]._name, renamedChampionString)) {
							// If an existing champion already has the specified name for the new champion
							found = true;
							break;
						}
					}
					if (!found)
						return;

					if (renamedChampionStringMode == k2_RENAME_CHAMPION_TITLE)
						renamedChampionString = champ->_title;

					strcpy(renamedChampionString = champ->_name, L0821_ac_ChampionNameBackupString);
					curCharacterIndex = characterIndexBackup;
				} else {
					if ((mousePos.x >= 107) && (mousePos.x <= 175) && (mousePos.y >= 147) && (mousePos.y <= 155)) { /* Coordinates of 'BACKSPACE' button */
						curCharacter = '\b';
						break;
					}
#if 0
					if ((mousePos.x < 107) || (mousePos.x > 215) || (mousePos.y < 116) || (mousePos.y > 144)) {/* Coordinates of table of all other characters */
						//goto T0281023;
					}
					if (!((mousePos.x + 4) % 10) || (!((mousePos.y + 5) % 10) && ((mousePos.x < 207) || (mousePos.y != 135)))) {
						//goto T0281023;
					}
#endif
					curCharacter = 'A' + (11 * ((mousePos.y - 116) / 10)) + ((mousePos.x - 107) / 10);
					if ((curCharacter == 86) || (curCharacter == 97)) {
						// The 'Return' button occupies two cells in the table
						curCharacter = '\r'; /* Carriage return */
						break;
					}

					if (curCharacter >= 87)
						// Compensate for the first cell occupied by 'Return' button
						curCharacter--;

					if (curCharacter > 'Z')
						curCharacter = reincarnateSpecialCharacters[(curCharacter - 'Z') - 1];

					break;
				}
			} else if (eventType == Common::EVENT_KEYDOWN)
				curCharacter = event.kbd.ascii;
		}

		if ((curCharacter >= 'a') && (curCharacter <= 'z'))
			curCharacter -= 32; // Convert to uppercase

		if (((curCharacter >= 'A') && (curCharacter <= 'Z')) || (curCharacter == '.') || (curCharacter == ',') || (curCharacter == ';') || (curCharacter == ':') || (curCharacter == ' ')) {
			if ((curCharacter != ' ') || curCharacterIndex != 0) {
				if (!championTitleIsFull) {
					renameChampionInputCharacterString[0] = curCharacter;
					_vm->_eventMan->showMouse();
					_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, k13_ColorLightestGray, k12_ColorDarkestGray, renameChampionInputCharacterString, k200_heightScreen);
					_vm->_eventMan->hideMouse();
					renamedChampionString[curCharacterIndex++] = curCharacter;
					renamedChampionString[curCharacterIndex] = '\0';
					textPosX += 6;
					if ((renamedChampionStringMode == k1_RENAME_CHAMPION_NAME) && (curCharacterIndex == 7)) {
						renamedChampionStringMode = k2_RENAME_CHAMPION_TITLE;
						renamedChampionString = champ->_title;
						textPosX = 105;
						textPosY = 109;
						curCharacterIndex = 0;
					}
				}
			}
		} else if (curCharacter == '\r') { // Carriage return
			if ((renamedChampionStringMode == k1_RENAME_CHAMPION_NAME) && (curCharacterIndex > 0)) {
				_vm->_eventMan->showMouse();
				_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, k13_ColorLightestGray, k12_ColorDarkestGray, underscoreCharacterString, k200_heightScreen);
				_vm->_eventMan->hideMouse();
				renamedChampionStringMode = k2_RENAME_CHAMPION_TITLE;
				renamedChampionString = champ->_title;
				textPosX = 105;
				textPosY = 109;
				curCharacterIndex = 0;
			}
		} else if (curCharacter == '\b') { // Backspace
			if ((renamedChampionStringMode == k1_RENAME_CHAMPION_NAME) && (curCharacterIndex == 0))
				continue;

			if (!championTitleIsFull) {
				_vm->_eventMan->showMouse();
				_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, k13_ColorLightestGray, k12_ColorDarkestGray, underscoreCharacterString, k200_heightScreen);
				_vm->_eventMan->hideMouse();
			}
			if (curCharacterIndex == 0) {
				renamedChampionString = champ->_name;
				curCharacterIndex = strlen(renamedChampionString) - 1;
				renamedChampionStringMode = k1_RENAME_CHAMPION_NAME;
				textPosX = 177 + (curCharacterIndex * 6);
				textPosY = 91;
			} else {
				curCharacterIndex--;
				textPosX -= 6;
			}
			renamedChampionString[curCharacterIndex] = '\0';
		}
	}
}

uint16 ChampionMan::getSkillLevel(int16 champIndex, uint16 skillIndex) {
	if (_partyIsSleeping)
		return 1;

	bool ignoreTmpExp = getFlag(skillIndex, k0x8000_IgnoreTemporaryExperience);
	bool ignoreObjModifiers = getFlag(skillIndex, k0x4000_IgnoreObjectModifiers);
	clearFlag(skillIndex, k0x8000_IgnoreTemporaryExperience | k0x4000_IgnoreObjectModifiers);
	Champion *champ = &_champions[champIndex];
	Skill *skill = &champ->_skills[skillIndex];
	int32 exp = skill->_experience;
	if (!ignoreTmpExp)
		exp += skill->_temporaryExperience;

	if (skillIndex > k3_ChampionSkillWizard) {
		// Hidden skill
		skill = &champ->_skills[(skillIndex - k4_ChampionSkillSwing) >> 2];
		exp += skill->_experience; // Add experience in the base skill
		if (!ignoreTmpExp)
			exp += skill->_temporaryExperience;

		exp >>= 1; // Halve experience to get average of base skill + hidden skill experience
	}
	int16 skillLevel = 1;
	while (exp >= 500) {
		exp >>= 1;
		skillLevel++;
	}
	if (!ignoreObjModifiers) {
		int16 actionHandIconIndex = _vm->_objectMan->f33_getIconIndex(champ->_slots[k1_ChampionSlotActionHand]);
		if (actionHandIconIndex == k27_IconIndiceWeaponTheFirestaff)
			skillLevel++;
		else if (actionHandIconIndex == k28_IconIndiceWeaponTheFirestaffComplete)
			skillLevel += 2;

		int16 neckIconIndex = _vm->_objectMan->f33_getIconIndex(champ->_slots[k10_ChampionSlotNeck]);
		switch (skillIndex) {
		case k3_ChampionSkillWizard:
			if (neckIconIndex == k124_IconIndiceJunkPendantFeral)
				skillLevel += 1;
			break;
		case k13_ChampionSkillHeal:
			// The skill modifiers of these two objects are not cumulative
			if ((neckIconIndex == k120_IconIndiceJunkGemOfAges) || (actionHandIconIndex == k66_IconIndiceWeaponSceptreOfLyf))
				skillLevel += 1;
			break;
		case k14_ChampionSkillInfluence:
			if (neckIconIndex == k122_IconIndiceJunkMoonstone)
				skillLevel += 1;
			break;
		case k15_ChampionSkillDefend:
			if (neckIconIndex == k121_IconIndiceJunkEkkhardCross)
				skillLevel += 1;
			break;
		default:
			break;
		}
	}
	return skillLevel;
}

}
