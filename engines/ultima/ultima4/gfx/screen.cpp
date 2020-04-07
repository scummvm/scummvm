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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/intro.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/gfx/textcolor.h"
#include "ultima/ultima4/map/dungeonview.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/tileanim.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/map/tileview.h"
#include "ultima/ultima4/map/annotation.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"

namespace Ultima {
namespace Ultima4 {

#define CURSOR_SIZE 20

Screen *g_screen;

Screen::Screen() : _filterScaler(nullptr), _currentCursor(-1) {
	g_screen = this;
	Common::fill(&_cursors[0], &_cursors[5], (Cursor *)nullptr);

	Graphics::PixelFormat SCREEN_FORMAT(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Common::Point size(SCREEN_WIDTH * settings._scale, SCREEN_HEIGHT * settings._scale);
	initGraphics(size.x, size.y, &SCREEN_FORMAT);

	create(size.x, size.y, SCREEN_FORMAT);
	loadCursors();
}

Screen::~Screen() {
	clear();
	g_screen = nullptr;

	// Delete cursors
	for (int idx = 0; idx < 5; ++idx)
		delete _cursors[idx];
}

void Screen::init() {
}

void Screen::clear() {
	Std::vector<Layout *>::const_iterator i;
	for (i = _layouts.begin(); i != _layouts.end(); ++i)
		delete(*i);
	_layouts.clear();

	ImageMgr::destroy();
}

void Screen::loadCursors() {
	// enable or disable the mouse cursor
	if (settings._mouseOptions._enabled) {
		g_system->showMouse(true);

		Shared::File cursorsFile("data/graphics/cursors.txt");

		for (int idx = 0; idx < 5; ++idx)
			_cursors[idx] = loadCursor(cursorsFile);

	} else {
		g_system->showMouse(false);
	}

	_filterScaler = scalerGet(settings._filter);
	if (!_filterScaler)
		errorFatal("%s is not a valid filter", settings._filter.c_str());
}

void Screen::setCursor(MouseCursor cursor) {
	const Cursor *c = _cursors[cursor];
	const uint TRANSPARENT = g_screen->format.RGBToColor(0x80, 0x80, 0x80);

	if (c && cursor != _currentCursor) {
		_currentCursor = cursor;
		CursorMan.replaceCursor(c->getPixels(), CURSOR_SIZE, CURSOR_SIZE,
			c->_hotspot.x, c->_hotspot.y, TRANSPARENT, false, &g_screen->format);
	}
}


#define CURSOR_SIZE 20

Cursor *Screen::loadCursor(Shared::File &src) {
	uint row, col, endCol, pixel;
	int hotX, hotY;
	Common::String line;
	byte *destP;
	const uint WHITE = g_screen->format.RGBToColor(0xff, 0xff, 0xff);
	const uint BLACK = g_screen->format.RGBToColor(0, 0, 0);
	const uint TRANSPARENT = g_screen->format.RGBToColor(0x80, 0x80, 0x80);
	int bpp = g_screen->format.bytesPerPixel;
	assert(bpp >= 2);

	Cursor *c = new Cursor();
	c->create(CURSOR_SIZE, CURSOR_SIZE, g_screen->format);
	c->clear(TRANSPARENT);

	for (row = 0; row < CURSOR_SIZE; row++) {
		line = src.readLine();
		destP = (byte *)c->getBasePtr(0, row);
		endCol = MIN(line.size(), (uint)CURSOR_SIZE);

		for (col = 0; col < endCol; ++col, destP += bpp) {
			pixel = TRANSPARENT;
			if (line[col] == 'X')
				pixel = BLACK;
			else if (line[col] == '.')
				pixel = WHITE;

			if (bpp == 2)
				*((uint16 *)destP) = pixel;
			else
				*((uint32 *)destP) = pixel;
		}
	}

	// Read in the hotspot position
	line = src.readLine();
	sscanf(line.c_str(), "%d,%d", &hotX, &hotY);
	c->_hotspot.x = hotX;
	c->_hotspot.y = hotY;

	return c;
}

/*-------------------------------------------------------------------*/

#define DBL_MAX 1e99

void screenLoadGraphicsFromConf(void);
Layout *screenLoadLayoutFromConf(const ConfigElement &conf);
void screenShowGemTile(Layout *layout, Map *map, MapTile &t, bool focus, int x, int y);

Std::vector<TileAnimSet *> tileanimSets;
Std::vector<Common::String> gemLayoutNames;
Std::vector<Common::String> filterNames;
Std::vector<Common::String> lineOfSightStyles;
Layout *gemlayout = NULL;
Std::map<Common::String, int> dungeonTileChars;
TileAnimSet *tileanims = NULL;
ImageInfo *charsetInfo = NULL;
ImageInfo *gemTilesInfo = NULL;

void screenFindLineOfSight(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]);
void screenFindLineOfSightDOS(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]);
void screenFindLineOfSightEnhanced(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]);

int screenNeedPrompt = 1;
int screenCurrentCycle = 0;
int screenCursorX = 0;
int screenCursorY = 0;
int screenCursorStatus = 0;
int screenCursorEnabled = 1;
int screenLos[VIEWPORT_W][VIEWPORT_H];

static const int BufferSize = 1024;

void screenInit() {
	filterNames.clear();
	filterNames.push_back("point");
	filterNames.push_back("2xBi");
	filterNames.push_back("2xSaI");
	filterNames.push_back("Scale2x");

	lineOfSightStyles.clear();
	lineOfSightStyles.push_back("DOS");
	lineOfSightStyles.push_back("Enhanced");

	charsetInfo = NULL;
	gemTilesInfo = NULL;

	screenLoadGraphicsFromConf();

	debug(1, "using %s scaler\n", settings._filter.c_str());

	/* if we can't use vga, reset to default:ega */
	if (!u4isUpgradeAvailable() && settings._videoType == "VGA")
		settings._videoType = "EGA";


	KeyHandler::setKeyRepeat(settings._keydelay, settings._keyinterval);

	/* find the tile animations for our tileset */
	tileanims = NULL;
	for (Std::vector<TileAnimSet *>::const_iterator i = tileanimSets.begin(); i != tileanimSets.end(); i++) {
		TileAnimSet *set = *i;
		if (set->_name == settings._videoType)
			tileanims = set;
	}
	if (!tileanims)
		errorFatal("unable to find tile animations for \"%s\" video mode in graphics.xml", settings._videoType.c_str());

	dungeonTileChars.clear();
	dungeonTileChars["brick_floor"] = CHARSET_FLOOR;
	dungeonTileChars["up_ladder"] = CHARSET_LADDER_UP;
	dungeonTileChars["down_ladder"] = CHARSET_LADDER_DOWN;
	dungeonTileChars["up_down_ladder"] = CHARSET_LADDER_UPDOWN;
	dungeonTileChars["chest"] = '$';
	dungeonTileChars["ceiling_hole"] = CHARSET_FLOOR;
	dungeonTileChars["floor_hole"] = CHARSET_FLOOR;
	dungeonTileChars["magic_orb"] = CHARSET_ORB;
	dungeonTileChars["ceiling_hole"] = 'T';
	dungeonTileChars["floor_hole"] = 'T';
	dungeonTileChars["fountain"] = 'F';
	dungeonTileChars["secret_door"] = CHARSET_SDOOR;
	dungeonTileChars["brick_wall"] = CHARSET_WALL;
	dungeonTileChars["dungeon_door"] = CHARSET_ROOM;
	dungeonTileChars["avatar"] = CHARSET_REDDOT;
	dungeonTileChars["dungeon_room"] = CHARSET_ROOM;
	dungeonTileChars["dungeon_altar"] = CHARSET_ANKH;
	dungeonTileChars["energy_field"] = '^';
	dungeonTileChars["fire_field"] = '^';
	dungeonTileChars["poison_field"] = '^';
	dungeonTileChars["sleep_field"] = '^';
}

/**
 * Re-initializes the screen and implements any changes made in settings
 */
void screenReInit() {
	intro->deleteIntro();       /* delete intro stuff */
	Tileset::unloadAllImages(); /* unload tilesets, which will be reloaded lazily as needed */
	ImageMgr::destroy();
	tileanims = NULL;
	g_screen->clear();
	g_screen->init();           // re-init screen stuff (loading new backgrounds, etc.)
	intro->init();    /* re-fix the backgrounds loaded and scale images, etc. */
}

void screenTextAt(int x, int y, const char *fmt, ...) {
	char buffer[BufferSize];
	unsigned int i;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, BufferSize, fmt, args);
	va_end(args);

	for (i = 0; i < strlen(buffer); i++)
		screenShowChar(buffer[i], x + i, y);
}

void screenPrompt() {
	if (screenNeedPrompt && screenCursorEnabled && g_context->col == 0) {
		screenMessage("%c", CHARSET_PROMPT);
		screenNeedPrompt = 0;
	}
}

void screenMessage(const char *fmt, ...) {
#ifdef IOS
	static bool recursed = false;
#endif

	if (!g_context)
		return; //Because some cases (like the intro) don't have the context initiated.
	char buffer[BufferSize];
	unsigned int i;
	int wordlen;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, BufferSize, fmt, args);
	va_end(args);
#ifdef IOS
	if (recursed)
		recursed = false;
	else
		U4IOS::drawMessageOnLabel(Common::String(buffer, 1024));
#endif

	screenHideCursor();

	/* scroll the message area, if necessary */
	if (g_context->_line == 12) {
		screenScrollMessageArea();
		g_context->_line--;
	}

	for (i = 0; i < strlen(buffer); i++) {
		// include whitespace and color-change codes
		wordlen = strcspn(buffer + i, " \b\t\n\024\025\026\027\030\031");

		/* backspace */
		if (buffer[i] == '\b') {
			g_context->col--;
			if (g_context->col < 0) {
				g_context->col += 16;
				g_context->_line--;
			}
			continue;
		}

		/* color-change codes */
		switch (buffer[i]) {
		case FG_GREY:
		case FG_BLUE:
		case FG_PURPLE:
		case FG_GREEN:
		case FG_RED:
		case FG_YELLOW:
		case FG_WHITE:
			screenTextColor(buffer[i]);
			continue;
		}

		/* check for word wrap */
		if ((g_context->col + wordlen > 16) || buffer[i] == '\n' || g_context->col == 16) {
			if (buffer[i] == '\n' || buffer[i] == ' ')
				i++;
			g_context->_line++;
			g_context->col = 0;
#ifdef IOS
			recursed = true;
#endif
			screenMessage("%s", buffer + i);
			return;
		}

		/* code for move cursor right */
		if (buffer[i] == 0x12) {
			g_context->col++;
			continue;
		}
		/* don't show a space in column 1.  Helps with Hawkwind. */
		if (buffer[i] == ' ' && g_context->col == 0)
			continue;
		screenShowChar(buffer[i], TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
		g_context->col++;
	}

	screenSetCursorPos(TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
	screenShowCursor();

	screenNeedPrompt = 1;
}

const Std::vector<Common::String> &screenGetFilterNames() {
	return filterNames;
}

const Std::vector<Common::String> &screenGetGemLayoutNames() {
	return gemLayoutNames;
}

const Std::vector<Common::String> &screenGetLineOfSightStyles() {
	return lineOfSightStyles;
}

void screenLoadGraphicsFromConf() {
	const Config *config = Config::getInstance();

	Std::vector<ConfigElement> graphicsConf = config->getElement("graphics").getChildren();
	for (Std::vector<ConfigElement>::iterator conf = graphicsConf.begin(); conf != graphicsConf.end(); conf++) {

		if (conf->getName() == "layout")
			g_screen->_layouts.push_back(screenLoadLayoutFromConf(*conf));
		else if (conf->getName() == "tileanimset")
			tileanimSets.push_back(new TileAnimSet(*conf));
	}

	gemLayoutNames.clear();
	Std::vector<Layout *>::const_iterator i;
	for (i = g_screen->_layouts.begin(); i != g_screen->_layouts.end(); i++) {
		Layout *layout = *i;
		if (layout->_type == LAYOUT_GEM) {
			gemLayoutNames.push_back(layout->_name);
		}
	}

	/*
	 * Find gem layout to use.
	 */
	for (i = g_screen->_layouts.begin(); i != g_screen->_layouts.end(); i++) {
		Layout *layout = *i;

		if (layout->_type == LAYOUT_GEM && layout->_name == settings._gemLayout) {
			gemlayout = layout;
			break;
		}
	}
	if (!gemlayout)
		errorFatal("no gem layout named %s found!\n", settings._gemLayout.c_str());
}

Layout *screenLoadLayoutFromConf(const ConfigElement &conf) {
	Layout *layout;
	static const char *typeEnumStrings[] = { "standard", "gem", "dungeon_gem", NULL };

	layout = new Layout();
	layout->_name = conf.getString("name");
	layout->_type = static_cast<LayoutType>(conf.getEnum("type", typeEnumStrings));

	Std::vector<ConfigElement> children = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "tileshape") {
			layout->_tileShape.x = i->getInt("width");
			layout->_tileShape.y = i->getInt("height");
		} else if (i->getName() == "viewport") {
			layout->_viewport.left = i->getInt("x");
			layout->_viewport.top = i->getInt("y");
			layout->_viewport.setWidth(i->getInt("width"));
			layout->_viewport.setHeight(i->getInt("height"));
		}
	}

	return layout;
}



Std::vector<MapTile> screenViewportTile(unsigned int width, unsigned int height, int x, int y, bool &focus) {
	MapCoords center = g_context->_location->_coords;
	static MapTile grass = g_context->_location->_map->_tileset->getByName("grass")->getId();

	if (g_context->_location->_map->_width <= width &&
	        g_context->_location->_map->_height <= height) {
		center.x = g_context->_location->_map->_width / 2;
		center.y = g_context->_location->_map->_height / 2;
	}

	MapCoords tc = center;

	tc.x += x - (width / 2);
	tc.y += y - (height / 2);

	/* Wrap the location if we can */
	tc.wrap(g_context->_location->_map);

	/* off the edge of the map: pad with grass tiles */
	if (MAP_IS_OOB(g_context->_location->_map, tc)) {
		focus = false;
		Std::vector<MapTile> result;
		result.push_back(grass);
		return result;
	}

	return g_context->_location->tilesAt(tc, focus);
}

bool screenTileUpdate(TileView *view, const Coords &coords, bool redraw) {
	if (g_context->_location->_map->_flags & FIRST_PERSON)
		return false;

	// Get the tiles
	bool focus;
	MapCoords mc(coords);
	mc.wrap(g_context->_location->_map);
	Std::vector<MapTile> tiles = g_context->_location->tilesAt(mc, focus);

	// Get the screen coordinates
	int x = coords.x;
	int y = coords.y;

	if (g_context->_location->_map->_width > VIEWPORT_W || g_context->_location->_map->_height > VIEWPORT_H) {
		//Center the coordinates to the viewport if you're on centered-view map.
		x = x - g_context->_location->_coords.x + VIEWPORT_W / 2;
		y = y - g_context->_location->_coords.y + VIEWPORT_H / 2;
	}

	// Draw if it is on screen
	if (x >= 0 && y >= 0 && x < VIEWPORT_W && y < VIEWPORT_H && screenLos[x][y]) {
		view->drawTile(tiles, focus, x, y);

		if (redraw) {
			//screenRedrawMapArea();
		}
		return true;
	}
	return false;
}

/**
 * Redraw the screen.  If showmap is set, the normal map is drawn in
 * the map area.  If blackout is set, the map area is blacked out. If
 * neither is set, the map area is left untouched.
 */
void screenUpdate(TileView *view, bool showmap, bool blackout) {
	ASSERT(g_context != NULL, "context has not yet been initialized");

	if (blackout) {
		screenEraseMapArea();
	} else if (g_context->_location->_map->_flags & FIRST_PERSON) {
		DungeonViewer.display(g_context, view);
		screenRedrawMapArea();
	} else if (showmap) {
		static MapTile black = g_context->_location->_map->_tileset->getByName("black")->getId();
		//static MapTile avatar = g_context->_location->_map->_tileset->getByName("avatar")->getId();

		int x, y;

		Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H];
		bool viewportFocus[VIEWPORT_W][VIEWPORT_H];

		for (y = 0; y < VIEWPORT_H; y++) {
			for (x = 0; x < VIEWPORT_W; x++) {
				viewportTiles[x][y] = screenViewportTile(VIEWPORT_W, VIEWPORT_H, x, y, viewportFocus[x][y]);
			}
		}

		screenFindLineOfSight(viewportTiles);

		for (y = 0; y < VIEWPORT_H; y++) {
			for (x = 0; x < VIEWPORT_W; x++) {
				if (screenLos[x][y]) {
					view->drawTile(viewportTiles[x][y], viewportFocus[x][y], x, y);
				} else
					view->drawTile(black, false, x, y);
			}
		}
		screenRedrawMapArea();
	}

	screenUpdateCursor();
	screenUpdateMoons();
	screenUpdateWind();
}

/**
 * Draw an image or subimage on the screen.
 */
void screenDrawImage(const Common::String &name, int x, int y) {
	ImageInfo *info = imageMgr->get(name);
	if (info) {
		info->_image->alphaOn();
		info->_image->draw(x, y);
		return;
	}

	SubImage *subimage = imageMgr->getSubImage(name);
	if (subimage)
		info = imageMgr->get(subimage->_srcImageName);

	if (info) {
		info->_image->alphaOn();

		if (info) {
			info->_image->drawSubRect(x, y,
			                          subimage->x * (settings._scale / info->_prescale),
			                          subimage->y * (settings._scale / info->_prescale),
			                          subimage->width * (settings._scale / info->_prescale),
			                          subimage->height * (settings._scale / info->_prescale));
			return;
		}
	}
	errorFatal("ERROR 1006: Unable to load the image \"%s\".\t\n\nIs %s installed?\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/", name.c_str(), settings._game.c_str());
}

void screenDrawImageInMapArea(const Common::String &name) {
	ImageInfo *info;

	info = imageMgr->get(name);
	if (!info)
		errorFatal("ERROR 1004: Unable to load data files.\t\n\nIs %s installed?\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/", settings._game.c_str());

	info->_image->drawSubRect(BORDER_WIDTH * settings._scale, BORDER_HEIGHT * settings._scale,
	                          BORDER_WIDTH * settings._scale, BORDER_HEIGHT * settings._scale,
	                          VIEWPORT_W * TILE_WIDTH * settings._scale,
	                          VIEWPORT_H * TILE_HEIGHT * settings._scale);
}


/**
 * Change the current text color
 */
void screenTextColor(int color) {
	if (charsetInfo == NULL) {
		charsetInfo = imageMgr->get(BKGD_CHARSET);
		if (!charsetInfo)
			errorFatal("ERROR 1003: Unable to load the \"%s\" data file.\t\n\nIs %s installed?\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/", BKGD_CHARSET, settings._game.c_str());
	}

	if (!settings._enhancements || !settings._enhancementsOptions._textColorization) {
		return;
	}

	switch (color) {
	case FG_GREY:
	case FG_BLUE:
	case FG_PURPLE:
	case FG_GREEN:
	case FG_RED:
	case FG_YELLOW:
	case FG_WHITE:
		charsetInfo->_image->setFontColorFG((ColorFG)color);
	}
}

/**
 * Draw a character from the charset onto the screen.
 */
void screenShowChar(int chr, int x, int y) {
	if (charsetInfo == NULL) {
		charsetInfo = imageMgr->get(BKGD_CHARSET);
		if (!charsetInfo)
			error("ERROR 1001: Unable to load the \"%s\" data file", BKGD_CHARSET);
	}

	charsetInfo->_image->drawSubRect(x * charsetInfo->_image->width(), y * (CHAR_HEIGHT * settings._scale),
	                                 0, chr * (CHAR_HEIGHT * settings._scale),
	                                 charsetInfo->_image->width(), CHAR_HEIGHT * settings._scale);
}

/**
 * Scroll the text in the message area up one position.
 */
void screenScrollMessageArea() {
	ASSERT(charsetInfo != NULL && charsetInfo->_image != NULL, "charset not initialized!");

	Image *screen = imageMgr->get("screen")->_image;

	screen->drawSubRectOn(screen,
	                      TEXT_AREA_X * charsetInfo->_image->width(),
	                      TEXT_AREA_Y * CHAR_HEIGHT * settings._scale,
	                      TEXT_AREA_X * charsetInfo->_image->width(),
	                      (TEXT_AREA_Y + 1) * CHAR_HEIGHT * settings._scale,
	                      TEXT_AREA_W * charsetInfo->_image->width(),
	                      (TEXT_AREA_H - 1) * CHAR_HEIGHT * settings._scale);


	screen->fillRect(TEXT_AREA_X * charsetInfo->_image->width(),
	                 TEXT_AREA_Y * CHAR_HEIGHT * settings._scale + (TEXT_AREA_H - 1) * CHAR_HEIGHT * settings._scale,
	                 TEXT_AREA_W * charsetInfo->_image->width(),
	                 CHAR_HEIGHT * settings._scale,
	                 0, 0, 0);

	g_screen->update();
}

void screenCycle() {
	if (++screenCurrentCycle >= SCR_CYCLE_MAX)
		screenCurrentCycle = 0;
	g_screen->update();
}

void screenUpdateCursor() {
	int phase = screenCurrentCycle * SCR_CYCLE_PER_SECOND / SCR_CYCLE_MAX;

	ASSERT(phase >= 0 && phase < 4, "derived an invalid cursor phase: %d", phase);

	if (screenCursorStatus) {
		screenShowChar(31 - phase, screenCursorX, screenCursorY);
		screenRedrawTextArea(screenCursorX, screenCursorY, 1, 1);
	}
}

void screenUpdateMoons() {
	int trammelChar, feluccaChar;

	/* show "L?" for the dungeon level */
	if (g_context->_location->_context == CTX_DUNGEON) {
		screenShowChar('L', 11, 0);
		screenShowChar('1' + g_context->_location->_coords.z, 12, 0);
	}
	/* show the current moons (non-combat) */
	else if ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context) {
		trammelChar = (g_ultima->_saveGame->_trammelPhase == 0) ?
		              MOON_CHAR + 7 :
		              MOON_CHAR + g_ultima->_saveGame->_trammelPhase - 1;
		feluccaChar = (g_ultima->_saveGame->_feluccaPhase == 0) ?
		              MOON_CHAR + 7 :
		              MOON_CHAR + g_ultima->_saveGame->_feluccaPhase - 1;

		screenShowChar(trammelChar, 11, 0);
		screenShowChar(feluccaChar, 12, 0);
	}

	screenRedrawTextArea(11, 0, 2, 1);
}

void screenUpdateWind() {

	/* show the direction we're facing in the dungeon */
	if (g_context->_location->_context == CTX_DUNGEON) {
		screenEraseTextArea(WIND_AREA_X, WIND_AREA_Y, WIND_AREA_W, WIND_AREA_H);
		screenTextAt(WIND_AREA_X, WIND_AREA_Y, "Dir: %5s", getDirectionName((Direction)g_ultima->_saveGame->_orientation));
	}
	/* show the wind direction */
	else if ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context) {
		screenEraseTextArea(WIND_AREA_X, WIND_AREA_Y, WIND_AREA_W, WIND_AREA_H);
		screenTextAt(WIND_AREA_X, WIND_AREA_Y, "Wind %5s", getDirectionName((Direction) g_context->_windDirection));
	}
	screenRedrawTextArea(WIND_AREA_X, WIND_AREA_Y, WIND_AREA_W, WIND_AREA_H);
}

void screenShowCursor() {
	if (!screenCursorStatus && screenCursorEnabled) {
		screenCursorStatus = 1;
		screenUpdateCursor();
	}
}

void screenHideCursor() {
	if (screenCursorStatus) {
		screenEraseTextArea(screenCursorX, screenCursorY, 1, 1);
		screenRedrawTextArea(screenCursorX, screenCursorY, 1, 1);
	}
	screenCursorStatus = 0;
}

void screenEnableCursor(void) {
	screenCursorEnabled = 1;
}

void screenDisableCursor(void) {
	screenHideCursor();
	screenCursorEnabled = 0;
}

void screenSetCursorPos(int x, int y) {
	screenCursorX = x;
	screenCursorY = y;
}

/**
 * Finds which tiles in the viewport are visible from the avatars
 * location in the middle. (original DOS algorithm)
 */
void screenFindLineOfSight(Std::vector <MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]) {
	int x, y;

	if (!g_context)
		return;

	/*
	 * if the map has the no line of sight flag, all is visible
	 */
	if (g_context->_location->_map->_flags & NO_LINE_OF_SIGHT) {
		for (y = 0; y < VIEWPORT_H; y++) {
			for (x = 0; x < VIEWPORT_W; x++) {
				screenLos[x][y] = 1;
			}
		}
		return;
	}

	/*
	 * otherwise calculate it from the map data
	 */
	for (y = 0; y < VIEWPORT_H; y++) {
		for (x = 0; x < VIEWPORT_W; x++) {
			screenLos[x][y] = 0;
		}
	}

	if (settings._lineOfSight == "DOS")
		screenFindLineOfSightDOS(viewportTiles);
	else if (settings._lineOfSight == "Enhanced")
		screenFindLineOfSightEnhanced(viewportTiles);
	else
		errorFatal("unknown line of sight style %s!\n", settings._lineOfSight.c_str());
}


/**
 * Finds which tiles in the viewport are visible from the avatars
 * location in the middle. (original DOS algorithm)
 */
void screenFindLineOfSightDOS(Std::vector <MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]) {
	int x, y;

	screenLos[VIEWPORT_W / 2][VIEWPORT_H / 2] = 1;

	for (x = VIEWPORT_W / 2 - 1; x >= 0; x--)
		if (screenLos[x + 1][VIEWPORT_H / 2] &&
		        !viewportTiles[x + 1][VIEWPORT_H / 2].front().getTileType()->isOpaque())
			screenLos[x][VIEWPORT_H / 2] = 1;

	for (x = VIEWPORT_W / 2 + 1; x < VIEWPORT_W; x++)
		if (screenLos[x - 1][VIEWPORT_H / 2] &&
		        !viewportTiles[x - 1][VIEWPORT_H / 2].front().getTileType()->isOpaque())
			screenLos[x][VIEWPORT_H / 2] = 1;

	for (y = VIEWPORT_H / 2 - 1; y >= 0; y--)
		if (screenLos[VIEWPORT_W / 2][y + 1] &&
		        !viewportTiles[VIEWPORT_W / 2][y + 1].front().getTileType()->isOpaque())
			screenLos[VIEWPORT_W / 2][y] = 1;

	for (y = VIEWPORT_H / 2 + 1; y < VIEWPORT_H; y++)
		if (screenLos[VIEWPORT_W / 2][y - 1] &&
		        !viewportTiles[VIEWPORT_W / 2][y - 1].front().getTileType()->isOpaque())
			screenLos[VIEWPORT_W / 2][y] = 1;

	for (y = VIEWPORT_H / 2 - 1; y >= 0; y--) {

		for (x = VIEWPORT_W / 2 - 1; x >= 0; x--) {
			if (screenLos[x][y + 1] &&
			        !viewportTiles[x][y + 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x + 1][y] &&
			         !viewportTiles[x + 1][y].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x + 1][y + 1] &&
			         !viewportTiles[x + 1][y + 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
		}

		for (x = VIEWPORT_W / 2 + 1; x < VIEWPORT_W; x++) {
			if (screenLos[x][y + 1] &&
			        !viewportTiles[x][y + 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x - 1][y] &&
			         !viewportTiles[x - 1][y].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x - 1][y + 1] &&
			         !viewportTiles[x - 1][y + 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
		}
	}

	for (y = VIEWPORT_H / 2 + 1; y < VIEWPORT_H; y++) {

		for (x = VIEWPORT_W / 2 - 1; x >= 0; x--) {
			if (screenLos[x][y - 1] &&
			        !viewportTiles[x][y - 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x + 1][y] &&
			         !viewportTiles[x + 1][y].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x + 1][y - 1] &&
			         !viewportTiles[x + 1][y - 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
		}

		for (x = VIEWPORT_W / 2 + 1; x < VIEWPORT_W; x++) {
			if (screenLos[x][y - 1] &&
			        !viewportTiles[x][y - 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x - 1][y] &&
			         !viewportTiles[x - 1][y].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
			else if (screenLos[x - 1][y - 1] &&
			         !viewportTiles[x - 1][y - 1].front().getTileType()->isOpaque())
				screenLos[x][y] = 1;
		}
	}
}

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
 *   http://www.fadden.com/techmisc/fast-los.html
 *
 * This function uses a lookup table to get the correct shadowmap,
 * therefore, the table will need to be updated if the viewport
 * dimensions increase. Also, the function assumes that the
 * viewport width and height are odd values and that the player
 * is always at the center of the screen.
 */
void screenFindLineOfSightEnhanced(Std::vector <MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]) {
	int x, y;

	/*
	 * the shadow rasters for each viewport octant
	 *
	 * shadowRaster[0][0]    // number of raster segments in this shadow
	 * shadowRaster[0][1]    // #1 shadow bitmask value (low three bits) + "newline" flag (high bit)
	 * shadowRaster[0][2]    // #1 length
	 * shadowRaster[0][3]    // #2 shadow bitmask value
	 * shadowRaster[0][4]    // #2 length
	 * shadowRaster[0][5]    // #3 shadow bitmask value
	 * shadowRaster[0][6]    // #3 length
	 * ...etc...
	 */
	const int shadowRaster[14][13] = {
		{ 6, __VCH, 4, _N_CH, 1, __VCH, 3, _N___, 1, ___CH, 1, __VCH, 1 },    // raster_1_0
		{ 6, __VC_, 1, _NVCH, 2, __VC_, 1, _NVCH, 3, _NVCH, 2, _NVCH, 1 },    // raster_1_1
		//
		{ 4, __VCH, 3, _N__H, 1, ___CH, 1, __VCH, 1,     0, 0,     0, 0 },    // raster_2_0
		{ 6, __VC_, 2, _N_CH, 1, __VCH, 2, _N_CH, 1, __VCH, 1, _N__H, 1 },    // raster_2_1
		{ 6, __V__, 1, _NVCH, 1, __VC_, 1, _NVCH, 1, __VC_, 1, _NVCH, 1 },    // raster_2_2
		//
		{ 2, __VCH, 2, _N__H, 2,     0, 0,     0, 0,     0, 0,     0, 0 },    // raster_3_0
		{ 3, __VC_, 2, _N_CH, 1, __VCH, 1,     0, 0,     0, 0,     0, 0 },    // raster_3_1
		{ 3, __VC_, 1, _NVCH, 2, _N_CH, 1,     0, 0,     0, 0,     0, 0 },    // raster_3_2
		{ 3, _NVCH, 1, __V__, 1, _NVCH, 1,     0, 0,     0, 0,     0, 0 },    // raster_3_3
		//
		{ 2, __VCH, 1, _N__H, 1,     0, 0,     0, 0,     0, 0,     0, 0 },    // raster_4_0
		{ 2, __VC_, 1, _N__H, 1,     0, 0,     0, 0,     0, 0,     0, 0 },    // raster_4_1
		{ 2, __VC_, 1, _N_CH, 1,     0, 0,     0, 0,     0, 0,     0, 0 },    // raster_4_2
		{ 2, __V__, 1, _NVCH, 1,     0, 0,     0, 0,     0, 0,     0, 0 },    // raster_4_3
		{ 2, __V__, 1, _NVCH, 1,     0, 0,     0, 0,     0, 0,     0, 0 }     // raster_4_4
	};

	/*
	 * As each viewport tile is processed, it will store the bitmask for the shadow it casts.
	 * Later, after processing all octants, the entire viewport will be marked visible except
	 * for those tiles that have the __VCH bitmask.
	 */
	const int _OCTANTS = 8;
	const int _NUM_RASTERS_COLS = 4;

	int octant;
	int xOrigin, yOrigin, xSign = 0, ySign = 0, reflect = false, xTile, yTile, xTileOffset, yTileOffset;

	for (octant = 0; octant < _OCTANTS; octant++) {
		switch (octant) {
		case 0:
			xSign =  1;
			ySign =  1;
			reflect = false;
			break;        // lower-right
		case 1:
			xSign =  1;
			ySign =  1;
			reflect = true;
			break;
		case 2:
			xSign =  1;
			ySign = -1;
			reflect = true;
			break;        // lower-left
		case 3:
			xSign = -1;
			ySign =  1;
			reflect = false;
			break;
		case 4:
			xSign = -1;
			ySign = -1;
			reflect = false;
			break;        // upper-left
		case 5:
			xSign = -1;
			ySign = -1;
			reflect = true;
			break;
		case 6:
			xSign = -1;
			ySign =  1;
			reflect = true;
			break;        // upper-right
		case 7:
			xSign =  1;
			ySign = -1;
			reflect = false;
			break;
		}

		// determine the origin point for the current LOS octant
		xOrigin = VIEWPORT_W / 2;
		yOrigin = VIEWPORT_H / 2;

		// make sure the segment doesn't reach out of bounds
		int maxWidth      = xOrigin;
		int maxHeight     = yOrigin;
		int currentRaster = 0;

		// just in case the viewport isn't square, swap the width and height
		if (reflect) {
			// swap height and width for later use
			maxWidth ^= maxHeight;
			maxHeight ^= maxWidth;
			maxWidth ^= maxHeight;
		}

		// check the visibility of each tile
		for (int currentCol = 1; currentCol <= _NUM_RASTERS_COLS; currentCol++) {
			for (int currentRow = 0; currentRow <= currentCol; currentRow++) {
				// swap X and Y to reflect the octant rasters
				if (reflect) {
					xTile = xOrigin + (currentRow * ySign);
					yTile = yOrigin + (currentCol * xSign);
				} else {
					xTile = xOrigin + (currentCol * xSign);
					yTile = yOrigin + (currentRow * ySign);
				}

				if (viewportTiles[xTile][yTile].front().getTileType()->isOpaque()) {
					// a wall was detected, so go through the raster for this wall
					// segment and mark everything behind it with the appropriate
					// shadow bitmask.
					//
					// first, get the correct raster
					//
					if ((currentCol == 1) && (currentRow == 0)) {
						currentRaster = 0;
					} else if ((currentCol == 1) && (currentRow == 1)) {
						currentRaster = 1;
					} else if ((currentCol == 2) && (currentRow == 0)) {
						currentRaster = 2;
					} else if ((currentCol == 2) && (currentRow == 1)) {
						currentRaster = 3;
					} else if ((currentCol == 2) && (currentRow == 2)) {
						currentRaster = 4;
					} else if ((currentCol == 3) && (currentRow == 0)) {
						currentRaster = 5;
					} else if ((currentCol == 3) && (currentRow == 1)) {
						currentRaster = 6;
					} else if ((currentCol == 3) && (currentRow == 2)) {
						currentRaster = 7;
					} else if ((currentCol == 3) && (currentRow == 3)) {
						currentRaster = 8;
					} else if ((currentCol == 4) && (currentRow == 0)) {
						currentRaster = 9;
					} else if ((currentCol == 4) && (currentRow == 1)) {
						currentRaster = 10;
					} else if ((currentCol == 4) && (currentRow == 2)) {
						currentRaster = 11;
					} else if ((currentCol == 4) && (currentRow == 3)) {
						currentRaster = 12;
					} else {
						currentRaster = 13;    // currentCol and currentRow must equal 4
					}

					xTileOffset = 0;
					yTileOffset = 0;

					//========================================
					for (int currentSegment = 0; currentSegment < shadowRaster[currentRaster][0]; currentSegment++) {
						// each shadow segment is 2 bytes
						int shadowType   = shadowRaster[currentRaster][currentSegment * 2 + 1];
						int shadowLength = shadowRaster[currentRaster][currentSegment * 2 + 2];

						// update the raster length to make sure it fits in the viewport
						shadowLength = (shadowLength + 1 + yTileOffset > maxWidth ? maxWidth : shadowLength);

						// check to see if we should move up a row
						if (shadowType & 0x80) {
							// remove the flag from the shadowType
							shadowType ^= _N___;
//                            if (currentRow + yTileOffset >= maxHeight) {
							if (currentRow + yTileOffset > maxHeight) {
								break;
							}
							xTileOffset = yTileOffset;
							yTileOffset++;
						}

						/* it is seemingly unnecessary to swap the edges for
						 * shadow tiles, because we only care about shadow
						 * tiles that have all three parts (V, C, and H)
						 * flagged.  if a tile has fewer than three, it is
						 * ignored during the draw phase, so vertical and
						 * horizontal shadow edge accuracy isn't important
						 */
						// if reflecting the octant, swap the edges
//                        if (reflect) {
//                            int shadowTemp = 0;
//                            // swap the vertical and horizontal shadow edges
//                            if (shadowType & __V__) { shadowTemp |= ____H; }
//                            if (shadowType & ___C_) { shadowTemp |= ___C_; }
//                            if (shadowType & ____H) { shadowTemp |= __V__; }
//                            shadowType = shadowTemp;
//                        }

						for (int currentShadow = 1; currentShadow <= shadowLength; currentShadow++) {
							// apply the shadow to the shadowMap
							if (reflect) {
								screenLos[xTile + ((yTileOffset) * ySign)][yTile + ((currentShadow + xTileOffset) * xSign)] |= shadowType;
							} else {
								screenLos[xTile + ((currentShadow + xTileOffset) * xSign)][yTile + ((yTileOffset) * ySign)] |= shadowType;
							}
						}
						xTileOffset += shadowLength;
					}  // for (int currentSegment = 0; currentSegment < shadowRaster[currentRaster][0]; currentSegment++)
					//========================================

				}  // if (viewportTiles[xTile][yTile].front().getTileType()->isOpaque())
			}  // for (int currentRow = 0; currentRow <= currentCol; currentRow++)
		}  // for (int currentCol = 1; currentCol <= _NUM_RASTERS_COLS; currentCol++)
	}  // for (octant = 0; octant < _OCTANTS; octant++)

	// go through all tiles on the viewable area and set the appropriate visibility
	for (y = 0; y < VIEWPORT_H; y++) {
		for (x = 0; x < VIEWPORT_W; x++) {
			// if the shadow flags equal __VCH, hide it, otherwise it's fully visible
			//
			if ((screenLos[x][y] & __VCH) == __VCH) {
				screenLos[x][y] = 0;
			} else {
				screenLos[x][y] = 1;
			}
		}
	}
}

/**
 * Generates terms a and b for equation "ax + b = y" that defines the
 * line containing the two given points.  Vertical lines are special
 * cased to return DBL_MAX for a and the x coordinate as b since they
 * cannot be represented with the above formula.
 */
static void screenGetLineTerms(int x1, int y1, int x2, int y2, double *a, double *b) {
	if (x2 - x1 == 0) {
		*a = DBL_MAX;
		*b = x1;
	} else {
		*a = ((double)(y2 - y1)) / ((double)(x2 - x1));
		*b = y1 - ((*a) * x1);
	}
}

/**
 * Determine if two points are on the same side of a line (or both on
 * the line).  The line is defined by the terms a and b of the
 * equation "ax + b = y".
 */
static int screenPointsOnSameSideOfLine(int x1, int y1, int x2, int y2, double a, double b) {
	double p1, p2;

	if (a == DBL_MAX) {
		p1 = x1 - b;
		p2 = x2 - b;
	} else {
		p1 = x1 * a + b - y1;
		p2 = x2 * a + b - y2;
	}

	if ((p1 > 0.0 && p2 > 0.0) ||
	        (p1 < 0.0 && p2 < 0.0) ||
	        (p1 == 0.0 && p2 == 0.0))
		return 1;

	return 0;
}

static int screenPointInTriangle(int x, int y, int tx1, int ty1, int tx2, int ty2, int tx3, int ty3) {
	double a[3], b[3];

	screenGetLineTerms(tx1, ty1, tx2, ty2, &(a[0]), &(b[0]));
	screenGetLineTerms(tx2, ty2, tx3, ty3, &(a[1]), &(b[1]));
	screenGetLineTerms(tx3, ty3, tx1, ty1, &(a[2]), &(b[2]));

	if (!screenPointsOnSameSideOfLine(x, y, tx3, ty3, a[0], b[0]))
		return 0;
	if (!screenPointsOnSameSideOfLine(x, y, tx1, ty1, a[1], b[1]))
		return 0;
	if (!screenPointsOnSameSideOfLine(x, y, tx2, ty2, a[2], b[2]))
		return 0;

	return 1;
}

/**
 * Determine if the given point is within a mouse area.
 */
int screenPointInMouseArea(int x, int y, const MouseArea *area) {
	ASSERT(area->_nPoints == 2 || area->_nPoints == 3, "unsupported number of points in area: %d", area->_nPoints);

	/* two points define a rectangle */
	if (area->_nPoints == 2) {
		if (x >= (int)(area->_point[0].x * settings._scale) &&
				y >= (int)(area->_point[0].y * settings._scale) &&
				x < (int)(area->_point[1].x * settings._scale) &&
				y < (int)(area->_point[1].y * settings._scale)) {
			return 1;
		}
	}

	/* three points define a triangle */
	else if (area->_nPoints == 3) {
		return screenPointInTriangle(x, y,
			area->_point[0].x * settings._scale, area->_point[0].y * settings._scale,
			area->_point[1].x * settings._scale, area->_point[1].y * settings._scale,
			area->_point[2].x * settings._scale, area->_point[2].y * settings._scale);
	}

	return 0;
}

void screenRedrawMapArea() {
	g_game->_mapArea.update();
}

void screenEraseMapArea() {
	Image *screen = imageMgr->get("screen")->_image;
	screen->fillRect(BORDER_WIDTH * settings._scale,
	                 BORDER_WIDTH * settings._scale,
	                 VIEWPORT_W * TILE_WIDTH * settings._scale,
	                 VIEWPORT_H * TILE_HEIGHT * settings._scale,
	                 0, 0, 0);
}

void screenEraseTextArea(int x, int y, int width, int height) {
	Image *screen = imageMgr->get("screen")->_image;
	screen->fillRect(x * CHAR_WIDTH * settings._scale,
	                 y * CHAR_HEIGHT * settings._scale,
	                 width * CHAR_WIDTH * settings._scale,
	                 height * CHAR_HEIGHT * settings._scale,
	                 0, 0, 0);
}

/**
 * Do the tremor spell effect where the screen shakes.
 */
void screenShake(int iterations) {
	int shakeOffset;
	unsigned short i;
	Image *screen = imageMgr->get("screen")->_image;
	Image *bottom;

	// the MSVC8 binary was generating a Access Violation when using
	// drawSubRectOn() or drawOn() to draw the screen surface on top
	// of itself.  Occured on settings.scale 2 and 4 only.
	// Therefore, a temporary Image buffer is used to store the area
	// that gets clipped at the bottom.

	if (settings._screenShakes) {
		// specify the size of the offset, and create a buffer
		// to store the offset row plus 1
		shakeOffset = 1;
		bottom = Image::create(SCALED(320), SCALED(shakeOffset + 1), false, Image::HARDWARE);

		for (i = 0; i < iterations; i++) {
			// store the bottom row
			screen->drawOn(bottom, 0, SCALED((shakeOffset + 1) - 200));

			// shift the screen down and make the top row black
			screen->drawSubRectOn(screen, 0, SCALED(shakeOffset), 0, 0, SCALED(320), SCALED(200 - (shakeOffset + 1)));
			bottom->drawOn(screen, 0, SCALED(200 - (shakeOffset)));
			screen->fillRect(0, 0, SCALED(320), SCALED(shakeOffset), 0, 0, 0);
			g_screen->update();
			EventHandler::sleep(settings._shakeInterval);

			// shift the screen back up, and replace the bottom row
			screen->drawOn(screen, 0, 0 - SCALED(shakeOffset));
			bottom->drawOn(screen, 0, SCALED(200 - (shakeOffset + 1)));
			g_screen->update();
			EventHandler::sleep(settings._shakeInterval);
		}
		// free the bottom row image
		delete bottom;
	}
}

/**
 * Draw a tile graphic on the screen.
 */
void screenShowGemTile(Layout *layout, Map *map, MapTile &t, bool focus, int x, int y) {
	// Make sure we account for tiles that look like other tiles (dungeon tiles, mainly)
	Common::String looks_like = t.getTileType()->getLooksLike();
	if (!looks_like.empty())
		t = map->_tileset->getByName(looks_like)->getId();

	unsigned int tile = map->translateToRawTileIndex(t);

	if (map->_type == Map::DUNGEON) {
		ASSERT(charsetInfo, "charset not initialized");
		Std::map<Common::String, int>::iterator charIndex = dungeonTileChars.find(t.getTileType()->getName());
		if (charIndex != dungeonTileChars.end()) {
			charsetInfo->_image->drawSubRect((layout->_viewport.left + (x * layout->_tileShape.x)) * settings._scale,
			                                 (layout->_viewport.top + (y * layout->_tileShape.y)) * settings._scale,
			                                 0,
			                                 charIndex->_value * layout->_tileShape.y * settings._scale,
			                                 layout->_tileShape.x * settings._scale,
			                                 layout->_tileShape.y * settings._scale);
		}
	} else {
		if (gemTilesInfo == NULL) {
			gemTilesInfo = imageMgr->get(BKGD_GEMTILES);
			if (!gemTilesInfo)
				errorFatal("ERROR 1002: Unable to load the \"%s\" data file.\t\n\nIs %s installed?\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/", BKGD_GEMTILES, settings._game.c_str());
		}

		if (tile < 128) {
			gemTilesInfo->_image->drawSubRect((layout->_viewport.left + (x * layout->_tileShape.x)) * settings._scale,
			                                  (layout->_viewport.top + (y * layout->_tileShape.y)) * settings._scale,
			                                  0,
			                                  tile * layout->_tileShape.y * settings._scale,
			                                  layout->_tileShape.x * settings._scale,
			                                  layout->_tileShape.y * settings._scale);
		} else {
			Image *screen = imageMgr->get("screen")->_image;
			screen->fillRect((layout->_viewport.left + (x * layout->_tileShape.x)) * settings._scale,
			                 (layout->_viewport.top + (y * layout->_tileShape.y)) * settings._scale,
			                 layout->_tileShape.x * settings._scale,
			                 layout->_tileShape.y * settings._scale,
			                 0, 0, 0);
		}
	}
}

Layout *screenGetGemLayout(const Map *map) {
	if (map->_type == Map::DUNGEON) {
		Std::vector<Layout *>::const_iterator i;
		for (i = g_screen->_layouts.begin(); i != g_screen->_layouts.end(); i++) {
			Layout *layout = *i;

			if (layout->_type == LAYOUT_DUNGEONGEM)
				return layout;
		}
		errorFatal("no dungeon gem layout found!\n");
		return NULL;
	} else
		return gemlayout;
}


void screenGemUpdate() {
	MapTile tile;
	int x, y;
	Image *screen = imageMgr->get("screen")->_image;

	screen->fillRect(BORDER_WIDTH * settings._scale,
	                 BORDER_HEIGHT * settings._scale,
	                 VIEWPORT_W * TILE_WIDTH * settings._scale,
	                 VIEWPORT_H * TILE_HEIGHT * settings._scale,
	                 0, 0, 0);

	Layout *layout = screenGetGemLayout(g_context->_location->_map);


	//TODO, move the code responsible for determining 'peer' visibility to a non SDL specific part of the code.
	if (g_context->_location->_map->_type == Map::DUNGEON) {
		//DO THE SPECIAL DUNGEON MAP TRAVERSAL
		Std::vector<Std::vector<int> > drawnTiles(layout->_viewport.width(), Std::vector<int>(layout->_viewport.height(), 0));
		Common::List<Std::pair<int, int> > coordStack;

		//Put the avatar's position on the stack
		int center_x = layout->_viewport.width() / 2 - 1;
		int center_y = layout->_viewport.height() / 2 - 1;
		int avt_x = g_context->_location->_coords.x - 1;
		int avt_y = g_context->_location->_coords.y - 1;

		coordStack.push_back(Std::pair<int, int>(center_x, center_y));
		bool weAreDrawingTheAvatarTile = true;

		//And draw each tile on the growing stack until it is empty
		while (coordStack.size() > 0) {
			Std::pair<int, int> currentXY = coordStack.back();
			coordStack.pop_back();

			x = currentXY.first;
			y = currentXY.second;

			if (x < 0 || x >= layout->_viewport.width() ||
			        y < 0 || y >= layout->_viewport.height())
				continue;   //Skip out of range tiles

			if (drawnTiles[x][y])
				continue;   //Skip already considered tiles

			drawnTiles[x][y] = 1;

			// DRAW THE ACTUAL TILE
			bool focus;


			Std::vector<MapTile> tiles = screenViewportTile(layout->_viewport.width(),
			                             layout->_viewport.height(), x - center_x + avt_x, y - center_y + avt_y, focus);
			tile = tiles.front();

			TileId avatarTileId = g_context->_location->_map->_tileset->getByName("avatar")->getId();


			if (!weAreDrawingTheAvatarTile) {
				//Hack to avoid showing the avatar tile multiple times in cycling dungeon maps
				if (tile.getId() == avatarTileId)
					tile = g_context->_location->_map->getTileFromData(g_context->_location->_coords)->getId();
			}

			screenShowGemTile(layout, g_context->_location->_map, tile, focus, x, y);

			if (!tile.getTileType()->isOpaque() || tile.getTileType()->isWalkable() ||  weAreDrawingTheAvatarTile) {
				//Continue the search so we can see through all walkable objects, non-opaque objects (like creatures)
				//or the avatar position in those rare circumstances where he is stuck in a wall

				//by adding all relative adjacency combinations to the stack for drawing
				coordStack.push_back(Std::pair<int, int>(x   + 1 ,   y   - 1));
				coordStack.push_back(Std::pair<int, int>(x   + 1 ,   y));
				coordStack.push_back(Std::pair<int, int>(x   + 1 ,   y   + 1));

				coordStack.push_back(Std::pair<int, int>(x       ,   y   - 1));
				coordStack.push_back(Std::pair<int, int>(x       ,   y   + 1));

				coordStack.push_back(Std::pair<int, int>(x   - 1 ,   y   - 1));
				coordStack.push_back(Std::pair<int, int>(x   - 1 ,   y));
				coordStack.push_back(Std::pair<int, int>(x   - 1 ,   y   + 1));

				// We only draw the avatar tile once, it is the first tile drawn
				weAreDrawingTheAvatarTile = false;
			}
		}

	} else {
		// DO THE REGULAR EVERYTHING-IS-VISIBLE MAP TRAVERSAL
		for (x = 0; x < layout->_viewport.width(); x++) {
			for (y = 0; y < layout->_viewport.height(); y++) {
				bool focus;
				tile = screenViewportTile(layout->_viewport.width(), layout->_viewport.height(),
				                          x, y, focus).front();
				screenShowGemTile(layout, g_context->_location->_map, tile, focus, x, y);
			}
		}
	}

	screenRedrawMapArea();

	screenUpdateCursor();
	screenUpdateMoons();
	screenUpdateWind();
}

#ifdef IOS
//Unsure if implementation required in iOS.
void inline screenLock() {};
void inline screenUnlock() {};
void inline screenWait(int numberOfAnimationFrames) {};
#endif

} // End of namespace Ultima4
} // End of namespace Ultima
