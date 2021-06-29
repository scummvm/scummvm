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

#ifndef SAGA2_AUDTWEAK_H
#define SAGA2_AUDTWEAK_H

namespace Saga2 {

enum audioTerrains {
	audioTerrainForest      = 1,
	audioTerrainSurf,
	audioTerrainCity,
	audioTerrainCavern,
	audioTerrainColdWind,
	audioTerrainJungle,
	audioTerrainLava,
	audioTerrainRiver,
	audioTerrainFire,

	audioTerrainLIMIT
};

struct IntermittentAudioRecord {
	int noSoundOdds;
	int soundOdds[4];
};

const IntermittentAudioRecord intermittentAudioRecords[audioTerrainLIMIT] = {
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

} // end of namespace Saga2

#endif
