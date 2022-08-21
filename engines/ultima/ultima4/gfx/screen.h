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
 */

#ifndef ULTIMA4_GFX_SCREEN_H
#define ULTIMA4_GFX_SCREEN_H

#include "graphics/screen.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/gfx/scale.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/map/map_tile.h"
#include "ultima/shared/core/file.h"

namespace Ultima {
namespace Ultima4 {

#define SCR_CYCLE_PER_SECOND 4
#define SCR_CYCLE_MAX 16
#define SCREEN_FRAME_TIME 50
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

/*
 * bitmasks for LOS shadows
 */
#define ____H 0x01    // obscured along the horizontal face
#define ___C_ 0x02    // obscured at the center
#define __V__ 0x04    // obscured along the vertical face
#define _N___ 0x80    // start of new raster

#define ___CH 0x03
#define __VCH 0x07
#define __VC_ 0x06

#define _N__H 0x81
#define _N_CH 0x83
#define _NVCH 0x87
#define _NVC_ 0x86
#define _NV__ 0x84


enum LayoutType {
	LAYOUT_STANDARD,
	LAYOUT_GEM,
	LAYOUT_DUNGEONGEM
};

enum MouseCursor {
	MC_DEFAULT,
	MC_WEST,
	MC_NORTH,
	MC_EAST,
	MC_SOUTH
};

class Image;
class Map;
class Tile;
class TileView;
class Coords;
class TileAnimSet;
class ImageInfo;

struct MouseCursorSurface : public Graphics::ManagedSurface {
	Common::Point _hotspot;
};

struct MouseArea {
	int _nPoints;
	struct {
		int x, y;
	} _point[3];
	MouseCursor _cursor;
	Direction _direction;
};

struct Layout {
	Common::String _name;
	LayoutType _type;
	Common::Point _tileShape;
	Common::Rect _viewport;
};

class Screen : public Graphics::Screen {
private:
	MouseCursorSurface *_mouseCursors[5];
	int _currentMouseCursor;

	Std::vector<TileAnimSet *> _tileAnimSets;
	Layout *_gemLayout;
	Common::HashMap<Common::String, int> _dungeonTileChars;
	ImageInfo *_charSetInfo;
	ImageInfo *_gemTilesInfo;

	int _needPrompt;
	Common::Point _cursorPos;
	int _cursorStatus;
	int _cursorEnabled;
	int _los[VIEWPORT_W][VIEWPORT_H];
	//bool _continueScreenRefresh;
	uint32 _priorFrameTime;

public:
	Std::vector<Common::String> _gemLayoutNames;
	Std::vector<Common::String> _filterNames;
	Std::vector<Common::String> _lineOfSightStyles;
	int _currentCycle;
	TileAnimSet *_tileAnims;
private:
	/**
	 * Load the cursors
	 */
	void loadMouseCursors();

	/**
	 * Loads the data for a single cursor from the passed file
	 */
	MouseCursorSurface *loadMouseCursor(Shared::File &src);

	void screenLoadGraphicsFromConf();
	Layout *screenLoadLayoutFromConf(const ConfigElement &conf);

	/**
	 * Draw a tile graphic on the screen.
	 */
	void screenShowGemTile(Layout *layout, Map *map, MapTile &t, bool focus, int x, int y);

	/**
	 * Finds which tiles in the viewport are visible from the avatars
	 * location in the middle. (original DOS algorithm)
	 */
	void screenFindLineOfSight(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]);

	/**
	 * Finds which tiles in the viewport are visible from the avatars
	 * location in the middle. (original DOS algorithm)
	 */
	void screenFindLineOfSightDOS(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]);

	/**
	 * Finds which tiles in the viewport are visible from the avatars
	 * location in the middle.
	 *
	 * A new, more accurate LOS function
	 *
	 * Based somewhat off Andy McFadden's 1994 article,
	 *   "Improvements to a Fast Algorithm for Calculating Shading
	 *   and Visibility in a Two-Dimensional Field"
	 *   -----
	 *   https://fadden.com/tech/fast-los.html
	 *
	 * This function uses a lookup table to get the correct shadowmap,
	 * therefore, the table will need to be updated if the viewport
	 * dimensions increase. Also, the function assumes that the
	 * viewport width and height are odd values and that the player
	 * is always at the center of the screen.
	 */
	void screenFindLineOfSightEnhanced(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]);

	/**
	 * Generates terms a and b for equation "ax + b = y" that defines the
	 * line containing the two given points.  Vertical lines are special
	 * cased to return DBL_MAX for a and the x coordinate as b since they
	 * cannot be represented with the above formula.
	 */
	void screenGetLineTerms(int x1, int y1, int x2, int y2, double *a, double *b);

	/**
	 * Determine if two points are on the same side of a line (or both on
	 * the line).  The line is defined by the terms a and b of the
	 * equation "ax + b = y".
	 */
	int screenPointsOnSameSideOfLine(int x1, int y1, int x2, int y2, double a, double b);

	int screenPointInTriangle(int x, int y, int tx1, int ty1, int tx2, int ty2, int tx3, int ty3);
	Layout *screenGetGemLayout(const Map *map);
public:
	Std::vector<Layout *> _layouts;
	Scaler _filterScaler;

public:
	Screen();
	~Screen();

	void init();

	/**
	 * Reset the screen
	 */
	void clear();

	/**
	 * Sets a given mouse cursor
	 */
	void setMouseCursor(MouseCursor cursor);

	/**
	 * Re-initializes the screen and implements any changes made in settings
	 */
	void screenReInit();
	void screenWait(int numberOfAnimationFrames);

	/**
	 * Draw an image or subimage on the screen.
	 */
	void screenDrawImage(const Common::String &name, int x = 0, int y = 0);
	void screenDrawImageInMapArea(const Common::String &bkgd);

	void screenFrame();
	void screenCycle();
	void screenEraseMapArea();
	void screenEraseTextArea(int x, int y, int width, int height);
	void screenGemUpdate();

	void screenMessage(const char *fmt, ...);
	void screenPrompt();
	void screenRedrawMapArea();
	void screenRedrawTextArea(int x, int y, int width, int height);

	/**
	 * Scroll the text in the message area up one position.
	 */
	void screenScrollMessageArea();

	/**
	 * Do the tremor spell effect where the screen shakes.
	 */
	void screenShake(int iterations);

	/**
	 * Draw a character from the charset onto the screen.
	 */
	void screenShowChar(int chr, int x, int y);
	void screenTextAt(int x, int y, const char *fmt, ...);

	/**
	 * Change the current text color
	 */
	void screenTextColor(int color);
	bool screenTileUpdate(TileView *view, const Coords &coords, bool redraw = true); //Returns true if the screen was affected

	/**
	 * Redraw the screen.  If showmap is set, the normal map is drawn in
	 * the map area.  If blackout is set, the map area is blacked out. If
	 * neither is set, the map area is left untouched.
	 */
	void screenUpdate(TileView *view, bool showmap, bool blackout);
	void screenUpdateCursor();
	void screenUpdateMoons();
	void screenUpdateWind();
	Std::vector<MapTile> screenViewportTile(uint width, uint height, int x, int y, bool &focus);

	void screenShowCursor();
	void screenHideCursor();
	void screenEnableCursor();
	void screenDisableCursor();
	void screenSetCursorPos(int x, int y);

	/**
	 * Determine if the given point is within a mouse area.
	 */
	int screenPointInMouseArea(int x, int y, const MouseArea *area);

	/**
	 * Scale an image up.  The resulting image will be scale * the
	 * original dimensions.  The original image is no longer deleted.
	 * n is the number of tiles in the image; each tile is filtered
	 * separately. filter determines whether or not to filter the
	 * resulting image.
	 */
	Image *screenScale(Image *src, int scale, int n, int filter);

	/**
	 * Scale an image down.  The resulting image will be 1/scale * the
	 * original dimensions.  The original image is no longer deleted.
	 */
	Image *screenScaleDown(Image *src, int scale);
};

extern Screen *g_screen;

extern const Std::vector<Common::String> &screenGetGemLayoutNames();
extern const Std::vector<Common::String> &screenGetFilterNames();
extern const Std::vector<Common::String> &screenGetLineOfSightStyles();

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
