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

#include "common/system.h"
#include "common/config-manager.h"
#include "image/bmp.h"
#include "graphics/paletteman.h"
#include "bagel/hodjnpodj/fuge/fuge.h"
#include "bagel/hodjnpodj/fuge/defines.h"
#include "bagel/hodjnpodj/boflib/vector.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/size.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

struct BRICK_VECTORS {
    VECTOR v1, v2;
};

static BRICK_VECTORS aBrickVectors[BRICKS_PER_ROW] = {
    {{-1,  1}, { 5, -2}},
    {{-5,  2}, { 1,  0}},
    {{-1,  0}, { 5,  2}},
    {{-5, -2}, { 1,  1}},
    {{-1, -1}, { 2,  5}},
    {{-2, -5}, { 0,  1}},
    {{ 0, -1}, {-2,  5}},
    {{ 2, -5}, {-1,  1}},
    {{ 1, -1}, {-5,  2}},
    {{ 5, -2}, {-1,  0}},
    {{ 1,  0}, {-5, -2}},
    {{ 5,  2}, {-1, -1}},
    {{ 1,  1}, {-2, -5}},
    {{ 2,  5}, { 0, -1}},
    {{ 0,  1}, { 2, -5}},
    {{-2,  5}, { 1, -1}}
};

static VECTOR vBrickCritPoints[N_ROWS][N_BRICK_POINTS] = {
    {{  0, -191}, {  8, -191}, { 15, -190}, { 23, -190}, { 31, -188},
     { 39, -187}, { 47, -185}, { 55, -183}, { 62, -181}, { 69, -178},
     { 67, -172}, { 64, -165}, { 56, -168}, { 48, -171}, { 40, -173},
     { 32, -174}, { 24, -175}, { 16, -176}, {  8, -177}, {  0, -177},
     {  0, -184}},

    {{  0, -175}, {  7, -175}, { 14, -175}, { 21, -174}, { 28, -173},
     { 35, -172}, { 42, -170}, { 49, -168}, { 56, -166}, { 63, -164},
     { 61, -158}, { 59, -152}, { 52, -154}, { 44, -157}, { 36, -159},
     { 29, -160}, { 21, -162}, { 14, -162}, { 7, -163},  {  0, -163},
     {  0, -169}},

   {{  0, -161}, {  7, -161}, { 13, -160}, { 19, -160}, { 26, -159},
    { 33, -158}, { 39, -156}, { 45, -155}, { 51, -153}, { 58, -150},
    { 56, -143}, { 54, -137}, { 48, -139}, { 41, -141}, { 35, -143},
    { 28, -144}, { 21, -146}, { 14, -146}, {  7, -147}, {  0, -147},
    {  0, -154}},

   {{  0, -145}, {  7, -145}, { 13, -145}, { 19, -144}, { 25, -143},
    { 31, -142}, { 36, -141}, { 41, -139}, { 47, -138}, { 53, -135},
    { 51, -129}, { 48, -122}, { 42, -124}, { 37, -126}, { 31, -127},
    { 26, -129}, { 19, -130}, { 13, -130}, {  7, -131}, {  0, -131},
    {  0, -138}},

   {{  0, -129}, {  6, -129}, { 11, -129}, { 16, -128}, { 21, -127},
    { 26, -127}, { 31, -125}, { 36, -124}, { 41, -123}, { 47, -121},
    { 45, -115}, { 43, -109}, { 37, -111}, { 32, -112}, { 27, -114},
    { 22, -115}, { 17, -116}, { 11, -116}, {  6, -117}, {  0, -117},
    {0, -123}},

   {{  0, -115}, {  6, -115}, { 11, -115}, { 16, -114}, { 20, -113},
    { 25, -112}, { 29, -111}, { 33, -110}, { 37, -109}, { 42, -107},
    { 40, -102}, { 38,  -96}, { 33,  -98}, { 29,  -99}, { 25, -100},
    { 21, -101}, { 16, -102}, { 11, -102}, {  6, -103}, {  0, -103},
    {  0, -109}}
};

static POINT ptBrickPos[N_BRICKS] = {
    {184,  62}, {250,  48}, {319,  48}, {385,  62},         // Purple Bricks
    {445, 104}, {483, 165}, {483, 240}, {445, 309},
    {385, 365}, {321, 404}, {250, 404}, {184, 365},
    {144, 309}, {128, 240}, {128, 164}, {144, 105},

    {195,  76}, {256,  64}, {319,  64}, {380,  76},         // Blue Bricks
    {435, 115}, {469, 171}, {469, 240}, {435, 304},
    {380, 355}, {321, 391}, {256, 391}, {195, 355},
    {158, 304}, {143, 240}, {143, 171}, {158, 115},

    {206,  90}, {261,  78}, {319,  78}, {375,  90},         // Green Bricks
    {424, 126}, {455, 177}, {455, 240}, {424, 298},
    {375, 344}, {321, 376}, {261, 376}, {206, 344},
    {171, 298}, {158, 240}, {158, 177}, {171, 126},

    {217, 104}, {266,  94}, {319,  94}, {369, 104},         // Yellow Bricks
    {413, 137}, {441, 183}, {441, 240}, {413, 292},
    {369, 333}, {321, 361}, {266, 361}, {217, 333},
    {185, 292}, {173, 240}, {173, 183}, {185, 137},

    {228, 119}, {272, 110}, {319, 110}, {364, 119},         // Orange Bricks
    {402, 148}, {427, 189}, {427, 240}, {402, 286},
    {364, 322}, {321, 348}, {272, 348}, {228, 322},
    {200, 286}, {189, 240}, {189, 189}, {200, 148},

    {238, 133}, {277, 124}, {319, 124}, {359, 133},         // Red Bricks
    {392, 158}, {414, 195}, {414, 240}, {392, 280},
    {359, 312}, {321, 335}, {277, 335}, {238, 312},
    {214, 280}, {204, 240}, {204, 195}, {214, 158}
};

static SIZE ptBrickSize[N_BRICKS] = {
    {70, 52}, {68, 27}, {70, 27}, {70, 52}, {50, 66}, {28, 74},
    {28, 75}, {50, 66}, {70, 52}, {68, 27}, {70, 27}, {69, 52},
    {50, 66}, {28, 74}, {28, 75}, {50, 65}, {64, 48}, {62, 24},
    {64, 24}, {64, 48}, {46, 60}, {27, 68}, {27, 68}, {46, 60},
    {64, 48}, {62, 24}, {64, 24}, {64, 48}, {46, 60}, {27, 68},
    {27, 68}, {46, 60}, {58, 45}, {57, 25}, {59, 25}, {58, 45},
    {44, 55}, {26, 62}, {26, 62}, {44, 55}, {58, 45}, {57, 25},
    {59, 25}, {58, 45}, {44, 55}, {26, 62}, {26, 62}, {44, 55},
    {53, 42}, {52, 24}, {54, 24}, {53, 42}, {41, 50}, {25, 56},
    {25, 56}, {41, 50}, {53, 42}, {52, 24}, {54, 24}, {53, 42},
    {41, 50}, {25, 56}, {25, 56}, {41, 50}, {47, 38}, {46, 21},
    {48, 21}, {47, 38}, {37, 45}, {23, 50}, {23, 50}, {37, 45},
    {47, 38}, {46, 21}, {48, 21}, {47, 38}, {37, 45}, {23, 50},
    {23, 50}, {37, 45}, {42, 34}, {41, 20}, {43, 20}, {42, 34},
    {33, 41}, {21, 44}, {21, 44}, {33, 41}, {42, 34}, {41, 20},
    {43, 20}, {42, 34}, {33, 41}, {21, 44}, {21, 44}, {33, 41}
};

static const char *pszFugeArt[N_ROWS + 1] = {
	"ART/FUGE6.BMP",
	"ART/FUGE1.BMP",
	"ART/FUGE2.BMP",
	"ART/FUGE3.BMP",
	"ART/FUGE4.BMP",
	"ART/FUGE5.BMP",
	"ART/FUGE6.BMP"
};

#define N_PADDLE_SIZES (PSIZE_MAX + 1)

static const char *pszPaddles[N_PADDLE_SIZES] = {
	"ART/PADCEL45.BMP",
	"ART/PADCEL60.BMP",
	"ART/PADCEL90.BMP"
};

static double fPaddleAngles[N_PADDLE_SIZES] = {
	PADDLE0_ANGLE,
	PADDLE1_ANGLE,
	PADDLE2_ANGLE
};

Fuge::Fuge() : View("Fuge"), m_GamePalette(0),
		m_rNewGameButton(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
			NEWGAME_LOCATION_X + NEWGAME_WIDTH, NEWGAME_LOCATION_Y + NEWGAME_HEIGHT),
		m_ptOrigin(GAME_WIDTH / 2, GAME_HEIGHT / 2) {
	clear();
}

void Fuge::clear() {
	m_nInitNumBalls = BALLS_DEF;
	m_nInitStartLevel = LEVEL_DEF;
	m_nInitBallSpeed = SPEED_DEF;
	m_nInitPaddleSize = PSIZE_DEF;
	m_nGForceFactor = GFORCE_DEF;

	m_pSoundTrack = NULL;
	m_bPause = FALSE;
	m_bGameActive = FALSE;
	m_bIgnoreScrollClick = FALSE;
	m_bBallOnPaddle = FALSE;
	m_nPaddleCelIndex = 29;
	m_pPaddle = NULL;
	m_pBall = NULL;
	m_bMovingPaddle = FALSE;
	m_lScore = 0;
	m_pBrickSound = NULL;
	m_pWallSound = NULL;
	m_pPaddleSound = NULL;
	m_pExtraLifeSound = NULL;
	m_hBrickRes = NULL;
	m_hWallRes = NULL;
	m_hPaddleRes = NULL;
	m_hExtraLifeRes = NULL;
	m_nNumRows = 0;
	m_bJoyActive = FALSE;
	Common::fill(m_bBrickVisible, m_bBrickVisible + N_BRICKS, false);
}

bool Fuge::msgOpen(const OpenMessage &msg) {
	// Add the minigame's folder to the search path
	Common::FSNode gamePath(ConfMan.getPath("path"));
	SearchMan.addDirectory("minigame", gamePath.getChild("fuge"), 0, 2);

	// Clear fields
	clear();

	// Get the game palette from a bitmap
	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open(MINI_GAME_MAP) || !decoder.loadStream(f))
		error("Could not load %s", MINI_GAME_MAP);

	m_GamePalette = Graphics::Palette(decoder.getPalette(), decoder.getPaletteColorCount());
	g_system->getPaletteManager()->setPalette(m_GamePalette);

	return true;
}

bool Fuge::msgClose(const CloseMessage &msg) {
	// Remove the SearchMan reference to our subfolder
	SearchMan.remove("minigame");

	// Clear bitmaps
	_background.clear();
	_backgroundNumRows = -1;

	return true;
}

bool Fuge::msgKeypress(const KeypressMessage &msg) {
	return false;
}

void Fuge::draw() {
	paintBricks();
	repaintSpriteList();

}

void Fuge::paintBricks() {
	assert((m_nNumRows >= 0) && (m_nNumRows <= N_ROWS));

	if (m_nNumRows != _backgroundNumRows) {
		// Change the background bitmap
		_backgroundNumRows = m_nNumRows;

		Image::BitmapDecoder decoder;
		Common::File f;
		if (!f.open(pszFugeArt[m_nNumRows]) || !decoder.loadStream(f))
			error("Could not load %s", MINI_GAME_MAP);

		_background.copyFrom(*decoder.getSurface());
	}

	// Draw the background
	GfxSurface s = getSurface();
	s.blitFrom(_background);

	// Erase all the bricks that are hidden
	if (m_nNumRows != 0) {
		for (int brickIndex = 0; brickIndex < N_BRICKS; brickIndex++) {
			if (!m_bBrickVisible[brickIndex]) {
				// Remove this brick from the screen
				s.floodFill(ptBrickPos[brickIndex].x + ptBrickSize[brickIndex].cx / 2,
					ptBrickPos[brickIndex].y + ptBrickSize[brickIndex].cy / 2,
					255);
			}
		}
	}
}

void Fuge::repaintSpriteList() {

}

bool Fuge::tick() {
	return true;
}

void Fuge::eraseBrick(int brickIndex) {
	assert((brickIndex >= 0) && (brickIndex < N_BRICKS));
	m_bBrickVisible[brickIndex] = false;
	redraw();
}


} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel
