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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

class SaveFileConstructor;
class SaveFileReader;

//  Function to enable/disable user interface keys
bool enableUIKeys(bool enabled);

void toggleAutoAggression(void);
bool isAutoAggressionSet(void);
void toggleAutoWeapon(void);
bool isAutoWeaponSet(void);

//  Notify this module of an aggressive act
void logAggressiveAct(ObjectID attackerID, ObjectID attackeeID);

//  Initialize the tile mode state
void initTileModeState(void);

//  Save the tile mode state to a save file
void saveTileModeState(SaveFileConstructor &saveGame);

//  Load the tile mode state from a save file
void loadTileModeState(SaveFileReader &saveGame);

//  Cleanup the tile mode state
inline void cleanupTileModeState(void) { /* do nothing */ }

void TileModeSetup(void);                        // alloc tile resources
void TileModeCleanup(void);                      // free tile resources

void noStickyMap(void);

} // end of namespace Saga2

#endif
