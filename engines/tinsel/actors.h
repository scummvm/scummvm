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
 * Prototypes of actor functions
 */

#ifndef TINSEL_ACTOR_H	// prevent multiple includes
#define TINSEL_ACTOR_H


#include "tinsel/dw.h"		// for SCNHANDLE
#include "tinsel/events.h"	// for TINSEL_EVENT
#include "tinsel/palette.h"	// for COLORREF
#include "tinsel/rince.h"	// for PMOVER

namespace Common {
class Serializer;
}

namespace Tinsel {

struct FREEL;
struct INT_CONTEXT;
struct MOVER;
struct OBJECT;
struct T1_ACTOR_STRUC;
struct T2_ACTOR_STRUC;
struct ACTORINFO;
struct Z_POSITIONS;

#define ACTORTAG_KEY 0x1000000
#define OTH_RELATEDACTOR	0x00000fff
#define OTH_RELATIVE		0x00001000
#define OTH_ABSOLUTE		0x00002000
#define MAX_TAGACTORS 10
#define NUM_ZPOSITIONS 200 // Reasonable-sounding number
#define MAX_REELS 6

struct TAGACTORDATA {
	// Copies of compiled data
	int id;
	SCNHANDLE hTagText;   // handle to tag text
	int32 tagPortionV;    // which portion is active
	int32 tagPortionH;    // which portion is active
	SCNHANDLE hActorCode; // The actor's script

	int tagFlags;
	SCNHANDLE hOverrideTag; // Override tag.
};

struct SAVED_ACTOR {
	short actorID;
	short zFactor;
	bool bAlive;
	bool bHidden;
	SCNHANDLE presFilm; ///< the film that reel belongs to
	short presRnum;     ///< the present reel number
	short presPlayX, presPlayY;
};

struct Z_POSITIONS {
	short actor;
	short column;
	int z;
};

typedef SAVED_ACTOR *PSAVED_ACTOR;

/*----------------------------------------------------------------------*/

class Actor {
public:
	Actor();
	virtual ~Actor();

	int GetLeadId();
	SCNHANDLE GetActorTagHandle(int ano);
	void ToggleActor(int ano, bool show);
	SCNHANDLE GetActorCode(int ano);
	SCNHANDLE GetTaggedActorCode(int ano);
	void RunCodeToCompletion(int ano);
	int GetCount() { return _numActors; }

	void RegisterActors(int num);
	void SetLeadId(int rid);
	bool ActorIsGhost(int actor);
	void StartTaggedActors(SCNHANDLE ah, int numActors, bool bRunScript);
	void DropActors();		// No actor reels running

	void DisableActor(int actor);
	void EnableActor(int actor);

	void Tag_Actor(int ano, SCNHANDLE tagtext, int tp);
	void UnTagActor(int ano);
	void ReTagActor(int ano);
	int TagType(int ano);

	bool actorAlive(int ano);
	int32 actorMaskType(int ano);
	void GetActorPos(int ano, int *x, int *y);
	void GetActorMidTop(int ano, int *x, int *y);
	int GetActorLeft(int ano);
	int GetActorRight(int ano);
	int GetActorTop(int ano);
	int GetActorBottom(int ano);
	bool ActorHidden(int ano);
	bool HideMovingActor(int id, int sf);
	void unHideMovingActor(int id);
	void restoreMovement(int id);
	void storeActorReel(int ano, const FREEL *reel, SCNHANDLE hFilm, OBJECT *pobj, int reelnum, int x, int y);
	const FREEL *actorReel(int ano);

	void SetActorPlayFilm(int ano, SCNHANDLE hFilm);
	SCNHANDLE GetActorPlayFilm(int ano);
	void SetActorTalkFilm(int ano, SCNHANDLE hFilm);
	SCNHANDLE GetActorTalkFilm(int ano);

	void SetActorTalking(int ano, bool tf);
	bool ActorIsTalking(int ano);
	void SetActorLatestFilm(int ano, SCNHANDLE hFilm);
	SCNHANDLE GetActorLatestFilm(int ano);

	void UpdateActorEsc(int ano, bool escOn, int escEvent);
	void UpdateActorEsc(int ano, int escEvent);
	bool ActorEsc(int ano);
	int ActorEev(int ano);
	void StoreActorPos(int ano, int x, int y);
	void StoreActorSteps(int ano, int steps);
	int GetActorSteps(int ano);

	void StoreActorZpos(int ano, int z, int column = -1);
	int GetActorZpos(int ano, int column);
	void IncLoopCount(int ano);
	int GetLoopCount(int ano);
	SCNHANDLE GetActorTag(int ano);
	void FirstTaggedActor();
	int NextTaggedActor();
	int NextTaggedActor(int previous);
	int AsetZPos(OBJECT *pObj, int y, int32 zFactor);

	void storeActorAttr(int ano, int r1, int g1, int b1);
	COLORREF GetActorRGB(int ano);
	void SetActorRGB(int ano, COLORREF color);
	void SetActorZfactor(int ano, uint32 zFactor);
	uint32 GetActorZfactor(int ano);
	void SetActorsOn();
	void ActorsLife(int id, bool bAlive);
	void dwEndActor(int ano);

	void SetActorPointedTo(int actor, bool bPointedTo);
	bool ActorIsPointedTo(int actor);
	void SetActorTagWanted(int actor, bool bTagWanted, bool bCursor, SCNHANDLE hOverrideTag);
	bool ActorTagIsWanted(int actor);
	bool InHotSpot(int ano, int curX, int curY);
	int FrontTaggedActor();
	void GetActorTagPos(int actor, int *pTagX, int *pTagY, bool bAbsolute);
	bool IsTaggedActor(int actor);

	void StoreActorPresFilm(int ano, SCNHANDLE hFilm, int x, int y);
	SCNHANDLE GetActorPresFilm(int ano);
	int GetActorFilmNumber(int ano);

	void StoreActorReel(int actor, int column, OBJECT *pObj);
	void NotPlayingReel(int actor, int filmNumber, int column);
	bool ActorReelPlaying(int actor, int column);

	int SaveActors(PSAVED_ACTOR sActorInfo);
	void RestoreActors(int numActors, PSAVED_ACTOR sActorInfo);

	void SaveZpositions(void *zpp);
	void RestoreZpositions(void *zpp);

	void SaveActorZ(byte *saveActorZ);
	void RestoreActorZ(byte *saveActorZ);

	int TaggedActorIndex(int actor);

	void syncAllActorsAlive(Common::Serializer &s);

private:
	void StartActor(const T1_ACTOR_STRUC *as, bool bRunScript);
	void GetActorTagPortion(int ano, unsigned *top, unsigned *bottom, unsigned *left, unsigned *right);

	ACTORINFO *_actorInfo;
	COLORREF _defaultColor; // Text color
	bool _actorsOn;
	int ti;
	TAGACTORDATA _taggedActors[MAX_TAGACTORS];
	int _numTaggedActors;
	uint8 *_zFactors;
	Z_POSITIONS _zPositions[NUM_ZPOSITIONS];
	int _leadActorId; // The lead actor
	int _numActors; // The total number of actors in the game
};

/*----------------------------------------------------------------------*/

void ActorEvent(int ano, TINSEL_EVENT event, PLR_EVENT be);
void ActorEvent(CORO_PARAM, int ano, TINSEL_EVENT tEvent, bool bWait, int myEscape, bool *result = NULL);
void ActorEvent(int ano, TINSEL_EVENT tEvent, bool bWait, int myEscape, bool *result = NULL);
void ShowActor(CORO_PARAM, int ano);
void HideActor(CORO_PARAM, int ano);
void RestoreActorProcess(int id, INT_CONTEXT *pic, bool savegameFlag);

} // End of namespace Tinsel

#endif /* TINSEL_ACTOR_H */
