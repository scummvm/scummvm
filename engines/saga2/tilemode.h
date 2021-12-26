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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TILEMODE_H
#define SAGA2_TILEMODE_H

#include "saga2/idtypes.h"

namespace Saga2 {

//  Function to enable/disable user interface keys
bool enableUIKeys(bool enabled);

void toggleAutoAggression();
bool isAutoAggressionSet();
void toggleAutoWeapon();
bool isAutoWeaponSet();

//  Notify this module of an aggressive act
void logAggressiveAct(ObjectID attackerID, ObjectID attackeeID);

//  Initialize the tile mode state
void initTileModeState();

void saveTileModeState(Common::OutSaveFile *outS);
void loadTileModeState(Common::InSaveFile *in);

//  Cleanup the tile mode state
inline void cleanupTileModeState() { /* do nothing */ }

void TileModeSetup();                        // alloc tile resources
void TileModeCleanup();                      // free tile resources

void noStickyMap();

class TileModeManager {
public:
	CalenderTime *_timeOfLastAggressiveAct;    //  Used to determine the

	TileModeManager();
	~TileModeManager();
};

} // end of namespace Saga2

#endif
