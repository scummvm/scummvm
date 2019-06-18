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
 */

#ifndef HDB_DRAW_MANAGER_H
#define HDB_DRAW_MANAGER_H

#include "graphics/managed_surface.h"

#include "hdb/hdb.h"

namespace HDB {

enum {
	kTileWidth = 32,
	kTileHeight = 32,
	kMaxSkies = 10,
	kNum3DStars = 300
};

class Tile;
class Picture;

struct TileLookup {
	const char *filename;
	Tile *tData;
	uint16 skyIndex;
	uint16 animIndex;

	TileLookup() : filename(NULL), tData(NULL), skyIndex(0), animIndex(0) {}
};


class DrawMan {
public:

	DrawMan();
	~DrawMan();

	Graphics::ManagedSurface _globalSurface;

	bool init();
	void fillScreen(uint32 color);
	void setFade(bool fadeIn, bool black, int steps);

	Tile *getTile(int index);
	int getTileIndex(const char *name);
	Picture *getPicture(const char *name);

	int isSky(int skyIndex);
	void setSky(int skyIndex);
	void setup3DStars();
	void setup3DStarsLeft();
	void draw3DStars();
	void draw3DStarsLeft();
	void drawSky();

private:
	int _numTiles;
	TileLookup *_tLookupArray;
	uint16 _skyTiles[kMaxSkies];

	int _currentSky;	// 0 if no Sky, 1+ for which Sky to use
	struct {
		bool active;
		bool stayFaded;
		bool isBlack;
		int speed;
		bool isFadeIn;

		int curStep;
	} _fadeInfo;

	struct {
		int x, y, speed;
		uint16 color;
	} _stars3D[kNum3DStars];

	struct {
		double x, y, speed;
		uint16 color;
	} _stars3DSlow[kNum3DStars];

	int _tileSkyStars; // Index of sky_stars tile
	int _tileSkyStarsLeft; // Left-scrolling stars, slow
	int _tileSkyClouds; // Index of sky_stars tile
	Picture *_starField[4];
	Picture *_skyClouds;

	bool _systemInit;

};

class Picture {
public:

	~Picture();

	Graphics::Surface load(Common::SeekableReadStream *stream);
	void draw(int x, int y);
	void drawMasked(int x, int y);

private:

	uint _width, _height;
	char _name[64];

	Graphics::ManagedSurface _surface;

};

class Tile {
public:

	~Tile();

	Graphics::Surface load(Common::SeekableReadStream *stream);
	void draw(int x, int y);
	void drawMasked(int x, int y);

	uint32 _flags;

private:
	char _name[64];

	Graphics::ManagedSurface _surface;
};

} // End of Namespace HDB

#endif // !HDB_DRAW_MANAGER_H
