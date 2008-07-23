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

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Prototypes of actor functions
 */

#ifndef TINSEL_ACTOR_H	// prevent multiple includes
#define TINSEL_ACTOR_H


#include "tinsel/dw.h"		// for SCNHANDLE
#include "tinsel/events.h"	// for USER_EVENT
#include "tinsel/palette.h"	// for COLORREF

namespace Tinsel {

struct FREEL;
struct INT_CONTEXT;
struct MACTOR;
struct OBJECT;


/*----------------------------------------------------------------------*/

void RegisterActors(int num);
void FreeActors(void);
void setleadid(int rid);
int LeadId(void);
void StartActors(SCNHANDLE ah, int numActors, bool bRunScript);
void DropActors(void);		// No actor reels running
void DisableActor(int actor);
void EnableActor(int actor);
void Tag_Actor(int ano, SCNHANDLE tagtext, int tp);
void UnTagActor(int ano);
void ReTagActor(int ano);
int TagType(int ano);
bool actorAlive(int ano);
int32 actorMaskType(int ano);
void GetActorPos(int ano, int *x, int *y);
void SetActorPos(int ano, int x, int y);
void GetActorMidTop(int ano, int *x, int *y);
int GetActorLeft(int ano);
int GetActorRight(int ano);
int GetActorTop(int ano);
int GetActorBottom(int ano);
void HideActor(int ano);
bool HideMovingActor(int id, int sf);
void unHideMovingActor(int id);
void restoreMovement(int id);
void storeActorReel(int ano, const FREEL *reel, SCNHANDLE film, OBJECT *pobj, int reelnum, int x, int y);
const FREEL *actorReel(int ano);
SCNHANDLE actorFilm(int ano);

void setActorPlayFilm(int ano, SCNHANDLE film);
SCNHANDLE getActorPlayFilm(int ano);
void setActorTalkFilm(int ano, SCNHANDLE film);
SCNHANDLE getActorTalkFilm(int ano);
void setActorTalking(int ano, bool tf);
bool isActorTalking(int ano);
void setActorLatestFilm(int ano, SCNHANDLE film);
SCNHANDLE getActorLatestFilm(int ano);

void updateActorEsc(int ano, bool escOn, int escEv);
bool actorEsc(int ano);
int actorEev(int ano);
void storeActorPos(int ano, int x, int y);
void storeActorSteps(int ano, int steps);
int getActorSteps(int ano);
void storeActorZpos(int ano, int z);
SCNHANDLE GetActorTag(int ano);
void FirstTaggedActor(void);
int NextTaggedActor(void);
int AsetZPos(OBJECT *pObj, int y, int32 zFactor);
void MAsetZPos(MACTOR *pActor, int y, int32 zFactor);
void actorEvent(int ano, USER_EVENT event, BUTEVENT be);

void storeActorAttr(int ano, int r1, int g1, int b1);
COLORREF getActorTcol(int ano);

void setactorson(void);

void ActorsLife(int id, bool bAlive);

/*----------------------------------------------------------------------*/

struct SAVED_ACTOR {
	short		actorID;
	short		z;
	bool		bAlive;
	SCNHANDLE 	presFilm;	//!< the film that reel belongs to
	short		presRnum;	//!< the present reel number
	short		presX, presY;
};

int SaveActors(SAVED_ACTOR *sActorInfo);

	
void RestoreActorProcess(int id, INT_CONTEXT *pic);


/*----------------------------------------------------------------------*/

} // end of namespace Tinsel

#endif /* TINSEL_ACTOR_H */
