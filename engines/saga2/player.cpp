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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/intrface.h"
#include "saga2/contain.h"
#include "saga2/task.h"
#include "saga2/motion.h"
#include "saga2/transit.h"
#include "saga2/localize.h"
#include "saga2/savefile.h"

namespace Saga2 {

extern bool             allPlayerActorsDead;

extern ObjectID         viewCenterObject;   //  ID of object which the
//  camera tracks

extern ReadyContainerView   *TrioCviews[kNumViews];
extern ReadyContainerView   *indivCviewTop, *indivCviewBot;
extern ContainerNode        *indivReadyNode;

void updateMainDisplay(void);

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

//  Master list of all playerActor structures
PlayerActor playerList[playerActors] = {
	PlayerActor(ActorBaseID +  0),       //  Julian
	PlayerActor(ActorBaseID +  1),       //  Philip
	PlayerActor(ActorBaseID +  2),       //  Kevin
};


/* ===================================================================== *
   Methods
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Resolve the banding state of this actor

void PlayerActor::resolveBanding(void) {
	Actor *follower         = getActor();
	Actor *centerActor_     = getCenterActor();

	// if already following, tell the actor to cease and desist
	if (follower->leader) {
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

void PlayerActor::recalcPortraitType(void) {
	PortraitType    pType;
	Actor           *a = getActor();
	ActorAttributes &stats = getBaseStats();

	if (a->isDead())
		pType = kPortraitDead;
	else if (a->enchantmentFlags & (1 << actorAsleep))
		pType = kPortraitAsleep;
	else if (stats.vitality >= a->effectiveStats.vitality * 3)
		pType = kPortraitWounded;
	else if (a->enchantmentFlags & ((1 << actorDiseased) | (1 << actorPoisoned)))
		pType = kPortraitSick;
	else if (stats.vitality * 2 > a->effectiveStats.vitality * 3)
		pType = kPortraitOuch;
	else if (a->enchantmentFlags & ((1 << actorParalyzed) | (1 << actorFear) | (1 << actorBlind)))
		pType = kPortraitConfused;
	else if (isAggressive())
		pType = kPortraitAngry;
	else
		pType = kPortraitNormal;

	if (pType != portraitType)
		updateBrotherPortrait(getPlayerActorID(this), portraitType = pType);
}


void PlayerActor::recoveryUpdate(void) { // change name to recovery update
	manaUpdate();
	AttribUpdate();
}



void PlayerActor::AttribUpdate(void) {
	// get the actor pointer for this character
	Actor *actor = getActor();

	// get the effective stats for this player actor
	ActorAttributes *effStats = &actor->effectiveStats;

	for (int16 i = 0; i < numSkills; i++) {
		// go through each skill and update as needed
		stdAttribUpdate(effStats->skill(i),
		                baseStats.skill(i),
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
		recover = attribPointsPerUpdate / attribPointsPerValue;

		// get the fraction
		fractionRecover = attribPointsPerUpdate % attribPointsPerValue;

		// if there is an overrun
		if (attribRecPools[index] + fractionRecover > attribPointsPerValue) {
			// add the overrun to the whole number
			recover++;
			attribRecPools[index] = (attribRecPools[index] + fractionRecover) - attribPointsPerValue;
		} else {
			attribRecPools[index] += fractionRecover;
		}


		if (stat + recover >= baseStat) {
			stat = baseStat;
		} else {
			stat += recover;
		}
	}
}

void PlayerActor::manaUpdate(void) {
	const   int numManas        = 6;
	const   int minMana         = 0;

	// get the actor pointer for this character
	Actor *actor = getActor();

	// get indirections for each of the effective mana types
	int16 *effectiveMana[numManas] = { &actor->effectiveStats.redMana,
	                                     &actor->effectiveStats.orangeMana,
	                                     &actor->effectiveStats.yellowMana,
	                                     &actor->effectiveStats.greenMana,
	                                     &actor->effectiveStats.blueMana,
	                                     &actor->effectiveStats.violetMana
	                                   };

	// get indirections for each of the base mana types
	int16 *baseMana[numManas] = { &baseStats.redMana,
	                                &baseStats.orangeMana,
	                                &baseStats.yellowMana,
	                                &baseStats.greenMana,
	                                &baseStats.blueMana,
	                                &baseStats.violetMana
	                              };

	uint16  diff;

	// do each mana type
	for (int16 i = 0; i < numManas; i++) {
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
//				manaMemory[i] -= diff;
			} else {
				manaMemory[i] += diff;
			}


			// if we bumped passed the ( +/- ) levelBump mark
			// decrement the base mana
			*baseMana[i] += (manaMemory[i] / levelBump);

			// get the fraction back to memory
			manaMemory[i] = manaMemory[i] % levelBump;

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
	uint8 advanceChance = ActorAttributes::skillBasePercent - skillLevel;

	// call the main body of code
	skillAdvance(stat, advanceChance, points, useMult);
}

// get the skill advancement number ( could be zero )
void PlayerActor::skillAdvance(ActorSkillID stat,
                               uint8 points,
                               uint8 useMult) { // useMult defaulted to 1
	// get the skill level for the skill passed ( i.e. 1-100 )
	uint8 skillLevel = clamp(0, baseStats.skill(stat), ActorAttributes::skillMaxLevel);

	// get the percentile chance of advancing
	uint8 advanceChance = ActorAttributes::skillBasePercent - skillLevel;

	// call the main body of code
	skillAdvance(stat, advanceChance, points, useMult);

	//WriteStatusF( 1, "adchan: %d, skillev: %d ", advanceChance, skillLevel );
}


void PlayerActor::skillAdvance(uint8 stat,
                               uint8 advanceChance,
                               uint8 points,
                               uint8 useMult) {
	// roll percentile dice
	if (rand() % 100 < advanceChance) {
		uint8 increase;
		int16   oldValue = baseStats.skill(stat) / ActorAttributes::skillFracPointsPerLevel;

		// success, now apply the multiplyer
		attribMemPools[stat] += points * useMult;

		// get the amout of whole increase points
		increase = attribMemPools[stat] / ActorAttributes::skillFracPointsPerLevel;

		// now set the pool with the fraction
		attribMemPools[stat] =
		    attribMemPools[stat]
		    -       increase
		    *   ActorAttributes::skillFracPointsPerLevel;

		// now apply changes to the baseStats
		baseStats.skill(stat) = clamp(
		                            0,
		                            baseStats.skill(stat) += increase,
		                            ActorAttributes::skillMaxLevel);

		if (baseStats.skill(stat) / ActorAttributes::skillFracPointsPerLevel != oldValue) {
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
		//WriteStatusF( 6, "frac: %d inc: %d, base: %d", attribMemPools[stat], increase, baseStats.allSkills[stat] );
	}
}

void PlayerActor::vitalityAdvance(uint8 points) {
	while (points-- > 0) {
		if (rand() % ActorAttributes::vitalityLimit > baseStats.vitality) {
			if (++vitalityMemory >= vitalityLevelBump) {
				vitalityMemory -= vitalityLevelBump;
				baseStats.vitality++;
				StatusMsg(VITALITY_STATUS, getActor()->objName());
			}
		}
	}

	assert(baseStats.vitality < ActorAttributes::vitalityLimit);
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
	if (skillID == skillVitality) {
		return effStats->vitality / ActorAttributes::skillFracPointsPerLevel;
	} else if (skillID == skillCartography) {
		// cartography has no levels
		return 0;
	}

	// get the index
	stat = getStatIndex(skill);


	// stat stored as skillLevel *
	//                skillFracPointsPerLevel +
	//                skillCurrentFracPoints
	if (base) {
		return clamp(0,
		             baseStats.skill(stat) / ActorAttributes::skillFracPointsPerLevel,
		             ActorAttributes::skillLevels - 1);
	} else {
		return clamp(0,
		             effStats->skill(stat) / ActorAttributes::skillFracPointsPerLevel,
		             ActorAttributes::skillLevels - 1);
	}
}

uint8 PlayerActor::getStatIndex(SkillProto *proto) {
	// get the id for this skill
	SpellID skillID = proto->getSpellID();
	uint16  stat;

	// now map the id gotten from spellid to the
	// attributeskilll enum for the allSkills array
	switch (skillID) {
	case skillPickpocket:
		stat = skillIDPilfer;
		break;

	case skillSeeHidden:
		stat = skillIDSpotHidden;
		break;

	case skillLockPick:
		stat = skillIDLockpick;
		break;

	case skillFirstAid:
		stat = skillIDFirstAid;
		break;

	case skillArchery:
		stat = skillIDArchery;
		break;

	case skillSwordcraft:
		stat = skillIDSwordcraft;
		break;

	case skillShieldcraft:
		stat = skillIDShieldcraft;
		break;

	case skillBludgeon:
		stat = skillIDBludgeon;
		break;

	case skillThrowing:
		stat = skillIDThrowing;
		break;

	case skillSpellcraft:
		stat = skillIDSpellcraft;
		break;

	case skillStealth:
		stat = skillIDStealth;
		break;

	case skillAgility:
		stat = skillIDAgility;
		break;

	case skillBrawn:
		stat = skillIDBrawn;
		break;

	default:

		error("Invalid case detected: Player.cpp-getSkillLevel() = %d", skillID);
	}

	// make sure we have a good index
	if (stat >= numSkills) {
		error("Invalid array index detected: Player.cpp-getSkillLevel()");
	}

	// return the index
	return stat;
}

ActorAttributes *PlayerActor::getEffStats(void) {
	// get the actor pointer for this character
	Actor *actor = getActor();

	// get the effective stats for this player actor
	ActorAttributes *effStats = &actor->effectiveStats;

	// valid?
	assert(effStats);

	// return current stats for this player actor
	return effStats;
}

//-----------------------------------------------------------------------
//	Notify the user of attack if necessary

void PlayerActor::handleAttacked(void) {
	if (!notifiedOfAttack) {
		StatusMsg(ATTACK_STATUS, getActor()->objName());
		notifiedOfAttack = true;
	}
}

/* ===================================================================== *
   Functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Return a pointer to a PlayerActor given it's ID

PlayerActor *getPlayerActorAddress(PlayerActorID id) {
	assert(id >= 0 && id < ARRAYSIZE(playerList));

	return &playerList[id];
}

//-----------------------------------------------------------------------
//	Return a PlayerActor ID given it's address

PlayerActorID getPlayerActorID(PlayerActor *p) {
	return p - playerList;
}

//-----------------------------------------------------------------------
//	Return a pointer the center actor's Actor structure

Actor *getCenterActor(void) {
	return playerList[centerActor].getActor();
}

//-----------------------------------------------------------------------
//  Return the center actor's object ID

ObjectID getCenterActorID(void) {
	return playerList[centerActor].getActorID();
}

//-----------------------------------------------------------------------
//  Return the center actor's player actor ID

PlayerActorID getCenterActorPlayerID(void) {
	return centerActor;
}

//-----------------------------------------------------------------------
//	Set a new center actor based upon a PlayerActor ID

void setCenterActor(PlayerActorID newCenter) {
	extern void setEnchantmentDisplay(void);

	assert(newCenter < playerActors);

	Actor                       *a = playerList[newCenter].getActor();
	PlayerActorIterator         iter;
	PlayerActor                 *player;

	//  If this actor is dead return immediately
	if (a->isDead()) return;

	//  Take previous center actor out of fight stance
	getCenterActor()->setFightStance(false);

	// get rid of any following assignments the center actor might have
	if (a->leader) {
		a->disband();
	}

	centerActor = newCenter;
	viewCenterObject = playerList[centerActor].getActorID();

	indivReadyNode->changeOwner(newCenter);
	globalContainerList.setPlayerNum(newCenter);
	setEnchantmentDisplay();

	if (a->curTask != NULL) {
		a->curTask->abortTask();
		delete a->curTask;
		a->curTask = NULL;
	}

	//  Set the new centers fight stance based upon his aggression state
	a->setFightStance(playerList[newCenter].isAggressive());

	// band actors to new center if banding button set
	for (player = iter.first(); player != NULL; player = iter.next()) {
		player->resolveBanding();
	}

	// clear the last center actor's button state
	updateBrotherRadioButtons(newCenter);
}

//-----------------------------------------------------------------------
//	Set a new center actor based upon an Actor address

void setCenterActor(Actor *newCenter) {
	assert(newCenter->disposition >= dispositionPlayer);
	setCenterActor(newCenter->disposition - dispositionPlayer);
}

//-----------------------------------------------------------------------
//	Set a new center actor based upon a PlayerActor address

void setCenterActor(PlayerActor *newCenter) {
	assert(newCenter >= playerList && newCenter < &playerList[playerActors]);
	setCenterActor(newCenter - playerList);
}

//-----------------------------------------------------------------------
//	Return the coordinates of the current center actor

TilePoint centerActorCoords(void) {
	Actor           *a;

	a = playerList[centerActor].getActor();
	return a->getLocation();
}


//-----------------------------------------------------------------------
//	Set or clear a player's aggressive state

void setAggression(PlayerActorID player, bool aggression) {
	assert(player >= 0 && player < playerActors);

	Actor       *a = playerList[player].getActor();

	if (a->isDead()) return;

	if (aggression)
		playerList[player].setAggression();
	else
		playerList[player].clearAggression();

	if (player == centerActor)
		a->setFightStance(aggression);

	a->evaluateNeeds();

	updateBrotherAggressionButton(player, aggression);
}

//-----------------------------------------------------------------------
//	Determine if player actor is in an aggressive state

bool isAggressive(PlayerActorID player) {
	assert(player >= 0 && player < playerActors);
	return playerList[player].isAggressive();
}

//-----------------------------------------------------------------------
//	Adjust the player actors aggression setting based upon their
//	proximity to enemies

void autoAdjustAggression(void) {
	PlayerActorID       i;

	//  Iterate through all player actors
	for (i = 0; i < playerActors; i++) {
		if (i == centerActor || isBanded(i)) {
			bool            enemiesPresent = false;
			Actor           *actor = playerList[i].getActor();

			if (actor->getStats()->vitality >= minAutoAggressionVitality) {
				GameObject      *obj;
				ActiveRegion    *activeReg = getActiveRegion(i);
				TileRegion      region = activeReg->getRegion();
				GameWorld       *world = activeReg->getWorld();

				RegionalObjectIterator  iter(world, region.min, region.max);

				//  Iterate through the objects in this player actor's
				//  active region to determine if their are enemy actor's
				//  in the vicinity.
				for (iter.first(&obj); obj != NULL; iter.next(&obj)) {
					Actor       *a;

					if (!isActor(obj)) continue;

					a = (Actor *)obj;

					if (a->disposition == dispositionEnemy) {
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
	assert(player >= 0 && player < playerActors);

	if (playerList[player].getActor()->isDead()) return;

	if (banded)
		playerList[player].setBanded();
	else
		playerList[player].clearBanded();

	playerList[player].resolveBanding();

	updateBrotherBandingButton(player, banded);
}

//-----------------------------------------------------------------------
//	Determine if a player actor is banded

bool isBanded(PlayerActorID player) {
	assert(player >= 0 && player < playerActors);
	return playerList[player].isBanded();
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
			for (player = iter.first(); player != NULL; player = iter.next()) {
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
	if (a->disposition >= dispositionPlayer) {
		result = a->disposition - dispositionPlayer;
		return true;
	}

	return false;
}

bool actorIDToPlayerID(ObjectID id, PlayerActorID &result) {
	if (!isActor(id)) return false;

	Actor       *a = (Actor *)GameObject::objectAddress(id);

	if (a->disposition >= dispositionPlayer) {
		result = a->disposition - dispositionPlayer;
		return true;
	}

	return false;
}

void handlePlayerActorDeath(PlayerActorID id) {
	assert(id >= 0 && id < playerActors);

	if (getCenterActor()->isDead()) {
		PlayerActor                 *newCenter;
		LivingPlayerActorIterator   iter;

		if ((newCenter = iter.first()) != NULL)
			setCenterActor(getPlayerActorID(newCenter));
		else
			allPlayerActorsDead = true;
	}

	PlayerActor     *player = &playerList[id];

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
	assert(isWorld(loc.context));

	fadeDown();

	Actor       *center = getCenterActor();
	TilePoint   centerLoc = center->getLocation();
	ObjectID    centerWorldID = center->world()->thisID();
	PlayerActor *player;
	LivingPlayerActorIterator   iter;

	center->move(loc);
	if (center->moveTask != NULL) center->moveTask->finishWalk();

	for (player = iter.first();
	        player != NULL;
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
			            loc.context,
			            loc,
			            1,
			            3,
			            false);

			if (dest != Nowhere) {
				a->move(Location(dest, loc.context));
				if (a->moveTask != NULL) a->moveTask->finishWalk();
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

void handleEndOfCombat(void) {
	PlayerActorID       i;

	//  Iterate through all player actors
	for (i = 0; i < playerActors; i++)
		playerList[i].resetAttackNotification();
}

/* ======================================================================= *
   PlayerActor list management functions
 * ======================================================================= */



//	This structure is used in archiving the player actor list
struct PlayerActorArchive {
	int16               portraitType;
	uint16              flags;
	ActorAttributes     baseStats;
	int16               manaMemory[numManas];
	uint8               attribRecPools[numSkills];
	uint8               attribMemPools[numSkills];
	uint8               vitalityMemory;
	bool                notifiedOfAttack;
};


//-----------------------------------------------------------------------
//	Initialize the player list

void initPlayerActors(void) {
	PlayerActorID   i;

	for (i = 0; i < playerActors; i++) {
		PlayerActor     *p = &playerList[i];
		Actor           *a = p->getActor();
		ActorProto      *proto = (ActorProto *)a->proto();

		//  Set the portrait type
		p->portraitType = kPortraitNormal;

		//  Clear all flags
		p->flags = 0;
		//  Copy the base stats from the actor's prototype
		memcpy(&p->baseStats, &proto->baseStats, sizeof(p->baseStats));

		//  Clear out the accumulation arrays
		memset(&p->manaMemory, 0, sizeof(p->manaMemory));
		memset(&p->attribRecPools, 0, sizeof(p->attribRecPools));
		memset(&p->attribMemPools, 0, sizeof(p->attribMemPools));

		//  Clear the vitalityMemory
		p->vitalityMemory = 0;

		//  Clear the attack notification flag
		p->notifiedOfAttack = false;

		//  Set the actor's disposition field to reflect that that
		//  actor is a player actor
		a->disposition = dispositionPlayer + i;

		//  Turn on banding for player actors
		setBanded(i, true);
	}

	readyContainerSetup();
}

//-----------------------------------------------------------------------
//	Save the player list data to a save file

void savePlayerActors(SaveFileConstructor &saveGame) {
	int16                   i;
	PlayerActorArchive      archiveBuffer[playerActors];

	for (i = 0; i < playerActors; i++) {
		PlayerActor         *p = &playerList[i];
		PlayerActorArchive  *a = &archiveBuffer[i];

		//  Store the portrait type
		a->portraitType = p->portraitType;

		//  Store the flags
		a->flags = p->flags;

		//  Store the base stats
		memcpy(&a->baseStats, &p->baseStats, sizeof(a->baseStats));

		//  Store accumulation arrays
		memcpy(
		    &a->manaMemory,
		    &p->manaMemory,
		    sizeof(a->manaMemory));
		memcpy(
		    &a->attribRecPools,
		    &p->attribRecPools,
		    sizeof(a->attribRecPools));
		memcpy(
		    &a->attribMemPools,
		    &p->attribMemPools,
		    sizeof(a->attribMemPools));

		//  Store the vitality memory
		a->vitalityMemory = p->vitalityMemory;

		//  Store the attack notification flag
		a->notifiedOfAttack = p->notifiedOfAttack;
	}

	//  Write the player actor chunk
	saveGame.writeChunk(
	    MakeID('P', 'L', 'Y', 'R'),
	    archiveBuffer,
	    sizeof(archiveBuffer));
}

//-----------------------------------------------------------------------
//	Load the player list data from a save file

void loadPlayerActors(SaveFileReader &saveGame) {
	int16                   i;
	PlayerActorArchive      archiveBuffer[playerActors];

	saveGame.read(archiveBuffer, sizeof(archiveBuffer));

	for (i = 0; i < playerActors; i++) {
		PlayerActor         *p = &playerList[i];
		PlayerActorArchive  *a = &archiveBuffer[i];

		//  Restore the portrait type
		p->portraitType = a->portraitType;

		//  Restore the flags
		p->flags = a->flags;

		//  Restore the base stats
		memcpy(&p->baseStats, &a->baseStats, sizeof(p->baseStats));

		//  Restore the accumulation arrays
		memcpy(
		    &p->manaMemory,
		    &a->manaMemory,
		    sizeof(p->manaMemory));
		memcpy(
		    &p->attribRecPools,
		    &a->attribRecPools,
		    sizeof(p->attribRecPools));
		memcpy(
		    &p->attribMemPools,
		    &a->attribMemPools,
		    sizeof(p->attribMemPools));

		//  Restore the vitality memory
		p->vitalityMemory = a->vitalityMemory;

		//  Restore the attack notification flag
		p->notifiedOfAttack = a->notifiedOfAttack;
	}

	readyContainerSetup();
}

//-----------------------------------------------------------------------
//	Cleanup the player actor list

void cleanupPlayerActors(void) {
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

void initCenterActor(void) {
	centerActor = FTA_JULIAN;
	viewCenterObject = playerList[centerActor].getActorID();

	// clear the last center actor's button state
	updateBrotherRadioButtons(FTA_JULIAN);
}

//-----------------------------------------------------------------------
//	Save the center actor ID and the view object ID to a save file

void saveCenterActor(SaveFileConstructor &saveGame) {
	CenterActorArchive  a;

	//  Store the center actor and view object
	a.centerActor       = centerActor;
	a.viewCenterObject  = viewCenterObject;

	saveGame.writeChunk(MakeID('C', 'N', 'T', 'R'), &a, sizeof(a));
}

//-----------------------------------------------------------------------
//	Load the center actor ID and the view object ID from the save file

void loadCenterActor(SaveFileReader &saveGame) {
	CenterActorArchive  a;

	saveGame.read(&a, sizeof(a));

	//  Restore the center actor and view object
	centerActor         = a.centerActor;
	viewCenterObject    = a.viewCenterObject;
}

//-----------------------------------------------------------------------
//	Iterates through all player actors

PlayerActor *PlayerActorIterator::first(void) {
	index = 0;
	return &playerList[index++];
}

PlayerActor *PlayerActorIterator::next(void) {
	return (index < playerActors) ? &playerList[index++] : NULL;
}

//-----------------------------------------------------------------------
//	Iterates through all player actors that are not dead.

PlayerActor *LivingPlayerActorIterator::first(void) {
	index = 0;
	return LivingPlayerActorIterator::next();
}

PlayerActor *LivingPlayerActorIterator::next(void) {
	if (index >= playerActors)
		return nullptr;

	Actor       *a = playerList[index].getActor();

	while (a == nullptr || a->isDead()) {
		if (++index >= playerActors)
			break;
		a = playerList[index].getActor();
	}

	return (index < playerActors) ? &playerList[index++] : nullptr;
}

} // end of namespace Saga2
