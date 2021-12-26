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

#ifndef SAGA2_MAPFEATR_H
#define SAGA2_MAPFEATR_H

#include "saga2/rect.h"
#include "saga2/tcoords.h"

namespace Saga2 {

class gPort;
class gPixelMap;

#define MAX_MAP_FEATURE_NAME_LENGTH 32

/* ===================================================================== *
   Types
 * ===================================================================== */

// ------------------------------------------------------------------------
// Pure virtual base class for map features

class CMapFeature {
	bool        visible;
	int16       world;
	TilePoint   featureCoords;
	char        name[MAX_MAP_FEATURE_NAME_LENGTH];


public:
	CMapFeature(TilePoint where, int16 inWorld, const char *desc);
	virtual ~CMapFeature() {}

	void expose(bool canSee = true) {
		visible = canSee;
	}
	void draw(TileRegion tr, int16 inWorld, TilePoint bc, gPort &tport);
	bool hitCheck(TileRegion vr, int16 inWorld, TilePoint bc, TilePoint cp);
	int16 getWorld() {
		return world;
	}
	int16 getU() {
		return featureCoords.u;
	}
	int16 getV() {
		return featureCoords.v;
	}
	char *getText() {
		return name;
	}

	// The only aspect of different map features is what they look like
	virtual void blit(gPort &tp, int32 x, int32 y) = 0;
	virtual bool isHit(TilePoint disp, TilePoint mouse) = 0;
	virtual void update() = 0;
};


typedef CMapFeature *pCMapFeature;

// ------------------------------------------------------------------------
// class for map features with static icons

class CStaticMapFeature : public CMapFeature {
	int16 color;

public:
	CStaticMapFeature(TilePoint where, int16 inWorld, const char *desc, int16 bColor);
	virtual void blit(gPort &tp, int32 x, int32 y);
	virtual void update() {}
	virtual bool isHit(TilePoint disp, TilePoint mouse);
};


// ------------------------------------------------------------------------
// class for map features with static icons

class CPictureMapFeature : public CMapFeature {
	gPixelMap *pic;

public:
	CPictureMapFeature(TilePoint where, int16 inWorld, char *desc, gPixelMap *pm);
	virtual void blit(gPort &tp, int32 x, int32 y);
	virtual void update() {}
	virtual bool isHit(TilePoint disp, TilePoint mouse) {
		return false;
	}
};


/* ===================================================================== *
   Prototypes
 * ===================================================================== */

void initMapFeatures() ;
void updateMapFeatures(int16 currentWorld);
void drawMapFeatures(TileRegion viewRegion,
                     int16 world,
                     TilePoint baseCoords,
                     gPort &tPort);
void termMapFeatures() ;
char *getMapFeaturesText(TileRegion viewRegion,
                         int16 inWorld,
                         TilePoint baseCoords,
                         Point16 mouseCoords) ;

} // end of namespace Saga2

#endif
