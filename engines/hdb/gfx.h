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
	kScreenWidth = 640,
	kScreenHeight = 480,
	kTileWidth = 32,
	kTileHeight = 32,
	kMaxSkies = 10,
	kNum3DStars = 300,
	kFontSpace = 5,
	kFontIncrement = 1,
	kGameFPS = 60,
	kAnimFrameDelay = kGameFPS / 30,
	kAnimSlowFrames = kAnimFrameDelay * 10,
	kAnimMediumFrames = kAnimFrameDelay * 6,
	kAnimFastFrames = kAnimFrameDelay * 2
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

	GfxCache() : tileGfx(NULL), size(0), loaded(0) { name[0] = 0; }
};

struct FontInfo {
	int		type;		// 0 = mono, 1 = proportional
	int		numChars;	// how many characters in font
	int		height;		// height of entire font
	int		kerning;	// space between chars
	int		leading;	// space between lines
};

struct CharInfo {
	int16 width;	// Character width in pixels
	int32 offset;	// From the start of the font charInfo chunk
};

class Gfx {
public:

	Gfx();
	~Gfx();

	Graphics::ManagedSurface _globalSurface;

	bool init();
	void fillScreen(uint32 color);
	void updateVideo();
	void setPointerState(int value);
	void drawPointer();
	void showPointer(bool status) {
		_showCursor = status;
	}
	bool getPointer() {
		return _showCursor;
	}

	void setFade(bool fadeIn, bool black, int steps);
	void updateFade();
	bool isFadeActive() { return _fadeInfo.active; }
	bool isFadeStaying() { return _fadeInfo.stayFaded; }
	void turnOffFade() { _fadeInfo.active = _fadeInfo.stayFaded = false; }
	void turnOffSnow() { _snowInfo.active = false; }

	Picture *loadPic(const char *picName);
	Tile *loadTile(const char *tileName);

	Tile *getTile(int index);
	void cacheTileSequence(int index, int count);
	int getTileIndex(const char *name);
	Picture *getPicture(const char *name);

	void emptyGfxCaches();

	// Returns: true->Tile, false->Pic
	bool selectGfxType(const char *name);
	Tile *getTileGfx(const char *name, int32 size);
	Picture *getPicGfx(const char *name, int32 size);

	int isSky(int skyIndex);
	void setSky(int skyIndex);
	void setup3DStars();
	void setup3DStarsLeft();
	void draw3DStars();
	void draw3DStarsLeft();
	void drawSky();
	void drawSnow();

	int animateTile(int tileIndex);

	// Font Functions

	bool loadFont(const char *string);
	void drawText(const char *string);
	void getDimensions(const char *string, int *pixelsWide, int *lines);
	int stringLength(const char *string);
	void setTextEdges(int left, int right, int top, int bottom);
	void getTextEdges(int *left, int *right, int *top, int *bottom);
	void setKernLead(int kern, int lead);
	void getKernLead(int *kern, int *lead);
	void setCursor(int x, int y);
	void getCursor(int *x, int *y);

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

#define MAX_SNOW                50                              // how many snowflakes onscreen
#define MAX_SNOW_XV             12
	struct {
		bool    active;
		double  x[MAX_SNOW];
		double  y[MAX_SNOW];
		double  yv[MAX_SNOW];
		int     xvindex[MAX_SNOW];
	} _snowInfo;

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
	Picture *_mousePointer[8];	// Gfx for screen pointer (4 Animations)
	int _pointerDisplayable;
	bool _showCursor;

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

	uint _width, _height;

	char *getName() { return _name; }

private:

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

	char *getName() { return _name; }
private:
	char _name[64];

	Graphics::ManagedSurface _surface;
};

} // End of Namespace HDB

#endif // !HDB_DRAW_MANAGER_H
