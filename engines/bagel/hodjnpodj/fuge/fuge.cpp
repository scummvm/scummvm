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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/fuge/fuge.h"
#include "bagel/hodjnpodj/fuge/usercfg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

extern CWnd *ghParentWnd;

#define CSOUND      0

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP   ".\\ART\\FUGE6.BMP"

// Fuge sprite bitmaps
//
#define IDB_BALL        103

#define N_PADDLE_CELS    31
#define PADDLE_START_X  285
#define PADDLE_START_Y  205
#define PADDLE_CEL_JUMP   1
#define FACE_ANGLE      (double)0.977322
#define PADDLE0_ANGLE   (PI/4)      // 45 degrees
#define PADDLE1_ANGLE   (PI/3)      // 60 degrees
#define PADDLE2_ANGLE   (PI/2)      // 90 degrees

#define MOUSE_SENS      32

#define BLACKHOLE_RADIUS     14
#define PADDLE_RADIUS        34
#define INNER_BRICK_RADIUS  102
#define ROW6_RADIUS         114
#define ROW5_RADIUS         129
#define ROW4_RADIUS         145
#define ROW3_RADIUS         160
#define ROW2_RADIUS         175
#define ROW1_RADIUS         190
#define WHEEL_RADIUS        191

#define BRICK0_ANGLE        0.000000
#define BRICK1_ANGLE        0.392699
#define BRICK2_ANGLE        0.785398
#define BRICK3_ANGLE        1.178097
#define BRICK4_ANGLE        1.570796
#define BRICK5_ANGLE        1.963495
#define BRICK6_ANGLE        2.356194
#define BRICK7_ANGLE        2.748894
#define BRICK8_ANGLE        3.141593
#define BRICK9_ANGLE        3.534291
#define BRICK10_ANGLE       3.926990
#define BRICK11_ANGLE       4.319690
#define BRICK12_ANGLE       4.712389
#define BRICK13_ANGLE       5.105088
#define BRICK14_ANGLE       5.497787
#define BRICK15_ANGLE       5.890486


#define N_BLACKHOLE_CELS  5

#define N_BRICK_POINTS   21
#define N_BALL_MOVES      3

#define BALL_START_X    309
#define BALL_START_Y    180
#define BALL_SIZE_X      20
#define BALL_SIZE_Y      20
#define BALL_RADIUS     (BALL_SIZE_X/2)
#define CENTER_X        319
#define CENTER_Y        239
#define G_FORCE         0.009000

#define TURBO_MIN       0
#define TURBO_DEF      10
#define TURBO_MAX      10

#define EXTRA_LIFE_SCORE 100

//
// Button ID constants
//
#define IDC_MENU     100

#define TIMER_ID 10
#define TIMER_INTERVAL  50

// Sounds
//
#define WAV_PADDLE      ".\\SOUND\\PADDLE.WAV"
#define WAV_WALL        ".\\SOUND\\WALL.WAV"
#define WAV_WINWAVE     ".\\SOUND\\WINWAVE.WAV"
#define WAV_GAMEOVER    ".\\SOUND\\SOSORRY.WAV"     //GAMEOVER.WAV"
#define WAV_LOSEBALL    ".\\SOUND\\TOILET.WAV"      //LOSEBALL.WAV"

#define WAV_NARRATION   ".\\SOUND\\FUGE.WAV"
#define MID_SOUNDTRACK  ".\\SOUND\\FUGE.MID"

#define WAV_CAR1        ".\\SOUND\\SICK.WAV"
#define WAV_CAR2        ".\\SOUND\\FERRIS.WAV"
#define WAV_TENT        ".\\SOUND\\LAWYER.WAV"
#define WAV_BOOTH       ".\\SOUND\\STEPUP.WAV"
#define WAV_PEOPLE1     ".\\SOUND\\BALLOON.WAV"
#define WAV_PEOPLE2     ".\\SOUND\\AUNTEDNA.WAV"

#define N_PADDLE_SIZES (PSIZE_MAX+1)

STATIC const char *pszPaddles[N_PADDLE_SIZES] = {
	".\\ART\\PADCEL45.BMP",
	".\\ART\\PADCEL60.BMP",
	".\\ART\\PADCEL90.BMP"
};

STATIC const double fPaddleAngles[N_PADDLE_SIZES] = {
	PADDLE0_ANGLE,
	PADDLE1_ANGLE,
	PADDLE2_ANGLE
};

#define NUM_WAVS        2                           //There are two people and two car sounds

// Audio Easter Egg areas
//
#define TENT_X          16
#define TENT_Y          320
#define TENT_DX         75
#define TENT_DY         47

#define BOOTH_X         503
#define BOOTH_Y         377
#define BOOTH_DX        93
#define BOOTH_DY        53

#define PEOPLE_X        27
#define PEOPLE_Y        370
#define PEOPLE_DX       102
#define PEOPLE_DY       60

#define CAR1_X          131
#define CAR1_Y          385
#define CAR_DX          41
#define CAR_DY          49

#define CAR2_X          84
#define CAR2_Y          231

#define CAR3_X          95
#define CAR3_Y          149

#define CAR4_X          133
#define CAR4_Y          73

#define CAR5_X          465
#define CAR5_Y          75

#define CAR6_X          503
#define CAR6_Y          150

#define CAR7_X          514
#define CAR7_Y          230

#define CAR8_X          500
#define CAR8_Y          315

#define CAR9_X          218
#define CAR9_Y          23
#define CAR9_DX         37
#define CAR9_DY         39

#define CAR10_X         384
#define CAR10_Y         23
#define CAR10_DX        39
#define CAR10_DY        39


//
// Globals
//
STATIC const char *pszFugeArt[N_ROWS + 1] = {
	".\\ART\\FUGE6.BMP",
	".\\ART\\FUGE1.BMP",
	".\\ART\\FUGE2.BMP",
	".\\ART\\FUGE3.BMP",
	".\\ART\\FUGE4.BMP",
	".\\ART\\FUGE5.BMP",
	".\\ART\\FUGE6.BMP"
};

STATIC CPalette    *pGamePalette;
const char  *INI_SECTION = "Fuge";
LPGAMESTRUCT pGameParams;

extern HWND ghParentWnd;

typedef struct {
	VECTOR v1, v2;
} BRICK_VECTORS;

STATIC const BRICK_VECTORS aBrickVectors[BRICKS_PER_ROW] = {
	{{-1,  1, 0}, { 5, -2, 0}},
	{{-5,  2, 0}, { 1,  0, 0}},
	{{-1,  0, 0}, { 5,  2, 0}},
	{{-5, -2, 0}, { 1,  1, 0}},
	{{-1, -1, 0}, { 2,  5, 0}},
	{{-2, -5, 0}, { 0,  1, 0}},
	{{ 0, -1, 0}, {-2,  5, 0}},
	{{ 2, -5, 0}, {-1,  1, 0}},
	{{ 1, -1, 0}, {-5,  2, 0}},
	{{ 5, -2, 0}, {-1,  0, 0}},
	{{ 1,  0, 0}, {-5, -2, 0}},
	{{ 5,  2, 0}, {-1, -1, 0}},
	{{ 1,  1, 0}, {-2, -5, 0}},
	{{ 2,  5, 0}, { 0, -1, 0}},
	{{ 0,  1, 0}, { 2, -5, 0}},
	{{-2,  5, 0}, { 1, -1, 0}}
};


STATIC const VECTOR BRICK_CRIT_POINTS[N_ROWS][N_BRICK_POINTS] = {
	{	{  0, -191, 0}, {  8, -191, 0}, { 15, -190, 0}, { 23, -190, 0}, { 31, -188, 0},
		{ 39, -187, 0}, { 47, -185, 0}, { 55, -183, 0}, { 62, -181, 0}, { 69, -178, 0},
		{ 67, -172, 0}, { 64, -165, 0}, { 56, -168, 0}, { 48, -171, 0}, { 40, -173, 0},
		{ 32, -174, 0}, { 24, -175, 0}, { 16, -176, 0}, {  8, -177, 0}, {  0, -177, 0},
		{  0, -184, 0}
	},

	{	{  0, -175, 0}, {  7, -175, 0}, { 14, -175, 0}, { 21, -174, 0}, { 28, -173, 0},
		{ 35, -172, 0}, { 42, -170, 0}, { 49, -168, 0}, { 56, -166, 0}, { 63, -164, 0},
		{ 61, -158, 0}, { 59, -152, 0}, { 52, -154, 0}, { 44, -157, 0}, { 36, -159, 0},
		{ 29, -160, 0}, { 21, -162, 0}, { 14, -162, 0}, {  7, -163, 0}, {  0, -163, 0},
		{  0, -169, 0}
	},

	{	{  0, -161, 0}, {  7, -161, 0}, { 13, -160, 0}, { 19, -160, 0}, { 26, -159, 0},
		{ 33, -158, 0}, { 39, -156, 0}, { 45, -155, 0}, { 51, -153, 0}, { 58, -150, 0},
		{ 56, -143, 0}, { 54, -137, 0}, { 48, -139, 0}, { 41, -141, 0}, { 35, -143, 0},
		{ 28, -144, 0}, { 21, -146, 0}, { 14, -146, 0}, {  7, -147, 0}, {  0, -147, 0},
		{  0, -154, 0}
	},

	{	{  0, -145, 0}, {  7, -145, 0}, { 13, -145, 0}, { 19, -144, 0}, { 25, -143, 0},
		{ 31, -142, 0}, { 36, -141, 0}, { 41, -139, 0}, { 47, -138, 0}, { 53, -135, 0},
		{ 51, -129, 0}, { 48, -122, 0}, { 42, -124, 0}, { 37, -126, 0}, { 31, -127, 0},
		{ 26, -129, 0}, { 19, -130, 0}, { 13, -130, 0}, {  7, -131, 0}, {  0, -131, 0},
		{  0, -138, 0}
	},

	{	{  0, -129, 0}, {  6, -129, 0}, { 11, -129, 0}, { 16, -128, 0}, { 21, -127, 0},
		{ 26, -127, 0}, { 31, -125, 0}, { 36, -124, 0}, { 41, -123, 0}, { 47, -121, 0},
		{ 45, -115, 0}, { 43, -109, 0}, { 37, -111, 0}, { 32, -112, 0}, { 27, -114, 0},
		{ 22, -115, 0}, { 17, -116, 0}, { 11, -116, 0}, {  6, -117, 0}, {  0, -117, 0},
		{  0, -123, 0}
	},

	{	{  0, -115, 0}, {  6, -115, 0}, { 11, -115, 0}, { 16, -114, 0}, { 20, -113, 0},
		{ 25, -112, 0}, { 29, -111, 0}, { 33, -110, 0}, { 37, -109, 0}, { 42, -107, 0},
		{ 40, -102, 0}, { 38,  -96, 0}, { 33,  -98, 0}, { 29,  -99, 0}, { 25, -100, 0},
		{ 21, -101, 0}, { 16, -102, 0}, { 11, -102, 0}, {  6, -103, 0}, {  0, -103, 0},
		{  0, -109, 0}
	}
};
STATIC VECTOR vBrickCritPoints[N_ROWS][N_BRICK_POINTS];

STATIC const POINT ptBrickPos[N_BRICKS] = {
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

STATIC const SIZE ptBrickSize[N_BRICKS] = {
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


// Local Prototypes
//
void CALLBACK GetGameParams(CWnd *);

CFugeWindow::CFugeWindow() : gvCenter(CENTER_X, CENTER_Y) {
	CString  WndClass;
	CRect    tmpRect;
	CDC     *pDC;
	CBitmap *pBmp;
	ERROR_CODE errCode;
	bool bSuccess;

	// assume no error
	errCode = ERR_NONE;

	// Initialize fields
	initMembers();
	initStatics();

	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y, NEWGAME_LOCATION_X + NEWGAME_WIDTH, NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	// can't play this game if the background art is not available
	//
	if (FileExists(MINI_GAME_MAP)) {

		// get this game's palette
		//
		if ((pDC = GetDC()) != nullptr) {
			if ((pBmp = FetchBitmap(pDC, &m_pGamePalette, MINI_GAME_MAP)) != nullptr) {
				delete pBmp;
				pBmp = nullptr;
				pGamePalette = m_pGamePalette;
			}
			ReleaseDC(pDC);
		}

	} else {
		errCode = ERR_FFIND;
	}

	// Center our window on the screen
	//
	tmpRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	#ifndef DEBUG
	if ((pDC = GetDC()) != nullptr) {
		tmpRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
		tmpRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
		tmpRect.right = tmpRect.left + GAME_WIDTH;
		tmpRect.bottom = tmpRect.top + GAME_HEIGHT;
		ReleaseDC(pDC);
	} else {
		errCode = ERR_UNKNOWN;
	}
	#endif

	// set mouse anchor point as center or game window
	//
	m_ptOrigin.x = GAME_WIDTH / 2 + tmpRect.left;
	m_ptOrigin.y = GAME_HEIGHT / 2 + tmpRect.top;

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	//
	Create(WndClass, "Boffo Games -- Fuge", WS_POPUP, tmpRect, nullptr, 0);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	PaintScreen();
	EndWaitCursor();

	ClipCursor(&tmpRect);

	// only continue if there was no error
	//
	if (errCode == ERR_NONE) {

		if ((m_pScrollButton = new CBmpButton) != nullptr) {

			m_bIgnoreScrollClick = false;
			tmpRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y, SCROLL_BUTTON_X + SCROLL_BUTTON_DX, SCROLL_BUTTON_Y + SCROLL_BUTTON_DY);
			bSuccess = m_pScrollButton->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, tmpRect, this, IDC_MENU);
			assert(bSuccess);
			if (bSuccess) {
				bSuccess = m_pScrollButton->LoadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);
				assert(bSuccess);
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_MEMORY;
		}

		// only continue if there was no error
		//
		if (errCode == ERR_NONE) {

			BeginWaitCursor();

			// Start the Fuge soundtrack
			//
			if (pGameParams->bMusicEnabled) {
				if ((m_pSoundTrack = new CSound) != nullptr) {
					m_pSoundTrack->initialize(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					m_pSoundTrack->midiLoopPlaySegment(5390, 32280, 0, FMT_MILLISEC);
				} else {
					errCode = ERR_MEMORY;
				}
			}

			// seed the random number generator
			//srand((unsigned int)time(nullptr));

			//
			// The vector table is rotated by 11 or so degrees, because it
			// was easier to type in (125, 320) as opposed to (114.452865, 300.526372).
			// So I do the conversion here.
			//
			RealignVectors();

			if (!errCode)
				errCode = LoadMasterSprites();

			if (!errCode)
				errCode = LoadMasterSounds();

			InitializeJoystick();

			EndWaitCursor();

			// as long as there was no error
			//
			if (!errCode) {

				// if we are not playing from the metagame
				//
				if (!pGameParams->bPlayingMetagame) {

					// Automatically bring up the main menu
					PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
				}
			}
		}
	}

	HandleError(errCode);
}

void CFugeWindow::initMembers() {
	m_nInitNumBalls = BALLS_DEF;
	m_nInitStartLevel = LEVEL_DEF;
	m_nInitBallSpeed = SPEED_DEF;
	m_nInitPaddleSize = PSIZE_DEF;
	m_nGForceFactor = GFORCE_DEF;

	m_pScrollButton = nullptr;
	m_pGamePalette = nullptr;
	m_pSoundTrack = nullptr;
	m_bPause = false;
	m_bGameActive = false;
	m_bIgnoreScrollClick = false;
	m_bBallOnPaddle = false;
	m_nPaddleCelIndex = 29;
	m_pPaddle = nullptr;
	m_pBall = nullptr;
	m_bMovingPaddle = false;
	m_lScore = 0;
	m_pBrickSound = nullptr;
	m_pWallSound = nullptr;
	m_pPaddleSound = nullptr;
	m_pExtraLifeSound = nullptr;
	m_hBrickRes = nullptr;
	m_hWallRes = nullptr;
	m_hPaddleRes = nullptr;
	m_hExtraLifeRes = nullptr;
	pGameParams->lScore = 0;
	m_nNumRows = 0;
	m_bJoyActive = false;
	memset(m_bBrickVisible, 0, sizeof(bool) * N_BRICKS);
}

void CFugeWindow::initStatics() {
	pGamePalette = nullptr;

	// Set up the brick critical points from defaults
	Common::copy(&BRICK_CRIT_POINTS[0][0], &BRICK_CRIT_POINTS[0][0] + N_ROWS * N_BRICK_POINTS,
		&vBrickCritPoints[0][0]);
}

void CFugeWindow::InitializeJoystick() {
	JOYINFO     joyInfo;

	if (joySetCapture(m_hWnd, JOYSTICKID1, 10000, true) == JOYERR_NOERROR) {
		//
		// Calibrate the joystick
		//
		joySetThreshold(JOYSTICKID1, 5000);
		joyGetPos(JOYSTICKID1, &joyInfo);
		m_nJoyOrgX = joyInfo.wXpos;
		m_nJoyOrgY = joyInfo.wYpos;
		m_bJoyActive = true;

	} else {
		//CMessageBox dlgNoJoystick((CWnd *)this, m_pGamePalette, "Warning!  No Joystick", "Driver Installed");
	}
}


void CFugeWindow::RealignVectors() {
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


ERROR_CODE CFugeWindow::LoadMasterSprites() {
	CDC *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	errCode = LoadNewPaddle(m_nInitPaddleSize);

	if (errCode == ERR_NONE) {

		if ((pDC = GetDC()) != nullptr) {

			if ((m_pBall = new CSprite) != nullptr) {

				if (m_pBall->SharePalette(m_pGamePalette) != false) {

					if (m_pBall->LoadResourceSprite(pDC, IDB_BALL) != false) {

						m_pBall->SetMasked(true);
						m_pBall->SetMobile(true);

						// uncomment this if we decide to animate the ball as
						// it moves
						//if (m_pBall->LoadResourceCels(pDC, IDB_BALLSTRIP, N_BALLS) == false)
						//    errCode = ERR_UNKNOWN;

					} else {
						errCode = ERR_UNKNOWN;
					}

				} else {
					errCode = ERR_UNKNOWN;
				}

			} else {
				errCode = ERR_MEMORY;
			}
			ReleaseDC(pDC);

		} else {
			errCode = ERR_MEMORY;
		}
	}

	return errCode;
}

void CFugeWindow::ReleaseMasterSprites() {
	assert(m_pBall != nullptr);
	if (m_pBall != nullptr) {
		delete m_pBall;
		m_pBall = nullptr;
	}

	assert(m_pPaddle != nullptr);
	if (m_pPaddle != nullptr) {
		delete m_pPaddle;
		m_pPaddle = nullptr;
	}
}

void CFugeWindow::HandleError(ERROR_CODE errCode) {
	//
	// Exit this application on fatal errors
	//
	if (errCode != ERR_NONE) {

		// pause the current game (if any)
		GamePause();

		// Display Error Message to the user
		MessageBox(errList[errCode], "Fatal Error!", MB_OK | MB_ICONSTOP);

		// Force this application to terminate
		PostMessage(WM_CLOSE, 0, 0);

		// Don't allow a repaint (remove all WM_PAINT messages)
		ValidateRect(nullptr);
	}
}


void CFugeWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	Invalidate(false);
	BeginPaint(&lpPaint);
	PaintScreen();
	EndPaint(&lpPaint);
}


void CFugeWindow::PaintScreen() {
	CDC      *pDC;

	//
	// Paint the background art and upadate any sprites called by OnPaint
	//

	if ((pDC = GetDC()) != nullptr) {

		// painting the bricks will paint the screen
		//
		PaintBricks(pDC);

		RepaintSpriteList(pDC);

		ReleaseDC(pDC);
	}
}


/*****************************************************************************
*
*  RepaintSpriteList -
*
*  DESCRIPTION:     Longer description of this function.  Continued onto next
*                   line like this.
*
*  SAMPLE USAGE:
*  errCode = RepaintSpriteList(pDC);
*  CDC *pDC;                                pointer to current device context
*
*  RETURNS:  ERROR_CODE = error return code
*
*****************************************************************************/
void CFugeWindow::RepaintSpriteList(CDC *pDC) {
	CSprite *pSprite;

	// can't use a null pointer
	assert(pDC != nullptr);

	//
	// Paint each sprite
	//
	pSprite = CSprite::GetSpriteChain();
	while (pSprite) {

		pSprite->ClearBackground();

		pSprite = pSprite->GetNextSprite();
	}

	pSprite = CSprite::GetSpriteChain();
	while (pSprite) {

		pSprite->RefreshSprite(pDC);

		pSprite = pSprite->GetNextSprite();
	}
}


bool CFugeWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CMainMenu COptionsWind((CWnd *)this,
	                       m_pGamePalette,
	                       (pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0) | (m_bGameActive ? 0 : NO_RETURN),
	                       GetGameParams, "fuge.txt", (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr), pGameParams);

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		//
		// must bring up our menu of controls
		//
		case IDC_MENU:

			// hide the command scroll
			//
			m_pScrollButton->SendMessage(BM_SETSTATE, true, 0);

			if (!m_bIgnoreScrollClick) {

				m_bIgnoreScrollClick = true;

				GamePause();

				CSound::waitWaveSounds();

				// Get users choice from command menu
				//
				switch (COptionsWind.DoModal()) {

				// User has chosen to play a new game
				//
				case IDC_OPTIONS_NEWGAME:
					PlayGame();
					break;

				// User has chosen to quit this mini-game
				//
				case IDC_OPTIONS_QUIT:
					PostMessage(WM_CLOSE, 0, 0);
					break;

				default:
					break;
				}

				// show the command scroll
				//
				m_pScrollButton->SendMessage(BM_SETSTATE, false, 0);
				m_bIgnoreScrollClick = false;

				if (!pGameParams->bMusicEnabled && (m_pSoundTrack != nullptr)) {

					m_pSoundTrack->stop();
					delete m_pSoundTrack;
					m_pSoundTrack = nullptr;

				} else if (pGameParams->bMusicEnabled && (m_pSoundTrack == nullptr)) {

					if ((m_pSoundTrack = new CSound) != nullptr) {
						m_pSoundTrack->initialize(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
						m_pSoundTrack->midiLoopPlaySegment(5390, 32280, 0, FMT_MILLISEC);
					}
				}

				GameResume();
			}

			return true;
		}
	}

	return false;
}


void CFugeWindow::GamePause() {
	m_bPause = true;
}


void CFugeWindow::GameResume() {
	if (!m_bBallOnPaddle && m_bMovingPaddle)
		m_bPause = false;
}


void CFugeWindow::PlayGame() {
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// load the .INI settings
	//
	LoadIniSettings();

	// reset all game parameters
	//
	GameReset();

	errCode = LoadNewPaddle(m_nInitPaddleSize);

	if (errCode == ERR_NONE) {

		//
		// Start game
		//

		// link
		StartBricks();
		StartBall();
		StartPaddle();

		// game starts paused
		m_bPause = true;
		m_bGameActive = true;
		SetTimer(TIMER_ID, TIMER_INTERVAL, nullptr);
	}

	HandleError(errCode);
}

ERROR_CODE CFugeWindow::LoadNewPaddle(int nNewSize) {
	CDC *pDC;
	ERROR_CODE errCode;

	assert(nNewSize >= PSIZE_MIN && nNewSize <= PSIZE_MAX);

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		// don't try to load the same paddle
		//
		if (m_nOldSize != nNewSize) {

			if (m_pPaddle != nullptr) {
				m_pPaddle->EraseSprite(pDC);
				m_pPaddle->UnlinkSprite();
				delete m_pPaddle;
				m_pPaddle = nullptr;
			}

			if ((m_pPaddle = new CSprite) != nullptr) {

				if (m_pPaddle->SharePalette(m_pGamePalette) != false) {

					if (m_pPaddle->LoadCels(pDC, pszPaddles[nNewSize], N_PADDLE_CELS) != false) {

						m_nOldSize = nNewSize;
						m_pPaddle->SetMasked(true);
						m_pPaddle->SetMobile(true);
						m_pPaddle->SetAnimated(true);

					} else {
						errCode = ERR_UNKNOWN;
					}

				} else {
					errCode = ERR_UNKNOWN;
				}

			} else {
				errCode = ERR_MEMORY;
			}
		}
		ReleaseDC(pDC);

	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}


ERROR_CODE CFugeWindow::LoadMasterSounds() {
	HANDLE hResInfo;
	HINSTANCE hInst;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	hInst = (HINSTANCE)GetWindowWord(m_hWnd, GWW_HINSTANCE);

	// Load and lock the Brick "ping" into memory
	//
	if ((hResInfo = FindResource(hInst, "brickSound", "WAVE")) != nullptr) {

		if ((m_hBrickRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

			if ((m_pBrickSound = (char *)LockResource((HGLOBAL)m_hBrickRes)) != nullptr) {

				// we have now loaded at least one of the master sounds

			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_UNKNOWN;
		}
	} else {
		errCode = ERR_UNKNOWN;
	}

	if (errCode == ERR_NONE) {

		// Load and lock the wall "ping" into memory
		//
		if ((hResInfo = FindResource(hInst, "wallSound", "WAVE")) != nullptr) {

			if ((m_hWallRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

				if ((m_pWallSound = (char *)LockResource((HGLOBAL)m_hWallRes)) == nullptr)
					errCode = ERR_UNKNOWN;
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_UNKNOWN;
		}
	}

	if (errCode == ERR_NONE) {

		// Load and lock the paddle "ping" into memory
		//
		if ((hResInfo = FindResource(hInst, "paddleSound", "WAVE")) != nullptr) {

			if ((m_hPaddleRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

				if ((m_pPaddleSound = (char *)LockResource((HGLOBAL)m_hPaddleRes)) == nullptr)
					errCode = ERR_UNKNOWN;
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_UNKNOWN;
		}
	}

	if (errCode == ERR_NONE) {

		// Load and lock the extra life sound into memory
		//
		if ((hResInfo = FindResource(hInst, "NewLife", "WAVE")) != nullptr) {

			if ((m_hExtraLifeRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

				if ((m_pExtraLifeSound = (char *)LockResource((HGLOBAL)m_hExtraLifeRes)) == nullptr)
					errCode = ERR_UNKNOWN;
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_UNKNOWN;
		}
	}

	return errCode;
}


void CFugeWindow::ReleaseMasterSounds() {
	if (m_hExtraLifeRes != nullptr) {
		FreeResource(m_hExtraLifeRes);
		m_hExtraLifeRes = nullptr;
	}
	if (m_pPaddleSound != nullptr) {
		FreeResource(m_hPaddleRes);
		m_pPaddleSound = nullptr;
	}
	if (m_pWallSound != nullptr) {
		FreeResource(m_hWallRes);
		m_pWallSound = nullptr;
	}
	if (m_pBrickSound != nullptr) {
		FreeResource(m_hBrickRes);
		m_pBrickSound = nullptr;
	}
}


void CFugeWindow::EndBall() {
	CDC *pDC;
	assert(m_pBall != nullptr);

	if ((pDC = GetDC()) != nullptr) {
		if (m_pBall->IsLinked()) {
			m_pBall->EraseSprite(pDC);
			m_pBall->UnlinkSprite();
		}
		ReleaseDC(pDC);
	}
}


void CFugeWindow::StartBall() {
	CDC *pDC;

	assert(m_pBall != nullptr);

	// have the ball start on the paddle like in arkinoids
	//

	m_ptBallLocation = BallOnPaddle();

	m_pBall->LinkSprite();
	if ((pDC = GetDC()) != nullptr) {
		m_pBall->PaintSprite(pDC, (int)m_ptBallLocation.x, (int)m_ptBallLocation.y);
		ReleaseDC(pDC);
	}
}


CVector CFugeWindow::BallOnPaddle() {
	CVector vBall(0, -(PADDLE_RADIUS + BALL_RADIUS));

	vBall.Rotate(fPaddleAngles[m_nInitPaddleSize] / 2);

	vBall.Rotate(m_nPaddleCelIndex * ((2 * PI) / N_PADDLE_CELS));
	vBall -= CVector(BALL_RADIUS, BALL_RADIUS);

	// this vector was relative to center so now make it a real point
	vBall += gvCenter;

	return vBall;
}


void CFugeWindow::PaintBall() {
	CPoint ptLast;
	CVector vBall, vGravity;
	double length;
	CDC *pDC;

	if (m_bGameActive && !m_bPause && !m_bBallOnPaddle) {

		assert(m_pBall != nullptr);

		ptLast.x = (int)m_ptBallLocation.x;
		ptLast.y = (int)m_ptBallLocation.y;

		vGravity = gvCenter - (m_ptBallLocation + BALL_RADIUS);
		vGravity.Unitize();

		// calc new ball location
		//
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

		// get radius of the ball from the center of the screen
		//
		vBall = m_ptBallLocation + BALL_RADIUS - gvCenter;

		length = vBall.Length() + BALL_RADIUS;

		// check to see if ball has entered the balck hole
		//
		if (length <= BLACKHOLE_RADIUS + BALL_RADIUS * 2) {

			// Play the ball-gets-sucked-into-black-hole animation
			//
			LoseBall();
			m_bPaddleHit = false;

			// or has ball hit the paddle?
			//
		} else if (length <= PADDLE_RADIUS + BALL_RADIUS * 2) {

			BallvsPaddle();

			// or has ball hit a brick?
			//
		} else if ((length >= INNER_BRICK_RADIUS) && (length < WHEEL_RADIUS + (m_bOutterWall ? BALL_RADIUS * 2 : 0))) {

			//
			// determine which row ball is in
			//
			BallvsBrick(length);

			//
			// determine if a ball actually hit a brick
			//
			m_bPaddleHit = false;


			// or did ball hit edge of ferris wheel
			//
		} else if (length >= WHEEL_RADIUS) {

			if (m_bOutterWall) {

				// has ball hit right border
				//
				if (m_ptBallLocation.x >= GAME_WIDTH - GAME_RIGHT_BORDER_WIDTH - BALL_SIZE_X) {

					m_ptBallLocation.x = GAME_WIDTH - GAME_RIGHT_BORDER_WIDTH - BALL_SIZE_X;

					m_vBallVector.x = -m_vBallVector.x;

					// randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad((brand() & 1) ? 0.125 : 0));

					m_fTurboBoost = (double)12 - m_nBallSpeed;

					if (pGameParams->bSoundEffectsEnabled) {

						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					// has ball hit left border
					//
				} else if (m_ptBallLocation.x <= 0 + GAME_LEFT_BORDER_WIDTH) {

					m_ptBallLocation.x = 0 + GAME_LEFT_BORDER_WIDTH;

					m_vBallVector.x = -m_vBallVector.x;

					// randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad((brand() & 1) ? 0.125 : 0));

					m_fTurboBoost = (double)12 - m_nBallSpeed;


					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					// has ball hit bottom of screen
					//
				} else if (m_ptBallLocation.y >= GAME_HEIGHT - GAME_BOTTOM_BORDER_WIDTH - BALL_SIZE_Y) {

					m_ptBallLocation.y = GAME_HEIGHT - GAME_BOTTOM_BORDER_WIDTH - BALL_SIZE_Y;

					m_vBallVector.y = -m_vBallVector.y;

					// randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad((brand() & 1) ? 0.125 : 0));

					m_fTurboBoost = (double)12 - m_nBallSpeed;


					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					// has ball hit top of screen
					//
				} else if (m_ptBallLocation.y <= 0 + GAME_TOP_BORDER_WIDTH) {

					m_ptBallLocation.y = 0 + GAME_TOP_BORDER_WIDTH;

					m_vBallVector.y = -m_vBallVector.y;

					// randomly rotate 1 or -1 degrees
					m_vBallVector.Rotate(Deg2Rad((brand() & 1) ? 0.125 : 0));

					m_fTurboBoost = (double)12 - m_nBallSpeed;

					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}
				}

			} else {

				if (pGameParams->bSoundEffectsEnabled) {
					sndPlaySound(m_pWallSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
				}

				// pull ball back to just on the border
				//
				m_ptBallLocation -= m_vBallVector * (length - WHEEL_RADIUS + 1);

				m_vBallVector.x = -m_vBallVector.x;
				m_vBallVector.y = -m_vBallVector.y;

				m_vBallVector.Reflect(vBall);
			}

			m_bPaddleHit = false;
		}

		// only paint the ball if it actually moved
		//
		if ((ptLast.x != (int)m_ptBallLocation.x) || (ptLast.y != (int)m_ptBallLocation.y)) {

			if (m_pBall->IsLinked()) {

				// paint the ball to it's new location
				//
				if ((pDC = GetDC()) != nullptr) {
					m_pBall->PaintSprite(pDC, (int)m_ptBallLocation.x, (int)m_ptBallLocation.y);
					ReleaseDC(pDC);
				}
			}
		}
	}
}


#define N_CRIT_POINTS 7

void CFugeWindow::BallvsPaddle() {
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

	vPoints[1] = gvCenter + vTmp;

	vPoints[0] = vTmp;
	vPoints[0].Rotate(fPaddleAngles[m_nInitPaddleSize] / 2);
	vPoints[0] += gvCenter;

	vPoints[2] = vTmp;
	vPoints[2].Rotate(fPaddleAngles[m_nInitPaddleSize]);
	vPoints[2] += gvCenter;

	vTmp.Unitize();

	vTmp *= 25;

	vPoints[3] = gvCenter + vTmp;

	vPoints[4] = vTmp;
	vPoints[4].Rotate(fPaddleAngles[m_nInitPaddleSize]);
	vPoints[4] += gvCenter;

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

		if (pGameParams->bSoundEffectsEnabled) {
			sndPlaySound(m_pPaddleSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
		}

		// if we hit the ball twice in a row
		//
		if (m_bPaddleHit) {

			TRACE("DoubleHit\n");

			// then shoot the ball away from the paddle
			//
			m_vBallVector = vBallCenter - gvCenter;
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

			vFace = vPoints[0] - gvCenter;

			if ((fMin == fLen1) || (fMin == fLen5) || (fMin == fLen2) || (fMin == fLen6)) {

				if (fMin == fLen5) {
					vFace = vBallCenter - gvCenter;
					vFace.Rotate(-Deg2Rad(10));

				} else if (fMin == fLen6) {
					vFace = vBallCenter - gvCenter;
					vFace.Rotate(Deg2Rad(10));
				}

				// get center of ball
				vBallCenter = m_ptBallLocation + BALL_RADIUS;

				// roll ball back to edge of paddle
				//
				while (distanceBetweenPoints(vBallCenter, gvCenter) < PADDLE_RADIUS + BALL_RADIUS) {

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
			if (m_vBallVector.AngleBetween(gvCenter - vBallCenter) <= Deg2Rad(15)) {
				//TRACE("RealAjusting the Vector\n");
				m_vBallVector.Rotate(Deg2Rad(180));
			}
		}

		m_bPaddleHit = true;
	}
}

#define MAX_BRICK_HITS 6


void CFugeWindow::BallvsBrick(double length) {
	char buf1[32], buf2[32];
	CVector vPoints[N_BRICK_POINTS];
	CVector vBrick, vBallCenter, vOrigin, vTmp;
	CDC *pDC;
	CRect rTmpRect, rBall, cRect;
	CPoint ptTmp;
	double fMin, fLast, fLen[N_BRICK_POINTS];
	double angle;
	int i, j, nIndex, nLastIndex, nBrickIndex, nMaxHits;
	int nBrick0, nBrick1, nRow0, nRow1, nRow2, nUse[MAX_BRICK_HITS];
	bool bHit, bStillHit;

	// get bounding rectangle of the ball
	//
	rBall.SetRect((int)m_ptBallLocation.x, (int)m_ptBallLocation.y, (int)m_ptBallLocation.x + BALL_SIZE_X, (int)m_ptBallLocation.y + BALL_SIZE_Y);

	// get center of the ball
	vBallCenter.SetVector(m_ptBallLocation.x + BALL_RADIUS, m_ptBallLocation.y + BALL_RADIUS);

	vOrigin.SetVector(-1, -1);
	vOrigin.Rotate(PI / BRICKS_PER_ROW);
	vTmp = vBallCenter - gvCenter;

	angle = vTmp.RealAngle(vOrigin);

	//TRACE("RealAngle: %f\n", angle);

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

	// which brick did we hit?
	//
	bHit = false;

	for (i = 0; i < nMaxHits; i++) {
		nBrickIndex = nUse[i];

		assert(nBrickIndex >= 0 && nBrickIndex < N_BRICKS);

		if (m_bBrickVisible[nBrickIndex]) {

			//TRACE("Checking %d\n", nBrickIndex);

			//  if ball's rectange intersects this brick's rectangle
			//
			cRect.SetRect(ptBrickPos[nBrickIndex].x, ptBrickPos[nBrickIndex].y,  ptBrickPos[nBrickIndex].x + ptBrickSize[nBrickIndex].cx,  ptBrickPos[nBrickIndex].y + ptBrickSize[nBrickIndex].cy);
			if (rTmpRect.IntersectRect(rBall, cRect)) {

				// calculate the 21 points for this brick
				//
				for (j = 0; j < N_BRICK_POINTS; j++) {

					vPoints[j] = vBrickCritPoints[nBrickIndex / BRICKS_PER_ROW][j];

					vPoints[j].Rotate(((2 * PI) / BRICKS_PER_ROW) * (nBrickIndex % BRICKS_PER_ROW));
					vPoints[j] += gvCenter;

					if (distanceBetweenPoints(vBallCenter, vPoints[j]) < 11.0) {
						bHit = true;
					}
				}

				if (bHit) {

					if (pGameParams->bSoundEffectsEnabled) {

						sndPlaySound(m_pBrickSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					if ((pDC = GetDC()) != nullptr) {
						m_pBall->EraseSprite(pDC);
						EraseBrick(pDC, nBrickIndex);
						m_pBall->PaintSprite(pDC, m_pBall->GetPosition());
						ReleaseDC(pDC);
					}

					// on less brick
					--m_nBricks;

					// Score: 1 point for each brick regardless of color
					m_lScore += 1;

					// did user earn an extra ball?
					//
					if (m_lScore >= m_lExtraLifeScore) {

						if (pGameParams->bSoundEffectsEnabled) {

							sndPlaySound(m_pExtraLifeSound, SND_MEMORY | SND_SYNC | SND_NODEFAULT);
						}

						// double the ammount the user needs for their next extra life
						m_lExtraLifeScore += m_lExtraLifeScore;

						// extra ball
						m_nBalls++;
					}

					// if no bricks left
					//
					if (m_nBricks == 0) {

						GamePause();

						// reset turbo
						m_fTurboBoost = 0.0;

						if (pGameParams->bSoundEffectsEnabled) {
							#if CSOUND
							pEffect = new CSound((CWnd *)this, WAV_WINWAVE,
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
						//
						Common::sprintf_s(buf1, "Round complete.");
						Common::sprintf_s(buf2, "Score:  %ld", m_lScore);
						CMessageBox dlgYouWin((CWnd *)this, m_pGamePalette, buf1, buf2);

						// stop all sounds
						//
						if (pGameParams->bSoundEffectsEnabled) {
							#if CSOUND
							#else
							sndPlaySound(nullptr, SND_ASYNC);
							#endif
						}

						m_nNumRows++;
						if (m_nNumRows > N_ROWS) {
							m_nNumRows = 1;
							m_nBallSpeed++;
							//if (m_nBallSpeed > SPEED_MAX)
							//    m_nBallSpeed = SPEED_MAX;
						}

						// get new brick count
						m_nBricks = m_nNumRows * BRICKS_PER_ROW;

						// if user is playing the metagame
						//
						if (pGameParams->bPlayingMetagame) {

							// return to the metagame
							pGameParams->lScore = m_lScore;
							PostMessage(WM_CLOSE, 0, 0);

						} else {

							// reset the ball position
							//
							EndPaddle();
							EndBall();
							StartBricks();
							StartBall();
							StartPaddle();

							GameResume();
						}

						//
						// there are more bricks left,
						// so just calc a new vector for the ball
						//
					} else {

						bStillHit = true;
						while (bStillHit) {

							// roll ball back to point of contact with brick
							m_ptBallLocation -= m_vBallVector;

							// get new center of ball
							vBallCenter = m_ptBallLocation + BALL_RADIUS;

							bStillHit = false;
							for (j = 0; j < N_BRICK_POINTS; j++) {

								if ((fLen[j] = distanceBetweenPoints(vBallCenter, vPoints[j])) < 11.0) {
									bStillHit = true;
									break;
								}
							}
						}

						// find the 2 closest points to the center of the ball
						//
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

						// make sure we actually found an intersect point
						assert((nIndex != -1) && (nLastIndex != -1));

						// if ball hit a corner, then use next best point to
						// determine which side the ball actually hit
						//
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

						// if hit back face of brick
						//
						if (nIndex >= 1 && nIndex <= 8) {

							vBrick = vPoints[nIndex] - gvCenter;

							// if hit inner face of brick
							//
						} else if (nIndex >= 12 && nIndex <= 18) {

							vBrick = gvCenter - vPoints[nIndex];

							// hit clockwise side of brick
							//
						} else if (nIndex == 10) {

							vBrick = aBrickVectors[nBrickIndex % BRICKS_PER_ROW].v2;

							// hit counter-clockwise side of brick
							//
						} else if (nIndex == 20) {
							vBrick = aBrickVectors[nBrickIndex % BRICKS_PER_ROW].v1;

							// invalid index
							//
						} else {
							warning("Invalid Index (%d)", nIndex);
							assert(0);
						}

						// determine the vector of the brick (using ptHit as the intersect point)
						//
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


void CFugeWindow::EraseBrick(CDC *pDC, int nBrickIndex) {
	assert(pDC != nullptr);

	assert((nBrickIndex >= 0) && (nBrickIndex < N_BRICKS));

	// remove this brick from the screen
	//
	pDC->FloodFill(ptBrickPos[nBrickIndex].x + ptBrickSize[nBrickIndex].cx / 2, ptBrickPos[nBrickIndex].y + ptBrickSize[nBrickIndex].cy / 2, RGB(255, 255, 255));

	m_bBrickVisible[nBrickIndex] = false;
}


void CFugeWindow::LoseBall() {
	char buf1[32], buf2[32];
	CDC *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// pause the game
	GamePause();

	// reset turbo
	m_fTurboBoost = 0.0;

	if ((pDC = GetDC()) != nullptr) {

		m_pBall->EraseSprite(pDC);
		m_pBall->UnlinkSprite();

		assert(m_nBalls > 0);

		// on less ball
		m_nBalls--;

		if (pGameParams->bSoundEffectsEnabled) {
			#if CSOUND
			pEffect = new CSound((CWnd *)this, WAV_LOSEBALL,
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
			#else
			sndPlaySound(WAV_LOSEBALL, SND_ASYNC);
			#endif
		}
		/* BLACK HOLE ANIMATION REMOVED DUE TO LACK OF ARTWORK
		        CSprite *pSprite;
		        int i;

		        // start the black hole animation
		        //
		        if ((pSprite = new CSprite) != nullptr) {

		            if (pSprite->SharePalette(m_pGamePalette) != false) {

		                if (pSprite->LoadCels(pDC, ".\\ART\\LOSEBALL.BMP", N_BLACKHOLE_CELS) != false) {

		                    pSprite->SetMasked(true);
		                    pSprite->SetMobile(true);
		                    pSprite->LinkSprite();

		                    for (i = 0; i < N_BLACKHOLE_CELS * 2; i++) {
		                        pSprite->PaintSprite(pDC, 296, 216);
		                        Sleep(300);
		                    }

		                    pSprite->EraseSprite(pDC);
		                    pSprite->UnlinkSprite();
		                    delete pSprite;

		                } else {
		                    errCode = ERR_UNKNOWN;
		                }

		            } else {
		                errCode = ERR_UNKNOWN;
		            }

		        } else {
		            errCode = ERR_MEMORY;
		        }
		*/
		// if no more balls left - user has lost
		//
		if (m_nBalls == 0) {

			if (pGameParams->bSoundEffectsEnabled) {
				#if CSOUND
				pEffect = new CSound((CWnd *)this, WAV_GAMEOVER,
				                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
				#else
				sndPlaySound(WAV_GAMEOVER, SND_ASYNC);
				#endif
			}

			Common::sprintf_s(buf1, "Score:  %ld", m_lScore);
			CMessageBox dlgLoseBall((CWnd *)this, m_pGamePalette, "Game over.", buf1);

			if (pGameParams->bPlayingMetagame) {

				// return the final score
				//
				pGameParams->lScore = m_lScore;
				PostMessage(WM_CLOSE, 0, 0);
			}
			GameReset();

		} else {

			// display score, and start a new ball
			//

			Common::sprintf_s(buf1, "Score:  %ld", m_lScore);
			Common::sprintf_s(buf2, "Balls Left:  %d", m_nBalls);
			CMessageBox dlgLoseBall((CWnd *)this, m_pGamePalette, buf1, buf2);

			//
			// reset the ball position
			//
			EndPaddle();
			StartPaddle();
			StartBall();
		}

		ReleaseDC(pDC);

	} else {
		errCode = ERR_MEMORY;
	}

	HandleError(errCode);
}


void CFugeWindow::StartPaddle() {
	assert(m_pPaddle != nullptr);

	if (m_pPaddle != nullptr)
		m_pPaddle->LinkSprite();

	m_bBallOnPaddle = true;

	PaintPaddle(true);
}


void CFugeWindow::EndPaddle() {
	CDC *pDC;

	assert(m_pPaddle != nullptr);

	if ((m_pPaddle != nullptr) && (m_pPaddle->IsLinked())) {
		if ((pDC = GetDC()) != nullptr) {
			m_pPaddle->EraseSprite(pDC);
			m_pPaddle->UnlinkSprite();
			ReleaseDC(pDC);
		}
	}
}

void CFugeWindow::LaunchBall() {
	assert(m_bGameActive);
	assert(m_bBallOnPaddle);

	m_bPause = false;
	m_bBallOnPaddle = false;

	// starting ball vector is determined by the location of the paddle
	m_vBallVector = gvCenter - (m_ptBallLocation + BALL_RADIUS);

	// add a slight randomness to the balls vector
	//
	m_vBallVector.Rotate(Deg2Rad((brand() % 2) * ((brand() & 1) ? -1 : 1)));
	m_vBallVector.Unitize();

	PaintBall();
}


void CFugeWindow::PaintPaddle(bool bPaint) {
	CVector vPaddle;
	CDC *pDC;
	int nOldIndex;
	bool bSuccess;

	// verify that the input was not tainted
	assert(m_nPaddleCelIndex < N_PADDLE_CELS * 2);
	assert(m_nPaddleCelIndex > -N_PADDLE_CELS);

	// can't access a null pointer
	assert(m_pPaddle != nullptr);

	// get old cel index
	nOldIndex = m_pPaddle->GetCelIndex();

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

		m_pPaddle->SetCel(m_nPaddleCelIndex - 1);

		// paint paddle to new location
		//
		if ((pDC = GetDC()) != nullptr) {
			bSuccess = m_pPaddle->PaintSprite(pDC, PADDLE_START_X, PADDLE_START_Y);
			assert(bSuccess);

			// if the ball is resting on the paddle
			//
			if (m_bBallOnPaddle) {

				assert(m_pBall != nullptr);

				// ball is rotating with paddle
				//
				m_ptBallLocation = BallOnPaddle();

				// paint the ball to it's new location
				//
				m_pBall->PaintSprite(pDC, (int)m_ptBallLocation.x, (int)m_ptBallLocation.y);

			} else {

				#if 0

				// if the paddle's rectangle hit the ball's rectangle
				//
				if (m_pPaddle->InterceptOccurred()) {

					// did the paddle actually hit the ball?
					//
					if (m_pPaddle->TestInterception(pDC, m_pBall)) {

						double angle;

						TRACE("Paddle Hit Ball\n");

						//
						// handle collision with ball
						//
						vPaddle.SetVector(-13, 31);

						// cel index determines paddle angle
						//
						angle = (2 * PI / N_PADDLE_CELS) * m_nPaddleCelIndex;

						// if paddle is moving clockwise then rotate -90 instead of 90 degrees
						//
						angle += ((nOldIndex < m_nPaddleCelIndex) ? (PI / 2) : (-PI / 2));
						vPaddle.Rotate(angle);
						vPaddle.Unitize();
						m_vBallVector.Unitize();

						// handle spin
						//
						m_vBallVector += vPaddle * 2;
					}
				}
				#endif
			}

			ReleaseDC(pDC);
		}
	}
}


void CFugeWindow::StartBricks() {
	CDC *pDC;
	int i, nBricks;

	nBricks = m_nNumRows * BRICKS_PER_ROW;

	for (i = 0; i < nBricks; i++) {

		if (!m_bBrickVisible[i]) {
			m_bBrickVisible[i] = true;
		}
	}

	if ((pDC = GetDC()) != nullptr) {
		PaintBricks(pDC);
		ReleaseDC(pDC);
	}
}


void CFugeWindow::PaintBricks(CDC *pDC) {
	CSize     size;
	CDC      *pMemDC;
	CBitmap  *pBmp, *pMemBmp;
	CPalette *pPalOld, *pScreenPalOld;
	HBITMAP   hOldBitmap;
	int i;

	assert(pDC != nullptr);
	assert((m_nNumRows >= 0) && (m_nNumRows <= N_ROWS));

	if (FileExists(pszFugeArt[m_nNumRows])) {

		pBmp = FetchBitmap(pDC, nullptr, pszFugeArt[m_nNumRows]);

		pMemDC = nullptr;
		pMemBmp = nullptr;
		if ((pMemDC = new CDC) != nullptr) {

			if ((pMemBmp = new CBitmap) != nullptr) {

				size = GetBitmapSize(pBmp);
				if ((m_nNumRows != 0) && (pMemBmp->CreateCompatibleBitmap(pDC, size.cx, size.cy) != false)) {

					pMemDC->CreateCompatibleDC(nullptr);
					pPalOld = pMemDC->SelectPalette(m_pGamePalette, false);
					pMemDC->RealizePalette();
					hOldBitmap = SelectBitmap(pMemDC->m_hDC, pMemBmp->m_hObject);

					PaintBitmap(pMemDC, m_pGamePalette, pBmp);

					// repaint all the visible bricks
					//
					for (i = 0; i < N_BRICKS; i++) {

						if (!m_bBrickVisible[i]) {
							EraseBrick(pMemDC, i);
						}
					}

					pScreenPalOld = pDC->SelectPalette(m_pGamePalette, false);
					pDC->BitBlt(0, 0, size.cx, size.cy, pMemDC, 0, 0, SRCCOPY);
					pDC->SelectPalette(pScreenPalOld, false);

					SelectBitmap(pMemDC->m_hDC, hOldBitmap);
					pMemDC->SelectPalette(pPalOld, false);

					// fall back to yucky method
					//
				} else {

					PaintBitmap(pDC, m_pGamePalette, pBmp);

					if (m_nNumRows != 0) {

						// repaint all the visible bricks
						//
						for (i = 0; i < N_BRICKS; i++) {

							if (!m_bBrickVisible[i]) {
								EraseBrick(pDC, i);
							}
						}
					}
				}
			}
		}

		if (pMemBmp != nullptr) {
			delete pMemBmp;
			pMemBmp = nullptr;
		}
		if (pMemDC != nullptr) {
			delete pMemDC;
			pMemDC = nullptr;
		}
		if (pBmp != nullptr) {
			delete pBmp;
			pBmp = nullptr;
		}
	}
	if (m_pScrollButton != nullptr) {
		m_pScrollButton->Invalidate(false);
		m_pScrollButton->UpdateWindow();
	}
}


void CFugeWindow::EndBricks() {
	memset(m_bBrickVisible, 0, sizeof(bool) * N_BRICKS);
}



void CFugeWindow::LoadIniSettings() {
	if (pGameParams->bPlayingMetagame) {

		m_bOutterWall = false;
		m_nInitNumBalls = 1;
		m_nInitStartLevel = 3;
		m_nGForceFactor = GFORCE_DEF;
		m_nInitPaddleSize = PSIZE_MAX;

		switch (pGameParams->nSkillLevel) {

		case SKILLLEVEL_LOW:
			m_nInitBallSpeed = 4;
			break;

		case SKILLLEVEL_MEDIUM:
			m_nInitBallSpeed = 6;
			break;

		default:
			assert(pGameParams->nSkillLevel == SKILLLEVEL_HIGH);
			m_nInitBallSpeed = 8;
			break;
		}

	} else {

		m_nInitNumBalls = GetPrivateProfileInt(INI_SECTION, "NumberOfBalls", BALLS_DEF, INI_FILENAME);
		if ((m_nInitNumBalls < BALLS_MIN) || (m_nInitNumBalls > BALLS_MAX))
			m_nInitNumBalls = BALLS_DEF;

		m_nInitStartLevel = GetPrivateProfileInt(INI_SECTION, "StartingLevel", LEVEL_DEF, INI_FILENAME);
		if ((m_nInitStartLevel < LEVEL_MIN) || (m_nInitStartLevel > LEVEL_MAX))
			m_nInitStartLevel = LEVEL_DEF;

		m_nInitBallSpeed = GetPrivateProfileInt(INI_SECTION, "BallSpeed", SPEED_DEF, INI_FILENAME);
		if ((m_nInitBallSpeed < SPEED_MIN) || (m_nInitBallSpeed > SPEED_MAX))
			m_nInitBallSpeed = SPEED_DEF;

		m_nInitPaddleSize = GetPrivateProfileInt(INI_SECTION, "PaddleSize", PSIZE_DEF, INI_FILENAME);
		if ((m_nInitPaddleSize < PSIZE_MIN) || (m_nInitPaddleSize > PSIZE_MAX))
			m_nInitPaddleSize = PSIZE_DEF;

		m_bOutterWall = GetPrivateProfileInt(INI_SECTION, "OutterWall", 0, INI_FILENAME);
		if (m_bOutterWall != 0)
			m_bOutterWall = true;

		m_nGForceFactor = GetPrivateProfileInt(INI_SECTION, "Gravity", GFORCE_DEF, INI_FILENAME);
		if ((m_nGForceFactor < GFORCE_MIN) || (m_nGForceFactor > GFORCE_MAX))
			m_nGForceFactor = GFORCE_DEF;
	}
}


void CFugeWindow::GameReset() {
	KillTimer(TIMER_ID);                        // stop the timer

	if (pGameParams->bSoundEffectsEnabled) {
		#if CSOUND
		#else
		sndPlaySound(nullptr, SND_ASYNC);          // stop all sounds
		#endif
	}

	EndBricks();                                // remove all bricks from sprite chain
	EndBall();                                  // remove ball from sprite chain
	EndPaddle();                                // remove paddle from sprite chain

	m_fTurboBoost = 0.0;                        // no turbo

	m_lExtraLifeScore = EXTRA_LIFE_SCORE;       // user needs this many points for an extra life

	m_bGameActive = false;                      // there is no currently active game

	m_bPause = false;                           // the game is not paused

	m_bBallOnPaddle = false;                    // Ball is not yet on paddle

	m_bMovingPaddle = false;                    // user is not moving the paddle

	m_nBalls = m_nInitNumBalls;                 // reset # of balls

	m_nBallSpeed = m_nInitBallSpeed;            // reset ball speed

	m_nNumRows = m_nInitStartLevel;             // reset number of brick rows

	m_nBricks = m_nNumRows * BRICKS_PER_ROW;    // get new brick count

	m_lScore = 0;                               // reset the score

	m_bPaddleHit = false;                       // paddle starts fresh
}


void CFugeWindow::OnTimer(uintptr nEvent) {
	// there should be only one timer
	assert(nEvent == TIMER_ID);

	//
	// continue as long as there is a currently active non-paused game
	//
	if (m_bGameActive) {

		// check for joystick movement
		//
		if (m_bJoyActive) {
			JOYINFO joyInfo;
			joyGetPos(JOYSTICKID1, &joyInfo);
			OnJoyStick(joyInfo.wButtons, ((long)joyInfo.wYpos << 16) | joyInfo.wXpos);
		}

		if (!m_bPause) {
			PaintBall();
			PaintBall();
		}
	}
}

long CFugeWindow::OnJoyStick(unsigned int wParam, long lParam) {
	long nThresholdX, nThresholdY;

	if (m_bGameActive) {

		if (wParam & JOY_BUTTON1) {
			if (m_bBallOnPaddle) {
				LaunchBall();
			}
		}

		nThresholdX = m_nJoyOrgX;
		nThresholdX -= (unsigned int)LOWORD(lParam);

		nThresholdY = m_nJoyOrgY;
		nThresholdY -= (unsigned int)HIWORD(lParam);

		if (nThresholdY > 5000) {

			// up and left
			if (nThresholdX > 5000) {
				m_nPaddleCelIndex = 25;

				// up and right
			} else if (nThresholdX < -5000) {
				m_nPaddleCelIndex = 2;

				// just up
			} else {
				m_nPaddleCelIndex = 29;
			}

		} else if (nThresholdY < -5000) {

			// down and left
			if (nThresholdX > 5000) {
				m_nPaddleCelIndex = 17;

				// down and right
			} else if (nThresholdX < -5000) {
				m_nPaddleCelIndex = 9;

				// just down
			} else {
				m_nPaddleCelIndex = 13;
			}

		} else {

			// left
			if (nThresholdX > 5000) {
				m_nPaddleCelIndex = 21;

				// right
			} else if (nThresholdX < -5000) {
				m_nPaddleCelIndex = 5;
			}
		}
		PaintPaddle(false);
	}

	return 0;
}


void CFugeWindow::OnMouseMove(unsigned int, CPoint point) {
	int nMove;

	if (m_bGameActive && m_bMovingPaddle) {

		// Need to implement the mouse better for paddle movement
		//
		GetCursorPos(&point);

		if (point.x > m_ptOrigin.x) {

			nMove = (point.x - m_ptOrigin.x) / MOUSE_SENS + 1;

			m_nPaddleCelIndex += nMove;
			PaintPaddle(false);
			SetCursorPos(m_ptOrigin.x, m_ptOrigin.y);

		} else if (point.x < m_ptOrigin.x) {

			nMove = -((m_ptOrigin.x - point.x) / MOUSE_SENS + 1);

			m_nPaddleCelIndex += nMove;
			PaintPaddle(false);
			SetCursorPos(m_ptOrigin.x, m_ptOrigin.y);
		}

		// hide the cursor
		SetCursor(nullptr);

	} else {
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}
}



void CFugeWindow::OnRButtonUp(unsigned int nFlags, CPoint point) {
	if (m_bGameActive) {

		// toggle move paddle mode
		m_bMovingPaddle = !m_bMovingPaddle;

		if (m_bMovingPaddle) {

			SetCursor(nullptr);
			SetCursorPos(m_ptOrigin.x, m_ptOrigin.y);
			GameResume();

		} else {
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			GamePause();
		}

	} else {

		CFrameWnd::OnRButtonDown(nFlags, point);
	}
}


void CFugeWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CRect   boothRect,
	        tentRect,
	        peopRect,
	        car1Rect,
	        car2Rect,
	        car3Rect,
	        car4Rect,
	        car5Rect,
	        car6Rect,
	        car7Rect,
	        car8Rect,
	        car9Rect,
	        car10Rect;
	int     nPick = 0;

	boothRect.SetRect(BOOTH_X, BOOTH_Y, BOOTH_X + BOOTH_DX, BOOTH_Y + BOOTH_DY);
	tentRect.SetRect(TENT_X, TENT_Y, TENT_X + TENT_DX, TENT_Y + TENT_DY);
	peopRect.SetRect(PEOPLE_X, PEOPLE_Y, PEOPLE_X + PEOPLE_DX, PEOPLE_Y + PEOPLE_DY);
	car1Rect.SetRect(CAR1_X, CAR1_Y, CAR1_X + CAR_DX, CAR1_Y + CAR_DY);
	car2Rect.SetRect(CAR2_X, CAR2_Y, CAR2_X + CAR_DX, CAR2_Y + CAR_DY);
	car3Rect.SetRect(CAR3_X, CAR3_Y, CAR3_X + CAR_DX, CAR3_Y + CAR_DY);
	car4Rect.SetRect(CAR4_X, CAR4_Y, CAR4_X + CAR_DX, CAR4_Y + CAR_DY);
	car5Rect.SetRect(CAR5_X, CAR5_Y, CAR5_X + CAR_DX, CAR5_Y + CAR_DY);
	car6Rect.SetRect(CAR6_X, CAR6_Y, CAR6_X + CAR_DX, CAR6_Y + CAR_DY);
	car7Rect.SetRect(CAR7_X, CAR7_Y, CAR7_X + CAR_DX, CAR7_Y + CAR_DY);
	car8Rect.SetRect(CAR8_X, CAR8_Y, CAR8_X + CAR_DX, CAR8_Y + CAR_DY);
	car9Rect.SetRect(CAR9_X, CAR9_Y, CAR9_X + CAR9_DX, CAR9_Y + CAR9_DY);
	car10Rect.SetRect(CAR10_X, CAR10_Y, CAR10_X + CAR10_DX, CAR10_Y + CAR10_DY);

	// User clicked on the Title - NewGame button
	//
	if (m_rNewGameButton.PtInRect(point)) {

		// if we are not playing from the metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			// start a new game
			PlayGame();
		}

	} else if (boothRect.PtInRect(point)) {

		if (pGameParams->bSoundEffectsEnabled) {
			#if CSOUND
			pEffect = new CSound((CWnd *)this, WAV_BOOTH,
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
			#else
			sndPlaySound(WAV_BOOTH, SND_ASYNC);
			#endif
		}
	} else if (tentRect.PtInRect(point)) {

		if (pGameParams->bSoundEffectsEnabled) {
			#if CSOUND
			pEffect = new CSound((CWnd *)this, WAV_TENT,
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
			#else
			sndPlaySound(WAV_TENT, SND_ASYNC);
			#endif
		}

	} else if (peopRect.PtInRect(point)) {

		if (pGameParams->bSoundEffectsEnabled) {
			nPick = brand() % NUM_WAVS;
			if (nPick == 0) {
				#if CSOUND
				pEffect = new CSound((CWnd *)this, WAV_PEOPLE1,
				                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
				#else
				sndPlaySound(WAV_PEOPLE1, SND_ASYNC);
				#endif
			} else {
				#if CSOUND
				pEffect = new CSound((CWnd *)this, WAV_PEOPLE2,
				                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);  //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
				#else
				sndPlaySound(WAV_PEOPLE2, SND_ASYNC);
				#endif
			}
		}

	} else if (((((((((car1Rect.PtInRect(point) || car2Rect.PtInRect(point)) || car3Rect.PtInRect(point)) ||
	                 car4Rect.PtInRect(point)) || car5Rect.PtInRect(point)) || car6Rect.PtInRect(point)) ||
	              car7Rect.PtInRect(point)) || car8Rect.PtInRect(point)) || car9Rect.PtInRect(point)) ||
	           car10Rect.PtInRect(point)) {

		if (pGameParams->bSoundEffectsEnabled) {

			nPick = brand() % NUM_WAVS;

			#if CSOUND
			// Wave file, to delete itself play the narration
			//
			pEffect = new CSound((CWnd *)this, ((nPick == 0) ? WAV_CAR1 : WAV_CAR2), SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);
			pEffect->play();
			#else
			sndPlaySound(((nPick == 0) ? WAV_CAR1 : WAV_CAR2), SND_ASYNC);
			#endif
		}
	} else {

		if (m_bGameActive) {
			if (m_bBallOnPaddle) {
				LaunchBall();
			}
		} else {

			// is this needed ?
			CFrameWnd::OnLButtonDown(nFlags, point);
		}
	}
}


void CFugeWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_Q
	//
	if ((nChar == 'q') && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CFrameWnd ::OnSysChar(nChar, nRepCnt, nFlags);
	}
}


void CFugeWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	switch (nChar) {

	// User has hit ALT_F4 so close down this App
	//
	case VK_F4:
		PostMessage(WM_CLOSE, 0, 0);
		break;

	default:
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}


void CFugeWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// Handle keyboard input
	//
	switch (nChar) {

	// Move paddle clockwise
	//
	case VK_UP:
	case VK_RIGHT:

		if (m_bGameActive) {
			m_nPaddleCelIndex += PADDLE_CEL_JUMP;
			PaintPaddle(false);
		}
		break;

	// Move paddle counter-clockwise
	//
	case VK_DOWN:
	case VK_LEFT:

		if (m_bGameActive) {
			m_nPaddleCelIndex -= PADDLE_CEL_JUMP;
			PaintPaddle(false);
		}
		break;

	case VK_RETURN:
	case VK_SPACE:
		if (m_bGameActive) {
			if (m_bBallOnPaddle) {
				LaunchBall();
			}
		}
		break;

	//
	// Bring up the Rules
	//
	case VK_F1: {
		GamePause();
		CSound::waitWaveSounds();
		CRules  RulesDlg(this, "fuge.txt", m_pGamePalette, (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr));
		RulesDlg.DoModal();
		GameResume();
	}
	break;

	//
	// Bring up the options menu
	//
	case VK_F2:
		SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		break;

	case VK_SCROLL:
		if (m_bGameActive) {
			if (!m_bPause) {
				GamePause();
			} else {
				GameResume();
			}
		}
		break;

	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}


void CFugeWindow::OnActivate(unsigned int nState, CWnd *, bool bMinimized) {
	if (!bMinimized) {

		switch (nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			InvalidateRect(nullptr, false);
			break;

		default:
			break;
		}
	}
}


LRESULT CFugeWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);

	return 0;
}


LRESULT CFugeWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);

	return 0;
}


void CFugeWindow::OnSoundNotify(CSound *) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}


void CFugeWindow::OnClose() {
	CBrush myBrush;
	CRect myRect;
	CDC *pDC;

	ClipCursor(nullptr);

	// perform cleanup
	//
	GameReset();

	ReleaseMasterSounds();

	ReleaseMasterSprites();                     // release all master sprites

	// Stop the sountrack
	//
	if (m_pSoundTrack != nullptr) {

		// we should have been playing music
		assert(pGameParams->bMusicEnabled);
		delete m_pSoundTrack;
		m_pSoundTrack = nullptr;
	}

	#if CSOUND
	CSound::clearSounds();
	#endif

	//
	// de-allocate the main menu scroll button
	//
	assert(m_pScrollButton != nullptr);
	if (m_pScrollButton != nullptr) {
		delete m_pScrollButton;
		m_pScrollButton = nullptr;
	}

	//
	// need to de-allocate the game palette
	//
	assert(m_pGamePalette != nullptr);
	if (m_pGamePalette != nullptr) {
		m_pGamePalette->DeleteObject();
		delete m_pGamePalette;
		m_pGamePalette = nullptr;
	}

	if ((pDC = GetDC()) != nullptr) {              // paint black

		myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
		myBrush.CreateStockObject(BLACK_BRUSH);
		pDC->FillRect(&myRect, &myBrush);
		ReleaseDC(pDC);
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}

//
// CFugeWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CFugeWindow, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CFugeWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CFugeWindow::OnMMIONotify)
END_MESSAGE_MAP()



void CALLBACK GetGameParams(CWnd *pParentWnd) {
	//
	// Our user preference dialog box is self contained in this object
	//
	CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);
}

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel
