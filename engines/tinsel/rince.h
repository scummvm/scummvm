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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Should really be called "moving actors.h"
 */

#ifndef TINSEL_RINCE_H	// prevent multiple includes
#define TINSEL_RINCE_H

#include "tinsel/anim.h"	// for ANIM
#include "tinsel/scene.h"	// for TFTYPE

namespace Tinsel {

struct OBJECT;
struct PROCESS;

enum NPS {NOT_IN, GOING_UP, GOING_DOWN, LEAVING, ENTERING};

enum IND {NO_PROB, TRY_CENTRE, TRY_CORNER, TRY_NEXTCORNER};

enum MAS {NO_MACTOR, NORM_MACTOR};

enum DIRREEL{ LEFTREEL, RIGHTREEL, FORWARD, AWAY };

enum {
	NUM_MAINSCALES	= 5,
	NUM_AUXSCALES	= 5,
	TOTAL_SCALES	= NUM_MAINSCALES + NUM_AUXSCALES
};

struct MACTOR {

	int     objx;           /* Co-ordinates object  */
	int     objy;

	int     targetX, targetY;
	int     ItargetX, ItargetY;     /* Intermediate destination */
	HPOLYGON hIpath;
	int     UtargetX, UtargetY;     /* Ultimate destination */
	HPOLYGON hUpath;

	HPOLYGON hCpath;

	bool over;
	int	ticket;

	IND	InDifficulty;

/* For use in 'follow nodes' polygons   */
	HPOLYGON hFnpath;
	NPS	npstatus;
	int     line;

	int     Tline;                   // NEW

	bool	TagReelRunning;


   /* Used internally */
	DIRREEL	dirn;		// Current reel
	int	scale;		// Current scale
	int	scount;		// Step count for walking reel synchronisation

    unsigned    fromx;
    unsigned    fromy;

	bool		bMoving;		// Set this to TRUE during a walk

	bool		bNoPath;
	bool		bIgPath;
	bool		walkReel;

	OBJECT		*actorObj;	// Actor's object
	ANIM		actorAnim;	// Actor's animation script

	SCNHANDLE	lastfilm;	// } Used by AlterActor()
	SCNHANDLE	pushedfilm;	// }

	int			actorID;
	int			actorToken;

	SCNHANDLE	WalkReels[TOTAL_SCALES][4];
	SCNHANDLE	StandReels[TOTAL_SCALES][4];
	SCNHANDLE	TalkReels[TOTAL_SCALES][4];

	MAS			MActorState;

	bool		aHidden;
	int			SlowFactor;	// Slow down movement while hidden

	bool		stop;

	/* NOTE: If effect polys can overlap, this needs improving */
	bool		InEffect;

	PROCESS		*pProc;
};
typedef MACTOR *PMACTOR;

//---------------------------------------------------------------------------


void MActorProcessCreate(int X, int Y, int id, MACTOR *pActor);


enum AR_FUNCTION { AR_NORMAL, AR_PUSHREEL, AR_POPREEL, AR_WALKREEL };


MACTOR *GetMover(int ano);
MACTOR *SetMover(int ano);
void KillMActor(MACTOR *pActor);
MACTOR *GetLiveMover(int index);

MAS getMActorState(MACTOR *psActor);

void hideMActor(MACTOR *pActor, int sf);
bool getMActorHideState(MACTOR *pActor);
void unhideMActor(MACTOR *pActor);
void DropMActors(void);
void MoveMActor(MACTOR *pActor, int x, int y);

void GetMActorPosition(MACTOR *pActor, int *aniX, int *aniY);
void GetMActorMidTopPosition(MACTOR *pActor, int *aniX, int *aniY);
int GetMActorLeft(MACTOR *pActor);
int GetMActorRight(MACTOR *pActor);

bool MActorIsInPolygon(MACTOR *pActor, HPOLYGON hPoly);
void AlterMActor(MACTOR *actor, SCNHANDLE film, AR_FUNCTION fn);
DIRREEL GetMActorDirection(MACTOR *pActor);
int GetMActorScale(MACTOR *pActor);
void SetMActorDirection(MACTOR *pActor, DIRREEL dirn);
void SetMActorStanding(MACTOR *actor);
void SetMActorWalkReel(MACTOR *actor, DIRREEL reel, int scale, bool force);

MACTOR *InMActorBlock(MACTOR *pActor, int x, int y);

void RebootMovers(void);

bool IsMAinEffectPoly(int index);
void SetMAinEffectPoly(int index, bool tf);

bool MAmoving(MACTOR *pActor);

int GetActorTicket(MACTOR *pActor);

/*----------------------------------------------------------------------*/

struct SAVED_MOVER {

	MAS	MActorState;
	int	actorID;
	int     objx;
	int     objy;
	SCNHANDLE lastfilm;

	SCNHANDLE	WalkReels[TOTAL_SCALES][4];
	SCNHANDLE	StandReels[TOTAL_SCALES][4];
	SCNHANDLE	TalkReels[TOTAL_SCALES][4];

};

void SaveMovers(SAVED_MOVER *sMoverInfo);
void RestoreAuxScales(SAVED_MOVER *sMoverInfo);

/*----------------------------------------------------------------------*/

/*
* Dodgy bit...
* These functions are now in MAREELS.C, but I can't be bothered to
* create a new header file.
*/
SCNHANDLE GetMactorTalkReel(MACTOR *pAactor, TFTYPE dirn);

void setscalingreels(int actor, int scale, int direction,
		SCNHANDLE left, SCNHANDLE right, SCNHANDLE forward, SCNHANDLE away);
SCNHANDLE ScalingReel(int ano, int scale1, int scale2, DIRREEL reel);
void RebootScalingReels(void);

enum {
	MAGICX	= -101,
	MAGICY	= -102
};

/*----------------------------------------------------------------------*/

} // end of namespace Tinsel

#endif /* TINSEL_RINCE_H */
