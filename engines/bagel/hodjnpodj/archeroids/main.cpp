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
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/llist.h"
#include "bagel/hodjnpodj/archeroids/main.h"
#include "bagel/hodjnpodj/archeroids/usercfg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Archeroids {

#define ARROWS  1

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP  ".\\ART\\FIELD.BMP"
const char *INI_SECTION = "Archeroids";

//
// Button ID constants
//
#define IDC_MENU                100
#define IDC_LIVES               101
#define IDC_SCORE               102
#define IDC_WAVE                103

//
// Bitmap IDs: Single Cel
//
#define IDB_HAY                 103
//
// Multi-Cel
//
#define IDB_BADWALK             107
#define IDB_BADSHOOT            108
#define IDB_HODJSHOOT           109
#define IDB_HODJWALK            110
#define IDB_BAD_ARROWS          111
#define IDB_GOOD_ARROWS         112
#define IDB_HAYBURNING          113
#define IDB_BADDIE              114
#define IDB_HODJDIE             115
#define IDB_HEART               116

#define N_BADWALK_CELLS         4
#define N_BADSHOOT_CELLS        3
#define N_HODJSHOOT_CELLS       3
#define N_HODJWALK_CELLS        4
#define N_BAD_ARROWS_CELLS      3
#define N_GOOD_ARROWS_CELLS     3
#define N_HAYBURNING_CELLS      3
#define N_BADDIE_CELLS          6
#define N_HODJDIE_CELLS         6

#define TIMER_ID                10

#define ARROW_SPEED              24
#define ARROW_MOVES_PER_CYCLE     2
#define MAX_GOOD_ARROWS           3
#define MAX_BAD_ARROWS            5
#define BAD_ARROW_LENGTH         29
#define BAD_ARROW_WIDTH           9
#define GOOD_ARROW_LENGTH        29
#define GOOD_ARROW_WIDTH          9
#define GOODGUY_START_X          23
#define GOODGUY_START_Y         100
#define HEART_START_X           (GAME_LEFT_BORDER_WIDTH + 2)
#define HEART_START_Y           372
#define HEART_SPACING             4
#define SPEED_FACTOR             25
#define N_HAY                     4
#define N_SECTIONS_PER_HAY       50
#define N_WAVES                  10
#define DEFAULT_ARROW_SPEED      50
#define BADGUYMOVE_Y             16
#define BADGUYMOVE_X             23
#define GOODGUYMOVE_Y             8
#define BADGUYSIZE_X             59
#define BADGUYSIZE_Y             48
#define END_GAME_AXIS            80
#define HAY_AXIS                140
#define LEVEL1                  (GAME_WIDTH-(GAME_RIGHT_BORDER_WIDTH+BADGUYSIZE_X))
#define LEVEL2                  (LEVEL1-(BADGUYSIZE_X+BADGUYMOVE_X))
#define LEVEL3                  (LEVEL2-(BADGUYSIZE_X+BADGUYMOVE_X))
#define LEVEL4                  (LEVEL3-(BADGUYSIZE_X+BADGUYMOVE_X))
#define LEVEL5                  (LEVEL4-(BADGUYSIZE_X+BADGUYMOVE_X))
#define LEVEL6                  (LEVEL5-(BADGUYSIZE_X+BADGUYMOVE_X))
#define LEVEL7                  (LEVEL6-(BADGUYSIZE_X+BADGUYMOVE_X))
#define LEVEL8                  (LEVEL7-(BADGUYSIZE_X+BADGUYMOVE_X))
#define ROW1                    (GAME_TOP_BORDER_WIDTH+BADGUYMOVE_Y)
#define ROW2                    (ROW1+(BADGUYSIZE_Y+BADGUYMOVE_Y))
#define ROW3                    (ROW2+(BADGUYSIZE_Y+BADGUYMOVE_Y))
#define ROW4                    (ROW3+(BADGUYSIZE_Y+BADGUYMOVE_Y))
#define ROW5                    (ROW4+(BADGUYSIZE_Y+BADGUYMOVE_Y))
#define ROW6                    (ROW5+(BADGUYSIZE_Y+BADGUYMOVE_Y))
#define ROW7                    (ROW6+(BADGUYSIZE_Y+BADGUYMOVE_Y))
#define ROW8                    (ROW7+(BADGUYSIZE_Y+BADGUYMOVE_Y))

// Game Sounds
//
#define WAV_DEATH               ".\\SOUND\\DEATH.WAV"
#define WAV_GAMEOVER            ".\\SOUND\\GAMEOVER.WAV"
#define WAV_WINWAVE             ".\\SOUND\\WINWAVE.WAV"
#define WAV_NARRATION           ".\\SOUND\\ARCH.WAV"
#define MID_SOUNDTRACK          ".\\SOUND\\ARCH.MID"

// Local prototypes
//
void CALLBACK GetGameParams(CWnd *);

//
// Globals
//
CPalette *pGamePalette;
LPGAMESTRUCT pGameParams;

extern HWND ghParentWnd;

STATIC int aBales[N_HAY];

STATIC const POINT aHayPosition[N_HAY][N_SECTIONS_PER_HAY] = {
	{	{100,  60}, {108,  60}, {116,  60}, {124,  60}, {132,  60},
		{100,  66}, {108,  66}, {116,  66}, {124,  66}, {132,  66},
		{100,  72}, {108,  72}, {116,  72}, {124,  72}, {132,  72},
		{100,  78}, {108,  78}, {116,  78}, {124,  78}, {132,  78},
		{100,  84}, {108,  84}, {116,  84}, {124,  84}, {132,  84},
		{100,  90}, {108,  90}, {116,  90}, {124,  90}, {132,  90},
		{100,  96}, {108,  96}, {116,  96}, {124,  96}, {132,  96},
		{100, 102}, {108, 102}, {116, 102}, {124, 102}, {132, 102},
		{100, 108}, {108, 108}, {116, 108}, {124, 108}, {132, 108},
		{100, 114}, {108, 114}, {116, 114}, {124, 114}, {132, 114}
	},

	{	{100, 160}, {108, 160}, {116, 160}, {124, 160}, {132, 160},
		{100, 166}, {108, 166}, {116, 166}, {124, 166}, {132, 166},
		{100, 172}, {108, 172}, {116, 172}, {124, 172}, {132, 172},
		{100, 178}, {108, 178}, {116, 178}, {124, 178}, {132, 178},
		{100, 184}, {108, 184}, {116, 184}, {124, 184}, {132, 184},
		{100, 190}, {108, 190}, {116, 190}, {124, 190}, {132, 190},
		{100, 196}, {108, 196}, {116, 196}, {124, 196}, {132, 196},
		{100, 202}, {108, 202}, {116, 202}, {124, 202}, {132, 202},
		{100, 208}, {108, 208}, {116, 208}, {124, 208}, {132, 208},
		{100, 214}, {108, 214}, {116, 214}, {124, 214}, {132, 214}
	},

	{	{100, 260}, {108, 260}, {116, 260}, {124, 260}, {132, 260},
		{100, 266}, {108, 266}, {116, 266}, {124, 266}, {132, 266},
		{100, 272}, {108, 272}, {116, 272}, {124, 272}, {132, 272},
		{100, 278}, {108, 278}, {116, 278}, {124, 278}, {132, 278},
		{100, 284}, {108, 284}, {116, 284}, {124, 284}, {132, 284},
		{100, 290}, {108, 290}, {116, 290}, {124, 290}, {132, 290},
		{100, 296}, {108, 296}, {116, 296}, {124, 296}, {132, 296},
		{100, 302}, {108, 302}, {116, 302}, {124, 302}, {132, 302},
		{100, 308}, {108, 308}, {116, 308}, {124, 308}, {132, 308},
		{100, 314}, {108, 314}, {116, 314}, {124, 314}, {132, 314}
	},

	{	{100, 360}, {108, 360}, {116, 360}, {124, 360}, {132, 360},
		{100, 366}, {108, 366}, {116, 366}, {124, 366}, {132, 366},
		{100, 372}, {108, 372}, {116, 372}, {124, 372}, {132, 372},
		{100, 378}, {108, 378}, {116, 378}, {124, 378}, {132, 378},
		{100, 384}, {108, 384}, {116, 384}, {124, 384}, {132, 384},
		{100, 390}, {108, 390}, {116, 390}, {124, 390}, {132, 390},
		{100, 396}, {108, 396}, {116, 396}, {124, 396}, {132, 396},
		{100, 402}, {108, 402}, {116, 402}, {124, 402}, {132, 402},
		{100, 408}, {108, 408}, {116, 408}, {124, 408}, {132, 408},
		{100, 414}, {108, 414}, {116, 414}, {124, 414}, {132, 414}
	}
};

STATIC POINT aHayPosUse[N_HAY][N_SECTIONS_PER_HAY];

//
// Position data for bad guys for each wave
//
STATIC const POINT aBadGuyPosition[N_WAVES][BADGUYS_MAX] = {

	{	{LEVEL4, ROW3}, {LEVEL4, ROW4}, {LEVEL4, ROW5}, {LEVEL4, ROW6},     // Wave 1
		{LEVEL3, ROW3}, {LEVEL3, ROW4}, {LEVEL3, ROW5}, {LEVEL3, ROW6},
		{LEVEL2, ROW3}, {LEVEL2, ROW4}, {LEVEL2, ROW5}, {LEVEL2, ROW6},
		{LEVEL1, ROW3}, {LEVEL1, ROW4}, {LEVEL1, ROW5}, {LEVEL1, ROW6}
	},

	{	{LEVEL4, ROW2}, {LEVEL4, ROW4}, {LEVEL4, ROW5}, {LEVEL4, ROW7},     // Wave 2
		{LEVEL3, ROW2}, {LEVEL3, ROW4}, {LEVEL3, ROW5}, {LEVEL3, ROW7},
		{LEVEL2, ROW2}, {LEVEL2, ROW4}, {LEVEL2, ROW5}, {LEVEL2, ROW7},
		{LEVEL1, ROW2}, {LEVEL1, ROW4}, {LEVEL1, ROW5}, {LEVEL1, ROW7}
	},

	{	{LEVEL4, ROW2}, {LEVEL4, ROW3}, {LEVEL4, ROW6}, {LEVEL4, ROW7},     // Wave 3
		{LEVEL3, ROW3}, {LEVEL3, ROW4}, {LEVEL3, ROW5}, {LEVEL3, ROW6},
		{LEVEL2, ROW2}, {LEVEL2, ROW3}, {LEVEL2, ROW4}, {LEVEL2, ROW5},
		{LEVEL2, ROW6}, {LEVEL2, ROW7}, {LEVEL1, ROW2}, {LEVEL1, ROW7}
	},

	{	{LEVEL4, ROW4}, {LEVEL4, ROW5}, {LEVEL4, ROW3}, {LEVEL4, ROW6},     // Wave 4
		{LEVEL3, ROW5}, {LEVEL3, ROW4}, {LEVEL3, ROW2}, {LEVEL3, ROW3},
		{LEVEL3, ROW6}, {LEVEL3, ROW7}, {LEVEL2, ROW2}, {LEVEL2, ROW3},
		{LEVEL2, ROW6}, {LEVEL2, ROW7}, {LEVEL1, ROW2}, {LEVEL1, ROW7}
	},

	{	{LEVEL4, ROW1}, {LEVEL4, ROW2}, {LEVEL4, ROW3}, {LEVEL4, ROW5},     // Wave 5
		{LEVEL4, ROW6}, {LEVEL4, ROW7}, {LEVEL3, ROW1}, {LEVEL3, ROW3},
		{LEVEL3, ROW5}, {LEVEL3, ROW7}, {LEVEL2, ROW1}, {LEVEL2, ROW2},
		{LEVEL2, ROW3}, {LEVEL2, ROW5}, {LEVEL2, ROW6}, {LEVEL2, ROW7}
	},

	{	{LEVEL4, ROW3}, {LEVEL4, ROW4}, {LEVEL4, ROW6}, {LEVEL4, ROW7},     // Wave 6
		{LEVEL3, ROW3}, {LEVEL3, ROW4}, {LEVEL3, ROW6}, {LEVEL3, ROW7},
		{LEVEL2, ROW2}, {LEVEL2, ROW3}, {LEVEL2, ROW5}, {LEVEL2, ROW6},
		{LEVEL1, ROW2}, {LEVEL1, ROW3}, {LEVEL1, ROW5}, {LEVEL1, ROW6}
	},

	{	{LEVEL4, ROW2}, {LEVEL4, ROW4}, {LEVEL4, ROW5}, {LEVEL4, ROW7},     // Wave 7
		{LEVEL3, ROW2}, {LEVEL3, ROW3}, {LEVEL3, ROW6}, {LEVEL3, ROW7},
		{LEVEL2, ROW2}, {LEVEL2, ROW3}, {LEVEL2, ROW6}, {LEVEL2, ROW7},
		{LEVEL1, ROW2}, {LEVEL1, ROW4}, {LEVEL1, ROW5}, {LEVEL1, ROW7}
	},

	{	{LEVEL4, ROW2}, {LEVEL4, ROW3}, {LEVEL4, ROW4}, {LEVEL4, ROW5},     // Wave 8
		{LEVEL4, ROW6}, {LEVEL3, ROW1}, {LEVEL3, ROW3}, {LEVEL3, ROW5},
		{LEVEL3, ROW7}, {LEVEL2, ROW2}, {LEVEL2, ROW4}, {LEVEL2, ROW6},
		{LEVEL1, ROW1}, {LEVEL1, ROW3}, {LEVEL1, ROW5}, {LEVEL1, ROW7}
	},

	{	{LEVEL4, ROW2}, {LEVEL4, ROW3}, {LEVEL4, ROW4}, {LEVEL4, ROW5},     // Wave 9
		{LEVEL4, ROW6}, {LEVEL3, ROW2}, {LEVEL3, ROW6}, {LEVEL2, ROW2},
		{LEVEL2, ROW6}, {LEVEL1, ROW1}, {LEVEL1, ROW2}, {LEVEL1, ROW3},
		{LEVEL1, ROW4}, {LEVEL1, ROW5}, {LEVEL1, ROW6}, {LEVEL1, ROW7}
	},

	{	{LEVEL4, ROW1}, {LEVEL4, ROW3}, {LEVEL4, ROW5}, {LEVEL4, ROW7},     // Wave 10
		{LEVEL3, ROW1}, {LEVEL3, ROW3}, {LEVEL3, ROW5}, {LEVEL3, ROW7},
		{LEVEL2, ROW1}, {LEVEL2, ROW3}, {LEVEL2, ROW5}, {LEVEL2, ROW7},
		{LEVEL1, ROW1}, {LEVEL1, ROW3}, {LEVEL1, ROW5}, {LEVEL1, ROW7}
	}
};


CMainWindow::CMainWindow() {
	CString     WndClass;
	CRect       tmpRect;
	CBitmap    *pSplashScreen;
	CDC        *pDC;
	ERROR_CODE  errCode;
	bool        bSuccess;

	// assume no error
	errCode = ERR_NONE;

	// Initialize data
	//
	m_pHayList = m_pBadGuyList = nullptr;
	m_pBadArrowList = m_pGoodArrowList = nullptr;
	m_pScrollSprite = nullptr;
	m_pGamePalette = nullptr;
	m_pFXList = nullptr;
	m_pHodj = nullptr;
	m_bTimerActive = false;
	m_bGameActive = false;
	m_bJoyActive = false;
	m_bMoveMode = false;
	m_bInMenu = false;
	m_pMasterBadArrow = nullptr;
	m_pMasterGoodArrow = nullptr;
	m_pMasterBurn = nullptr;
	m_pMasterBadWalk = nullptr;
	m_pMasterBadShoot = nullptr;
	m_pMasterBadDie = nullptr;
	m_pMasterGoodWalk = nullptr;
	m_pMasterGoodShoot = nullptr;
	m_pMasterGoodDie = nullptr;
	m_pMasterHeart = nullptr;

	m_pSoundTrack = nullptr;
	m_pBadDieSound = nullptr;
	m_pBoltSound = nullptr;
	m_pArrowSound = nullptr;
	m_pBurnSound = nullptr;
	m_hBadDieRes = nullptr;
	m_hBoltRes = nullptr;
	m_hArrowRes = nullptr;
	m_hBurnRes = nullptr;

	// make sure score is initially zero
	pGameParams->lScore = 0;

	// no animations if playing on a 386
	m_bAnimationsOn = !(GetWinFlags() & WF_CPU386);

	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	// Acquire the shared palette for our game from the splash screen art
	//
	if (FileExists(MINI_GAME_MAP)) {

		if ((pDC = GetDC()) != nullptr) {
			pSplashScreen = FetchBitmap(pDC, &m_pGamePalette, MINI_GAME_MAP);
			bSuccess = CSprite::SetBackdrop(pDC, m_pGamePalette, pSplashScreen);
			assert(bSuccess);
			pGamePalette = m_pGamePalette;
			ReleaseDC(pDC);
		} else {
			errCode = ERR_MEMORY;
		}

	} else {
		errCode = ERR_FFIND;
	}

	// Center our window on the screen
	//
	tmpRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	#ifndef DEBUG
	if ((pDC = GetDC()) != nullptr) {
		tmpRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) / 2;
		tmpRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) / 2;
		tmpRect.right = tmpRect.left + GAME_WIDTH;
		tmpRect.bottom = tmpRect.top + GAME_HEIGHT;
		ReleaseDC(pDC);
	}
	#endif

	// get mouse anchor point
	//
	m_ptAnchor.x = GAME_WIDTH / 2 + tmpRect.left;
	m_ptAnchor.y = GAME_HEIGHT / 2 + tmpRect.top;

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	Create(WndClass, "Boffo Games -- Archeroids", WS_POPUP, tmpRect, nullptr, 0);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	PaintScreen();
	EndWaitCursor();

	// limit the mouse cursor to the bounds of this game
	ClipCursor(&tmpRect);

	if (errCode == ERR_NONE) {

		//
		// build our main menu button
		//
		if ((m_pScrollSprite = new CSprite) != nullptr) {

			m_pScrollSprite->SharePalette(m_pGamePalette);

			if ((pDC = GetDC()) != nullptr) {

				bSuccess = m_pScrollSprite->LoadResourceSprite(pDC, IDB_SCROLBTN);
				if (!bSuccess)
					errCode = ERR_UNKNOWN;

				ReleaseDC(pDC);
			} else {
				errCode = ERR_MEMORY;
			}
			m_pScrollSprite->SetMasked(true);
			m_pScrollSprite->SetMobile(true);
		} else {
			errCode = ERR_MEMORY;
		}
	}

	// only continue if there was no error
	//
	if (errCode == ERR_NONE) {

		BeginWaitCursor();

		// Seed the random number generator
		//srand((unsigned int)time(nullptr));

		errCode = LoadMasterSprites();

		if (!errCode)
			errCode = LoadMasterSounds();

		InitializeJoystick();

		EndWaitCursor();

		if (pGameParams->bMusicEnabled) {
			m_pSoundTrack = new CSound(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
			assert(m_pSoundTrack != nullptr);
			m_pSoundTrack->midiLoopPlaySegment(6400, 37680, 0, FMT_MILLISEC);    //6320
		} // end if m_pSoundTrack


		// if we are not playing from the metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			// Automatically bring up the main menu
			//
			PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		}
	}

	HandleError(errCode);
}


void CMainWindow::InitializeJoystick() {
	JOYINFO     joyInfo;

	if (joySetCapture(m_hWnd, JOYSTICKID1, 10000, true) == JOYERR_NOERROR) {
		//
		// Calibrate the joystick
		//
		joySetThreshold(JOYSTICKID1, 5000);
		joyGetPos(JOYSTICKID1, &joyInfo);
		m_nJoyLast = joyInfo.wYpos;
		m_bJoyActive = true;

	} else {
		//CMessageBox dlgNoJoystick((CWnd *)this, m_pGamePalette, "Warning!  No Joystick", "Driver Installed");
	}
}


ERROR_CODE CMainWindow::LoadMasterSprites() {
	CDC *pDC;
	ERROR_CODE errCode;

	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		if ((m_pMasterGoodDie = new CSprite) != nullptr) {

			if (m_pMasterGoodDie->SharePalette(m_pGamePalette) != false) {

				if (m_pMasterGoodDie->LoadResourceCels(pDC, IDB_HODJDIE, N_HODJDIE_CELLS)) {

					m_pMasterGoodDie->SetTypeCode(200);
					m_pMasterGoodDie->SetMasked(true);
					m_pMasterGoodDie->SetMobile(true);
					m_pMasterGoodDie->SetAnimated(true);

				} else {
					errCode = ERR_UNKNOWN;
				}

			} else {
				errCode = ERR_UNKNOWN;
			}

		} else {
			errCode = ERR_MEMORY;
		}
		#if 0
		if (errCode == ERR_NONE) {

			if ((m_pMasterGoodShoot = new CSprite) != nullptr) {

				if (m_pMasterGoodShoot->SharePalette(m_pGamePalette) != false) {

					if (m_pMasterGoodShoot->LoadResourceCels(pDC, IDB_HODJSHOOT, N_HODJSHOOT_CELLS) != false) {

						m_pMasterGoodShoot->SetTypeCode(200);
						m_pMasterGoodShoot->SetMasked(true);
						m_pMasterGoodShoot->SetMobile(true);
						m_pMasterGoodShoot->SetAnimated(true);

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
		#endif
		if (errCode == ERR_NONE) {
			if ((m_pMasterGoodWalk = new CSprite) != nullptr) {

				if (m_pMasterGoodWalk->SharePalette(m_pGamePalette) != false) {

					if (m_pMasterGoodWalk->LoadResourceCels(pDC, IDB_HODJWALK, N_HODJWALK_CELLS) != false) {

						m_pMasterGoodWalk->SetTypeCode(200);
						m_pMasterGoodWalk->SetMasked(true);
						m_pMasterGoodWalk->SetMobile(true);
						m_pMasterGoodWalk->SetAnimated(true);

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


		if (errCode == ERR_NONE) {

			if ((m_pMasterBadDie = new CSprite) != nullptr) {

				if (m_pMasterBadDie->SharePalette(m_pGamePalette) != false) {

					if (m_pMasterBadDie->LoadResourceCels(pDC, IDB_BADDIE, N_BADDIE_CELLS) != false) {

						m_pMasterBadDie->SetTypeCode(false);
						m_pMasterBadDie->SetMasked(true);
						m_pMasterBadDie->SetMobile(true);
						m_pMasterBadDie->SetAnimated(true);

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
		#if 0
		if (errCode == ERR_NONE) {

			if ((m_pMasterBadShoot = new CSprite) != nullptr) {

				if (m_pMasterBadShoot->SharePalette(m_pGamePalette) != false) {

					if (m_pMasterBadShoot->LoadResourceCels(pDC, IDB_BADSHOOT, N_BADSHOOT_CELLS) != false) {

						m_pMasterBadShoot->SetTypeCode(false);
						m_pMasterBadShoot->SetMasked(true);
						m_pMasterBadShoot->SetMobile(true);
						m_pMasterBadShoot->SetAnimated(true);

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
		#endif
		if ((m_pMasterBadWalk = new CSprite) != nullptr) {

			if (m_pMasterBadWalk->SharePalette(m_pGamePalette) != false) {

				if (m_pMasterBadWalk->LoadResourceCels(pDC, IDB_BADWALK, N_BADWALK_CELLS) != false) {

					m_pMasterBadWalk->SetTypeCode(false);
					m_pMasterBadWalk->SetMasked(true);
					m_pMasterBadWalk->SetMobile(true);
					m_pMasterBadWalk->SetAnimated(true);

				} else {
					errCode = ERR_UNKNOWN;
				}

			} else {
				errCode = ERR_UNKNOWN;
			}

		} else {
			errCode = ERR_MEMORY;
		}


		// Create the master burning hay
		//
		if ((m_pMasterBurn = new CSprite) != nullptr) {

			// attach arrow to the Game Palette
			//
			if (m_pMasterBurn->SharePalette(m_pGamePalette) != false) {

				if (m_pMasterBurn->LoadResourceCels(pDC, IDB_HAYBURNING, N_HAYBURNING_CELLS) != false) {

					m_pMasterBurn->SetTypeCode(300);
					m_pMasterBurn->SetMasked(true);
					m_pMasterBurn->SetMobile(true);
					m_pMasterBurn->SetAnimated(true);

				} else {
					errCode = ERR_UNKNOWN;
				}

			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_MEMORY;
		}

		if (errCode == ERR_NONE) {

			// Create the master bad arrow
			//
			if ((m_pMasterBadArrow = new CSprite) != nullptr) {

				// attach arrow to the Game Palette
				//
				if (m_pMasterBadArrow->SharePalette(m_pGamePalette) != false) {

					//
					// load this arrow's bitmap into the sprite
					//
					if (m_pMasterBadArrow->LoadResourceCels(pDC, IDB_BAD_ARROWS, N_BAD_ARROWS_CELLS) != false) {

						m_pMasterBadArrow->SetTypeCode(218);
						m_pMasterBadArrow->SetMasked(true);
						m_pMasterBadArrow->SetMobile(true);
						m_pMasterBadArrow->SetAnimated(true);

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

		if (errCode == ERR_NONE) {

			// Create the master good arrow
			//
			if ((m_pMasterGoodArrow = new CSprite) != nullptr) {

				// attach arrow to the Game Palette
				//
				if (m_pMasterGoodArrow->SharePalette(m_pGamePalette) != false) {

					//
					// load this arrow's bitmap into the sprite
					//
					if (m_pMasterGoodArrow->LoadResourceCels(pDC, IDB_GOOD_ARROWS, N_GOOD_ARROWS_CELLS) != false) {

						m_pMasterGoodArrow->SetTypeCode(217);
						m_pMasterGoodArrow->SetMasked(true);
						m_pMasterGoodArrow->SetMobile(true);
						m_pMasterGoodArrow->SetAnimated(true);

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

		if (errCode == ERR_NONE) {

			if ((m_pMasterHeart = new CSprite) != nullptr) {

				//
				// load picture of heart
				//
				if (m_pMasterHeart->LoadResourceSprite(pDC, IDB_HEART) != false) {

					// attach heart to the Game Palette
					//
					if (m_pMasterHeart->SharePalette(m_pGamePalette) != false) {

						m_pMasterHeart->SetTypeCode(301);
						m_pMasterHeart->SetMasked(true);
						m_pMasterHeart->SetMobile(true);

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

void CMainWindow::ReleaseMasterSprites() {
	//
	// free all master sprite objects
	//

	if (m_pMasterHeart != nullptr) {
		delete m_pMasterHeart;
		m_pMasterHeart = nullptr;
	}

	if (m_pMasterGoodArrow != nullptr) {
		delete m_pMasterGoodArrow;
		m_pMasterGoodArrow = nullptr;
	}

	if (m_pMasterBadArrow != nullptr) {
		delete m_pMasterBadArrow;
		m_pMasterBadArrow = nullptr;
	}

	if (m_pMasterBurn != nullptr) {
		delete m_pMasterBurn;
		m_pMasterBurn = nullptr;
	}

	if (m_pMasterBadWalk != nullptr) {
		delete m_pMasterBadWalk;
		m_pMasterBadWalk = nullptr;
	}

	if (m_pMasterBadShoot != nullptr) {
		delete m_pMasterBadShoot;
		m_pMasterBadShoot = nullptr;
	}

	if (m_pMasterBadDie != nullptr) {
		delete m_pMasterBadDie;
		m_pMasterBadDie = nullptr;
	}

	if (m_pMasterGoodWalk != nullptr) {
		delete m_pMasterGoodWalk;
		m_pMasterGoodWalk = nullptr;
	}

	if (m_pMasterGoodShoot != nullptr) {
		delete m_pMasterGoodShoot;
		m_pMasterGoodShoot = nullptr;
	}

	if (m_pMasterGoodDie != nullptr) {
		delete m_pMasterGoodDie;
		m_pMasterGoodDie = nullptr;
	}
}


ERROR_CODE CMainWindow::LoadMasterSounds() {
	HANDLE hResInfo;
	HINSTANCE hInst;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	hInst = (HINSTANCE)GetWindowWord(m_hWnd, GWW_HINSTANCE);

	// Load and lock
	//
	if ((hResInfo = FindResource(hInst, "BadDieSound", "WAVE")) != nullptr) {

		if ((m_hBadDieRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

			if ((m_pBadDieSound = (char *)LockResource((HGLOBAL)m_hBadDieRes)) != nullptr) {

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

		// Load and lock
		//
		if ((hResInfo = FindResource(hInst, "BoltSound", "WAVE")) != nullptr) {

			if ((m_hBoltRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

				if ((m_pBoltSound = (char *)LockResource((HGLOBAL)m_hBoltRes)) == nullptr)
					errCode = ERR_UNKNOWN;
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_UNKNOWN;
		}
	}

	if (errCode == ERR_NONE) {

		// Load and lock
		//
		if ((hResInfo = FindResource(hInst, "ArrowSound", "WAVE")) != nullptr) {

			if ((m_hArrowRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

				if ((m_pArrowSound = (char *)LockResource((HGLOBAL)m_hArrowRes)) == nullptr)
					errCode = ERR_UNKNOWN;
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_UNKNOWN;
		}
	}

	if (errCode == ERR_NONE) {

		// Load and lock the hay burn sound into memory
		//
		if ((hResInfo = FindResource(hInst, "BurnHay", "WAVE")) != nullptr) {

			if ((m_hBurnRes = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {

				if ((m_pBurnSound = (char *)LockResource((HGLOBAL)m_hBurnRes)) == nullptr)
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


void CMainWindow::ReleaseMasterSounds() {
	if (m_hExtraLifeRes != nullptr) {
		FreeResource(m_hExtraLifeRes);
		m_hExtraLifeRes = nullptr;
	}
	if (m_hBurnRes != nullptr) {
		FreeResource(m_hBurnRes);
		m_hBurnRes = nullptr;
	}
	if (m_hArrowRes != nullptr) {
		FreeResource(m_hArrowRes);
		m_hArrowRes = nullptr;
	}
	if (m_hBoltRes != nullptr) {
		FreeResource(m_hBoltRes);
		m_hBoltRes = nullptr;
	}
	if (m_hBadDieRes != nullptr) {
		FreeResource(m_hBadDieRes);
		m_hBadDieRes = nullptr;
	}
}


void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	Invalidate(false);
	BeginPaint(&lpPaint);
	PaintScreen();
	EndPaint(&lpPaint);
}


void CMainWindow::PaintScreen() {
	CDC     *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		CSprite::RefreshBackdrop(pDC, m_pGamePalette);

		if (!m_bInMenu && (m_pScrollSprite != nullptr)) {
			m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
		}

		// update the on-screen sprites
		errCode = RepaintSpriteList(pDC);

		ReleaseDC(pDC);

	} else {
		errCode = ERR_MEMORY;
	}

	HandleError(errCode);
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
ERROR_CODE CMainWindow::RepaintSpriteList(CDC *pDC) {
	CSprite *pSprite;
	ERROR_CODE errCode;

	// can't use a null pointer
	assert(pDC != nullptr);

	// assume no error
	errCode = ERR_NONE;

	if (pDC == nullptr) {
		errCode = ERR_UNKNOWN;
	} else {

		//
		// Paint each sprite
		//
		pSprite = CSprite::GetSpriteChain();
		while (pSprite) {

			pSprite->ClearBackground();
			pSprite->RefreshSprite(pDC);

			pSprite = pSprite->GetNextSprite();
		}
	}
	return errCode;
}


bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CMainMenu COptionsWind((CWnd *)this, m_pGamePalette, (pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : (m_bGameActive ? 0 : NO_RETURN)), GetGameParams, "arch.txt", (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr), pGameParams);
	CDC *pDC;
	bool bSuccess;

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		//
		// must bring up our menu of controls
		//
		case IDC_MENU:

			// pause timer
			GamePause();
			m_bInMenu = true;

			pDC = GetDC();
			if (m_pScrollSprite != nullptr) {
				bSuccess = m_pScrollSprite->EraseSprite(pDC);
				ASSERT(bSuccess);
			}

			CSound::waitWaveSounds();

			switch (COptionsWind.DoModal()) {

			case IDC_OPTIONS_NEWGAME:
				PlayGame();
				break;

			case IDC_OPTIONS_QUIT:
				PostMessage(WM_CLOSE, 0, 0);
				break;

			default:
				break;
			}

			if (m_pScrollSprite != nullptr) {
				bSuccess = m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
				ASSERT(bSuccess);
			}
			ReleaseDC(pDC);

			m_bInMenu = false;

			if (!pGameParams->bMusicEnabled && (m_pSoundTrack != nullptr)) {

				m_pSoundTrack->stop();
				delete m_pSoundTrack;
				m_pSoundTrack = nullptr;

			} else if (pGameParams->bMusicEnabled && (m_pSoundTrack == nullptr)) {
				m_pSoundTrack = new CBofSound(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
				m_pSoundTrack->midiLoopPlaySegment(6400, 37680, 0, FMT_MILLISEC);
			}

			// resume timer
			//GameResume();

			return true;
		}
	}

	return false;
}

CSprite *CMainWindow::NewLife(int iLifeIndex) {
	CSize size;
	CDC *pDC;
	CSprite *pSprite;

	pSprite = nullptr;

	if ((pDC = GetDC()) != nullptr) {

		if ((pSprite = m_pMasterHeart->DuplicateSprite(pDC)) != nullptr) {

			pSprite->SetZOrder(SPRITE_HINDMOST);

			size = pSprite->GetSize();

			// add heart to sprite list
			pSprite->LinkSprite();

			// set intial heart position
			pSprite->PaintSprite(pDC, HEART_START_X, HEART_START_Y + (LIVES_MAX - (iLifeIndex + 1)) * (size.cy + HEART_SPACING));
		}
		ReleaseDC(pDC);
	}

	return pSprite;
}


ERROR_CODE CMainWindow::CreateLives() {
	int i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// create one heart for each live
	//
	for (i = 0; i < m_nInitNumLives; i++) {

		// create our hearts
		//
		if ((m_pLives[i] = NewLife(i)) == nullptr) {
			errCode = ERR_MEMORY;
			break;
		}
	}

	return errCode;
}


/*****************************************************************************
*
*  CreateGoodGuy    -
*
*  DESCRIPTION:     Longer description of this function
*
*
*  SAMPLE USAGE:
*  errCode = CreateGoodGuy();
*
*  RETURNS:  ERROR_CODE = error return code
*
*****************************************************************************/
ERROR_CODE CMainWindow::CreateGoodGuy() {
	CDC *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		// create our good guy sprite
		//
		if ((m_pHodj = m_pMasterGoodWalk->DuplicateSprite(pDC)) != nullptr) {

			m_pHodj->SetZOrder(SPRITE_FOREGROUND);
			m_pHodj->SetAnimated(true);

			// add good guy to sprite list
			m_pHodj->LinkSprite();

			// set intial good guy position
			m_pHodj->PaintSprite(pDC, GOODGUY_START_X, GOODGUY_START_Y);

		} else {
			errCode = ERR_MEMORY;
		}

		ReleaseDC(pDC);

	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}


/*****************************************************************************
*
*  CreateHay        -
*
*  DESCRIPTION:     Longer description of this function.  Continued onto next
*
*
*  SAMPLE USAGE:
*  errCode = CreateHay();
*
*  RETURNS:  ERROR_CODE = error return code.
*
*****************************************************************************/
ERROR_CODE CMainWindow::CreateHay() {
	CLList *pList;
	CSprite *pSprite;
	CDC *pDC;
	int i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	memcpy(aHayPosUse, aHayPosition, sizeof(POINT) * N_HAY * N_SECTIONS_PER_HAY);

	if ((pDC = GetDC()) == nullptr) {
		errCode = ERR_UNKNOWN;

	} else {

		for (i = 0; i < 4; i++) {

			if ((pSprite = new CSprite()) == nullptr) {
				errCode = ERR_MEMORY;
				break;

			} else {

				//
				// load the hay picture
				//
				if (pSprite->LoadResourceSprite(pDC, IDB_HAY) == false) {
					errCode = ERR_UNKNOWN;
					break;

				} else {

					if (pSprite->SharePalette(m_pGamePalette) == false) {
						errCode = ERR_UNKNOWN;
						break;

					} else {

						pSprite->SetTypeCode(i);
						pSprite->SetMasked(true);
						pSprite->SetMobile(true);
						pSprite->LinkSprite();                          // add hay to sprite list

						pSprite->PaintSprite(pDC, 100, (i * 100) + 60); // set intial hay positions

						aBales[i] = N_SECTIONS_PER_HAY;

						//
						// add this hay sprite to our private hay list
						//
						pList = new CLList(pSprite);

						if (m_pHayList != nullptr) {
							m_pHayList->Insert(pList);
						} else {
							m_pHayList = pList;
						}
					}
				}
			}
		}
		ReleaseDC(pDC);
	}

	return errCode;
}

ERROR_CODE CMainWindow::CreateBurningHay(CPoint point) {
	CLList *pList;
	CSprite *pSprite;
	CDC *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		if ((pSprite = m_pMasterBurn->DuplicateSprite(pDC)) != nullptr) {

			pSprite->SetZOrder(SPRITE_BACKGROUND);
			pSprite->SetAnimated(true);

			pSprite->LinkSprite();                  // add burning hay to sprite list

			pSprite->PaintSprite(pDC, point);       // set intial hay positions

			//
			// add this burning hay to our special FX list
			//
			if ((pList = new CLList(pSprite)) != nullptr) {

				if (m_pFXList != nullptr) {
					m_pFXList->Insert(pList);
				} else {
					m_pFXList = pList;
				}
			} else {
				errCode = ERR_MEMORY;
			}

		} else {
			errCode = ERR_MEMORY;
		}

		ReleaseDC(pDC);
	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}


/*****************************************************************************
*
*  CreateBadArrow   -
*
*  DESCRIPTION:     Longer description of this function.  Continued onto next
*
*
*  SAMPLE USAGE:
*  errCode = CreateBadArrow(point);
*  POINT point;                             starting position of new arrow
*
*  RETURNS:  ERROR_CODE = error return code
*
*****************************************************************************/
ERROR_CODE CMainWindow::CreateBadArrow(CSprite *pBadGuy) {
	CRect newRect, oldRect, overlappedRect;
	CPoint point;
	CSize size;
	CLList *pList;
	CSprite *pSprite;
	CDC *pDC;
	ERROR_CODE errCode;
	bool bHit;

	// assume no error
	errCode = ERR_NONE;

	if (ARROWS && (m_nBadArrows < MAX_BAD_ARROWS)) {

		point = pBadGuy->GetPosition();
		point = GetLeftMostBadGuy();
		point.y = pBadGuy->GetPosition().y;
		size = pBadGuy->GetSize();
		point.x -= BAD_ARROW_LENGTH;
		point.y += size.cy / 2 - BAD_ARROW_WIDTH / 2;
		point.y -= 8;

		newRect.SetRect(point.x, point.y, point.x + BAD_ARROW_LENGTH, point.y + BAD_ARROW_WIDTH);

		//
		// Make sure this arrow does not touch hay
		//
		bHit = false;
		pList = m_pHayList;
		while (pList != nullptr) {
			oldRect = ((CSprite *)pList->getData())->GetRect();

			// Check for intercection
			//
			if (overlappedRect.IntersectRect(oldRect, newRect)) {
				bHit = true;
				break;
			}
			pList = pList->getNext();
		}

		// Don't create this arrow if it interects another arrow or if it
		// intersects hay
		//
		if (!bHit) {

			if ((pDC = GetDC()) != nullptr) {

				// Animate this badguy to shoot
				//
				#if 0
				if ((pSprite = m_pMasterBadShoot->DuplicateSprite(pDC)) != nullptr) {
					int i;

					pSprite->LinkSprite();
					pSprite->PaintSprite(pDC, pBadGuy->GetPosition());
					pBadGuy->EraseSprite(pDC);

					for (i = 1; i < N_BADSHOOT_CELLS; i++) {
						pSprite->PaintSprite(pDC, pBadGuy->GetPosition());
					}

					pBadGuy->PaintSprite(pDC, pBadGuy->GetPosition());
					DeleteSprite(pSprite);
				}
				#endif
				// Create the new arrow
				//
				if ((pSprite = m_pMasterBadArrow->DuplicateSprite(pDC)) != nullptr) {

					pSprite->SetZOrder(SPRITE_TOPMOST);
					pSprite->SetAnimated(true);

					m_nBadArrows++;

					pSprite->LinkSprite();              // add arrow to sprite list

					pSprite->PaintSprite(pDC, point);   // set intial arrow position

					// Play the arrow shoot sound
					//
					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pBoltSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					//
					// add this arrow to the aBadArrowList
					//
					if ((pList = new CLList(pSprite)) != nullptr) {

						if (m_pBadArrowList != nullptr)
							m_pBadArrowList->addToTail(pList);
						else {
							m_pBadArrowList = pList;
						}

					} else {
						errCode = ERR_MEMORY;
					}

				} else {
					errCode = ERR_MEMORY;
				}

				ReleaseDC(pDC);

			} else {
				errCode = ERR_MEMORY;
			}
		}
	}

	HandleError(errCode);

	return errCode;
}


POINT CMainWindow::GetLeftMostBadGuy() {
	POINT ptTmp, ptBest;
	CLList *pList;

	assert(m_pBadGuyList != nullptr);

	ptBest.x = 9999;
	ptBest.y = 0;

	pList = m_pBadGuyList->getHead();
	while (pList != nullptr) {
		ptTmp = ((CSprite *)pList->getData())->GetPosition();

		if (ptTmp.x < ptBest.x)
			ptBest = ptTmp;

		pList = pList->getNext();
	}

	return ptBest;
}


/*****************************************************************************
*
*  CreateGoodArrow  -
*
*  DESCRIPTION:     Longer description of this function.  Continued onto next
*                   line like this.
*
*  SAMPLE USAGE:
*  errCode = CreateGoodArrow();
*
*  RETURNS:  ERROR_CODE = error return code
*
*****************************************************************************/
ERROR_CODE CMainWindow::CreateGoodArrow() {
	CRect newRect, oldRect, overlappedRect;
	CLList *pList;
	CDC *pDC;
	CSize size;
	ERROR_CODE errCode;
	CSprite *pSprite;
	POINT point;
	bool bHit;

	// assume no error
	errCode = ERR_NONE;

	GameResume();

	if (m_nGoodArrows < MAX_GOOD_ARROWS) {

		// Good guy must be valid to shoot an arrow
		assert(m_pHodj != nullptr);

		// Arrow is shot from end of crossbow
		//
		size = m_pHodj->GetSize();
		point = m_pHodj->GetPosition();
		point.y += 15;
		point.x += size.cx - 3;

		newRect.SetRect(point.x, point.y, point.x + GOOD_ARROW_LENGTH, point.y + GOOD_ARROW_WIDTH);

		//
		// Make sure this arrow does not touch another arrow
		//
		bHit = false;
		pList = m_pGoodArrowList;
		while (pList != nullptr) {
			oldRect = ((CSprite *)pList->getData())->GetRect();

			// Check for intercection
			//
			if (overlappedRect.IntersectRect(oldRect, newRect)) {
				bHit = true;
				break;
			}
			pList = pList->getNext();
		}

		//
		// Don't create the arrow if it intersects another good arrow
		//
		if (!bHit) {

			if ((pDC = GetDC()) == nullptr) {
				errCode = ERR_UNKNOWN;

			} else {
				#if 0
				if ((pSprite = m_pMasterGoodShoot->DuplicateSprite(pDC)) != nullptr) {

					int i;
					pSprite->LinkSprite();
					pSprite->PaintSprite(pDC, m_pHodj->GetPosition());
					m_pHodj->EraseSprite(pDC);

					for (i = 1; i < N_HODJSHOOT_CELLS; i++) {
						pSprite->PaintSprite(pDC, m_pHodj->GetPosition());
						Sleep(5);
					}

					m_pHodj->PaintSprite(pDC, m_pHodj->GetPosition());
					DeleteSprite(pSprite);
				}
				#endif

				// Create the new arrow
				//
				if ((pSprite = m_pMasterGoodArrow->DuplicateSprite(pDC)) != nullptr) {

					// set arrows to topmost
					pSprite->SetZOrder(SPRITE_TOPMOST);
					pSprite->SetAnimated(true);

					m_nGoodArrows++;

					pSprite->LinkSprite();

					pSprite->PaintSprite(pDC, point);

					if (pGameParams->bSoundEffectsEnabled) {
						sndPlaySound(m_pArrowSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
					}

					//
					// add this arrow to the aGoodArrowList
					//
					pList = new CLList(pSprite);
					if (m_pGoodArrowList != nullptr)
						m_pGoodArrowList->addToTail(pList);
					else {
						m_pGoodArrowList = pList;
					}

				} else {
					errCode = ERR_MEMORY;
				}

				ReleaseDC(pDC);
			}
		}
	}

	HandleError(errCode);

	return errCode;
}


/*****************************************************************************
*
*  CreateBadGuys    -
*
*  DESCRIPTION:     Longer description of this function.
*
*
*  SAMPLE USAGE:
*  errCode = CreateBadGuys();
*
*  RETURNS:  ERROR_CODE = error return code
*
*****************************************************************************/
ERROR_CODE CMainWindow::CreateBadGuys() {
	CLList *pList;
	CSprite *pSprite;
	CDC *pDC;
	POINT point;
	int i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		//
		// create a sprite for each of the on screen bad guys
		//
		for (i = 0; i < m_nBadGuys; i++) {

			if ((pSprite = m_pMasterBadWalk->DuplicateSprite(pDC)) != nullptr) {

				pSprite->SetZOrder(SPRITE_FOREGROUND);
				pSprite->SetAnimated(true);

				// add badguy to sprite list
				pSprite->LinkSprite();

				// if this fails it will trash global memory
				assert(m_nBadGuys <= BADGUYS_MAX);

				// Adjust for Archer Level
				//
				point = aBadGuyPosition[m_nWave % N_WAVES][i];
				point.x -= (m_nInitArcherLevel - 1) * BADGUYMOVE_X;

				// set intial position
				pSprite->PaintSprite(pDC, point);

				//
				// add this badguy sprite to our private badguy list
				//
				if ((pList = new CLList(pSprite)) != nullptr) {

					if (m_pBadGuyList != nullptr) {
						m_pBadGuyList->Insert(pList);
					} else {
						m_pBadGuyList = pList;
					}
				} else {
					errCode = ERR_MEMORY;
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


/*****************************************************************************
*
*  PlayGame         - one line discription
*
*  DESCRIPTION:     Longer description of this function.  Continued onto next
*
*
*  SAMPLE USAGE:
*  PlayGame();
*
*  RETURNS:  nothing
*
*****************************************************************************/
void CMainWindow::PlayGame() {
	ERROR_CODE errCode;

	errCode = ERR_NONE;                                 // assume no error

	if (!m_bTimerActive) {

		m_bTimerActive = true;
		CWnd::SetTimer(TIMER_ID, 50, nullptr);
	}

	m_bNewGame = true;                                  // a new game has been started

	LoadIniSettings();                                  // load game defaults

	GameReset();                                        // intilize game data


	if ((errCode = CreateGoodGuy()) == ERR_NONE) {      // create our good guy

		if ((errCode = CreateLives()) == ERR_NONE) {    // create hearts for lives

			if ((errCode = CreateHay()) == ERR_NONE) {  // create the hay bales

				if ((errCode = CreateBadGuys()) == ERR_NONE) {

					m_bGameActive = true;

					FlushInputEvents();

					// game starts paused
					m_bPause = true;
				}
			}
		}
	}

	HandleError(errCode);
}

void CMainWindow::PlayNextWave() {
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	WaveReset();

	if ((m_nWave % N_WAVES == 0) && (m_nGameSpeed > 0))
		m_nGameSpeed--;

	if ((errCode = CreateBadGuys()) == ERR_NONE) {

		if ((errCode = CreateHay()) == ERR_NONE) {

		}
	}

	HandleError(errCode);
}

void CMainWindow::WaveReset() {
	CDC *pDC;
	int i;

	m_nBadGuys = m_nInitNumBadGuys;                 // reset # badguys

	KillAnimation();                                // stop all animation sequences

	PruneDeadBadGuys();                             // remove all dead bad guys

	m_pHodj->UnlinkSprite();                        // unlink good guy so we don't flush him from sprite chain

	for (i = 0; i < LIVES_MAX; i++) {               // unlink hearts so we don't flush them
		if (m_pLives[i] != nullptr)
			m_pLives[i]->UnlinkSprite();
	}

	if ((pDC = GetDC()) != nullptr) {                  // erase any sprites from the screen
		CSprite::EraseSprites(pDC);
		ReleaseDC(pDC);
	}
	CSprite::FlushSpriteChain();                    // flush all sprites from the chain

	for (i = 0; i < LIVES_MAX; i++) {               // link the hearts back into the sprite list
		if (m_pLives[i] != nullptr)
			m_pLives[i]->LinkSprite();
	}
	m_pHodj->LinkSprite();                          // link good guy back into list

	m_nGoodArrows = m_nBadArrows = 0;               // reset # of arrows

	if (m_pHayList != nullptr) {                       // reset the hay list
		m_pHayList->FlushList();
		delete m_pHayList;
		m_pHayList = nullptr;
	}
	if (m_pBadGuyList != nullptr) {                    // reset the bad guy list
		m_pBadGuyList->FlushList();
		delete m_pBadGuyList;
		m_pBadGuyList = nullptr;
	}
	if (m_pBadArrowList != nullptr) {                  // reset the bad guy arrow list
		m_pBadArrowList->FlushList();
		delete m_pBadArrowList;
		m_pBadArrowList = nullptr;
	}
	if (m_pGoodArrowList != nullptr) {                 // reset the good guy arrow list
		m_pGoodArrowList->FlushList();
		delete m_pGoodArrowList;
		m_pGoodArrowList = nullptr;
	}

	m_nBadGuySpeed = m_nBadGuys * m_nGameSpeed * SPEED_FACTOR;// set default sprite speed

	m_nState = 0;                                   // set initial bad guy state

	m_bPause = true;                                // game starts paused
}

/*****************************************************************************
*
*  GameReset        - one line discription
*
*  DESCRIPTION:     Longer description of this function.  Continued onto next
*                   line like this.
*
*  SAMPLE USAGE:
*  GameReset();
*
*  RETURNS:  nothing
*
*****************************************************************************/
void CMainWindow::GameReset() {
	CDC *pDC;

	m_bGameActive = false;                          // no current active game

	m_bPause = false;                               // game starts paused

	m_nGoodArrows = m_nBadArrows = 0;               // reset # of arrows

	m_nMoveArrows = 0;                              // there are no arrows to move

	m_lScore = 0;                                   // reset score to zero

	m_lNewLifeScore = 400;                          // bonus life every 1000 points

	KillAnimation();                                // stop all animation sequences

	PruneDeadBadGuys();                             // remove all dead bad guys

	memset(m_pLives, 0, sizeof(CSprite *) * LIVES_MAX); // reset hearts

	if ((pDC = GetDC()) != nullptr) {                  // erase any sprites from the screen
		CSprite::EraseSprites(pDC);
		ReleaseDC(pDC);
	}
	CSprite::FlushSpriteChain();                    // clean up after any previous game

	if (m_pHayList != nullptr) {                       // reset the hay list
		m_pHayList->FlushList();
		delete m_pHayList;
		m_pHayList = nullptr;
	}
	if (m_pBadGuyList != nullptr) {                    // reset the bad guy list
		m_pBadGuyList->FlushList();
		delete m_pBadGuyList;
		m_pBadGuyList = nullptr;
	}
	if (m_pBadArrowList != nullptr) {                  // reset the bad guy arrow list
		m_pBadArrowList->FlushList();
		delete m_pBadArrowList;
		m_pBadArrowList = nullptr;
	}
	if (m_pGoodArrowList != nullptr) {                 // reset the good guy arrow list
		m_pGoodArrowList->FlushList();
		delete m_pGoodArrowList;
		m_pGoodArrowList = nullptr;
	}
	m_pHodj = nullptr;                                 // reset our good guy

	m_nBadGuys = m_nInitNumBadGuys;                 // reset # badguys

	m_nArrowSpeed = DEFAULT_ARROW_SPEED;            // set default sprite speeds
	m_nBadGuySpeed = m_nBadGuys * m_nGameSpeed * SPEED_FACTOR;// set default sprite speeds

	m_nState = 0;                                   // set initial bad guy state

	m_nWave = 0;                                    // reset to Wave 1
}

void CMainWindow::KillAnimation() {
	CLList *pList;

	// delete the Special FX list
	//
	while (m_pFXList != nullptr) {

		DeleteSprite((CSprite *)m_pFXList->getData());

		pList = m_pFXList;
		m_pFXList = m_pFXList->getNext();
		delete pList;
	}
}


void CMainWindow::LoadIniSettings() {
	int nVal;

	if (pGameParams->bPlayingMetagame) {

		m_nInitNumLives = 1;
		m_nInitNumBadGuys = DEFAULT_BADGUYS;

		switch (pGameParams->nSkillLevel) {

		case SKILLLEVEL_LOW:

			m_nInitArcherLevel = 1;
			m_nInitGameSpeed = 2;
			break;

		case SKILLLEVEL_MEDIUM:

			m_nInitArcherLevel = 3;
			m_nInitGameSpeed = 5;
			break;

		default:

			assert(pGameParams->nSkillLevel == SKILLLEVEL_HIGH);
			m_nInitArcherLevel = 3;
			m_nInitGameSpeed = 8;
			break;
		}

	} else {

		// Get the game speed (1..10)
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "GameSpeed", DEFAULT_GAME_SPEED, INI_FILENAME);
		m_nInitGameSpeed = nVal;
		if (nVal < SPEED_MIN || nVal > SPEED_MAX)
			m_nInitGameSpeed = DEFAULT_GAME_SPEED;

		// Get the Archer level (1..8)
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "ArcherLevel", DEFAULT_ARCHER_LEVEL, INI_FILENAME);
		m_nInitArcherLevel = nVal;
		if (nVal < LEVEL_MIN || nVal > LEVEL_MAX)
			m_nInitArcherLevel = DEFAULT_ARCHER_LEVEL;

		// Get initial number of lives
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "NumberOfLives", DEFAULT_LIVES, INI_FILENAME);
		m_nInitNumLives = nVal;
		if (nVal < LIVES_MIN || nVal > LIVES_MAX)
			m_nInitNumLives = DEFAULT_LIVES;

		// Get initial number of badguys
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "NumberOfBadGuys", DEFAULT_BADGUYS, INI_FILENAME);
		m_nInitNumBadGuys = nVal;
		if (nVal < BADGUYS_MIN || nVal > BADGUYS_MAX)
			m_nInitNumBadGuys = DEFAULT_BADGUYS;

		// Get overide for Animations On/Off
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "AnimationsOn", m_bAnimationsOn, INI_FILENAME);
		m_bAnimationsOn = (nVal == 0 ? false : true);
	}
	m_nLives = m_nInitNumLives;
	m_nGameSpeed = abs(SPEED_MAX - m_nInitGameSpeed);
}


void CMainWindow::OnTimer(uintptr nEventID) {
	KillTimer(nEventID);

	while (m_bTimerActive) {
		if (MainLoop())
			break;

		if (CheckMessages()) {
			break;
		}
	}
	PostMessage(WM_CLOSE, 0, 0);
}


bool CMainWindow::MainLoop() {
	CLList *pList, *pNext;
	CSprite *pSprite;
	CDC *pDC;
	CRect tmpRect, newRect;
	CSize size;
	unsigned long t1;

	// new game state is over.  We are now playing the game
	//
	m_bNewGame = false;

	if (m_bGameActive && !m_bPause) {

		if ((pDC = GetDC()) != nullptr) {

			t1 = GetTickCount();

			//
			// Handle special effects (Animation)
			//
			pList = m_pFXList;
			while (pList != nullptr) {
				pNext = pList->getNext();

				// get local pointer to this cell's sprite
				pSprite = (CSprite *)pList->getData();

				// paint this cell
				pSprite->PaintSprite(pDC, pSprite->GetPosition());

				// if animations are off or this is the last cell in animation,
				// then destroy this strip
				//
				if (!m_bAnimationsOn || (pSprite->GetCelIndex() == pSprite->GetCelCount() - 1)) {

					// if this is the head, then move the head
					//
					if (pList == m_pFXList)
						m_pFXList = pNext;

					delete pList;

					DeleteSprite(pSprite);
				}

				pList = pNext;
			}

			// Handle Arrow movement
			//
			MoveArrows(pDC);

			// Handle badguy movement.
			//
			if (MoveBadGuys(pDC))
				return true;

			if (!m_bNewGame) {
				while (m_nMoveArrows < ARROW_MOVES_PER_CYCLE)
					MoveArrows(pDC);
				m_nMoveArrows = 0;
			}

			ReleaseDC(pDC);

			// No governor for Cheetah on Steroids (just go fast as possible)
			//
			if (m_nBadGuySpeed > 0) {
				while (GetTickCount() < t1 + (20 + m_nBadGuys * 20)) {
					if (CheckMessages())
						return true;
				}
			}
		}
	}
	return false;
}


bool CMainWindow::MoveArrows(CDC *pDC) {
	char buf1[40], buf2[40];
	CLList *pList, *pNext, *pSearchList;
	CSprite *pSprite, *pTmpSprite;
	POINT point;
	CRect tmpRect, newRect;
	CSize size;
	bool bHit;

	assert(pDC != nullptr);

	// acknowledge that we have moved the arrows one more time
	m_nMoveArrows++;

	//
	// parse Bad Guy arrow list and move each one left
	//
	pList = m_pBadArrowList;
	while (pList != nullptr) {
		pSprite = (CSprite *)pList->getData();
		point = pSprite->GetPosition();
		point.x -= ARROW_SPEED;

		// save pointer to next arrow
		pNext = pList->getNext();

		if (point.x > (0 + GAME_LEFT_BORDER_WIDTH)) {
			pSprite->PaintSprite(pDC, point);

			if (point.x < HAY_AXIS) {

				if (pSprite->InterceptOccurred()) {

					bHit = false;

					pSearchList = m_pHayList;
					while (pSearchList != nullptr) {

						newRect = pSprite->GetRect();
						//newRect.right = newRect.left + 2;
						pTmpSprite = (CSprite *)pSearchList->getData();
						if (tmpRect.IntersectRect(newRect, pTmpSprite->GetRect())) {

							if (pSprite->TestInterception(pDC, pTmpSprite)) {

								DestroyBadArrow(pList);
								DestroyHay(pSearchList, tmpRect, pDC, true);
								bHit = true;
								break;
							}
						}

						pSearchList = pSearchList->getNext();
					}

					if (!bHit) {

						if (pSprite->TestInterception(pDC, m_pHodj)) {
							DestroyBadArrow(pList);
							LoseLife(pDC, true);
							return true;
						}
					}
				}
			}

		} else {
			DestroyBadArrow(pList);
		}

		// go to next in list
		pList = pNext;
	}

	//
	// parse Good Guy arrow list and move each one right
	//
	pList = m_pGoodArrowList;
	while (pList != nullptr) {
		pSprite = (CSprite *)pList->getData();
		size = pSprite->GetSize();

		// calc new arrow location
		//
		point = pSprite->GetPosition();
		point.x += ARROW_SPEED;

		// save pointer to next arrow
		pNext = pList->getNext();

		// If arrow did not leave the playing field
		//
		if (point.x < GAME_WIDTH - (size.cx + GAME_RIGHT_BORDER_WIDTH)) {
			pSprite->PaintSprite(pDC, point);

			// If arrow hit something, then check to see what it was
			//
			if (pSprite->InterceptOccurred()) {

				bHit = false;

				if (point.x < HAY_AXIS) {

					pSearchList = m_pHayList;
					while (pSearchList != nullptr) {

						newRect = pSprite->GetRect();
						//newRect.left = newRect.right - 2;
						pTmpSprite = (CSprite *)pSearchList->getData();
						if (tmpRect.IntersectRect(newRect, pTmpSprite->GetRect())) {

							if (pSprite->TestInterception(pDC, pTmpSprite)) {

								DestroyGoodArrow(pList);
								DestroyHay(pSearchList, tmpRect, pDC, true);
								bHit = true;
								break;
							}
						}

						pSearchList = pSearchList->getNext();
					}
				}

				if (!bHit) {
					//
					// check for hitting bad guy as a secondary hit
					//
					pSearchList = m_pBadGuyList;
					while (pSearchList != nullptr) {

						// if badguy is not already dead
						//
						pTmpSprite = (CSprite *)pSearchList->getData();
						if (pTmpSprite->GetTypeCode() == false) {

							if (pSprite->TestInterception(pDC, pTmpSprite)) {

								DestroyGoodArrow(pList);
								DestroyBadGuy(pSearchList, pDC);

								//
								// Are all the bad guys dead?
								//
								if (m_nBadGuys == 0) {
									pNext = nullptr;
									GamePause();
									++m_nWave;

									if (pGameParams->bSoundEffectsEnabled)
										sndPlaySound(WAV_WINWAVE, SND_SYNC);

									if (pGameParams->bPlayingMetagame) {
										Common::sprintf_s(buf1, "You have defeated");
										Common::sprintf_s(buf2, "all of the archers.");

									} else {
										Common::sprintf_s(buf1, "Wave %d completed.", m_nWave);
										Common::sprintf_s(buf2, "Score: %ld   Lives: %d", m_lScore, m_nLives);
									}

									FlushInputEvents();
									CMessageBox dlgWaveComplete((CWnd *)this, m_pGamePalette, buf1, buf2);

									if (pGameParams->bPlayingMetagame) {

										pGameParams->lScore = 1;
										PostMessage(WM_CLOSE, 0, 0);
									} else {
										PlayNextWave();
									}
									return true;
								}
								break;
							}
						}
						pSearchList = pSearchList->getNext();
					}
				}
			}

		} else {
			DestroyGoodArrow(pList);
		}

		// go to next in list
		pList = pNext;
	}
	return false;
}

bool CMainWindow::CheckMessages() {
	JOYINFO joyInfo;
	MSG msg;
	bool bEndTask = false;

	assert(m_bTimerActive);

	// check for joystick movement
	//
	if (m_bJoyActive) {
		joyGetPos(JOYSTICKID1, &joyInfo);
		OnJoyStick(joyInfo.wButtons, (long)joyInfo.wYpos << 16);
	}

	if (PeekMessage(&msg, nullptr, MM_MCINOTIFY, MM_MCINOTIFY, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

		if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
			m_bTimerActive = false;
			bEndTask = true;
		} else {

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return bEndTask;
}

bool CMainWindow::MoveBadGuys(CDC *pDC) {
	CRect tmpRect;
	POINT point;
	CLList *pList, *pSearchList, *pSearchNext;
	CSprite *pSprite;
	int i, n;

	// can't access a null pointer
	assert(pDC != nullptr);
	assert(m_bTimerActive);
	assert(m_bGameActive);

	if (!m_bPause && (m_nBadGuys > 0) && (m_pBadGuyList != nullptr) && !m_bNewGame) {

		PruneDeadBadGuys();

		//
		// There are 4 bad guy states
		// 0        Bad guys move up while in this state
		// 1 & 3    Bad guys move left 1 level when in this state
		// 2        Bad guys move down while in this state
		//
		n = (m_nBadGuys / ARROW_MOVES_PER_CYCLE);
		switch (m_nState) {

		//
		// move all bad guys up
		//
		case 0:

			//
			// parse Bad Guy list and move each one up
			//
			i = 0;
			pList = m_pBadGuyList;
			while (pList != nullptr) {
				pSprite = (CSprite *)pList->getData();

				assert(pSprite != nullptr);

				// as long as this badguy is not dead then move him
				//
				if (pSprite->GetTypeCode() == false) {

					point = pSprite->GetPosition();
					point.y -= BADGUYMOVE_Y - m_nGameSpeed;
					pSprite->PaintSprite(pDC, point);

					//
					//  if any are gonna hit the top border, then set
					//  to state 1 (time to move left 1 level)
					//
					if (point.y < (0 + GAME_TOP_BORDER_WIDTH + BADGUYMOVE_Y - m_nGameSpeed)) {
						if (m_nState == 0)
							m_nState = 1;
					}

					if (point.x < HAY_AXIS) {

						if (pSprite->InterceptOccurred()) {

							//
							// Test badguy vs hay
							//
							pSearchList = m_pHayList;
							while (pSearchList != nullptr) {
								pSearchNext = pSearchList->getNext();

								if (tmpRect.IntersectRect(pSprite->GetRect(), ((CSprite *)pSearchList->getData())->GetRect())) {

									point.x = tmpRect.left;
									point.y = tmpRect.top;
									DestroyHay(pSearchList, tmpRect, pDC, false);
									break;
								}
								pSearchList = pSearchNext;
							}
						}
					}

					if ((brand() & 0x001f) == 0) {
						CreateBadArrow(pSprite);
					}

					if (CheckMessages())
						return true;

					if (m_bNewGame)
						return false;

					if (n > 0) {
						if ((i % n) == n - 1) {
							if (MoveArrows(pDC)) {
								break;
							}
						}
					}
					i++;
				}

				pList = pList->getNext();
			}
			break;

		//
		// move all bad guys down
		//
		case 2:

			//
			// parse Bad Guy list and move each one down
			//
			i = 0;
			pList = m_pBadGuyList;
			while (pList != nullptr) {
				pSprite = (CSprite *)pList->getData();

				assert(pSprite != nullptr);

				// as long as this badguy is not dead then move him
				//
				if (pSprite->GetTypeCode() == false) {

					point = pSprite->GetPosition();
					point.y += BADGUYMOVE_Y - m_nGameSpeed;
					pSprite->PaintSprite(pDC, point);

					//
					//  if any hit are gonna hit the top border, then set
					//  to state 1 (time to move left 1 level)
					//
					if (point.y > GAME_HEIGHT - (GAME_BOTTOM_BORDER_WIDTH + BADGUYMOVE_Y + BADGUYSIZE_Y - m_nGameSpeed)) {
						if (m_nState == 2)
							m_nState = 3;
					}

					if (point.x < HAY_AXIS) {

						if (pSprite->InterceptOccurred()) {

							//
							// Test badguy vs hay
							//
							pSearchList = m_pHayList;
							while (pSearchList != nullptr) {
								pSearchNext = pSearchList->getNext();

								if (tmpRect.IntersectRect(pSprite->GetRect(), ((CSprite *)pSearchList->getData())->GetRect())) {

									point.x = tmpRect.left;
									point.y = tmpRect.top;
									DestroyHay(pSearchList, tmpRect, pDC, false);
									break;
								}
								pSearchList = pSearchNext;
							}
						}
					}

					// Crossbow shoots arrow from center of badguy
					//
					if ((brand() & 0x001f) == 0) {
						CreateBadArrow(pSprite);
					}

					if (CheckMessages())
						return true;

					if (m_bNewGame)
						return false;

					if (n > 0) {
						if ((i % n) == n - 1) {
							if (MoveArrows(pDC)) {
								break;
							}
						}
					}
					i++;
				}

				pList = pList->getNext();
			}
			break;

		//
		// move all bad guys left 1 level
		//
		case 1:
		case 3:

			// test speed
			//ErrorLog("SPEED.LOG", "%ld", GetTickCount()/1000);

			//
			// parse Bad Guy list and move each one left 1 level
			//
			i = 0;
			pList = m_pBadGuyList;
			while (pList != nullptr) {
				pSprite = (CSprite *)pList->getData();

				assert(pSprite != nullptr);

				// as long as this badguy is not dead then move him
				//
				if (pSprite->GetTypeCode() == false) {

					point = pSprite->GetPosition();
					point.x -= BADGUYMOVE_X;
					pSprite->PaintSprite(pDC, point);

					//
					// Test badguy vs good guy
					//
					if (point.x < END_GAME_AXIS) {

						LoseLife(pDC, false);
						break;

					} else if (point.x < HAY_AXIS) {

						if (pSprite->InterceptOccurred()) {

							//
							// Test badguy vs hay
							//
							pSearchList = m_pHayList;
							while (pSearchList != nullptr) {
								pSearchNext = pSearchList->getNext();

								if (tmpRect.IntersectRect(pSprite->GetRect(), ((CSprite *)pSearchList->getData())->GetRect())) {

									point.x = tmpRect.left;
									point.y = tmpRect.top;
									DestroyHay(pSearchList, tmpRect, pDC, false);
									break;
								}
								pSearchList = pSearchNext;
							}
						}
					}

					// Crossbow shoots arrow from center of badguy
					//
					if ((brand() & 0x001f) == 0) {
						CreateBadArrow(pSprite);
					}

					if (CheckMessages())
						return true;

					if (m_bNewGame)
						return false;

					if (n > 0) {
						if ((i % n) == n - 1) {
							if (MoveArrows(pDC)) {
								break;
							}
						}
					}
					i++;
				}

				pList = pList->getNext();
			}

			// goto next state
			//
			m_nState++;
			m_nState %= 4;
			break;

		default:
			assert(0);
		}
		PruneDeadBadGuys();
	}
	return false;
}


void CMainWindow::LoseLife(CDC *pDC, bool bAnimate) {
	char szTmpBuf[40];
	CSprite *pSprite;
	//CSound *pSound;
	CLList *pList, *pNext;
	POINT point;
	int i;

	// validate the device context
	assert(pDC != nullptr);

	GamePause();                                    // stop the timer while we process stuff

	--m_nLives;                                     // one less life

	DeleteSprite(m_pLives[m_nLives]);               // remove 1 heart
	m_pLives[m_nLives] = nullptr;

	// play the death sound
	//
	if (pGameParams->bSoundEffectsEnabled) {
		//pSound = new CSound((CWnd *)this, WAV_DEATH, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);
		sndPlaySound(WAV_DEATH, SND_ASYNC);
	}

	//CSound::waitWaveSounds();

	if (bAnimate) {

		if ((pSprite = m_pMasterGoodDie->DuplicateSprite(pDC)) != nullptr) {

			pSprite->SetAnimated(true);
			pSprite->LinkSprite();
			pSprite->PaintSprite(pDC, m_pHodj->GetPosition());

			m_pHodj->EraseSprite(pDC);

			for (i = 1; i < N_HODJDIE_CELLS; i++) {
				pSprite->PaintSprite(pDC, m_pHodj->GetPosition());
				pause();
				Sleep(300);
			}
			DeleteSprite(pSprite);
		}
	}

	if (m_nLives > 0) {

		FlushInputEvents();

		// Inform user that he has lost a life
		//
		Common::sprintf_s(szTmpBuf, "You have %d li%s left.", m_nLives, m_nLives != 1 ? "ves" : "fe");
		CMessageBox dlgLoseLife((CWnd *)this, m_pGamePalette, "That Hurts!", szTmpBuf);

		//
		// Need to erase all on-screen arrows
		//
		pList = m_pBadArrowList;                // destroy all bad arrows
		while (pList != nullptr) {
			pNext = pList->getNext();
			DestroyBadArrow(pList);
			pList = pNext;
		}
		pList = m_pGoodArrowList;               // destroy all good arrows
		while (pList != nullptr) {
			pNext = pList->getNext();
			DestroyGoodArrow(pList);
			pList = pNext;
		}

		KillAnimation();                        // terminate any animation sequences

		PruneDeadBadGuys();

		//
		// Need to reset bad guys to top of screen
		//
		i = 0;
		pList = m_pBadGuyList;
		while (pList != nullptr) {
			pSprite = (CSprite *)pList->getData();

			assert(pSprite != nullptr);

			point = aBadGuyPosition[m_nWave % N_WAVES][i];
			point.x -= (m_nInitArcherLevel - 1) * BADGUYMOVE_X;
			pSprite->PaintSprite(pDC, point);
			i++;
			pList = pList->getNext();
		}
		m_pHodj->PaintSprite(pDC, m_pHodj->GetPosition());

		PostMessage(WM_RBUTTONUP, 0, 0);

		//GameResume();                           // resume the timer
	} else {

		// Game Over
		//
		if (pGameParams->bSoundEffectsEnabled)
			sndPlaySound(WAV_GAMEOVER, SND_SYNC);

		FlushInputEvents();

		Common::sprintf_s(szTmpBuf, "Score: %ld.", m_lScore);
		CMessageBox dlgGameOver((CWnd *)this, m_pGamePalette, "Game over.", szTmpBuf);
		GameReset();

		if (pGameParams->bPlayingMetagame) {
			PostMessage(WM_CLOSE, 0, 0);
		}
	}
}

void CMainWindow::PruneDeadBadGuys() {
	CLList *pList, *pNext;
	CSprite *pSprite;

	pList = m_pBadGuyList;
	while (pList != nullptr) {

		pNext = pList->getNext();

		pSprite = (CSprite *)(pList->getData());
		assert(pSprite != nullptr);

		if (pSprite->GetTypeCode() == 1) {
			delete pSprite;

			// if this is the 1st bad guy in the list, then move the head
			//
			if (pList == m_pBadGuyList)
				m_pBadGuyList = pNext;

			delete pList;
		}

		pList = pNext;
	}
}

void CMainWindow::DestroyBadGuy(CLList *pList, CDC *pDC) {
	CSprite *pSprite, *pNewSprite;
	CLList *pNewList;
	POINT point;
	int i;

	// can't access null pointers
	assert(pList != nullptr);
	assert(pDC != nullptr);

	// can't delete a non existant bad guy
	assert(m_nBadGuys != 0);

	// one less bad guy
	m_nBadGuys--;

	// Update the score
	//
	pSprite = (CSprite *)pList->getData();

	assert(pSprite != nullptr);

	point = pSprite->GetPosition();
	m_lScore += ((LEVEL1 - point.x) / BADGUYMOVE_X) + 1;

	// add code to give player extra life every 400, 800, 1600 etc. points
	//
	if ((m_nLives <= 4) && (m_lScore >= m_lNewLifeScore)) {

		// play the You get a new life sound
		if (pGameParams->bSoundEffectsEnabled) {
			sndPlaySound(m_pExtraLifeSound, SND_MEMORY | SND_SYNC | SND_NODEFAULT);
		}

		m_lNewLifeScore += m_lNewLifeScore;
		m_pLives[m_nLives] = NewLife(m_nLives);
		m_nLives++;
	}

	// increase bad guy speed
	//
	m_nBadGuySpeed = m_nBadGuys * m_nGameSpeed * SPEED_FACTOR;

	// Play the Bad Guy gets killed sound
	//
	if (pGameParams->bSoundEffectsEnabled) {
		sndPlaySound(m_pBadDieSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
	}

	// indicate that this badguy is dead
	pSprite->SetTypeCode(true);

	//
	// perform some animation to kill a bad guy
	//
	if ((pNewSprite = m_pMasterBadDie->DuplicateSprite(pDC)) != nullptr) {
		pNewSprite->SetZOrder(SPRITE_BACKGROUND);
		pNewSprite->SetAnimated(true);
		pNewSprite->LinkSprite();
		pNewSprite->PaintSprite(pDC, point);
	}

	pSprite->EraseSprite(pDC);
	pSprite->UnlinkSprite();

	//
	// if this is not the last bad guy, the set up an animation event
	//
	if (m_nBadGuys != 0) {

		if ((pNewList = new CLList) != nullptr) {

			pNewList->PutData(pNewSprite);

			if (m_pFXList == nullptr)
				m_pFXList = pNewList;
			else
				m_pFXList->Insert(pNewList);
		}

	} else if (pNewSprite) {
		KillAnimation();

		//
		// since this is the last bad guy, then do the animation inline
		//
		for (i = 1; i < N_BADDIE_CELLS; i++) {

			pNewSprite->PaintSprite(pDC, pNewSprite->GetPosition());
			Sleep(300);
		}
		DeleteSprite(pNewSprite);
	}
}

void CMainWindow::DestroyHay(CLList *pList, CRect rect, CDC *pDC, bool bAnimate) {
	CSprite *pSprite;
	CRect tmpRect, overlappedRect;
	POINT ptTmp, point;
	int i, nID;

	// can't access null pointers
	assert(pList != nullptr);
	assert(pDC != nullptr);

	pSprite = (CSprite *)pList->getData();
	assert(pSprite != nullptr);

	point = pSprite->GetPosition();
	nID = pSprite->GetTypeCode();

	// can't hit an empty bale
	assert(aBales[nID] > 0);

	// find and crop all sections of hay that need to be cropped
	//
	for (i = 0; i < N_SECTIONS_PER_HAY; i++) {
		ptTmp = aHayPosUse[nID][i];

		if (ptTmp.x != 0) {

			tmpRect.SetRect(ptTmp.x, ptTmp.y, ptTmp.x + 8 - 1, ptTmp.y + 6 - 1);

			if (overlappedRect.IntersectRect(tmpRect, rect)) {
				tmpRect.left -= point.x;
				tmpRect.top -= point.y;
				tmpRect.right -= point.x - 1;
				tmpRect.bottom -= point.y - 1;
				pSprite->CropImage(pDC, &tmpRect);
				aHayPosUse[nID][i].x = 0;
				aHayPosUse[nID][i].y = 0;

				// one less bale in this hay
				--aBales[nID];

				if (aBales[nID] != 0) {
					pSprite->PaintSprite(pDC, pSprite->GetPosition());

				} else {
					//
					// All sections have been removed from this Bale
					//
					DeleteSprite(pSprite);
					if (pList == m_pHayList)
						m_pHayList = m_pHayList->getNext();

					delete pList;
					break;
				}

				if (bAnimate) {
					CreateBurningHay(ptTmp);
				}
			}
		}
	}

	if (bAnimate) {

		// sound effect for when hay is struck by flaming arrow
		if (pGameParams->bSoundEffectsEnabled) {
			sndPlaySound(m_pBurnSound, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
		}
	}
}

void CMainWindow::DestroyGoodArrow(CLList *pList) {
	// can't access a null pointer
	assert(pList != nullptr);

	// can't delete a non-existant arrow
	assert(m_nGoodArrows > 0);

	// one less arrow
	m_nGoodArrows--;

	// Delete the actual sprite
	//
	DeleteSprite((CSprite *)pList->getData());

	// if this is the 1st arrow in the list, then move the head
	//
	if (pList == m_pGoodArrowList)
		m_pGoodArrowList = pList->getNext();

	delete pList;
}


void CMainWindow::DestroyBadArrow(CLList *pList) {
	// can't access a null pointer
	assert(pList != nullptr);

	// can't delete a non-existant arrow
	assert(m_nBadArrows > 0);

	// one less arrow
	m_nBadArrows--;

	// Delete the actual sprite
	//
	DeleteSprite((CSprite *)pList->getData());

	// if this is the 1st arrow in the list, then move the head
	//
	if (pList == m_pBadArrowList)
		m_pBadArrowList = pList->getNext();

	delete pList;
}


void CMainWindow::DeleteSprite(CSprite *pSprite) {
	CDC *pDC;

	// can't delete a null pointer
	assert(pSprite != nullptr);

	if ((pDC = GetDC()) != nullptr) {
		pSprite->EraseSprite(pDC);                  // erase it from screen
		ReleaseDC(pDC);
	}
	pSprite->UnlinkSprite();                        // unlink it

	delete pSprite;                                 // delete it
}



void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CRect   cTestRect;

	cTestRect = m_pScrollSprite->GetRect();

	if (cTestRect.PtInRect(point)) {
		SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);

		// User clicked on the Title - NewGame button
		//
	} else if (m_rNewGameButton.PtInRect(point) && !m_bMoveMode) {

		// if we are not playing from the metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			// start a new game
			PlayGame();
		}

		// User is trying to shoot an arrow
		//
	} else {

		if (m_bGameActive)
			CreateGoodArrow();
	}

	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
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


void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	switch (nChar) {

	// Shoot an arrow
	//
	case VK_RETURN:
	case VK_SPACE:
		if (m_bGameActive) {
			CreateGoodArrow();
		}
		break;

	// Move good-guy up
	//
	case VK_UP:
		MoveHodj(-GOODGUYMOVE_Y);
		break;

	// Move good-guy down
	//
	case VK_DOWN:
		MoveHodj(GOODGUYMOVE_Y);
		break;

	//
	// Bring up the Rules
	//
	case VK_F1: {
		GamePause();
		CSound::waitWaveSounds();
		CRules  RulesDlg(this, "arch.txt", m_pGamePalette, (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr));
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

	default:
		CFrameWnd::OnChar(nChar, nRepCnt, nFlags);
		break;
	}
}

void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_Q
	//
	if ((nChar == 'q') && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CFrameWnd ::OnSysChar(nChar, nRepCnt, nFlags);
	}
}

void CMainWindow::MoveHodj(int y) {
	POINT point;
	CDC *pDC;

	//
	// As long as the game is active, move hodj
	//
	if (m_bGameActive) {

		// if Hodj actually moved
		//
		if (y != 0) {

			GameResume();

			// something is wrong if hodj does not exist
			assert(m_pHodj != nullptr);

			// limit his movement
			//
			if (y > GOODGUYMOVE_Y * 2) {
				y = GOODGUYMOVE_Y * 2;

			} else if (y < GOODGUYMOVE_Y * -2) {
				y = GOODGUYMOVE_Y * -2;
			}

			point = m_pHodj->GetPosition();
			point.y += y;

			if (point.y > GAME_HEIGHT - (GAME_BOTTOM_BORDER_WIDTH + m_pHodj->GetSize().cy)) {
				point.y = GAME_HEIGHT - (GAME_BOTTOM_BORDER_WIDTH + m_pHodj->GetSize().cy);

			} else if (point.y < GAME_TOP_BORDER_WIDTH) {
				point.y = GAME_TOP_BORDER_WIDTH;
			}

			if ((pDC = GetDC()) != nullptr) {
				m_pHodj->PaintSprite(pDC, point);
				ReleaseDC(pDC);
			}
		}
	}
}


void CMainWindow::HandleError(ERROR_CODE errCode) {
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


void CMainWindow::OnClose() {
	CBrush myBrush;
	CRect  myRect;
	CDC   *pDC;

	ClipCursor(nullptr);                           // release mouse limits

	CSound::clearSounds();                      // stop and delete all CSounds

	ReleaseMasterSounds();                      // release pre-loaded WAVs

	ReleaseMasterSprites();                     // delete all master sprite objects

	KillTimer(TIMER_ID);                        // kill the timer (if any)

	ReleaseCapture();                           // release the capture of mouse events

	GameReset();                                // perform clean-up

	if (m_bJoyActive)                           // release the joystick
		joyReleaseCapture(JOYSTICKID1);


	if (m_pScrollSprite != nullptr) {              // release sprite scroll button
		delete m_pScrollSprite;
		m_pScrollSprite = nullptr;
	}

	CSprite::ClearBackdrop();                   // free the off screen bitmap

	if (m_pGamePalette != nullptr) {               // need to de-allocate the palette
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

	DestroyWindow();                            // destruct the main window

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}


long CMainWindow::OnJoyStick(unsigned int wParam, long lParam) {
	long nThreshold;

	if (m_bGameActive) {

		if (wParam & JOY_BUTTON1) {
			CreateGoodArrow();
		}

		nThreshold = m_nJoyLast;
		nThreshold -= (unsigned int)HIWORD(lParam);

		if (nThreshold > 5000) {
			MoveHodj(-GOODGUYMOVE_Y);

			/*if (nThreshold > 20000)
			    MoveHodj(-GOODGUYMOVE_Y);*/
		}

		if (nThreshold < -5000) {
			MoveHodj(GOODGUYMOVE_Y);

			/*if (nThreshold < -20000)
			    MoveHodj(GOODGUYMOVE_Y);*/
		}
	}

	return 0;
}

void CMainWindow::GamePause() {
	m_bPause = true;
	if (m_bMoveMode) {
		m_bMoveMode = false;
		MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}
}


void CMainWindow::GameResume() {
	m_bPause = false;

	if (m_bMoveMode)
		SetCursor(nullptr);
}

void CMainWindow::OnActivate(unsigned int nState, CWnd *, bool) {
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
		SetFocus();
}


void CMainWindow::OnRButtonUp(unsigned int, CPoint) {
	//
	// Toggle Good Guy Move Mode
	//
	if (m_bGameActive) {
		m_bMoveMode = (m_bMoveMode ? false : true);

		if (m_bMoveMode) {
			GameResume();

			// hide the cursor
			SetCursor(nullptr);

			// move cursor to anchor point
			SetCursorPos(m_ptAnchor.x, m_ptAnchor.y);
		} else {

			GamePause();

			// unhide the cursor
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
	}
}

void CMainWindow::OnMouseMove(unsigned int, CPoint point) {
	//
	// If the Right mouse button is down when we move the mouse, then
	// the user is moving the good guy, otherwise it is a normal mouse move.
	//
	if (m_bGameActive && !m_bPause && m_bMoveMode) {

		GetCursorPos(&point);

		// Move good guy according to how the mouse was moved
		//
		MoveHodj(point.y - m_ptAnchor.y);

		// reset cursor position
		//
		SetCursorPos(m_ptAnchor.x, m_ptAnchor.y);

	} else {

		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}
}

//////////// Additional Sound Notify routines //////////////

LRESULT CMainWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CMainWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CMainWindow::OnSoundNotify(CSound *) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

void CMainWindow::FlushInputEvents() {
	MSG msg;

	// find and remove all keyboard events
	//
	while (true) {
		if (!PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	// find and remove all mouse events
	//
	while (true) {
		if (!PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}

//
// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSCHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//ON_MESSAGE(MM_JOY1MOVE, OnJoyStick)
END_MESSAGE_MAP()


void CALLBACK GetGameParams(CWnd *pParentWnd) {
	//
	// Our user preference dialog box is self contained in this object
	//
	CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);
}

} // namespace Archeroids
} // namespace HodjNPodj
} // namespace Bagel
