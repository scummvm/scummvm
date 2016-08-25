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


#include "projexpl.h"
#include "dungeonman.h"
#include "timeline.h"
#include "group.h"
#include "objectman.h"
#include "movesens.h"
#include "sounds.h"

namespace DM {

ProjExpl::ProjExpl(DMEngine* vm) : _vm(vm) {
	_creatureDamageOutcome = 0;
	_secondaryDirToOrFromParty = 0;
	_lastCreatureAttackTime = -200;
	_createLauncherProjectile = false;
	_projectilePoisonAttack = 0;
	_projectileAttackType = 0;
	_lastPartyMovementTime = 0;
}

void ProjExpl::createProjectile(Thing thing, int16 mapX, int16 mapY, uint16 cell, Direction dir, byte kineticEnergy, byte attack, byte stepEnergy) {
	Thing L0466_T_ProjectileThing;
	Projectile* L0467_ps_Projectile;
	TimelineEvent L0468_s_Event;


	if ((L0466_T_ProjectileThing = _vm->_dungeonMan->getUnusedThing(k14_ProjectileThingType)) == Thing::_none) { /* BUG0_16 If the game cannot create a projectile thing because it has run out of such things (60 maximum) then the object being thrown/shot/launched is orphaned. If the game has run out of projectile things it will try to remove a projectile from elsewhere in the dungeon, except in an area of 11x11 squares centered around the party (to make sure the player cannot actually see the thing disappear on screen) */
		return;
	}
	L0466_T_ProjectileThing = thingWithNewCell(L0466_T_ProjectileThing, cell);
	L0467_ps_Projectile = (Projectile *)_vm->_dungeonMan->getThingData(L0466_T_ProjectileThing);
	L0467_ps_Projectile->_slot = thing;
	L0467_ps_Projectile->_kineticEnergy = MIN((int16)kineticEnergy, (int16)255);
	L0467_ps_Projectile->_attack = attack;
	_vm->_dungeonMan->linkThingToList(L0466_T_ProjectileThing, Thing(0), mapX, mapY); /* Projectiles are added on the square and not 'moved' onto the square. In the case of a projectile launcher sensor, this means that the new projectile traverses the square in front of the launcher without any trouble: there is no impact if it is a wall, the projectile direction is not changed if it is a teleporter. Impacts with creatures and champions are still processed */
	setMapAndTime(L0468_s_Event._mapTime, _vm->_dungeonMan->_currMapIndex, _vm->_gameTime + 1);
	if (_createLauncherProjectile) {
		L0468_s_Event._type = k49_TMEventTypeMoveProjectile; /* Launcher projectiles can impact immediately */
	} else {
		L0468_s_Event._type = k48_TMEventTypeMoveProjectileIgnoreImpacts; /* Projectiles created by champions or creatures ignore impacts on their first movement */
	}
	L0468_s_Event._priority = 0;
	L0468_s_Event._B._slot = L0466_T_ProjectileThing.toUint16();
	L0468_s_Event._C._projectile.setMapX(mapX);
	L0468_s_Event._C._projectile.setMapY(mapY);
	L0468_s_Event._C._projectile.setStepEnergy(stepEnergy);
	L0468_s_Event._C._projectile.setDir(dir);
	L0467_ps_Projectile->_eventIndex = _vm->_timeline->f238_addEventGetEventIndex(&L0468_s_Event);
}

bool ProjExpl::hasProjectileImpactOccurred(int16 impactType, int16 mapXCombo, int16 mapYCombo, int16 cell, Thing projectileThing) {
#define AP0454_i_ProjectileTargetMapX mapXCombo
#define AP0455_i_ProjectileTargetMapY mapYCombo
#define AP0456_i_ChampionIndex cell
	Projectile* L0490_ps_Projectile;
	Group* L0491_ps_Group;
	Thing L0486_T_ProjectileAssociatedThing;
	int16 L0487_i_Multiple;
#define AL0487_i_DoorState  L0487_i_Multiple
#define AL0487_i_IconIndex  L0487_i_Multiple
#define AL0487_i_Outcome    L0487_i_Multiple
#define AL0487_i_WeaponType L0487_i_Multiple
	int16 L0488_i_Attack = 0;
	Potion* L0492_ps_Potion = nullptr;
	CreatureInfo* L0493_ps_CreatureInfo;
	Door* L0494_ps_Door;
	Weapon* L0495_ps_Weapon;
	uint16* L0496_pui_CreatureHealth;
	Thing* L0497_pT_GroupSlot;
	Thing L0498_T_ExplosionThing;
	int16 L0499_i_ProjectileMapX;
	int16 L0500_i_ProjectileMapY;
	int16 L0501_i_MapXCombo;
	int16 L0502_i_MapYCombo;
	byte L0503_uc_Square;
	bool L0505_B_CreateExplosionOnImpact;
	int16 L0489_i_ChampionAttack;
	uint16 L0507_ui_Multiple;
#define AL0507_ui_ExplosionAttack L0507_ui_Multiple
#define AL0507_ui_SoundIndex      L0507_ui_Multiple
	int16 L0508_i_PotionPower = 0;
	bool L0509_B_RemovePotion;
	int16 L0510_i_ProjectileAssociatedThingType;
	uint16 L0511_ui_CreatureType;
	uint16 L0512_ui_CreatureIndex;

	L0490_ps_Projectile = (Projectile *)_vm->_dungeonMan->getThingData(Thing(projectileThing));
	L0501_i_MapXCombo = mapXCombo;
	L0502_i_MapYCombo = mapYCombo;
	L0509_B_RemovePotion = false;
	_creatureDamageOutcome = k0_outcomeKilledNoCreaturesInGroup;
	if ((L0510_i_ProjectileAssociatedThingType = (L0486_T_ProjectileAssociatedThing = L0490_ps_Projectile->_slot).getType()) == k8_PotionThingType) {
		L0491_ps_Group = (Group *)_vm->_dungeonMan->getThingData(L0486_T_ProjectileAssociatedThing);
		switch (((Potion *)L0491_ps_Group)->getType()) {
		case k3_PotionTypeVen:
			L0498_T_ExplosionThing = Thing::_explPoisonCloud;
			goto T0217004;
		case k19_PotionTypeFulBomb:
			L0498_T_ExplosionThing = Thing::_explFireBall;
T0217004:
			L0509_B_RemovePotion = true;
			L0508_i_PotionPower = ((Potion *)L0491_ps_Group)->getPower();
			L0492_ps_Potion = (Potion *)L0491_ps_Group;
			break;
		default:
			break;
		}
	}
	L0505_B_CreateExplosionOnImpact = (L0510_i_ProjectileAssociatedThingType == k15_ExplosionThingType) && (L0486_T_ProjectileAssociatedThing != Thing::_explSlime) && (L0486_T_ProjectileAssociatedThing != Thing::_explPoisonBolt);
	L0497_pT_GroupSlot = NULL;
	L0489_i_ChampionAttack = 0;
	if (mapXCombo <= 255) {
		L0499_i_ProjectileMapX = mapXCombo;
		L0500_i_ProjectileMapY = mapYCombo;
	} else {
		L0499_i_ProjectileMapX = (mapXCombo >> 8) - 1;
		L0500_i_ProjectileMapY = (mapYCombo >> 8);
		AP0454_i_ProjectileTargetMapX &= 0x00FF;
		AP0455_i_ProjectileTargetMapY &= 0x00FF;
	}
	switch (impactType) {
	case k4_DoorElemType:
		AL0487_i_DoorState = Square(L0503_uc_Square = _vm->_dungeonMan->_currMapData[AP0454_i_ProjectileTargetMapX][AP0455_i_ProjectileTargetMapY]).getDoorState();
		L0494_ps_Door = (Door *)_vm->_dungeonMan->getSquareFirstThingData(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY);
		if ((AL0487_i_DoorState != k5_doorState_DESTROYED) && (L0486_T_ProjectileAssociatedThing == Thing::_explOpenDoor)) {
			if (L0494_ps_Door->hasButton()) {
				_vm->_moveSens->addEvent(k10_TMEventTypeDoor, AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, 0, k2_SensorEffToggle, _vm->_gameTime + 1);
			}
			break;
		}
		if ((AL0487_i_DoorState == k5_doorState_DESTROYED) ||
			(AL0487_i_DoorState <= k1_doorState_FOURTH) ||
			(getFlag(_vm->_dungeonMan->_currMapDoorInfo[L0494_ps_Door->getType()]._attributes, k0x0002_MaskDoorInfo_ProjectilesCanPassThrough) &&
			((L0510_i_ProjectileAssociatedThingType == k15_ExplosionThingType) ?
			 (L0486_T_ProjectileAssociatedThing.toUint16() >= Thing::_explHarmNonMaterial.toUint16()) :
			 ((L0490_ps_Projectile->_attack > _vm->getRandomNumber(128)) &&
			  getFlag(_vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->getObjectInfoIndex(L0486_T_ProjectileAssociatedThing)].getAllowedSlots(), k0x0100_ObjectAllowedSlotPouchPassAndThroughDoors)
			  && ((L0510_i_ProjectileAssociatedThingType != k10_JunkThingType) ||
			  ((AL0487_i_IconIndex = _vm->_objectMan->getIconIndex(L0486_T_ProjectileAssociatedThing)) < 0) ||
				  (!((AL0487_i_IconIndex >= k176_IconIndiceJunkIronKey) && (AL0487_i_IconIndex <= k191_IconIndiceJunkMasterKey))))
			  )))) { /* ASSEMBLY_COMPILATION_DIFFERENCE jmp */
			return false;
		}
		L0488_i_Attack = getProjectileImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing) + 1;
		_vm->_groupMan->groupIsDoorDestoryedByAttack(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, L0488_i_Attack + _vm->getRandomNumber(L0488_i_Attack), false, 0);
		break;
	case kM2_ChampionElemType:
		if ((AP0456_i_ChampionIndex = _vm->_championMan->getIndexInCell(cell)) < 0) {
			return false;
		}
		L0489_i_ChampionAttack = L0488_i_Attack = getProjectileImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing);
		break;
	case kM1_CreatureElemType:
		L0491_ps_Group = (Group *)_vm->_dungeonMan->getThingData(_vm->_groupMan->groupGetThing(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY));
		if (!(L0512_ui_CreatureIndex = _vm->_groupMan->getCreatureOrdinalInCell(L0491_ps_Group, cell))) {
			return false;
		}
		L0512_ui_CreatureIndex--;
		L0493_ps_CreatureInfo = &g243_CreatureInfo[L0511_ui_CreatureType = L0491_ps_Group->_type];
		if ((L0486_T_ProjectileAssociatedThing == Thing::_explFireBall) && (L0511_ui_CreatureType == k11_CreatureTypeBlackFlame)) {
			L0496_pui_CreatureHealth = &L0491_ps_Group->_health[L0512_ui_CreatureIndex];
			*L0496_pui_CreatureHealth = MIN(1000, *L0496_pui_CreatureHealth + getProjectileImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing));
			goto T0217044;
		}
		if (getFlag(L0493_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial) && (L0486_T_ProjectileAssociatedThing != Thing::_explHarmNonMaterial)) {
			return false;
		}
		L0488_i_Attack = (uint16)((unsigned long)getProjectileImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing) << 6) / L0493_ps_CreatureInfo->_defense;
		if (L0488_i_Attack) {
			if ((AL0487_i_Outcome = _vm->_groupMan->groupGetDamageCreatureOutcome(L0491_ps_Group, L0512_ui_CreatureIndex, AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, L0488_i_Attack + _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(L0511_ui_CreatureType, _projectilePoisonAttack), true)) != k0_outcomeKilledNoCreaturesInGroup) {
				_vm->_groupMan->processEvents29to41(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, kM2_TMEventTypeCreateReactionEvent30HitByProjectile, 0);
			}
			_creatureDamageOutcome = AL0487_i_Outcome;
			if (!L0505_B_CreateExplosionOnImpact &&
				(AL0487_i_Outcome == k0_outcomeKilledNoCreaturesInGroup) &&
				(L0510_i_ProjectileAssociatedThingType == k5_WeaponThingType) &&
				getFlag(L0493_ps_CreatureInfo->_attributes, k0x0400_MaskCreatureInfo_keepThrownSharpWeapon)) {
				L0495_ps_Weapon = (Weapon *)_vm->_dungeonMan->getThingData(L0486_T_ProjectileAssociatedThing);
				AL0487_i_WeaponType = L0495_ps_Weapon->getType();
				if ((AL0487_i_WeaponType == k8_WeaponTypeDagger) || (AL0487_i_WeaponType == k27_WeaponTypeArrow) || (AL0487_i_WeaponType == k28_WeaponTypeSlayer) || (AL0487_i_WeaponType == k31_WeaponTypePoisonDart) || (AL0487_i_WeaponType == k32_WeaponTypeThrowingStar)) {
					L0497_pT_GroupSlot = &L0491_ps_Group->_slot;
				}
			}
		}
	}
	if (L0489_i_ChampionAttack && _vm->_championMan->addPendingDamageAndWounds_getDamage(AP0456_i_ChampionIndex, L0488_i_Attack, k0x0004_ChampionWoundHead | k0x0008_ChampionWoundTorso, _projectileAttackType) && _projectilePoisonAttack && _vm->getRandomNumber(2)) {
		_vm->_championMan->championPoison(AP0456_i_ChampionIndex, _projectilePoisonAttack);
	}
	if (L0505_B_CreateExplosionOnImpact || L0509_B_RemovePotion
		) {
		if (L0509_B_RemovePotion) {
			L0486_T_ProjectileAssociatedThing = L0498_T_ExplosionThing;
			AL0507_ui_ExplosionAttack = L0508_i_PotionPower;
		} else {
			AL0507_ui_ExplosionAttack = L0490_ps_Projectile->_kineticEnergy;
		}
		if ((L0486_T_ProjectileAssociatedThing == Thing::_explLightningBolt) && !(AL0507_ui_ExplosionAttack >>= 1))
			goto T0217044;
		createExplosion(L0486_T_ProjectileAssociatedThing, AL0507_ui_ExplosionAttack, L0501_i_MapXCombo, L0502_i_MapYCombo, (L0486_T_ProjectileAssociatedThing == Thing::_explPoisonCloud) ? k255_CreatureTypeSingleCenteredCreature : cell);
	} else {
		if ((L0486_T_ProjectileAssociatedThing).getType() == k5_WeaponThingType) {
			AL0507_ui_SoundIndex = k00_soundMETALLIC_THUD;
		} else {
			if (L0486_T_ProjectileAssociatedThing == Thing::_explPoisonBolt) {
				AL0507_ui_SoundIndex = k13_soundSPELL;
			} else {
				AL0507_ui_SoundIndex = k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM;
			}
		}
		_vm->_sound->f064_SOUND_RequestPlay_CPSD(AL0507_ui_SoundIndex, L0499_i_ProjectileMapX, L0500_i_ProjectileMapY, k1_soundModePlayIfPrioritized);
	}
T0217044:
	if (L0509_B_RemovePotion) {
		L0492_ps_Potion->_nextThing = Thing::_none;
		L0490_ps_Projectile->_slot = L0498_T_ExplosionThing;
	}
	_vm->_dungeonMan->unlinkThingFromList(projectileThing, Thing(0), L0499_i_ProjectileMapX, L0500_i_ProjectileMapY);
	projectileDelete(projectileThing, L0497_pT_GroupSlot, L0499_i_ProjectileMapX, L0500_i_ProjectileMapY);
	return true;
}

uint16 ProjExpl::getProjectileImpactAttack(Projectile* projectile, Thing thing) {
	WeaponInfo* L0485_ps_WeaponInfo;
	uint16 L0483_ui_Multiple;
#define AL0483_ui_ThingType L0483_ui_Multiple
#define AL0483_ui_Attack    L0483_ui_Multiple
	uint16 L0484_ui_KineticEnergy;


	_projectilePoisonAttack = 0;
	_projectileAttackType = k3_attackType_BLUNT;

	L0484_ui_KineticEnergy = projectile->_kineticEnergy;
	if ((AL0483_ui_ThingType = thing.getType()) != k15_ExplosionThingType) {
		if (AL0483_ui_ThingType == k5_WeaponThingType) {
			L0485_ps_WeaponInfo = _vm->_dungeonMan->getWeaponInfo(thing);
			AL0483_ui_Attack = L0485_ps_WeaponInfo->_kineticEnergy;
			_projectileAttackType = k3_attackType_BLUNT;
		} else {
			AL0483_ui_Attack = _vm->getRandomNumber(4);
		}
		AL0483_ui_Attack += _vm->_dungeonMan->getObjectWeight(thing) >> 1;
	} else {
		if (thing == Thing::_explSlime) {
			AL0483_ui_Attack = _vm->getRandomNumber(16);
			_projectilePoisonAttack = AL0483_ui_Attack + 10;
			AL0483_ui_Attack += _vm->getRandomNumber(32);
		} else {
			if (thing.toUint16() >= Thing::_explHarmNonMaterial.toUint16()) {
				_projectileAttackType = k5_attackType_MAGIC;
				if (thing == Thing::_explPoisonBolt) {
					_projectilePoisonAttack = L0484_ui_KineticEnergy;
					return 1;
				}
				return 0;
			}
			_projectileAttackType = k1_attackType_FIRE;
			AL0483_ui_Attack = _vm->getRandomNumber(16) + _vm->getRandomNumber(16) + 10;
			if (thing == Thing::_explLightningBolt) {
				_projectileAttackType = k7_attackType_LIGHTNING;
				AL0483_ui_Attack *= 5;
			}
		}
	}
	AL0483_ui_Attack = ((AL0483_ui_Attack + L0484_ui_KineticEnergy) >> 4) + 1;
	AL0483_ui_Attack += _vm->getRandomNumber((AL0483_ui_Attack >> 1) + 1) + _vm->getRandomNumber(4);
	AL0483_ui_Attack = MAX(AL0483_ui_Attack >> 1, AL0483_ui_Attack - (32 - (projectile->_attack >> 3)));
	return AL0483_ui_Attack;
}

void ProjExpl::createExplosion(Thing explThing, uint16 attack, uint16 mapXCombo, uint16 mapYCombo, uint16 cell) {
#define AP0443_ui_ProjectileMapX mapXCombo
#define AP0444_ui_ProjectileMapY mapYCombo
	Thing L0473_T_Thing = _vm->_dungeonMan->getUnusedThing(k15_ExplosionThingType);
	if (L0473_T_Thing == Thing::_none) {
		return;
	}

	Explosion *L0470_ps_Explosion = &((Explosion *)_vm->_dungeonMan->_thingData[k15_ExplosionThingType])[(L0473_T_Thing).getIndex()];
	int16 L0474_i_ProjectileTargetMapX;
	int16 L0475_i_ProjectileTargetMapY;
	if (mapXCombo <= 255) {
		L0474_i_ProjectileTargetMapX = mapXCombo;
		L0475_i_ProjectileTargetMapY = mapYCombo;
	} else {
		L0474_i_ProjectileTargetMapX = mapXCombo & 0x00FF;
		L0475_i_ProjectileTargetMapY = mapYCombo & 0x00FF;
		AP0443_ui_ProjectileMapX >>= 8;
		AP0443_ui_ProjectileMapX--;
		AP0444_ui_ProjectileMapY >>= 8;
	}
	if (cell == k255_CreatureTypeSingleCenteredCreature) {
		L0470_ps_Explosion->setCentered(true);
	} else {
		L0470_ps_Explosion->setCentered(false);
		L0473_T_Thing = thingWithNewCell(L0473_T_Thing, cell);
	}
	L0470_ps_Explosion->setType(explThing.toUint16() - Thing::_firstExplosion.toUint16());
	L0470_ps_Explosion->setAttack(attack);
	if (explThing.toUint16() < Thing::_explHarmNonMaterial.toUint16()) {
		_vm->_sound->f064_SOUND_RequestPlay_CPSD((attack > 80) ? k05_soundSTRONG_EXPLOSION : k20_soundWEAK_EXPLOSION, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, k1_soundModePlayIfPrioritized);
	} else {
		if (explThing != Thing::_explSmoke) {
			_vm->_sound->f064_SOUND_RequestPlay_CPSD(k13_soundSPELL, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, k1_soundModePlayIfPrioritized);
		}
	}
	_vm->_dungeonMan->linkThingToList(L0473_T_Thing, Thing(0), AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY);
	TimelineEvent L0476_s_Event;
	setMapAndTime(L0476_s_Event._mapTime, _vm->_dungeonMan->_currMapIndex, _vm->_gameTime + ((explThing == Thing::_explRebirthStep1) ? 5 : 1));
	L0476_s_Event._type = k25_TMEventTypeExplosion;
	L0476_s_Event._priority = 0;
	L0476_s_Event._C._slot = L0473_T_Thing.toUint16();
	L0476_s_Event._B._location._mapX = AP0443_ui_ProjectileMapX;
	L0476_s_Event._B._location._mapY = AP0444_ui_ProjectileMapY;
	_vm->_timeline->f238_addEventGetEventIndex(&L0476_s_Event);
	if ((explThing == Thing::_explLightningBolt) || (explThing == Thing::_explFireBall)) {
		AP0443_ui_ProjectileMapX = L0474_i_ProjectileTargetMapX;
		AP0444_ui_ProjectileMapY = L0475_i_ProjectileTargetMapY;
		attack = (attack >> 1) + 1;
		attack += _vm->getRandomNumber(attack) + 1;
		if ((explThing == Thing::_explFireBall) || (attack >>= 1)) {
			if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (AP0443_ui_ProjectileMapX == _vm->_dungeonMan->_partyMapX) && (AP0444_ui_ProjectileMapY == _vm->_dungeonMan->_partyMapY)) {
				_vm->_championMan->getDamagedChampionCount(attack, k0x0001_ChampionWoundReadHand | k0x0002_ChampionWoundActionHand | k0x0004_ChampionWoundHead | k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs | k0x0020_ChampionWoundFeet, k1_attackType_FIRE);
			} else {
				if ((L0473_T_Thing = _vm->_groupMan->groupGetThing(AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY)) != Thing::_endOfList) { /* ASSEMBLY_COMPILATION_DIFFERENCE jmp */
					Group *L0472_ps_Group = (Group *)_vm->_dungeonMan->getThingData(L0473_T_Thing);
					CreatureInfo *L0471_ps_CreatureInfo = &g243_CreatureInfo[L0472_ps_Group->_type];
					int16 L0469_i_CreatureFireResistance = L0471_ps_CreatureInfo->getFireResistance();
					if (L0469_i_CreatureFireResistance != k15_immuneToFire) {
						if (getFlag(L0471_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
							attack >>= 2;
						}
						if ((attack -= _vm->getRandomNumber((L0469_i_CreatureFireResistance << 1) + 1)) > 0) {
							_creatureDamageOutcome = _vm->_groupMan->getDamageAllCreaturesOutcome(L0472_ps_Group, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, attack, true);
						}
					}
				}
			}
		}
	}
}

int16 ProjExpl::projectileGetImpactCount(int16 impactType, int16 mapX, int16 mapY, int16 cell) {
	Thing L0513_T_Thing;
	int16 L0514_i_ImpactCount;


	L0514_i_ImpactCount = 0;
	_creatureDamageOutcome = k0_outcomeKilledNoCreaturesInGroup;
T0218001:
	L0513_T_Thing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	while (L0513_T_Thing != Thing::_endOfList) {
		if (((L0513_T_Thing).getType() == k14_ProjectileThingType) &&
			((L0513_T_Thing).getCell() == cell) &&
			hasProjectileImpactOccurred(impactType, mapX, mapY, cell, L0513_T_Thing)) {
			projectileDeleteEvent(L0513_T_Thing);
			L0514_i_ImpactCount++;
			if ((impactType == kM1_CreatureElemType) && (_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup))
				break;
			goto T0218001;
		}
		L0513_T_Thing = _vm->_dungeonMan->getNextThing(L0513_T_Thing);
	}
	return L0514_i_ImpactCount;
}

void ProjExpl::projectileDeleteEvent(Thing thing) {
	Projectile* L0477_ps_Projectile;


	L0477_ps_Projectile = (Projectile *)_vm->_dungeonMan->getThingData(thing);
	_vm->_timeline->f237_deleteEvent(L0477_ps_Projectile->_eventIndex);
}

void ProjExpl::projectileDelete(Thing projectileThing, Thing* groupSlot, int16 mapX, int16 mapY) {
	Projectile *L0480_ps_Projectile = (Projectile *)_vm->_dungeonMan->getThingData(projectileThing);
	Thing L0479_T_Thing = L0480_ps_Projectile->_slot;
	if (L0479_T_Thing.getType() != k15_ExplosionThingType) {
		if (groupSlot != NULL) {
			Thing L0478_T_PreviousThing = *groupSlot;
			if (L0478_T_PreviousThing == Thing::_endOfList) {
				Thing *L0481_ps_Generic = (Thing *)_vm->_dungeonMan->getThingData(L0479_T_Thing);
				*L0481_ps_Generic = Thing::_endOfList;
				*groupSlot = L0479_T_Thing;
			} else {
				_vm->_dungeonMan->linkThingToList(L0479_T_Thing, L0478_T_PreviousThing, kM1_MapXNotOnASquare, 0);
			}
		} else {
			_vm->_moveSens->getMoveResult(Thing((L0479_T_Thing).getTypeAndIndex() | getFlag(projectileThing.toUint16(), 0xC)), -2, 0, mapX, mapY);
		}
	}
	L0480_ps_Projectile->_nextThing = Thing::_none;
}

void ProjExpl::processEvents48To49(TimelineEvent* event) {
	TimelineEvent* L0519_ps_Event;
	Projectile* L0520_ps_Projectile;
	Thing L0515_T_ProjectileThingNewCell;
	uint16 L0516_ui_Multiple;
#define AL0516_ui_StepEnergy L0516_ui_Multiple
#define AL0516_ui_Square     L0516_ui_Multiple
	Thing L0521_T_ProjectileThing;
	uint16 L0517_ui_ProjectileDirection;
	bool L0522_B_ProjectileMovesToOtherSquare;
	int16 L0523_i_DestinationMapX;
	int16 L0524_i_DestinationMapY;
	uint16 L0518_ui_Cell;
	int16 L0525_i_SourceMapX = -1;
	int16 L0526_i_SourceMapY = -1;
	TimelineEvent L0527_s_Event;


	L0527_s_Event = *event;
	L0519_ps_Event = &L0527_s_Event;
	L0520_ps_Projectile = (Projectile*)_vm->_dungeonMan->getThingData(L0521_T_ProjectileThing = L0515_T_ProjectileThingNewCell = Thing(L0519_ps_Event->_B._slot));
	L0523_i_DestinationMapX = L0519_ps_Event->_C._projectile.getMapX();
	L0524_i_DestinationMapY = L0519_ps_Event->_C._projectile.getMapY();
	if (L0519_ps_Event->_type == k48_TMEventTypeMoveProjectileIgnoreImpacts) {
		L0519_ps_Event->_type = k49_TMEventTypeMoveProjectile;
	} else {
		L0518_ui_Cell = (L0515_T_ProjectileThingNewCell).getCell();
		if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (L0523_i_DestinationMapX == _vm->_dungeonMan->_partyMapX) && (L0524_i_DestinationMapY == _vm->_dungeonMan->_partyMapY) && hasProjectileImpactOccurred(kM2_ChampionElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0515_T_ProjectileThingNewCell)) {
			return;
		}
		if ((_vm->_groupMan->groupGetThing(L0523_i_DestinationMapX, L0524_i_DestinationMapY) != Thing::_endOfList) && hasProjectileImpactOccurred(kM1_CreatureElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0521_T_ProjectileThing)) {
			return;
		}
		if (L0520_ps_Projectile->_kineticEnergy <= (AL0516_ui_StepEnergy = L0519_ps_Event->_C._projectile.getStepEnergy())) {
			_vm->_dungeonMan->unlinkThingFromList(L0515_T_ProjectileThingNewCell = L0521_T_ProjectileThing, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
			projectileDelete(L0515_T_ProjectileThingNewCell, NULL, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
			return;
		}
		L0520_ps_Projectile->_kineticEnergy -= AL0516_ui_StepEnergy;
		if (L0520_ps_Projectile->_attack < AL0516_ui_StepEnergy) {
			L0520_ps_Projectile->_attack = 0;
		} else {
			L0520_ps_Projectile->_attack -= AL0516_ui_StepEnergy;
		}
	}
	L0517_ui_ProjectileDirection = L0519_ps_Event->_C._projectile.getDir();
	L0515_T_ProjectileThingNewCell = Thing(L0519_ps_Event->_B._slot);
	L0518_ui_Cell = L0515_T_ProjectileThingNewCell.getCell();
	L0522_B_ProjectileMovesToOtherSquare = (L0517_ui_ProjectileDirection == L0518_ui_Cell) || (returnNextVal(L0517_ui_ProjectileDirection) == L0518_ui_Cell);
	if (L0522_B_ProjectileMovesToOtherSquare) {
		L0525_i_SourceMapX = L0523_i_DestinationMapX;
		L0526_i_SourceMapY = L0524_i_DestinationMapY;
		L0523_i_DestinationMapX += _vm->_dirIntoStepCountEast[L0517_ui_ProjectileDirection], L0524_i_DestinationMapY += _vm->_dirIntoStepCountNorth[L0517_ui_ProjectileDirection];
		AL0516_ui_Square = _vm->_dungeonMan->getSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY).toByte();
		if ((Square(AL0516_ui_Square).getType() == k0_WallElemType) ||
			((Square(AL0516_ui_Square).getType() == k6_FakeWallElemType) && !getFlag(AL0516_ui_Square, (k0x0001_FakeWallImaginary | k0x0004_FakeWallOpen))) ||
			((Square(AL0516_ui_Square).getType() == k3_StairsElemType) && (Square(_vm->_dungeonMan->_currMapData[L0525_i_SourceMapX][L0526_i_SourceMapY]).getType() == k3_StairsElemType))) {
			if (hasProjectileImpactOccurred(Square(AL0516_ui_Square).getType(), L0525_i_SourceMapX, L0526_i_SourceMapY, L0518_ui_Cell, L0515_T_ProjectileThingNewCell)) {
				return;
			}
		}
	}
	if ((L0517_ui_ProjectileDirection & 0x0001) == (L0518_ui_Cell & 0x0001)) {
		L0518_ui_Cell--;
	} else {
		L0518_ui_Cell++;
	}
	L0515_T_ProjectileThingNewCell = thingWithNewCell(L0515_T_ProjectileThingNewCell, L0518_ui_Cell &= 0x0003);
	if (L0522_B_ProjectileMovesToOtherSquare) {
		_vm->_moveSens->getMoveResult(L0515_T_ProjectileThingNewCell, L0525_i_SourceMapX, L0526_i_SourceMapY, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
		L0519_ps_Event->_C._projectile.setMapX(_vm->_moveSens->_moveResultMapX);
		L0519_ps_Event->_C._projectile.setMapY(_vm->_moveSens->_moveResultMapY);
		L0519_ps_Event->_C._projectile.setDir((Direction)_vm->_moveSens->_moveResultDir);
		L0515_T_ProjectileThingNewCell = thingWithNewCell(L0515_T_ProjectileThingNewCell, _vm->_moveSens->_moveResultCell);
		M31_setMap(L0519_ps_Event->_mapTime, _vm->_moveSens->_moveResultMapIndex);
	} else {
		if ((Square(_vm->_dungeonMan->getSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY)).getType() == k4_DoorElemType) && hasProjectileImpactOccurred(k4_DoorElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0521_T_ProjectileThing)) {
			return;
		}
		_vm->_dungeonMan->unlinkThingFromList(L0515_T_ProjectileThingNewCell, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
		_vm->_dungeonMan->linkThingToList(L0515_T_ProjectileThingNewCell, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
	}
	L0519_ps_Event->_mapTime += (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) ? 1 : 3;
	//Strangerke: CHECKME: Maybe we should keep that piece of code too as it sounds like it's fixing a weird behavior of projectiles on different maps
#ifdef COMPILE42_CSB20EN_CSB21EN /* CHANGE7_20_IMPROVEMENT Projectiles now move at the same speed on all maps instead of moving slower on maps other than the party map */
	L0519_ps_Event->Map_Time++;
#endif
	L0519_ps_Event->_B._slot = L0515_T_ProjectileThingNewCell.toUint16();
	L0520_ps_Projectile->_eventIndex = _vm->_timeline->f238_addEventGetEventIndex(L0519_ps_Event);
}

void ProjExpl::processEvent25(TimelineEvent* event) {
	Explosion* L0532_ps_Explosion;
	Group* L0533_ps_Group = nullptr;
	CreatureInfo* L0534_ps_CreatureInfo = nullptr;
	uint16 L0528_ui_MapX;
	uint16 L0529_ui_MapY;
	int16 L0530_i_Attack;
	int16 L0531_i_Multiple;
#define AL0531_i_SquareType    L0531_i_Multiple
#define AL0531_i_CreatureCount L0531_i_Multiple
	Thing L0535_T_GroupThing;
	Thing L0536_T_ExplosionThing;
	uint16 L0537_ui_Multiple = 0;
#define AL0537_ui_CreatureType                L0537_ui_Multiple
#define AL0537_ui_NonMaterialAdditionalAttack L0537_ui_Multiple
	bool L0538_B_ExplosionOnPartySquare;
	TimelineEvent L0539_s_Event;

	L0528_ui_MapX = event->_B._location._mapX;
	L0529_ui_MapY = event->_B._location._mapY;
	L0532_ps_Explosion = &((Explosion*)_vm->_dungeonMan->_thingData[k15_ExplosionThingType])[Thing((event->_C._slot)).getIndex()];
	AL0531_i_SquareType = Square(_vm->_dungeonMan->_currMapData[L0528_ui_MapX][L0529_ui_MapY]).getType();
	L0538_B_ExplosionOnPartySquare = (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (L0528_ui_MapX == _vm->_dungeonMan->_partyMapX) && (L0529_ui_MapY == _vm->_dungeonMan->_partyMapY);
	if ((L0535_T_GroupThing = _vm->_groupMan->groupGetThing(L0528_ui_MapX, L0529_ui_MapY)) != Thing::_endOfList) {
		L0533_ps_Group = (Group*)_vm->_dungeonMan->getThingData(L0535_T_GroupThing);
		L0534_ps_CreatureInfo = &g243_CreatureInfo[AL0537_ui_CreatureType = L0533_ps_Group->_type];
	}
	if ((L0536_T_ExplosionThing = Thing(Thing::_firstExplosion.toUint16() + L0532_ps_Explosion->getType())) == Thing::_explPoisonCloud) {
		L0530_i_Attack = MAX(1, MIN(L0532_ps_Explosion->getAttack() >> 5, 4) + _vm->getRandomNumber(2)); /* Value between 1 and 5 */
	} else {
		L0530_i_Attack = (L0532_ps_Explosion->getAttack() >> 1) + 1;
		L0530_i_Attack += _vm->getRandomNumber(L0530_i_Attack) + 1;
	}


	switch (L0536_T_ExplosionThing.toUint16()) {
	case 0xFF82:
		if (!(L0530_i_Attack >>= 1))
			break;
	case 0xFF80:
		if (AL0531_i_SquareType == k4_DoorElemType) {
			_vm->_groupMan->groupIsDoorDestoryedByAttack(L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, true, 0);
		}
		break;
	case 0xFF83:
		if ((L0535_T_GroupThing != Thing::_endOfList) && getFlag(L0534_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
			if ((AL0537_ui_CreatureType == k19_CreatureTypeMaterializerZytaz) && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)) { /* ASSEMBLY_COMPILATION_DIFFERENCE jmp */
				L0530_i_Attack -= (AL0537_ui_NonMaterialAdditionalAttack = L0530_i_Attack >> 3);
				AL0537_ui_NonMaterialAdditionalAttack <<= 1;
				AL0537_ui_NonMaterialAdditionalAttack++;
				AL0531_i_CreatureCount = L0533_ps_Group->getCount();
				do {
					if (getFlag(_vm->_groupMan->_activeGroups[L0533_ps_Group->getActiveGroupIndex()]._aspect[AL0531_i_CreatureCount], k0x0080_MaskActiveGroupIsAttacking)) { /* Materializer / Zytaz can only be damaged while they are attacking */
						_vm->_groupMan->groupGetDamageCreatureOutcome(L0533_ps_Group, AL0531_i_CreatureCount, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack + _vm->getRandomNumber(AL0537_ui_NonMaterialAdditionalAttack) + _vm->getRandomNumber(4), true);
					}
				} while (--AL0531_i_CreatureCount >= 0);
			} else {
				_vm->_groupMan->getDamageAllCreaturesOutcome(L0533_ps_Group, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, true);
			}
		}
		break;
	case 0xFFE4:
		L0532_ps_Explosion->setType(L0532_ps_Explosion->getType() + 1);
		_vm->_sound->f064_SOUND_RequestPlay_CPSD(k05_soundSTRONG_EXPLOSION, L0528_ui_MapX, L0529_ui_MapY, k1_soundModePlayIfPrioritized);
		goto T0220026;
	case 0xFFA8:
		if (L0532_ps_Explosion->getAttack() > 55) {
			L0532_ps_Explosion->setAttack(L0532_ps_Explosion->getAttack() - 40);
			goto T0220026;
		}
		break;
	case 0xFF87:
		if (L0538_B_ExplosionOnPartySquare) {
			_vm->_championMan->getDamagedChampionCount(L0530_i_Attack, k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
		} else {
			if ((L0535_T_GroupThing != Thing::_endOfList) && (L0530_i_Attack = _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(AL0537_ui_CreatureType, L0530_i_Attack)) && (_vm->_groupMan->getDamageAllCreaturesOutcome(L0533_ps_Group, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, true) != k2_outcomeKilledAllCreaturesInGroup) && (L0530_i_Attack > 2)) {
				_vm->_groupMan->processEvents29to41(L0528_ui_MapX, L0529_ui_MapY, kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, 0);
			}
		}
		if (L0532_ps_Explosion->getAttack() >= 6) {
			L0532_ps_Explosion->setAttack(L0532_ps_Explosion->getAttack() - 3);
T0220026:
			L0539_s_Event = *event;
			L0539_s_Event._mapTime++;
			_vm->_timeline->f238_addEventGetEventIndex(&L0539_s_Event);
			return;
		}
	}
	_vm->_dungeonMan->unlinkThingFromList(Thing(event->_C._slot), Thing(0), L0528_ui_MapX, L0529_ui_MapY);
	L0532_ps_Explosion->setNextThing(Thing::_none);
}
}
