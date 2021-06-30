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
#include "saga2/music.h"
#include "saga2/player.h"

#include "saga2/audtweak.h"

namespace Saga2 {

#define AUXTHEMES 2
#define USEAUXTHEME 0xe0
/* ===================================================================== *
   Types
 * ===================================================================== */

struct auxAudioTheme {
	bool active;
	Location l;
	soundSegment loopID;

	auxAudioTheme() {
		active = false;
		loopID = 0;
		l = Nowhere;
	}
};

auxAudioTheme aats[AUXTHEMES];

void addAuxTheme(Location loc, soundSegment lid);
void killAuxTheme(soundSegment lid);
void killAllAuxThemes(void);

/* ===================================================================== *
   Constants
 * ===================================================================== */

const TilePoint AudibilityVector = TilePoint(1, 1, 0);
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
static Point32 themeAt;

static int32 lastGameTime = 0;
static int32 elapsedGameTime = 0;

static bool playingExternalLoop = false;

int activeFactions[maxFactions];


TilePoint themeVectors[MaxThemes];



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
			aats[i].l = loc;
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
				regn.max = baseCoords + ((AudibilityVector * r) << kPlatShift) ; ///kTileUVSize;
				regn.min = baseCoords - ((AudibilityVector * r) << kPlatShift); ///kTileUVSize;
				MetaTileIterator    mIter(currentMapNum, regn);
				int i = 0;
				int j = 0;

				TilePoint       dist = AudibilityVector * r << (kPlatShift + 1);
				dist = dist << 4;
				themePos.x = dist.u;
				themePos.y = dist.v;
				MetaTilePtr     mt = mIter.first(&mtPos);
				for (i = 0; i < 16; i++) {
					themeVectors[i] = TilePoint(0, 0, 0);
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
							TilePoint tp = (aats[i].l >> kTileUVShift) - baseCoords;
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
		} else if (!playingExternalLoop) {
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
	if (currentTheme != audioSet || auxTheme != auxID) {
		currentTheme = audioSet;
		auxTheme = auxID;
		themeAt = relPos;
		_playLoop(0);
		if (currentTheme)
			playLoopAt(res, themeAt);

	} else if (currentTheme && themeAt != relPos) {
#if DEBUG
		if (debugAudioThemes) {
			WriteStatusF(9, "Thm: %2.2d (%d,%d) was (%d,%d)   ", audioSet, relPos.x, relPos.y, themeAt.x, themeAt.y);
		}
#endif
		themeAt = relPos;
		moveLoop(themeAt);
	}
}

//-----------------------------------------------------------------------
// Intermittent sound check

void audioEnvironmentCheck(void) {

	uint32 delta = g_system->getMillis() - lastGameTime;
	lastGameTime = g_system->getMillis();
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
