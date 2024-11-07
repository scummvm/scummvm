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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Should really be called "moving actors.h"
 */

#ifndef TINSEL_MOVERS_H	// prevent multiple includes
#define TINSEL_MOVERS_H

#include "tinsel/anim.h"	// for ANIM
#include "tinsel/scene.h"	// for TFTYPE
#include "tinsel/tinsel.h"

namespace Tinsel {

struct OBJECT;

enum NPS {NOT_IN, GOING_UP, GOING_DOWN, LEAVING, ENTERING};

enum IND {NO_PROB, TRY_CENTER, TRY_CORNER, TRY_NEXTCORNER};

enum DIRECTION { LEFTREEL, RIGHTREEL, FORWARD, AWAY };

#define NUM_MAINSCALES	((TinselVersion >= 2) ? 10 : 5)
#define NUM_AUXSCALES	5
#define TOTAL_SCALES	(NUM_MAINSCALES + NUM_AUXSCALES)
#define REQ_MAIN_SCALES		10
#define REQ_TOTAL_SCALES	15

#define BOGUS_BRIGHTNESS -1

enum MOVER_TYPE { MOVER_2D, MOVER_3D };

struct MOVER {
	int		objX, objY;           /* Co-ordinates object  */

	int     targetX, targetY;
	int     ItargetX, ItargetY;     /* Intermediate destination */
	int     UtargetX, UtargetY;     /* Ultimate destination */

	HPOLYGON hIpath;	/* Intermediate path */
	HPOLYGON hUpath;	/* Ultimate path */
	HPOLYGON hCpath;	/* Current path */

	bool over;
	int	walkNumber;

	IND	InDifficulty;

	/* For use in 'follow nodes' polygons   */
	HPOLYGON hFnpath;
	NPS	npstatus;
	int     line;

	int     Tline;                   // NEW

	// TODO: TagReelRunning may be the same as bSpecReel
	bool		bSpecReel;

	/* Used internally */
	DIRECTION	direction;		// Current reel
	int			scale;		// Current scale

	int			stepCount;		// Step count for walking reel synchronisation

	int			walkedFromX, walkedFromY;

	bool		bMoving;		// Set this to TRUE during a walk

	bool		bNoPath;
	bool		bIgPath;
	bool		bWalkReel;

	OBJECT		*actorObj;	// Actor's object
	ANIM		actorAnim;	// Actor's animation script

	SCNHANDLE	hLastFilm;	// } Used by AlterMover()
	SCNHANDLE	hPushedFilm;	// }

	int			actorID;
	int			actorToken;

	SCNHANDLE	walkReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	standReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	talkReels[REQ_TOTAL_SCALES][4];

	bool		bActive;

	int			SlowFactor;	// Slow down movement while hidden

	bool		bStop;

	/* NOTE: If effect polys can overlap, this needs improving */
	bool		bInEffect;

	Common::PROCESS		*pProc;

	// Discworld 2 specific fields
	int32		zOverride;
	bool		bHidden;
	int			brightness;	// Current brightness
	int			startColor;
	int			paletteLength;
	HPOLYGON	hRpath;		// Recent path

	// Noir specific fields
	MOVER_TYPE	type;
	SCNHANDLE	hModelName;
	SCNHANDLE	hTextureName;

	int			posX, posY, posZ;
	int			animSpeed;
	uint		nextIdleAnim;
};

struct MAINIT {
	int	X;
	int	Y;
	MOVER *pMover;
};

//---------------------------------------------------------------------------


void MoverProcessCreate(int X, int Y, int id, MOVER *pMover);


enum AR_FUNCTION { AR_NORMAL, AR_PUSHREEL, AR_POPREEL, AR_WALKREEL };

void StoreMoverPalette(MOVER *pMover, int startColor, int length);

void MoverBrightness(MOVER *pMover, int brightness);

MOVER *GetMover(int ano);
MOVER *RegisterMover(int ano);
void KillMover(MOVER *pMover);
MOVER *GetLiveMover(int index);

bool getMActorState(MOVER *psActor);
int GetMoverId(MOVER *pMover);
void SetMoverZ(MOVER *pMover, int y, uint32 zFactor);
void SetMoverZoverride(MOVER *pMover, uint32 zFactor);

void HideMover(MOVER *pMover, int sf = 0);
bool MoverHidden(MOVER *pMover);
bool MoverIs(MOVER *pMover);
bool MoverIsSWalking(MOVER *pMover);
bool MoverMoving(MOVER *pMover);
int GetWalkNumber(MOVER *pMover);
void UnHideMover(MOVER *pMover);
void DropMovers();
void PositionMover(MOVER *pMover, int x, int y);

void GetMoverPosition(MOVER *pMover, int *aniX, int *aniY);
void GetMoverMidTop(MOVER *pMover, int *aniX, int *aniY);
int GetMoverLeft(MOVER *pMover);
int GetMoverRight(MOVER *pMover);
int GetMoverTop(MOVER *pMover);
int GetMoverBottom(MOVER *pMover);

bool MoverIsInPolygon(MOVER *pMover, HPOLYGON hPoly);
void AlterMover(MOVER *pMover, SCNHANDLE film, AR_FUNCTION fn);
DIRECTION GetMoverDirection(MOVER *pMover);
int GetMoverScale(MOVER *pMover);
void SetMoverDirection(MOVER *pMover, DIRECTION dirn);
void SetMoverStanding(MOVER *pMover);
void SetMoverWalkReel(MOVER *pMover, DIRECTION reel, int scale, bool force);

MOVER *InMoverBlock(MOVER *pMover, int x, int y);

void RebootMovers();

bool IsMAinEffectPoly(int index);
void SetMoverInEffect(int index, bool tf);

void StopMover(MOVER *pMover);

/*----------------------------------------------------------------------*/

struct SAVED_MOVER {

	int		actorID;
	int		objX;
	int		objY;
	SCNHANDLE hLastfilm;

	SCNHANDLE	walkReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	standReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	talkReels[REQ_TOTAL_SCALES][4];

	bool	bActive;
	bool	bHidden;
	int		brightness;
	int		startColor;
	int		paletteLength;
};

void SaveMovers(SAVED_MOVER *sMoverInfo);
void RestoreAuxScales(SAVED_MOVER *sMoverInfo);

MOVER *NextMover(MOVER *pMover);

/*----------------------------------------------------------------------*/

enum {
	MAGICX	= -101,
	MAGICY	= -102
};

/*----------------------------------------------------------------------*/

} // End of namespace Tinsel

#endif /* TINSEL_MOVERS_H */
