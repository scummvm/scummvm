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

#include "dm.h"


namespace DM {

#define k0_outcomeKilledNoCreaturesInGroup 0 // @ C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP   
#define k1_outcomeKilledSomeCreaturesInGroup 1 // @ C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP 
#define k2_outcomeKilledAllCreaturesInGroup 2 // @ C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP  

#define k00_soundMETALLIC_THUD 0 // @ C00_SOUND_METALLIC_THUD
#define k01_soundSWITCH 1 // @ C01_SOUND_SWITCH
#define k02_soundDOOR_RATTLE 2 // @ C02_SOUND_DOOR_RATTLE
#define k03_soundATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON 3 // @ C03_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON
#define k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM 4 // @ C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM
#define k05_soundSTRONG_EXPLOSION 5 // @ C05_SOUND_STRONG_EXPLOSION
#define k06_soundSCREAM 6 // @ C06_SOUND_SCREAM
#define k07_soundATTACK_MUMMY_GHOST_RIVE 7 // @ C07_SOUND_ATTACK_MUMMY_GHOST_RIVE
#define k08_soundSWALLOW 8 // @ C08_SOUND_SWALLOW
#define k09_soundCHAMPION_0_DAMAGED 9 // @ C09_SOUND_CHAMPION_0_DAMAGED
#define k10_soundCHAMPION_1_DAMAGED 10 // @ C10_SOUND_CHAMPION_1_DAMAGED
#define k11_soundCHAMPION_2_DAMAGED 11 // @ C11_SOUND_CHAMPION_2_DAMAGED
#define k12_soundCHAMPION_3_DAMAGED 12 // @ C12_SOUND_CHAMPION_3_DAMAGED
#define k13_soundSPELL 13 // @ C13_SOUND_SPELL
#define k14_soundATTACK_SCREAMER_OITU 14 // @ C14_SOUND_ATTACK_SCREAMER_OITU
#define k15_soundATTACK_GIANT_SCORPION_SCORPION 15 // @ C15_SOUND_ATTACK_GIANT_SCORPION_SCORPION
#define k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT 16 // @ C16_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT
#define k17_soundBUZZ 17 // @ C17_SOUND_BUZZ
#define k18_soundPARTY_DAMAGED 18 // @ C18_SOUND_PARTY_DAMAGED
#define k19_soundATTACK_MAGENTA_WORM_WORM 19 // @ C19_SOUND_ATTACK_MAGENTA_WORM_WORM
#define k20_soundWEAK_EXPLOSION 20 // @ C20_SOUND_WEAK_EXPLOSION
#define k21_soundATTACK_GIGGLER 21 // @ C21_SOUND_ATTACK_GIGGLER
#define k22_soundMOVE_ANIMATED_ARMOUR_DETH_KNIGHT 22 // @ C22_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT
#define k23_soundMOVE_COUATL_GIANT_WASP_MUNCHER 23 // @ C23_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER
#define k24_soundMOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON 24 // @ C24_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON
#define k25_soundBLOW_HORN 25 // @ C25_SOUND_BLOW_HORN
#define k26_soundMOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU 26 // @ C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU
#define k27_soundMOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL 27 // @ C27_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL
#define k28_soundWAR_CRY 28 // @ C28_SOUND_WAR_CRY
#define k29_soundATTACK_ROCK_ROCKPILE 29 // @ C29_SOUND_ATTACK_ROCK_ROCKPILE
#define k30_soundATTACK_WATER_ELEMENTAL 30 // @ C30_SOUND_ATTACK_WATER_ELEMENTAL
#define k31_soundATTACK_COUATL 31 // @ C31_SOUND_ATTACK_COUATL
#define k32_soundMOVE_RED_DRAGON 32 // @ C32_SOUND_MOVE_RED_DRAGON
#define k33_soundMOVE_SKELETON 33 // @ C33_SOUND_MOVE_SKELETON


#define M31_setMap(map_time, map) ((map_time) = (((map_time) & 0x00FFFFFF) | (((int32)(map)) << 24)))

class TimelineEvent;
class Projectile;

class ProjExpl {
	DMEngine *_vm;
public:
	int16 _g364_creatureDamageOutcome; // @ G0364_i_CreatureDamageOutcome
	int16 _g363_secondaryDirToOrFromParty; // @ G0363_i_SecondaryDirectionToOrFromParty
	int32 _g361_lastCreatureAttackTime; // @ G0361_l_LastCreatureAttackTime
	bool _g365_createLanucherProjectile; // @ G0365_B_CreateLauncherProjectile
	int16 _g366_projectilePoisonAttack; // @ G0366_i_ProjectilePoisonAttack
	int16 _g367_projectileAttackType; // @ G0367_i_ProjectileAttackType
	int32 _g362_lastPartyMovementTime; // @ G0362_l_LastPartyMovementTime
	explicit ProjExpl(DMEngine *vm);
	void f212_projectileCreate(Thing thing, int16 mapX, int16 mapY, uint16 cell, direction dir,
							   byte kineticEnergy, byte attack, byte stepEnergy); // @ F0212_PROJECTILE_Create
	bool f217_projectileHasImpactOccurred(int16 impactType, int16 mapXCombo, int16 mapYCombo,
										  int16 cell, Thing projectileThing); // @ F0217_PROJECTILE_HasImpactOccured
	uint16 f216_projectileGetImpactAttack(Projectile *projectile, Thing thing); // @ F0216_PROJECTILE_GetImpactAttack
	void f213_explosionCreate(Thing explThing, uint16 attack, uint16 mapXCombo,
							  uint16 mapYCombo, uint16 cell); // @ F0213_EXPLOSION_Create
	int16 f218_projectileGetImpactCount(int16 impactType, int16 mapX, int16 mapY, int16 cell); // @ F0218_PROJECTILE_GetImpactCount
	void f214_projectileDeleteEvent(Thing thing); // @ F0214_PROJECTILE_DeleteEvent
	void f215_projectileDelete(Thing projectileThing, Thing *groupSlot, int16 mapX, int16 mapY); // @ F0215_PROJECTILE_Delete
	void f219_processEvents48To49_projectile(TimelineEvent *event); // @ F0219_PROJECTILE_ProcessEvents48To49_Projectile
	void f220_explosionProcessEvent25_explosion(TimelineEvent *event); // @ F0220_EXPLOSION_ProcessEvent25_Explosion
};

}

#endif
