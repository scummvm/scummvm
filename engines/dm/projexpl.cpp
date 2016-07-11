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

namespace DM {

ProjExpl::ProjExpl(DMEngine* vm) : _vm(vm) {
	_g364_creatureDamageOutcome = 0;
	_g363_secondaryDirToOrFromParty = 0;
	_g361_lastCreatureAttackTime = -200;
	_g365_createLanucherProjectile = false;
	_g366_projectilePoisonAttack = 0;
	_g367_projectileAttackType = 0;
	_g362_lastPartyMovementTime = 0;
}

void ProjExpl::f212_projectileCreate(Thing thing, int16 mapX, int16 mapY, uint16 cell, direction dir, byte kineticEnergy, byte attack, byte stepEnergy) {
	Thing L0466_T_ProjectileThing;
	Projectile* L0467_ps_Projectile;
	TimelineEvent L0468_s_Event;


	if ((L0466_T_ProjectileThing = _vm->_dungeonMan->f166_getUnusedThing(k14_ProjectileThingType)) == Thing::_none) { /* BUG0_16 If the game cannot create a projectile thing because it has run out of such things (60 maximum) then the object being thrown/shot/launched is orphaned. If the game has run out of projectile things it will try to remove a projectile from elsewhere in the dungeon, except in an area of 11x11 squares centered around the party (to make sure the player cannot actually see the thing disappear on screen) */
		return;
	}
	L0466_T_ProjectileThing = M15_thingWithNewCell(L0466_T_ProjectileThing, cell);
	L0467_ps_Projectile = (Projectile *)_vm->_dungeonMan->f156_getThingData(L0466_T_ProjectileThing);
	L0467_ps_Projectile->_slot = thing;
	L0467_ps_Projectile->_kineticEnergy = MIN((int16)kineticEnergy, (int16)255);
	L0467_ps_Projectile->_attack = attack;
	_vm->_dungeonMan->f163_linkThingToList(L0466_T_ProjectileThing, Thing(0), mapX, mapY); /* Projectiles are added on the square and not 'moved' onto the square. In the case of a projectile launcher sensor, this means that the new projectile traverses the square in front of the launcher without any trouble: there is no impact if it is a wall, the projectile direction is not changed if it is a teleporter. Impacts with creatures and champions are still processed */
	M33_setMapAndTime(L0468_s_Event._mapTime, _vm->_dungeonMan->_g272_currMapIndex, _vm->_g313_gameTime + 1);
	if (_g365_createLanucherProjectile) {
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

bool ProjExpl::f217_projectileHasImpactOccurred(int16 impactType, int16 mapXCombo, int16 mapYCombo, int16 cell, Thing projectileThing) {
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

	L0490_ps_Projectile = (Projectile *)_vm->_dungeonMan->f156_getThingData(Thing(projectileThing));
	L0501_i_MapXCombo = mapXCombo;
	L0502_i_MapYCombo = mapYCombo;
	L0509_B_RemovePotion = false;
	_g364_creatureDamageOutcome = k0_outcomeKilledNoCreaturesInGroup;
	if ((L0510_i_ProjectileAssociatedThingType = (L0486_T_ProjectileAssociatedThing = L0490_ps_Projectile->_slot).getType()) == k8_PotionThingType) {
		L0491_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(L0486_T_ProjectileAssociatedThing);
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
		AL0487_i_DoorState = Square(L0503_uc_Square = _vm->_dungeonMan->_g271_currMapData[AP0454_i_ProjectileTargetMapX][AP0455_i_ProjectileTargetMapY]).getDoorState();
		L0494_ps_Door = (Door *)_vm->_dungeonMan->f157_getSquareFirstThingData(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY);
		if ((AL0487_i_DoorState != k5_doorState_DESTROYED) && (L0486_T_ProjectileAssociatedThing == Thing::_explOpenDoor)) {
			if (L0494_ps_Door->hasButton()) {
				_vm->_movsens->f268_addEvent(k10_TMEventTypeDoor, AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, 0, k2_SensorEffToggle, _vm->_g313_gameTime + 1);
			}
			break;
		}
		if ((AL0487_i_DoorState == k5_doorState_DESTROYED) ||
			(AL0487_i_DoorState <= k1_doorState_FOURTH) ||
			(getFlag(_vm->_dungeonMan->_g275_currMapDoorInfo[L0494_ps_Door->getType()]._attributes, k0x0002_MaskDoorInfo_ProjectilesCanPassThrough) &&
			((L0510_i_ProjectileAssociatedThingType == k15_ExplosionThingType) ?
			 (L0486_T_ProjectileAssociatedThing.toUint16() >= Thing::_explHarmNonMaterial.toUint16()) :
			 ((L0490_ps_Projectile->_attack > _vm->getRandomNumber(128)) &&
			  getFlag(g237_ObjectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(L0486_T_ProjectileAssociatedThing)].getAllowedSlots(), k0x0100_ObjectAllowedSlotPouchPassAndThroughDoors)
			  && ((L0510_i_ProjectileAssociatedThingType != k10_JunkThingType) ||
			  ((AL0487_i_IconIndex = _vm->_objectMan->f33_getIconIndex(L0486_T_ProjectileAssociatedThing)) < 0) ||
				  (!((AL0487_i_IconIndex >= k176_IconIndiceJunkIronKey) && (AL0487_i_IconIndex <= k191_IconIndiceJunkMasterKey))))
			  )))) { /* ASSEMBLY_COMPILATION_DIFFERENCE jmp */
			return false;
		}
		L0488_i_Attack = f216_projectileGetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing) + 1;
		_vm->_groupMan->f232_groupIsDoorDestoryedByAttack(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, L0488_i_Attack + _vm->getRandomNumber(L0488_i_Attack), false, 0);
		break;
	case kM2_ChampionElemType:
		if ((AP0456_i_ChampionIndex = _vm->_championMan->f285_getIndexInCell(cell)) < 0) {
			return false;
		}
		L0489_i_ChampionAttack = L0488_i_Attack = f216_projectileGetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing);
		break;
	case kM1_CreatureElemType:
		L0491_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(_vm->_groupMan->f175_groupGetThing(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY));
		if (!(L0512_ui_CreatureIndex = _vm->_groupMan->f176_getCreatureOrdinalInCell(L0491_ps_Group, cell))) {
			return false;
		}
		L0512_ui_CreatureIndex--;
		L0493_ps_CreatureInfo = &g243_CreatureInfo[L0511_ui_CreatureType = L0491_ps_Group->_type];
		if ((L0486_T_ProjectileAssociatedThing == Thing::_explFireBall) && (L0511_ui_CreatureType == k11_CreatureTypeBlackFlame)) {
			L0496_pui_CreatureHealth = &L0491_ps_Group->_health[L0512_ui_CreatureIndex];
			*L0496_pui_CreatureHealth = MIN(1000, *L0496_pui_CreatureHealth + f216_projectileGetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing));
			goto T0217044;
		}
		if (getFlag(L0493_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial) && (L0486_T_ProjectileAssociatedThing != Thing::_explHarmNonMaterial)) {
			return false;
		}
		if (L0488_i_Attack = (uint16)((unsigned long)f216_projectileGetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing) << 6) / L0493_ps_CreatureInfo->_defense) {
			if ((AL0487_i_Outcome = _vm->_groupMan->f190_groupGetDamageCreatureOutcome(L0491_ps_Group, L0512_ui_CreatureIndex, AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, L0488_i_Attack + _vm->_groupMan->f192_groupGetResistanceAdjustedPoisonAttack(L0511_ui_CreatureType, _g366_projectilePoisonAttack), true)) != k0_outcomeKilledNoCreaturesInGroup) {
				_vm->_groupMan->f209_processEvents29to41(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, kM2_TMEventTypeCreateReactionEvent30HitByProjectile, 0);
			}
			_g364_creatureDamageOutcome = AL0487_i_Outcome;
			if (!L0505_B_CreateExplosionOnImpact &&
				(AL0487_i_Outcome == k0_outcomeKilledNoCreaturesInGroup) &&
				(L0510_i_ProjectileAssociatedThingType == k5_WeaponThingType) &&
				getFlag(L0493_ps_CreatureInfo->_attributes, k0x0400_MaskCreatureInfo_keepThrownSharpWeapon)) {
				L0495_ps_Weapon = (Weapon *)_vm->_dungeonMan->f156_getThingData(L0486_T_ProjectileAssociatedThing);
				AL0487_i_WeaponType = L0495_ps_Weapon->getType();
				if ((AL0487_i_WeaponType == k8_WeaponTypeDagger) || (AL0487_i_WeaponType == k27_WeaponTypeArrow) || (AL0487_i_WeaponType == k28_WeaponTypeSlayer) || (AL0487_i_WeaponType == k31_WeaponTypePoisonDart) || (AL0487_i_WeaponType == k32_WeaponTypeThrowingStar)) {
					L0497_pT_GroupSlot = &L0491_ps_Group->_slot;
				}
			}
		}
	}
	if (L0489_i_ChampionAttack && _vm->_championMan->f321_addPendingDamageAndWounds_getDamage(AP0456_i_ChampionIndex, L0488_i_Attack, k0x0004_ChampionWoundHead | k0x0008_ChampionWoundTorso, _g367_projectileAttackType) && _g366_projectilePoisonAttack && _vm->getRandomNumber(2)) {
		_vm->_championMan->f322_championPoison(AP0456_i_ChampionIndex, _g366_projectilePoisonAttack);
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
		f213_explosionCreate(L0486_T_ProjectileAssociatedThing, AL0507_ui_ExplosionAttack, L0501_i_MapXCombo, L0502_i_MapYCombo, (L0486_T_ProjectileAssociatedThing == Thing::_explPoisonCloud) ? k255_CreatureTypeSingleCenteredCreature : cell);
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
		warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
	}
T0217044:
	if (L0509_B_RemovePotion) {
		L0492_ps_Potion->_nextThing = Thing::_none;
		L0490_ps_Projectile->_slot = L0498_T_ExplosionThing;
	}
	_vm->_dungeonMan->f164_unlinkThingFromList(projectileThing, Thing(0), L0499_i_ProjectileMapX, L0500_i_ProjectileMapY);
	f215_projectileDelete(projectileThing, L0497_pT_GroupSlot, L0499_i_ProjectileMapX, L0500_i_ProjectileMapY);
	return true;
}

uint16 ProjExpl::f216_projectileGetImpactAttack(Projectile* projectile, Thing thing) {
	WeaponInfo* L0485_ps_WeaponInfo;
	uint16 L0483_ui_Multiple;
#define AL0483_ui_ThingType L0483_ui_Multiple
#define AL0483_ui_Attack    L0483_ui_Multiple
	uint16 L0484_ui_KineticEnergy;


	_g366_projectilePoisonAttack = 0;
	_g367_projectileAttackType = k3_attackType_BLUNT;

	L0484_ui_KineticEnergy = projectile->_kineticEnergy;
	if ((AL0483_ui_ThingType = thing.getType()) != k15_ExplosionThingType) {
		if (AL0483_ui_ThingType == k5_WeaponThingType) {
			L0485_ps_WeaponInfo = _vm->_dungeonMan->f158_getWeaponInfo(thing);
			AL0483_ui_Attack = L0485_ps_WeaponInfo->_kineticEnergy;
			_g367_projectileAttackType = k3_attackType_BLUNT;
		} else {
			AL0483_ui_Attack = _vm->getRandomNumber(4);
		}
		AL0483_ui_Attack += _vm->_dungeonMan->f140_getObjectWeight(thing) >> 1;
	} else {
		if (thing == Thing::_explSlime) {
			AL0483_ui_Attack = _vm->getRandomNumber(16);
			_g366_projectilePoisonAttack = AL0483_ui_Attack + 10;
			AL0483_ui_Attack += _vm->getRandomNumber(32);
		} else {
			if (thing.toUint16() >= Thing::_explHarmNonMaterial.toUint16()) {
				_g367_projectileAttackType = k5_attackType_MAGIC;
				if (thing == Thing::_explPoisonBolt) {
					_g366_projectilePoisonAttack = L0484_ui_KineticEnergy;
					return 1;
				}
				return 0;
			}
			_g367_projectileAttackType = k1_attackType_FIRE;
			AL0483_ui_Attack = _vm->getRandomNumber(16) + _vm->getRandomNumber(16) + 10;
			if (thing == Thing::_explLightningBolt) {
				_g367_projectileAttackType = k7_attackType_LIGHTNING;
				AL0483_ui_Attack *= 5;
			}
		}
	}
	AL0483_ui_Attack = ((AL0483_ui_Attack + L0484_ui_KineticEnergy) >> 4) + 1;
	AL0483_ui_Attack += _vm->getRandomNumber((AL0483_ui_Attack >> 1) + 1) + _vm->getRandomNumber(4);
	AL0483_ui_Attack = MAX(AL0483_ui_Attack >> 1, AL0483_ui_Attack - (32 - (projectile->_attack >> 3)));
	return AL0483_ui_Attack;
}

void ProjExpl::f213_explosionCreate(Thing explThing, uint16 attack, uint16 mapXCombo, uint16 mapYCombo, uint16 cell) {
#define AP0443_ui_ProjectileMapX mapXCombo
#define AP0444_ui_ProjectileMapY mapYCombo
	Thing L0473_T_Thing = _vm->_dungeonMan->f166_getUnusedThing(k15_ExplosionThingType);
	if (L0473_T_Thing == Thing::_none) {
		return;
	}

	Explosion *L0470_ps_Explosion = &((Explosion *)_vm->_dungeonMan->_g284_thingData[k15_ExplosionThingType])[(L0473_T_Thing).getIndex()];
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
		L0473_T_Thing = M15_thingWithNewCell(L0473_T_Thing, cell);
	}
	L0470_ps_Explosion->setType(explThing.toUint16() - Thing::_firstExplosion.toUint16());
	L0470_ps_Explosion->setAttack(attack);
	if (explThing.toUint16() < Thing::_explHarmNonMaterial.toUint16()) {
		warning(false, "MISING CODE: F0064_SOUND_RequestPlay_CPSD");
	} else {
		if (explThing != Thing::_explSmoke) {
			warning(false, "MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
		}
	}
	_vm->_dungeonMan->f163_linkThingToList(L0473_T_Thing, Thing(0), AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY);
	TimelineEvent L0476_s_Event;
	M33_setMapAndTime(L0476_s_Event._mapTime, _vm->_dungeonMan->_g272_currMapIndex, _vm->_g313_gameTime + ((explThing == Thing::_explRebirthStep1) ? 5 : 1));
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
			if ((_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (AP0443_ui_ProjectileMapX == _vm->_dungeonMan->_g306_partyMapX) && (AP0444_ui_ProjectileMapY == _vm->_dungeonMan->_g307_partyMapY)) {
				_vm->_championMan->f324_damageAll_getDamagedChampionCount(attack, k0x0001_ChampionWoundReadHand | k0x0002_ChampionWoundActionHand | k0x0004_ChampionWoundHead | k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs | k0x0020_ChampionWoundFeet, k1_attackType_FIRE);
			} else {
				if ((L0473_T_Thing = _vm->_groupMan->f175_groupGetThing(AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY)) != Thing::_endOfList) { /* ASSEMBLY_COMPILATION_DIFFERENCE jmp */
					Group *L0472_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(L0473_T_Thing);
					CreatureInfo *L0471_ps_CreatureInfo = &g243_CreatureInfo[L0472_ps_Group->_type];
					int16 L0469_i_CreatureFireResistance = L0471_ps_CreatureInfo->M60_getFireResistance();
					if (L0469_i_CreatureFireResistance != k15_immuneToFire) {
						if (getFlag(L0471_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
							attack >>= 2;
						}
						if ((attack -= _vm->getRandomNumber((L0469_i_CreatureFireResistance << 1) + 1)) > 0) {
							_g364_creatureDamageOutcome = _vm->_groupMan->f191_getDamageAllCreaturesOutcome(L0472_ps_Group, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, attack, true);
						}
					}
				}
			}
		}
	}
}

int16 ProjExpl::f218_projectileGetImpactCount(int16 impactType, int16 mapX, int16 mapY, int16 cell) {
	Thing L0513_T_Thing;
	int16 L0514_i_ImpactCount;


	L0514_i_ImpactCount = 0;
	_g364_creatureDamageOutcome = k0_outcomeKilledNoCreaturesInGroup;
T0218001:
	L0513_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(mapX, mapY);
	while (L0513_T_Thing != Thing::_endOfList) {
		if (((L0513_T_Thing).getType() == k14_ProjectileThingType) &&
			((L0513_T_Thing).getCell() == cell) &&
			f217_projectileHasImpactOccurred(impactType, mapX, mapY, cell, L0513_T_Thing)) {
			f214_projectileDeleteEvent(L0513_T_Thing);
			L0514_i_ImpactCount++;
			if ((impactType == kM1_CreatureElemType) && (_g364_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup))
				break;
			goto T0218001;
		}
		L0513_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0513_T_Thing);
	}
	return L0514_i_ImpactCount;
}

void ProjExpl::f214_projectileDeleteEvent(Thing thing) {
	Projectile* L0477_ps_Projectile;


	L0477_ps_Projectile = (Projectile *)_vm->_dungeonMan->f156_getThingData(thing);
	_vm->_timeline->f237_deleteEvent(L0477_ps_Projectile->_eventIndex);
}

void ProjExpl::f215_projectileDelete(Thing projectileThing, Thing* groupSlot, int16 mapX, int16 mapY) {
	Projectile *L0480_ps_Projectile = (Projectile *)_vm->_dungeonMan->f156_getThingData(projectileThing);
	Thing L0479_T_Thing = L0480_ps_Projectile->_slot;
	if (L0479_T_Thing.getType() != k15_ExplosionThingType) {
		if (groupSlot != NULL) {
			Thing L0478_T_PreviousThing = *groupSlot;
			if (L0478_T_PreviousThing == Thing::_endOfList) {
				Thing *L0481_ps_Generic = (Thing *)_vm->_dungeonMan->f156_getThingData(L0479_T_Thing);
				*L0481_ps_Generic = Thing::_endOfList;
				*groupSlot = L0479_T_Thing;
			} else {
				_vm->_dungeonMan->f163_linkThingToList(L0479_T_Thing, L0478_T_PreviousThing, kM1_MapXNotOnASquare, 0);
			}
		} else {
			_vm->_movsens->f267_getMoveResult(Thing((L0479_T_Thing).getTypeAndIndex() | getFlag(projectileThing.toUint16(), 0xC)), -2, 0, mapX, mapY);
		}
	}
	L0480_ps_Projectile->_nextThing = Thing::_none;
}

void ProjExpl::f219_processEvents48To49_projectile(TimelineEvent* event) {
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
	int16 L0525_i_SourceMapX;
	int16 L0526_i_SourceMapY;
	TimelineEvent L0527_s_Event;


	L0527_s_Event = *event;
	L0519_ps_Event = &L0527_s_Event;
	L0520_ps_Projectile = (Projectile*)_vm->_dungeonMan->f156_getThingData(L0521_T_ProjectileThing = L0515_T_ProjectileThingNewCell = Thing(L0519_ps_Event->_B._slot));
	L0523_i_DestinationMapX = L0519_ps_Event->_C._projectile.getMapX();
	L0524_i_DestinationMapY = L0519_ps_Event->_C._projectile.getMapY();
	if (L0519_ps_Event->_type == k48_TMEventTypeMoveProjectileIgnoreImpacts) {
		L0519_ps_Event->_type = k49_TMEventTypeMoveProjectile;
	} else {
		L0518_ui_Cell = (L0515_T_ProjectileThingNewCell).getCell();
		if ((_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (L0523_i_DestinationMapX == _vm->_dungeonMan->_g306_partyMapX) && (L0524_i_DestinationMapY == _vm->_dungeonMan->_g307_partyMapY) && f217_projectileHasImpactOccurred(kM2_ChampionElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0515_T_ProjectileThingNewCell)) {
			return;
		}
		if ((_vm->_groupMan->f175_groupGetThing(L0523_i_DestinationMapX, L0524_i_DestinationMapY) != Thing::_endOfList) && f217_projectileHasImpactOccurred(kM1_CreatureElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0521_T_ProjectileThing)) {
			return;
		}
		if (L0520_ps_Projectile->_kineticEnergy <= (AL0516_ui_StepEnergy = L0519_ps_Event->_C._projectile.getStepEnergy())) {
			_vm->_dungeonMan->f164_unlinkThingFromList(L0515_T_ProjectileThingNewCell = L0521_T_ProjectileThing, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
			f215_projectileDelete(L0515_T_ProjectileThingNewCell, NULL, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
			return;
		}
		L0520_ps_Projectile->_kineticEnergy -= AL0516_ui_StepEnergy;
		if (L0520_ps_Projectile->_attack < AL0516_ui_StepEnergy) {
			L0520_ps_Projectile->_attack = 0;
		} else {
			L0520_ps_Projectile->_attack -= AL0516_ui_StepEnergy;
		}
	}
	if (L0522_B_ProjectileMovesToOtherSquare = ((L0517_ui_ProjectileDirection = L0519_ps_Event->_C._projectile.getDir()) == (L0518_ui_Cell = (L0515_T_ProjectileThingNewCell = Thing(L0519_ps_Event->_B._slot)).getCell())) || (returnNextVal(L0517_ui_ProjectileDirection) == L0518_ui_Cell)) {
		L0525_i_SourceMapX = L0523_i_DestinationMapX;
		L0526_i_SourceMapY = L0524_i_DestinationMapY;
		L0523_i_DestinationMapX += _vm->_dirIntoStepCountEast[L0517_ui_ProjectileDirection], L0524_i_DestinationMapY += _vm->_dirIntoStepCountNorth[L0517_ui_ProjectileDirection];
		if ((Square(AL0516_ui_Square = _vm->_dungeonMan->f151_getSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY).toByte()).getType() == k0_ElementTypeWall) ||
			((Square(AL0516_ui_Square).getType() == k6_ElementTypeFakeWall) && !getFlag(AL0516_ui_Square, (k0x0001_FakeWallImaginary | k0x0004_FakeWallOpen))) ||
			((Square(AL0516_ui_Square).getType() == k3_ElementTypeStairs) && (Square(_vm->_dungeonMan->_g271_currMapData[L0525_i_SourceMapX][L0526_i_SourceMapY]).getType() == k3_ElementTypeStairs))) {
			if (f217_projectileHasImpactOccurred(Square(AL0516_ui_Square).getType(), L0525_i_SourceMapX, L0526_i_SourceMapY, L0518_ui_Cell, L0515_T_ProjectileThingNewCell)) {
				return;
			}
		}
	}
	if ((L0517_ui_ProjectileDirection & 0x0001) == (L0518_ui_Cell & 0x0001)) {
		L0518_ui_Cell--;
	} else {
		L0518_ui_Cell++;
	}
	L0515_T_ProjectileThingNewCell = M15_thingWithNewCell(L0515_T_ProjectileThingNewCell, L0518_ui_Cell &= 0x0003);
	if (L0522_B_ProjectileMovesToOtherSquare) {
		_vm->_movsens->f267_getMoveResult(L0515_T_ProjectileThingNewCell, L0525_i_SourceMapX, L0526_i_SourceMapY, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
		L0519_ps_Event->_C._projectile.setMapX(_vm->_movsens->_g397_moveResultMapX);
		L0519_ps_Event->_C._projectile.setMapY(_vm->_movsens->_g398_moveResultMapY);
		L0519_ps_Event->_C._projectile.setDir((direction)_vm->_movsens->_g400_moveResultDir);
		L0515_T_ProjectileThingNewCell = M15_thingWithNewCell(L0515_T_ProjectileThingNewCell, _vm->_movsens->_g401_moveResultCell);
		M31_setMap(L0519_ps_Event->_mapTime, _vm->_movsens->_g399_moveResultMapIndex);
	} else {
		if ((Square(_vm->_dungeonMan->f151_getSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY)).getType() == k4_DoorElemType) && f217_projectileHasImpactOccurred(k4_DoorElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0521_T_ProjectileThing)) {
			return;
		}
		_vm->_dungeonMan->f164_unlinkThingFromList(L0515_T_ProjectileThingNewCell, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
		_vm->_dungeonMan->f163_linkThingToList(L0515_T_ProjectileThingNewCell, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
	}
	L0519_ps_Event->_mapTime += (_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) ? 1 : 3;
	//Strangerke: CHECKME: Maybe we should keep that piece of code too as it sounds like it's fixing a weird behavior of projectiles on different maps
#ifdef COMPILE42_CSB20EN_CSB21EN /* CHANGE7_20_IMPROVEMENT Projectiles now move at the same speed on all maps instead of moving slower on maps other than the party map */
	L0519_ps_Event->Map_Time++;
#endif
	L0519_ps_Event->_B._slot = L0515_T_ProjectileThingNewCell.toUint16();
	L0520_ps_Projectile->_eventIndex = _vm->_timeline->f238_addEventGetEventIndex(L0519_ps_Event);
}
}
