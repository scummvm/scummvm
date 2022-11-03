/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/detection.h"
#include "saga2/intrface.h"
#include "saga2/contain.h"
#include "saga2/task.h"
#include "saga2/motion.h"
#include "saga2/transit.h"
#include "saga2/localize.h"

namespace Saga2 {

extern bool             allPlayerActorsDead;

extern ObjectID         viewCenterObject;   //  ID of object which the
//  camera tracks

extern ReadyContainerView   *TrioCviews[kNumViews];
extern ReadyContainerView   *indivCviewTop, *indivCviewBot;
extern ContainerNode        *indivReadyNode;

void updateMainDisplay();

TilePoint selectNearbySite(
    ObjectID        worldID,
    const TilePoint &startingCoords,
    int32           minDist,
    int32           maxDist,
    bool            offScreenOnly);

bool checkPath(
    ObjectID            worldID,
    uint8               height,
    const TilePoint     &startingPt,
    const TilePoint     &destPt);

/* ===================================================================== *
   Globals
 * ===================================================================== */

static PlayerActorID    centerActor;        //  Index of the current center
//  actor

bool                    brotherBandingEnabled;

/* ===================================================================== *
   Methods
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Resolve the banding state of this actor

void PlayerActor::resolveBanding() {
	Actor *follower         = getActor();
	Actor *centerActor_     = getCenterActor();

	// if already following, tell the actor to cease and desist
	if (follower->_leader) {
		follower->disband();
	}

	// do not allow actor to follow it's self
	if (brotherBandingEnabled
	        &&  isBanded()
	        &&  follower != centerActor_) {
		// create a new follow assignment

		follower->bandWith(centerActor_);
	}
}

//-----------------------------------------------------------------------
//	Re-evaluate the portrait type for this player actor

void PlayerActor::recalcPortraitType() {
	PortraitType    pType;
	Actor           *a = getActor();
	ActorAttributes &stats = getBaseStats();

	if (a->isDead())
		pType = kPortraitDead;
	else if (a->_enchantmentFlags & (1 << kActorAsleep))
		pType = kPortraitAsleep;
	else if (stats.vitality >= a->_effectiveStats.vitality * 3)
		pType = kPortraitWounded;
	else if (a->_enchantmentFlags & ((1 << kActorDiseased) | (1 << kActorPoisoned)))
		pType = kPortraitSick;
	else if (stats.vitality * 2 > a->_effectiveStats.vitality * 3)
		pType = kPortraitOuch;
	else if (a->_enchantmentFlags & ((1 << kActorParalyzed) | (1 << kActorFear) | (1 << kActorBlind)))
		pType = kPortraitConfused;
	else if (isAggressive())
		pType = kPortraitAngry;
	else
		pType = kPortraitNormal;

	if (pType != _portraitType)
		updateBrotherPortrait(getPlayerActorID(this), _portraitType = pType);
}


void PlayerActor::recoveryUpdate() { // change name to recovery update
	manaUpdate();
	AttribUpdate();
}



void PlayerActor::AttribUpdate() {
	// get the actor pointer for this character
	Actor *actor = getActor();

	// get the effective stats for this player actor
	ActorAttributes *effStats = &actor->_effectiveStats;

	for (int16 i = 0; i < kNumSkills; i++) {
		// go through each skill and update as needed
		stdAttribUpdate(effStats->skill(i),
		                _baseStats.skill(i),
		                i);
	}
}


void PlayerActor::stdAttribUpdate(uint8 &stat, uint8 baseStat, int16 index) {
	// first find out if this actor is wounded
	if (stat < baseStat) {
		// whole vitality number goes here
		int16 recover;
		int16 fractionRecover;

		// get the whole number first
		recover = kAttribPointsPerUpdate / kAttribPointsPerValue;

		// get the fraction
		fractionRecover = kAttribPointsPerUpdate % kAttribPointsPerValue;

		// if there is an overrun
		if (_attribRecPools[index] + fractionRecover > kAttribPointsPerValue) {
			// add the overrun to the whole number
			recover++;
			_attribRecPools[index] = (_attribRecPools[index] + fractionRecover) - kAttribPointsPerValue;
		} else {
			_attribRecPools[index] += fractionRecover;
		}


		if (stat + recover >= baseStat) {
			stat = baseStat;
		} else {
			stat += recover;
		}
	}
}

void PlayerActor::manaUpdate() {
	const   int kNumManas        = 6;
	const   int minMana         = 0;

	// get the actor pointer for this character
	Actor *actor = getActor();

	// get indirections for each of the effective mana types
	int16 *effectiveMana[kNumManas] = { &actor->_effectiveStats.redMana,
	                                     &actor->_effectiveStats.orangeMana,
	                                     &actor->_effectiveStats.yellowMana,
	                                     &actor->_effectiveStats.greenMana,
	                                     &actor->_effectiveStats.blueMana,
	                                     &actor->_effectiveStats.violetMana
	                                   };

	// get indirections for each of the base mana types
	int16 *baseMana[kNumManas] = { &_baseStats.redMana,
	                                &_baseStats.orangeMana,
	                                &_baseStats.yellowMana,
	                                &_baseStats.greenMana,
	                                &_baseStats.blueMana,
	                                &_baseStats.violetMana
	                              };

	uint16  diff;

	// do each mana type
	for (int16 i = 0; i < kNumManas; i++) {
		int     levelBump;
		int     recRate;

		// if baseMana has gone to zero, force it to 1
		if (*baseMana[i] <= 0) *baseMana[i] = 1;

		//  Make mana harder to increase as it goes up.
		if (*baseMana[i] >= 100) levelBump = 40;
		else if (*baseMana[i] >= 40) levelBump = 20;
		else levelBump = 10;

		// is their current mana less then their maximum
		if (*effectiveMana[i] < *baseMana[i]) {
			diff = *effectiveMana[i];

			recRate = 1;
			if (*baseMana[i] >= 120)     recRate = 3;
			else if (*baseMana[i] >= 80)      recRate = 2;
			else if (*baseMana[i] >= 40) {
				//  This effectively causes recRate to be 1.5, i.e.
				//  hald of the time its 1 and the other half its 2.
				if (*effectiveMana[i] % 3 == 0) recRate = 2;
			}

			// recover mana at specified rate
			*effectiveMana[i] = clamp(minMana,
			                            *effectiveMana[i] += recRate,
			                            *baseMana[i]);

			// get the difference between the manas
			diff = *effectiveMana[i] - diff;


			// find out if we're recovering from below one third
			if (*effectiveMana[i] < *baseMana[i] / 3) {
				// add the diff
//	Deleted at request of Client.
//				_manaMemory[i] -= diff;
			} else {
				_manaMemory[i] += diff;
			}


			// if we bumped passed the ( +/- ) levelBump mark
			// decrement the base mana
			*baseMana[i] += (_manaMemory[i] / levelBump);

			// get the fraction back to memory
			_manaMemory[i] = _manaMemory[i] % levelBump;

			//WriteStatusF( 4, " mana: %d", *effectiveMana[i] );
		}
	}
}

// get the skill advancement number ( could be zero )
void PlayerActor::skillAdvance(SkillProto *proto,
                               uint8 points,
                               uint8 useMult) { // useMult defaulted to 1
	// get the skill level for the skill passed ( i.e. 1-100 )
	uint8 skillLevel = getSkillLevel(proto, true);   // true, use base stats

	// get the stat index in question
	uint8 stat = getStatIndex(proto);

	// get the percentile chance of advancing
	uint8 advanceChance = ActorAttributes::kSkillBasePercent - skillLevel;

	// call the main body of code
	skillAdvance(stat, advanceChance, points, useMult);
}

// get the skill advancement number ( could be zero )
void PlayerActor::skillAdvance(ActorSkillID stat,
                               uint8 points,
                               uint8 useMult) { // useMult defaulted to 1
	// get the skill level for the skill passed ( i.e. 1-100 )
	uint8 skillLevel = clamp(0, _baseStats.skill(stat), ActorAttributes::kSkillMaxLevel);

	// get the percentile chance of advancing
	uint8 advanceChance = ActorAttributes::kSkillBasePercent - skillLevel;

	// call the main body of code
	skillAdvance(stat, advanceChance, points, useMult);

	//WriteStatusF( 1, "adchan: %d, skillev: %d ", advanceChance, skillLevel );
}


void PlayerActor::skillAdvance(uint8 stat,
                               uint8 advanceChance,
                               uint8 points,
                               uint8 useMult) {
	// roll percentile dice
	if (g_vm->_rnd->getRandomNumber(99) < advanceChance) {
		uint8 increase;
		int16   oldValue = _baseStats.skill(stat) / ActorAttributes::kSkillFracPointsPerLevel;

		// success, now apply the multiplyer
		_attribMemPools[stat] += points * useMult;

		// get the amout of whole increase points
		increase = _attribMemPools[stat] / ActorAttributes::kSkillFracPointsPerLevel;

		// now set the pool with the fraction
		_attribMemPools[stat] =
		    _attribMemPools[stat]
		    -       increase
		    *   ActorAttributes::kSkillFracPointsPerLevel;

		// now apply changes to the _baseStats
		_baseStats.skill(stat) = clamp(
		                            0,
		                            _baseStats.skill(stat) += increase,
		                            ActorAttributes::kSkillMaxLevel);

		if (_baseStats.skill(stat) / ActorAttributes::kSkillFracPointsPerLevel != oldValue) {
			static const char *skillNames[] = {
				ARCHERY_SKILL,
				SWORD_SKILL,
				SHIELD_SKILL,
				BLUDGEON_SKILL,
				DEAD_SKILL,
				SPELL_SKILL,
				DEAD2_SKILL,
				AGILITY_SKILL,
				BRAWN_SKILL
			};

			StatusMsg(SKILL_STATUS, getActor()->objName(), skillNames[stat]);
		}
		//WriteStatusF( 6, "frac: %d inc: %d, base: %d", _attribMemPools[stat], increase, _baseStats.allSkills[stat] );
	}
}

void PlayerActor::vitalityAdvance(uint8 points) {
	while (points-- > 0) {
		if ((int16)g_vm->_rnd->getRandomNumber(ActorAttributes::kVitalityLimit - 1) > _baseStats.vitality) {
			if (++_vitalityMemory >= kVitalityLevelBump) {
				_vitalityMemory -= kVitalityLevelBump;
				_baseStats.vitality++;
				StatusMsg(VITALITY_STATUS, getActor()->objName());
			}
		}
	}

	assert(_baseStats.vitality < ActorAttributes::kVitalityLimit);
}

// this function will return a value of 0 - 4 to indicate
// relative proficiency in requested skill
int8 PlayerActor::getSkillLevel(SkillProto *skill, bool base) { // basestats defaulted to false
	// get the id for this skill
	SpellID skillID = skill->getSpellID();

	// index
	uint16  stat;

	// get the current stats for this player actor
	ActorAttributes *effStats = getEffStats();

	// check to see if this is a special case
	if (skillID == kSkillVitality) {
		return effStats->vitality / ActorAttributes::kSkillFracPointsPerLevel;
	} else if (skillID == kSkillCartography) {
		// cartography has no levels
		return 0;
	}

	// get the index
	stat = getStatIndex(skill);


	// stat stored as skillLevel *
	//                kSkillFracPointsPerLevel +
	//                skillCurrentFracPoints
	if (base) {
		return clamp(0,
		             _baseStats.skill(stat) / ActorAttributes::kSkillFracPointsPerLevel,
		             ActorAttributes::kSkillLevels - 1);
	} else {
		return clamp(0,
		             effStats->skill(stat) / ActorAttributes::kSkillFracPointsPerLevel,
		             ActorAttributes::kSkillLevels - 1);
	}
}

uint8 PlayerActor::getStatIndex(SkillProto *proto) {
	// get the id for this skill
	SpellID skillID = proto->getSpellID();
	uint16  stat;

	// now map the id gotten from spellid to the
	// attributeskilll enum for the allSkills array
	switch (skillID) {
	case kSkillPickpocket:
		stat = kSkillIDPilfer;
		break;

	case kSkillSeeHidden:
		stat = kSkillIDSpotHidden;
		break;

	case kSkillLockPick:
		stat = kSkillIDLockpick;
		break;

	case kSkillFirstAid:
		stat = kSkillIDFirstAid;
		break;

	case kSkillArchery:
		stat = kSkillIDArchery;
		break;

	case kSkillSwordcraft:
		stat = kSkillIDSwordcraft;
		break;

	case kSkillShieldcraft:
		stat = kSkillIDShieldcraft;
		break;

	case kSkillBludgeon:
		stat = kSkillIDBludgeon;
		break;

	case kSkillThrowing:
		stat = kSkillIDThrowing;
		break;

	case kSkillSpellcraft:
		stat = kSkillIDSpellcraft;
		break;

	case kSkillStealth:
		stat = kSkillIDStealth;
		break;

	case kSkillAgility:
		stat = kSkillIDAgility;
		break;

	case kSkillBrawn:
		stat = kSkillIDBrawn;
		break;

	default:

		error("Invalid case detected: Player.cpp-getSkillLevel() = %d", skillID);
	}

	// make sure we have a good index
	if (stat >= kNumSkills) {
		error("Invalid array index detected: Player.cpp-getSkillLevel()");
	}

	// return the index
	return stat;
}

ActorAttributes *PlayerActor::getEffStats() {
	// get the actor pointer for this character
	Actor *actor = getActor();

	// get the effective stats for this player actor
	ActorAttributes *effStats = &actor->_effectiveStats;

	// valid?
	assert(effStats);

	// return current stats for this player actor
	return effStats;
}

//-----------------------------------------------------------------------
//	Notify the user of attack if necessary

void PlayerActor::handleAttacked() {
	if (!_notifiedOfAttack) {
		StatusMsg(ATTACK_STATUS, getActor()->objName());
		_notifiedOfAttack = true;
	}
}

/* ===================================================================== *
   Functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Return a pointer to a PlayerActor given it's ID

PlayerActor *getPlayerActorAddress(PlayerActorID id) {
	assert(id >= 0 && id < (int)g_vm->_playerList.size());

	return g_vm->_playerList[id];
}

//-----------------------------------------------------------------------
//	Return a PlayerActor ID given it's address

PlayerActorID getPlayerActorID(PlayerActor *p) {
	for (int i = 0; i < (int)g_vm->_playerList.size(); ++i) {
		if (g_vm->_playerList[i] == p)
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------
//	Return a pointer the center actor's Actor structure

Actor *getCenterActor() {
	return g_vm->_playerList[centerActor]->getActor();
}

//-----------------------------------------------------------------------
//  Return the center actor's object ID

ObjectID getCenterActorID() {
	return ActorBaseID + centerActor;
}

//-----------------------------------------------------------------------
//  Return the center actor's player actor ID

PlayerActorID getCenterActorPlayerID() {
	return centerActor;
}

//-----------------------------------------------------------------------
//	Set a new center actor based upon a PlayerActor ID

void setCenterActor(PlayerActorID newCenter) {
	extern void setEnchantmentDisplay();

	assert(newCenter < kPlayerActors);

	Actor                       *a = g_vm->_playerList[newCenter]->getActor();
	PlayerActorIterator         iter;
	PlayerActor                 *player;

	//  If this actor is dead return immediately
	if (a->isDead()) return;

	//  Take previous center actor out of fight stance
	getCenterActor()->setFightStance(false);

	// get rid of any following assignments the center actor might have
	if (a->_leader) {
		a->disband();
	}

	centerActor = newCenter;
	viewCenterObject = g_vm->_playerList[centerActor]->getActorID();

	indivReadyNode->changeOwner(newCenter);
	g_vm->_cnm->setPlayerNum(newCenter);
	setEnchantmentDisplay();

	if (a->_curTask != nullptr) {
		a->_curTask->abortTask();
		delete a->_curTask;
		a->_curTask = nullptr;
	}

	//  Set the new centers fight stance based upon his aggression state
	a->setFightStance(g_vm->_playerList[newCenter]->isAggressive());

	// band actors to new center if banding button set
	for (player = iter.first(); player != nullptr; player = iter.next()) {
		player->resolveBanding();
	}

	// clear the last center actor's button state
	updateBrotherRadioButtons(newCenter);
}

//-----------------------------------------------------------------------
//	Set a new center actor based upon an Actor address

void setCenterActor(Actor *newCenter) {
	assert(newCenter->_disposition >= kDispositionPlayer);
	setCenterActor(newCenter->_disposition - kDispositionPlayer);
}

//-----------------------------------------------------------------------
//	Set a new center actor based upon a PlayerActor address

void setCenterActor(PlayerActor *newCenter) {
	assert(getPlayerActorID(newCenter) >= 0);
	setCenterActor(getPlayerActorID(newCenter));
}

//-----------------------------------------------------------------------
//	Return the coordinates of the current center actor

TilePoint centerActorCoords() {
	Actor           *a;

	a = g_vm->_playerList[centerActor]->getActor();
	return a->getLocation();
}


//-----------------------------------------------------------------------
//	Set or clear a player's aggressive state

void setAggression(PlayerActorID player, bool aggression) {
	assert(player >= 0 && player < kPlayerActors);

	Actor       *a = g_vm->_playerList[player]->getActor();

	if (a->isDead()) return;

	if (aggression)
		g_vm->_playerList[player]->setAggression();
	else
		g_vm->_playerList[player]->clearAggression();

	if (player == centerActor)
		a->setFightStance(aggression);

	a->evaluateNeeds();

	updateBrotherAggressionButton(player, aggression);
}

//-----------------------------------------------------------------------
//	Determine if player actor is in an aggressive state

bool isAggressive(PlayerActorID player) {
	assert(player >= 0 && player < kPlayerActors);
	return g_vm->_playerList[player]->isAggressive();
}

//-----------------------------------------------------------------------
//	Adjust the player actors aggression setting based upon their
//	proximity to enemies

void autoAdjustAggression() {
	PlayerActorID       i;

	//  Iterate through all player actors
	for (i = 0; i < kPlayerActors; i++) {
		if (i == centerActor || isBanded(i)) {
			bool            enemiesPresent = false;
			Actor           *actor = g_vm->_playerList[i]->getActor();

			if (actor->getStats()->vitality >= kMinAutoAggressionVitality) {
				GameObject      *obj = nullptr;
				ActiveRegion    *activeReg = getActiveRegion(i);
				TileRegion      region = activeReg->getRegion();
				GameWorld       *world = activeReg->getWorld();

				RegionalObjectIterator  iter(world, region.min, region.max);

				//  Iterate through the objects in this player actor's
				//  active region to determine if their are enemy actor's
				//  in the vicinity.
				for (iter.first(&obj); obj != nullptr; iter.next(&obj)) {
					Actor       *a;

					if (!isActor(obj)) continue;

					a = (Actor *)obj;

					if (a->_disposition == kDispositionEnemy) {
						enemiesPresent = true;
						break;
					}
				}
			}

			//  Set this player actor's aggression
			setAggression(i, enemiesPresent);
		}
	}
}

//-----------------------------------------------------------------------
//	Set a player actor's banding

void setBanded(PlayerActorID player, bool banded) {
	assert(player >= 0 && player < kPlayerActors);

	if (g_vm->_playerList[player]->getActor()->isDead()) return;

	if (banded)
		g_vm->_playerList[player]->setBanded();
	else
		g_vm->_playerList[player]->clearBanded();

	g_vm->_playerList[player]->resolveBanding();

	updateBrotherBandingButton(player, banded);
}

//-----------------------------------------------------------------------
//	Determine if a player actor is banded

bool isBanded(PlayerActorID player) {
	assert(player >= 0 && player < kPlayerActors);
	return g_vm->_playerList[player]->isBanded();
}

//-----------------------------------------------------------------------
//	Globally enable or disable brother banding

void setBrotherBanding(bool enabled) {
	if (brotherBandingEnabled != enabled) {
		brotherBandingEnabled = enabled;

		if (areActorsInitialized()) {
			PlayerActorIterator iter;
			PlayerActor         *player;

			//  Update the state of the banding
			for (player = iter.first(); player != nullptr; player = iter.next()) {
				player->resolveBanding();
			}
		}
	}
}



//-----------------------------------------------------------------------
//	Set the portrait type for this actor based on the current state.

void recalcPortraitType(PlayerActorID brotherID) {
	PlayerActor     *pa = getPlayerActorAddress(brotherID);

	pa->recalcPortraitType();
}

//-----------------------------------------------------------------------
//	Returns an integer value representing this player actor's portrait
//	state

int16 getPortraitType(PlayerActorID id) {
	PlayerActor     *pa = getPlayerActorAddress(id);

	return pa->getPortraitType();
}

//-----------------------------------------------------------------------
//	Given an actor, returns the corresponding player Actor ID

bool actorToPlayerID(Actor *a, PlayerActorID &result) {
	if (a->_disposition >= kDispositionPlayer) {
		result = a->_disposition - kDispositionPlayer;
		return true;
	}

	return false;
}

bool actorIDToPlayerID(ObjectID id, PlayerActorID &result) {
	if (!isActor(id)) return false;

	Actor       *a = (Actor *)GameObject::objectAddress(id);

	if (a->_disposition >= kDispositionPlayer) {
		result = a->_disposition - kDispositionPlayer;
		return true;
	}

	return false;
}

void handlePlayerActorDeath(PlayerActorID id) {
	assert(id >= 0 && id < kPlayerActors);

	if (getCenterActor()->isDead()) {
		PlayerActor                 *newCenter;
		LivingPlayerActorIterator   iter;

		if ((newCenter = iter.first()) != nullptr)
			setCenterActor(getPlayerActorID(newCenter));
		else
			allPlayerActorsDead = true;
	}

	PlayerActor     *player = g_vm->_playerList[id];

	player->clearAggression();
	player->clearBanded();
	updateBrotherAggressionButton(id, false);
	updateBrotherBandingButton(id, false);

	StatusMsg(DEATH_STATUS, player->getActor()->objName());
}

//-----------------------------------------------------------------------
//	Transport the center actor and the banded brothers who have a path
//	to the center actor

void transportCenterBand(const Location &loc) {
	assert(isWorld(loc._context));

	fadeDown();

	Actor       *center = getCenterActor();
	TilePoint   centerLoc = center->getLocation();
	ObjectID    centerWorldID = center->world()->thisID();
	PlayerActor *player;
	LivingPlayerActorIterator   iter;

	center->move(loc);
	if (center->_moveTask != nullptr)
		center->_moveTask->finishWalk();

	for (player = iter.first();
	        player != nullptr;
	        player = iter.next()) {
		Actor       *a = player->getActor();

		if (a != center
		        &&  player->isBanded()
		        &&  a->world()->thisID() == centerWorldID
		        &&  checkPath(
		            centerWorldID,
		            a->proto()->height,
		            a->getLocation(),
		            centerLoc)) {
			TilePoint       dest;

			dest =  selectNearbySite(
			            loc._context,
			            loc,
			            1,
			            3,
			            false);

			if (dest != Nowhere) {
				a->move(Location(dest, loc._context));
				if (a->_moveTask != nullptr)
					a->_moveTask->finishWalk();
				player->resolveBanding();
			}
		}
	}

	updateMainDisplay();

	fadeUp();
}

//-----------------------------------------------------------------------

void handlePlayerActorAttacked(PlayerActorID id) {
	PlayerActor     *pa = getPlayerActorAddress(id);

	pa->handleAttacked();
}

//-----------------------------------------------------------------------

void handleEndOfCombat() {
	PlayerActorID       i;

	//  Iterate through all player actors
	for (i = 0; i < kPlayerActors; i++)
		g_vm->_playerList[i]->resetAttackNotification();
}

/* ======================================================================= *
   PlayerActor list management functions
 * ======================================================================= */



//	This structure is used in archiving the player actor list
struct PlayerActorArchive {
	int16               _portraitType;
	uint16              flags;
	ActorAttributes     _baseStats;
	int16               _manaMemory[kNumManas];
	uint8               _attribRecPools[kNumSkills];
	uint8               _attribMemPools[kNumSkills];
	uint8               _vitalityMemory;
	bool                _notifiedOfAttack;
};


//-----------------------------------------------------------------------
//	Initialize the player list

void initPlayerActors() {
	if (g_vm->getGameId() == GID_DINO) {
		warning("TODO: initPlayerActors() for Dino");
		return;
	}

	g_vm->_playerList.push_back(new PlayerActor(ActorBaseID + 0)); // Julian
	g_vm->_playerList.push_back(new PlayerActor(ActorBaseID + 1)); // Philip
	g_vm->_playerList.push_back(new PlayerActor(ActorBaseID + 2)); // Kevin

	for (int i = 0; i < kPlayerActors; i++) {
		PlayerActor     *p = g_vm->_playerList[i];
		Actor           *a = p->getActor();
		ActorProto      *proto = (ActorProto *)a->proto();

		//  Set the portrait type
		p->_portraitType = kPortraitNormal;

		//  Clear all flags
		p->_flags = 0;
		//  Copy the base stats from the actor's prototype
		memcpy(&p->_baseStats, &proto->baseStats, sizeof(p->_baseStats));

		//  Clear out the accumulation arrays
		memset(&p->_manaMemory, 0, sizeof(p->_manaMemory));
		memset(&p->_attribRecPools, 0, sizeof(p->_attribRecPools));
		memset(&p->_attribMemPools, 0, sizeof(p->_attribMemPools));

		//  Clear the _vitalityMemory
		p->_vitalityMemory = 0;

		//  Clear the attack notification flag
		p->_notifiedOfAttack = false;

		//  Set the actor's disposition field to reflect that that
		//  actor is a player actor
		a->_disposition = kDispositionPlayer + i;

		//  Turn on banding for player actors
		setBanded(i, true);
	}

	readyContainerSetup();
}

void savePlayerActors(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving PlayerActors");

	outS->write("PLYR", 4);
	CHUNK_BEGIN;
	for (int i = 0; i < kPlayerActors; i++) {
		debugC(3, kDebugSaveload, "Saving PlayerActor %d", i);

		PlayerActor *p = g_vm->_playerList[i];

		//  Store the portrait type
		out->writeSint16LE(p->_portraitType);

		//  Store the flags
		out->writeUint16LE(p->_flags);

		//  Store the base stats
		p->_baseStats.write(out);

		//  Store accumulation arrays
		for (int j = 0; j < kNumManas; ++j)
			out->writeSint16LE(p->_manaMemory[j]);

		for (int j = 0; j < kNumSkills; ++j)
			out->writeByte(p->_attribRecPools[j]);

		for (int j = 0; j < kNumSkills; ++j)
			out->writeByte(p->_attribMemPools[j]);

		//  Store the vitality memory
		out->writeByte(p->_vitalityMemory);

		//  Store the attack notification flag
		out->writeUint16LE(p->_notifiedOfAttack);

		debugC(4, kDebugSaveload, "... playerList[%d]._portraitType = %d", i, p->_portraitType);
		debugC(4, kDebugSaveload, "... playerList[%d].flags = %d", i, p->_flags);
		debugC(4, kDebugSaveload, "... playerList[%d]._vitalityMemory = %d", i, p->_vitalityMemory);
		debugC(4, kDebugSaveload, "... playerList[%d]._notifiedOfAttack = %d", i, p->_notifiedOfAttack);
	}
	CHUNK_END;
}

void loadPlayerActors(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading PlayerActors");

	for (int i = 0; i < kPlayerActors; i++) {
		debugC(3, kDebugSaveload, "Loading PlayerActor %d", i);

		PlayerActor *p = g_vm->_playerList[i];

		//  Restore the portrait type
		p->_portraitType = in->readSint16LE();

		//  Restore the flags
		p->_flags = in->readUint16LE();

		//  Restore the base stats
		p->_baseStats.read(in);

		//  Restore the accumulation arrays
		for (int j = 0; j < kNumManas; ++j)
			p->_manaMemory[j] = in->readSint16LE();

		for (int j = 0; j < kNumSkills; ++j)
			p->_attribRecPools[j] = in->readByte();

		for (int j = 0; j < kNumSkills; ++j)
			p->_attribMemPools[j] = in->readByte();

		//  Restore the vitality memory
		p->_vitalityMemory = in->readByte();

		//  Restore the attack notification flag
		p->_notifiedOfAttack = in->readUint16LE();

		debugC(4, kDebugSaveload, "... playerList[%d]._portraitType = %d", i, p->_portraitType);
		debugC(4, kDebugSaveload, "... playerList[%d].flags = %d", i, p->_flags);
		debugC(4, kDebugSaveload, "... playerList[%d]._vitalityMemory = %d", i, p->_vitalityMemory);
		debugC(4, kDebugSaveload, "... playerList[%d]._notifiedOfAttack = %d", i, p->_notifiedOfAttack);
	}

	readyContainerSetup();
}

//-----------------------------------------------------------------------
//	Cleanup the player actor list

void cleanupPlayerActors() {
	cleanupReadyContainers();
}

/* ======================================================================= *
   CenterActor management function prototypes
 * ======================================================================= */

//	This structure is used in archiving the center actor ID and the view
//	object ID
struct CenterActorArchive {
	PlayerActorID   centerActor;
	ObjectID        viewCenterObject;
};

//-----------------------------------------------------------------------
//	Initialize the center actor ID and view object ID

void initCenterActor() {
	if (g_vm->getGameId() == GID_DINO) {
		warning("TODO: initCenterActor() for Dino");
		return;
	}

	centerActor = FTA_JULIAN;
	viewCenterObject = g_vm->_playerList[centerActor]->getActorID();

	// clear the last center actor's button state
	updateBrotherRadioButtons(FTA_JULIAN);
}

void saveCenterActor(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving CenterActor");

	outS->write("CNTR", 4);
	CHUNK_BEGIN;
	//  Store the center actor and view object
	out->writeSint16LE(centerActor);
	out->writeUint16LE(viewCenterObject);
	CHUNK_END;

	debugC(3, kDebugSaveload, "... centerActor = %d", centerActor);
	debugC(3, kDebugSaveload, "... viewCenterObject = %d", viewCenterObject);
}

void loadCenterActor(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading CenterActor");

	//  Restore the center actor and view object
	centerActor = in->readSint16LE();
	viewCenterObject = in->readUint16LE();

	debugC(3, kDebugSaveload, "... centerActor = %d", centerActor);
	debugC(3, kDebugSaveload, "... viewCenterObject = %d", viewCenterObject);
}

//-----------------------------------------------------------------------
//	Iterates through all player actors

PlayerActor *PlayerActorIterator::first() {
	_index = 0;
	return g_vm->_playerList[_index++];
}

PlayerActor *PlayerActorIterator::next() {
	return (_index < kPlayerActors) ? g_vm->_playerList[_index++] : NULL;
}

//-----------------------------------------------------------------------
//	Iterates through all player actors that are not dead.

PlayerActor *LivingPlayerActorIterator::first() {
	_index = 0;
	return LivingPlayerActorIterator::next();
}

PlayerActor *LivingPlayerActorIterator::next() {
	if (_index >= kPlayerActors)
		return nullptr;

	Actor *a = g_vm->_playerList[_index]->getActor();

	while (a == nullptr || a->isDead()) {
		if (++_index >= kPlayerActors)
			break;
		a = g_vm->_playerList[_index]->getActor();
	}

	return (_index < kPlayerActors) ? g_vm->_playerList[_index++] : nullptr;
}

} // end of namespace Saga2
