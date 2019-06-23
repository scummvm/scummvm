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
	kNum3DStars = 300,
	kFontSpace = 5,
	kFontIncrement = 1
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

struct GfxCache {
	char name[32];
	union {
		Tile *tileGfx;
		Picture *picGfx;
	};
	uint32 size;
	int16 loaded;

	GfxCache() : name(""), tileGfx(NULL), size(0), loaded(0) {}
};

struct FontInfo {
	uint16		type;		// 0 = mono, 1 = proportional
	uint16		numChars;	// how many characters in font
	uint16		height;		// height of entire font
	uint16		kerning;	// space between chars
	uint16		leading;	// space between lines
};

struct CharInfo {
	uint16 width;	// Character width in pixels
	uint32 offset;	// From the start of the font charInfo chunk
};

class DrawMan {
public:

	DrawMan();
	~DrawMan();

	Graphics::ManagedSurface _globalSurface;

	bool init();
	void fillScreen(uint32 color);
	void updateVideo();

	void setFade(bool fadeIn, bool black, int steps);
	void updateFade();
	bool isFadeActive() { return _fadeInfo.active; }
	bool isFadeStaying() { return _fadeInfo.stayFaded; }
	void turnOffFade() { _fadeInfo.active = _fadeInfo.stayFaded = false; }

	Picture *loadPic(const char *picName);

	Tile *getTile(int index);
	void cacheTileSequence(int index, int count);
	int getTileIndex(const char *name);
	Picture *getPicture(const char *name);

	// Returns: true->Tile, false->Pic
	bool selectGfxType(const char *name);
	Tile *getTileGfx(const char *name, uint32 size);
	Picture *getPicGfx(const char *name, uint32 size);

	int isSky(int skyIndex);
	void setSky(int skyIndex);
	void setup3DStars();
	void setup3DStarsLeft();
	void draw3DStars();
	void draw3DStarsLeft();
	void drawSky();

	int animateTile(int tileIndex);

private:
	int _numTiles;
	TileLookup *_tLookupArray;
	uint16 _skyTiles[kMaxSkies];

	Common::Array<GfxCache *> *_gfxCache;

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

	// Cursor
	int _cursorX, _cursorY;

	// Font Data

	FontInfo _fontHeader;
	Common::Array<CharInfo *> _charInfoBlocks;
	Graphics::Surface _fontSurfaces[256];
	uint16 _fontGfx;
	int _eLeft, _eRight, _eTop, _eBottom;

	bool _systemInit;

};

class Picture {
public:

	Picture();
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

	Tile();
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
