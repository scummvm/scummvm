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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PLAYER_H
#define SAGA2_PLAYER_H

#include "saga2/actor.h"

namespace Saga2 {

#define FTA_JULIAN  (PlayerActorID)0
#define FTA_PHILIP  (PlayerActorID)1
#define FTA_KEVIN   (PlayerActorID)2

const int playerActors  = 3;
const int BASE_REC_RATE = 1;

const int minAutoAggressionVitality = 5;

/* ======================================================================= *
   PlayerActor -- data specific to possible center actors
 * ======================================================================= */

class ContainerNode;

class PlayerActor {
	friend class Actor;

	friend void initPlayerActors(void);
	friend void cleanupPlayerActors(void);

	ObjectID        actorID;            // ID of player's actor

public:
	int16           portraitType;       // Integer representing portrait state
	// for this player actor
	uint16          flags;              // various flags

	ActorAttributes baseStats;          // Base stats for this actor
	enum PlayerActorFlags {
		playerAggressive        = (1 << 0), // Player is in aggressive mode
		playerBanded            = (1 << 1), // Player is banded
		playerHasCartography    = (1 << 2)  // Player has ability to map
	};

	// recovery information
	enum Recovery {
		baseManaRec             = 1,
		attribPointsPerUpdate   = 1,
		attribPointsPerValue    = 10
	};

	enum {
		vitalityLevelBump       = 50
	};

	//  Container node for ready containers
	ContainerNode           *readyNode;

	// mana 'experience' pool
	int16   manaMemory[numManas];

	// attrib recovery pools
	uint8   attribRecPools[numSkills];

	// skills 'expericene' pool
	uint8   attribMemPools[numSkills];

	// vitality pool
	uint8 vitalityMemory;

	//  Flag indicating wether the user has been notified that this player
	//  actor has been attacked since the last combat
	bool notifiedOfAttack;

	//  Constructor
	PlayerActor(ObjectID a) :  actorID(a), portraitType(0), flags(0), readyNode(NULL),
			vitalityMemory(0), notifiedOfAttack(false) {

		assert(ActorAttributes::skillFracPointsPerLevel > 0);    // this is used in a divide

		memset(&baseStats, 0, sizeof(baseStats));

		for (int i = 0; i < numManas; i++)
			manaMemory[i] = 0;

		for (int i = 0; i < numSkills; i++) {
			attribRecPools[i] = 0;
			attribMemPools[i] = 0;
		}
	}

	// gets level of skill
	int8 getSkillLevel(SkillProto *, bool base = false);

	// get the actorAttributes allskills index from proto
	uint8 getStatIndex(SkillProto *);

	// get the effective stats of this player actor
	ActorAttributes *getEffStats(void);

	// these update a players baseStat skills
	void skillAdvance(uint8 stat,
	                  uint8 advanceChance,
	                  uint8 points,
	                  uint8 useMult = 1);

	void skillAdvance(SkillProto *proto,
	                  uint8 points,
	                  uint8 useMult = 1);

	void skillAdvance(ActorSkillID stat,
	                  uint8 points,
	                  uint8 useMult = 1);

	void vitalityAdvance(uint8 points);

	//  Return Actor structure pointer
	Actor *getActor(void) {
		return (Actor *)GameObject::objectAddress(actorID);
	}

	//  Return Actor's object ID
	ObjectID getActorID(void) {
		return actorID;
	}

	//  Set player to be aggressive
	void setAggression(void) {
		flags |= playerAggressive;
	}

	//  Set player to not aggressive
	void clearAggression(void) {
		flags &= ~playerAggressive;
	}

	//  Determine if actor is in aggressive state
	bool isAggressive(void) {
		return (flags & playerAggressive) != 0;
	}

	//  Set the player to be banded
	void setBanded(void) {
		flags |= playerBanded;
	}

	//  Set the player to not be banded
	void clearBanded(void) {
		flags &= ~playerBanded;
	}

	//  Determine if this player actor is banded
	bool isBanded(void) {
		return (flags & playerBanded) != 0;
	}

	//  Resolve the banding state of this actor
	void resolveBanding(void);

	//  Re-evaluate the portrait type for this player actor
	void recalcPortraitType(void);

	//  Return the integer representing the portrait type for this
	//  player actor
	int16 getPortraitType(void) {
		return portraitType;
	}

	// figures out what what ( if any ) changes are required to
	// the charaters vitality
	void recoveryUpdate(void);
	void manaUpdate(void);
	void AttribUpdate(void);
	void stdAttribUpdate(uint8 &stat, uint8 baseStat, int16 index);

	// get this player actor's base stats
	ActorAttributes &getBaseStats(void) {
		return baseStats;
	}

	//  Notify the user of attack if necessary
	void handleAttacked(void);

	//  Simply reset the attack notification flag
	void resetAttackNotification(void) {
		notifiedOfAttack = false;
	}
};

//  Return a pointer to a PlayerActor given it's ID
PlayerActor *getPlayerActorAddress(PlayerActorID id);

//  Return a PlayerActor ID given it's address
PlayerActorID getPlayerActorID(PlayerActor *p);

//  Return a pointer to the center actor
Actor *getCenterActor(void);
//  Return the center actor's object ID
ObjectID getCenterActorID(void);
//  Return the center actor's player actor ID
PlayerActorID getCenterActorPlayerID(void);

//  Set a new center actor based upon the PlayerActor ID
void setCenterActor(PlayerActorID newCenter);

//  Set a new center actor based upon the address of the actor's struct
void setCenterActor(Actor *newCenter);

//  Set a new center actor based upon the address of the PlayerActor
//  struct
void setCenterActor(PlayerActor *newCenter);

//  Get the coordinates of the center actor
TilePoint centerActorCoords(void);

//  Set a player actor's aggression
void setAggression(PlayerActorID player, bool aggression);

//  Set the center actor's aggression
inline void setCenterActorAggression(bool aggression) {
	setAggression(getCenterActorPlayerID(), aggression);
}

//  Determine the state of a player actor's aggression
bool isAggressive(PlayerActorID player);

//  Determine if center actor is aggressive
inline bool isCenterActorAggressive(void) {
	return isAggressive(getCenterActorPlayerID());
}

//  Set a player actor's banding
void setBanded(PlayerActorID player, bool banded);

//  Determine if a player actor is banded
bool isBanded(PlayerActorID player);

//  Globally enable or disable player actor banding
void setBrotherBanding(bool enabled);

//  Adjust the player actors aggression setting based upon their
//  proximity to enemies
void autoAdjustAggression(void);

//  Calculate the portrait for this brother's current state.
void recalcPortraitType(PlayerActorID id);

//  Returns an integer value representing this player actor's portrait
//  state
int16 getPortraitType(PlayerActorID id);

bool actorToPlayerID(Actor *a, PlayerActorID &result);
bool actorIDToPlayerID(ObjectID id, PlayerActorID &result);

void handlePlayerActorDeath(PlayerActorID id);

//  Transport the center actor and the banded brothers who have a path
//  the the center actor
void transportCenterBand(const Location &loc);

void handlePlayerActorAttacked(PlayerActorID id);

void handleEndOfCombat(void);

/* ======================================================================= *
   PlayerActor list management function prototypes
 * ======================================================================= */


//  Initialize the player actor list
void initPlayerActors(void);

void savePlayerActors(Common::OutSaveFile *out);
void loadPlayerActors(Common::InSaveFile *in);

//  Cleanup the player actor list
void cleanupPlayerActors(void);

/* ======================================================================= *
   CenterActor management function prototypes
 * ======================================================================= */

//  Initialize the center actor ID and view object ID
void initCenterActor(void);

void saveCenterActor(Common::OutSaveFile *out);
void loadCenterActor(Common::InSaveFile *in);

//  Do nothing
inline void cleanupCenterActor(void) {}

/* ======================================================================= *
   PlayerActor iteration class
 * ======================================================================= */

class PlayerActorIterator {
protected:
	int16               index;

public:
	PlayerActorIterator(void) {
		index = 0;
	}

	PlayerActor *first(void);
	PlayerActor *next(void);
};

//  Iterates through all player actors that are not dead.

class LivingPlayerActorIterator : public PlayerActorIterator {

public:
	LivingPlayerActorIterator(void) {}

	PlayerActor *first(void);
	PlayerActor *next(void);
};

} // end of namespace Saga2

#endif
