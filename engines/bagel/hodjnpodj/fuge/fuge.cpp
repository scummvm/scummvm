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
#include "bagel/hodjnpodj/libs/vector.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/size.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/hodjnpodj/views/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

#define CSOUND 1

struct BRICK_VECTORS {
    VECTOR v1, v2;
};

static const BRICK_VECTORS aBrickVectors[BRICKS_PER_ROW] = {
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

static const POINT ptBrickPos[N_BRICKS] = {
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

static const SIZE ptBrickSize[N_BRICKS] = {
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
	"FUGE/ART/FUGE6.BMP",
	"FUGE/ART/FUGE1.BMP",
	"FUGE/ART/FUGE2.BMP",
	"FUGE/ART/FUGE3.BMP",
	"FUGE/ART/FUGE4.BMP",
	"FUGE/ART/FUGE5.BMP",
	"FUGE/ART/FUGE6.BMP"
};

#define N_PADDLE_SIZES (PSIZE_MAX + 1)

static const char *pszPaddles[N_PADDLE_SIZES] = {
	"FUGE/ART/PADCEL45.BMP",
	"FUGE/ART/PADCEL60.BMP",
	"FUGE/ART/PADCEL90.BMP"
};

static double fPaddleAngles[N_PADDLE_SIZES] = {
	PADDLE0_ANGLE,
	PADDLE1_ANGLE,
	PADDLE2_ANGLE
};

Fuge::Fuge() : MinigameView("Fuge", "fuge/hnpfuge.dll"), m_GamePalette(0),
		m_rNewGameButton(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
			NEWGAME_LOCATION_X + NEWGAME_WIDTH, NEWGAME_LOCATION_Y + NEWGAME_HEIGHT),
		m_ScrollButton("ScrollButton", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY)),
		m_ptOrigin(GAME_WIDTH / 2, GAME_HEIGHT / 2),
		_gvCenter(CENTER_X, CENTER_Y),
		m_pPaddle(this),
		m_pBall(this) {

	clear();

	// The vector table is rotated by 11 or so degrees, because it
	// was easier to type in (125, 320) as opposed to (114.452865, 300.526372).
	// So the conversion is done here.
	realignVectors();

	// Add mappings to resource entries
	addResource("fuge/art/ball.bmp", 103);
	addResource("fuge/sound/newlife.wav", "brickSound");
}

void Fuge::clear() {
	m_nInitNumBalls = BALLS_DEF;
	m_nInitStartLevel = LEVEL_DEF;
	m_nInitBallSpeed = SPEED_DEF;
	m_nInitPaddleSize = PSIZE_DEF;
	m_nGForceFactor = GFORCE_DEF;

	m_pSoundTrack = NULL;
	m_bPause = false;
	m_bGameActive = false;
	m_bIgnoreScrollClick = false;
	m_bBallOnPaddle = false;
	m_nPaddleCelIndex = 29;
	m_bMovingPaddle = false;
	m_lScore = 0;
	m_pBrickSound = NULL;
	m_pWallSound = NULL;
	m_pPaddleSound = NULL;
	m_pExtraLifeSound = NULL;
	m_nNumRows = 0;
	m_bJoyActive = false;
	Common::fill(m_bBrickVisible, m_bBrickVisible + N_BRICKS, false);
}

bool Fuge::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	// Clear fields
	clear();

	// Get the game palette from a bitmap
	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open(MINI_GAME_MAP) || !decoder.loadStream(f))
		error("Could not load %s", MINI_GAME_MAP);

	m_GamePalette = Graphics::Palette(decoder.getPalette(), decoder.getPaletteColorCount());
	g_system->getPaletteManager()->setPalette(m_GamePalette);

	// Load scroll button
	m_ScrollButton.loadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);

	// Start the Fuge soundtrack
	if (gameInfo.bMusicEnabled) {
		m_pSoundTrack = new CBofSound(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		m_pSoundTrack->midiLoopPlaySegment(5390, 32280, 0, FMT_MILLISEC);
	}

	// Load data
	loadMasterSprites();
	loadMasterSounds();

	if (gameInfo.bPlayingMetagame) {
		playGame();
	} else {
		draw();
		showOptionsMenu();
	}

	return true;
}

bool Fuge::msgClose(const CloseMessage &msg) {
	MinigameView::msgClose(msg);

	// Clear bitmaps
	_background.clear();
	m_ScrollButton.clear();
	m_pBall.clear();
	m_pPaddle.clear();
	_backgroundNumRows = -1;
	_paddleOldSize = -1;

	// Clear sounds
	delete m_hBrickRes;
	delete m_hWallRes;
	delete m_hPaddleRes;
	delete m_hExtraLifeRes;
	m_hBrickRes = nullptr;
	m_hWallRes = nullptr;
	m_hPaddleRes = nullptr;
	m_hExtraLifeRes = nullptr;

	return true;
}

bool Fuge::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	// Move paddle clockwise
	case Common::KEYCODE_UP:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_KP6:
		if (m_bGameActive) {
			m_nPaddleCelIndex += PADDLE_CEL_JUMP;
			paintPaddle(false);
		}
		break;

	// Move paddle counter-clockwise
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_KP4:
		if (m_bGameActive) {
			m_nPaddleCelIndex -= PADDLE_CEL_JUMP;
			paintPaddle(false);
		}
		break;

	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_SPACE:
		if (m_bGameActive) {
			if (m_bBallOnPaddle) {
				launchBall();
			}
		}
		break;

	// Bring up the Rules
	case Common::KEYCODE_F1: {
		gamePause();
		CBofSound::waitWaveSounds();
		Rules::show("fuge.txt",
			(gameInfo.bSoundEffectsEnabled ? WAV_NARRATION : NULL),
			[]() {
				((Fuge *)g_events->findView("Fuge"))->gameResume();
			});
		break;
	}

	//
	// Bring up the options menu
	case Common::KEYCODE_F2:
		showOptionsMenu();
		break;

	case Common::KEYCODE_SCROLLOCK:
		if (m_bGameActive) {
			if (!m_bPause) {
				gamePause();
			} else {
				gameResume();
			}
		}
		break;

	default:
		return MinigameView::msgKeypress(msg);
		break;
	}

	redraw();
	return true;
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
	for (int brickIndex = 0;
			brickIndex < (m_nNumRows * BRICKS_PER_ROW);
			brickIndex++) {
		if (!m_bBrickVisible[brickIndex]) {
			// Remove this brick from the screen
			s.floodFill(ptBrickPos[brickIndex].x + ptBrickSize[brickIndex].cx / 2,
				ptBrickPos[brickIndex].y + ptBrickSize[brickIndex].cy / 2,
				255);
		}
	}
}

void Fuge::repaintSpriteList() {
	drawSprites();
}

bool Fuge::tick() {
	// Continue as long as there is a currently active non-paused game
	if (m_bGameActive && !m_bPause) {
		if (++_timerCtr >= 5) {
			moveBall();
			redraw();
		}
	}

	return true;
}

void Fuge::eraseBrick(int brickIndex) {
	assert((brickIndex >= 0) && (brickIndex < N_BRICKS));
	m_bBrickVisible[brickIndex] = false;
	redraw();
}

void Fuge::realignVectors() {
	CVector vTmp;
	int i, j;

	for (i = 0; i < N_ROWS; i++) {

		for (j = 0; j < N_BRICK_POINTS; j++) {
			vTmp = vBrickCritPoints[i][j];
			vTmp.Rotate(2 * PI / BRICKS_PER_ROW * -2);
			vBrickCritPoints[i][j] = vTmp;
		}
	}
}


void Fuge::loadMasterSprites() {
	loadNewPaddle(m_nInitPaddleSize);

	m_pBall.loadBitmap(BALL_BMP);
	m_pBall.setTransparentColor(255);
}

void Fuge::loadNewPaddle(int nNewSize) {
	assert(nNewSize >= PSIZE_MIN && nNewSize <= SIZE_MAX);

	// Don't try to load the same paddle
	if (_paddleOldSize != nNewSize) {
		_paddleOldSize = nNewSize;
		m_pPaddle.loadCels(pszPaddles[nNewSize], N_PADDLE_CELS);
	}
}

void Fuge::loadMasterSounds() {
	// Load the Brick "ping" into memory
	{
		Common::File f;
		if (!f.open(WAV_BRICK))
			error("Error loading sound - %s", WAV_BRICK);
		m_hBrickRes = f.readStream(f.size());
	}

	// Load the wall "ping" into memory
	{
		Common::File f;
		if (!f.open(WAV_WALL))
			error("Error loading sound - %s", WAV_WALL);
		m_hWallRes = f.readStream(f.size());
	}

	// Load the paddle "ping" into memory
	{
		Common::File f;
		if (!f.open(WAV_PADDLE))
			error("Error loading sound - %s", WAV_PADDLE);
		m_hPaddleRes = f.readStream(f.size());
	}

	// Load the extra life sound into memory
	{
		Common::File f;
		if (!f.open(WAV_NEWLIFE))
			error("Error loading sound - %s", WAV_NEWLIFE);
		m_hExtraLifeRes = f.readStream(f.size());
	}
}

void Fuge::showOptionsMenu() {
	m_ScrollButton.setPressed(true);

	if (!m_bIgnoreScrollClick) {
		m_bIgnoreScrollClick = true;

		gamePause();
		CBofSound::waitWaveSounds();

		// Show the options view
		MainMenu::show([]() {
			((Fuge *)g_events->findView("Fuge"))->optionsClosed();
		});
	}
}

void Fuge::gamePause() {
	m_bPause = true;
}

void Fuge::gameResume() {
	if (!m_bBallOnPaddle && m_bMovingPaddle)
		m_bPause = false;
}

void Fuge::playGame() {
	// load the .INI settings
	loadIniSettings();

	// reset all game parameters
	gameReset();

	loadNewPaddle(m_nInitPaddleSize);

	// Start game
	startBricks();
	startPaddle();
	startBall();

	// game starts paused
	m_bPause = true;
	m_bGameActive = true;
}

void Fuge::gameReset() {
	if (gameInfo.bSoundEffectsEnabled) {
		BofPlaySound(NULL, SOUND_ASYNCH);	// Stop all sounds
	}

	endBricks();	// remove all bricks from sprite chain
	endBall();		// remove ball from sprite chain
	endPaddle();    // remove paddle from sprite chain

	m_fTurboBoost = 0.0;	// no turbo
	m_lExtraLifeScore = EXTRA_LIFE_SCORE; // user needs this many points for an extra life
	m_bGameActive = false;	// there is no currently active game
	m_bPause = false;		// the game is not paused
	m_bBallOnPaddle = false;			// Ball is not yet on paddle
	m_bMovingPaddle = false;			// user is not moving the paddle
	m_nBalls = m_nInitNumBalls;			// reset # of balls
	m_nBallSpeed = m_nInitBallSpeed;	// reset ball speed

	m_nNumRows = m_nInitStartLevel;				// reset number of brick rows
	m_nBricks = m_nNumRows * BRICKS_PER_ROW;	// get new brick count
	m_lScore = 0;                               // reset the score
	m_bPaddleHit = false;                       // paddle starts fresh
}

void Fuge::loadIniSettings() {
	if (gameInfo.bPlayingMetagame) {
		m_bOutterWall = false;
		m_nInitNumBalls = 1;
		m_nInitStartLevel = 3;
		m_nGForceFactor = GFORCE_DEF;
		m_nInitPaddleSize = PSIZE_MAX;

		switch (gameInfo.nSkillLevel) {
		case SKILLLEVEL_LOW:
			m_nInitBallSpeed = 4;
			break;

		case SKILLLEVEL_MEDIUM:
			m_nInitBallSpeed = 6;
			break;

		default:
			assert(gameInfo.nSkillLevel == SKILLLEVEL_HIGH);
			m_nInitBallSpeed = 8;
			break;
		}

	} else {
		Common::String domain = ConfMan.getActiveDomainName();
		ConfMan.setActiveDomain(INI_SECTION);

		m_nInitNumBalls = ConfMan.hasKey("NumberOfBalls") ?
			ConfMan.getInt("NumberOfBalls") : BALLS_DEF;
		if ((m_nInitNumBalls < BALLS_MIN) || (m_nInitNumBalls > BALLS_MAX))
			m_nInitNumBalls = BALLS_DEF;

		m_nInitStartLevel = ConfMan.hasKey("StartingLevel") ?
			ConfMan.getInt("StartingLevel") : LEVEL_DEF;
		if ((m_nInitStartLevel < LEVEL_MIN) || (m_nInitStartLevel > LEVEL_MAX))
			m_nInitStartLevel = LEVEL_DEF;

		m_nInitBallSpeed = ConfMan.hasKey("BallSpeed") ?
			ConfMan.getInt("BallSpeed") : SPEED_DEF;
		if ((m_nInitBallSpeed < SPEED_MIN) || (m_nInitBallSpeed > SPEED_MAX))
			m_nInitBallSpeed = SPEED_DEF;

		m_nInitPaddleSize = ConfMan.hasKey("PaddleSize") ?
			ConfMan.getInt("PaddleSize") : PSIZE_DEF;
		if ((m_nInitPaddleSize < PSIZE_MIN) || (m_nInitPaddleSize > PSIZE_MAX))
			m_nInitPaddleSize = PSIZE_DEF;

		m_bOutterWall = ConfMan.hasKey("OutterWall") ?
			ConfMan.getBool("OutterWall") : false;

		m_nGForceFactor = ConfMan.hasKey("Gravity") ?
			ConfMan.getInt("Gravity") : GFORCE_DEF;
		if ((m_nGForceFactor < GFORCE_MIN) || (m_nGForceFactor > GFORCE_MAX))
			m_nGForceFactor = GFORCE_DEF;

		ConfMan.setActiveDomain(domain);
	}
}

void Fuge::endBall() {
	// Nothing to do
}

void Fuge::startBall() {
	// Have the ball start on the paddle like in Arkinoids
	m_ptBallLocation = ballOnPaddle();
	m_pBall.setPosition(m_ptBallLocation.x, m_ptBallLocation.y);
	m_pBall.linkSprite();
}

CVector Fuge::ballOnPaddle() {
	CVector vBall(0, -(PADDLE_RADIUS + BALL_RADIUS));

	vBall.Rotate(fPaddleAngles[m_nInitPaddleSize] / 2);

	vBall.Rotate(m_nPaddleCelIndex * ((2 * PI) / N_PADDLE_CELS));
	vBall -= CVector(BALL_RADIUS, BALL_RADIUS);

	// this vector was relative to center so now make it a real point
	vBall += _gvCenter;

	return vBall;
}


void Fuge::startPaddle() {
	assert(!m_pPaddle.empty());

	m_pPaddle.linkSprite();
	m_bBallOnPaddle = true;

	paintPaddle(true);
}


void Fuge::endPaddle() {
	assert(!m_pPaddle.empty());

	if (m_pPaddle.isLinked())
		m_pPaddle.unlinkSprite();
}

void Fuge::launchBall() {
	assert(m_bGameActive);
	assert(m_bBallOnPaddle);

	m_bPause = false;
	m_bBallOnPaddle = false;

	// starting ball vector is determined by the location of the paddle
	m_vBallVector = _gvCenter - (m_ptBallLocation + BALL_RADIUS);

	// add a slight randomness to the balls vector
	m_vBallVector.Rotate(Deg2Rad(getRandomNumber(2) - 1));
	m_vBallVector.Unitize();

	moveBall();
}

void Fuge::startBricks() {
	int i, nBricks;

	nBricks = m_nNumRows * BRICKS_PER_ROW;

	for (i = 0; i < nBricks; i++) {
		if (!m_bBrickVisible[i]) {
			m_bBrickVisible[i] = true;
		}
	}
}

void Fuge::endBricks() {
	Common::fill(m_bBrickVisible, m_bBrickVisible + N_BRICKS, false);
}

void Fuge::moveBall() {
	Common::Point ptLast;
	CVector vBall, vGravity;
	double length;

	if (m_bGameActive && !m_bPause && !m_bBallOnPaddle) {
		assert(!m_pBall.empty());

		ptLast.x = (int)m_ptBallLocation.x;
		ptLast.y = (int)m_ptBallLocation.y;

		vGravity = _gvCenter - (m_ptBallLocation + BALL_RADIUS);
		vGravity.Unitize();

		// Calc new ball location
		if (m_nGForceFactor != 0) {
			vGravity *= G_FORCE * m_nGForceFactor;
			m_vBallVector += vGravity;
			m_ptBallLocation += m_vBallVector * (m_fTurboBoost + m_nBallSpeed);

		} else {
			vGravity *= G_FORCE * 10;
			m_ptBallLocation += (m_vBallVector + vGravity) * (m_fTurboBoost + m_nBallSpeed);
		}

		assert(m_fTurboBoost <= TURBO_MAX);
		m_fTurboBoost -= 0.05;
		if (m_fTurboBoost < 0.0)
			m_fTurboBoost = 0.0;

		m_vBallVector.Unitize();

		// Get radius of the ball from the center of the screen
		vBall = m_ptBallLocation + BALL_RADIUS - _gvCenter;

		length = vBall.Length() + BALL_RADIUS;

		// check to see if ball has entered the black hole
		if (length <= BLACKHOLE_RADIUS + BALL_RADIUS * 2) {
			// Play the ball-gets-sucked-into-black-hole animation
			loseBall();
			m_bPaddleHit = false;

		} else if (length <= PADDLE_RADIUS + BALL_RADIUS * 2) {
			// The ball hit the paddle
			ballvsPaddle();

		} else if ((length >= INNER_BRICK_RADIUS) && (length < WHEEL_RADIUS + (m_bOutterWall ? BALL_RADIUS * 2 : 0))) {
			// The ball hit a brick
			// Determine if a ball actually hit a brick
			ballvsBrick(length);
			m_bPaddleHit = false;

		} else if (length >= WHEEL_RADIUS) {
			// The ball hit edge of Ferris Wheel
			if (m_bOutterWall) {
				// has ball hit right border
				if (m_ptBallLocation.x >= GAME_WIDTH - GAME_RIGHT_BORDER_WIDTH - BALL_SIZE_X) {
					m_ptBallLocation.x = GAME_WIDTH - GAME_RIGHT_BORDER_WIDTH - BALL_SIZE_X;
					m_vBallVector.x = -m_vBallVector.x;

					// Randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));

					m_fTurboBoost = (double)12 - m_nBallSpeed;

					if (gameInfo.bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SOUND_ASYNCH | SND_NODEFAULT);
					}

				} else if (m_ptBallLocation.x <= 0 + GAME_LEFT_BORDER_WIDTH) {
					// Ball hit left border
					m_ptBallLocation.x = 0 + GAME_LEFT_BORDER_WIDTH;
					m_vBallVector.x = -m_vBallVector.x;

					// Randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));
					m_fTurboBoost = (double)12 - m_nBallSpeed;

					if (gameInfo.bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

				} else if (m_ptBallLocation.y >= GAME_HEIGHT - GAME_BOTTOM_BORDER_WIDTH - BALL_SIZE_Y) {
					// Ball hit bottom of screen
					m_ptBallLocation.y = GAME_HEIGHT - GAME_BOTTOM_BORDER_WIDTH - BALL_SIZE_Y;
					m_vBallVector.y = -m_vBallVector.y;

					// Randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));
					m_fTurboBoost = (double)12 - m_nBallSpeed;

					if (gameInfo.bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

				} else if (m_ptBallLocation.y <= 0 + GAME_TOP_BORDER_WIDTH) {
					// Ball hit top of screen
					m_ptBallLocation.y = 0 + GAME_TOP_BORDER_WIDTH;
					m_vBallVector.y = -m_vBallVector.y;

					// Randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));
					m_fTurboBoost = (double)12 - m_nBallSpeed;

					if (gameInfo.bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}
				}

			} else {
				if (gameInfo.bSoundEffectsEnabled) {
					sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
				}

				// Pull ball back to just on the border
				m_ptBallLocation -= m_vBallVector * (length - WHEEL_RADIUS + 1);

				m_vBallVector.x = -m_vBallVector.x;
				m_vBallVector.y = -m_vBallVector.y;

				m_vBallVector.Reflect(vBall);
			}

			m_bPaddleHit = false;
		}

		// Only update the ball if it actually moved
		if ((ptLast.x != (int)m_ptBallLocation.x) || (ptLast.y != (int)m_ptBallLocation.y)) {
			if (m_pBall.isLinked()) {
				// Move the ball to it's new location
				m_pBall.x = m_ptBallLocation.x;
				m_pBall.y = m_ptBallLocation.y;
			}
		}
	}
}

void Fuge::paintPaddle(bool bPaint) {
	CVector vPaddle;
	int nOldIndex;

	// verify that the input was not tainted
	assert(m_nPaddleCelIndex < N_PADDLE_CELS * 2);
	assert(m_nPaddleCelIndex > -N_PADDLE_CELS);

	// can't access a null pointer
	assert(!m_pPaddle.empty());

	// get old cel index
	nOldIndex = m_pPaddle.getCelIndex();

	if (m_nPaddleCelIndex >= N_PADDLE_CELS) {
		m_nPaddleCelIndex = m_nPaddleCelIndex - N_PADDLE_CELS;

	} else if (m_nPaddleCelIndex < 0) {
		m_nPaddleCelIndex = m_nPaddleCelIndex + N_PADDLE_CELS;
	}

	// verify our calculations
	assert((m_nPaddleCelIndex >= 0) && (m_nPaddleCelIndex < N_PADDLE_CELS));

	// don't re-paint the paddle if we would paint the same cel
	//
	if (bPaint || (nOldIndex != m_nPaddleCelIndex)) {
		m_pPaddle.setCel(m_nPaddleCelIndex);

		// move paddle to new location
		m_pPaddle.x = PADDLE_START_X;
		m_pPaddle.y = PADDLE_START_Y;

		// If the ball is resting on the paddle
		if (m_bBallOnPaddle) {
			assert(!m_pBall.empty());

			// ball is rotating with paddle
			m_ptBallLocation = ballOnPaddle();

			// set the ball to it's new location
			m_pBall.x = m_ptBallLocation.x;
			m_pBall.y = m_ptBallLocation.y;
		}
	}
}

void Fuge::loseBall() {
	CSound *pEffect = NULL;
	ErrorCode errCode;

	// assume no error
	errCode = ERR_NONE;

	// pause the game
	gamePause();

	// reset turbo
	m_fTurboBoost = 0.0;

	m_pBall.unlinkSprite();

	assert(m_nBalls > 0);

	// on less ball
	m_nBalls--;

	if (gameInfo.bSoundEffectsEnabled) {
#if CSOUND
		pEffect = new CBofSound(this, WAV_LOSEBALL,
			SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
#else
		sndPlaySound(WAV_LOSEBALL, SND_ASYNC);
#endif
	}

	// if no more balls left - user has lost
	if (m_nBalls == 0) {

		if (gameInfo.bSoundEffectsEnabled) {
#if CSOUND
			pEffect = new CSound(this, WAV_GAMEOVER,
				SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
#else
			sndPlaySound(WAV_GAMEOVER, SND_ASYNC);
#endif
		}

		Common::String msg = Common::String::format("Score:  %ld", m_lScore);
		MessageBox::show("Game over.", msg, []() {
			((Fuge *)g_events->findView("Fuge"))->gameOverClosed();
		});

	} else {
		// Display score, and start a new ball
		Common::String title = Common::String::format("Score:  %ld", m_lScore);
		Common::String msg = Common::String::format("Balls Left:  %d", m_nBalls);
		MessageBox::show(title, msg, []() {
			((Fuge *)g_events->findView("Fuge"))->newLifeClosed();
		});
	}
}

void Fuge::gameOverClosed() {
	if (gameInfo.bPlayingMetagame) {
		// return the final score
		gameInfo.lScore = m_lScore;
	}

	gameReset();
}

void Fuge::newLifeClosed() {
	// reset the ball position
	endPaddle();
	startPaddle();
	startBall();
}

void Fuge::optionsClosed() {
	// show the command scroll
	m_ScrollButton.setPressed(false);
	m_bIgnoreScrollClick = false;

	if (!gameInfo.bMusicEnabled && (m_pSoundTrack != NULL)) {
		m_pSoundTrack->stop();
		delete m_pSoundTrack;
		m_pSoundTrack = NULL;

	} else if (gameInfo.bMusicEnabled && (m_pSoundTrack == NULL)) {
		m_pSoundTrack = new CBofSound(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		m_pSoundTrack->midiLoopPlaySegment(5390, 32280, 0, FMT_MILLISEC);
	}

	gameResume();
}

#define N_CRIT_POINTS 7

void Fuge::ballvsPaddle() {
	CVector vPoints[N_CRIT_POINTS];
	CVector vTmp, vPaddle, vFace, vBallCenter, vBallEdge;
	double a1, a2;
	double fLen1, fLen2, fLen3, fLen4, fLen5, fLen6, fMin, length;
	int i, j, k;
	int nRollBack = 0;
	bool bHit;

	// calculate the 7 critical points for the paddle
	//
	vTmp.SetVector(0, -PADDLE_RADIUS);

	// cel index determines paddle angle
	//
	vTmp.Rotate((2 * PI / N_PADDLE_CELS) * m_nPaddleCelIndex);

	vPoints[1] = _gvCenter + vTmp;

	vPoints[0] = vTmp;
	vPoints[0].Rotate(fPaddleAngles[m_nInitPaddleSize] / 2);
	vPoints[0] += _gvCenter;

	vPoints[2] = vTmp;
	vPoints[2].Rotate(fPaddleAngles[m_nInitPaddleSize]);
	vPoints[2] += _gvCenter;

	vTmp.Unitize();

	vTmp *= 25;

	vPoints[3] = _gvCenter + vTmp;

	vPoints[4] = vTmp;
	vPoints[4].Rotate(fPaddleAngles[m_nInitPaddleSize]);
	vPoints[4] += _gvCenter;

	vPoints[5] = vPoints[1];
	vPoints[6] = vPoints[2];
	if (m_nInitPaddleSize > PSIZE_MIN) {
		vTmp = vPoints[0] - vPoints[1];
		vTmp.Unitize();
		vTmp *= 9; // paddle width

		vPoints[5] = vPoints[1] + vTmp;

		vTmp = vPoints[0] - vPoints[2];
		vTmp.Unitize();
		vTmp *= 9; // paddle width
		vPoints[6] = vPoints[2] - vTmp;
	}

	// get center of the ball
	vBallCenter = m_ptBallLocation + BALL_RADIUS;

	// if any of those points are less than the radius distance
	// away from the center of the ball, then the ball has hit
	// the paddle
	//
	bHit = false;
	for (i = 0; i < N_CRIT_POINTS - 1; i++) {

		switch (i) {

		case 0:
			j = 0;
			k = 5;
			break;

		case 1:
			j = 0;
			k = 6;
			break;

		case 2:
			j = 1;
			k = 5;
			break;

		case 3:
			j = 2;
			k = 6;
			break;

		case 4:
			j = 1;
			k = 3;
			break;

		default:
			j = 2;
			k = 4;
			break;
		}

		length = distanceBetweenPoints(vPoints[j], vPoints[k]) / 2;
		length = sqrt(BALL_RADIUS * BALL_RADIUS + length * length) * 2;

		fLen1 = distanceBetweenPoints(vPoints[j], vBallCenter);
		fLen2 = distanceBetweenPoints(vPoints[k], vBallCenter);

		nRollBack = 0;
		if (fLen1 <= BALL_RADIUS) {
			nRollBack = BALL_RADIUS - (int)fLen1;
			bHit = true;
			break;
		} else if (fLen2 <= BALL_RADIUS) {
			nRollBack = (BALL_RADIUS - (int)fLen2);
			bHit = true;
			break;
		} else if (fLen1 + fLen2 <= length) {
			nRollBack = (int)(length - (fLen1 + fLen2)) + 2;
			bHit = true;
			break;
		}
	}

	if (bHit) {
		nRollBack = MIN(m_nInitBallSpeed, nRollBack);

		if (gameInfo.bSoundEffectsEnabled) {
			sndPlaySound(m_pPaddleSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
		}

		// if we hit the ball twice in a row
		//
		if (m_bPaddleHit) {
			error("DoubleHit\n");

			// then shoot the ball away from the paddle
			//
			m_vBallVector = vBallCenter - _gvCenter;
			m_vBallVector.Unitize();
			m_vBallVector *= 2;

		} else {

			// role the ball back to the exact point that it hit the paddle
			//
			vTmp = m_vBallVector;
			vTmp.Unitize();
			vTmp *= nRollBack;

			m_ptBallLocation -= vTmp;

			// get center of ball
			vBallCenter = m_ptBallLocation + BALL_RADIUS;

			fLen1 = distanceBetweenPoints(vBallCenter, vPoints[0]);
			fLen1 += distanceBetweenPoints(vBallCenter, vPoints[1]);
			fMin = fLen1;

			fLen2 = distanceBetweenPoints(vBallCenter, vPoints[0]);
			fLen2 += distanceBetweenPoints(vBallCenter, vPoints[2]);

			fMin = MIN(fMin, fLen2);

			fLen3 = distanceBetweenPoints(vBallCenter, vPoints[3]);
			fLen3 += distanceBetweenPoints(vBallCenter, vPoints[1]);

			fMin = MIN(fMin, fLen3);

			fLen4 = distanceBetweenPoints(vBallCenter, vPoints[4]);
			fLen4 += distanceBetweenPoints(vBallCenter, vPoints[2]);

			fMin = MIN(fMin, fLen4);

			fLen5 = fLen1;
			fLen6 = fLen2;
			if (m_nInitPaddleSize > PSIZE_MIN) {

				fLen5 = distanceBetweenPoints(vBallCenter, vPoints[1]);
				fLen5 += distanceBetweenPoints(vBallCenter, vPoints[5]);
				fMin = MIN(fMin, fLen5);

				fLen6 = distanceBetweenPoints(vBallCenter, vPoints[2]);
				fLen6 += distanceBetweenPoints(vBallCenter, vPoints[6]);
				fMin = MIN(fMin, fLen6);
			}

			vTmp = m_vBallVector;
			m_vBallVector.x = -m_vBallVector.x;
			m_vBallVector.y = -m_vBallVector.y;

			vFace = vPoints[0] - _gvCenter;

			if ((fMin == fLen1) || (fMin == fLen5) || (fMin == fLen2) || (fMin == fLen6)) {

				if (fMin == fLen5) {
					vFace = vBallCenter - _gvCenter;
					vFace.Rotate(-Deg2Rad(10));

				} else if (fMin == fLen6) {
					vFace = vBallCenter - _gvCenter;
					vFace.Rotate(Deg2Rad(10));
				}

				// get center of ball
				vBallCenter = m_ptBallLocation + BALL_RADIUS;

				// roll ball back to edge of paddle
				//
				while (distanceBetweenPoints(vBallCenter, _gvCenter) < PADDLE_RADIUS + BALL_RADIUS) {

					m_ptBallLocation -= vTmp;

					// get center of ball
					vBallCenter = m_ptBallLocation + BALL_RADIUS;
				}

				vPaddle = vFace;

			} else if (fMin == fLen3) {

				vPaddle.SetVector(vPoints[3].x - CENTER_X, vPoints[3].y - CENTER_Y);
				vPaddle.Rotate(-PI / 2);

				a1 = vPaddle.AngleBetween(m_vBallVector);
				a2 = vFace.AngleBetween(m_vBallVector);

				// kludge to compensate for when angle is too big
				//
				if (a1 > a2) {
					vPaddle = vFace;
				}

			} else if (fMin == fLen4) {

				vPaddle.SetVector(vPoints[4].x - CENTER_X, vPoints[4].y - CENTER_Y);
				vPaddle.Rotate(PI / 2);

				a1 = vPaddle.AngleBetween(m_vBallVector);
				a2 = vFace.AngleBetween(m_vBallVector);

				// kludge to compensate for when angle is too big
				//
				if (a1 > a2) {
					vPaddle = vFace;
				}
			}

			// reflect the ball vector around the final paddle (mirror) vector
			//
			m_vBallVector.Reflect(vPaddle);

			//
			// one final check to make sure the ball is bouncing in the
			// correct direction.
			//
			if (m_vBallVector.AngleBetween(_gvCenter - vBallCenter) <= Deg2Rad(15)) {
				m_vBallVector.Rotate(Deg2Rad(180));
			}
		}

		m_bPaddleHit = true;
	}
}

#define MAX_BRICK_HITS 6

void Fuge::ballvsBrick(double length) {
	CVector vPoints[N_BRICK_POINTS];
	CVector vBrick, vBallCenter, vOrigin, vTmp;
	CSound *pEffect = NULL;
	Common::Rect rTmpRect, rBall, cRect;
	Common::Point ptTmp;
	double fMin, fLast, fLen[N_BRICK_POINTS];
	double angle;
	int i, j, nIndex, nLastIndex, nBrickIndex, nMaxHits;
	int nBrick0, nBrick1, nRow0, nRow1, nRow2, nUse[MAX_BRICK_HITS];
	bool bHit, bStillHit;
	Common::String title, msg;

	// Get bounding rectangle of the ball
	rBall = Common::Rect((int)m_ptBallLocation.x, (int)m_ptBallLocation.y,
		(int)m_ptBallLocation.x + BALL_SIZE_X,
		(int)m_ptBallLocation.y + BALL_SIZE_Y);

	// Get center of the ball
	vBallCenter.SetVector(m_ptBallLocation.x + BALL_RADIUS, m_ptBallLocation.y + BALL_RADIUS);

	vOrigin.SetVector(-1, -1);
	vOrigin.Rotate(PI / BRICKS_PER_ROW);
	vTmp = vBallCenter - _gvCenter;

	angle = vTmp.RealAngle(vOrigin);

	nMaxHits = MAX_BRICK_HITS;
	if (length <= ROW6_RADIUS) {
		nRow0 = 5;
		nRow1 = 5;
		nRow2 = 5;
		nMaxHits = 2;

	} else if (length <= ROW5_RADIUS) {
		nRow0 = 5;
		nRow1 = 4;
		nRow2 = 4;
		nMaxHits = 4;

	} else if (length <= ROW4_RADIUS - BALL_RADIUS) {
		nRow0 = 5;
		nRow1 = 4;
		nRow2 = 3;

	} else if (length <= ROW3_RADIUS - BALL_RADIUS) {
		nRow0 = 4;
		nRow1 = 3;
		nRow2 = 2;

	} else if (length <= ROW2_RADIUS - BALL_RADIUS) {
		nRow0 = 3;
		nRow1 = 2;
		nRow2 = 1;

	} else {
		nRow0 = 2;
		nRow1 = 1;
		nRow2 = 0;
	}

	if (angle >= BRICK15_ANGLE) {
		nBrick0 = 0;
		nBrick1 = 15;

	} else if (angle >= BRICK14_ANGLE) {
		nBrick0 = 15;
		nBrick1 = 14;

	} else if (angle >= BRICK13_ANGLE) {
		nBrick0 = 14;
		nBrick1 = 13;

	} else if (angle >= BRICK12_ANGLE) {
		nBrick0 = 13;
		nBrick1 = 12;

	} else if (angle >= BRICK11_ANGLE) {
		nBrick0 = 12;
		nBrick1 = 11;

	} else if (angle >= BRICK10_ANGLE) {
		nBrick0 = 11;
		nBrick1 = 10;

	} else if (angle >= BRICK9_ANGLE) {
		nBrick0 = 10;
		nBrick1 = 9;

	} else if (angle >= BRICK8_ANGLE) {
		nBrick0 = 9;
		nBrick1 = 8;

	} else if (angle >= BRICK7_ANGLE) {
		nBrick0 = 8;
		nBrick1 = 7;

	} else if (angle >= BRICK6_ANGLE) {
		nBrick0 = 7;
		nBrick1 = 6;

	} else if (angle >= BRICK5_ANGLE) {
		nBrick0 = 6;
		nBrick1 = 5;

	} else if (angle >= BRICK4_ANGLE) {
		nBrick0 = 5;
		nBrick1 = 4;

	} else if (angle >= BRICK3_ANGLE) {
		nBrick0 = 4;
		nBrick1 = 3;

	} else if (angle >= BRICK2_ANGLE) {
		nBrick0 = 3;
		nBrick1 = 2;

	} else if (angle >= BRICK1_ANGLE) {
		nBrick0 = 2;
		nBrick1 = 1;

	} else {
		nBrick0 = 1;
		nBrick1 = 0;
	}
	nUse[0] = (nRow0 * BRICKS_PER_ROW) + nBrick0;
	nUse[1] = (nRow0 * BRICKS_PER_ROW) + nBrick1;
	nUse[2] = (nRow1 * BRICKS_PER_ROW) + nBrick0;
	nUse[3] = (nRow1 * BRICKS_PER_ROW) + nBrick1;
	nUse[4] = (nRow2 * BRICKS_PER_ROW) + nBrick0;
	nUse[5] = (nRow2 * BRICKS_PER_ROW) + nBrick1;

	// Which brick did we hit?
	bHit = false;

	for (i = 0; i < nMaxHits; i++) {
		nBrickIndex = nUse[i];

		assert(nBrickIndex >= 0 && nBrickIndex < N_BRICKS);

		if (m_bBrickVisible[nBrickIndex]) {
			// Does the ball's rectange intersects this brick's rectangle
			cRect = Common::Rect(
				ptBrickPos[nBrickIndex].x, ptBrickPos[nBrickIndex].y,
				ptBrickPos[nBrickIndex].x + ptBrickSize[nBrickIndex].cx,
				ptBrickPos[nBrickIndex].y + ptBrickSize[nBrickIndex].cy);

			rTmpRect = rBall.findIntersectingRect(cRect);
			if (rTmpRect.isValidRect()) {
				// Calculate the 21 points for this brick
				for (j = 0; j < N_BRICK_POINTS; j++) {
					vPoints[j] = vBrickCritPoints[nBrickIndex / BRICKS_PER_ROW][j];

					vPoints[j].Rotate(((2 * PI) / BRICKS_PER_ROW) * (nBrickIndex % BRICKS_PER_ROW));
					vPoints[j] += _gvCenter;

					if (distanceBetweenPoints(vBallCenter, vPoints[j]) < 11.0)
						bHit = true;
				}

				if (bHit) {
					if (gameInfo.bSoundEffectsEnabled) {
						sndPlaySound(m_pBrickSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					// Remove the brick
					eraseBrick(nBrickIndex);

					// One less brick
					--m_nBricks;

					// Score: 1 point for each brick regardless of color
					m_lScore += 1;

					// Did user earn an extra ball?
					if (m_lScore >= m_lExtraLifeScore) {
						if (gameInfo.bSoundEffectsEnabled) {
							sndPlaySound(m_pExtraLifeSound, SND_MEMORY | SND_SYNC | SND_NODEFAULT);
						}

						// Double the amount the user needs for their next extra life
						m_lExtraLifeScore += m_lExtraLifeScore;

						// Extra ball
						m_nBalls++;
					}

					// If no bricks left
					if (m_nBricks == 0) {
						gamePause();

						// Reset turbo
						m_fTurboBoost = 0.0;

						if (gameInfo.bSoundEffectsEnabled) {
#if CSOUND
							pEffect = new CBofSound(this, WAV_WINWAVE,
								SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
							(*pEffect).play();                                                      //...play the narration
#else
							sndPlaySound(WAV_WINWAVE, SND_ASYNC);
#endif
						}

						// 5 point bonus for clearing all bricks
						m_lScore += 5;

						//
						// User wins this round
						title = Common::String::format("Round complete.");
						msg = Common::String::format("Score:  %ld", m_lScore);
						MessageBox::show(title, msg, []() {
							((Fuge *)g_events->findView("Fuge"))->roundCompleteClosed();
						});

					} else {
						// There are more bricks left,
						// So just calc a new vector for the ball

						bStillHit = true;
						while (bStillHit) {
							// Roll ball back to point of contact with brick
							m_ptBallLocation -= m_vBallVector;

							// Get new center of ball
							vBallCenter = m_ptBallLocation + BALL_RADIUS;

							bStillHit = false;
							for (j = 0; j < N_BRICK_POINTS; j++) {
								if ((fLen[j] = distanceBetweenPoints(vBallCenter, vPoints[j])) < 11.0) {
									bStillHit = true;
									break;
								}
							}
						}

						// Find the 2 closest points to the center of the ball
						nIndex = nLastIndex = -1;
						fMin = fLast = 9999;
						for (j = 0; j < N_BRICK_POINTS; j++) {
							if (fLen[j] < fMin) {
								fLast = fMin;
								fMin = fLen[j];
								nLastIndex = nIndex;
								nIndex = j;

							} else if (fLen[j] < fLast) {
								fLast = fLen[j];
								nLastIndex = j;
							}
						}

						// Make sure we actually found an intersect point
						assert((nIndex != -1) && (nLastIndex != -1));

						// If ball hit a corner, then use next best point to
						// determine which side the ball actually hit
						switch (nIndex) {
						case 0:
						case 9:
						case 11:
						case 19:
							nIndex = nLastIndex;
							break;

						default:
							break;
						}

						if (nIndex >= 1 && nIndex <= 8) {
							// Hit back face of brick
							vBrick = vPoints[nIndex] - _gvCenter;

						} else if (nIndex >= 12 && nIndex <= 18) {
							// Hit inner face of brick
							vBrick = _gvCenter - vPoints[nIndex];

						} else if (nIndex == 10) {
							// Hit clockwise side of brick
							vBrick = aBrickVectors[nBrickIndex % BRICKS_PER_ROW].v2;

						} else if (nIndex == 20) {
							// Hit counter-clockwise side of brick
							vBrick = aBrickVectors[nBrickIndex % BRICKS_PER_ROW].v1;

						} else {
							// Invalid index
							error("Invalid Index (%d)\n", nIndex);
						}

						// Determine the vector of the brick (using ptHit as the intersect point)
						m_vBallVector.x = -m_vBallVector.x;
						m_vBallVector.y = -m_vBallVector.y;

						m_vBallVector.Reflect(vBrick);
					}
					break;
				}
			}
		}
	}
}

void Fuge::roundCompleteClosed() {
	// stop all sounds
	if (gameInfo.bSoundEffectsEnabled) {
#if CSOUND
#else
		sndPlaySound(NULL, SND_ASYNC);
#endif
	}

	m_nNumRows++;
	if (m_nNumRows > N_ROWS) {
		m_nNumRows = 1;
		m_nBallSpeed++;
	}

	// get new brick count
	m_nBricks = m_nNumRows * BRICKS_PER_ROW;

	if (gameInfo.bPlayingMetagame) {
		// if user is playing the metagame
		// return to the metagame
		gameInfo.lScore = m_lScore;
		close();

	} else {
		// reset the ball position
		endPaddle();
		endBall();
		startBricks();
		startBall();
		startPaddle();

		gameResume();
	}
}

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel
