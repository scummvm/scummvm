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
#include "common/file.h"
#include "image/bmp.h"
#include "graphics/paletteman.h"
#include "bagel/metaengine.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/size.h"
#include "bagel/hodjnpodj/fuge/fuge.h"
#include "bagel/hodjnpodj/fuge/defines.h"
#include "bagel/hodjnpodj/libs/vector.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
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

Fuge::Fuge() : MinigameView("Fuge", "fuge/hnpfuge.dll"),
		_rNewGameButton(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
			NEWGAME_LOCATION_X + NEWGAME_WIDTH, NEWGAME_LOCATION_Y + NEWGAME_HEIGHT),
		_scrollButton("ScrollButton", this, Common::Rect(
			SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
			SCROLL_BUTTON_X + SCROLL_BUTTON_DX,
			SCROLL_BUTTON_Y + SCROLL_BUTTON_DY)),
		_ptOrigin(GAME_WIDTH / 2, GAME_HEIGHT / 2),
		_gvCenter(CENTER_X, CENTER_Y),
		_sprPaddle(this),
		_sprBall(this) {

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
	_nInitNumBalls = BALLS_DEF;
	_nInitStartLevel = LEVEL_DEF;
	_nInitBallSpeed = SPEED_DEF;
	_nInitPaddleSize = PSIZE_DEF;
	_nGForceFactor = GFORCE_DEF;

	_soundTrack = nullptr;
	_bPause = false;
	_bGameActive = false;
	_bBallOnPaddle = false;
	_nPaddleCelIndex = 29;
	_bMovingPaddle = false;
	_lScore = 0;
	m_pBrickSound = nullptr;
	m_pWallSound = nullptr;
	m_pPaddleSound = nullptr;
	m_pExtraLifeSound = nullptr;
	_nNumRows = 0;
	_bJoyActive = false;
	Common::fill(_brickVisible, _brickVisible + N_BRICKS, false);
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

	assert(decoder.getPaletteColorCount() == PALETTE_COUNT);
	loadPalette(decoder.getPalette());

	// Load scroll button
	_scrollButton.loadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);

	// Start the Fuge soundtrack
	if (pGameParams->bMusicEnabled) {
		_soundTrack = new CBofSound(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		_soundTrack->midiLoopPlaySegment(5390, 32280, 0, FMT_MILLISEC);
	}

	// Load data
	loadMasterSprites();
	loadMasterSounds();

	_showMenuCtr = pGameParams->bPlayingMetagame ? 0 : 2;
	if (pGameParams->bPlayingMetagame)
		playGame();

	return true;
}

bool Fuge::msgClose(const CloseMessage &msg) {
	MinigameView::msgClose(msg);

	gameReset();
	releaseMasterSounds();
	releaseMasterSprites();	// Release all master sprites

	return true;
}

bool Fuge::msgFocus(const FocusMessage &msg) {
	_scrollButton.setPressed(false);

	if (msg._priorView && msg._priorView->getName() == "MainMenu")
		optionsClosed();

	return MinigameView::msgFocus(msg);
}

bool Fuge::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_UP:
	case KEYBIND_RIGHT:
		// Move paddle clockwise
		if (_bGameActive) {
			_nPaddleCelIndex += PADDLE_CEL_JUMP;
			movePaddle(false);
		}
		break;

	case KEYBIND_DOWN:
	case KEYBIND_LEFT:
		// Move paddle counter-clockwise
		if (_bGameActive) {
			_nPaddleCelIndex -= PADDLE_CEL_JUMP;
			movePaddle(false);
		}
		break;

	case KEYBIND_SELECT:
		// Launch ball
		if (_bGameActive) {
			if (_bBallOnPaddle) {
				launchBall();
			}
		}
		break;

	default:
		return MinigameView::msgAction(msg);
	}

	redraw();
	return true;
}

bool Fuge::msgGame(const GameMessage &msg) {
	if (msg._name == "NEW_GAME") {
		playGame();
		return true;
	} else if (msg._name == "GAME_OVER") {
		gameOverClosed();
		return true;
	} else if (msg._name == "NEW_LIFE") {
		newLifeClosed();
		return true;
	} else if (msg._name == "ROUND_COMPLETE") {
		roundCompleteClosed();
		return true;
	} else if (msg._name == "OPTIONS") {
		_optionsView.addView();
		return true;
	} else if (msg._name == "BUTTON") {
		// Scroll button
		showMainMenu();
		return true;
	}

	return false;
}

bool Fuge::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_F1:
		// Bring up the Rules
		gamePause();
		CBofSound::waitWaveSounds();
		Rules::show("fuge/fuge.txt",
			(pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr)
		);
		break;

	case Common::KEYCODE_F2:
		// Options menu
		showMainMenu();
		break;

	case Common::KEYCODE_SCROLLOCK:
		if (_bGameActive) {
			if (!_bPause) {
				gamePause();
			} else {
				gameResume();
			}
		}
		break;

	case Common::KEYCODE_BACKSPACE:
		MessageBox::show("ScummVM", "Lorem Ipsum");
		break;

	default:
		return MinigameView::msgKeypress(msg);
	}

	redraw();
	return true;
}

bool Fuge::msgMouseMove(const MouseMoveMessage &msg) {
	int nMove;

	// Even the original authors admit that mouse control
	// mode (toggled by right clicking) kinda sucks.
	// But it's left in to remain faithful to the original
	if (_bGameActive && _bMovingPaddle) {
		Common::Point point = msg._pos;

		if (point.x > _ptOrigin.x) {
			nMove = (point.x - _ptOrigin.x) / MOUSE_SENS + 1;
			_nPaddleCelIndex += nMove;
			movePaddle(false);
			g_system->warpMouse(_ptOrigin.x, _ptOrigin.y);

		} else if (point.x < _ptOrigin.x) {
			nMove = -((_ptOrigin.x - point.x) / MOUSE_SENS + 1);
			_nPaddleCelIndex += nMove;
			movePaddle(false);
			g_system->warpMouse(_ptOrigin.x, _ptOrigin.y);
		}

		// Hide the cursor
		g_events->setCursor(0);

	} else {
		g_events->setCursor(IDC_ARROW);
	}

	return true;
}

bool Fuge::msgMouseDown(const MouseDownMessage &msg) {
	CBofSound *pEffect = nullptr;
	Common::Rect boothRect, tentRect, peopRect,
		car1Rect, car2Rect, car3Rect, car4Rect,
		car5Rect, car6Rect, car7Rect, car8Rect,
		car9Rect, car10Rect;
	int nPick = 0;

	if (MinigameView::msgMouseDown(msg))
		return true;
	if (msg._button != MouseDownMessage::MB_LEFT)
		return false;

	boothRect = Common::Rect(BOOTH_X, BOOTH_Y, BOOTH_X + BOOTH_DX, BOOTH_Y + BOOTH_DY);
	tentRect = Common::Rect(TENT_X, TENT_Y, TENT_X + TENT_DX, TENT_Y + TENT_DY);
	peopRect = Common::Rect(PEOPLE_X, PEOPLE_Y, PEOPLE_X + PEOPLE_DX, PEOPLE_Y + PEOPLE_DY);
	car1Rect = Common::Rect(CAR1_X, CAR1_Y, CAR1_X + CAR_DX, CAR1_Y + CAR_DY);
	car2Rect = Common::Rect(CAR2_X, CAR2_Y, CAR2_X + CAR_DX, CAR2_Y + CAR_DY);
	car3Rect = Common::Rect(CAR3_X, CAR3_Y, CAR3_X + CAR_DX, CAR3_Y + CAR_DY);
	car4Rect = Common::Rect(CAR4_X, CAR4_Y, CAR4_X + CAR_DX, CAR4_Y + CAR_DY);
	car5Rect = Common::Rect(CAR5_X, CAR5_Y, CAR5_X + CAR_DX, CAR5_Y + CAR_DY);
	car6Rect = Common::Rect(CAR6_X, CAR6_Y, CAR6_X + CAR_DX, CAR6_Y + CAR_DY);
	car7Rect = Common::Rect(CAR7_X, CAR7_Y, CAR7_X + CAR_DX, CAR7_Y + CAR_DY);
	car8Rect = Common::Rect(CAR8_X, CAR8_Y, CAR8_X + CAR_DX, CAR8_Y + CAR_DY);
	car9Rect = Common::Rect(CAR9_X, CAR9_Y, CAR9_X + CAR9_DX, CAR9_Y + CAR9_DY);
	car10Rect = Common::Rect(CAR10_X, CAR10_Y, CAR10_X + CAR10_DX, CAR10_Y + CAR10_DY);

	if (_rNewGameButton.contains(msg._pos)) {
		// User clicked on the left hand side of the title area.
		// If we're not in the metagame, start a new Fuge game
		if (!pGameParams->bPlayingMetagame) {
			// Start a new game
			playGame();
		}
	} else if (boothRect.contains(msg._pos)) {
		if (pGameParams->bSoundEffectsEnabled) {
#if CSOUND
			pEffect = new CBofSound(this, WAV_BOOTH,
				SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
#else
			sndPlaySound(WAV_BOOTH, SND_ASYNC);
#endif
		}
	} else if (tentRect.contains(msg._pos)) {

		if (pGameParams->bSoundEffectsEnabled) {
#if CSOUND
			pEffect = new CBofSound(this, WAV_TENT,
				SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
#else
			sndPlaySound(WAV_TENT, SND_ASYNC);
#endif
		}

	} else if (peopRect.contains(msg._pos)) {
		if (pGameParams->bSoundEffectsEnabled) {
			nPick = getRandomNumber(NUM_WAVS - 1);
			if (nPick == 0) {
#if CSOUND
				pEffect = new CBofSound(this, WAV_PEOPLE1,
					SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
#else
				sndPlaySound(WAV_PEOPLE1, SND_ASYNC);
#endif
			} else {
#if CSOUND
				pEffect = new CBofSound(this, WAV_PEOPLE2,
					SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
#else
				sndPlaySound(WAV_PEOPLE2, SND_ASYNC);
#endif
			}
		}

	} else if (((((((((car1Rect.contains(msg._pos) || car2Rect.contains(msg._pos)) || car3Rect.contains(msg._pos)) ||
		car4Rect.contains(msg._pos)) || car5Rect.contains(msg._pos)) || car6Rect.contains(msg._pos)) ||
		car7Rect.contains(msg._pos)) || car8Rect.contains(msg._pos)) || car9Rect.contains(msg._pos)) ||
		car10Rect.contains(msg._pos)) {

		if (pGameParams->bSoundEffectsEnabled) {
			nPick = getRandomNumber(NUM_WAVS - 1);

#if CSOUND
			// Wave file, to delete itself play the narration
			pEffect = new CBofSound(this, ((nPick == 0) ? WAV_CAR1 : WAV_CAR2), SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);
			pEffect->play();
#else
			sndPlaySound(((nPick == 0) ? WAV_CAR1 : WAV_CAR2), SND_ASYNC);
#endif
		}
	} else {

		if (_bGameActive) {
			if (_bBallOnPaddle) {
				launchBall();
			}
		}
	}

	return true;
}

bool Fuge::msgMouseUp(const MouseUpMessage &msg) {
	if (MinigameView::msgMouseUp(msg))
		return true;

	if (_bGameActive && msg._button == MouseUpMessage::MB_RIGHT) {
		// Toggle move paddle mode
		_bMovingPaddle = !_bMovingPaddle;

		if (_bMovingPaddle) {
			g_events->setCursor(IDC_NONE);
			g_system->warpMouse(_ptOrigin.x, _ptOrigin.y);
			gameResume();

		} else {
			g_events->setCursor(IDC_ARROW);
			gamePause();
		}
	}

	return true;
}

void Fuge::draw() {
	paintBricks();
	repaintSpriteList();
}

void Fuge::paintBricks() {
	assert((_nNumRows >= 0) && (_nNumRows <= N_ROWS));

	if (_nNumRows != _backgroundNumRows) {
		// Change the background bitmap
		_backgroundNumRows = _nNumRows;

		Image::BitmapDecoder decoder;
		Common::File f;
		if (!f.open(pszFugeArt[_nNumRows]) || !decoder.loadStream(f))
			error("Could not load %s", MINI_GAME_MAP);

		_background.copyFrom(*decoder.getSurface());
	}

	// Draw the background
	GfxSurface s = getSurface();
	s.blitFrom(_background);

	// Erase all the bricks that are hidden
	for (int brickIndex = 0;
			brickIndex < (_nNumRows * BRICKS_PER_ROW);
			brickIndex++) {
		if (!_brickVisible[brickIndex]) {
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
	if (_showMenuCtr) {
		if (--_showMenuCtr == 0)
			showMainMenu();
	}

	// Continue as long as there is a currently active non-paused game
	if (_bGameActive && !_bPause) {
		if (++_timerCtr >= 5) {
			moveBall();
			redraw();
		}
	}

	return true;
}

void Fuge::eraseBrick(int brickIndex) {
	assert((brickIndex >= 0) && (brickIndex < N_BRICKS));
	_brickVisible[brickIndex] = false;
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
	loadNewPaddle(_nInitPaddleSize);

	_sprBall.loadBitmap(BALL_BMP);
	_sprBall.setTransparentColor(255);
}

void Fuge::releaseMasterSprites() {
	// Clear bitmaps
	_background.clear();
	_scrollButton.clear();
	_sprBall.clear();
	_sprPaddle.clear();
	_backgroundNumRows = -1;
	_paddleOldSize = -1;
}

void Fuge::loadNewPaddle(int nNewSize) {
	assert(nNewSize >= PSIZE_MIN && nNewSize <= SIZE_MAX);

	// Don't try to load the same paddle
	if (_paddleOldSize != nNewSize) {
		_paddleOldSize = nNewSize;
		_sprPaddle.loadCels(pszPaddles[nNewSize], N_PADDLE_CELS);
	}
}

void Fuge::loadMasterSounds() {
	// Load the Brick "ping" into memory
	{
		Common::File f;
		if (!f.open(WAV_BRICK))
			error("Error loading sound - %s", WAV_BRICK);
		_brickSound = f.readStream(f.size());
	}

	// Load the wall "ping" into memory
	{
		Common::File f;
		if (!f.open(WAV_WALL))
			error("Error loading sound - %s", WAV_WALL);
		_wallSound = f.readStream(f.size());
	}

	// Load the paddle "ping" into memory
	{
		Common::File f;
		if (!f.open(WAV_PADDLE))
			error("Error loading sound - %s", WAV_PADDLE);
		_paddleSound = f.readStream(f.size());
	}

	// Load the extra life sound into memory
	{
		Common::File f;
		if (!f.open(WAV_NEWLIFE))
			error("Error loading sound - %s", WAV_NEWLIFE);
		_extraLifeSound = f.readStream(f.size());
	}
}

void Fuge::releaseMasterSounds() {
	// Stop the soundtrack
	if (_soundTrack != nullptr) {
		// We should have been playing music
		assert(pGameParams->bMusicEnabled);
		delete _soundTrack;
		_soundTrack = nullptr;
	}

#if CSOUND
	CBofSound::clearSounds();
#endif

	delete _extraLifeSound;
	_extraLifeSound = nullptr;
	delete _paddleSound;
	m_pPaddleSound = nullptr;
	delete _wallSound;
	m_pWallSound = nullptr;
	delete _brickSound;
	m_pBrickSound = nullptr;
}

void Fuge::showMainMenu() {
	_scrollButton.setPressed(true);

	CBofSound::waitWaveSounds();

	// Show the options view
	MainMenu::show(
		(pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0) |
		(_bGameActive ? 0 : NO_RETURN),
		"fuge/fuge.txt",
		pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : NULL
	);
}

void Fuge::gamePause() {
	_bPause = true;
}

void Fuge::gameResume() {
	if (!_bBallOnPaddle && _bMovingPaddle)
		_bPause = false;
}

void Fuge::playGame() {
	// Load the .INI settings
	loadIniSettings();

	// Reset all game parameters
	gameReset();

	loadNewPaddle(_nInitPaddleSize);

	// Start game
	startBricks();
	startPaddle();
	startBall();

	// Game starts paused
	_bPause = true;
	_bGameActive = true;
	redraw();
}

void Fuge::gameReset() {
	if (pGameParams->bSoundEffectsEnabled) {
		BofPlaySound(nullptr, SOUND_ASYNCH);	// Stop all sounds
	}

	endBricks();	// Remove all bricks from sprite chain
	endBall();		// Remove ball from sprite chain
	endPaddle();    // Remove paddle from sprite chain

	_fTurboBoost = 0.0;					// No turbo
	_lExtraLifeScore = EXTRA_LIFE_SCORE;	// User needs this many points for an extra life
	_bGameActive = false;	// There is no currently active game
	_bPause = false;		// The game is not paused
	_bBallOnPaddle = false;			// Ball is not yet on paddle
	_bMovingPaddle = false;			// User is not moving the paddle
	_nBalls = _nInitNumBalls;			// Reset # of balls
	_nBallSpeed = _nInitBallSpeed;	// Reset ball speed

	_nNumRows = _nInitStartLevel;				// Reset number of brick rows
	_nBricks = _nNumRows * BRICKS_PER_ROW;	// Get new brick count
	_lScore = 0;                               // Reset the score
	_bPaddleHit = false;                       // Paddle starts fresh
}

void Fuge::loadIniSettings() {
	if (pGameParams->bPlayingMetagame) {
		_bOuterWall = false;
		_nInitNumBalls = 1;
		_nInitStartLevel = 3;
		_nGForceFactor = GFORCE_DEF;
		_nInitPaddleSize = PSIZE_MAX;

		switch (pGameParams->nSkillLevel) {
		case SkillLevel::SKILLLEVEL_LOW:
			_nInitBallSpeed = 4;
			break;

		case SkillLevel::SKILLLEVEL_MEDIUM:
			_nInitBallSpeed = 6;
			break;

		default:
			assert(pGameParams->nSkillLevel == SkillLevel::SKILLLEVEL_HIGH);
			_nInitBallSpeed = 8;
			break;
		}

	} else {
		Common::String domain = ConfMan.getActiveDomainName();
		ConfMan.setActiveDomain(INI_SECTION);

		_nInitNumBalls = ConfMan.hasKey("NumberOfBalls") ?
			ConfMan.getInt("NumberOfBalls") : BALLS_DEF;
		if ((_nInitNumBalls < BALLS_MIN) || (_nInitNumBalls > BALLS_MAX))
			_nInitNumBalls = BALLS_DEF;

		_nInitStartLevel = ConfMan.hasKey("StartingLevel") ?
			ConfMan.getInt("StartingLevel") : LEVEL_DEF;
		if ((_nInitStartLevel < LEVEL_MIN) || (_nInitStartLevel > LEVEL_MAX))
			_nInitStartLevel = LEVEL_DEF;

		_nInitBallSpeed = ConfMan.hasKey("BallSpeed") ?
			ConfMan.getInt("BallSpeed") : SPEED_DEF;
		if ((_nInitBallSpeed < SPEED_MIN) || (_nInitBallSpeed > SPEED_MAX))
			_nInitBallSpeed = SPEED_DEF;

		_nInitPaddleSize = ConfMan.hasKey("PaddleSize") ?
			ConfMan.getInt("PaddleSize") : PSIZE_DEF;
		if ((_nInitPaddleSize < PSIZE_MIN) || (_nInitPaddleSize > PSIZE_MAX))
			_nInitPaddleSize = PSIZE_DEF;

		_bOuterWall = ConfMan.hasKey("OutterWall") ?
			ConfMan.getBool("OutterWall") : false;

		_nGForceFactor = ConfMan.hasKey("Gravity") ?
			ConfMan.getInt("Gravity") : GFORCE_DEF;
		if ((_nGForceFactor < GFORCE_MIN) || (_nGForceFactor > GFORCE_MAX))
			_nGForceFactor = GFORCE_DEF;

		ConfMan.setActiveDomain(domain);
	}
}

void Fuge::endBall() {
	if (_sprBall.isLinked())
		_sprBall.unlinkSprite();
}

void Fuge::startBall() {
	// Have the ball start on the paddle like in Arkinoids
	_ptBallLocation = ballOnPaddle();
	_sprBall.setPosition(_ptBallLocation.x, _ptBallLocation.y);
	_sprBall.linkSprite();
}

CVector Fuge::ballOnPaddle() {
	CVector vBall(0, -(PADDLE_RADIUS + BALL_RADIUS));

	vBall.Rotate(fPaddleAngles[_nInitPaddleSize] / 2);

	vBall.Rotate(_nPaddleCelIndex * ((2 * PI) / N_PADDLE_CELS));
	vBall -= CVector(BALL_RADIUS, BALL_RADIUS);

	// This vector was relative to center so now make it a real point
	vBall += _gvCenter;

	return vBall;
}


void Fuge::startPaddle() {
	assert(!_sprPaddle.empty());

	_sprPaddle.linkSprite();
	_bBallOnPaddle = true;

	movePaddle(true);
}


void Fuge::endPaddle() {
	assert(!_sprPaddle.empty());

	if (_sprPaddle.isLinked())
		_sprPaddle.unlinkSprite();
}

void Fuge::launchBall() {
	assert(_bGameActive);
	assert(_bBallOnPaddle);

	_bPause = false;
	_bBallOnPaddle = false;

	// Starting ball vector is determined by the location of the paddle
	_vBallVector = _gvCenter - (_ptBallLocation + BALL_RADIUS);

	// Add a slight randomness to the balls vector
	_vBallVector.Rotate(Deg2Rad(getRandomNumber(2) - 1));
	_vBallVector.Unitize();

	moveBall();
}

void Fuge::startBricks() {
	int i, nBricks;

	nBricks = _nNumRows * BRICKS_PER_ROW;

	for (i = 0; i < nBricks; i++) {
		if (!_brickVisible[i]) {
			_brickVisible[i] = true;
		}
	}
}

void Fuge::endBricks() {
	Common::fill(_brickVisible, _brickVisible + N_BRICKS, false);
}

void Fuge::moveBall() {
	Common::Point ptLast;
	CVector vBall, vGravity;
	double length;

	if (_bGameActive && !_bPause && !_bBallOnPaddle) {
		assert(!_sprBall.empty());

		ptLast.x = (int)_ptBallLocation.x;
		ptLast.y = (int)_ptBallLocation.y;

		vGravity = _gvCenter - (_ptBallLocation + BALL_RADIUS);
		vGravity.Unitize();

		// Calc new ball location
		if (_nGForceFactor != 0) {
			vGravity *= G_FORCE * _nGForceFactor;
			_vBallVector += vGravity;
			_ptBallLocation += _vBallVector * (_fTurboBoost + _nBallSpeed);

		} else {
			vGravity *= G_FORCE * 10;
			_ptBallLocation += (_vBallVector + vGravity) * (_fTurboBoost + _nBallSpeed);
		}

		assert(_fTurboBoost <= TURBO_MAX);
		_fTurboBoost -= 0.05;
		if (_fTurboBoost < 0.0)
			_fTurboBoost = 0.0;

		_vBallVector.Unitize();

		// Get radius of the ball from the center of the screen
		vBall = _ptBallLocation + BALL_RADIUS - _gvCenter;

		length = vBall.Length() + BALL_RADIUS;

		// Check to see if ball has entered the black hole
		if (length <= BLACKHOLE_RADIUS + BALL_RADIUS * 2) {
			// Play the ball-gets-sucked-into-black-hole animation
			loseBall();
			_bPaddleHit = false;

		} else if (length <= PADDLE_RADIUS + BALL_RADIUS * 2) {
			// The ball hit the paddle
			ballvsPaddle();

		} else if ((length >= INNER_BRICK_RADIUS) && (length < WHEEL_RADIUS + (_bOuterWall ? BALL_RADIUS * 2 : 0))) {
			// The ball hit a brick
			// Determine if a ball actually hit a brick
			ballvsBrick(length);
			_bPaddleHit = false;

		} else if (length >= WHEEL_RADIUS) {
			// The ball hit edge of Ferris Wheel
			if (_bOuterWall) {
				// Has ball hit right border
				if (_ptBallLocation.x >= GAME_WIDTH - GAME_RIGHT_BORDER_WIDTH - BALL_SIZE_X) {
					_ptBallLocation.x = GAME_WIDTH - GAME_RIGHT_BORDER_WIDTH - BALL_SIZE_X;
					_vBallVector.x = -_vBallVector.x;

					// Randomly rotate 1 or -1 degrees
					_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));

					_fTurboBoost = (double)12 - _nBallSpeed;

					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SOUND_ASYNCH | SND_NODEFAULT);
					}

				} else if (_ptBallLocation.x <= 0 + GAME_LEFT_BORDER_WIDTH) {
					// Ball hit left border
					_ptBallLocation.x = 0 + GAME_LEFT_BORDER_WIDTH;
					_vBallVector.x = -_vBallVector.x;

					// Randomly rotate 1 or -1 degrees
					_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));
					_fTurboBoost = (double)12 - _nBallSpeed;

					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

				} else if (_ptBallLocation.y >= GAME_HEIGHT - GAME_BOTTOM_BORDER_WIDTH - BALL_SIZE_Y) {
					// Ball hit bottom of screen
					_ptBallLocation.y = GAME_HEIGHT - GAME_BOTTOM_BORDER_WIDTH - BALL_SIZE_Y;
					_vBallVector.y = -_vBallVector.y;

					// Randomly rotate 1 or -1 degrees
					_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));
					_fTurboBoost = (double)12 - _nBallSpeed;

					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

				} else if (_ptBallLocation.y <= 0 + GAME_TOP_BORDER_WIDTH) {
					// Ball hit top of screen
					_ptBallLocation.y = 0 + GAME_TOP_BORDER_WIDTH;
					_vBallVector.y = -_vBallVector.y;

					// Randomly rotate 1 or -1 degrees
					_vBallVector.Rotate(Deg2Rad(getRandomNumber(1) ? 0.125 : 0));
					_fTurboBoost = (double)12 - _nBallSpeed;

					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}
				}

			} else {
				if (pGameParams->bSoundEffectsEnabled) {
					sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
				}

				// Pull ball back to just on the border
				_ptBallLocation -= _vBallVector * (length - WHEEL_RADIUS + 1);

				_vBallVector.x = -_vBallVector.x;
				_vBallVector.y = -_vBallVector.y;

				_vBallVector.Reflect(vBall);
			}

			_bPaddleHit = false;
		}

		// Only update the ball if it actually moved
		if ((ptLast.x != (int)_ptBallLocation.x) || (ptLast.y != (int)_ptBallLocation.y)) {
			if (_sprBall.isLinked()) {
				// Move the ball to it's new location
				_sprBall.x = _ptBallLocation.x;
				_sprBall.y = _ptBallLocation.y;
			}
		}
	}
}

void Fuge::movePaddle(bool bPaint) {
	CVector vPaddle;
	int nOldIndex;

	// Verify that the input was not tainted
	assert(_nPaddleCelIndex < N_PADDLE_CELS * 2);
	assert(_nPaddleCelIndex > -N_PADDLE_CELS);

	// Can't access a null pointer
	assert(!_sprPaddle.empty());

	// Get old cel index
	nOldIndex = _sprPaddle.getCelIndex();

	if (_nPaddleCelIndex >= N_PADDLE_CELS) {
		_nPaddleCelIndex = _nPaddleCelIndex - N_PADDLE_CELS;

	} else if (_nPaddleCelIndex < 0) {
		_nPaddleCelIndex = _nPaddleCelIndex + N_PADDLE_CELS;
	}

	// Verify our calculations
	assert((_nPaddleCelIndex >= 0) && (_nPaddleCelIndex < N_PADDLE_CELS));

	// Don't adjust the paddle if we would paint the same cel
	if (bPaint || (nOldIndex != _nPaddleCelIndex)) {
		_sprPaddle.setCel(_nPaddleCelIndex);

		// Move paddle to new location
		_sprPaddle.x = PADDLE_START_X;
		_sprPaddle.y = PADDLE_START_Y;

		// If the ball is resting on the paddle
		if (_bBallOnPaddle) {
			assert(!_sprBall.empty());

			// Ball is rotating with paddle
			_ptBallLocation = ballOnPaddle();

			// Set the ball to it's new location
			_sprBall.x = _ptBallLocation.x;
			_sprBall.y = _ptBallLocation.y;
		}
	}
}

void Fuge::loseBall() {
	CSound *pEffect = nullptr;
	ErrorCode errCode;

	// Assume no error
	errCode = ERR_NONE;

	// Pause the game
	gamePause();

	// Reset turbo
	_fTurboBoost = 0.0;

	_sprBall.unlinkSprite();

	assert(_nBalls > 0);

	// One less ball
	_nBalls--;

	if (pGameParams->bSoundEffectsEnabled) {
#if CSOUND
		pEffect = new CBofSound(this, WAV_LOSEBALL,
			SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
#else
		sndPlaySound(WAV_LOSEBALL, SND_ASYNC);
#endif
	}

	// If no more balls left - user has lost
	if (_nBalls == 0) {

		if (pGameParams->bSoundEffectsEnabled) {
#if CSOUND
			pEffect = new CSound(this, WAV_GAMEOVER,
				SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
#else
			sndPlaySound(WAV_GAMEOVER, SND_ASYNC);
#endif
		}

		Common::String msg = Common::String::format("Score:  %ld", _lScore);
		MessageBox::show("Game over.", msg, "GAME_OVER");

	} else {
		// Display score, and start a new ball
		Common::String title = Common::String::format("Score:  %ld", _lScore);
		Common::String msg = Common::String::format("Balls Left:  %d", _nBalls);
		MessageBox::show(title, msg, "NEW_LIFE");
	}
}

void Fuge::gameOverClosed() {
	if (pGameParams->bPlayingMetagame) {
		// Return the final score
		pGameParams->lScore = _lScore;
	}

	gameReset();
}

void Fuge::newLifeClosed() {
	// Reset the ball position
	endPaddle();
	startPaddle();
	startBall();
}

void Fuge::optionsClosed() {
	if (!pGameParams->bMusicEnabled && (_soundTrack != nullptr)) {
		_soundTrack->stop();
		delete _soundTrack;
		_soundTrack = nullptr;

	} else if (pGameParams->bMusicEnabled && (_soundTrack == nullptr)) {
		_soundTrack = new CBofSound(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		_soundTrack->midiLoopPlaySegment(5390, 32280, 0, FMT_MILLISEC);
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

	// Calculate the 7 critical points for the paddle
	vTmp.SetVector(0, -PADDLE_RADIUS);

	// Cel index determines paddle angle
	vTmp.Rotate((2 * PI / N_PADDLE_CELS) * _nPaddleCelIndex);

	vPoints[1] = _gvCenter + vTmp;

	vPoints[0] = vTmp;
	vPoints[0].Rotate(fPaddleAngles[_nInitPaddleSize] / 2);
	vPoints[0] += _gvCenter;

	vPoints[2] = vTmp;
	vPoints[2].Rotate(fPaddleAngles[_nInitPaddleSize]);
	vPoints[2] += _gvCenter;

	vTmp.Unitize();

	vTmp *= 25;

	vPoints[3] = _gvCenter + vTmp;

	vPoints[4] = vTmp;
	vPoints[4].Rotate(fPaddleAngles[_nInitPaddleSize]);
	vPoints[4] += _gvCenter;

	vPoints[5] = vPoints[1];
	vPoints[6] = vPoints[2];
	if (_nInitPaddleSize > PSIZE_MIN) {
		vTmp = vPoints[0] - vPoints[1];
		vTmp.Unitize();
		vTmp *= 9; // Paddle width

		vPoints[5] = vPoints[1] + vTmp;

		vTmp = vPoints[0] - vPoints[2];
		vTmp.Unitize();
		vTmp *= 9; // Paddle width
		vPoints[6] = vPoints[2] - vTmp;
	}

	// Get center of the ball
	vBallCenter = _ptBallLocation + BALL_RADIUS;

	// If any of those points are less than the radius distance
	// away from the center of the ball, then the ball has hit
	// the paddle
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
		nRollBack = MIN(_nInitBallSpeed, nRollBack);

		if (pGameParams->bSoundEffectsEnabled) {
			sndPlaySound(m_pPaddleSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
		}

		// If we hit the ball twice in a row
		if (_bPaddleHit) {
			// Then shoot the ball away from the paddle
			_vBallVector = vBallCenter - _gvCenter;
			_vBallVector.Unitize();
			_vBallVector *= 2;

		} else {
			// Roll the ball back to the exact point that it hit the paddle
			vTmp = _vBallVector;
			vTmp.Unitize();
			vTmp *= nRollBack;

			_ptBallLocation -= vTmp;

			// Get center of ball
			vBallCenter = _ptBallLocation + BALL_RADIUS;

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
			if (_nInitPaddleSize > PSIZE_MIN) {

				fLen5 = distanceBetweenPoints(vBallCenter, vPoints[1]);
				fLen5 += distanceBetweenPoints(vBallCenter, vPoints[5]);
				fMin = MIN(fMin, fLen5);

				fLen6 = distanceBetweenPoints(vBallCenter, vPoints[2]);
				fLen6 += distanceBetweenPoints(vBallCenter, vPoints[6]);
				fMin = MIN(fMin, fLen6);
			}

			vTmp = _vBallVector;
			_vBallVector.x = -_vBallVector.x;
			_vBallVector.y = -_vBallVector.y;

			vFace = vPoints[0] - _gvCenter;

			if ((fMin == fLen1) || (fMin == fLen5) || (fMin == fLen2) || (fMin == fLen6)) {

				if (fMin == fLen5) {
					vFace = vBallCenter - _gvCenter;
					vFace.Rotate(-Deg2Rad(10));

				} else if (fMin == fLen6) {
					vFace = vBallCenter - _gvCenter;
					vFace.Rotate(Deg2Rad(10));
				}

				// Get center of ball
				vBallCenter = _ptBallLocation + BALL_RADIUS;

				// Roll ball back to edge of paddle
				while (distanceBetweenPoints(vBallCenter, _gvCenter) < PADDLE_RADIUS + BALL_RADIUS) {
					_ptBallLocation -= vTmp;

					// Get center of ball
					vBallCenter = _ptBallLocation + BALL_RADIUS;
				}

				vPaddle = vFace;

			} else if (fMin == fLen3) {

				vPaddle.SetVector(vPoints[3].x - CENTER_X, vPoints[3].y - CENTER_Y);
				vPaddle.Rotate(-PI / 2);

				a1 = vPaddle.AngleBetween(_vBallVector);
				a2 = vFace.AngleBetween(_vBallVector);

				// Kludge to compensate for when angle is too big
				if (a1 > a2) {
					vPaddle = vFace;
				}

			} else if (fMin == fLen4) {

				vPaddle.SetVector(vPoints[4].x - CENTER_X, vPoints[4].y - CENTER_Y);
				vPaddle.Rotate(PI / 2);

				a1 = vPaddle.AngleBetween(_vBallVector);
				a2 = vFace.AngleBetween(_vBallVector);

				// Kludge to compensate for when angle is too big
				if (a1 > a2) {
					vPaddle = vFace;
				}
			}

			// Reflect the ball vector around the final paddle (mirror) vector
			_vBallVector.Reflect(vPaddle);

			// One final check to make sure the ball is bouncing in the
			// correct direction.
			if (_vBallVector.AngleBetween(_gvCenter - vBallCenter) <= Deg2Rad(15)) {
				_vBallVector.Rotate(Deg2Rad(180));
			}
		}

		_bPaddleHit = true;
	}
}

#define MAX_BRICK_HITS 6

void Fuge::ballvsBrick(double length) {
	CVector vPoints[N_BRICK_POINTS];
	CVector vBrick, vBallCenter, vOrigin, vTmp;
	CSound *pEffect = nullptr;
	Common::Rect rTmpRect, rBall, cRect;
	Common::Point ptTmp;
	double fMin, fLast, fLen[N_BRICK_POINTS];
	double angle;
	int i, j, nIndex, nLastIndex, nBrickIndex, nMaxHits;
	int nBrick0, nBrick1, nRow0, nRow1, nRow2, nUse[MAX_BRICK_HITS];
	bool bHit, bStillHit;
	Common::String title, msg;

	// Get bounding rectangle of the ball
	rBall = Common::Rect((int)_ptBallLocation.x, (int)_ptBallLocation.y,
		(int)_ptBallLocation.x + BALL_SIZE_X,
		(int)_ptBallLocation.y + BALL_SIZE_Y);

	// Get center of the ball
	vBallCenter.SetVector(_ptBallLocation.x + BALL_RADIUS, _ptBallLocation.y + BALL_RADIUS);

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

		if (_brickVisible[nBrickIndex]) {
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
					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pBrickSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					// Remove the brick
					eraseBrick(nBrickIndex);

					// One less brick
					--_nBricks;

					// Score: 1 point for each brick regardless of color
					_lScore += 1;

					// Did user earn an extra ball?
					if (_lScore >= _lExtraLifeScore) {
						if (pGameParams->bSoundEffectsEnabled) {
							sndPlaySound(m_pExtraLifeSound, SND_MEMORY | SND_SYNC | SND_NODEFAULT);
						}

						// Double the amount the user needs for their next extra life
						_lExtraLifeScore += _lExtraLifeScore;

						// Extra ball
						_nBalls++;
					}

					// If no bricks left
					if (_nBricks == 0) {
						gamePause();

						// Reset turbo
						_fTurboBoost = 0.0;

						if (pGameParams->bSoundEffectsEnabled) {
#if CSOUND
							pEffect = new CBofSound(this, WAV_WINWAVE,
								SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
							(*pEffect).play();                                                      //...play the narration
#else
							sndPlaySound(WAV_WINWAVE, SND_ASYNC);
#endif
						}

						// 5 point bonus for clearing all bricks
						_lScore += 5;

						//
						// User wins this round
						title = Common::String::format("Round complete.");
						msg = Common::String::format("Score:  %ld", _lScore);
						MessageBox::show(title, msg, "ROUND_COMPLETE");

					} else {
						// There are more bricks left,
						// So just calc a new vector for the ball

						bStillHit = true;
						while (bStillHit) {
							// Roll ball back to point of contact with brick
							_ptBallLocation -= _vBallVector;

							// Get new center of ball
							vBallCenter = _ptBallLocation + BALL_RADIUS;

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
						_vBallVector.x = -_vBallVector.x;
						_vBallVector.y = -_vBallVector.y;

						_vBallVector.Reflect(vBrick);
					}
					break;
				}
			}
		}
	}
}

void Fuge::roundCompleteClosed() {
	// Stop all sounds
	if (pGameParams->bSoundEffectsEnabled) {
#if CSOUND
#else
		sndPlaySound(nullptr, SND_ASYNC);
#endif
	}

	_nNumRows++;
	if (_nNumRows > N_ROWS) {
		_nNumRows = 1;
		_nBallSpeed++;
	}

	// Get new brick count
	_nBricks = _nNumRows * BRICKS_PER_ROW;

	if (pGameParams->bPlayingMetagame) {
		// If user is playing the metagame
		// return to the metagame
		pGameParams->lScore = _lScore;
		close();

	} else {
		// Reset the ball position
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
