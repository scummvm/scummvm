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
#ifndef DM_PROJEXPL_H
#define DM_PROJEXPL_H

#include "dm/dm.h"

namespace DM {

enum KillOutcome {
	kDMKillOutcomeNoCreaturesInGroup = 0,   // @ C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP   
	kDMKillOutcomeSomeCreaturesInGroup = 1, // @ C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP 
	kDMKillOutcomeAllCreaturesInGroup = 2   // @ C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP  
};

enum SoundIndex {
	kDMSoundIndexMetallicThud = 0, // @ C00_SOUND_METALLIC_THUD
	kDMSoundIndexSwitch = 1, // @ C01_SOUND_SWITCH
	kDMSoundIndexDoorRattle = 2, // @ C02_SOUND_DOOR_RATTLE
	kDMSoundIndexAttackPainRatHellHoundRedDragon = 3, // @ C03_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON
	kDMSoundIndexWoodenThudAttackTrolinAntmanStoneGolem = 4, // @ C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM
	kDMSoundIndexStrongExplosion = 5, // @ C05_SOUND_STRONG_EXPLOSION
	kDMSoundIndexScream = 6, // @ C06_SOUND_SCREAM
	kDMSoundIndexAttackMummyGhostRive = 7, // @ C07_SOUND_ATTACK_MUMMY_GHOST_RIVE
	kDMSoundIndexSwallow = 8, // @ C08_SOUND_SWALLOW
	kDMSoundIndexChampion0Damaged = 9, // @ C09_SOUND_CHAMPION_0_DAMAGED
	kDMSoundIndexChampion1Damaged = 10, // @ C10_SOUND_CHAMPION_1_DAMAGED
	kDMSoundIndexChampion2Damaged = 11, // @ C11_SOUND_CHAMPION_2_DAMAGED
	kDMSoundIndexChampion3Damaged = 12, // @ C12_SOUND_CHAMPION_3_DAMAGED
	kDMSoundIndexSpell = 13, // @ C13_SOUND_SPELL
	kDMSoundIndexAttackScreamerOitu = 14, // @ C14_SOUND_ATTACK_SCREAMER_OITU
	kDMSoundIndexAttackGiantScorpion = 15, // @ C15_SOUND_ATTACK_GIANT_SCORPION_SCORPION
	kDMSoundIndexAttackSkelettonAnimatedArmorDethKnight = 16, // @ C16_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT
	kDMSoundIndexBuzz = 17, // @ C17_SOUND_BUZZ
	kDMSoundIndexPartyDamaged = 18, // @ C18_SOUND_PARTY_DAMAGED
	kDMSoundIndexAttackMagentaWorm = 19, // @ C19_SOUND_ATTACK_MAGENTA_WORM_WORM
	kDMSoundIndexWeakExplosion = 20, // @ C20_SOUND_WEAK_EXPLOSION
	kDMSoundIndexAttackGiggler = 21, // @ C21_SOUND_ATTACK_GIGGLER
	kDMSoundIndexMoveAnimatedArmorDethKnight = 22, // @ C22_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT
	kDMSoundIndexMoveCouatlGiantWaspMuncher = 23, // @ C23_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER
	kDMSoundIndexMoveMummyTrolinAntmanStoneGolemGiggleVexirkDemon = 24, // @ C24_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON
	kDMSoundIndexBlowHorn = 25, // @ C25_SOUND_BLOW_HORN
	kDMSoundIndexMoveScreamerRocksWormPainRatHellHoundRusterScorpionsOitu = 26, // @ C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU
	kDMSoundIndexMoveSlimesDevilWaterElemental = 27, // @ C27_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL
	kDMSoundIndexWarCry = 28, // @ C28_SOUND_WAR_CRY
	kDMSoundIndexAttackRocks = 29, // @ C29_SOUND_ATTACK_ROCK_ROCKPILE
	kDMSoundIndexAttackWaterElemental = 30, // @ C30_SOUND_ATTACK_WATER_ELEMENTAL
	kDMSoundIndexAttackCouatl = 31, // @ C31_SOUND_ATTACK_COUATL
	kDMSoundIndexMoveRedDragon = 32, // @ C32_SOUND_MOVE_RED_DRAGON
	kDMSoundIndexMoveSkeletton = 33 // @ C33_SOUND_MOVE_SKELETON
};

#define M31_setMap(map_time, map) ((map_time) = (((map_time) & 0x00FFFFFF) | (((int32)(map)) << 24)))

class TimelineEvent;
class Projectile;

class ProjExpl {
	DMEngine *_vm;
public:
	int16 _creatureDamageOutcome; // @ G0364_i_CreatureDamageOutcome
	int16 _secondaryDirToOrFromParty; // @ G0363_i_SecondaryDirectionToOrFromParty
	int32 _lastCreatureAttackTime; // @ G0361_l_LastCreatureAttackTime
	bool _createLauncherProjectile; // @ G0365_B_CreateLauncherProjectile
	int16 _projectilePoisonAttack; // @ G0366_i_ProjectilePoisonAttack
	int16 _projectileAttackType; // @ G0367_i_ProjectileAttackType
	int32 _lastPartyMovementTime; // @ G0362_l_LastPartyMovementTime

	explicit ProjExpl(DMEngine *vm);

	void createProjectile(Thing thing, int16 mapX, int16 mapY, uint16 cell, Direction dir,
							   byte kineticEnergy, byte attack, byte stepEnergy); // @ F0212_PROJECTILE_Create
	bool hasProjectileImpactOccurred(int16 impactType, int16 mapXCombo, int16 mapYCombo,
										  int16 cell, Thing projectileThing); // @ F0217_PROJECTILE_HasImpactOccured
	uint16 getProjectileImpactAttack(Projectile *projectile, Thing thing); // @ F0216_PROJECTILE_GetImpactAttack
	void createExplosion(Thing explThing, uint16 attack, uint16 mapXCombo,
							  uint16 mapYCombo, uint16 cell); // @ F0213_EXPLOSION_Create
	int16 projectileGetImpactCount(int16 impactType, int16 mapX, int16 mapY, int16 cell); // @ F0218_PROJECTILE_GetImpactCount
	void projectileDeleteEvent(Thing thing); // @ F0214_PROJECTILE_DeleteEvent
	void projectileDelete(Thing projectileThing, Thing *groupSlot, int16 mapX, int16 mapY); // @ F0215_PROJECTILE_Delete
	void processEvents48To49(TimelineEvent *event); // @ F0219_PROJECTILE_ProcessEvents48To49_Projectile
	void processEvent25(TimelineEvent *event); // @ F0220_EXPLOSION_ProcessEvent25_Explosion
};
}

#endif
