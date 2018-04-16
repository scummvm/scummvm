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
