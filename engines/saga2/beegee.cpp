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

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/tile.h"
#include "saga2/beegee.h"
#include "saga2/player.h"

namespace Saga2 {

#define AUXTHEMES 2
#define USEAUXTHEME 0xe0

struct auxAudioTheme {
	bool active;
	StaticLocation l;
	soundSegment loopID;
};

static auxAudioTheme aats[AUXTHEMES] = {
	{false, {Nowhere, 0}, 0},
	{false, {Nowhere, 0}, 0}
};

void addAuxTheme(Location loc, soundSegment lid);
void killAuxTheme(soundSegment lid);
void killAllAuxThemes(void);


enum audioTerrains {
	kAudioTerrainForest      = 1,
	kAudioTerrainSurf,
	kAudioTerrainCity,
	kAudioTerrainCavern,
	kAudioTerrainColdWind,
	kAudioTerrainJungle,
	kAudioTerrainLava,
	kAudioTerrainRiver,
	kAudioTerrainFire,

	kAudioTerrainLIMIT
};

struct IntermittentAudioRecord {
	int noSoundOdds;
	int soundOdds[4];
};

const IntermittentAudioRecord intermittentAudioRecords[kAudioTerrainLIMIT] = {
	//
	// none  1   2   3   4
	//
	{  0, {  0,  0,  0,  0 } },  // no record
	{  1, {  1,  0,  0,  0 } },  // Forest
	{  0, {  0,  0,  0,  0 } },  // Surf
	{  0, {  0,  0,  0,  0 } },  // City
	{  0, {  0,  0,  0,  0 } },  // Cavern
	{  0, {  0,  0,  0,  0 } },  // ColdWind
	{  0, {  0,  0,  0,  0 } },  // Jungle
	{  0, {  0,  0,  0,  0 } },  // Lava
	{  0, {  0,  0,  0,  0 } },  // River
	{  0, {  0,  0,  0,  0 } }   // Fire
};


// Factional music mapping
//   Built in factions
//     0 = suspended
//     1 = daytime 1
//     2 = daytime 2
//     3 = underground
//     4 = nighttime
//     5 = aggressive
//   Faction based
//     6 = faction 0
//     7 = faction 1
//       etc

inline int8 musicMapping(int16 musicChoice) {
	if (musicChoice < 1) {
		return 0;
	} else if (musicChoice < 6) {
		return musicChoice;
	} else if (musicChoice == 6) {
		return 6;
	} else if (musicChoice > 11 && musicChoice < 14) {
		return 7;
	} else if (musicChoice > 8 && musicChoice < 12) {
		return 8;
	} else {
		return 6;
	}
}

const static StaticTilePoint AudibilityVector = { 1, 1, 0 };

const int32 checkGameTime = 1000;

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern volatile int32           gameTime;

extern int16                currentMapNum;          // which map is in use
extern uint16               rippedRoofID;

extern GameObject *getViewCenterObject(void);

#if DEBUG
extern bool debugAudioThemes;
#endif

/* ===================================================================== *
   Locals
 * ===================================================================== */

static Deejay grandMasterFTA;

static uint32 currentTheme = 0;
static uint32 auxTheme = 0;
static StaticPoint32 themeAt = {0, 0};

static int32 lastGameTime = 0;
static int32 elapsedGameTime = 0;

static bool playingExternalLoop = false;

int activeFactions[maxFactions];


static StaticTilePoint themeVectors[MaxThemes] = {
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0}
};



int16 themeCount[MaxThemes];

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

#define musicResID(i) MKTAG('X','M','I',i)

inline metaTileNoise getSound(MetaTilePtr mt) {
	int hmm = mt->HeavyMetaMusic();
	return (hmm >= 0 && hmm < MaxThemes) ? hmm : 0;
}
inline uint32 metaNoiseID(metaTileNoise mtnID) {
	return mtnID ? MKTAG('T', 'E', 'R', mtnID) : 0;
}

void _playLoop(uint32 s);
MetaTileID lookupMetaID(TilePoint coords);

/* ===================================================================== *
   Module code
 * ===================================================================== */

//-----------------------------------------------------------------------
// init

void initAudioEnvirons(void) {
}

/* ===================================================================== *
   Looped sound engine
 * ===================================================================== */

void addAuxTheme(Location loc, soundSegment lid) {
	for (int i = 0; i < AUXTHEMES; i++) {
		if (!aats[i].active) {
			aats[i].l.set(loc, loc.context);
			aats[i].loopID = lid;
			aats[i].active = true;
			return;
		}
	}
}

void killAuxTheme(soundSegment lid) {
	for (int i = 0; i < AUXTHEMES; i++) {
		if (aats[i].active &&   aats[i].loopID == lid) {
			aats[i].active = false;
		}
	}
}

void killAllAuxThemes(void) {
	for (int i = 0; i < AUXTHEMES; i++) {
		aats[i].active = false;
	}
}

//-----------------------------------------------------------------------
// Hooks to allow other loops to play

void disableBGLoop(bool s) {
	playingExternalLoop = s;
}

void enableBGLoop(void) {
	uint32 cr = currentTheme;
	playingExternalLoop = false;
	currentTheme = 0;
	audioEnvironmentUseSet(cr, auxTheme, themeAt);
}

//-----------------------------------------------------------------------
// Main loop selection routine - called from Tile.cpp

static int32 pct = 0;
void setAreaSound(const TilePoint &) {
	pct = (pct + 1) % 8;
	if (pct == 0) {
		if (!playingExternalLoop) {
			TilePoint baseCoords = centerActorCoords() >> kTileUVShift;
			TilePoint       mtPos;
			metaTileNoise   loopID = 0;
			soundSegment ss = 0;
			Point32 themePos;
			for (int r = 1; r < 5 && loopID == 0 ; r++) {
				TileRegion  regn;
				TilePoint AudVec = TilePoint(AudibilityVector);
				regn.max = baseCoords + ((AudVec * r) << kPlatShift) ; ///kTileUVSize;
				regn.min = baseCoords - ((AudVec * r) << kPlatShift); ///kTileUVSize;
				MetaTileIterator    mIter(currentMapNum, regn);
				int i = 0;
				int j = 0;

				TilePoint       dist = AudVec * r << (kPlatShift + 1);
				dist = dist << 4;
				themePos.x = dist.u;
				themePos.y = dist.v;
				MetaTilePtr     mt = mIter.first(&mtPos);
				for (i = 0; i < 16; i++) {
					themeVectors[i].set(0, 0, 0);
				}
				while (mt) {
					i++;
					if (getSound(mt)) {
						j++;
						TilePoint thisDist = mtPos - baseCoords;
						int theme = getSound(mt);
						themeCount[theme]++;
						if (thisDist.magnitude() < dist.magnitude()) {
							dist = thisDist;
							loopID = getSound(mt);
							themePos.x = thisDist.u;
							themePos.y = thisDist.v;
						}
					}
					mt = mIter.next(&mtPos);
				}
				for (i = 0; i < AUXTHEMES; i++) {
					if (aats[i].active) {
						Location loc = getCenterActor()->notGetWorldLocation();
						if (aats[i].l.context == Nothing || loc.context == aats[i].l.context) {
							TilePoint tp = (aats[i].l.tile >> kTileUVShift) - baseCoords;
							if (tp.magnitude() < dist.magnitude()) {
								dist = tp;
								loopID = USEAUXTHEME;
								ss = aats[i].loopID;
								themePos.x = tp.u;
								themePos.y = tp.v;
							}
						}
					}
				}
			}
			if (rippedRoofID) {
				loopID = 0;
			}
			audioEnvironmentUseSet(loopID, ss, themePos << kPlatShift);
		} else if (playingExternalLoop) {
			audioEnvironmentUseSet(playingExternalLoop, 0, Point16(0, 0)); //themePos << kPlatShift);
		}
	}
}

//-----------------------------------------------------------------------
// Implement a particular loop

void audioEnvironmentUseSet(int16 audioSet, int32 auxID, Point32 relPos) {
	uint32          res;
	if (audioSet == USEAUXTHEME)
		res = auxID;
	else if (audioSet > 0 && audioSet <= 16)
		res = metaNoiseID(audioSet);
	else
		res = 0;
	if (currentTheme != (uint16)audioSet || auxTheme != (uint32)auxID) {
		currentTheme = audioSet;
		auxTheme = auxID;
		themeAt.x = relPos.x;
		themeAt.y = relPos.y;
		_playLoop(0);
		if (currentTheme)
			playLoopAt(res, themeAt);

	} else if (currentTheme && themeAt != relPos) {
#if DEBUG
		if (debugAudioThemes) {
			WriteStatusF(9, "Thm: %2.2d (%d,%d) was (%d,%d)   ", audioSet, relPos.x, relPos.y, themeAt.x, themeAt.y);
		}
#endif
		themeAt.x = relPos.x;
		themeAt.y = relPos.y;
		moveLoop(themeAt);
	}
}

//-----------------------------------------------------------------------
// Intermittent sound check

void audioEnvironmentCheck(void) {

	uint32 delta = gameTime - lastGameTime;
	lastGameTime = gameTime;
	if (currentTheme) {
		elapsedGameTime += delta;
		if (elapsedGameTime > checkGameTime) {
			int i;
			elapsedGameTime = 0;
			const IntermittentAudioRecord &iar = intermittentAudioRecords[currentTheme];
			int16 totalProb = iar.noSoundOdds;
			for (i = 0; i < 4; i++)
				totalProb += iar.soundOdds[i];
			if (totalProb <= iar.noSoundOdds)
				return;
			int32 pval = g_vm->_rnd->getRandomNumber(totalProb - 1);
			if (pval < iar.noSoundOdds)
				return;
			pval -= iar.noSoundOdds;
			for (i = 0; i < 4; i++) {
				if (pval < iar.soundOdds[i]) {
					//GameObject *go=getViewCenterObject();
					//Location cal=Location(TilePoint(themeAt.x,themeAt.y,0),go->IDParent());
					//playSound(metaNoiseID((currentTheme*10)+i));
					playSoundAt(metaNoiseID((currentTheme * 10) + i), themeAt);
					return;
				} else
					pval -= iar.soundOdds[i];
			}

		}
	}

}

/* ===================================================================== *
   Music environment engine
 * ===================================================================== */

//-----------------------------------------------------------------------
// Intermittent sound check

int Deejay::current = 0;
int Deejay::currentID = 0;

void Deejay::select(void) {
	int choice = 0;
#if DEBUG & 0
	if (1)
		choice = 0;
	else
#endif
		if (susp)
			choice = 0;
		else if (enemy >= 0)
			choice = enemy + 6;
		else if (aggr)
			choice = 5;
		else if (ugd)
			choice = 3;
	//else if ( !day )
	//  choice=4;
		else if (current != 4 && (current > 2 || current < 1)) {
			choice = 1 + g_vm->_rnd->getRandomNumber(2);
			if (choice == 3) choice++;
		} else
			choice = current;

	if (currentID != musicMapping(choice)) {
		currentID = musicMapping(choice);
		if (currentID)
			playMusic(musicResID(currentID));
		else
			playMusic(0);
	}
	current = choice;
#if DEBUG
	if (debugAudioThemes) {
		WriteStatusF(8, "Music: %2.2d => %2.2d    ", current, currentID);
	}
#endif

}

//-----------------------------------------------------------------------
// Faction enumeration routines

void clearActiveFactions(void) {
	for (int i = 0; i < maxFactions; i++)
		activeFactions[i] = 0;
}

void incrementActiveFaction(Actor *a) {
	activeFactions[a->faction]++;
}

void useActiveFactions(void) {
	int highCount = 0;
	int highFaction = 0;
	for (int i = 0; i < maxFactions; i++) {
		if (activeFactions[i] > highCount) {
			highCount = activeFactions[i];
			highFaction = i;
		}
	}
	if (highCount)
		grandMasterFTA.setEnemy(highFaction);
	else
		grandMasterFTA.setEnemy(NoEnemy);
}

//-----------------------------------------------------------------------
// Aggresssion & day/night control

void audioEnvironmentSetAggression(bool onOff) {
	grandMasterFTA.setAggression(onOff);
}

void audioEnvironmentSetDaytime(bool onOff) {
	grandMasterFTA.setDaytime(onOff);
}

void audioEnvironmentSuspend(bool onOff) {
	grandMasterFTA.setSuspend(onOff);
}

void audioEnvironmentSetWorld(int mapNum) {
	grandMasterFTA.setWorld(mapNum == 1);
}

} // end of namespace Saga2
