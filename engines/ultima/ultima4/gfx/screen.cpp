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

#include "ultima/ultima4/gfx/screen.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "ultima/ultima4/controllers/intro_controller.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/gfx/textcolor.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/tileanim.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/views/dungeonview.h"
#include "ultima/ultima4/views/tileview.h"

namespace Ultima {
namespace Ultima4 {

#define MOUSE_CURSOR_SIZE 20
#ifndef DBL_MAX
#define DBL_MAX 1e99
#endif
#define BUFFER_SIZE 1024

Screen *g_screen;

Screen::Screen() : _filterScaler(nullptr), _currentMouseCursor(-1),
				   _gemLayout(nullptr), _tileAnims(nullptr), _charSetInfo(nullptr),
				   _gemTilesInfo(nullptr), _needPrompt(1), _currentCycle(0),
				   _cursorStatus(0), _cursorEnabled(1), _priorFrameTime(0)
				   /* , _continueScreenRefresh(true) */ {
	g_screen = this;
	Common::fill(&_mouseCursors[0], &_mouseCursors[5], (MouseCursorSurface *)nullptr);
	Common::fill(&_los[0][0], &_los[0][0] + (VIEWPORT_W * VIEWPORT_H), 0);

	_filterNames.clear();
	_filterNames.push_back("point");
	_filterNames.push_back("2xBi");
	_filterNames.push_back("2xSaI");
	_filterNames.push_back("Scale2x");

	_lineOfSightStyles.clear();
	_lineOfSightStyles.push_back("DOS");
	_lineOfSightStyles.push_back("Enhanced");
}

Screen::~Screen() {
	clear();
	for (uint idx = 0; idx < _tileAnimSets.size(); ++idx)
		delete _tileAnimSets[idx];

	g_screen = nullptr;
}

void Screen::init() {
	Graphics::PixelFormat SCREEN_FORMAT(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Common::Point size(SCREEN_WIDTH * settings._scale, SCREEN_HEIGHT * settings._scale);

	initGraphics(size.x, size.y, &SCREEN_FORMAT);
	create(size.x, size.y, SCREEN_FORMAT);

	loadMouseCursors();
	screenLoadGraphicsFromConf();

	debug(1, "using %s scaler\n", settings._filter.c_str());

	/* find the tile animations for our tileset */
	_tileAnims = nullptr;
	for (Std::vector<TileAnimSet *>::const_iterator i = _tileAnimSets.begin(); i != _tileAnimSets.end(); i++) {
		TileAnimSet *set = *i;
		if (set->_name == settings._videoType)
			_tileAnims = set;
	}
	if (!_tileAnims)
		error("unable to find tile animations for \"%s\" video mode in graphics.xml", settings._videoType.c_str());

	_dungeonTileChars.clear();
	_dungeonTileChars["brick_floor"] = CHARSET_FLOOR;
	_dungeonTileChars["up_ladder"] = CHARSET_LADDER_UP;
	_dungeonTileChars["down_ladder"] = CHARSET_LADDER_DOWN;
	_dungeonTileChars["up_down_ladder"] = CHARSET_LADDER_UPDOWN;
	_dungeonTileChars["chest"] = '$';
	_dungeonTileChars["ceiling_hole"] = CHARSET_FLOOR;
	_dungeonTileChars["floor_hole"] = CHARSET_FLOOR;
	_dungeonTileChars["magic_orb"] = CHARSET_ORB;
	_dungeonTileChars["ceiling_hole"] = 'T';
	_dungeonTileChars["floor_hole"] = 'T';
	_dungeonTileChars["fountain"] = 'F';
	_dungeonTileChars["secret_door"] = CHARSET_SDOOR;
	_dungeonTileChars["brick_wall"] = CHARSET_WALL;
	_dungeonTileChars["dungeon_door"] = CHARSET_ROOM;
	_dungeonTileChars["avatar"] = CHARSET_REDDOT;
	_dungeonTileChars["dungeon_room"] = CHARSET_ROOM;
	_dungeonTileChars["dungeon_altar"] = CHARSET_ANKH;
	_dungeonTileChars["energy_field"] = '^';
	_dungeonTileChars["fire_field"] = '^';
	_dungeonTileChars["poison_field"] = '^';
	_dungeonTileChars["sleep_field"] = '^';
}

void Screen::clear() {
	// Clear any pending updates for the current screen
	update();

	Std::vector<Layout *>::const_iterator i;
	for (i = _layouts.begin(); i != _layouts.end(); ++i)
		delete (*i);
	_layouts.clear();

	ImageMgr::destroy();
	_charSetInfo = nullptr;

	// Delete cursors
	for (int idx = 0; idx < 5; ++idx) {
		delete _mouseCursors[idx];
		_mouseCursors[idx] = nullptr;
	}
}

void Screen::loadMouseCursors() {
	// enable or disable the mouse cursor
	if (settings._mouseOptions._enabled) {
		Shared::File cursorsFile("data/graphics/cursors.txt");

		for (int idx = 0; idx < 5; ++idx)
			_mouseCursors[idx] = loadMouseCursor(cursorsFile);

		// Set the default initial cursor
		const uint TRANSPARENT = format.RGBToColor(0x80, 0x80, 0x80);
		MouseCursorSurface *c = _mouseCursors[MC_DEFAULT];
		CursorMan.pushCursor(c->getPixels(),
			MOUSE_CURSOR_SIZE, MOUSE_CURSOR_SIZE,
			c->_hotspot.x, c->_hotspot.y, TRANSPARENT, false, &format);
		CursorMan.showMouse(true);

	} else {
		CursorMan.showMouse(false);
	}

	_filterScaler = scalerGet(settings._filter);
	if (!_filterScaler)
		error("%s is not a valid filter", settings._filter.c_str());
}

void Screen::setMouseCursor(MouseCursor cursor) {
	const MouseCursorSurface *c = _mouseCursors[cursor];

	if (c && cursor != _currentMouseCursor) {
		_currentMouseCursor = cursor;

		const uint TRANSPARENT = format.RGBToColor(0x80, 0x80, 0x80);
		CursorMan.replaceCursor(c->getPixels(), MOUSE_CURSOR_SIZE, MOUSE_CURSOR_SIZE,
		                        c->_hotspot.x, c->_hotspot.y, TRANSPARENT, false, &format);
	}
}

MouseCursorSurface *Screen::loadMouseCursor(Shared::File &src) {
	uint row, col, endCol, pixel;
	int hotX, hotY;
	Common::String line;
	byte *destP;
	const uint WHITE = format.RGBToColor(0xff, 0xff, 0xff);
	const uint BLACK = format.RGBToColor(0, 0, 0);
	const uint TRANSPARENT = format.RGBToColor(0x80, 0x80, 0x80);
	int bpp = format.bytesPerPixel;
	assert(bpp >= 2);

	MouseCursorSurface *c = new MouseCursorSurface();
	c->create(MOUSE_CURSOR_SIZE, MOUSE_CURSOR_SIZE, format);
	c->clear(TRANSPARENT);

	for (row = 0; row < MOUSE_CURSOR_SIZE; row++) {
		line = src.readLine();
		destP = (byte *)c->getBasePtr(0, row);
		endCol = MIN(line.size(), (uint)MOUSE_CURSOR_SIZE);

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
	(void)sscanf(line.c_str(), "%d,%d", &hotX, &hotY);
	c->_hotspot.x = hotX;
	c->_hotspot.y = hotY;

	return c;
}

void Screen::screenReInit() {
	g_intro->deleteIntro();        // delete intro stuff
	g_tileSets->unloadAllImages(); // unload tilesets, which will be reloaded lazily as needed
	ImageMgr::destroy();
	_tileAnims = nullptr;
	clear();
	init();          // Re-init screen stuff (loading new backgrounds, etc.)
	g_intro->init(); // Re-fix the backgrounds loaded and scale images, etc.
}

void Screen::screenTextAt(int x, int y, const char *fmt, ...) {
	char buffer[BUFFER_SIZE];
	uint i;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, BUFFER_SIZE, fmt, args);
	va_end(args);

	for (i = 0; i < strlen(buffer); i++)
		screenShowChar(buffer[i], x + i, y);
}

void Screen::screenPrompt() {
	if (_needPrompt && _cursorEnabled && g_context->_col == 0) {
		screenMessage("%c", CHARSET_PROMPT);
		_needPrompt = 0;
	}
}

void Screen::screenMessage(const char *fmt, ...) {
#ifdef IOS_ULTIMA4
	static bool recursed = false;
#endif

	if (!g_context)
		return; //Because some cases (like the intro) don't have the context initiated.
	char buffer[BUFFER_SIZE];
	uint i;
	int wordlen;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, BUFFER_SIZE, fmt, args);
	va_end(args);
#ifdef IOS_ULTIMA4
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
			g_context->_col--;
			if (g_context->_col < 0) {
				g_context->_col += 16;
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
		if ((g_context->_col + wordlen > 16) || buffer[i] == '\n' || g_context->_col == 16) {
			if (buffer[i] == '\n' || buffer[i] == ' ')
				i++;
			g_context->_line++;
			g_context->_col = 0;
#ifdef IOS_ULTIMA4
			recursed = true;
#endif
			screenMessage("%s", buffer + i);
			return;
		}

		/* code for move cursor right */
		if (buffer[i] == 0x12) {
			g_context->_col++;
			continue;
		}
		/* don't show a space in column 1.  Helps with Hawkwind. */
		if (buffer[i] == ' ' && g_context->_col == 0)
			continue;
		screenShowChar(buffer[i], TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);
		g_context->_col++;
	}

	screenSetCursorPos(TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);
	screenShowCursor();

	_needPrompt = 1;
}

void Screen::screenLoadGraphicsFromConf() {
	const Config *config = Config::getInstance();

	Std::vector<ConfigElement> graphicsConf = config->getElement("graphics").getChildren();
	for (Std::vector<ConfigElement>::iterator conf = graphicsConf.begin(); conf != graphicsConf.end(); conf++) {

		if (conf->getName() == "layout")
			_layouts.push_back(screenLoadLayoutFromConf(*conf));
		else if (conf->getName() == "tileanimset")
			_tileAnimSets.push_back(new TileAnimSet(*conf));
	}

	_gemLayoutNames.clear();
	Std::vector<Layout *>::const_iterator i;
	for (i = _layouts.begin(); i != _layouts.end(); i++) {
		Layout *layout = *i;
		if (layout->_type == LAYOUT_GEM) {
			_gemLayoutNames.push_back(layout->_name);
		}
	}

	/*
	 * Find gem layout to use.
	 */
	for (i = _layouts.begin(); i != _layouts.end(); i++) {
		Layout *layout = *i;

		if (layout->_type == LAYOUT_GEM && layout->_name == settings._gemLayout) {
			_gemLayout = layout;
			break;
		}
	}
	if (!_gemLayout)
		error("no gem layout named %s found!\n", settings._gemLayout.c_str());
}

Layout *Screen::screenLoadLayoutFromConf(const ConfigElement &conf) {
	Layout *layout;
	static const char *typeEnumStrings[] = {"standard", "gem", "dungeon_gem", nullptr};

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

Std::vector<MapTile> Screen::screenViewportTile(uint width, uint height, int x, int y, bool &focus) {
	MapCoords center = g_context->_location->_coords;
	static MapTile grass = g_context->_location->_map->_tileSet->getByName("grass")->getId();

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

bool Screen::screenTileUpdate(TileView *view, const Coords &coords, bool redraw) {
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
	if (x >= 0 && y >= 0 && x < VIEWPORT_W && y < VIEWPORT_H && _los[x][y]) {
		view->drawTile(tiles, focus, x, y);

		if (redraw) {
			//screenRedrawMapArea();
		}
		return true;
	}
	return false;
}

void Screen::screenUpdate(TileView *view, bool showmap, bool blackout) {
	assertMsg(g_context != nullptr, "context has not yet been initialized");

	if (blackout) {
		screenEraseMapArea();
	} else if (g_context->_location->_viewMode == VIEW_GEM) {
		// No need to render view when cheat overhead map showing
	} else if (g_context->_location->_map->_flags & FIRST_PERSON) {
		DungeonViewer.display(g_context, view);
		screenRedrawMapArea();
	} else if (showmap) {
		static MapTile black = g_context->_location->_map->_tileSet->getByName("black")->getId();
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
				if (_los[x][y]) {
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

void Screen::screenDrawImage(const Common::String &name, int x, int y) {
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
			                          subimage->left * (settings._scale / info->_prescale),
			                          subimage->top * (settings._scale / info->_prescale),
			                          subimage->width() * (settings._scale / info->_prescale),
			                          subimage->height() * (settings._scale / info->_prescale));
			return;
		}
	}
	error("ERROR 1006: Unable to load the image \"%s\"", name.c_str());
}

void Screen::screenDrawImageInMapArea(const Common::String &name) {
	ImageInfo *info;

	info = imageMgr->get(name);
	if (!info)
		error("ERROR 1004: Unable to load data files");

	info->_image->drawSubRect(BORDER_WIDTH * settings._scale, BORDER_HEIGHT * settings._scale,
	                          BORDER_WIDTH * settings._scale, BORDER_HEIGHT * settings._scale,
	                          VIEWPORT_W * TILE_WIDTH * settings._scale,
	                          VIEWPORT_H * TILE_HEIGHT * settings._scale);
}

void Screen::screenTextColor(int color) {
	if (_charSetInfo == nullptr) {
		_charSetInfo = imageMgr->get(BKGD_CHARSET);
		if (!_charSetInfo)
			error("ERROR 1003: Unable to load the \"%s\" data file", BKGD_CHARSET);
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
		_charSetInfo->_image->setFontColorFG((ColorFG)color);
	}
}

void Screen::screenShowChar(int chr, int x, int y) {
	if (_charSetInfo == nullptr) {
		_charSetInfo = imageMgr->get(BKGD_CHARSET);
		if (!_charSetInfo)
			error("ERROR 1001: Unable to load the \"%s\" data file", BKGD_CHARSET);
	}

	_charSetInfo->_image->drawSubRect(x * _charSetInfo->_image->width(), y * (CHAR_HEIGHT * settings._scale),
	                                  0, chr * (CHAR_HEIGHT * settings._scale),
	                                  _charSetInfo->_image->width(), CHAR_HEIGHT * settings._scale);
}

void Screen::screenScrollMessageArea() {
	assertMsg(_charSetInfo != nullptr && _charSetInfo->_image != nullptr, "charset not initialized!");

	Image *screen = imageMgr->get("screen")->_image;

	screen->drawSubRectOn(screen,
	                      TEXT_AREA_X * _charSetInfo->_image->width(),
	                      TEXT_AREA_Y * CHAR_HEIGHT * settings._scale,
	                      TEXT_AREA_X * _charSetInfo->_image->width(),
	                      (TEXT_AREA_Y + 1) * CHAR_HEIGHT * settings._scale,
	                      TEXT_AREA_W * _charSetInfo->_image->width(),
	                      (TEXT_AREA_H - 1) * CHAR_HEIGHT * settings._scale);

	screen->fillRect(TEXT_AREA_X * _charSetInfo->_image->width(),
	                 TEXT_AREA_Y * CHAR_HEIGHT * settings._scale + (TEXT_AREA_H - 1) * CHAR_HEIGHT * settings._scale,
	                 TEXT_AREA_W * _charSetInfo->_image->width(),
	                 CHAR_HEIGHT * settings._scale,
	                 0, 0, 0);

	update();
}

void Screen::screenFrame() {
	uint32 time = g_system->getMillis();

	if (time >= (_priorFrameTime + SCREEN_FRAME_TIME)) {
		_priorFrameTime = time;
		update();
	}
}

void Screen::screenCycle() {
	if (++_currentCycle >= SCR_CYCLE_MAX)
		_currentCycle = 0;
}

void Screen::screenUpdateCursor() {
	int phase = _currentCycle * SCR_CYCLE_PER_SECOND / SCR_CYCLE_MAX;

	assertMsg(phase >= 0 && phase < 4, "derived an invalid cursor phase: %d", phase);

	if (_cursorStatus) {
		screenShowChar(31 - phase, _cursorPos.x, _cursorPos.y);
		screenRedrawTextArea(_cursorPos.x, _cursorPos.y, 1, 1);
	}
}

void Screen::screenUpdateMoons() {
	int trammelChar, feluccaChar;

	/* show "L?" for the dungeon level */
	if (g_context->_location->_context == CTX_DUNGEON) {
		screenShowChar('L', 11, 0);
		screenShowChar('1' + g_context->_location->_coords.z, 12, 0);
	}
	/* show the current moons (non-combat) */
	else if ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context) {
		trammelChar = (g_ultima->_saveGame->_trammelPhase == 0) ? MOON_CHAR + 7 : MOON_CHAR + g_ultima->_saveGame->_trammelPhase - 1;
		feluccaChar = (g_ultima->_saveGame->_feluccaPhase == 0) ? MOON_CHAR + 7 : MOON_CHAR + g_ultima->_saveGame->_feluccaPhase - 1;

		screenShowChar(trammelChar, 11, 0);
		screenShowChar(feluccaChar, 12, 0);
	}

	screenRedrawTextArea(11, 0, 2, 1);
}

void Screen::screenUpdateWind() {

	/* show the direction we're facing in the dungeon */
	if (g_context->_location->_context == CTX_DUNGEON) {
		screenEraseTextArea(WIND_AREA_X, WIND_AREA_Y, WIND_AREA_W, WIND_AREA_H);
		screenTextAt(WIND_AREA_X, WIND_AREA_Y, "Dir: %5s", getDirectionName((Direction)g_ultima->_saveGame->_orientation));
	}
	/* show the wind direction */
	else if ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context) {
		screenEraseTextArea(WIND_AREA_X, WIND_AREA_Y, WIND_AREA_W, WIND_AREA_H);
		screenTextAt(WIND_AREA_X, WIND_AREA_Y, "Wind %5s", getDirectionName((Direction)g_context->_windDirection));
	}
	screenRedrawTextArea(WIND_AREA_X, WIND_AREA_Y, WIND_AREA_W, WIND_AREA_H);
}

void Screen::screenShowCursor() {
	if (!_cursorStatus && _cursorEnabled) {
		_cursorStatus = 1;
		screenUpdateCursor();
	}
}

void Screen::screenHideCursor() {
	if (_cursorStatus) {
		screenEraseTextArea(_cursorPos.x, _cursorPos.y, 1, 1);
		screenRedrawTextArea(_cursorPos.x, _cursorPos.y, 1, 1);
	}
	_cursorStatus = 0;
}

void Screen::screenEnableCursor(void) {
	_cursorEnabled = 1;
}

void Screen::screenDisableCursor(void) {
	screenHideCursor();
	_cursorEnabled = 0;
}

void Screen::screenSetCursorPos(int x, int y) {
	_cursorPos.x = x;
	_cursorPos.y = y;
}

void Screen::screenFindLineOfSight(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]) {
	int x, y;

	if (!g_context)
		return;

	/*
	 * if the map has the no line of sight flag, all is visible
	 */
	if (g_context->_location->_map->_flags & NO_LINE_OF_SIGHT) {
		for (y = 0; y < VIEWPORT_H; y++) {
			for (x = 0; x < VIEWPORT_W; x++) {
				_los[x][y] = 1;
			}
		}
		return;
	}

	/*
	 * otherwise calculate it from the map data
	 */
	for (y = 0; y < VIEWPORT_H; y++) {
		for (x = 0; x < VIEWPORT_W; x++) {
			_los[x][y] = 0;
		}
	}

	if (settings._lineOfSight == "DOS")
		screenFindLineOfSightDOS(viewportTiles);
	else if (settings._lineOfSight == "Enhanced")
		screenFindLineOfSightEnhanced(viewportTiles);
	else
		error("unknown line of sight style %s!\n", settings._lineOfSight.c_str());
}

void Screen::screenFindLineOfSightDOS(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]) {
	int x, y;

	_los[VIEWPORT_W / 2][VIEWPORT_H / 2] = 1;

	for (x = VIEWPORT_W / 2 - 1; x >= 0; x--)
		if (_los[x + 1][VIEWPORT_H / 2] &&
		    !viewportTiles[x + 1][VIEWPORT_H / 2].front().getTileType()->isOpaque())
			_los[x][VIEWPORT_H / 2] = 1;

	for (x = VIEWPORT_W / 2 + 1; x < VIEWPORT_W; x++)
		if (_los[x - 1][VIEWPORT_H / 2] &&
		    !viewportTiles[x - 1][VIEWPORT_H / 2].front().getTileType()->isOpaque())
			_los[x][VIEWPORT_H / 2] = 1;

	for (y = VIEWPORT_H / 2 - 1; y >= 0; y--)
		if (_los[VIEWPORT_W / 2][y + 1] &&
		    !viewportTiles[VIEWPORT_W / 2][y + 1].front().getTileType()->isOpaque())
			_los[VIEWPORT_W / 2][y] = 1;

	for (y = VIEWPORT_H / 2 + 1; y < VIEWPORT_H; y++)
		if (_los[VIEWPORT_W / 2][y - 1] &&
		    !viewportTiles[VIEWPORT_W / 2][y - 1].front().getTileType()->isOpaque())
			_los[VIEWPORT_W / 2][y] = 1;

	for (y = VIEWPORT_H / 2 - 1; y >= 0; y--) {

		for (x = VIEWPORT_W / 2 - 1; x >= 0; x--) {
			if (_los[x][y + 1] &&
			    !viewportTiles[x][y + 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x + 1][y] &&
			         !viewportTiles[x + 1][y].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x + 1][y + 1] &&
			         !viewportTiles[x + 1][y + 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
		}

		for (x = VIEWPORT_W / 2 + 1; x < VIEWPORT_W; x++) {
			if (_los[x][y + 1] &&
			    !viewportTiles[x][y + 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x - 1][y] &&
			         !viewportTiles[x - 1][y].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x - 1][y + 1] &&
			         !viewportTiles[x - 1][y + 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
		}
	}

	for (y = VIEWPORT_H / 2 + 1; y < VIEWPORT_H; y++) {

		for (x = VIEWPORT_W / 2 - 1; x >= 0; x--) {
			if (_los[x][y - 1] &&
			    !viewportTiles[x][y - 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x + 1][y] &&
			         !viewportTiles[x + 1][y].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x + 1][y - 1] &&
			         !viewportTiles[x + 1][y - 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
		}

		for (x = VIEWPORT_W / 2 + 1; x < VIEWPORT_W; x++) {
			if (_los[x][y - 1] &&
			    !viewportTiles[x][y - 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x - 1][y] &&
			         !viewportTiles[x - 1][y].front().getTileType()->isOpaque())
				_los[x][y] = 1;
			else if (_los[x - 1][y - 1] &&
			         !viewportTiles[x - 1][y - 1].front().getTileType()->isOpaque())
				_los[x][y] = 1;
		}
	}
}

void Screen::screenFindLineOfSightEnhanced(Std::vector<MapTile> viewportTiles[VIEWPORT_W][VIEWPORT_H]) {
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
	    {6, __VCH, 4, _N_CH, 1, __VCH, 3, _N___, 1, ___CH, 1, __VCH, 1}, // raster_1_0
	    {6, __VC_, 1, _NVCH, 2, __VC_, 1, _NVCH, 3, _NVCH, 2, _NVCH, 1}, // raster_1_1
	    //
	    {4, __VCH, 3, _N__H, 1, ___CH, 1, __VCH, 1, 0, 0, 0, 0},         // raster_2_0
	    {6, __VC_, 2, _N_CH, 1, __VCH, 2, _N_CH, 1, __VCH, 1, _N__H, 1}, // raster_2_1
	    {6, __V__, 1, _NVCH, 1, __VC_, 1, _NVCH, 1, __VC_, 1, _NVCH, 1}, // raster_2_2
	    //
	    {2, __VCH, 2, _N__H, 2, 0, 0, 0, 0, 0, 0, 0, 0},     // raster_3_0
	    {3, __VC_, 2, _N_CH, 1, __VCH, 1, 0, 0, 0, 0, 0, 0}, // raster_3_1
	    {3, __VC_, 1, _NVCH, 2, _N_CH, 1, 0, 0, 0, 0, 0, 0}, // raster_3_2
	    {3, _NVCH, 1, __V__, 1, _NVCH, 1, 0, 0, 0, 0, 0, 0}, // raster_3_3
	    //
	    {2, __VCH, 1, _N__H, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // raster_4_0
	    {2, __VC_, 1, _N__H, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // raster_4_1
	    {2, __VC_, 1, _N_CH, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // raster_4_2
	    {2, __V__, 1, _NVCH, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // raster_4_3
	    {2, __V__, 1, _NVCH, 1, 0, 0, 0, 0, 0, 0, 0, 0}  // raster_4_4
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
			xSign = 1;
			ySign = 1;
			reflect = false;
			break; // lower-right
		case 1:
			xSign = 1;
			ySign = 1;
			reflect = true;
			break;
		case 2:
			xSign = 1;
			ySign = -1;
			reflect = true;
			break; // lower-left
		case 3:
			xSign = -1;
			ySign = 1;
			reflect = false;
			break;
		case 4:
			xSign = -1;
			ySign = -1;
			reflect = false;
			break; // upper-left
		case 5:
			xSign = -1;
			ySign = -1;
			reflect = true;
			break;
		case 6:
			xSign = -1;
			ySign = 1;
			reflect = true;
			break; // upper-right
		case 7:
			xSign = 1;
			ySign = -1;
			reflect = false;
			break;
		}

		// determine the origin point for the current LOS octant
		xOrigin = VIEWPORT_W / 2;
		yOrigin = VIEWPORT_H / 2;

		// make sure the segment doesn't reach out of bounds
		int maxWidth = xOrigin;
		int maxHeight = yOrigin;
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
						currentRaster = 13; // currentCol and currentRow must equal 4
					}

					xTileOffset = 0;
					yTileOffset = 0;

					//========================================
					for (int currentSegment = 0; currentSegment < shadowRaster[currentRaster][0]; currentSegment++) {
						// each shadow segment is 2 bytes
						int shadowType = shadowRaster[currentRaster][currentSegment * 2 + 1];
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
								_los[xTile + ((yTileOffset)*ySign)][yTile + ((currentShadow + xTileOffset) * xSign)] |= shadowType;
							} else {
								_los[xTile + ((currentShadow + xTileOffset) * xSign)][yTile + ((yTileOffset)*ySign)] |= shadowType;
							}
						}
						xTileOffset += shadowLength;
					} // for (int currentSegment = 0; currentSegment < shadowRaster[currentRaster][0]; currentSegment++)
					  //========================================

				} // if (viewportTiles[xTile][yTile].front().getTileType()->isOpaque())
			}     // for (int currentRow = 0; currentRow <= currentCol; currentRow++)
		}         // for (int currentCol = 1; currentCol <= _NUM_RASTERS_COLS; currentCol++)
	}             // for (octant = 0; octant < _OCTANTS; octant++)

	// go through all tiles on the viewable area and set the appropriate visibility
	for (y = 0; y < VIEWPORT_H; y++) {
		for (x = 0; x < VIEWPORT_W; x++) {
			// if the shadow flags equal __VCH, hide it, otherwise it's fully visible
			//
			if ((_los[x][y] & __VCH) == __VCH) {
				_los[x][y] = 0;
			} else {
				_los[x][y] = 1;
			}
		}
	}
}

void Screen::screenGetLineTerms(int x1, int y1, int x2, int y2, double *a, double *b) {
	if (x2 - x1 == 0) {
		*a = DBL_MAX;
		*b = x1;
	} else {
		*a = ((double)(y2 - y1)) / ((double)(x2 - x1));
		*b = y1 - ((*a) * x1);
	}
}

int Screen::screenPointsOnSameSideOfLine(int x1, int y1, int x2, int y2, double a, double b) {
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

int Screen::screenPointInTriangle(int x, int y, int tx1, int ty1, int tx2, int ty2, int tx3, int ty3) {
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

int Screen::screenPointInMouseArea(int x, int y, const MouseArea *area) {
	assertMsg(area->_nPoints == 2 || area->_nPoints == 3, "unsupported number of points in area: %d", area->_nPoints);

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

void Screen::screenRedrawMapArea() {
	g_game->_mapArea.update();
}

void Screen::screenEraseMapArea() {
	Image *screen = imageMgr->get("screen")->_image;
	screen->fillRect(BORDER_WIDTH * settings._scale,
	                 BORDER_WIDTH * settings._scale,
	                 VIEWPORT_W * TILE_WIDTH * settings._scale,
	                 VIEWPORT_H * TILE_HEIGHT * settings._scale,
	                 0, 0, 0);
}

void Screen::screenEraseTextArea(int x, int y, int width, int height) {
	Image *screen = imageMgr->get("screen")->_image;
	screen->fillRect(x * CHAR_WIDTH * settings._scale,
	                 y * CHAR_HEIGHT * settings._scale,
	                 width * CHAR_WIDTH * settings._scale,
	                 height * CHAR_HEIGHT * settings._scale,
	                 0, 0, 0);
}

void Screen::screenShake(int iterations) {
	if (settings._screenShakes) {
		// specify the size of the shake
		const int SHAKE_OFFSET = 1 * settings._scale;

		for (int i = 0; i < iterations; ++i) {
			// Shift the screen down
			g_system->setShakePos(0, SHAKE_OFFSET);
			g_system->updateScreen();
			EventHandler::sleep(settings._shakeInterval);

			// shift the screen back up
			g_system->setShakePos(0, 0);
			g_system->updateScreen();
			EventHandler::sleep(settings._shakeInterval);
		}
	}
}

void Screen::screenShowGemTile(Layout *layout, Map *map, MapTile &t, bool focus, int x, int y) {
	// Make sure we account for tiles that look like other tiles (dungeon tiles, mainly)
	Common::String looks_like = t.getTileType()->getLooksLike();
	if (!looks_like.empty())
		t = map->_tileSet->getByName(looks_like)->getId();

	uint tile = map->translateToRawTileIndex(t);

	if (map->_type == Map::DUNGEON) {
		assertMsg(_charSetInfo, "charset not initialized");
		Common::HashMap<Common::String, int>::iterator charIndex = _dungeonTileChars.find(t.getTileType()->getName());
		if (charIndex != _dungeonTileChars.end()) {
			_charSetInfo->_image->drawSubRect((layout->_viewport.left + (x * layout->_tileShape.x)) * settings._scale,
			                                  (layout->_viewport.top + (y * layout->_tileShape.y)) * settings._scale,
			                                  0,
			                                  charIndex->_value * layout->_tileShape.y * settings._scale,
			                                  layout->_tileShape.x * settings._scale,
			                                  layout->_tileShape.y * settings._scale);
		}
	} else {
		if (_gemTilesInfo == nullptr) {
			_gemTilesInfo = imageMgr->get(BKGD_GEMTILES);
			if (!_gemTilesInfo)
				error("ERROR 1002: Unable to load the \"%s\" data file", BKGD_GEMTILES);
		}

		if (tile < 128) {
			_gemTilesInfo->_image->drawSubRect((layout->_viewport.left + (x * layout->_tileShape.x)) * settings._scale,
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

Layout *Screen::screenGetGemLayout(const Map *map) {
	if (map->_type == Map::DUNGEON) {
		Std::vector<Layout *>::const_iterator i;
		for (i = _layouts.begin(); i != _layouts.end(); i++) {
			Layout *layout = *i;

			if (layout->_type == LAYOUT_DUNGEONGEM)
				return layout;
		}
		error("no dungeon gem layout found!\n");
		return nullptr;
	} else
		return _gemLayout;
}

void Screen::screenGemUpdate() {
	MapTile tile;
	int x, y;
	Image *screen = imageMgr->get("screen")->_image;

	screen->fillRect(BORDER_WIDTH * settings._scale,
	                 BORDER_HEIGHT * settings._scale,
	                 VIEWPORT_W * TILE_WIDTH * settings._scale,
	                 VIEWPORT_H * TILE_HEIGHT * settings._scale,
	                 0, 0, 0);

	Layout *layout = screenGetGemLayout(g_context->_location->_map);

	// TODO: Move the code responsible for determining 'peer' visibility to a non SDL specific part of the code.
	if (g_context->_location->_map->_type == Map::DUNGEON) {
		//DO THE SPECIAL DUNGEON MAP TRAVERSAL
		Std::vector<Std::vector<int> > drawnTiles(layout->_viewport.width(), Std::vector<int>(layout->_viewport.height(), 0));
		Common::List<Common::Pair<int, int> > coordStack;

		//Put the avatar's position on the stack
		int center_x = layout->_viewport.width() / 2 - 1;
		int center_y = layout->_viewport.height() / 2 - 1;
		int avt_x = g_context->_location->_coords.x - 1;
		int avt_y = g_context->_location->_coords.y - 1;

		coordStack.push_back(Common::Pair<int, int>(center_x, center_y));
		bool weAreDrawingTheAvatarTile = true;

		//And draw each tile on the growing stack until it is empty
		while (coordStack.size() > 0) {
			Common::Pair<int, int> currentXY = coordStack.back();
			coordStack.pop_back();

			x = currentXY.first;
			y = currentXY.second;

			if (x < 0 || x >= layout->_viewport.width() ||
			    y < 0 || y >= layout->_viewport.height())
				continue; //Skip out of range tiles

			if (drawnTiles[x][y])
				continue; //Skip already considered tiles

			drawnTiles[x][y] = 1;

			// DRAW THE ACTUAL TILE
			bool focus;

			Std::vector<MapTile> tiles = screenViewportTile(layout->_viewport.width(),
			                                                layout->_viewport.height(), x - center_x + avt_x, y - center_y + avt_y, focus);
			tile = tiles.front();

			TileId avatarTileId = g_context->_location->_map->_tileSet->getByName("avatar")->getId();

			if (!weAreDrawingTheAvatarTile) {
				//Hack to avoid showing the avatar tile multiple times in cycling dungeon maps
				if (tile.getId() == avatarTileId)
					tile = g_context->_location->_map->getTileFromData(g_context->_location->_coords)->getId();
			}

			screenShowGemTile(layout, g_context->_location->_map, tile, focus, x, y);

			if (!tile.getTileType()->isOpaque() || tile.getTileType()->isWalkable() || weAreDrawingTheAvatarTile) {
				//Continue the search so we can see through all walkable objects, non-opaque objects (like creatures)
				//or the avatar position in those rare circumstances where he is stuck in a wall

				//by adding all relative adjacency combinations to the stack for drawing
				coordStack.push_back(Common::Pair<int, int>(x + 1, y - 1));
				coordStack.push_back(Common::Pair<int, int>(x + 1, y));
				coordStack.push_back(Common::Pair<int, int>(x + 1, y + 1));

				coordStack.push_back(Common::Pair<int, int>(x, y - 1));
				coordStack.push_back(Common::Pair<int, int>(x, y + 1));

				coordStack.push_back(Common::Pair<int, int>(x - 1, y - 1));
				coordStack.push_back(Common::Pair<int, int>(x - 1, y));
				coordStack.push_back(Common::Pair<int, int>(x - 1, y + 1));

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
				                          x, y, focus)
				           .front();
				screenShowGemTile(layout, g_context->_location->_map, tile, focus, x, y);
			}
		}
	}

	screenRedrawMapArea();

	screenUpdateCursor();
	screenUpdateMoons();
	screenUpdateWind();
}

void Screen::screenRedrawTextArea(int x, int y, int width, int height) {
	g_system->updateScreen();
}

void Screen::screenWait(int numberOfAnimationFrames) {
	update();
	g_system->delayMillis(numberOfAnimationFrames * SCREEN_FRAME_TIME);
}

Image *Screen::screenScale(Image *src, int scale, int n, int filter) {
	Image *dest = nullptr;
	bool isTransparent;
	uint transparentIndex;
	bool alpha = src->isAlphaOn();

	if (n == 0)
		n = 1;

	isTransparent = src->getTransparentIndex(transparentIndex);
	src->alphaOff();

	while (filter && _filterScaler && (scale % 2 == 0)) {
		dest = (*_filterScaler)(src, 2, n);
		src = dest;
		scale /= 2;
	}
	if (scale == 3 && scaler3x(settings._filter)) {
		dest = (*_filterScaler)(src, 3, n);
		src = dest;
		scale /= 3;
	}

	if (scale != 1)
		dest = (*scalerGet("point"))(src, scale, n);

	if (!dest)
		dest = Image::duplicate(src);

	if (isTransparent)
		dest->setTransparentIndex(transparentIndex);

	if (alpha)
		src->alphaOn();

	return dest;
}

Image *Screen::screenScaleDown(Image *src, int scale) {
	int x, y;
	Image *dest;
	bool isTransparent;
	uint transparentIndex;
	bool alpha = src->isAlphaOn();

	isTransparent = src->getTransparentIndex(transparentIndex);

	src->alphaOff();

	dest = Image::create(src->width() / scale, src->height() / scale, src->isIndexed(), Image::HARDWARE);
	if (!dest)
		return nullptr;

	if (dest->isIndexed())
		dest->setPaletteFromImage(src);

	for (y = 0; y < src->height(); y += scale) {
		for (x = 0; x < src->width(); x += scale) {
			uint index;
			src->getPixelIndex(x, y, index);
			dest->putPixelIndex(x / scale, y / scale, index);
		}
	}

	if (isTransparent)
		dest->setTransparentIndex(transparentIndex);

	if (alpha)
		src->alphaOn();

	return dest;
}

#ifdef IOS_ULTIMA4
//Unsure if implementation required in iOS.
void inline screenLock(){};
void inline screenUnlock(){};
void inline screenWait(int numberOfAnimationFrames){};
#endif

const Std::vector<Common::String> &screenGetFilterNames() {
	return g_screen->_filterNames;
}

const Std::vector<Common::String> &screenGetGemLayoutNames() {
	return g_screen->_gemLayoutNames;
}

const Std::vector<Common::String> &screenGetLineOfSightStyles() {
	return g_screen->_lineOfSightStyles;
}

} // End of namespace Ultima4
} // End of namespace Ultima
