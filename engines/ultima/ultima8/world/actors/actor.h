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

#ifndef WORLD_ACTORS_ACTOR_H
#define WORLD_ACTORS_ACTOR_H

#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/world/actors/animation.h"

namespace Ultima8 {

class ActorAnimProcess;
struct PathfindingState;
class CombatProcess;

class Actor : public Container {
	friend class ActorAnimProcess;
	friend class AnimationTracker;
public:
	Actor();
	~Actor();

	int16 getStr() const {
		return strength;
	}
	void setStr(int16 str) {
		strength = str;
	}
	int16 getDex() const {
		return dexterity;
	}
	void setDex(int16 dex) {
		dexterity = dex;
	}
	int16 getInt() const {
		return intelligence;
	}
	void setInt(int16 int_) {
		intelligence = int_;
	}
	uint16 getHP() const {
		return hitpoints;
	}
	void setHP(uint16 hp) {
		hitpoints = hp;
	}
	int16 getMana() const {
		return mana;
	}
	void setMana(int16 mp) {
		mana = mp;
	}

	int16 getMaxMana() const;
	uint16 getMaxHP() const;

	bool isDead() const {
		return (actorflags & ACT_DEAD) != 0;
	}

	bool isInCombat() const {
		return (actorflags & ACT_INCOMBAT) != 0;
	}
	void toggleInCombat() {
		if (isInCombat()) clearInCombat();
		else setInCombat();
	}

	CombatProcess *getCombatProcess();
	virtual void setInCombat();
	virtual void clearInCombat();

	uint16 getAlignment() const {
		return alignment;
	}
	void setAlignment(uint16 a) {
		alignment = a;
	}
	uint16 getEnemyAlignment() const {
		return enemyalignment;
	}
	void setEnemyAlignment(uint16 a) {
		enemyalignment = a;
	}

	Animation::Sequence getLastAnim() const {
		return lastanim;
	}
	void setLastAnim(Animation::Sequence anim) {
		lastanim = anim;
	}
	uint16 getDir() const {
		return direction;
	}
	void setDir(uint16 dir) {
		direction = dir;
	}
	int32 getFallStart() const {
		return fallstart;
	}
	void setFallStart(int32 zp) {
		fallstart = zp;
	}
	void setUnk0C(uint8 b) {
		unk0C = b;
	}

	uint32 getActorFlags() const {
		return actorflags;
	}
	void setActorFlag(uint32 mask) {
		actorflags |= mask;
	}
	void clearActorFlag(uint32 mask) {
		actorflags &= ~mask;
	}

	//! set stats from MonsterInfo (hp, dex, alignment, enemyalignment)
	//! \return true if a MonsterInfo struct was found, false otherwise
	bool loadMonsterStats();

	//! add treasure according to the TreasureInfo in the MonsterInfo
	//! \return true if a MonsterInfo struct was found, false otherwise
	bool giveTreasure();

	virtual void teleport(int mapnum, int32 x, int32 y, int32 z);

	virtual bool removeItem(Item *item);

	//! \return the PID of the spawned usecode process if any (otherwise 0)
	uint16 schedule(uint32 time);

	bool setEquip(Item *item, bool checkwghtvol = false);
	uint16 getEquip(uint32 type);

	virtual uint32 getArmourClass();
	virtual uint16 getDefenseType();
	virtual int16 getAttackingDex();
	virtual int16 getDefendingDex();

	virtual uint16 getDamageType();
	virtual int getDamageAmount();

	//! calculate the damage an attack against this Actor does.
	//! \param other the attacker (can be zero)
	//! \param damage base damage
	//! \param type damage type
	//! \return the amount of damage to be applied. Zero if attack missed.
	int calculateAttackDamage(uint16 other, int damage, uint16 type);

	//! receive a hit
	//! \param damage base damage (or zero to use attacker's default damage)
	//! \param type damage type (or zero to use attacker's default type)
	virtual void receiveHit(uint16 other, int dir, int damage, uint16 type);

	//! die
	//! \param damageType damage type that caused the death
	//! \return the process ID of the death animation
	virtual ProcId die(uint16 damageType);

	//! kill all processes except those related to combat
	void killAllButCombatProcesses();

	//! kill all animation processes except those related to dying/falling
	//! \return PID of animprocess doing the falling (or getting up)
	ProcId killAllButFallAnims(bool death);

	//! check if NPCs are near which are in combat mode and hostile
	bool areEnemiesNear();

	//! starts an activity
	//! \return processID of process handling the activity or zero
	uint16 cSetActivity(int activity);

	//! run the given animation
	//! \return the PID of the ActorAnimProcess
	uint16 doAnim(Animation::Sequence anim, int dir, unsigned int steps = 0);

	//! check if this actor has a specific animation
	bool hasAnim(Animation::Sequence anim);

	//! check if the given animation can be done from the location in state,
	//! without walking into things. If state is non-zero, and successful,
	//! state will be updated to after the animation. If unsuccessful,
	//! the contents of state are undefined.
	//! \param anim Action to try
	//! \param dir direction to walk in
	//! \param state the state to start from, or 0 to use the current state
	Animation::Result tryAnim(Animation::Sequence anim, int dir, unsigned int steps = 0, PathfindingState *state = 0);

	//! create an actor, assign objid, make it ethereal and load monster stats.
	static Actor *createActor(uint32 shape, uint32 frame);

	virtual uint16 assignObjId(); // assign an NPC objid

	virtual void dumpInfo();

	bool loadData(IDataSource *ids, uint32 version);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	INTRINSIC(I_isNPC);
	INTRINSIC(I_getDir);
	INTRINSIC(I_getLastAnimSet);
	INTRINSIC(I_pathfindToItem);
	INTRINSIC(I_pathfindToPoint);
	INTRINSIC(I_getStr);
	INTRINSIC(I_getDex);
	INTRINSIC(I_getInt);
	INTRINSIC(I_getHp);
	INTRINSIC(I_getMana);
	INTRINSIC(I_getAlignment);
	INTRINSIC(I_getEnemyAlignment);
	INTRINSIC(I_setStr);
	INTRINSIC(I_setDex);
	INTRINSIC(I_setInt);
	INTRINSIC(I_setHp);
	INTRINSIC(I_setMana);
	INTRINSIC(I_setAlignment);
	INTRINSIC(I_setEnemyAlignment);
	INTRINSIC(I_getMap);
	INTRINSIC(I_teleport);
	INTRINSIC(I_doAnim);
	INTRINSIC(I_isInCombat);
	INTRINSIC(I_setInCombat);
	INTRINSIC(I_clrInCombat);
	INTRINSIC(I_setTarget);
	INTRINSIC(I_getTarget);
	INTRINSIC(I_isEnemy);
	INTRINSIC(I_isDead);
	INTRINSIC(I_setDead);
	INTRINSIC(I_clrDead);
	INTRINSIC(I_isImmortal);
	INTRINSIC(I_setImmortal);
	INTRINSIC(I_clrImmortal);
	INTRINSIC(I_isWithstandDeath);
	INTRINSIC(I_setWithstandDeath);
	INTRINSIC(I_clrWithstandDeath);
	INTRINSIC(I_isFeignDeath);
	INTRINSIC(I_setFeignDeath);
	INTRINSIC(I_clrFeignDeath);
	INTRINSIC(I_areEnemiesNear);
	INTRINSIC(I_isBusy);
	INTRINSIC(I_createActor);
	INTRINSIC(I_cSetActivity);
	INTRINSIC(I_setAirWalkEnabled);
	INTRINSIC(I_getAirWalkEnabled);
	INTRINSIC(I_schedule);
	INTRINSIC(I_getEquip);
	INTRINSIC(I_setEquip);

	enum ActorFlags {
		ACT_INVINCIBLE     = 0x000001, // flags from npcdata byte 0x1B
		ACT_ASCENDING      = 0x000002,
		ACT_DESCENDING     = 0x000004,
		ACT_ANIMLOCK       = 0x000008,

		ACT_FIRSTSTEP      = 0x000400, // flags from npcdata byte 0x2F
		ACT_INCOMBAT       = 0x000800,
		ACT_DEAD           = 0x001000,
		ACT_COMBATRUN      = 0x008000,

		ACT_AIRWALK        = 0x010000, // flags from npcdata byte 0x30
		ACT_IMMORTAL       = 0x040000,
		ACT_WITHSTANDDEATH = 0x080000,
		ACT_FEIGNDEATH     = 0x100000,
		ACT_STUNNED        = 0x200000,
		ACT_POISONED       = 0x400000,
		ACT_PATHFINDING    = 0x800000
	};

protected:
	virtual void saveData(ODataSource *ods);

	int16 strength;
	int16 dexterity;
	int16 intelligence;
	uint16 hitpoints;
	int16 mana;

	uint16 alignment, enemyalignment;

	Animation::Sequence lastanim;
	uint16 animframe;
	uint16 direction;

	int32 fallstart;
	uint8 unk0C; // unknown byte 0x0C from npcdata.dat

	uint32 actorflags;
};

} // End of namespace Ultima8

#endif
