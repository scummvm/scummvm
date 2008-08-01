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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Definition of POLYGON structure and functions in POLYGONS.C
 */

#ifndef TINSEL_POLYGONS_H     // prevent multiple includes
#define TINSEL_POLYGONS_H

#include "tinsel/dw.h"	// for SCNHANDLE
#include "tinsel/scene.h"	// for PPOLY and REEL

namespace Tinsel {


// Polygon Types
enum PTYPE {
	TEST, PATH, EXIT, BLOCKING,
	EFFECT, REFER, TAG, EX_TAG, EX_EXIT, EX_BLOCK
};

// subtype
enum {
	NORMAL = 0,
	NODE   = 1       // For paths
};

// tagState
enum TSTATE {
	TAG_OFF, TAG_ON
};

// pointState
enum PSTATE {
	NO_POINT, NOT_POINTING, POINTING
};


enum {
	NOPOLY = -1
};



/*-------------------------------------------------------------------------*/

bool IsInPolygon(int xt, int yt, HPOLYGON p);
HPOLYGON InPolygon(int xt, int yt, PTYPE type);
void BlockingCorner(HPOLYGON poly, int *x, int *y, int tarx, int tary);
void FindBestPoint(HPOLYGON path, int *x, int *y, int *line);
bool IsAdjacentPath(HPOLYGON path1, HPOLYGON path2);
HPOLYGON getPathOnTheWay(HPOLYGON from, HPOLYGON to);
int NearestEndNode(HPOLYGON path, int x, int y);
int NearEndNode(HPOLYGON spath, HPOLYGON dpath);
int NearestNodeWithin(HPOLYGON npath, int x, int y);
void NearestCorner(int *x, int *y, HPOLYGON spath, HPOLYGON dpath);
bool IsPolyCorner(HPOLYGON hPath, int x, int y);
int GetScale(HPOLYGON path, int y);
void getNpathNode(HPOLYGON npath, int node, int *px, int *py);
void getPolyTagInfo(HPOLYGON p, SCNHANDLE *hTagText, int *tagx, int *tagy);
SCNHANDLE getPolyFilm(HPOLYGON p);
void getPolyNode(HPOLYGON p, int *px, int *py);
SCNHANDLE getPolyScript(HPOLYGON p);
REEL getPolyReelType(HPOLYGON p);
int32 getPolyZfactor(HPOLYGON p);
int numNodes(HPOLYGON pp);
void RebootDeadTags(void);
void DisableBlock(int blockno);
void EnableBlock(int blockno);
void DisableTag(int tagno);
void EnableTag(int tagno);
void DisableExit(int exitno);
void EnableExit(int exitno);
HPOLYGON FirstPathPoly(void);
HPOLYGON GetPolyHandle(int i);
void InitPolygons(SCNHANDLE ph, int numPoly, bool bRestart);
void DropPolygons(void);


void SaveDeadPolys(bool *sdp);
void RestoreDeadPolys(bool *sdp);

/*-------------------------------------------------------------------------*/

PTYPE PolyType(HPOLYGON hp);		// ->type
int PolySubtype(HPOLYGON hp);		// ->subtype
int PolyCentreX(HPOLYGON hp);		// ->pcentrex
int PolyCentreY(HPOLYGON hp);		// ->pcentrey
int PolyCornerX(HPOLYGON hp, int n);	// ->cx[n]
int PolyCornerY(HPOLYGON hp, int n);	// ->cy[n]
PSTATE PolyPointState(HPOLYGON hp);	// ->pointState
TSTATE PolyTagState(HPOLYGON hp);	// ->tagState
SCNHANDLE PolyTagHandle(HPOLYGON hp);	// ->oTagHandle

void SetPolyPointState(HPOLYGON hp, PSTATE ps);	// ->pointState
void SetPolyTagState(HPOLYGON hp, TSTATE ts);	// ->tagState
void SetPolyTagHandle(HPOLYGON hp, SCNHANDLE th);// ->oTagHandle

void MaxPolygons(int maxPolys);

/*-------------------------------------------------------------------------*/

} // end of namespace Tinsel

#endif		/* TINSEL_POLYGONS_H */
