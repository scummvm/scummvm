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

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/tile.h"
#include "saga2/tileline.h"
#include "saga2/actor.h"

namespace Saga2 {


extern WorldMapData     *mapList;

static int16        prevMapNum;
static StaticTilePoint prevCoords = {(int16)minint16, (int16)minint16, (int16)minint16};
static MetaTilePtr  prevMeta = nullptr;

/* ===================================================================== *
   Terrain damage info
 * ===================================================================== */

void drown(GameObject *obj) {
	if (isActor(obj)) {
		Actor *a = (Actor *) obj;
		if (!a->hasEffect(actorWaterBreathe)) {
			if (g_vm->_rnd->getRandomNumber(drowningDamageOddsYes + drowningDamageOddsNo - 1) > drowningDamageOddsNo - 1) {
				a->acceptDamage(a->thisID(), drowningDamagePerFrame);
			}
		}
	}

}

void lavaDamage(GameObject *obj) {
	if (isActor(obj)) {
		Actor *a = (Actor *) obj;
		if (a->resists(resistHeat))
			return;
	}
	if (g_vm->_rnd->getRandomNumber(heatDamageOddsYes + heatDamageOddsNo - 1) > heatDamageOddsNo - 1) {
		obj->acceptDamage(obj->thisID(), heatDamagePerFrame, kDamageHeat, heatDamageDicePerFrame, 6);
	}
}

void coldDamage(GameObject *obj) {
	if (isActor(obj)) {
		Actor *a = (Actor *) obj;
		if (a->resists(resistCold))
			return;
	}
	if (g_vm->_rnd->getRandomNumber(coldDamageOddsYes + coldDamageOddsNo - 1) > coldDamageOddsNo - 1) {
		obj->acceptDamage(obj->thisID(), coldDamagePerFrame, kDamageCold, coldDamageDicePerFrame, 6);
	}
}

void terrainDamageSlash(GameObject *obj) {
	if (g_vm->_rnd->getRandomNumber(terrainDamageOddsYes + terrainDamageOddsNo - 1) > terrainDamageOddsNo - 1) {
		obj->acceptDamage(obj->thisID(), terrainDamagePerFrame, kDamageSlash, terrainDamageDicePerFrame, 6);
	}
}

void terrainDamageBash(GameObject *obj) {
	if (g_vm->_rnd->getRandomNumber(terrainDamageOddsYes + terrainDamageOddsNo - 1) > terrainDamageOddsNo - 1) {
		obj->acceptDamage(obj->thisID(), terrainDamagePerFrame, kDamageImpact, terrainDamageDicePerFrame, 6);
	}
}

void fallingDamage(GameObject *obj, int16 speed) {
	if (isActor(obj)) {
		Actor *a = (Actor *) obj;
		if (!a->hasEffect(actorSlowFall)) {
			a->acceptDamage(a->thisID(), (MAX(0, speed - 16)*fallingDamageMult) / fallingDamageDiv);
		}
	}

}



/* ===================================================================== *
   Function to get the terrain bits for a single UV location (with mask)
 * ===================================================================== */

uint32 tileTerrain(
    int16 mapNum,
    const TilePoint &pt,
    int16 mask,
    int16 minZ,
    int16 maxZ) {
	WorldMapData    *map = &mapList[mapNum];

	TilePoint       metaCoords = pt >> kPlatShift,
	                origin = metaCoords << kPlatShift,
	                coords = pt - origin;
	MetaTilePtr     metaPtr;
	uint32          terrain = 0;

	//  A simple method for avoiding looking up the metatile again.
	/*  if (    prevMeta
	        &&  prevMapNum == mapNum
	        &&  prevCoords == metaCoords )
	        metaPtr = prevMeta;
	    else
	*/
	{
		//  Look up the metatile on the map.
		metaPtr = prevMeta = map->lookupMeta(metaCoords);
		prevMapNum = mapNum;
		prevCoords.set(metaCoords.u, metaCoords.v, metaCoords.z);
	}

	if (metaPtr == nullptr) return 0L;

	for (int i = 0; i < maxPlatforms; i++) {
		Platform    *p;

		if ((p = metaPtr->fetchPlatform(mapNum, i)) == nullptr)
			continue;

		if (p->flags & plVisible) {
			int16           height;
			TileInfo        *ti;
			int16           trFlags;

			ti =    p->fetchTile(
			            mapNum,
			            coords,
			            origin,
			            height,
			            trFlags);

			if (ti) {
				int16   tileMinZ = height,
				        tileMaxZ = height;
				int32   combinedMask = ti->combinedTerrainMask();

				if (combinedMask & terrainRaised)
					tileMaxZ += ti->attrs.terrainHeight;
				if (combinedMask & terrainWater)
					tileMinZ -= ti->attrs.terrainHeight;

				if (tileMinZ <  maxZ
				        &&  tileMaxZ >= minZ) {
					uint32   terrainResult = 0,
					         tileFgdTerrain = (1 << ti->attrs.fgdTerrain),
					         tileBgdTerrain = (1 << ti->attrs.bgdTerrain);

					//  If only checking the top of raised terrain treat it
					//  as if it were normal terrain.
					if (minZ >= tileMaxZ) {
						if (tileFgdTerrain & terrainSupportingRaised)
							tileFgdTerrain = terrainNormal;
						if (tileBgdTerrain & terrainSupportingRaised)
							tileBgdTerrain = terrainNormal;
					}

					//  If this tile is sensitive to being walked on,
					//  set the "sensitive" flag.
					if (trFlags & trTileSensitive)
						terrainResult |= terrainActive;

					if (mask & ti->attrs.terrainMask)
						terrainResult |= tileFgdTerrain;

					if (mask & ~ti->attrs.terrainMask)
						terrainResult |= tileBgdTerrain;

					//  This prevents actors from walking through
					//  catwalks and other surfaces which have no bottom.

					if ((terrainResult & terrainSolidSurface)
					        &&  height > minZ + kMaxStepHeight) {
						terrainResult |= terrainStone;
					}

					terrain |= terrainResult;
				}
			}
		}
	}
	return terrain;
}

/* ===================================================================== *
   Function to get the terrain infor for a rectilinear volume
 * ===================================================================== */

uint16          uMaxMasks[4] = { 0x0000, 0x000F, 0x00FF, 0x0FFF },
                                 uMinMasks[4] = { 0xFFFF, 0xFFF0, 0xFF00, 0xF000 },
                                         vMaxMasks[4] = { 0x0000, 0x1111, 0x3333, 0x7777 },
                                                 vMinMasks[4] = { 0xFFFF, 0xEEEE, 0xCCCC, 0x8888 };

uint32 volumeTerrain(int16 mapNum, const TileRegion &vol) {
	uint32      terrain = 0;            // accumulated terrain
	TilePoint   tilePt;
	TileRegion  footprint,
	            subPos,
	            volume;

	//  Convert to subtile coords
	volume.min.u = vol.min.u >> kSubTileShift;
	volume.min.v = vol.min.v >> kSubTileShift;
	volume.max.u = (vol.max.u + kSubTileMask) >> kSubTileShift;
	volume.max.v = (vol.max.v + kSubTileMask) >> kSubTileShift;
	volume.min.z = vol.min.z;
	volume.max.z = vol.max.z;

	//  Calculate the footprint of the object (in subtile coords)
	footprint.min.u = volume.min.u >> kTileSubShift;
	footprint.min.v = volume.min.v >> kTileSubShift;
	footprint.max.u = volume.max.u >> kTileSubShift;
	footprint.max.v = volume.max.v >> kTileSubShift;

	//  Calculate which subtiles the region falls upon.
	subPos.min.u = volume.min.u & kSubTileMask;
	subPos.min.v = volume.min.v & kSubTileMask;
	subPos.max.u = volume.max.u & kSubTileMask;
	subPos.max.v = volume.max.v & kSubTileMask;

	tilePt.z = 0;

	for (tilePt.v = footprint.min.v;
	        tilePt.v <= footprint.max.v;
	        tilePt.v++) {
		uint16      vSectionMask = 0xFFFF;

		if (tilePt.v == footprint.min.v)
			vSectionMask &= vMinMasks[subPos.min.v];

		if (tilePt.v == footprint.max.v)
			vSectionMask &= vMaxMasks[subPos.max.v];

		for (tilePt.u = footprint.min.u;
		        tilePt.u <= footprint.max.u;
		        tilePt.u++) {
			uint16  uSectionMask = vSectionMask;

			if (tilePt.u == footprint.min.u)
				uSectionMask &= uMinMasks[subPos.min.u];

			if (tilePt.u == footprint.max.u)
				uSectionMask &= uMaxMasks[subPos.max.u];

			terrain |= tileTerrain(
			               mapNum,
			               tilePt,
			               uSectionMask,
			               volume.min.z,
			               volume.max.z);
		}
	}
	return terrain;
}

uint32 volumeTerrain(
    int16 mapNum,
    const TilePoint &pos,
    int16 objSection,
    int16 objHeight) {
	uint32      terrain = 0;            // accumulated terrain
	TileRegion  volume;

	//  Calculate the volume the object occupies
	volume.min.u = pos.u - objSection;
	volume.min.v = pos.v - objSection;
	volume.max.u = pos.u + objSection;
	volume.max.v = pos.v + objSection;
	volume.min.z = pos.z;
	volume.max.z = pos.z + objHeight;

	terrain = volumeTerrain(mapNum, volume);

	return terrain;
}

uint32 volumeTerrain(
    int16 mapNum,
    const TilePoint &pos,
    int16 uCross,
    int16 vCross,
    int16 objHeight) {
	uint32      terrain = 0;            // accumulated terrain
	TileRegion  volume;

	//  Calculate the volume the object occupies
	volume.min.u = pos.u - uCross;
	volume.min.v = pos.v - vCross;
	volume.max.u = pos.u + uCross;
	volume.max.v = pos.v + vCross;
	volume.min.z = pos.z;
	volume.max.z = pos.z + objHeight;

	if (debugChannelSet(-1, kDebugTiles)) {
		TilePoint minUminV(volume.min.u, volume.min.v, volume.min.z);
		TilePoint maxUminV(volume.max.u, volume.min.v, volume.min.z);
		TilePoint maxUmaxV(volume.max.u, volume.max.v, volume.min.z);
		TilePoint minUmaxV(volume.min.u, volume.max.v, volume.min.z);

		TPLine(minUminV, maxUminV, 7);
		TPLine(maxUminV, maxUmaxV, 7);
		TPLine(maxUmaxV, minUmaxV, 7);
		TPLine(minUmaxV, minUminV, 7);
	}

	terrain = volumeTerrain(mapNum, volume);

	return terrain;
}

/* ===================================================================== *
   Function to get the terrain info for linear area
 * ===================================================================== */

uint16  uMask[4] = { 0x000F, 0x00F0, 0x0F00, 0xF000 },
                     vMask[4] = { 0x1111, 0x2222, 0x4444, 0x8888 };

uint32 lineTerrain(
    int16           mapNum,
    const TilePoint &from,
    const TilePoint &to,
    uint32          opaqueTerrain) {
	uint32      terrain = 0;

	TilePoint   curSubTile,
	            destSubTile,
	            tilePt;

	int8        uStep,
	            vStep;

	uint16      uDiff,
	            vDiff;

	uint16      errorTerm = 0;

	uint16      subTileMask_ = 0;

	int16       tileStartZ,
	            minZ,
	            maxZ;

	int32       curZ,
	            zStep;

	TilePoint   prevPoint = from;
	TilePoint   tempPoint;

	//  Calculate starting subtile coordinates
	curSubTile.u = from.u >> kSubTileShift;
	curSubTile.v = from.v >> kSubTileShift;
	curSubTile.z = tileStartZ = from.z;

	//  Calculate destination subtil coordinates
	destSubTile.u = to.u >> kSubTileShift;
	destSubTile.v = to.v >> kSubTileShift;
	destSubTile.z = to.z;

	tilePt.u = curSubTile.u >> kTileSubShift;
	tilePt.v = curSubTile.v >> kTileSubShift;
	tilePt.z = 0;

	if (destSubTile.u > curSubTile.u) {
		uStep = 1;
		uDiff = destSubTile.u - curSubTile.u;
	} else {
		uStep = -1;
		uDiff = curSubTile.u - destSubTile.u;
	}

	if (destSubTile.v > curSubTile.v) {
		vStep = 1;
		vDiff = destSubTile.v - curSubTile.v;
	} else {
		vStep = -1;
		vDiff = curSubTile.v - destSubTile.v;
	}

	if (uDiff == 0 && vDiff == 0) return 0;

	curZ = (int32)curSubTile.z << 16;
	zStep = ((int32)(destSubTile.z - curSubTile.z) << 16);
	if (zStep > 0) {
		minZ = tileStartZ;
		maxZ = curSubTile.z;
	} else {
		minZ = curSubTile.z;
		maxZ = tileStartZ;
	}

	if (uDiff > vDiff) {
		//  U difference is greater

		zStep /= uDiff;

		for (;
		        curSubTile.u != destSubTile.u;
		        curSubTile.u += uStep) {
			curZ += zStep;

			if ((curSubTile.u >> kTileSubShift) != tilePt.u) {
				curSubTile.z = curZ >> 16;

				terrain |=  tileTerrain(
				                mapNum,
				                tilePt,
				                subTileMask_,
				                minZ,
				                maxZ + 1);
				if (terrain & opaqueTerrain) return terrain;

				tilePt.u = curSubTile.u >> kTileSubShift;
				tileStartZ = curSubTile.z;
				subTileMask_ = 0;
			}

			subTileMask_ |= (uMask[curSubTile.u & kSubTileMask] &
			                vMask[curSubTile.v & kSubTileMask]);

			if (debugChannelSet(-1, kDebugTiles)) {
				tempPoint.u = curSubTile.u << kTileSubShift;
				tempPoint.v = curSubTile.v << kTileSubShift;
				tempPoint.z = curSubTile.z;
				TPLine(prevPoint, tempPoint);
				prevPoint = tempPoint;
			}

			errorTerm += vDiff;
			if (errorTerm >= uDiff) {
				errorTerm -= uDiff;
				curSubTile.v += vStep;

				if ((curSubTile.v >> kTileSubShift) != tilePt.z) {
					curSubTile.z = curZ >> 16;

					terrain |=  tileTerrain(
					                mapNum,
					                tilePt,
					                subTileMask_,
					                minZ,
					                maxZ + 1);
					if (terrain & opaqueTerrain) return terrain;

					tilePt.v = curSubTile.v >> kTileSubShift;
					tileStartZ = curSubTile.z;
					subTileMask_ = 0;
				}

				subTileMask_ |= (uMask[curSubTile.u & kSubTileMask] &
				                vMask[curSubTile.v & kSubTileMask]);

				if (debugChannelSet(-1, kDebugTiles)) {
					tempPoint.u = curSubTile.u << kTileSubShift;
					tempPoint.v = curSubTile.v << kTileSubShift;
					tempPoint.z = curSubTile.z;
					TPLine(prevPoint, tempPoint);
					prevPoint = tempPoint;
				warning("***************************");
				}
			}
		}
	} else {
		//  V difference is greater

		zStep /= vDiff;

		for (;
		        curSubTile.v != destSubTile.v;
		        curSubTile.v += vStep) {
			curZ += zStep;

			if ((curSubTile.v >> kTileSubShift) != tilePt.v) {
				curSubTile.z = curZ >> 16;

				terrain |=  tileTerrain(
				                mapNum,
				                tilePt,
				                subTileMask_,
				                minZ,
				                maxZ + 1);
				if (terrain & opaqueTerrain) return terrain;

				tilePt.v = curSubTile.v >> kTileSubShift;
				tileStartZ = curSubTile.z;
				subTileMask_ = 0;
			}

			subTileMask_ |= (uMask[curSubTile.u & kSubTileMask] &
			                vMask[curSubTile.v & kSubTileMask]);

			if (debugChannelSet(-1, kDebugTiles)) {
				tempPoint.u = curSubTile.u << kTileSubShift;
				tempPoint.v = curSubTile.v << kTileSubShift;
				tempPoint.z = curSubTile.z;
				TPLine(prevPoint, tempPoint);
				prevPoint = tempPoint;
			}

			errorTerm += uDiff;
			if (errorTerm >= vDiff) {
				errorTerm -= vDiff;
				curSubTile.u += uStep;

				if ((curSubTile.u >> kTileSubShift) != tilePt.u) {
					curSubTile.z = curZ >> 16;

					terrain |=  tileTerrain(
					                mapNum,
					                tilePt,
					                subTileMask_,
					                minZ,
					                maxZ + 1);
					if (terrain & opaqueTerrain) return terrain;

					tilePt.u = curSubTile.u >> kTileSubShift;
					tileStartZ = curSubTile.z;
					subTileMask_ = 0;
				}

				subTileMask_ |= (uMask[curSubTile.u & kSubTileMask] &
				                vMask[curSubTile.v & kSubTileMask]);

				if (debugChannelSet(-1, kDebugTiles)) {
					tempPoint.u = curSubTile.u << kTileSubShift;
					tempPoint.v = curSubTile.v << kTileSubShift;
					tempPoint.z = curSubTile.z;
					TPLine(prevPoint, tempPoint);
					prevPoint = tempPoint;
				}
			}
		}
	}

	curSubTile.z = curZ >> 16;

	terrain |=  tileTerrain(
	                mapNum,
	                tilePt,
	                subTileMask_,
	                minZ,
	                maxZ);

	return terrain;
}


/* ===================================================================== *
   Function to return slope information
 * ===================================================================== */

//  This function determines the height of the tile's actual
//  surface, given by the tile slope information. If there are
//  several tiles within the same space, then it uses the highest
//  one who's base is below the character's feet. Tiles which
//  have no supporting surfaces are not considered.
//
//  This routine now also returns a bunch of information about the
//  tile which forms the surface.

int16 tileSlopeHeight(
    const TilePoint     &pt,
    int16               mapNum,
    int                 objectHeight,
    StandingTileInfo    *stiResult,
    uint8               *platformResult) {
	//  Calculate coordinates of tile, metatile, and subtile
	TilePoint       tileCoords = pt >> kTileUVShift,
	                metaCoords = tileCoords >> kPlatShift,
	                origin = metaCoords << kPlatShift,
	                coords = tileCoords - origin,
	                subTile((pt.u >> kSubTileShift) & kSubTileMask,
	                        (pt.v >> kSubTileShift) & kSubTileMask,
	                        0);

	MetaTilePtr     metaPtr;
	StandingTileInfo highestTile,   //  Represents highest tile which is below
	                 //  object's base
	                 lowestTile;     //  Represents lowest tile at tile position
	int16           supportHeight,
	                highestSupportHeight,
	                lowestSupportHeight;
	uint8           i,
	                highestSupportPlatform = 0,
	                lowestSupportPlatform = 0;

	//  Look up the metatile on the map.
	metaPtr = prevMeta = mapList[mapNum].lookupMeta(metaCoords);
	prevMapNum = mapNum;
	prevCoords.set(metaCoords.u, metaCoords.v, metaCoords.z);

	if (metaPtr != nullptr) {
		highestTile.surfaceTile = lowestTile.surfaceTile = nullptr;
		highestSupportHeight = -100;
		lowestSupportHeight = 0x7FFF;

		//  Search each platform until we find a tile which is under
		//  the character.

		for (i = 0; i < maxPlatforms; i++) {
			Platform    *p;

			if ((p = metaPtr->fetchPlatform(mapNum, i)) == nullptr)
				continue;

			if (p->flags & plVisible) {
				TileInfo        *ti;
				StandingTileInfo sti;

				//  Get the tile, and its base height
				ti =    p->fetchTAGInstance(
				            mapNum,
				            coords,
				            origin,
				            sti);

				if (ti) {
					int16   tileBase = sti.surfaceHeight;
					int32 subTileTerrain =
					    ti->attrs.testTerrain(calcSubTileMask(subTile.u,
					                          subTile.v));
					if (subTileTerrain & terrainInsubstantial)
						continue;
					else if (subTileTerrain & terrainSupportingRaised)
						// calculate height of raised surface
						supportHeight = sti.surfaceHeight +
						                ti->attrs.terrainHeight;
					else if (subTileTerrain & terrainWater) {
						// calculate depth of water
						supportHeight = sti.surfaceHeight -
						                ti->attrs.terrainHeight;
						tileBase =  supportHeight;
					} else
						// calculate height of unraised surface
						supportHeight = sti.surfaceHeight +
						                ptHeight(TilePoint(pt.u & kTileUVMask,
						                                   pt.v & kTileUVMask,
						                                   0),
						                         ti->attrs.cornerHeight);

					//  See if the tile is a potential supporting surface
					if (tileBase < pt.z + objectHeight
					        &&  supportHeight >= highestSupportHeight
					        && (ti->combinedTerrainMask() &
					            (terrainSurface | terrainRaised))) {
						highestTile = sti;
						highestSupportHeight = supportHeight;
						highestSupportPlatform = i;
					} else if (highestTile.surfaceTile == nullptr &&
					           supportHeight <= lowestSupportHeight &&
					           (ti->combinedTerrainMask() &
					            (terrainSurface | terrainRaised))) {
						lowestTile = sti;
						lowestSupportHeight = supportHeight;
						lowestSupportPlatform = i;
					}
				}
			}
		}

		if (highestTile.surfaceTile) {
			if (stiResult) *stiResult = highestTile;
			if (platformResult) *platformResult = highestSupportPlatform;
			return highestSupportHeight;
		}
		if (lowestTile.surfaceTile) {
			if (stiResult) *stiResult = lowestTile;
			if (platformResult) *platformResult = lowestSupportPlatform;
			return lowestSupportHeight;
		}
	}

	if (stiResult) {
		stiResult->surfaceTile = nullptr;
		stiResult->surfaceTAG = nullptr;
		stiResult->surfaceHeight = 0;
	}
	if (platformResult) *platformResult = 0;
	return 0;
}

//  Old-style version of tileSlopeHeight()
int16 tileSlopeHeight(
    const TilePoint     &pt,
    GameObject          *obj,
    StandingTileInfo    *stiResult,
    uint8               *platformResult) {
	assert(obj);
	assert(obj->proto());
	return  tileSlopeHeight(
	            pt,
	            obj->getMapNum(),
	            obj->proto()->height,
	            stiResult,
	            platformResult);
}

//  A version of tileSlopeHeight that takes an explicit map number
int16 tileSlopeHeight(
    const TilePoint     &pt,
    int                 mapNum,
    GameObject          *obj,
    StandingTileInfo    *stiResult,
    uint8               *platformResult) {
	assert(obj);
	assert(obj->proto());
	return  tileSlopeHeight(
	            pt,
	            mapNum,
	            obj->proto()->height,
	            stiResult,
	            platformResult);
}

/* ===================================================================== *
   Test functions
 * ===================================================================== */

//  return terrain that object is currently interacting with
uint32 objectTerrain(GameObject *obj, StandingTileInfo &sti) {
	int16           mapNum = obj->getMapNum();
	ProtoObj        *proto = obj->proto();
	uint32          terrain;
	TilePoint       loc = obj->getLocation();

	sti.surfaceTAG = nullptr;

	terrain = volumeTerrain(mapNum,
	                        loc,
	                        proto->crossSection,
	                        proto->height);

	//  If one of the tiles we're standing on is active,
	//  double check to see if we're really standing on it.

	if (terrain & terrainActive) {
		int16       tHeight;

		//  Determine the height of the landscape we're on

		tHeight = tileSlopeHeight(loc, obj, &sti);

		//  If the character is indeed standing ON the landscape
		//  REM: This depends on the nature of the tile I think!!!

		if (sti.surfaceTile == nullptr
		        ||  sti.surfaceTAG == nullptr
		        ||  !(sti.surfaceRef.flags & trTileSensitive)
		        ||  loc.z >= tHeight + 2
		        /* ||   loc.z >= standingTile->attrs.terrainHeight */) {
			terrain &= ~terrainActive;
		}
	}

	return terrain;
}

//  return terrain that object is currently interacting with
int16 checkBlocked(
    GameObject *obj,
    int16 mapNum,
    const TilePoint &loc,
    GameObject **blockResultObj) {
	ProtoObj        *proto = obj->proto();
	uint8           height = proto->height;
	int32           terrain;
	GameObject      *blockObj;
	GameWorld       *world;

	if (blockResultObj) *blockResultObj = nullptr;


	//  check to make sure the actor recognizes terrain
	if (!isActor(obj) || !((Actor *) obj)->hasEffect(actorNoncorporeal)) {
		TilePoint       testLoc = loc;

		testLoc.z = MAX<int16>(loc.z, 8);

		terrain = volumeTerrain(mapNum,
		                        testLoc,
		                        proto->crossSection,
		                        height);

		//  Check for intersection with a wall or obstacle
		if (terrain & terrainRaised) return blockageTerrain;
	}

	//  See if object collided with an object
	world = (GameWorld *)GameObject::objectAddress(mapList[mapNum].worldID);
	blockObj = objectCollision(obj, world, loc);
	if (blockObj) {
		if (blockResultObj) *blockResultObj = blockObj;
		return blockageObject;
	}

	return blockageNone;
}

//  return terrain that object is currently interacting with
int16 checkBlocked(
    GameObject *obj,
    const TilePoint &loc,
    GameObject **blockResultObj) {
	return checkBlocked(obj, obj->getMapNum(), loc, blockResultObj);
}

//  same as checkBlocked() above but includes additional "walking off
//  cliff" check
int16 checkWalkable(
    GameObject *obj,
    const TilePoint &loc,
    GameObject **blockResultObj) {
	int16               result;
	int16               supportHeight;
	StandingTileInfo    sti;

	if ((result = checkBlocked(obj, loc, blockResultObj)) != blockageNone)
		return result;

	supportHeight = tileSlopeHeight(loc, obj, &sti);

	if (supportHeight < loc.z - kMaxStepHeight * 4)
		return blockageTerrain;

	if (sti.surfaceTile != nullptr) {
		int16               subTileU,
		                    subTileV,
		                    mask;

		subTileU = (loc.u & kTileUVMask) >> kSubTileShift;
		subTileV = (loc.v & kTileUVMask) >> kSubTileShift;
		mask = 1 << ((subTileU << kSubTileShift) + subTileV);

		//  If the suporting subtile is funiture consider this blocked
		if (sti.surfaceTile->attrs.testTerrain(mask) & terrainFurniture)
			return blockageTerrain;
	}

	return blockageNone;
}

//  return terrain that object is currently interacting with
int16 checkContact(
    GameObject *obj,
    const TilePoint &loc,
    GameObject **blockResultObj) {
	int16           mapNum = obj->getMapNum();
	ProtoObj        *proto = obj->proto();
	int32           terrain;
	GameObject      *blockObj;
	GameWorld       *world;

	if (blockResultObj) *blockResultObj = nullptr;

	terrain = volumeTerrain(mapNum,
	                        loc,
	                        proto->crossSection,
	                        proto->height);

	//  Check for intersection with a wall or obstacle
	if (terrain & terrainRaised) return blockageTerrain;

	//  Check for intersection with slope of the terrain.
	if (((terrain & terrainSurface)
	        &&  loc.z <= tileSlopeHeight(loc, obj))
	        || (!(terrain & terrainWater)
	            &&  loc.z <= 0))
		return blockageTerrain;

	//  See if object collided with an object
	world = (GameWorld *)GameObject::objectAddress(mapList[mapNum].worldID);
	blockObj = objectCollision(obj, world, loc);
	if (blockObj) {
		if (blockResultObj) *blockResultObj = blockObj;
		return blockageObject;
	}

	return blockageNone;
}

} // end of namespace Saga2
