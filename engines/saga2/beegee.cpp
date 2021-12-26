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

void addAuxTheme(Location loc, uint32 lid);
void killAuxTheme(uint32 lid);
void killAllAuxThemes();


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

enum {
	kCheckGameTime = 1000
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

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern volatile int32           gameTime;

extern uint16               rippedRoofID;

extern GameObject *getViewCenterObject();

#if DEBUG
extern bool debugAudioThemes;
#endif

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

#define musicResID(i) MKTAG('X','M','I',i)

inline metaTileNoise getSound(MetaTilePtr mt) {
	int hmm = mt->HeavyMetaMusic();
	return (hmm >= 0 && hmm < kMaxThemes) ? hmm : 0;
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

void initAudioEnvirons() {
}

/* ===================================================================== *
   Looped sound engine
 * ===================================================================== */

void addAuxTheme(Location loc, uint32 lid) {
	for (int i = 0; i < AUXTHEMES; i++) {
		if (!g_vm->_grandMasterFTA->_aats[i].active) {
			g_vm->_grandMasterFTA->_aats[i].l = loc;
			g_vm->_grandMasterFTA->_aats[i].loopID = lid;
			g_vm->_grandMasterFTA->_aats[i].active = true;
			return;
		}
	}
}

void killAuxTheme(uint32 lid) {
	for (int i = 0; i < AUXTHEMES; i++) {
		if (g_vm->_grandMasterFTA->_aats[i].active &&   g_vm->_grandMasterFTA->_aats[i].loopID == lid) {
			g_vm->_grandMasterFTA->_aats[i].active = false;
		}
	}
}

void killAllAuxThemes() {
	for (int i = 0; i < AUXTHEMES; i++) {
		g_vm->_grandMasterFTA->_aats[i].active = false;
	}
}

//-----------------------------------------------------------------------
// Hooks to allow other loops to play

void disableBGLoop(bool s) {
	g_vm->_grandMasterFTA->_playingExternalLoop = s;
}

void enableBGLoop() {
	uint32 cr = g_vm->_grandMasterFTA->_currentTheme;
	g_vm->_grandMasterFTA->_playingExternalLoop = false;
	g_vm->_grandMasterFTA->_currentTheme = 0;
	audioEnvironmentUseSet(cr, g_vm->_grandMasterFTA->_auxTheme, g_vm->_grandMasterFTA->_themeAt);
}

//-----------------------------------------------------------------------
// Main loop selection routine - called from Tile.cpp

void setAreaSound(const TilePoint &) {
	g_vm->_grandMasterFTA->_pct = (g_vm->_grandMasterFTA->_pct + 1) % 8;
	if (g_vm->_grandMasterFTA->_pct == 0) {
		if (!g_vm->_grandMasterFTA->_playingExternalLoop) {
			TilePoint baseCoords = centerActorCoords() >> kTileUVShift;
			TilePoint       mtPos;
			metaTileNoise   loopID = 0;
			uint32 ss = 0;
			Point32 themePos;
			for (int r = 1; r < 5 && loopID == 0 ; r++) {
				TileRegion  regn;
				TilePoint AudVec = TilePoint(AudibilityVector);
				regn.max = baseCoords + ((AudVec * r) << kPlatShift) ; ///kTileUVSize;
				regn.min = baseCoords - ((AudVec * r) << kPlatShift); ///kTileUVSize;
				MetaTileIterator    mIter(g_vm->_currentMapNum, regn);
				int i = 0;
				int j = 0;

				TilePoint       dist = AudVec * r << (kPlatShift + 1);
				dist = dist << 4;
				themePos.x = dist.u;
				themePos.y = dist.v;
				MetaTilePtr     mt = mIter.first(&mtPos);
				while (mt) {
					i++;
					if (getSound(mt)) {
						j++;
						TilePoint thisDist = mtPos - baseCoords;
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
					if (g_vm->_grandMasterFTA->_aats[i].active) {
						Location loc = getCenterActor()->notGetWorldLocation();
						if (g_vm->_grandMasterFTA->_aats[i].l.context == Nothing || loc.context == g_vm->_grandMasterFTA->_aats[i].l.context) {
							TilePoint tp = (g_vm->_grandMasterFTA->_aats[i].l >> kTileUVShift) - baseCoords;
							if (tp.magnitude() < dist.magnitude()) {
								dist = tp;
								loopID = USEAUXTHEME;
								ss = g_vm->_grandMasterFTA->_aats[i].loopID;
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
		} else if (g_vm->_grandMasterFTA->_playingExternalLoop) {
			audioEnvironmentUseSet(g_vm->_grandMasterFTA->_playingExternalLoop, 0, Point16(0, 0)); //themePos << kPlatShift);
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
	if (g_vm->_grandMasterFTA->_currentTheme != (uint16)audioSet || g_vm->_grandMasterFTA->_auxTheme != (uint32)auxID) {
		g_vm->_grandMasterFTA->_currentTheme = audioSet;
		g_vm->_grandMasterFTA->_auxTheme = auxID;
		g_vm->_grandMasterFTA->_themeAt.x = relPos.x;
		g_vm->_grandMasterFTA->_themeAt.y = relPos.y;
		_playLoop(0);
		if (g_vm->_grandMasterFTA->_currentTheme)
			playLoopAt(res, g_vm->_grandMasterFTA->_themeAt);

	} else if (g_vm->_grandMasterFTA->_currentTheme && g_vm->_grandMasterFTA->_themeAt != relPos) {
#if DEBUG
		if (debugAudioThemes) {
			WriteStatusF(9, "Thm: %2.2d (%d,%d) was (%d,%d)   ", audioSet, relPos.x, relPos.y, g_vm->_grandMasterFTA->_themeAt.x, g_vm->_grandMasterFTA->_themeAt.y);
		}
#endif
		g_vm->_grandMasterFTA->_themeAt.x = relPos.x;
		g_vm->_grandMasterFTA->_themeAt.y = relPos.y;
		moveLoop(g_vm->_grandMasterFTA->_themeAt);
	}
}

//-----------------------------------------------------------------------
// Intermittent sound check

void audioEnvironmentCheck() {

	uint32 delta = gameTime - g_vm->_grandMasterFTA->_lastGameTime;
	g_vm->_grandMasterFTA->_lastGameTime = gameTime;
	if (g_vm->_grandMasterFTA->_currentTheme > 0 && g_vm->_grandMasterFTA->_currentTheme <= kAudioTerrainLIMIT) {
		g_vm->_grandMasterFTA->_elapsedGameTime += delta;
		if (g_vm->_grandMasterFTA->_elapsedGameTime > kCheckGameTime) {
			int i;
			g_vm->_grandMasterFTA->_elapsedGameTime = 0;
			const IntermittentAudioRecord &iar = intermittentAudioRecords[g_vm->_grandMasterFTA->_currentTheme];
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
					//Location cal=Location(TilePoint(g_vm->_grandMasterFTA->_themeAt.x,g_vm->_grandMasterFTA->_themeAt.y,0),go->IDParent());
					//playSound(metaNoiseID((g_vm->_grandMasterFTA->_currentTheme*10)+i));
					playSoundAt(metaNoiseID((g_vm->_grandMasterFTA->_currentTheme * 10) + i), g_vm->_grandMasterFTA->_themeAt);
					return;
				} else
					pval -= iar.soundOdds[i];
			}

		}
	} else if (g_vm->_grandMasterFTA->_currentTheme)
		warning("currentTheme out of range: %d", g_vm->_grandMasterFTA->_currentTheme);

}

/* ===================================================================== *
   Music environment engine
 * ===================================================================== */

//-----------------------------------------------------------------------
// Intermittent sound check

void Deejay::select() {
	int choice = 0;
#if DEBUG & 0
	if (1)
		choice = 0;
	else
#endif
		if (_susp)
			choice = 0;
		else if (_enemy >= 0)
			choice = _enemy + 6;
		else if (_aggr)
			choice = 5;
		else if (_ugd)
			choice = 3;
	//else if ( !day )
	//  choice=4;
		else if (_current != 4 && (_current > 2 || _current < 1)) {
			choice = 1 + g_vm->_rnd->getRandomNumber(2);
			if (choice == 3) choice++;
		} else
			choice = _current;

	if (_currentID != musicMapping(choice)) {
		_currentID = musicMapping(choice);
		if (_currentID)
			playMusic(musicResID(_currentID));
		else
			playMusic(0);
	}
	_current = choice;
#if DEBUG
	if (debugAudioThemes) {
		WriteStatusF(8, "Music: %2.2d => %2.2d    ", _current, _currentID);
	}
#endif

}

//-----------------------------------------------------------------------
// Faction enumeration routines

void clearActiveFactions() {
	for (int i = 0; i < kMaxFactions; i++)
		g_vm->_grandMasterFTA->_activeFactions[i] = 0;
}

void incrementActiveFaction(Actor *a) {
	g_vm->_grandMasterFTA->_activeFactions[a->_faction]++;
}

void useActiveFactions() {
	int highCount = 0;
	int highFaction = 0;
	for (int i = 0; i < kMaxFactions; i++) {
		if (g_vm->_grandMasterFTA->_activeFactions[i] > highCount) {
			highCount = g_vm->_grandMasterFTA->_activeFactions[i];
			highFaction = i;
		}
	}
	if (highCount)
		g_vm->_grandMasterFTA->setEnemy(highFaction);
	else
		g_vm->_grandMasterFTA->setEnemy(kNoEnemy);
}

//-----------------------------------------------------------------------
// Aggresssion & day/night control

void audioEnvironmentSetAggression(bool onOff) {
	g_vm->_grandMasterFTA->setAggression(onOff);
}

void audioEnvironmentSetDaytime(bool onOff) {
	g_vm->_grandMasterFTA->setDaytime(onOff);
}

void audioEnvironmentSuspend(bool onOff) {
	g_vm->_grandMasterFTA->setSuspend(onOff);
}

void audioEnvironmentSetWorld(int mapNum) {
	g_vm->_grandMasterFTA->setWorld(mapNum == 1);
}

} // end of namespace Saga2
