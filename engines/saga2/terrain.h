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

#ifndef SAGA2_TERRAIN_H
#define SAGA2_TERRAIN_H

namespace Saga2 {

struct TilePoint;
struct TileRegion;

// these control the rate of drowning damage
// the first two determine the chances of doing damage on a
// each screen refresh
// the third is the damage done
#define drowningDamageOddsYes   (1)
#define drowningDamageOddsNo    (3)
#define drowningDamagePerFrame  (1)

// these control damage taken by falling
// the magnatude of the velocity vector is multiplied by the first
// then divided by the second.
// as it turns out the velocity is a pretty reasonable damage amount

#define fallingDamageMult       (2)
#define fallingDamageDiv        (1)

// these control the rate of lava damage
// the first two determine the chances of doing damage on a
// each screen refresh
// the last two are the damage done (absolute & d6)
#define heatDamageOddsYes       (1)
#define heatDamageOddsNo        (3)
#define heatDamagePerFrame      (1)
#define heatDamageDicePerFrame  (1)

// these control the rate of freezing damage
// the first two determine the chances of doing damage on a
// each screen refresh
// the last two are the damage done (absolute & d6)
#define coldDamageOddsYes       (1)
#define coldDamageOddsNo        (15)
#define coldDamagePerFrame      (1)
#define coldDamageDicePerFrame  (0)

#define terrainDamageOddsYes        (1)
#define terrainDamageOddsNo         (1)
#define terrainDamagePerFrame       (1)
#define terrainDamageDicePerFrame   (2)

/* ===================================================================== *
   Classes referenced by this header
 * ===================================================================== */

class GameObject;
struct StandingTileInfo;

/* ===================================================================== *
   Terrain damage prototypes
 * ===================================================================== */

void drown(GameObject *obj);
void fallingDamage(GameObject *obj, int16 speed);
void lavaDamage(GameObject *obj);
void coldDamage(GameObject *obj);
void terrainDamageSlash(GameObject *obj);
void terrainDamageBash(GameObject *obj);

/* ===================================================================== *
   other Terrain prototypes
 * ===================================================================== */



//  Determine the height of the tile terrain contour
#if 0
int16 tileSlopeHeight(
    const TilePoint &pt,
    GameObject *obj,
    StandingTileInfo *sti = NULL);
#else

//  We've replaced tileSlopeHeight with a new version that takes
//  slightly more generalized parameters, and made an inline function
//  that takes the old-style parameters. Take your pick...

//  New-style version of tileSlopeHeight
int16 tileSlopeHeight(
    const TilePoint     &tileCoords,
    int16               mapNum,
    int                 objectHeight,
    StandingTileInfo    *stiResult = NULL,
    uint8               *platformResult = NULL);

//  Old-style version of tileSlopeHeight()
int16 tileSlopeHeight(
    const TilePoint     &pt,
    GameObject          *obj,
    StandingTileInfo    *stiResult = NULL,
    uint8               *platformResult = NULL);

//  A version which takes the map number explicitly
int16 tileSlopeHeight(
    const TilePoint     &pt,
    int                 mapNum,
    GameObject          *obj,
    StandingTileInfo    *stiResult = NULL,
    uint8               *platformResult = NULL);

#endif

//  Query a tile about its terrain
uint32 tileTerrain(
    int16 mapNum,
    const TilePoint &pt,
    int16 mask,
    int16 minZ,
    int16 maxZ);

//  Determine the terrain for a given volume
uint32 volumeTerrain(int16 mapNum, const TileRegion &volume);

uint32 volumeTerrain(
    int16           mapNum,
    const TilePoint &pos,
    int16           objSection,
    int16           objHeight);

uint32 volumeTerrain(
    int16           mapNum,
    const TilePoint &pos,
    int16           uCross,
    int16           vCross,
    int16           objHeight);

//  Determine the terrain between two points
uint32 lineTerrain(
    int16           mapNum,
    const TilePoint &from,
    const TilePoint &to,
    uint32          opaqueTerrain);

//  return terrain that object is currently interacting with
uint32 objectTerrain(GameObject *obj, StandingTileInfo &sti);

//  return terrain that object is currently interacting with
int16 checkBlocked(
    GameObject *obj,
    int16 mapNum,
    const TilePoint &loc,
    GameObject **blockResultObj = NULL);

//  return terrain that object is currently interacting with
int16 checkBlocked(
    GameObject *obj,
    const TilePoint &loc,
    GameObject **blockResultObj = NULL);

int16 checkWalkable(
    GameObject *obj,
    const TilePoint &loc,
    GameObject **blockResultObj = NULL);

//  return terrain that object is currently interacting with
int16 checkContact(
    GameObject *obj,
    const TilePoint &loc,
    GameObject **blockResultObj = NULL);

} // end of namespace Saga2

#endif
