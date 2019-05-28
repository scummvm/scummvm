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

#include "dm/champion.h"
#include "dm/dungeonman.h"
#include "dm/eventman.h"
#include "dm/menus.h"
#include "dm/inventory.h"
#include "dm/objectman.h"
#include "dm/text.h"
#include "dm/timeline.h"
#include "dm/projexpl.h"
#include "dm/group.h"
#include "dm/movesens.h"
#include "dm/sounds.h"


namespace DM {

void Champion::resetToZero() {
	for (int16 i = 0; i < 30; ++i)
		_slots[i] = _vm->_thingNone;
	for (int16 i = 0; i < 20; ++i)
		_skills[i].resetToZero();
	_attributes = _wounds = 0;
	memset(_statistics, 0, 7 * 3);
	memset(_name, '\0', 8);
	memset(_title, '\0', 20);
	_dir = kDMDirNorth;
	_cell = kDMViewCellFronLeft;
	_actionIndex = kDMActionN;
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
	static const char *baseSkillNameEN[4] = {"FIGHTER", "NINJA", "PRIEST", "WIZARD"};
	static const char *baseSkillNameDE[4] = {"KAEMPFER", "NINJA", "PRIESTER", "MAGIER"};
	static const char *baseSkillNameFR[4] = {"GUERRIER", "NINJA", "PRETRE", "SORCIER"};
	static Box boxChampionIcons[4] = {
		Box(281, 299,  0, 13),
		Box(301, 319,  0, 13),
		Box(301, 319, 15, 28),
		Box(281, 299, 15, 28)
	};

	static Color championColor[4] = {kDMColorLightGreen, kDMColorYellow, kDMColorRed, kDMColorBlue};
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

	const char **baseSkillName;
	switch (_vm->getGameLanguage()) { // localized
	case Common::EN_ANY:
		baseSkillName = baseSkillNameEN;
		break;
	case Common::DE_DEU:
		baseSkillName = baseSkillNameDE;
		break;
	case Common::FR_FRA:
		baseSkillName = baseSkillNameFR;
		break;
	default:
		error("Unexpected language used");
	}

	for (int i = 0; i < 4; ++i) {
		_baseSkillName[i] = baseSkillName[i];
		_championColor[i] = championColor[i];
		_boxChampionIcons[i] = boxChampionIcons[i];
	}

	for (int i = 0; i < 16; i++)
		_lightPowerToLightAmount[i] = lightPowerToLightAmount[i];

	for (int i = 0; i < 38; i++)
		_slotMasks[i] = slotMasks[i];
}

ChampionMan::ChampionMan(DMEngine *vm) : _vm(vm) {
	_champions = new Champion[4];
	for (uint16 i = 0; i < 4; ++i) {
		_champions[i].setVm(_vm);
		_championPendingDamage[i] = 0;
		_championPendingWounds[i] = 0;
		_champions[i].resetToZero();
	}
	_partyChampionCount = 0;
	_partyDead = false;
	_leaderHandObject = Thing(0);
	_leaderIndex = kDMChampionNone;
	_candidateChampionOrdinal = 0;
	_partyIsSleeping = false;
	_actingChampionOrdinal = 0;
	_leaderHandObjectIconIndex = (IconIndice)0;
	_leaderEmptyHanded = false;
	_party.resetToZero();
	_magicCasterChampionIndex = kDMChampionNone;
	_mousePointerHiddenToDrawChangedObjIconOnScreen = false;

	initConstants();
}

ChampionMan::~ChampionMan() {
	delete[] _champions;
}

bool ChampionMan::isLeaderHandObjectThrown(int16 side) {
	if (_leaderIndex == kDMChampionNone)
		return false;

	return isObjectThrown(_leaderIndex, kDMSlotLeaderHand, side);
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
		actionHandThing = curChampion->getSlot(kDMSlotActionHand);
		curChampion->setSlot(kDMSlotActionHand, curThing);
		slotIndex = kDMSlotActionHand;
		throwingLeaderHandObjectFl = true;
	}

	int16 kineticEnergy = getStrength(champIndex, slotIndex);
	if (throwingLeaderHandObjectFl) {
		// In this case, curChampion and actionHandThing are set.
		curChampion->setSlot((ChampionSlot)slotIndex, actionHandThing);
	} else {
		curThing = getObjectRemovedFromSlot(champIndex, slotIndex);
		if (curThing == _vm->_thingNone)
			return false;
	}

	DungeonMan &dungeon = *_vm->_dungeonMan;
	_vm->_sound->requestPlay(kDMSoundIndexAttackSkelettonAnimatedArmorDethKnight, dungeon._partyMapX, dungeon._partyMapY, kDMSoundModePlayIfPrioritized);
	decrementStamina(champIndex, getThrowingStaminaCost(curThing));
	disableAction(champIndex, 4);
	int16 experience = 8;
	int16 weaponKineticEnergy = 1;
	if (curThing.getType() == kDMThingTypeWeapon) {
		experience += 4;
		WeaponInfo *curWeapon = dungeon.getWeaponInfo(curThing);
		if (curWeapon->_class <= kDMWeaponClassPoisinDart) {
			weaponKineticEnergy = curWeapon->_kineticEnergy;
			experience += weaponKineticEnergy >> 2;
		}
	}
	addSkillExperience(champIndex, kDMSkillThrow, experience);
	kineticEnergy += weaponKineticEnergy;
	int16 skillLevel = getSkillLevel((ChampionIndex)champIndex, kDMSkillThrow);
	kineticEnergy += _vm->getRandomNumber(16) + (kineticEnergy >> 1) + skillLevel;
	int16 attack = CLIP<int16>(40, ((skillLevel << 3) + _vm->getRandomNumber(32)), 200);
	int16 stepEnergy = MAX(5, 11 - skillLevel);
	_vm->_projexpl->createProjectile(curThing, dungeon._partyMapX, dungeon._partyMapY,
										  _vm->normalizeModulo4(dungeon._partyDir + side),
										  dungeon._partyDir, kineticEnergy, attack, stepEnergy);
	_vm->_projectileDisableMovementTicks = 4;
	_vm->_lastProjectileDisabledMovementDirection = dungeon._partyDir;
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
	TextMan &txtMan = *_vm->_textMan;

	Common::String tmp = getStringFromInteger(currVal, true, 3);
	txtMan.printToViewport(55, posY, kDMColorLightestGray, tmp.c_str());
	txtMan.printToViewport(73, posY, kDMColorLightestGray, "/");
	tmp = getStringFromInteger(maxVal, true, 3);
	txtMan.printToViewport(79, posY, kDMColorLightestGray, tmp.c_str());
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
	int16 statIndex = kDMStatLuck;
	int16 modifier = 0;
	ThingType thingType = thing.getType();

	bool cursed = false;
	if (((thingType == kDMThingTypeWeapon) || (thingType == kDMThingTypeArmour))
		&& (slotIndex >= kDMSlotReadyHand) && (slotIndex <= kDMSlotQuiverLine1_1)) {
		if (thingType == kDMThingTypeWeapon) {
			Weapon *weapon = (Weapon *)_vm->_dungeonMan->getThingData(thing);
			cursed = weapon->getCursed();
		} else {
			// k6_ArmourThingType
			Armour *armour = (Armour *)_vm->_dungeonMan->getThingData(thing);
			cursed = armour->getCursed();
		}

		if (cursed) {
			statIndex = kDMStatLuck;
			modifier = -3;
		}
	}

	if (!cursed) {
		statIndex = (ChampionStatType)thingType; // variable sharing

		if ((iconIndex == kDMIconIndiceJunkRabbitsFoot) && (slotIndex < kDMSlotChest1)) {
			statIndex = kDMStatLuck;
			modifier = 10;
		} else if (slotIndex == kDMSlotActionHand) {
			if (iconIndex == kDMIconIndiceWeaponMaceOfOrder) {
				statIndex = kDMStatStrength;
				modifier = 5;
			} else {
				statIndex = kDMStatMana;
				if ((iconIndex >= kDMIconIndiceWeaponStaffOfClawsEmpty) && (iconIndex <= kDMIconIndiceWeaponStaffOfClawsFull)) {
					modifier = 4;
				} else {
					switch (iconIndex) {
					case kDMIconIndiceWeaponDeltaSideSplitter:
						modifier = 1;
						break;
					case kDMIconIndiceWeaponTheInquisitorDragonFang:
						modifier = 2;
						break;
					case kDMIconIndiceWeaponVorpalBlade:
						modifier = 4;
						break;
					case kDMIconIndiceWeaponStaff:
						modifier = 2;
						break;
					case kDMIconIndiceWeaponWand:
						modifier = 1;
						break;
					case kDMIconIndiceWeaponTeowand:
						modifier = 6;
						break;
					case kDMIconIndiceWeaponYewStaff:
						modifier = 4;
						break;
					case kDMIconIndiceWeaponStaffOfManarStaffOfIrra:
						modifier = 10;
						break;
					case kDMIconIndiceWeaponSnakeStaffCrossOfNeta:
						modifier = 8;
						break;
					case kDMIconIndiceWeaponTheConduitSerpentStaff:
						modifier = 16;
						break;
					case kDMIconIndiceWeaponDragonSpit:
						modifier = 7;
						break;
					case kDMIconIndiceWeaponSceptreOfLyf:
						modifier = 5;
						break;
					default:
						break;
					}
				}
			}
		} else if (slotIndex == kDMSlotLegs) {
			if (iconIndex == kDMIconIndiceArmourPowertowers) {
				statIndex = kDMStatStrength;
				modifier = 10;
			}
		} else if (slotIndex == kDMSlotHead) {
			switch (iconIndex) {
			case kDMIconIndiceArmourCrownOfNerra:
				statIndex = kDMStatWisdom;
				modifier = 10;
				break;
			case kDMIconIndiceArmourDexhelm:
				statIndex = kDMStatDexterity;
				modifier = 10;
				break;
			default:
				break;
			}
		} else if (slotIndex == kDMSlotTorso) {
			switch (iconIndex) {
			case kDMIconIndiceArmourFlamebain:
				statIndex = kDMStatAntifire;
				modifier = 12;
				break;
			case kDMIconIndiceArmourCloakOfNight:
				statIndex = kDMStatDexterity;
				modifier = 8;
				break;
			default:
				break;
			}
		} else if (slotIndex == kDMSlotNeck) {
			switch (iconIndex) {
			case kDMIconIndiceJunkJewelSymalUnequipped:
			case kDMIconIndiceJunkJewelSymalEquipped:
				statIndex = kDMStatAntimagic;
				modifier = 15;
				break;
			case kDMIconIndiceArmourCloakOfNight:
				statIndex = kDMStatDexterity;
				modifier = 8;
				break;
			case kDMIconIndiceJunkMoonstone:
				statIndex = kDMStatMana;
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
		if (statIndex == kDMStatMana) {
			champ->_maxMana += modifier;
		} else if (statIndex < kDMStatAntifire + 1) {
			for (uint16 statValIndex = kDMStatMaximum; statValIndex <= kDMStatMinimum; ++statValIndex) {
				champ->getStatistic((ChampionStatType)statIndex, (ChampionStatValue)statValIndex) += modifier;
			}
		}
	}
}

bool ChampionMan::hasObjectIconInSlotBoxChanged(int16 slotBoxIndex, Thing thing) {
	ObjectMan &objMan = *_vm->_objectMan;

	IconIndice currIconIndex = objMan.getIconIndexInSlotBox(slotBoxIndex);
	if (((currIconIndex < kDMIconIndiceWeaponDagger) && (currIconIndex >= kDMIconIndiceJunkCompassNorth))
		|| ((currIconIndex >= kDMIconIndicePotionMaPotionMonPotion) && (currIconIndex <= kDMIconIndicePotionWaterFlask))
		|| (currIconIndex == kDMIconIndicePotionEmptyFlask)) {
		IconIndice newIconIndex = objMan.getIconIndex(thing);
		if (newIconIndex != currIconIndex) {
			if ((slotBoxIndex < kDMSlotBoxInventoryFirstSlot) && !_mousePointerHiddenToDrawChangedObjIconOnScreen) {
				_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
				_vm->_eventMan->hideMouse();
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
	EventManager &eventMan = *_vm->_eventMan;

	uint16 invChampOrdinal = invMan._inventoryChampionOrdinal;
	if (_candidateChampionOrdinal && !invChampOrdinal)
		return;

	_mousePointerHiddenToDrawChangedObjIconOnScreen = false;
	IconIndice leaderHandObjIconIndex = _leaderHandObjectIconIndex;

	if (((leaderHandObjIconIndex < kDMIconIndiceWeaponDagger) && (leaderHandObjIconIndex >= kDMIconIndiceJunkCompassNorth))	// < instead of <= is correct
		|| ((leaderHandObjIconIndex >= kDMIconIndicePotionMaPotionMonPotion) && (leaderHandObjIconIndex <= kDMIconIndicePotionWaterFlask))
		|| (leaderHandObjIconIndex == kDMIconIndicePotionEmptyFlask)) {
		IconIndice iconIndex = objMan.getIconIndex(_leaderHandObject);
		if (iconIndex != leaderHandObjIconIndex) {
			_mousePointerHiddenToDrawChangedObjIconOnScreen = true;
			eventMan.hideMouse();
			objMan.extractIconFromBitmap(iconIndex, objMan._objectIconForMousePointer);
			eventMan.setPointerToObject(_vm->_objectMan->_objectIconForMousePointer);
			_leaderHandObjectIconIndex = iconIndex;
			objMan.drawLeaderObjectName(_leaderHandObject);
		}
	}

	for (uint16 slotBoxIndex = 0; slotBoxIndex < (_partyChampionCount * 2); ++slotBoxIndex) {
		int16 champIndex = slotBoxIndex >> 1;
		if (invChampOrdinal == _vm->indexToOrdinal(champIndex))
			continue;

		if (hasObjectIconInSlotBoxChanged(slotBoxIndex, _champions[champIndex].getSlot((ChampionSlot)getHandSlotIndex(slotBoxIndex)))
			&& (getHandSlotIndex(slotBoxIndex) == kDMSlotActionHand)) {

			menuMan.drawActionIcon((ChampionIndex)champIndex);
		}
	}

	if (invChampOrdinal) {
		Champion *champ = &_champions[_vm->ordinalToIndex(invChampOrdinal)];
		Thing *thing = &champ->getSlot(kDMSlotReadyHand);
		uint16 drawViewport = 0;

		for (uint16 slotIndex = kDMSlotReadyHand; slotIndex < kDMSlotChest1; slotIndex++, thing++) {
			uint16 objIconChanged = hasObjectIconInSlotBoxChanged(slotIndex + kDMSlotBoxInventoryFirstSlot, *thing) ? 1 : 0;
			drawViewport |= objIconChanged;
			if (objIconChanged && (slotIndex == kDMSlotActionHand)) {
				menuMan.drawActionIcon((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
			}
		}

		if (invMan._panelContent == kDMPanelContentChest) {
			thing = invMan._chestSlots;
			for (int16 slotIndex = 0; slotIndex < 8; ++slotIndex, thing++) {
				drawViewport |= (hasObjectIconInSlotBoxChanged(slotIndex + kDMSlotBoxChestFirstSlot, *thing) ? 1 : 0);
			}
		}

		if (drawViewport) {
			champ->setAttributeFlag(kDMAttributeViewport, true);
			drawChampionState((ChampionIndex)_vm->ordinalToIndex(invChampOrdinal));
		}
	}

	if (_mousePointerHiddenToDrawChangedObjIconOnScreen)
		eventMan.showMouse();
}

void ChampionMan::addObjectInSlot(ChampionIndex champIndex, Thing thing, ChampionSlot slotIndex) {
	if (thing == _vm->_thingNone)
		return;

	InventoryMan &invMan = *_vm->_inventoryMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	MenuMan &menuMan = *_vm->_menuMan;
	Champion *champ = &_champions[champIndex];

	if (slotIndex >= kDMSlotChest1)
		invMan._chestSlots[slotIndex - kDMSlotChest1] = thing;
	else
		champ->setSlot(slotIndex, thing);

	champ->_load += dunMan.getObjectWeight(thing);
	champ->setAttributeFlag(kDMAttributeLoad, true);
	IconIndice iconIndex = objMan.getIconIndex(thing);
	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == invMan._inventoryChampionOrdinal);
	applyModifiersToStatistics(champ, slotIndex, iconIndex, 1, thing);
	uint16 *rawObjPtr = dunMan.getThingData(thing);

	if (slotIndex < kDMSlotHead) {
		if (slotIndex == kDMSlotActionHand) {
			champ->setAttributeFlag(kDMAttributeActionHand, true);
			if (_actingChampionOrdinal == _vm->indexToOrdinal(champIndex))
				menuMan.clearActingChampion();

			if ((iconIndex >= kDMIconIndiceScrollOpen) && (iconIndex <= kDMIconIndiceScrollClosed)) {
				((Scroll *)rawObjPtr)->setClosed(false);
				drawChangedObjectIcons();
			}
		}

		if (iconIndex == kDMIconIndiceWeaponTorchUnlit) {
			((Weapon *)rawObjPtr)->setLit(true);
			invMan.setDungeonViewPalette();
			drawChangedObjectIcons();
		} else if (isInventoryChampion && (slotIndex == kDMSlotActionHand) &&
			((iconIndex == kDMIconIndiceContainerChestClosed) || ((iconIndex >= kDMIconIndiceScrollOpen) && (iconIndex <= kDMIconIndiceScrollClosed)))) {
			champ->setAttributeFlag(kDMAttributePanel, true);
		}
	} else if (slotIndex == kDMSlotNeck) {
		if ((iconIndex >= kDMIconIndiceJunkIllumuletUnequipped) && (iconIndex <= kDMIconIndiceJunkIllumuletEquipped)) {
			((Junk *)rawObjPtr)->setChargeCount(1);
			_party._magicalLightAmount += _lightPowerToLightAmount[2];
			invMan.setDungeonViewPalette();
			iconIndex = (IconIndice)(iconIndex + 1);
		} else if ((iconIndex >= kDMIconIndiceJunkJewelSymalUnequipped) && (iconIndex <= kDMIconIndiceJunkJewelSymalEquipped)) {
			((Junk *)rawObjPtr)->setChargeCount(1);
			iconIndex = (IconIndice)(iconIndex + 1);
		}
	}

	drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		champ->setAttributeFlag(kDMAttributeViewport, true);
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
			if ((*(--scent)).toUint16() == searchedScentRedEagle)
				return _vm->indexToOrdinal(scentIndex);
		} while (scentIndex--);
	}
	return 0;
}

Thing ChampionMan::getObjectRemovedFromLeaderHand() {
	EventManager &eventMan = *_vm->_eventMan;
	_leaderEmptyHanded = true;
	Thing leaderHandObject = _leaderHandObject;

	if (leaderHandObject != _vm->_thingNone) {
		_leaderHandObject = _vm->_thingNone;
		_leaderHandObjectIconIndex = kDMIconIndiceNone;
		eventMan.showMouse();
		_vm->_objectMan->clearLeaderObjectName();
		eventMan.setMousePointer();
		eventMan.hideMouse();
		if (_leaderIndex != kDMChampionNone) {
			_champions[_leaderIndex]._load -= _vm->_dungeonMan->getObjectWeight(leaderHandObject);
			setFlag(_champions[_leaderIndex]._attributes, kDMAttributeLoad);
			drawChampionState(_leaderIndex);
		}
	}
	return leaderHandObject;
}

uint16 ChampionMan::getStrength(int16 champIndex, int16 slotIndex) {
	Champion *curChampion = &_champions[champIndex];
	DungeonMan &dungeon = *_vm->_dungeonMan;
	int16 strength = _vm->getRandomNumber(16) + curChampion->_statistics[kDMStatStrength][kDMStatCurrent];
	Thing curThing = curChampion->_slots[slotIndex];
	uint16 objectWeight = dungeon.getObjectWeight(curThing);
	uint16 oneSixteenthMaximumLoad = getMaximumLoad(curChampion) >> 4;

	if (objectWeight <= oneSixteenthMaximumLoad)
		strength += objectWeight - 12;
	else {
		int16 loadThreshold = oneSixteenthMaximumLoad + ((oneSixteenthMaximumLoad - 12) >> 1);
		if (objectWeight <= loadThreshold)
			strength += (objectWeight - oneSixteenthMaximumLoad) >> 1;
		else
			strength -= (objectWeight - loadThreshold) << 1;
	}

	if (curThing.getType() == kDMThingTypeWeapon) {
		WeaponInfo *weaponInfo = dungeon.getWeaponInfo(curThing);
		strength += weaponInfo->_strength;
		uint16 skillLevel = 0;
		uint16 weaponClass = weaponInfo->_class;
		if ((weaponClass == kDMWeaponClassSwingWeapon) || (weaponClass == kDMWeaponClassDaggerAndAxes))
			skillLevel = getSkillLevel(champIndex, kDMSkillSwing);

		if ((weaponClass != kDMWeaponClassSwingWeapon) && (weaponClass < kDMWeaponClassFirstBow))
			skillLevel += getSkillLevel(champIndex, kDMSkillThrow);

		if ((weaponClass >= kDMWeaponClassFirstBow) && (weaponClass < kDMWeaponClassFirstMagicWeapon))
			skillLevel += getSkillLevel(champIndex, kDMSkillShoot);

		strength += skillLevel << 1;
	}
	strength = getStaminaAdjustedValue(curChampion, strength);
	if (getFlag(curChampion->_wounds, (slotIndex == kDMSlotReadyHand) ? kDMWoundReadHand : kDMWoundActionHand))
		strength >>= 1;

	return CLIP(0, strength >> 1, 100);
}

Thing ChampionMan::getObjectRemovedFromSlot(uint16 champIndex, uint16 slotIndex) {
	Champion *curChampion = &_champions[champIndex];
	DungeonMan &dungeon = *_vm->_dungeonMan;
	InventoryMan &inventory = *_vm->_inventoryMan;

	Thing curThing;

	if (slotIndex >= kDMSlotChest1) {
		curThing = inventory._chestSlots[slotIndex - kDMSlotChest1];
		inventory._chestSlots[slotIndex - kDMSlotChest1] = _vm->_thingNone;
	} else {
		curThing = curChampion->_slots[slotIndex];
		curChampion->_slots[slotIndex] = _vm->_thingNone;
	}

	if (curThing == _vm->_thingNone)
		return _vm->_thingNone;

	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == inventory._inventoryChampionOrdinal);
	int16 curIconIndex = _vm->_objectMan->getIconIndex(curThing);
	// Remove object modifiers
	applyModifiersToStatistics(curChampion, slotIndex, curIconIndex, -1, curThing);

	Weapon *curWeapon = (Weapon *)dungeon.getThingData(curThing);
	if (slotIndex == kDMSlotNeck) {
		if ((curIconIndex >= kDMIconIndiceJunkIllumuletUnequipped) && (curIconIndex <= kDMIconIndiceJunkIllumuletEquipped)) {
			((Junk *)curWeapon)->setChargeCount(0);
			_party._magicalLightAmount -= _lightPowerToLightAmount[2];
			inventory.setDungeonViewPalette();
		} else if ((curIconIndex >= kDMIconIndiceJunkJewelSymalUnequipped) && (curIconIndex <= kDMIconIndiceJunkJewelSymalEquipped)) {
			((Junk *)curWeapon)->setChargeCount(0);
		}
	}

	drawSlot(champIndex, slotIndex);
	if (isInventoryChampion)
		setFlag(curChampion->_attributes, kDMAttributeViewport);

	if (slotIndex < kDMSlotHead) {
		if (slotIndex == kDMSlotActionHand) {
			setFlag(curChampion->_attributes, kDMAttributeActionHand);
			if (_actingChampionOrdinal == _vm->indexToOrdinal(champIndex))
				_vm->_menuMan->clearActingChampion();

			if ((curIconIndex >= kDMIconIndiceScrollOpen) && (curIconIndex <= kDMIconIndiceScrollClosed)) {
				((Scroll *)curWeapon)->setClosed(true);
				drawChangedObjectIcons();
			}
		}

		if ((curIconIndex >= kDMIconIndiceWeaponTorchUnlit) && (curIconIndex <= kDMIconIndiceWeaponTorchLit)) {
			curWeapon->setLit(false);
			inventory.setDungeonViewPalette();
			drawChangedObjectIcons();
		}

		if (isInventoryChampion && (slotIndex == kDMSlotActionHand)) {
			switch (curIconIndex) {
			case kDMIconIndiceContainerChestClosed:
				inventory.closeChest();
				// fall through
			case kDMIconIndiceScrollOpen:
			case kDMIconIndiceScrollClosed:
				setFlag(curChampion->_attributes, kDMAttributePanel);
				break;
			default:
				break;
			}
		}
	}
	curChampion->_load -= dungeon.getObjectWeight(curThing);
	setFlag(curChampion->_attributes, kDMAttributeLoad);
	return curThing;
}

void ChampionMan::decrementStamina(int16 championIndex, int16 decrement) {
	if (championIndex == kDMChampionNone)
		return;

	Champion *curChampion = &_champions[championIndex];
	curChampion->_currStamina -= decrement;

	int16 stamina = curChampion->_currStamina;
	if (stamina <= 0) {
		curChampion->_currStamina = 0;
		addPendingDamageAndWounds_getDamage(championIndex, (-stamina) >> 1, kDMWoundNone, kDMAttackTypeNormal);
	} else if (stamina > curChampion->_maxStamina)
		curChampion->_currStamina = curChampion->_maxStamina;

	setFlag(curChampion->_attributes, kDMAttributeLoad | kDMAttributeStatistics);
}

int16 ChampionMan::addPendingDamageAndWounds_getDamage(int16 champIndex, int16 attack, int16 allowedWounds, uint16 attackType) {
	if (attack <= 0)
		return 0;

	Champion *curChampion = &_champions[champIndex];
	if (!curChampion->_currHealth)
		return 0;

	if (attackType != kDMAttackTypeNormal) {
		uint16 defense = 0;
		uint16 woundCount = 0;
		for (int16 woundIndex = kDMSlotReadyHand; woundIndex <= kDMSlotFeet; woundIndex++) {
			if (allowedWounds & (1 << woundIndex)) {
				woundCount++;
				defense += getWoundDefense(champIndex, woundIndex | ((attackType == kDMAttackTypeSharp) ? kDMMaskSharpDefense : kDMMaskNoSharpDefense));
			}
		}
		if (woundCount)
			defense /= woundCount;

		bool skipScaling = false;
		switch (attackType) {
		case kDMAttackTypePsychic:
		{
			int16 wisdomFactor = 115 - curChampion->_statistics[kDMStatWisdom][kDMStatCurrent];
			if (wisdomFactor <= 0)
				attack = 0;
			else
				attack = _vm->getScaledProduct(attack, 6, wisdomFactor);

			skipScaling = true;
		}
		break;
		case kDMAttackTypeMagic:
			attack = getStatisticAdjustedAttack(curChampion, kDMStatAntimagic, attack);
			attack -= _party._spellShieldDefense;
			skipScaling = true;
			break;
		case kDMAttackTypeFire:
			attack = getStatisticAdjustedAttack(curChampion, kDMStatAntifire, attack);
			attack -= _party._fireShieldDefense;
			break;
		case kDMAttackTypeSelf:
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

		int16 adjustedAttack = getStatisticAdjustedAttack(curChampion, kDMStatVitality, _vm->getRandomNumber(128) + 10);
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

	DungeonMan &dungeon = *_vm->_dungeonMan;
	Champion *curChampion = &_champions[champIndex];
	bool useSharpDefense = getFlag(woundIndex, kDMMaskSharpDefense);
	if (useSharpDefense)
		clearFlag(woundIndex, kDMMaskSharpDefense);

	uint16 armorShieldDefense = 0;
	for (int16 slotIndex = kDMSlotReadyHand; slotIndex <= kDMSlotActionHand; slotIndex++) {
		Thing curThing = curChampion->_slots[slotIndex];
		if (curThing.getType() == kDMThingTypeArmour) {
			ArmourInfo *armorInfo = (ArmourInfo *)dungeon.getThingData(curThing);
			armorInfo = &dungeon._armourInfos[((Armour *)armorInfo)->getType()];
			if (getFlag(armorInfo->_attributes, kDMArmourAttributeShield))
				armorShieldDefense += ((getStrength(champIndex, slotIndex) + dungeon.getArmourDefense(armorInfo, useSharpDefense)) * woundDefenseFactor[woundIndex]) >> ((slotIndex == woundIndex) ? 4 : 5);
		}
	}

	int16 woundDefense = _vm->getRandomNumber((curChampion->_statistics[kDMStatVitality][kDMStatCurrent] >> 3) + 1);
	if (useSharpDefense)
		woundDefense >>= 1;

	woundDefense += curChampion->_actionDefense + curChampion->_shieldDefense + _party._shieldDefense + armorShieldDefense;
	if (woundIndex > kDMSlotActionHand) {
		Thing curThing = curChampion->_slots[woundIndex];
		if (curThing.getType() == kDMThingTypeArmour) {
			ArmourInfo *armourInfo = (ArmourInfo *)dungeon.getThingData(curThing);
			woundDefense += dungeon.getArmourDefense(&dungeon._armourInfos[((Armour *)armourInfo)->getType()], useSharpDefense);
		}
	}

	if (getFlag(curChampion->_wounds, 1 << woundIndex))
		woundDefense -= 8 + _vm->getRandomNumber(4);

	if (_partyIsSleeping)
		woundDefense >>= 1;

	return CLIP(0, woundDefense >> 1, 100);
}

uint16 ChampionMan::getStatisticAdjustedAttack(Champion *champ, uint16 statIndex, uint16 attack) {
	int16 factor = 170 - champ->_statistics[statIndex][kDMStatCurrent];

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
	EventManager &evtMan = *_vm->_eventMan;

	_vm->_stopWaitingForPlayerInput = true;
	_partyIsSleeping = false;
	_vm->_waitForInputMaxVerticalBlankCount = 10;
	_vm->delay(10);
	_vm->_displayMan->drawFloorAndCeiling();
	evtMan._primaryMouseInput = evtMan._primaryMouseInputInterface;
	evtMan._secondaryMouseInput = evtMan._secondaryMouseInputMovement;
	evtMan._primaryKeyboardInput = evtMan._primaryKeyboardInputInterface;
	evtMan._secondaryKeyboardInput = evtMan._secondaryKeyboardInputMovement;
	evtMan.discardAllInput();
	_vm->_menuMan->drawEnabledMenus();
}

int16 ChampionMan::getThrowingStaminaCost(Thing thing) {
	int16 weight = _vm->_dungeonMan->getObjectWeight(thing) >> 1;
	int16 staminaCost = CLIP<int16>(1, weight, 10);

	while ((weight -= 10) > 0)
		staminaCost += weight >> 1;

	return staminaCost;
}

void ChampionMan::disableAction(uint16 champIndex, uint16 ticks) {
	Champion *curChampion = &_champions[champIndex];
	int32 updatedEnableActionEventTime = _vm->_gameTime + ticks;

	TimelineEvent curEvent;
	curEvent._type = kDMEventTypeEnableChampionAction;
	curEvent._priority = champIndex;
	curEvent._Bu._slotOrdinal = 0;

	int16 eventIndex = curChampion->_enableActionEventIndex;
	if (eventIndex >= 0) {
		int32 currentEnableActionEventTime = _vm->filterTime(_vm->_timeline->_events[eventIndex]._mapTime);
		if (updatedEnableActionEventTime >= currentEnableActionEventTime)
			updatedEnableActionEventTime += (currentEnableActionEventTime - _vm->_gameTime) >> 1;
		else
			updatedEnableActionEventTime = currentEnableActionEventTime + (ticks >> 1);
		_vm->_timeline->deleteEvent(eventIndex);
	} else {
		setFlag(curChampion->_attributes, kDMAttributeActionHand | kDMAttributeDisableAction);
		drawChampionState((ChampionIndex)champIndex);
	}
	curEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_partyMapIndex, updatedEnableActionEventTime);
	curChampion->_enableActionEventIndex = _vm->_timeline->addEventGetEventIndex(&curEvent);
}

void ChampionMan::addSkillExperience(uint16 champIndex, uint16 skillIndex, uint16 exp) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	TextMan &txtMan = *_vm->_textMan;

	if ((skillIndex >= kDMSkillSwing) && (skillIndex <= kDMSkillShoot) && (_vm->_projexpl->_lastCreatureAttackTime < _vm->_gameTime - 150))
		exp >>= 1;

	if (exp) {
		if (dungeon._currMap->_difficulty)
			exp *= dungeon._currMap->_difficulty;

		Champion *curChampion = &_champions[champIndex];
		uint16 baseSkillIndex;
		if (skillIndex >= kDMSkillSwing)
			baseSkillIndex = (skillIndex - kDMSkillSwing) >> 2;
		else
			baseSkillIndex = skillIndex;

		uint16 skillLevelBefore = getSkillLevel(champIndex, baseSkillIndex | (kDMIgnoreObjectModifiers | kDMIgnoreTemporaryExperience));

		if ((skillIndex >= kDMSkillSwing) && (_vm->_projexpl->_lastCreatureAttackTime > _vm->_gameTime - 25))
			exp <<= 1;

		Skill *curSkill = &curChampion->_skills[skillIndex];
		curSkill->_experience += exp;
		if (curSkill->_temporaryExperience < 32000)
			curSkill->_temporaryExperience += CLIP(1, exp >> 3, 100);

		curSkill = &curChampion->_skills[baseSkillIndex];
		if (skillIndex >= kDMSkillSwing)
			curSkill->_experience += exp;

		uint16 skillLevelAfter = getSkillLevel(champIndex, baseSkillIndex | (kDMIgnoreObjectModifiers | kDMIgnoreTemporaryExperience));
		if (skillLevelAfter > skillLevelBefore) {
			int16 newBaseSkillLevel = skillLevelAfter;
			int16 minorStatIncrease = _vm->getRandomNumber(2);
			int16 majorStatIncrease = 1 + _vm->getRandomNumber(2);
			uint16 vitalityAmount = _vm->getRandomNumber(2); /* For Priest skill, the amount is 0 or 1 for all skill levels */
			if (baseSkillIndex != kDMSkillPriest) {
				vitalityAmount &= skillLevelAfter; /* For non Priest skills the amount is 0 for even skill levels. The amount is 0 or 1 for odd skill levels */
			}
			curChampion->_statistics[kDMStatVitality][kDMStatMaximum] += vitalityAmount;
			uint16 staminaAmount = curChampion->_maxStamina;
			curChampion->_statistics[kDMStatAntifire][kDMStatMaximum] += _vm->getRandomNumber(2) & ~skillLevelAfter; /* The amount is 0 for odd skill levels. The amount is 0 or 1 for even skill levels */
			bool increaseManaFl = false;
			switch (baseSkillIndex) {
			case kDMSkillFighter:
				staminaAmount >>= 4;
				skillLevelAfter *= 3;
				curChampion->_statistics[kDMStatStrength][kDMStatMaximum] += majorStatIncrease;
				curChampion->_statistics[kDMStatDexterity][kDMStatMaximum] += minorStatIncrease;
				break;
			case kDMSkillNinja:
				staminaAmount /= 21;
				skillLevelAfter <<= 1;
				curChampion->_statistics[kDMStatStrength][kDMStatMaximum] += minorStatIncrease;
				curChampion->_statistics[kDMStatDexterity][kDMStatMaximum] += majorStatIncrease;
				break;
			case kDMSkillWizard:
				staminaAmount >>= 5;
				curChampion->_maxMana += skillLevelAfter + (skillLevelAfter >> 1);
				curChampion->_statistics[kDMStatWisdom][kDMStatMaximum] += majorStatIncrease;
				increaseManaFl = true;
				break;
			case kDMSkillPriest:
				staminaAmount /= 25;
				curChampion->_maxMana += skillLevelAfter;
				skillLevelAfter += (skillLevelAfter + 1) >> 1;
				curChampion->_statistics[kDMStatWisdom][kDMStatMaximum] += minorStatIncrease;
				increaseManaFl = true;
				break;
			default:
				break;
			}
			if (increaseManaFl) {
				if ((curChampion->_maxMana += MIN(_vm->getRandomNumber(4), (uint16)(newBaseSkillLevel - 1))) > 900)
					curChampion->_maxMana = 900;
				curChampion->_statistics[kDMStatAntimagic][kDMStatMaximum] += _vm->getRandomNumber(3);
			}

			if ((curChampion->_maxHealth += skillLevelAfter + _vm->getRandomNumber((skillLevelAfter >> 1) + 1)) > 999)
				curChampion->_maxHealth = 999;

			if ((curChampion->_maxStamina += staminaAmount + _vm->getRandomNumber((staminaAmount >> 1) + 1)) > 9999)
				curChampion->_maxStamina = 9999;

			setFlag(curChampion->_attributes, kDMAttributeStatistics);
			drawChampionState((ChampionIndex)champIndex);
			txtMan.printLineFeed();
			Color curChampionColor = _championColor[champIndex];
			txtMan.printMessage(curChampionColor, curChampion->_name);

			switch (_vm->getGameLanguage()) { // localized
			default:
			case Common::EN_ANY:
				txtMan.printMessage(curChampionColor, " JUST GAINED A ");
				break;
			case Common::DE_DEU:
				txtMan.printMessage(curChampionColor, " HAT SOEBEN STUFE");
				break;
			case Common::FR_FRA:
				txtMan.printMessage(curChampionColor, " VIENT DE DEVENIR ");
				break;
			}

			txtMan.printMessage(curChampionColor, _baseSkillName[baseSkillIndex]);

			switch (_vm->getGameLanguage()) { // localized
			default:
			case Common::EN_ANY:
				txtMan.printMessage(curChampionColor, "!");
				break;
			case Common::DE_DEU:
				txtMan.printMessage(curChampionColor, " LEVEL!");
				break;
			case Common::FR_FRA:
				txtMan.printMessage(curChampionColor, " ERREICHT!");
				break;
			}
		}
	}
}

int16 ChampionMan::getDamagedChampionCount(uint16 attack, int16 wounds, int16 attackType) {
	int16 randomMax = (attack >> 3) + 1;
	uint16 reducedAttack = attack - randomMax;
	randomMax <<= 1;

	int16 damagedChampionCount = 0;
	for (int16 championIndex = kDMChampionFirst; championIndex < _partyChampionCount; championIndex++) {
		// Actual attack is attack +/- (attack / 8)
		if (addPendingDamageAndWounds_getDamage(championIndex, MAX(1, reducedAttack + _vm->getRandomNumber(randomMax)), wounds, attackType))
			damagedChampionCount++;
	}

	return damagedChampionCount;
}

int16 ChampionMan::getTargetChampionIndex(int16 mapX, int16 mapY, uint16 cell) {
	DungeonMan &dungeon = *_vm->_dungeonMan;

	if (_partyChampionCount && (_vm->getDistance(mapX, mapY, dungeon._partyMapX, dungeon._partyMapY) <= 1)) {
		signed char orderedCellsToAttack[4];
		_vm->_groupMan->setOrderedCellsToAttack(orderedCellsToAttack, dungeon._partyMapX, dungeon._partyMapY, mapX, mapY, cell);
		for (uint16 i = 0; i < 4; i++) {
			int16 championIndex = getIndexInCell(orderedCellsToAttack[i]);
			if (championIndex >= 0)
				return championIndex;
		}
	}
	return kDMChampionNone;
}

int16 ChampionMan::getDexterity(Champion *champ) {
	int16 dexterity = _vm->getRandomNumber(8) + champ->_statistics[kDMStatDexterity][kDMStatCurrent];
	dexterity -= ((int32)(dexterity >> 1) * (int32)champ->_load) / getMaximumLoad(champ);
	if (_partyIsSleeping)
		dexterity >>= 1;

	return CLIP(1 + _vm->getRandomNumber(8), dexterity >> 1, 100 - _vm->getRandomNumber(8));
}

bool ChampionMan::isLucky(Champion *champ, uint16 percentage) {
	if (_vm->getRandomNumber(2) && (_vm->getRandomNumber(100) > percentage))
		return true;

	unsigned char *curStat = champ->_statistics[kDMStatLuck];
	bool retVal = (_vm->getRandomNumber(curStat[kDMStatCurrent]) > percentage);
	curStat[kDMStatCurrent] = CLIP<unsigned char>(curStat[kDMStatMinimum], curStat[kDMStatCurrent] + (retVal ? -2 : 2), curStat[kDMStatMaximum]);
	return retVal;
}

void ChampionMan::championPoison(int16 champIndex, uint16 attack) {
	if ((champIndex == kDMChampionNone) || (_vm->indexToOrdinal(champIndex) == _candidateChampionOrdinal))
		return;

	InventoryMan &inventory = *_vm->_inventoryMan;
	Champion *curChampion = &_champions[champIndex];
	addPendingDamageAndWounds_getDamage(champIndex, MAX(1, attack >> 6), kDMWoundNone, kDMAttackTypeNormal);
	setFlag(curChampion->_attributes, kDMAttributeStatistics);
	if ((_vm->indexToOrdinal(champIndex) == inventory._inventoryChampionOrdinal) && (inventory._panelContent == kDMPanelContentFoodWaterPoisoned))
		setFlag(curChampion->_attributes, kDMAttributePanel);

	if (--attack) {
		curChampion->_poisonEventCount++;
		TimelineEvent newEvent;
		newEvent._type = kDMEventTypePoisonChampion;
		newEvent._priority = champIndex;
		newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 36);
		newEvent._Bu._attack = attack;
		_vm->_timeline->addEventGetEventIndex(&newEvent);
	}

	drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::setPartyDirection(int16 dir) {
	DungeonMan &dungeon = *_vm->_dungeonMan;

	if (dir == dungeon._partyDir)
		return;

	int16 dirDiff = dir - dungeon._partyDir;
	if (dirDiff < 0)
		dirDiff += 4;

	Champion *curChampion = _champions;
	for (int16 i = kDMChampionFirst; i < _partyChampionCount; i++) {
		curChampion->_cell = (ViewCell)_vm->normalizeModulo4(curChampion->_cell + dirDiff);
		curChampion->_dir = (Direction)_vm->normalizeModulo4(curChampion->_dir + dirDiff);
		curChampion++;
	}

	dungeon._partyDir = (Direction)dir;
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
		bool mergeFl = getFlag(cycleCount, kDMMaskMergeCycles);
		if (mergeFl)
			clearFlag(cycleCount, kDMMaskMergeCycles);

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
	if (thing == _vm->_thingNone)
		return;

	EventManager &evtMan = *_vm->_eventMan;

	_leaderEmptyHanded = false;
	_vm->_objectMan->extractIconFromBitmap(_leaderHandObjectIconIndex = _vm->_objectMan->getIconIndex(_leaderHandObject = thing), _vm->_objectMan->_objectIconForMousePointer);
	evtMan.showMouse();
	_vm->_objectMan->drawLeaderObjectName(thing);

	if (setMousePointer)
		_vm->_setMousePointerToObjectInMainLoop = true;
	else
		evtMan.setPointerToObject(_vm->_objectMan->_objectIconForMousePointer);

	evtMan.hideMouse();
	if (_leaderIndex != kDMChampionNone) {
		_champions[_leaderIndex]._load += _vm->_dungeonMan->getObjectWeight(thing);
		setFlag(_champions[_leaderIndex]._attributes, kDMAttributeLoad);
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

	if (getFlag(champ->_wounds, kDMWoundFeet))
		ticks += woundTicks;

	if (_vm->_objectMan->getIconIndex(champ->_slots[kDMSlotFeet]) == kDMIconIndiceArmourBootOfSpeed)
		ticks--;

	return ticks;
}

bool ChampionMan::isAmmunitionCompatibleWithWeapon(uint16 champIndex, uint16 weaponSlotIndex, uint16 ammunitionSlotIndex) {
	Champion *curChampion = &_champions[champIndex];
	DungeonMan &dungeon = *_vm->_dungeonMan;
	Thing curThing = curChampion->_slots[weaponSlotIndex];

	if (curThing.getType() != kDMThingTypeWeapon)
		return false;

	WeaponInfo *weaponInfo = dungeon.getWeaponInfo(curThing);
	int16 weaponClass = kDMWeaponClassNone;

	if ((weaponInfo->_class >= kDMWeaponClassFirstBow) && (weaponInfo->_class <= kDMWeaponClassLastBow))
		weaponClass = kDMWeaponClassBowAmmunition;
	else if ((weaponInfo->_class >= kDMWeaponClassFirstSling) && (weaponInfo->_class <= kDMWeaponClassLastSling))
		weaponClass = kDMWeaponClassSlingAmmunition;

	if (weaponClass == kDMWeaponClassNone)
		return false;

	curThing = curChampion->_slots[ammunitionSlotIndex];
	weaponInfo = dungeon.getWeaponInfo(curThing);
	return ((curThing.getType() == kDMThingTypeWeapon) && (weaponInfo->_class == weaponClass));
}

void ChampionMan::drawAllChampionStates() {
	for (int16 i = kDMChampionFirst; i < _partyChampionCount; i++)
		drawChampionState((ChampionIndex)i);
}

void ChampionMan::viAltarRebirth(uint16 champIndex) {
	Champion *curChampion = &_champions[champIndex];
	if (getIndexInCell(curChampion->_cell) != kDMChampionNone) {
		uint16 numCell = kDMCellNorthWest;
		while (getIndexInCell(numCell) != kDMChampionNone)
			numCell++;

		curChampion->_cell = (ViewCell)numCell;
	}

	uint16 maximumHealth = curChampion->_maxHealth;
	curChampion->_maxHealth = MAX(25, maximumHealth - (maximumHealth >> 6) - 1);
	curChampion->_currHealth = curChampion->_maxHealth >> 1;
	_vm->_menuMan->drawSpellAreaControls(_magicCasterChampionIndex);
	curChampion->_dir = _vm->_dungeonMan->_partyDir;
	setFlag(curChampion->_attributes, kDMAttributeActionHand | kDMAttributeStatusBox | kDMAttributeIcon);
	drawChampionState((ChampionIndex)champIndex);
}

void ChampionMan::clickOnSlotBox(uint16 slotBoxIndex) {
	uint16 champIndex;
	uint16 slotIndex;

	InventoryMan &inventory = *_vm->_inventoryMan;
	if (slotBoxIndex < kDMSlotBoxInventoryFirstSlot) {
		if (_candidateChampionOrdinal)
			return;

		champIndex = slotBoxIndex >> 1;
		if ((champIndex >= _partyChampionCount) || (_vm->indexToOrdinal(champIndex) == inventory._inventoryChampionOrdinal) || !_champions[champIndex]._currHealth)
			return;

		slotIndex = getHandSlotIndex(slotBoxIndex);
	} else {
		champIndex = _vm->ordinalToIndex(inventory._inventoryChampionOrdinal);
		slotIndex = slotBoxIndex - kDMSlotBoxInventoryFirstSlot;
	}

	DungeonMan &dungeon = *_vm->_dungeonMan;
	Thing leaderHandObject = _leaderHandObject;
	Thing slotThing;
	if (slotIndex >= kDMSlotChest1)
		slotThing = inventory._chestSlots[slotIndex - kDMSlotChest1];
	else
		slotThing = _champions[champIndex]._slots[slotIndex];

	if ((slotThing == _vm->_thingNone) && (leaderHandObject == _vm->_thingNone))
		return;

	if ((leaderHandObject != _vm->_thingNone) && (!(dungeon._objectInfos[dungeon.getObjectInfoIndex(leaderHandObject)]._allowedSlots & _slotMasks[slotIndex])))
		return;

	EventManager &evtMan = *_vm->_eventMan;
	evtMan.showMouse();
	if (leaderHandObject != _vm->_thingNone)
		getObjectRemovedFromLeaderHand();

	if (slotThing != _vm->_thingNone) {
		getObjectRemovedFromSlot(champIndex, slotIndex);
		putObjectInLeaderHand(slotThing, false);
	}

	if (leaderHandObject != _vm->_thingNone)
		addObjectInSlot((ChampionIndex)champIndex, leaderHandObject, (ChampionSlot)slotIndex);

	drawChampionState((ChampionIndex)champIndex);
	evtMan.hideMouse();
}

bool ChampionMan::isProjectileSpellCast(uint16 champIndex, Thing thing, int16 kineticEnergy, uint16 requiredManaAmount) {
	Champion *curChampion = &_champions[champIndex];
	if (curChampion->_currMana < requiredManaAmount)
		return false;

	curChampion->_currMana -= requiredManaAmount;
	setFlag(curChampion->_attributes, kDMAttributeStatistics);
	int16 stepEnergy = 10 - MIN(8, curChampion->_maxMana >> 3);
	if (kineticEnergy < (stepEnergy << 2)) {
		kineticEnergy += 3;
		stepEnergy--;
	}

	championShootProjectile(curChampion, thing, kineticEnergy, 90, stepEnergy);
	return true; // fix BUG_01
}

void ChampionMan::championShootProjectile(Champion *champ, Thing thing, int16 kineticEnergy, int16 attack, int16 stepEnergy) {
	Direction newDirection = champ->_dir;
	DungeonMan &dungeon = *_vm->_dungeonMan;
	_vm->_projexpl->createProjectile(thing, dungeon._partyMapX, dungeon._partyMapY, _vm->normalizeModulo4((((champ->_cell - newDirection + 1) & 0x0002) >> 1) + newDirection), newDirection, kineticEnergy, attack, stepEnergy);
	_vm->_projectileDisableMovementTicks = 4;
	_vm->_lastProjectileDisabledMovementDirection = newDirection;
}

void ChampionMan::applyAndDrawPendingDamageAndWounds() {
	Champion *championPtr = _champions;
	EventManager &evtMan = *_vm->_eventMan;
	TextMan &txtMan = *_vm->_textMan;
	DisplayMan &display = *_vm->_displayMan;

	for (uint16 championIndex = kDMChampionFirst; championIndex < _partyChampionCount; championIndex++, championPtr++) {
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

		if (curHealth <= 0)
			championKill(championIndex);
		else {
			championPtr->_currHealth = curHealth;
			setFlag(championPtr->_attributes, kDMAttributeStatistics);
			if (pendingWounds)
				setFlag(championPtr->_attributes, kDMAttributeWounds);

			int16 textPosX = championIndex * kDMChampionStatusBoxSpacing;
			int16 textPosY;

			Box blitBox;
			blitBox._rect.top = 0;
			evtMan.showMouse();

			if (_vm->indexToOrdinal(championIndex) == _vm->_inventoryMan->_inventoryChampionOrdinal) {
				blitBox._rect.bottom = 28;
				blitBox._rect.left = textPosX + 7;
				blitBox._rect.right = blitBox._rect.left + 31; /* Box is over the champion portrait in the status box */
				display.blitToScreen(display.getNativeBitmapOrGraphic(kDMGraphicIdxDamageToChampionBig), &blitBox, k16_byteWidth, kDMColorFlesh, 29);
				// Check the number of digits and sets the position accordingly.
				if (pendingDamage < 10) // 1 digit
					textPosX += 21;
				else if (pendingDamage < 100)  // 2 digits
					textPosX += 18;
				else // 3 digits
					textPosX += 15;

				textPosY = 16;
			} else {
				blitBox._rect.bottom = 6;
				blitBox._rect.left = textPosX;
				blitBox._rect.right = blitBox._rect.left + 47; /* Box is over the champion name in the status box */
				display.blitToScreen(display.getNativeBitmapOrGraphic(kDMGraphicIdxDamageToChampionSmall), &blitBox, k24_byteWidth, kDMColorFlesh, 7);
				// Check the number of digits and sets the position accordingly.
				if (pendingDamage < 10) // 1 digit
					textPosX += 19;
				else if (pendingDamage < 100) // 2 digits
					textPosX += 16;
				else //3 digits
					textPosX += 13;

				textPosY = 5;
			}
			txtMan.printToLogicalScreen(textPosX, textPosY, kDMColorWhite, kDMColorRed, getStringFromInteger(pendingDamage, false, 3).c_str());

			int16 eventIndex = championPtr->_hideDamageReceivedIndex;
			if (eventIndex == -1) {
				TimelineEvent newEvent;
				newEvent._type = kDMEventTypeHideDamageReceived;
				newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 5);
				newEvent._priority = championIndex;
				championPtr->_hideDamageReceivedIndex = _vm->_timeline->addEventGetEventIndex(&newEvent);
			} else {
				TimelineEvent *curEvent = &_vm->_timeline->_events[eventIndex];
				curEvent->_mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + 5);
				_vm->_timeline->fixChronology(_vm->_timeline->getIndex(eventIndex));
			}
			drawChampionState((ChampionIndex)championIndex);
			evtMan.hideMouse();
		}
	}
}

void ChampionMan::championKill(uint16 champIndex) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	Champion *curChampion = &_champions[champIndex];
	EventManager &evtMan = *_vm->_eventMan;
	DisplayMan &display = *_vm->_displayMan;
	InventoryMan &inventory = *_vm->_inventoryMan;
	curChampion->_currHealth = 0;
	setFlag(curChampion->_attributes, kDMAttributeStatusBox);
	if (_vm->indexToOrdinal(champIndex) == inventory._inventoryChampionOrdinal) {
		if (_vm->_pressingEye) {
			_vm->_pressingEye = false;
			evtMan._ignoreMouseMovements = false;
			if (!_leaderEmptyHanded) {
				_vm->_objectMan->drawLeaderObjectName(_leaderHandObject);
			}
			evtMan._hideMousePointerRequestCount = 1;
			evtMan.hideMouse();
		} else if (_vm->_pressingMouth) {
			_vm->_pressingMouth = false;
			evtMan._ignoreMouseMovements = false;
			evtMan._hideMousePointerRequestCount = 1;
			evtMan.hideMouse();
		}
		inventory.toggleInventory(kDMChampionCloseInventory);
	}
	dropAllObjects(champIndex);
	Thing unusedThing = dungeon.getUnusedThing(kDMMaskChampionBones | kDMThingTypeJunk);
	uint16 curCell = 0;
	if (unusedThing != _vm->_thingNone) {
		Junk *L0966_ps_Junk = (Junk *)dungeon.getThingData(unusedThing);
		L0966_ps_Junk->setType(kDMJunkTypeBones);
		L0966_ps_Junk->setDoNotDiscard(true);
		L0966_ps_Junk->setChargeCount(champIndex);
		curCell = curChampion->_cell;
		_vm->_moveSens->getMoveResult(_vm->thingWithNewCell(unusedThing, curCell), kDMMapXNotOnASquare, 0, dungeon._partyMapX, dungeon._partyMapY);
	}
	curChampion->_symbolStep = 0;
	curChampion->_symbols[0] = '\0';
	curChampion->_dir = dungeon._partyDir;
	curChampion->_maximumDamageReceived = 0;
	uint16 curChampionIconIndex = getChampionIconIndex(curCell, dungeon._partyDir);
	if (_vm->indexToOrdinal(curChampionIconIndex) == evtMan._useChampionIconOrdinalAsMousePointerBitmap) {
		evtMan._mousePointerBitmapUpdated = true;
		evtMan._useChampionIconOrdinalAsMousePointerBitmap = _vm->indexToOrdinal(kDMChampionNone);
	}

	if (curChampion->_poisonEventCount)
		unpoison(champIndex);

	display._useByteBoxCoordinates = false;
	display.fillScreenBox(_boxChampionIcons[curChampionIconIndex], kDMColorBlack);
	drawChampionState((ChampionIndex)champIndex);

	ChampionIndex aliveChampionIndex;
	int idx = 0;
	for (curChampion = _champions; (idx < _partyChampionCount) && (curChampion->_currHealth == 0); idx++, curChampion++)
		;

	aliveChampionIndex = (ChampionIndex)idx;
	if (aliveChampionIndex == _partyChampionCount) { /* BUG0_43 The game does not end if the last living champion in the party is killed while looking at a candidate champion in a portrait. The condition to end the game when the whole party is killed is not true because the code considers the candidate champion as alive (in the loop above) */
		_partyDead = true;
		return;
	}

	if (champIndex == _leaderIndex)
		evtMan.commandSetLeader(aliveChampionIndex);

	if (champIndex == _magicCasterChampionIndex)
		_vm->_menuMan->setMagicCasterAndDrawSpellArea(aliveChampionIndex);
	else
		_vm->_menuMan->drawSpellAreaControls(_magicCasterChampionIndex);
}

void ChampionMan::dropAllObjects(uint16 champIndex) {
	static const int16 slotDropOrder[30] = {
		kDMSlotFeet,
		kDMSlotLegs,
		kDMSlotQuiverLine2_2,
		kDMSlotQuiverLine1_2,
		kDMSlotQuiverLine2_1,
		kDMSlotQuiverLine1_1,
		kDMSlotPouch_2,
		kDMSlotPouch1,
		kDMSlotTorso,
		kDMSlotBackpackLine1_1,
		kDMSlotBackpackLine2_2,
		kDMSlotBackpackLine2_3,
		kDMSlotBackpackLine2_4,
		kDMSlotBackpackLine2_5,
		kDMSlotBackpackLine2_6,
		kDMSlotBackpackLine2_7,
		kDMSlotBackpackLine2_8,
		kDMSlotBackpackLine2_9,
		kDMSlotBackpackLine1_2,
		kDMSlotBackpackLine1_3,
		kDMSlotBackpackLine1_4,
		kDMSlotBackpackLine1_5,
		kDMSlotBackpackLine1_6,
		kDMSlotBackpackLine1_7,
		kDMSlotBackpackLine1_8,
		kDMSlotBackpackLine1_9,
		kDMSlotNeck,
		kDMSlotHead,
		kDMSlotReadyHand,
		kDMSlotActionHand
	};

	uint16 curCell = _champions[champIndex]._cell;
	DungeonMan &dungeon = *_vm->_dungeonMan;
	for (uint16 slotIndex = kDMSlotReadyHand; slotIndex < kDMSlotChest1; slotIndex++) {
		Thing curThing = getObjectRemovedFromSlot(champIndex, slotDropOrder[slotIndex]);
		if (curThing != _vm->_thingNone)
			_vm->_moveSens->getMoveResult(_vm->thingWithNewCell(curThing, curCell), kDMMapXNotOnASquare, 0, dungeon._partyMapX, dungeon._partyMapY);
	}
}

void ChampionMan::unpoison(int16 champIndex) {
	if (champIndex == kDMChampionNone)
		return;

	TimelineEvent *eventPtr = _vm->_timeline->_events;
	for (uint16 eventIndex = 0; eventIndex < _vm->_timeline->_eventMaxCount; eventPtr++, eventIndex++) {
		if ((eventPtr->_type == kDMEventTypePoisonChampion) && (eventPtr->_priority == champIndex))
			_vm->_timeline->deleteEvent(eventIndex);
	}
	_champions[champIndex]._poisonEventCount = 0;
}

void ChampionMan::applyTimeEffects() {
	if (!_partyChampionCount)
		return;

	DungeonMan &dungeon = *_vm->_dungeonMan;
	InventoryMan &inventory = *_vm->_inventoryMan;

	Scent checkScent;
	checkScent.setMapX(dungeon._partyMapX);
	checkScent.setMapY(dungeon._partyMapY);
	checkScent.setMapIndex(dungeon._partyMapIndex);

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
	for (uint16 championIndex = kDMChampionFirst; championIndex < _partyChampionCount; championIndex++, championPtr++) {
		if (championPtr->_currHealth && (_vm->indexToOrdinal(championIndex) != _candidateChampionOrdinal)) {
			uint16 wizardSkillLevel = getSkillLevel(championIndex, kDMSkillWizard) + getSkillLevel(championIndex, kDMSkillPriest);
			if ((championPtr->_currMana < championPtr->_maxMana)
				&& (timeCriteria < championPtr->_statistics[kDMStatWisdom][kDMStatCurrent] + wizardSkillLevel)) {
				int16 manaGain = championPtr->_maxMana / 40;
				if (_partyIsSleeping)
					manaGain <<= 1;

				manaGain++;
				decrementStamina(championIndex, manaGain * MAX(7, 16 - wizardSkillLevel));
				championPtr->_currMana += MIN<int16>(manaGain, championPtr->_maxMana - championPtr->_currMana);
			} else if (championPtr->_currMana > championPtr->_maxMana)
				championPtr->_currMana--;

			for (int16 idx = kDMSkillWater; idx >= kDMSkillFighter; idx--) {
				if (championPtr->_skills[idx]._temporaryExperience > 0)
					championPtr->_skills[idx]._temporaryExperience--;
			}
			uint16 staminaGainCycleCount = 4;
			int16 staminaMagnitude = championPtr->_maxStamina;
			while (championPtr->_currStamina < (staminaMagnitude >>= 1))
				staminaGainCycleCount += 2;

			int16 staminaLoss = 0;
			int16 staminaAmount = CLIP(1, (championPtr->_maxStamina >> 8) - 1, 6);
			if (_partyIsSleeping)
				staminaAmount <<= 1;

			int32 compDelay = _vm->_gameTime - _vm->_projexpl->_lastPartyMovementTime;
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

			if ((championPtr->_currHealth < championPtr->_maxHealth) && (championPtr->_currStamina >= (championPtr->_maxStamina >> 2)) && (timeCriteria < (championPtr->_statistics[kDMStatVitality][kDMStatCurrent] + 12))) {
				int16 healthGain = (championPtr->_maxHealth >> 7) + 1;
				if (_partyIsSleeping)
					healthGain <<= 1;

				if (_vm->_objectMan->getIconIndex(championPtr->_slots[kDMSlotNeck]) == kDMIconIndiceJunkEkkhardCross)
					healthGain += (healthGain >> 1) + 1;

				championPtr->_currHealth += MIN(healthGain, (int16)(championPtr->_maxHealth - championPtr->_currHealth));
			}
			if (!((int)_vm->_gameTime & (_partyIsSleeping ? 63 : 255))) {
				for (uint16 i = kDMStatLuck; i <= kDMStatAntifire; i++) {
					byte *curStatistic = championPtr->_statistics[i];
					uint16 statisticMaximum = curStatistic[kDMStatMaximum];
					if (curStatistic[kDMStatCurrent] < statisticMaximum)
						curStatistic[kDMStatCurrent]++;
					else if (curStatistic[kDMStatCurrent] > statisticMaximum)
						curStatistic[kDMStatCurrent] -= curStatistic[kDMStatCurrent] / statisticMaximum;
				}
			}
			if (!_partyIsSleeping && (championPtr->_dir != dungeon._partyDir) && (_vm->_projexpl->_lastCreatureAttackTime + 60 < _vm->_gameTime)) {
				championPtr->_dir = dungeon._partyDir;
				championPtr->_maximumDamageReceived = 0;
				setFlag(championPtr->_attributes, kDMAttributeIcon);
			}
			setFlag(championPtr->_attributes, kDMAttributeStatistics);
			if (_vm->indexToOrdinal(championIndex) == inventory._inventoryChampionOrdinal) {
				if (_vm->_pressingMouth || _vm->_pressingEye || (inventory._panelContent == kDMPanelContentFoodWaterPoisoned)) {
					setFlag(championPtr->_attributes, kDMAttributePanel);
				}
			}
		}
	}
	drawAllChampionStates();
}

void ChampionMan::savePartyPart2(Common::OutSaveFile *file) {
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

void ChampionMan::loadPartyPart2(Common::InSaveFile *file) {
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

	return kDMChampionNone;
}

void ChampionMan::resetDataToStartGame() {
	if (_vm->_gameMode == kDMModeLoadSavedGame) {
		Thing handThing = _leaderHandObject;
		EventManager &evtMan = *_vm->_eventMan;

		if (handThing == _vm->_thingNone) {
			_leaderEmptyHanded = true;
			_leaderHandObjectIconIndex = kDMIconIndiceNone;
			evtMan.setMousePointer();
		} else
			putObjectInLeaderHand(handThing, true); /* This call will add the weight of the leader hand object to the Load of the leader a first time */

		Champion *curChampion = _champions;
		for (int16 idx = 0; idx < _partyChampionCount; idx++, curChampion++) {
			clearFlag(curChampion->_attributes, kDMAttributeNameTitle | kDMAttributeStatistics | kDMAttributeLoad | kDMAttributeIcon | kDMAttributePanel | kDMAttributeStatusBox | kDMAttributeWounds | kDMAttributeViewport | kDMAttributeActionHand);
			setFlag(curChampion->_attributes, kDMAttributeActionHand | kDMAttributeStatusBox | kDMAttributeIcon);
		}
		drawAllChampionStates();

		ChampionIndex championIndex = _leaderIndex;
		if (championIndex != kDMChampionNone) {
			_leaderIndex = kDMChampionNone;
			evtMan.commandSetLeader(championIndex);
		}

		championIndex = _magicCasterChampionIndex;
		if (championIndex != kDMChampionNone) {
			_magicCasterChampionIndex = kDMChampionNone;
			_vm->_menuMan->setMagicCasterAndDrawSpellArea(championIndex);
		}
		return;
	}

	_leaderHandObject = _vm->_thingNone;
	_leaderHandObjectIconIndex = kDMIconIndiceNone;
	_leaderEmptyHanded = true;
}

void ChampionMan::addCandidateChampionToParty(uint16 championPortraitIndex) {
	if (!_leaderEmptyHanded)
		return;

	if (_partyChampionCount == 4)
		return;

	DungeonMan &dungeon = *_vm->_dungeonMan;
	DisplayMan &display = *_vm->_displayMan;

	uint16 previousPartyChampionCount = _partyChampionCount;
	Champion *championPtr = &_champions[previousPartyChampionCount];
	championPtr->resetToZero();
	// Strangerke - TODO: Check if the new code is possible to run on the older version (example: the portraits could be missing in the data)
	display._useByteBoxCoordinates = true;
	display.blitToBitmap(display.getNativeBitmapOrGraphic(kDMGraphicIdxChampionPortraits), championPtr->_portrait, _boxChampionPortrait, getChampionPortraitX(championPortraitIndex), getChampionPortraitY(championPortraitIndex), k128_byteWidth, k16_byteWidth, kDMColorNoTransparency, 87, 29);
	championPtr->_actionIndex = kDMActionNone;
	championPtr->_enableActionEventIndex = -1;
	championPtr->_hideDamageReceivedIndex = -1;
	championPtr->_dir = dungeon._partyDir;
	uint16 viewCell = kDMViewCellFronLeft;
	while (getIndexInCell(_vm->normalizeModulo4(viewCell + dungeon._partyDir)) != kDMChampionNone)
		viewCell++;

	championPtr->_cell = (ViewCell)_vm->normalizeModulo4(viewCell + dungeon._partyDir);
	championPtr->_attributes = kDMAttributeIcon;
	championPtr->_directionMaximumDamageReceived = dungeon._partyDir;
	championPtr->_food = 1500 + _vm->getRandomNumber(256);
	championPtr->_water = 1500 + _vm->getRandomNumber(256);
	for (int16 slotIdx = kDMSlotReadyHand; slotIdx < kDMSlotChest1; slotIdx++)
		championPtr->_slots[slotIdx] = _vm->_thingNone;

	Thing curThing = dungeon.getSquareFirstThing(dungeon._partyMapX, dungeon._partyMapY);
	while (curThing.getType() != kDMstringTypeText)
		curThing = dungeon.getNextThing(curThing);

	char L0807_ac_DecodedChampionText[77];
	char *decodedStringPtr = L0807_ac_DecodedChampionText;
	dungeon.decodeText(decodedStringPtr, curThing, (TextType)(kDMTextTypeScroll | kDMMaskDecodeEvenIfInvisible));

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
		setFlag(championPtr->_attributes, kDMAttributeMale);

	decodedStringPtr++;
	championPtr->_currHealth = championPtr->_maxHealth = getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	championPtr->_currStamina = championPtr->_maxStamina = getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	championPtr->_currMana = championPtr->_maxMana = getDecodedValue(decodedStringPtr, 4);
	decodedStringPtr += 4;
	decodedStringPtr++;
	for (int16 statIdx = kDMStatLuck; statIdx <= kDMStatAntifire; statIdx++) {
		championPtr->_statistics[statIdx][kDMStatMinimum] = 30;
		championPtr->_statistics[statIdx][kDMStatCurrent] = championPtr->_statistics[statIdx][kDMStatMaximum] = getDecodedValue(decodedStringPtr, 2);
		decodedStringPtr += 2;
	}
	championPtr->_statistics[kDMStatLuck][kDMStatMinimum] = 10;
	decodedStringPtr++;
	for (uint16 skillIdx = kDMSkillSwing; skillIdx <= kDMSkillWater; skillIdx++) {
		int skillValue = *decodedStringPtr++ - 'A';
		if (skillValue > 0)
			championPtr->_skills[skillIdx]._experience = 125L << skillValue;
	}
	for (uint16 skillIdx = kDMSkillFighter; skillIdx <= kDMSkillWizard; skillIdx++) {
		int32 baseSkillExperience = 0;
		int16 hiddenSkillIndex = (skillIdx + 1) << 2;
		for (uint16 hiddenIdx = 0; hiddenIdx < 4; hiddenIdx++)
			baseSkillExperience += championPtr->_skills[hiddenSkillIndex + hiddenIdx]._experience;

		championPtr->_skills[skillIdx]._experience = baseSkillExperience;
	}
	_candidateChampionOrdinal = previousPartyChampionCount + 1;
	if (++_partyChampionCount == 1) {
		_vm->_eventMan->commandSetLeader(kDMChampionFirst);
		_vm->_menuMan->_refreshActionArea = true;
	} else {
		_vm->_menuMan->clearActingChampion();
		_vm->_menuMan->drawActionIcon((ChampionIndex)(_partyChampionCount - 1));
	}

	int16 curMapX = dungeon._partyMapX;
	int16 curMapY = dungeon._partyMapY;
	uint16 championObjectsCell = _vm->returnOppositeDir(dungeon._partyDir);
	curMapX += _vm->_dirIntoStepCountEast[dungeon._partyDir];
	curMapY += _vm->_dirIntoStepCountNorth[dungeon._partyDir];
	curThing = dungeon.getSquareFirstThing(curMapX, curMapY);
	int16 slotIdx = kDMSlotBackpackLine1_1;
	while (curThing != _vm->_thingEndOfList) {
		ThingType thingType = curThing.getType();
		if ((thingType > kDMThingTypeSensor) && (curThing.getCell() == championObjectsCell)) {
			int16 objectAllowedSlots = dungeon._objectInfos[dungeon.getObjectInfoIndex(curThing)]._allowedSlots;
			uint16 curSlotIndex = kDMSlotReadyHand;
			switch (thingType) {
			case kDMThingTypeArmour: {
				bool skipCheck = false;
				for (curSlotIndex = kDMSlotHead; curSlotIndex <= kDMSlotFeet; curSlotIndex++) {
					if (objectAllowedSlots & _slotMasks[curSlotIndex]) {
						skipCheck = true;
						break;
					}
				}

				if (skipCheck)
					break;

				if ((objectAllowedSlots & _slotMasks[kDMSlotNeck]) && (championPtr->_slots[kDMSlotNeck] == _vm->_thingNone))
					curSlotIndex = kDMSlotNeck;
				else
					curSlotIndex = slotIdx++;

				break;
			}
			case kDMThingTypeWeapon:
				if (championPtr->_slots[kDMSlotActionHand] == _vm->_thingNone)
					curSlotIndex = kDMSlotActionHand;
				else if ((objectAllowedSlots & _slotMasks[kDMSlotNeck]) && (championPtr->_slots[kDMSlotNeck] == _vm->_thingNone))
					curSlotIndex = kDMSlotNeck;
				else
					curSlotIndex = slotIdx++;
				break;
			case kDMThingTypeScroll:
			case kDMThingTypePotion:
				if (championPtr->_slots[kDMSlotPouch1] == _vm->_thingNone)
					curSlotIndex = kDMSlotPouch1;
				else if (championPtr->_slots[kDMSlotPouch_2] == _vm->_thingNone)
					curSlotIndex = kDMSlotPouch_2;
				else if ((objectAllowedSlots & _slotMasks[kDMSlotNeck]) && (championPtr->_slots[kDMSlotNeck] == _vm->_thingNone))
					curSlotIndex = kDMSlotNeck;
				else
					curSlotIndex = slotIdx++;
				break;
			case kDMThingTypeContainer:
			case kDMThingTypeJunk:
				if ((objectAllowedSlots & _slotMasks[kDMSlotNeck]) && (championPtr->_slots[kDMSlotNeck] == _vm->_thingNone))
					curSlotIndex = kDMSlotNeck;
				else
					curSlotIndex = slotIdx++;

				break;
			default:
				break;
			}

			while (championPtr->_slots[curSlotIndex] != _vm->_thingNone) {
				if ((objectAllowedSlots & _slotMasks[kDMSlotNeck]) && (championPtr->_slots[kDMSlotNeck] == _vm->_thingNone))
					curSlotIndex = kDMSlotNeck;
				else
					curSlotIndex = slotIdx++;
			}
			addObjectInSlot((ChampionIndex)previousPartyChampionCount, curThing, (ChampionSlot)curSlotIndex);
		}
		curThing = dungeon.getNextThing(curThing);
	}
	_vm->_inventoryMan->toggleInventory((ChampionIndex)previousPartyChampionCount);
	_vm->_menuMan->drawDisabledMenu();
}

void ChampionMan::drawChampionBarGraphs(ChampionIndex champIndex) {
	Champion *champ = &_champions[champIndex];
	EventManager &evtMan = *_vm->_eventMan;
	DisplayMan &display = *_vm->_displayMan;

	int16 barGraphHeights[3];
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
	evtMan.showMouse();

	// Strangerke - TO CHECK: if portraits, maybe the old (assembly) code is required for older versions
	Box box;
	box._rect.left = champIndex * kDMChampionStatusBoxSpacing + 46;
	box._rect.right = box._rect.left + 3;
	box._rect.top = 2;
	box._rect.bottom = 26;
	for (int16 barGraphIndex = 0; barGraphIndex < 3; barGraphIndex++) {
		int16 barGraphHeight = barGraphHeights[barGraphIndex];
		if (barGraphHeight < 25) {
			box._rect.top = 2;
			box._rect.bottom = 27 - barGraphHeight;
			display.fillScreenBox(box, kDMColorDarkestGray);
		}
		if (barGraphHeight) {
			box._rect.top = 27 - barGraphHeight;
			box._rect.bottom = 26;
			display.fillScreenBox(box, _championColor[champIndex]);
		}
		box._rect.left += 7;
		box._rect.right += 7;
	}
	evtMan.hideMouse();
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
	uint16 maximumLoad = champ->getStatistic(kDMStatStrength, kDMStatCurrent) * 8 + 100;
	maximumLoad = getStaminaAdjustedValue(champ, maximumLoad);
	int16 wounds = champ->getWounds();
	if (wounds)
		maximumLoad -= maximumLoad >> (champ->getWoundsFlag(kDMWoundLegs) ? 2 : 3);

	if (_vm->_objectMan->getIconIndex(champ->getSlot(kDMSlotFeet)) == kDMIconIndiceArmourElvenBoots)
		maximumLoad += maximumLoad * 16;

	maximumLoad += 9;
	maximumLoad -= maximumLoad % 10;
	return maximumLoad;
}

void ChampionMan::drawChampionState(ChampionIndex champIndex) {
	static Box boxMouth = Box(55, 72, 12, 29); // @ G0048_s_Graphic562_Box_Mouth
	static Box boxEye = Box(11, 28, 12, 29); // @ G0049_s_Graphic562_Box_Eye

	int16 championStatusBoxX = champIndex * kDMChampionStatusBoxSpacing;
	Champion *curChampion = &_champions[champIndex];
	EventManager &evtMan = *_vm->_eventMan;
	TextMan &txtMan = *_vm->_textMan;
	DisplayMan &display = *_vm->_displayMan;
	InventoryMan &inventory = *_vm->_inventoryMan;

	uint16 championAttributes = curChampion->_attributes;
	if (!getFlag(championAttributes, kDMAttributeNameTitle | kDMAttributeStatistics | kDMAttributeLoad | kDMAttributeIcon | kDMAttributePanel | kDMAttributeStatusBox | kDMAttributeWounds | kDMAttributeViewport | kDMAttributeActionHand))
		return;

	bool isInventoryChampion = (_vm->indexToOrdinal(champIndex) == inventory._inventoryChampionOrdinal);
	display._useByteBoxCoordinates = false;
	evtMan.showMouse();
	if (getFlag(championAttributes, kDMAttributeStatusBox)) {
		Box box;
		box._rect.top = 0;
		box._rect.bottom = 28;
		box._rect.left = championStatusBoxX;
		box._rect.right = box._rect.left + 66;
		if (curChampion->_currHealth) {
			display.fillScreenBox(box, kDMColorDarkestGray);
			int16 nativeBitmapIndices[3];
			for (uint16 i = 0; i < 3; ++i)
				nativeBitmapIndices[i] = 0;

			uint16 borderCount = 0;
			if (_party._fireShieldDefense > 0)
				nativeBitmapIndices[borderCount++] = kDMGraphicIdxBorderPartyFireshield;

			if (_party._spellShieldDefense > 0)
				nativeBitmapIndices[borderCount++] = kDMGraphicIdxBorderPartySpellshield;

			if ((_party._shieldDefense > 0) || curChampion->_shieldDefense)
				nativeBitmapIndices[borderCount++] = kDMGraphicIdxBorderPartyShield;

			while (borderCount--)
				display.blitToScreen(display.getNativeBitmapOrGraphic(nativeBitmapIndices[borderCount]), &box, k40_byteWidth, kDMColorFlesh, 29);

			if (isInventoryChampion) {
				inventory.drawStatusBoxPortrait(champIndex);
				setFlag(championAttributes, kDMAttributeStatistics);
			} else
				setFlag(championAttributes, kDMAttributeNameTitle | kDMAttributeStatistics | kDMAttributeWounds | kDMAttributeActionHand);
		} else {
			display.blitToScreen(display.getNativeBitmapOrGraphic(kDMGraphicIdxStatusBoxDeadChampion), &box, k40_byteWidth, kDMColorNoTransparency, 29);
			txtMan.printToLogicalScreen(championStatusBoxX + 1, 5, kDMColorLightestGray, kDMColorDarkGary, curChampion->_name);
			_vm->_menuMan->drawActionIcon(champIndex);

			clearFlag(curChampion->_attributes, kDMAttributeNameTitle | kDMAttributeStatistics | kDMAttributeLoad | kDMAttributeIcon | kDMAttributePanel | kDMAttributeStatusBox | kDMAttributeWounds | kDMAttributeViewport | kDMAttributeActionHand);
			evtMan.hideMouse();
			return;
		}
	}
	if (!(curChampion->_currHealth)) {
		clearFlag(curChampion->_attributes, kDMAttributeNameTitle | kDMAttributeStatistics | kDMAttributeLoad | kDMAttributeIcon | kDMAttributePanel | kDMAttributeStatusBox | kDMAttributeWounds | kDMAttributeViewport | kDMAttributeActionHand);
		evtMan.hideMouse();
		return;
	}

	if (getFlag(championAttributes, kDMAttributeNameTitle)) {
		Color nameColor = (champIndex == _leaderIndex) ? kDMColorGold : kDMColorLightestGray;
		if (isInventoryChampion) {
			char *championName = curChampion->_name;
			txtMan.printToViewport(3, 7, nameColor, championName);
			int16 championTitleX = 6 * strlen(championName) + 3;
			char titleFirstCharacter = curChampion->_title[0];
			if ((titleFirstCharacter != ',') && (titleFirstCharacter != ';') && (titleFirstCharacter != '-'))
				championTitleX += 6;

			txtMan.printToViewport(championTitleX, 7, nameColor, curChampion->_title);
			setFlag(championAttributes, kDMAttributeViewport);
		} else {
			Box box;
			box._rect.top = 0;
			box._rect.bottom = 6;
			box._rect.left = championStatusBoxX;
			box._rect.right = box._rect.left + 42;
			display.fillScreenBox(box, kDMColorDarkGary);
			txtMan.printToLogicalScreen(championStatusBoxX + 1, 5, nameColor, kDMColorDarkGary, curChampion->_name);
		}
	}
	if (getFlag(championAttributes, kDMAttributeStatistics)) {
		drawChampionBarGraphs(champIndex);
		if (isInventoryChampion) {
			drawHealthStaminaManaValues(curChampion);
			int16 nativeBitmapIndex;
			if ((curChampion->_food < 0) || (curChampion->_water < 0) || (curChampion->_poisonEventCount))
				nativeBitmapIndex = kDMGraphicIdxSlotBoxWounded;
			else
				nativeBitmapIndex = kDMGraphicIdxSlotBoxNormal;

			display.blitToViewport(display.getNativeBitmapOrGraphic(nativeBitmapIndex), boxMouth, k16_byteWidth, kDMColorDarkestGray, 18);
			nativeBitmapIndex = kDMGraphicIdxSlotBoxNormal;
			for (int i = kDMStatStrength; i <= kDMStatAntifire; i++) {
				if ((curChampion->_statistics[i][kDMStatCurrent] < curChampion->_statistics[i][kDMStatMaximum])) {
					nativeBitmapIndex = kDMGraphicIdxSlotBoxWounded;
					break;
				}
			}
			display.blitToViewport(display.getNativeBitmapOrGraphic(nativeBitmapIndex), boxEye, k16_byteWidth, kDMColorDarkestGray, 18);
			setFlag(championAttributes, kDMAttributeViewport);
		}
	}
	if (getFlag(championAttributes, kDMAttributeWounds)) {
		for (int i = isInventoryChampion ? kDMSlotFeet : kDMSlotActionHand; i >= kDMSlotReadyHand; i--)
			drawSlot(champIndex, i);

		if (isInventoryChampion)
			setFlag(championAttributes, kDMAttributeViewport);
	}
	if (getFlag(championAttributes, kDMAttributeLoad) && isInventoryChampion) {
		uint16 maxLoad = getMaximumLoad(curChampion);
		Color loadColor;
		if (curChampion->_load > maxLoad)
			loadColor = kDMColorRed;
		else if (((long)curChampion->_load << 3) > ((long)maxLoad * 5))
			loadColor = kDMColorYellow;
		else
			loadColor = kDMColorLightestGray;

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
			txtMan.printToViewport(104, 132, loadColor, "LOAD ");
			break;
		case Common::DE_DEU:
			txtMan.printToViewport(104, 132, loadColor, "LAST ");
			break;
		case Common::FR_FRA:
			txtMan.printToViewport(104, 132, loadColor, "CHARGE ");
			break;
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
		txtMan.printToViewport(148, 132, loadColor, _vm->_stringBuildBuffer);
		setFlag(championAttributes, kDMAttributeViewport);
	}
	DungeonMan &dungeon = *_vm->_dungeonMan;
	uint16 championIconIndex = getChampionIconIndex(curChampion->_cell, dungeon._partyDir);
	if (getFlag(championAttributes, kDMAttributeIcon) && (evtMan._useChampionIconOrdinalAsMousePointerBitmap != _vm->indexToOrdinal(championIconIndex))) {
		display.fillScreenBox(_boxChampionIcons[championIconIndex], _championColor[champIndex]);
		display.blitToBitmap(display.getNativeBitmapOrGraphic(kDMGraphicIdxChampionIcons), display._bitmapScreen, _boxChampionIcons[championIconIndex], getChampionIconIndex(curChampion->_dir, dungeon._partyDir) * 19, 0, k40_byteWidth, k160_byteWidthScreen, kDMColorDarkestGray, 14, k200_heightScreen);
	}
	if (getFlag(championAttributes, kDMAttributePanel) && isInventoryChampion) {
		if (_vm->_pressingMouth)
			inventory.drawPanelFoodWaterPoisoned();
		else if (_vm->_pressingEye) {
			if (_leaderEmptyHanded)
				inventory.drawChampionSkillsAndStatistics();
		} else
			inventory.drawPanel();

		setFlag(championAttributes, kDMAttributeViewport);
	}
	if (getFlag(championAttributes, kDMAttributeActionHand)) {
		drawSlot(champIndex, kDMSlotActionHand);
		_vm->_menuMan->drawActionIcon(champIndex);
		if (isInventoryChampion)
			setFlag(championAttributes, kDMAttributeViewport);
	}
	if (getFlag(championAttributes, kDMAttributeViewport))
		display.drawViewport(k0_viewportNotDungeonView);

	clearFlag(curChampion->_attributes, kDMAttributeNameTitle | kDMAttributeStatistics | kDMAttributeLoad | kDMAttributeIcon | kDMAttributePanel | kDMAttributeStatusBox | kDMAttributeWounds | kDMAttributeViewport | kDMAttributeActionHand);
	evtMan.hideMouse();
}

uint16 ChampionMan::getChampionIconIndex(int16 val, Direction dir) {
	return ((val + 4 - dir) & 0x3);
}

void ChampionMan::drawHealthStaminaManaValues(Champion *champ) {
	drawHealthOrStaminaOrManaValue(116, champ->_currHealth, champ->_maxHealth);
	drawHealthOrStaminaOrManaValue(124, champ->_currStamina, champ->_maxStamina);
	drawHealthOrStaminaOrManaValue(132, champ->_currMana, champ->_maxMana);
}

void ChampionMan::drawSlot(uint16 champIndex, int16 slotIndex) {
	Champion *champ = &_champions[champIndex];
	EventManager &evtMan = *_vm->_eventMan;
	DisplayMan &display = *_vm->_displayMan;
	InventoryMan &inventory = *_vm->_inventoryMan;

	int16 nativeBitmapIndex = -1;
	bool isInventoryChamp = (inventory._inventoryChampionOrdinal == _vm->indexToOrdinal(champIndex));

	uint16 slotBoxIndex;
	if (!isInventoryChamp) {
		// If drawing a slot for a champion other than the champion whose inventory is open
		if ((slotIndex > kDMSlotActionHand) || (_candidateChampionOrdinal == _vm->indexToOrdinal(champIndex)))
			return;
		slotBoxIndex = (champIndex << 1) + slotIndex;
	} else
		slotBoxIndex = kDMSlotBoxInventoryFirstSlot + slotIndex;

	Thing thing;
	if (slotIndex >= kDMSlotChest1)
		thing = inventory._chestSlots[slotIndex - kDMSlotChest1];
	else
		thing = champ->getSlot((ChampionSlot)slotIndex);

	SlotBox *slotBox = &_vm->_objectMan->_slotBoxes[slotBoxIndex];
	Box box;
	box._rect.left = slotBox->_x - 1;
	box._rect.top = slotBox->_y - 1;
	box._rect.right = box._rect.left + 17;
	box._rect.bottom = box._rect.top + 17;

	if (!isInventoryChamp)
		evtMan.hideMouse();

	int16 iconIndex;
	if (thing == _vm->_thingNone) {
		if (slotIndex <= kDMSlotFeet) {
			iconIndex = kDMIconIndiceReadyHand + (slotIndex << 1);
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex))) {
				iconIndex++;
				nativeBitmapIndex = kDMGraphicIdxSlotBoxWounded;
			} else
				nativeBitmapIndex = kDMGraphicIdxSlotBoxNormal;
		} else {
			if ((slotIndex >= kDMSlotNeck) && (slotIndex <= kDMSlotBackpackLine1_1))
				iconIndex = kDMIconIndiceNeck + (slotIndex - kDMSlotNeck);
			else
				iconIndex = kDMIconIndiceEmptyBox;
		}
	} else {
		iconIndex = _vm->_objectMan->getIconIndex(thing); // BUG0_35
		if (isInventoryChamp && (slotIndex == kDMSlotActionHand) && ((iconIndex == kDMIconIndiceContainerChestClosed) || (iconIndex == kDMIconIndiceScrollOpen))) {
			iconIndex++;
		} // BUG2_00
		if (slotIndex <= kDMSlotFeet) {
			if (champ->getWoundsFlag((ChampionWound)(1 << slotIndex)))
				nativeBitmapIndex = kDMGraphicIdxSlotBoxWounded;
			else
				nativeBitmapIndex = kDMGraphicIdxSlotBoxNormal;
		}
	}

	if ((slotIndex == kDMSlotActionHand) && (_vm->indexToOrdinal(champIndex) == _actingChampionOrdinal))
		nativeBitmapIndex = kDMGraphicIdxSlotBoxActingHand;

	if (nativeBitmapIndex != -1) {
		display._useByteBoxCoordinates = false;
		if (isInventoryChamp) {
			display.blitToBitmap(display.getNativeBitmapOrGraphic(nativeBitmapIndex),
												display._bitmapViewport, box, 0, 0, 16, k112_byteWidthViewport,
												kDMColorDarkestGray, display.getPixelHeight(nativeBitmapIndex), k136_heightViewport);
		} else {
			display.blitToBitmap(display.getNativeBitmapOrGraphic(nativeBitmapIndex),
												display._bitmapScreen, box, 0, 0, 16, k160_byteWidthScreen,
												kDMColorDarkestGray, display.getPixelHeight(nativeBitmapIndex), k136_heightViewport);
		}
	}

	_vm->_objectMan->drawIconInSlotBox(slotBoxIndex, iconIndex);

	if (!isInventoryChamp)
		evtMan.showMouse();
}

void ChampionMan::renameChampion(Champion *champ) {
#define kDMRenameChampionName 1
#define kDMRenameChampionTitle 2
	static const char underscoreCharacterString[2] = "_";
	static char renameChampionInputCharacterString[2] = " ";
	static const char reincarnateSpecialCharacters[6] = {',', '.', ';', ':', ' '};

	EventManager &evtMan = *_vm->_eventMan;
	TextMan &txtMan = *_vm->_textMan;
	DisplayMan &display = *_vm->_displayMan;

	Box displayBox;
	displayBox._rect.top = 3;
	displayBox._rect.bottom = 8;
	displayBox._rect.left = 3;
	displayBox._rect.right = displayBox._rect.left + 167;

	display.fillBoxBitmap(display._bitmapViewport, displayBox, kDMColorDarkestGray, k112_byteWidthViewport, k136_heightViewport);
	display.blitToViewport(display.getNativeBitmapOrGraphic(kDMGraphicIdxPanelRenameChampion), _vm->_inventoryMan->_boxPanel, k72_byteWidth, kDMColorCyan, 73);
	txtMan.printToViewport(177, 58, kDMColorLightestGray, "_______");
	txtMan.printToViewport(105, 76, kDMColorLightestGray, "___________________");
	evtMan.showMouse();
	display.drawViewport(k0_viewportNotDungeonView);
	evtMan.setMousePointerToNormal(k0_pointerArrow);
	evtMan.hideMouse();
	uint16 curCharacterIndex = 0;
	champ->_name[curCharacterIndex] = '\0';
	champ->_title[0] = '\0';
	int16 renamedChampionStringMode = kDMRenameChampionName;
	char *renamedChampionString = champ->_name;
	int16 textPosX = 177;
	int16 textPosY = 91;

	for (;;) { /*_Infinite loop_*/
		bool championTitleIsFull = ((renamedChampionStringMode == kDMRenameChampionTitle) && (curCharacterIndex == 19));
		if (!championTitleIsFull) {
			evtMan.showMouse();
			txtMan.printTextToBitmap(display._bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, kDMColorGold, kDMColorDarkestGray, underscoreCharacterString, k200_heightScreen);
			evtMan.hideMouse();
		}

		int16 curCharacter = 256;
		while (curCharacter == 256) {
			Common::Event event;
			Common::EventType eventType = evtMan.processInput(&event, &event);
			display.updateScreen();
			if (_vm->_engineShouldQuit)
				return;
			display.updateScreen();
				//_vm->f22_delay(1);

			if (eventType == Common::EVENT_LBUTTONDOWN) {
				// If left mouse button status has changed

				Common::Point mousePos = evtMan.getMousePos();
				if ((renamedChampionStringMode == kDMRenameChampionTitle || (curCharacterIndex > 0)) && (mousePos.x >= 197) && (mousePos.x <= 215) && (mousePos.y >= 147) && (mousePos.y <= 155)) { /* Coordinates of 'OK' button */
					int16 characterIndexBackup = curCharacterIndex;
					char championNameBackupString[8];
					renamedChampionString = champ->_name;
					strcpy(championNameBackupString, renamedChampionString);
					curCharacterIndex = strlen(renamedChampionString);
					// Replace space characters on the right of the champion name by '\0' characters
					while (renamedChampionString[--curCharacterIndex] == ' ')
						renamedChampionString[curCharacterIndex] = '\0';

					bool found = false;
					for (uint16 idx = kDMChampionFirst; idx < _partyChampionCount - 1; idx++) {
						if (!strcmp(_champions[idx]._name, renamedChampionString)) {
							// If an existing champion already has the specified name for the new champion
							found = true;
							break;
						}
					}
					if (!found)
						return;

					if (renamedChampionStringMode == kDMRenameChampionTitle)
						renamedChampionString = champ->_title;

					strcpy(renamedChampionString = champ->_name, championNameBackupString);
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
					evtMan.showMouse();
					txtMan.printTextToBitmap(display._bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, kDMColorLightestGray, kDMColorDarkestGray, renameChampionInputCharacterString, k200_heightScreen);
					evtMan.hideMouse();
					renamedChampionString[curCharacterIndex++] = curCharacter;
					renamedChampionString[curCharacterIndex] = '\0';
					textPosX += 6;
					if ((renamedChampionStringMode == kDMRenameChampionName) && (curCharacterIndex == 7)) {
						renamedChampionStringMode = kDMRenameChampionTitle;
						renamedChampionString = champ->_title;
						textPosX = 105;
						textPosY = 109;
						curCharacterIndex = 0;
					}
				}
			}
		} else if (curCharacter == '\r') { // Carriage return
			if ((renamedChampionStringMode == kDMRenameChampionName) && (curCharacterIndex > 0)) {
				evtMan.showMouse();
				txtMan.printTextToBitmap(display._bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, kDMColorLightestGray, kDMColorDarkestGray, underscoreCharacterString, k200_heightScreen);
				evtMan.hideMouse();
				renamedChampionStringMode = kDMRenameChampionTitle;
				renamedChampionString = champ->_title;
				textPosX = 105;
				textPosY = 109;
				curCharacterIndex = 0;
			}
		} else if (curCharacter == '\b') { // Backspace
			if ((renamedChampionStringMode == kDMRenameChampionName) && (curCharacterIndex == 0))
				continue;

			if (!championTitleIsFull) {
				evtMan.showMouse();
				txtMan.printTextToBitmap(display._bitmapScreen, k160_byteWidthScreen, textPosX, textPosY, kDMColorLightestGray, kDMColorDarkestGray, underscoreCharacterString, k200_heightScreen);
				evtMan.hideMouse();
			}
			if (curCharacterIndex == 0) {
				renamedChampionString = champ->_name;
				curCharacterIndex = strlen(renamedChampionString) - 1;
				renamedChampionStringMode = kDMRenameChampionName;
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

	bool ignoreTmpExp = getFlag(skillIndex, kDMIgnoreTemporaryExperience);
	bool ignoreObjModifiers = getFlag(skillIndex, kDMIgnoreObjectModifiers);
	clearFlag(skillIndex, kDMIgnoreTemporaryExperience | kDMIgnoreObjectModifiers);
	Champion *champ = &_champions[champIndex];
	Skill *skill = &champ->_skills[skillIndex];
	int32 exp = skill->_experience;
	if (!ignoreTmpExp)
		exp += skill->_temporaryExperience;

	if (skillIndex > kDMSkillWizard) {
		// Hidden skill
		skill = &champ->_skills[(skillIndex - kDMSkillSwing) >> 2];
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
		int16 actionHandIconIndex = _vm->_objectMan->getIconIndex(champ->_slots[kDMSlotActionHand]);
		if (actionHandIconIndex == kDMIconIndiceWeaponTheFirestaff)
			skillLevel++;
		else if (actionHandIconIndex == kDMIconIndiceWeaponTheFirestaffComplete)
			skillLevel += 2;

		int16 neckIconIndex = _vm->_objectMan->getIconIndex(champ->_slots[kDMSlotNeck]);
		switch (skillIndex) {
		case kDMSkillWizard:
			if (neckIconIndex == kDMIconIndiceJunkPendantFeral)
				skillLevel += 1;
			break;
		case kDMSkillHeal:
			// The skill modifiers of these two objects are not cumulative
			if ((neckIconIndex == kDMIconIndiceJunkGemOfAges) || (actionHandIconIndex == kDMIconIndiceWeaponSceptreOfLyf))
				skillLevel += 1;
			break;
		case kDMSkillInfluence:
			if (neckIconIndex == kDMIconIndiceJunkMoonstone)
				skillLevel += 1;
			break;
		case kDMSkillDefend:
			if (neckIconIndex == kDMIconIndiceJunkEkkhardCross)
				skillLevel += 1;
			break;
		default:
			break;
		}
	}
	return skillLevel;
}

}
