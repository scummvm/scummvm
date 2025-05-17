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
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/battlefish/bfish.h"
#include "bagel/hodjnpodj/battlefish/usercfg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Battlefish {

#define CSOUND 0

//INT bob[5] = { 27, 56, 7, 63, 31 };
//INT fred;

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP   ".\\ART\\BFISH.BMP"

//
// Game theme song
//
#define MID_SOUNDTRACK  ".\\SOUND\\BFISH.MID"

//
// Button ID constants
//
#define IDC_MENU     100

#define COMPUTERS_TURN 101

// Bitmap IDs
//
#define IDB_FISH     200
#define IDB_FISH0    200
#define IDB_FISH1    201
#define IDB_FISH2    202
#define IDB_FISH3    203

#define IDB_HARPOON  204

#define IDB_HIT      205
#define IDB_MISS     206
#define IDB_FISHROT  207
#define IDB_FISHROT0 207
#define IDB_FISHROT1 208
#define IDB_FISHROT2 202     // 2x2 fish has same picture
#define IDB_FISHROT3 210

// animation sprites
#define IDB_PLUME    IDB_MISS
#define IDB_HARP     IDB_HIT
#define IDB_APLUME   211
#define IDB_AHARP    212

#define IDB_HOOK    213
#define IDB_HOOK0   213
#define IDB_HOOK1   214
#define IDB_HOOK2   215
#define IDB_HOOK3   216

#define IDB_OCTOPUS 217

#define N_PLUME_CELS  14
#define N_HARP_CELS   10

// Rules Text File Identifier
//
#define RULESFILE       "BFISH.TXT"

//
// .WAV sounds for Battle Fish
//
#define WAV_PLACESHIP   ".\\SOUND\\PUTFISH.WAV"
#define WAV_ROTATESHIP  ".\\SOUND\\TURNFISH.WAV"
#define WAV_MYTURN1     ".\\SOUND\\IGO1.WAV"
#define WAV_MYTURN2     ".\\SOUND\\IGO2.WAV"
#define WAV_YOURTURN1   ".\\SOUND\\YOUGO1.WAV"
#define WAV_YOURTURN2   ".\\SOUND\\YOUGO2.WAV"
#define WAV_SHOOT       ".\\SOUND\\SHOOT.WAV"
#define WAV_YOUMISS     ".\\SOUND\\MISS.WAV"
#define WAV_YOUHIT      ".\\SOUND\\HIT.WAV"
#define WAV_BADSINK1    ".\\SOUND\\SANK1.WAV"
#define WAV_BADSINK2    ".\\SOUND\\SANK2.WAV"
#define WAV_BADSINK3    ".\\SOUND\\SANK3.WAV"
#define WAV_BADSINK4    ".\\SOUND\\SANK4.WAV"
#define WAV_BADSINK5    ".\\SOUND\\SANK5.WAV"
#define WAV_BADSINK6    ".\\SOUND\\SANK6.WAV"
#define WAV_BADSINK7    ".\\SOUND\\SANK7.WAV"
#define WAV_BADSINK8    ".\\SOUND\\SANK8.WAV"
#define WAV_YOUSINK     ".\\SOUND\\SINKFISH.WAV"
#define WAV_YOUWIN      ".\\SOUND\\FANFARE2.WAV"
#define WAV_GAMEOVER    ".\\SOUND\\SOSORRY.WAV"
#define WAV_INVALID     ".\\SOUND\\INVALID.WAV"
#define WAV_NARRATION   ".\\SOUND\\BFISH.WAV"               // Rules wav file

#define NUM_SINK_WAVS   8                                   // Number of "You sank my.." sounds
#define NUM_TURN_WAVS   2
#define VOICE_CUTOFF    2

//
// Audio easter eggs
//
#define WAV_WINDOW      ".\\SOUND\\WINDOW.WAV"
#define WAV_TRAWLER     ".\\SOUND\\FOGHORN.WAV"
#define WAV_ROWBOAT     ".\\SOUND\\ROWBOAT.WAV"
#define WAV_SAILBOAT    ".\\SOUND\\ANCHORS.WAV"
//
// Audio easter egg locations
//
#define WINDOW_X        70
#define WINDOW_Y        23
#define WINDOW_DX       41
#define WINDOW_DY       49

#define TRAWLER_X       156
#define TRAWLER_Y       23
#define TRAWLER_DX      138
#define TRAWLER_DY      93

#define ROWBOAT_X       537
#define ROWBOAT_Y       135
#define ROWBOAT_DX      87
#define ROWBOAT_DY      40

#define SAILBOAT_X      323
#define SAILBOAT_Y      23
#define SAILBOAT_DX     74
#define SAILBOAT_DY     71

#define OCTOPUS_X       233
#define OCTOPUS_Y       95

#define EMPTY           0x00
#define SHOT            0x01

#define FISH0           0
#define FISH1           1
#define FISH2           2
#define FISH3           3

#define NONE            0x50

#define SEARCH_FACTOR   5

#define DIFF_WIMPY      0
#define DIFF_AVERAGE    1
#define DIFF_HEFTY      2

#define FISHBIN_X       92
#define FISHBIN_Y      166

#define RGRID_MIN_X    321
#define RGRID_MIN_Y    202
#define RGRID_MAX_X    598
#define RGRID_MAX_Y    442


STATIC FISH gFishSizes[MAX_FISH] = {
	{{{0, 0}, {0, 1}, {NONE, NONE}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}, 1 * 2},
	{{{0, 1}, {0, 0}, {0, 2}, {NONE, NONE}, {0, 0}, {0, 0}, {0, 0}}, 1 * 3},
	{{{0, 0}, {0, 1}, {1, 0}, {1, 1}, {NONE, NONE}, {0, 0}, {0, 0}}, 2 * 2},
	{{{0, 1}, {1, 1}, {0, 0}, {1, 0}, {0, 2}, {1, 2}, {NONE, NONE}}, 2 * 3}
};

STATIC POINT ptHarpoons[MAX_TURNS] = {
	{460, 153},
	{450, 153},
	{440, 153},
	{430, 153}
};


STATIC POINT ptFishBin[MAX_FISH] = {
	{FISHBIN_X, FISHBIN_Y},
	{FISHBIN_X + 48, FISHBIN_Y},
	{FISHBIN_X + 136, FISHBIN_Y - 25},
	{FISHBIN_X + 184, FISHBIN_Y},
};

STATIC POINT ptFishHooks[MAX_FISH] = {
	{430, GAME_TOP_BORDER_WIDTH},
	{470, GAME_TOP_BORDER_WIDTH},
	{505, GAME_TOP_BORDER_WIDTH},
	{550, GAME_TOP_BORDER_WIDTH},
};

STATIC const CHAR *pszFishSound[MAX_FISH] = {
	WAV_BADSINK8,
	WAV_BADSINK4,
	WAV_BADSINK7,
	WAV_BADSINK1
};


// Local Prototypes
//
VOID CALLBACK GetGameParams(CWnd *);

//
// Globals
//
CPalette    *pGamePalette;
const CHAR  *INI_SECTION = "BattleFish";
LPGAMESTRUCT pGameParams;

#ifdef _USRDLL
	extern HWND ghParentWnd;
#endif


// these arrays to be filled with the values of the grid screen coordinates
//
POINT gLeftGrid[GRID_ROWS][GRID_COLS] = {

	{{82, 203}, {110, 203}, {138, 203}, {168, 203}, {196, 203}, {224, 203}, {252, 203}, {281, 203}},
	{{77, 230}, {106, 230}, {135, 230}, {163, 230}, {193, 230}, {222, 230}, {251, 230}, {280, 230}},
	{{72, 257}, {102, 257}, {131, 257}, {160, 257}, {190, 257}, {220, 257}, {249, 257}, {279, 257}},
	{{67, 284}, { 97, 284}, {127, 284}, {156, 284}, {187, 284}, {218, 284}, {248, 284}, {278, 284}},
	{{60, 315}, { 91, 315}, {122, 315}, {153, 315}, {184, 315}, {215, 315}, {246, 315}, {277, 315}},
	{{52, 346}, { 86, 346}, {118, 346}, {149, 346}, {181, 346}, {212, 346}, {244, 346}, {276, 346}},
	{{48, 381}, { 80, 381}, {113, 381}, {145, 381}, {178, 381}, {213, 381}, {245, 381}, {278, 381}},
	{{41, 414}, { 74, 414}, {108, 414}, {141, 414}, {174, 414}, {208, 414}, {241, 414}, {275, 414}},

	/*{{54, 344}, { 86, 344}, {118, 344}, {149, 344}, {181, 344}, {212, 344}, {244, 344}, {276, 344}},
	{{48, 376}, { 80, 376}, {113, 376}, {145, 376}, {177, 376}, {210, 376}, {242, 376}, {275, 376}},
	{{41, 410}, { 74, 410}, {108, 410}, {141, 410}, {174, 410}, {207, 410}, {240, 410}, {274, 410}},*/
};

POINT gRightGrid[GRID_ROWS][GRID_COLS] = {

	{{324, 203}, {353, 203}, {381, 203}, {410, 203}, {439, 203}, {467, 203}, {496, 203}, {524, 203}},
	{{325, 230}, {354, 230}, {383, 230}, {412, 230}, {442, 230}, {470, 230}, {499, 230}, {529, 230}},
	{{325, 257}, {355, 257}, {385, 257}, {414, 257}, {444, 257}, {474, 257}, {503, 257}, {533, 257}},
	{{325, 284}, {356, 284}, {386, 284}, {417, 284}, {447, 284}, {478, 284}, {507, 284}, {538, 284}},
	{{325, 314}, {357, 314}, {388, 314}, {419, 314}, {450, 314}, {480, 314}, {512, 314}, {543, 314}},
	{{326, 344}, {358, 344}, {390, 344}, {422, 344}, {453, 344}, {485, 344}, {516, 344}, {548, 344}},
	{{326, 376}, {359, 376}, {392, 376}, {424, 376}, {457, 376}, {489, 376}, {521, 376}, {553, 376}},
	{{327, 410}, {360, 410}, {394, 410}, {427, 410}, {460, 410}, {493, 410}, {525, 410}, {560, 410}},
};

/*****************************************************************
*
*  CBFishWindow
*
*  FUNCTIONAL DESCRIPTION:
*
*       Constructor for CBFishWindow
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
CBFishWindow::CBFishWindow(VOID) {
	CString  WndClass;
	CRect    tmpRect;
	CDC     *pDC;
	CDibDoc *pDibDoc;
	ERROR_CODE errCode;
	BOOL bSuccess;

	// assume no error
	errCode = ERR_NONE;

	// Initialize members
	//
	m_pGamePalette = NULL;
	m_bPause = FALSE;
	m_bGameActive = FALSE;
	m_bMovingFish = FALSE;
	m_bUserEditMode = FALSE;
	m_bInMenu = FALSE;
	m_pMasterHit = NULL;
	m_pMasterMiss = NULL;
	m_pDragFish = NULL;
	m_pTxtClickHere = NULL;
	m_pScrollSprite = NULL;
	m_pSoundTrack = NULL;                               // Game theme song

	// make sure score is initially zero
	pGameParams->lScore = 0;

	// Init the fish sprites
	memset(m_pFish, 0, sizeof(CSprite *) * MAX_FISH);
	memset(m_pEnemyFish, 0, sizeof(CSprite *) * MAX_FISH);

	// Init the player grids
	//
	memset(m_nUserGrid, EMPTY, sizeof(UBYTE) * GRID_ROWS * GRID_COLS);
	memset(m_nEnemyGrid, EMPTY, sizeof(UBYTE) * GRID_ROWS * GRID_COLS);

	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	// Set the coordinates for the "End Placement Button"
	m_rEndPlacement.SetRect(380, 157, 531, 200);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, NULL, NULL, NULL);

	// can't play this game if the background art is not available
	//
	if (FileExists(MINI_GAME_MAP)) {

		// Acquire the shared palette for our game from the splash screen art
		//
		if ((pDibDoc = new CDibDoc()) != NULL) {
			if (pDibDoc->OpenDocument(MINI_GAME_MAP) != FALSE)
				pGamePalette = m_pGamePalette = pDibDoc->DetachPalette();
			else
				errCode = ERR_UNKNOWN;
			delete pDibDoc;
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
	if ((pDC = GetDC()) != NULL) {
		tmpRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
		tmpRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
		tmpRect.right = tmpRect.left + GAME_WIDTH;
		tmpRect.bottom = tmpRect.top + GAME_HEIGHT;
		ReleaseDC(pDC);
	} else {
		errCode = ERR_UNKNOWN;
	}
	#endif

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	//
	Create(WndClass, "Boffo Games -- Battlefish", WS_POPUP, tmpRect, NULL, NULL);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	PaintScreen();
	EndWaitCursor();

	// only continue if there was no error
	//
	if (errCode == ERR_NONE) {

		if ((m_pScrollSprite = new CSprite) != NULL) {

			m_pScrollSprite->SharePalette(m_pGamePalette);

			if ((pDC = GetDC()) != NULL) {
				bSuccess = m_pScrollSprite->LoadResourceSprite(pDC, IDB_SCROLBTN);

				if (!bSuccess)
					errCode = ERR_UNKNOWN;
				ReleaseDC(pDC);
			} else {
				errCode = ERR_MEMORY;
			}
			m_pScrollSprite->SetMasked(TRUE);
			m_pScrollSprite->SetMobile(TRUE);
		} else {
			errCode = ERR_MEMORY;
		}

		// only continue if there was no error
		//
		if (errCode == ERR_NONE) {

			#ifdef  BAGEL_DEBUG
			pGameParams->bMusicEnabled = TRUE;
			#endif  //BAGEL_DEBUG

			// Start the BFish soundtrack
			//
			if (pGameParams->bMusicEnabled) {
				if ((m_pSoundTrack = new CSound) != NULL) {
					m_pSoundTrack->initialize(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					m_pSoundTrack->midiLoopPlaySegment(2470, 32160, 0, FMT_MILLISEC);
				} else {
					errCode = ERR_MEMORY;
				}
			}

			// seed the random number generator
			//srand((unsigned)time(NULL));

			errCode = LoadMasterSprites();

			// if we are not playing from the metagame
			//
			if (!pGameParams->bPlayingMetagame) {

				// Automatically bring up the main menu
				//
				PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
			}
		}
	}

	HandleError(errCode);
}

/*****************************************************************
*
*  HandleError
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles Fatal error by show a message box, and posting WM_CLOSE
*
*  FORMAL PARAMETERS:
*
*       ERROR_CODE = Error return code to indicate type of fatal error
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::HandleError(ERROR_CODE errCode) {
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
		ValidateRect(NULL);
	}
}

ERROR_CODE CBFishWindow::LoadMasterSprites(VOID) {
	CDC *pDC;
	INT i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != NULL) {

		if ((m_pMasterHit = new CSprite) != NULL) {

			// this BMP uses the same palette as entire game
			//
			if (m_pMasterHit->SharePalette(m_pGamePalette) != FALSE) {

				if (m_pMasterHit->LoadResourceSprite(pDC, IDB_HIT) != FALSE) {

					m_pMasterHit->SetMasked(TRUE);
					m_pMasterHit->SetMobile(TRUE);
					m_pMasterHit->SetZOrder(SPRITE_TOPMOST);
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

			if ((m_pMasterMiss = new CSprite) != NULL) {

				// this BMP uses the same palette as entire game
				//
				if (m_pMasterMiss->SharePalette(m_pGamePalette) != FALSE) {

					if (m_pMasterMiss->LoadResourceSprite(pDC, IDB_MISS) != FALSE) {

						m_pMasterMiss->SetMasked(TRUE);
						m_pMasterMiss->SetMobile(TRUE);
						m_pMasterMiss->SetZOrder(SPRITE_TOPMOST);
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

			if ((m_pMasterHarpoon = new CSprite) != NULL) {

				// this BMP uses the same palette as entire game
				//
				if (m_pMasterHarpoon->SharePalette(m_pGamePalette) != FALSE) {

					if (m_pMasterHarpoon->LoadResourceSprite(pDC, IDB_HARPOON) != FALSE) {

						m_pMasterHarpoon->SetMasked(TRUE);
						m_pMasterHarpoon->SetMobile(TRUE);
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

			if ((m_pOctopus = new CSprite) != NULL) {

				// this BMP uses the same palette as entire game
				//
				if (m_pOctopus->SharePalette(m_pGamePalette) != FALSE) {

					if (m_pOctopus->LoadResourceSprite(pDC, IDB_OCTOPUS) != FALSE) {
						m_pOctopus->SetZOrder(SPRITE_BACKGROUND);
						m_pOctopus->SetMasked(FALSE);
						m_pOctopus->SetMobile(FALSE);
						m_pOctopus->SetPosition(OCTOPUS_X, OCTOPUS_Y);
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

			for (i = 0; i < MAX_FISH; i++) {

				if ((m_pFish[i] = new CSprite) != NULL) {

					// attach sprite to the Game Palette
					//
					if (m_pFish[i]->SharePalette(m_pGamePalette) != FALSE) {

						if (m_pFish[i]->LoadResourceSprite(pDC, IDB_FISH + i) != FALSE) {

							m_pFish[i]->SetTypeCode(FALSE);
							m_pFish[i]->SetMasked(TRUE);
							m_pFish[i]->SetMobile(TRUE);
							m_pFish[i]->SetZOrder(SPRITE_BACKGROUND);
							//m_pFish[i]->SetPosition(ptFishBin[i]);
							//m_pFish[i]->LinkSprite();
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
		}

		if (errCode == ERR_NONE) {

			for (i = 0; i < MAX_FISH; i++) {

				if ((m_pEnemyFish[i] = new CSprite) != NULL) {

					// attach good guy to the Game Palette
					//
					if (m_pEnemyFish[i]->SharePalette(m_pGamePalette) != FALSE) {

						if (m_pEnemyFish[i]->LoadResourceSprite(pDC, IDB_HOOK + i) != FALSE) {

							// TRUE if linked into chain, FALSE otherwise
							m_pEnemyFish[i]->SetTypeCode(FALSE);

							m_pEnemyFish[i]->SetMasked(TRUE);
							m_pEnemyFish[i]->SetMobile(TRUE);
							m_pEnemyFish[i]->SetPosition(ptFishHooks[i]);
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
		}

		ReleaseDC(pDC);

	} else {
		errCode = ERR_MEMORY;
	}

	return (errCode);
}


VOID CBFishWindow::ReleaseMasterSprites(VOID) {
	INT i;

	// free the hooked fish, and the users fish
	//
	for (i = 0; i < MAX_FISH; i++) {

		if (m_pEnemyFish[i] != NULL) {
			delete m_pEnemyFish[i];
			m_pEnemyFish[i] = NULL;
		}

		if (m_pFish[i] != NULL) {
			delete m_pFish[i];
			m_pFish[i] = NULL;
		}
	}

	// free the octopus sprite
	//
	assert(m_pOctopus != NULL);
	if (m_pOctopus != NULL) {
		delete m_pOctopus;
		m_pOctopus = NULL;
	}

	// free the master turn-harpoon sprite
	//
	assert(m_pMasterHarpoon != NULL);
	if (m_pMasterHarpoon != NULL) {
		delete m_pMasterHarpoon;
		m_pMasterHarpoon = NULL;
	}

	// free the master shoot-miss sprite
	//
	assert(m_pMasterMiss != NULL);
	if (m_pMasterMiss != NULL) {
		delete m_pMasterMiss;
		m_pMasterMiss = NULL;
	}

	// free the master shoot-hit sprite
	//
	assert(m_pMasterHit != NULL);
	if (m_pMasterHit != NULL) {
		delete m_pMasterHit;
		m_pMasterHit = NULL;
	}
}


/*****************************************************************
*
*  OnPaint
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_PAINT messages
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	Invalidate(FALSE);
	BeginPaint(&lpPaint);
	PaintScreen();
	EndPaint(&lpPaint);
}


/*****************************************************************
*
*  PaintScreen
*
*  FUNCTIONAL DESCRIPTION:
*
*       Repaints background art, sprites, and text fields
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::PaintScreen() {
	CDibDoc myDoc;
	CRect   rcDest;
	CRect   rcDIB;
	HDIB    hDIB;
	CDC     *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	//
	// Paint the background art and update any sprites
	//
	if (FileExists(MINI_GAME_MAP)) {

		myDoc.OpenDocument(MINI_GAME_MAP);
		hDIB = myDoc.GetHDIB();

		pDC = GetDC();
		assert(pDC != NULL);

		if (pDC != NULL) {

			if (hDIB && (m_pGamePalette != NULL)) {

				GetClientRect(rcDest);
				LPSTR lpDIB = (LPSTR) GlobalLock((HGLOBAL) hDIB);
				rcDIB.top = rcDIB.left = 0;
				rcDIB.right = (INT) DIBWidth(lpDIB);
				rcDIB.bottom = (INT) DIBHeight(lpDIB);
				GlobalUnlock((HGLOBAL) hDIB);
				PaintDIB(pDC->m_hDC, &rcDest, hDIB, &rcDIB, m_pGamePalette);
			}

			if (!m_bInMenu && (m_pScrollSprite != NULL)) {
				m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
			}

			// repaint any on-screen sprites
			//
			errCode = RepaintSpriteList(pDC);

			// redisplay the "click here" text
			if (m_pTxtClickHere != NULL) {
				m_pTxtClickHere->DisplayString(pDC, "Click here when done", 12, TEXT_NORMAL, RGB(0, 0, 0));
			}

			ReleaseDC(pDC);

		} else {
			errCode = ERR_MEMORY;
		}

	} else {
		errCode = ERR_FFIND;
	}

	HandleError(errCode);
}

/*****************************************************************************
*
*  RepaintSpriteList -
*
*  DESCRIPTION:     Repaint all Linked Sprites
*
*
*  SAMPLE USAGE:
*  errCode = RepaintSpriteList(pDC);
*  CDC *pDC;                                pointer to current device context
*
*  RETURNS:  ERROR_CODE = error return code
*
*****************************************************************************/
ERROR_CODE CBFishWindow::RepaintSpriteList(CDC *pDC) {
	CSprite *pSprite;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// can't use a null pointer
	assert(pDC != NULL);

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

	return (errCode);
}


/*****************************************************************
*
*  OnCommand
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_COMMAND messages
*
*  FORMAL PARAMETERS:
*
*       WPARAM = WORD parameter for this message
*       LPARAM = LONG parameter for this message
*
*  RETURN VALUE:
*
*       BOOL = TRUE if message was handled
*
****************************************************************/
BOOL CBFishWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CMainMenu COptionsWind((CWnd *)this,
	                       m_pGamePalette,
	                       (pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0) | (m_bGameActive || m_bUserEditMode ? 0 : NO_RETURN),
	                       GetGameParams, "bfish.txt", (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : NULL), pGameParams);
	CDC *pDC;
	//CSound *pSound;
	INT nPick = 0;

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		//
		// must bring up our menu of controls
		//
		case IDC_MENU:

			GamePause();

			m_bInMenu = TRUE;

			if ((pDC = GetDC()) != NULL) {
				m_pScrollSprite->EraseSprite(pDC);
			}

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

			if (pDC != NULL) {
				m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
				ReleaseDC(pDC);
			}

			m_bInMenu = FALSE;

			if (!pGameParams->bMusicEnabled && (m_pSoundTrack != NULL)) {

				m_pSoundTrack->stop();
				delete m_pSoundTrack;
				m_pSoundTrack = NULL;

			} else if (pGameParams->bMusicEnabled && (m_pSoundTrack == NULL)) {

				if ((m_pSoundTrack = new CSound) != NULL) {
					m_pSoundTrack->initialize(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					m_pSoundTrack->midiLoopPlaySegment(2470, 32160, 0, FMT_MILLISEC);
				}
			}

			GameResume();

			return (TRUE);

		case COMPUTERS_TURN:

			if ((pDC = GetDC()) != NULL) {
				if (m_pOctopus != NULL) {
					m_pOctopus->LinkSprite();
					m_pOctopus->PaintSprite(pDC, OCTOPUS_X, OCTOPUS_Y);
				}
				ReleaseDC(pDC);
			}

			if (pGameParams->bSoundEffectsEnabled &&
			        ((m_nUserFish > VOICE_CUTOFF) && (m_nEnemyFish > VOICE_CUTOFF))) {
				nPick = brand() % NUM_TURN_WAVS;
				if (nPick == 0) {
					#if CSOUND
					pSound = new CSound((CWnd *)this, WAV_MYTURN1, SOUND_WAVE | SOUND_AUTODELETE);
					#else
					sndPlaySound(WAV_MYTURN1, SND_SYNC);
					#endif
				} else {
					#if CSOUND
					pSound = new CSound((CWnd *)this, WAV_MYTURN2, SOUND_WAVE | SOUND_AUTODELETE);
					#else
					sndPlaySound(WAV_MYTURN2, SND_SYNC);
					#endif
				}
			}

			BeginWaitCursor();

			m_nTurns = m_nEnemyFish;
			while (m_nTurns-- > 0) {
				//CSound::HandleMessages();
				ComputersTurn();
			}

			m_nTurns = m_nUserFish;
			m_bUsersTurn = TRUE;

			FlushInputEvents();

			EndWaitCursor();

			if (m_bGameActive) {

				if (pGameParams->bSoundEffectsEnabled &&
				        ((m_nUserFish > VOICE_CUTOFF) && (m_nEnemyFish > VOICE_CUTOFF))) {
					nPick = brand() % NUM_TURN_WAVS;
					if (nPick == 0) {
						#if CSOUND
						pSound = new CSound((CWnd *)this, WAV_YOURTURN1, SOUND_WAVE | SOUND_AUTODELETE);
						#else
						sndPlaySound(WAV_YOURTURN1, SND_SYNC);
						#endif
					} else {
						#if CSOUND
						pSound = new CSound((CWnd *)this, WAV_YOURTURN2, SOUND_WAVE | SOUND_AUTODELETE);
						#else
						sndPlaySound(WAV_YOURTURN2, SND_SYNC);
						#endif
					}
				}

				PlaceTurnHarpoons();

				if ((pDC = GetDC()) != NULL) {
					if (m_pOctopus != NULL) {
						m_pOctopus->EraseSprite(pDC);
						m_pOctopus->UnlinkSprite();
					}
					ReleaseDC(pDC);
				}
			}
			break;

		default:
			assert(0);
			break;
		}
	}

	return (FALSE);
}

VOID CBFishWindow::PlaceTurnHarpoons(VOID) {
	CSprite *pSprite;
	CDC *pDC;
	INT i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != NULL) {

		for (i = 0; i < m_nTurns; i++) {

			// create a dup of the master harpoon
			//
			if ((pSprite = m_pMasterHarpoon->DuplicateSprite(pDC)) != NULL) {
				pSprite->LinkSprite();
				pSprite->PaintSprite(pDC, ptHarpoons[i]);
				m_pHarpoons[i] = pSprite;

			} else {
				errCode = ERR_MEMORY;
				break;
			}
		}
		ReleaseDC(pDC);

	} else {
		errCode = ERR_MEMORY;
	}

	HandleError(errCode);
}

VOID CBFishWindow::RemoveTurnHarpoon(VOID) {
	assert(m_nTurns >= 0 && m_nTurns < MAX_TURNS);
	assert(m_pHarpoons[m_nTurns] != NULL);

	DeleteSprite(m_pHarpoons[m_nTurns]);
	m_pHarpoons[m_nTurns] = NULL;
}


/*****************************************************************
*
*  GamePause
*
*  FUNCTIONAL DESCRIPTION:
*
*       Pauses the current game (if any)
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::GamePause(VOID) {
	m_bPause = TRUE;
};


/*****************************************************************
*
*  GameResume
*
*  FUNCTIONAL DESCRIPTION:
*
*       Resumes the current game (if any)
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::GameResume(VOID) {
	m_bPause = FALSE;
};


/*****************************************************************
*
*  PlayGame
*
*  FUNCTIONAL DESCRIPTION:
*
*       Stops any active game, resets all game parameters, and starts new game
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::PlayGame(VOID) {
	CRect rTmpRect;
	CDC *pDC;
	INT i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// load the .INI settings
	//
	LoadIniSettings();

	// reset all game parameters
	//
	GameReset();

	//
	// Start game
	//

	// show fish in the bin
	//
	if ((pDC = GetDC()) != NULL) {

		//RepaintSpriteList(pDC);
		for (i = 0; i < MAX_FISH; i++) {
			m_pFish[i]->LinkSprite();
			m_pFish[i]->PaintSprite(pDC, ptFishBin[i]);
		}

		// show the "Click here when you are finished placing your fish" text
		//
		if (m_pTxtClickHere != NULL) {
			m_pTxtClickHere->RestoreBackground(pDC);
			delete m_pTxtClickHere;
			m_pTxtClickHere = NULL;
		}
		rTmpRect.SetRect(380, 180, 490, 200);
		if ((m_pTxtClickHere = new CText) != NULL) {
			m_pTxtClickHere->SetupText(pDC, m_pGamePalette, &rTmpRect, JUSTIFY_LEFT);
			m_pTxtClickHere->DisplayString(pDC, "Click here when done", 12, TEXT_NORMAL, RGB(0, 0, 0));
		} else {
			errCode = ERR_MEMORY;
		}

		ReleaseDC(pDC);
	} else {
		errCode = ERR_MEMORY;
	}

	// computer places pieces
	PlaceEnemyFish();

	// User places pieces (fish)
	PlaceUserFish();

	HandleError(errCode);
}


VOID CBFishWindow::PlaceUserFish(VOID) {
	//
	// Initiate User-Edit-Mode (Allow user to drag and drop fish to grid)
	//
	m_bUserEditMode = TRUE;
}


VOID CBFishWindow::PlaceEnemyFish(VOID) {
	INT i, j, k;
	INT row, col, rowTmp, colTmp;
	BOOL bFound;

	// For each fish, randomly select a location in the grid (rotate if neccessary)
	//
	for (i = 0; i < MAX_FISH; i++) {

		bFound = FALSE;
		do {

			// select random starting square
			//
			row = brand() % GRID_ROWS;
			col = brand() % GRID_COLS;

			// make a copy of this fish
			memcpy(&m_aEnemyFishInfo[i], &gFishSizes[i], sizeof(FISH));

			// rotate some of the fish
			//
			if (brand() & 1) {
				for (k = 0; k < MAX_FISH_SIZE; k++) {
					m_aEnemyFishInfo[i].nLoc[k].x = gFishSizes[i].nLoc[k].y;
					m_aEnemyFishInfo[i].nLoc[k].y = gFishSizes[i].nLoc[k].x;
				}
			}

			// Try fish at both 0 and 90 degrees
			//
			for (j = 0; j < 2; j++) {

				// Does the fish fit at this location
				//
				bFound = TRUE;
				for (k = 0; k < MAX_FISH_SIZE; k++) {

					// if there are no more squares for this fish, then done
					//
					if (m_aEnemyFishInfo[i].nLoc[k].x == NONE)
						break;

					assert(m_aEnemyFishInfo[i].nLoc[k].y != NONE);

					rowTmp = (m_aEnemyFishInfo[i].nLoc[k].x += row);
					colTmp = (m_aEnemyFishInfo[i].nLoc[k].y += col);

					if ((rowTmp >= GRID_ROWS) || (colTmp >= GRID_COLS) || (m_nEnemyGrid[rowTmp][colTmp] != EMPTY)) {
						bFound = FALSE;
						break;
					}
				}

				// the fish fit - so set the grid and then go on to next fish
				//
				if (bFound) {
					for (k = 0; k < MAX_FISH_SIZE; k++) {

						// if there are no more squares for this fish, then done
						//
						if (m_aEnemyFishInfo[i].nLoc[k].x == NONE)
							break;

						assert(m_aEnemyFishInfo[i].nLoc[k].x < NONE);
						assert(m_aEnemyFishInfo[i].nLoc[k].y < NONE);

						rowTmp = m_aEnemyFishInfo[i].nLoc[k].x;
						colTmp = m_aEnemyFishInfo[i].nLoc[k].y;

						m_nEnemyGrid[rowTmp][colTmp] = (UBYTE)IndexToId(i);
					}
					break;
				}

				// Fish didn't fit, so try rotating it by swapping row
				// and column for each square in fish
				//
				for (k = 0; k < MAX_FISH_SIZE; k++) {
					m_aEnemyFishInfo[i].nLoc[k].x = gFishSizes[i].nLoc[k].y;
					m_aEnemyFishInfo[i].nLoc[k].y = gFishSizes[i].nLoc[k].x;
				}
			}

		} while (!bFound);
	}
}



/*****************************************************************
*
*  LoadIniSettings
*
*  FUNCTIONAL DESCRIPTION:
*
*       Loads this game's parameters from .INI file
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::LoadIniSettings(VOID) {
	INT nVal;

	if (pGameParams->bPlayingMetagame) {

		switch (pGameParams->nSkillLevel) {

		case SKILLLEVEL_LOW:
			m_nDifficultyLevel = 1;
			m_bUsersTurn = TRUE;
			break;

		case SKILLLEVEL_MEDIUM:
			m_nDifficultyLevel = 1;
			m_bUsersTurn = FALSE;
			break;

		case SKILLLEVEL_HIGH:
			m_nDifficultyLevel = 2;
			m_bUsersTurn = FALSE;
			break;

		default:
			assert(0);
			break;
		}

	} else {

		// Get the Difficulty level  (0..2)
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "DifficultyLevel", DIFF_DEF, INI_FILENAME);
		m_nDifficultyLevel = nVal;
		if (nVal < DIFF_MIN || nVal > DIFF_MAX)
			m_nDifficultyLevel = DIFF_DEF;

		nVal = GetPrivateProfileInt(INI_SECTION, "UserGoesFirst", 0, INI_FILENAME);
		m_bUsersTurn = FALSE;
		if (nVal != 0)
			m_bUsersTurn = TRUE;
	}
}


/*****************************************************************
*
*  SaveIniSettings
*
*  FUNCTIONAL DESCRIPTION:
*
*       Saves this game's parameters to it's .INI file
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::SaveIniSettings(VOID) {
}


/*****************************************************************
*
*  GameReset
*
*  FUNCTIONAL DESCRIPTION:
*
*       Resets all game parameters
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::GameReset(VOID) {
	CDC *pDC;
	INT i;

	//fred = 0;

	if (pGameParams->bSoundEffectsEnabled)
		sndPlaySound(NULL, SND_ASYNC);          // stop all sounds

	m_bGameActive = FALSE;                      // there is no active game

	m_bPause = FALSE;                           // the game is not paused

	m_bUserEditMode = FALSE;                    // user can't edit board

	m_bMovingFish = FALSE;                      // user is not moving fish

	if ((pDC = GetDC()) != NULL) {              // erase all sprites from the screen
		CSprite::EraseSprites(pDC);

		for (i = 0; i < MAX_FISH; i++) {        // remove fish from the sprite chain
			assert(m_pFish[i] != NULL);         // ...so they are not discarded
			m_pFish[i]->UnlinkSprite();

			assert(m_pEnemyFish[i] != NULL);
			if (m_pEnemyFish[i]->GetTypeCode()) {
				m_pEnemyFish[i]->UnlinkSprite();
				m_pEnemyFish[i]->SetTypeCode(FALSE);
			}

			if (m_pFish[i]->GetTypeCode()) {    // need to re-rotate this fish
				m_pFish[i]->SetTypeCode(FALSE);
				m_pFish[i]->LoadResourceSprite(pDC, IDB_FISH + i);
			}
		}

		// make sure we don't delete the octopus yet
		//
		if (m_pOctopus != NULL) {
			if (m_pOctopus->IsLinked()) {
				m_pOctopus->UnlinkSprite();
			}
		}

		CSprite::FlushSpriteChain();            // delete all linked sprites

		for (i = 0; i < MAX_FISH; i++) {        // Put the active fish back
			assert(m_pFish[i] != NULL);         // and reset them to bin coordinates
			//m_pFish[i]->SetPosition(ptFishBin[i]);
			//m_pFish[i]->LinkSprite();
		}
		ReleaseDC(pDC);
	}

	// reset the play grids
	//
	memset(m_nUserGrid, EMPTY, sizeof(UBYTE) * GRID_ROWS * GRID_COLS);
	memset(m_nEnemyGrid, EMPTY, sizeof(UBYTE) * GRID_ROWS * GRID_COLS);

	// make a copy of the fish sizes
	memcpy(&m_aUserFishInfo, &gFishSizes, sizeof(FISH) * MAX_FISH);
	memcpy(&m_aEnemyFishInfo, &gFishSizes, sizeof(FISH) * MAX_FISH);

	// reset the turn-harpoons
	memset(m_pHarpoons, 0, sizeof(CSprite *) * MAX_TURNS);

	m_nUserFish = m_nEnemyFish = MAX_FISH;      // set intial number of fish

	m_nTurns = 1;                               // User has first turn

	m_bStillCheck = FALSE;                      // Computer AI starts fresh
}


void CBFishWindow::RotateFish(INT nFishIndex) {
	CSize size;
	CRect rect, tmpRect;
	POINT point;
	CSprite *pSprite;
	CDC *pDC;
	INT nIDB;
	BOOL bRotated, bPaintFish;

	// validate the index
	assert((nFishIndex >= 0) && (nFishIndex < MAX_FISH));

	// must be in User-Edit Mode to rotate a fish
	assert(m_bUserEditMode == TRUE);

	// game can not yet be active
	assert(m_bGameActive == FALSE);

	pSprite = m_pFish[nFishIndex];

	// can't access a null pointer
	assert(pSprite != NULL);

	if ((pDC = GetDC()) != NULL) {

		// is this fish horizontal or vertical
		//
		bRotated = pSprite->GetTypeCode();
		nIDB = (bRotated ? IDB_FISH : IDB_FISHROT);

		// get fish size and location
		//
		point = pSprite->GetPosition();
		size = pSprite->GetSize();

		// Rotate fish during drag'n'drop
		//
		if (m_bMovingFish) {

			// play the rotate fish sound
			//
			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_ROTATESHIP, SND_ASYNC);

			// indicate that the fish is rotated
			//
			pSprite->SetTypeCode(!bRotated);

			// aquire the center of the fish
			point.x += size.cx / 2;
			point.y += size.cy / 2;

			// erase old fish
			pSprite->EraseSprite(pDC);

			// load BMP of new rotated fish
			pSprite->LoadResourceSprite(pDC, nIDB + nFishIndex);

			// re-aquire the upper-left corner of the fish
			//
			size = pSprite->GetSize();
			point.x -= size.cx / 2;
			point.y -= size.cy / 2;

			// paint the fish
			pSprite->PaintSprite(pDC, point);

			// must check to make sure we do not rotate this fish onto another
			//
		} else {

			// get rectangle of fish rotated 90 degrees
			// NOTE: (size.cx and size.cy are switched on purpose)
			//
			rect.SetRect(point.x, point.y, point.x + size.cy, point.y + size.cx);

			// assume we can rotate
			bPaintFish = TRUE;

			if (!OkToPlaceFish(nFishIndex, m_pFish[nFishIndex]->GetPosition(), (m_pFish[nFishIndex]->GetTypeCode() ? FALSE : TRUE))) {
				bPaintFish = FALSE;
			}

			// Ok, it is safe to rotate
			//
			if (bPaintFish) {

				// play the rotate fish sound
				//
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(WAV_ROTATESHIP, SND_ASYNC);

				// indicate that the fish is rotated
				//
				pSprite->SetTypeCode(!bRotated);

				PlaceFish(nFishIndex, pSprite->GetPosition());

				// erase old image
				pSprite->EraseSprite(pDC);

				// load new rotated fish
				pSprite->LoadResourceSprite(pDC, nIDB + nFishIndex);

				// paint him
				pSprite->PaintSprite(pDC, point);
			}
		}

		ReleaseDC(pDC);
	}
}

VOID CBFishWindow::AssignFishToGrid(INT nFishIndex) {
	CPoint point;
	INT i, nRow, nCol, nRowTmp, nColTmp;

	assert(nFishIndex >= 0 && nFishIndex < MAX_FISH);

	point = m_pFish[nFishIndex]->GetPosition();

	i = GetUserGridIndex(point);

	assert(i != -1);

	// calc row and column from grid index
	//
	nRow = i / GRID_ROWS;
	nCol = i % GRID_ROWS;

	for (i = 0; i < MAX_FISH_SIZE; i++) {

		// if there are no more squares for this fish, then done
		//
		if (m_aUserFishInfo[nFishIndex].nLoc[i].x == NONE)
			break;

		assert(m_aUserFishInfo[nFishIndex].nLoc[i].x < NONE);
		assert(m_aUserFishInfo[nFishIndex].nLoc[i].y < NONE);

		// if fish is rotated, then swap x and y
		//
		if (m_pFish[nFishIndex]->GetTypeCode()) {

			//nRowTmp = nRow + m_aUserFishInfo[nFishIndex].nLoc[i].y;
			//nColTmp = nCol + m_aUserFishInfo[nFishIndex].nLoc[i].x;

			nRowTmp = (m_aUserFishInfo[nFishIndex].nLoc[i].y += nRow);
			nColTmp = (m_aUserFishInfo[nFishIndex].nLoc[i].x += nCol);
		} else {

			//nRowTmp = nRow + m_aUserFishInfo[nFishIndex].nLoc[i].x;
			//nColTmp = nCol + m_aUserFishInfo[nFishIndex].nLoc[i].y;

			nRowTmp = (m_aUserFishInfo[nFishIndex].nLoc[i].x += nRow);
			nColTmp = (m_aUserFishInfo[nFishIndex].nLoc[i].y += nCol);
		}

		assert(nRowTmp >= 0 && nRowTmp < GRID_ROWS);
		assert(nColTmp >= 0 && nColTmp < GRID_COLS);

		// this square must be empty (or can contain parts of same fish)
		assert(m_nUserGrid[nRowTmp][nColTmp] == EMPTY || m_nUserGrid[nRowTmp][nColTmp] == (UBYTE)IndexToId(nFishIndex));

		m_nUserGrid[nRowTmp][nColTmp] = (UBYTE)IndexToId(nFishIndex);
	}
}


INT CBFishWindow::GetUserGridIndex(CPoint point) {
	INT i, j, iVal, jVal, nGridIndex;
	BOOL bEndLoop;

	iVal = -1;
	jVal = -1;
	bEndLoop = FALSE;
	for (i = 0; i < GRID_ROWS; i++) {
		for (j = 0; j < GRID_COLS; j++) {
			if ((point.x == gLeftGrid[i][j].x) && (point.y == gLeftGrid[i][j].y)) {
				assert(iVal == -1 && jVal == -1);
				bEndLoop = TRUE;
				iVal = i;
				jVal = j;
				break;
			}
		}
		if (bEndLoop)
			break;
	}

	nGridIndex = (iVal * GRID_ROWS) + jVal;

	// this point does notif we did not find the correct index
	if ((iVal == -1) && (jVal == -1))
		nGridIndex = -1;

	// this method will not work if GRID_ROWS or GRID_COLS is greater then 10

	return (nGridIndex);
}


INT CBFishWindow::GetFishIndex(CSprite *pSprite) {
	INT i, nIndex;

	assert(pSprite != NULL);

	nIndex = -1;
	for (i = 0; i < MAX_FISH; i++) {
		if (pSprite == m_pFish[i]) {
			nIndex = i;
			break;
		}
	}

	assert(nIndex != -1);

	return (nIndex);
}


/*****************************************************************
*
*  OnRButtonDown
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_RBUTTONDOWN messages
*
*  FORMAL PARAMETERS:
*
*       UINT nFlags  = Mouse button down flags
*       CPoint point = Point where the mouse was at time of this message
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::OnRButtonDown(UINT, CPoint point) {
	CRect tmpRect;
	INT i;

	if (m_bUserEditMode) {

		assert(m_bGameActive != TRUE);

		if (m_bMovingFish) {

			RotateFish(GetFishIndex(m_pDragFish));

		} else {

			for (i = 0; i < MAX_FISH; i++) {
				tmpRect = m_pFish[i]->GetRect();

				if (tmpRect.PtInRect(point)) {
					RotateFish(i);
					break;
				}
			}
		}
	}
}


/*****************************************************************
*
*  OnLButtonDown
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_LBUTTONDOWN messages
*
*  FORMAL PARAMETERS:
*
*       UINT nFlags  = Mouse button down flags
*       CPoint point = Point where the mouse was at time of this message
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::OnLButtonDown(UINT, CPoint point) {
	CRect   tmpRect,
	        winRect,
	        trawlerRect,
	        sailRect,
	        rowRect;
	CPoint  ptTmp;
	//CSound *pSound;
	CDC    *pDC;
	INT     i,
	        nPick;
	BOOL    bOkToPlay;

	tmpRect = m_pScrollSprite->GetRect();

	winRect.SetRect(WINDOW_X, WINDOW_Y, WINDOW_X + WINDOW_DX, WINDOW_Y + WINDOW_DY);
	trawlerRect.SetRect(TRAWLER_X, TRAWLER_Y, TRAWLER_X + TRAWLER_DX, TRAWLER_Y + TRAWLER_DY);
	sailRect.SetRect(SAILBOAT_X, SAILBOAT_Y, SAILBOAT_X + SAILBOAT_DX, SAILBOAT_Y + SAILBOAT_DY);
	rowRect.SetRect(ROWBOAT_X, ROWBOAT_Y, ROWBOAT_X + ROWBOAT_DX, ROWBOAT_Y + ROWBOAT_DY);

	if (tmpRect.PtInRect(point)) {

		if (!m_bMovingFish) {
			SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		}

		// User clicked on the Title - NewGame button
		//
	} else if (m_rNewGameButton.PtInRect(point)) {

		// if we are not playing from metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			// start a new game
			PlayGame();
		}

	} else if (winRect.PtInRect(point) && pGameParams->bSoundEffectsEnabled) {

		sndPlaySound(WAV_WINDOW, SND_ASYNC);

	} else if (trawlerRect.PtInRect(point) && pGameParams->bSoundEffectsEnabled) {

		sndPlaySound(WAV_TRAWLER, SND_ASYNC);

	} else if (sailRect.PtInRect(point) && pGameParams->bSoundEffectsEnabled) {

		sndPlaySound(WAV_SAILBOAT, SND_ASYNC);

	} else if (rowRect.PtInRect(point) && pGameParams->bSoundEffectsEnabled) {

		sndPlaySound(WAV_ROWBOAT, SND_ASYNC);

		// End User-Placement Mode
		//
	} else if (!m_bMovingFish && m_rEndPlacement.PtInRect(point)) {

		// there can be NO current fish being dragged
		assert(m_pDragFish == NULL);

		if (m_bUserEditMode) {

			// Check to make sure all fish are correctly placed
			//
			bOkToPlay = TRUE;
			for (i = 0; i < MAX_FISH; i++) {
				ptTmp = m_pFish[i]->GetPosition();
				if (ptTmp.x == ptFishBin[i].x && ptTmp.y == ptFishBin[i].y) {
					bOkToPlay = FALSE;
					break;
				}
			}

			if (bOkToPlay) {

				if ((pDC = GetDC()) != NULL) {
					if (m_pTxtClickHere != NULL) {
						m_pTxtClickHere->RestoreBackground(pDC);
						delete m_pTxtClickHere;
						m_pTxtClickHere = NULL;
					}
					ReleaseDC(pDC);
				}

				m_bUserEditMode = FALSE;
				m_bGameActive = TRUE;
				m_nTurns = m_nUserFish;


				// Convert each fish to grid locations
				//
				for (i = 0; i < MAX_FISH; i++) {
					AssignFishToGrid(i);
				}

				// select who will go first
				//
				if (!m_bUsersTurn) {

					// computer goes first
					//
					SendMessage(WM_COMMAND, COMPUTERS_TURN, BN_CLICKED);

					// player goes first
					//
				} else {

					if ((pDC = GetDC()) != NULL) {                                  // put up the octopus
						if (m_pOctopus != NULL) {
							m_pOctopus->LinkSprite();
							m_pOctopus->PaintSprite(pDC, OCTOPUS_X, OCTOPUS_Y);
						}
					}

					if (pGameParams->bSoundEffectsEnabled &&
					        ((m_nUserFish > VOICE_CUTOFF) && (m_nEnemyFish > VOICE_CUTOFF))) {             // have her say somethin' clever
						nPick = brand() % NUM_TURN_WAVS;
						if (nPick == 0) {
							#if CSOUND
							pSound = new CSound((CWnd *)this, WAV_YOURTURN1, SOUND_WAVE | SOUND_AUTODELETE);
							#else
							sndPlaySound(WAV_YOURTURN1, SND_SYNC);
							#endif
						} else {
							#if CSOUND
							pSound = new CSound((CWnd *)this, WAV_YOURTURN2, SOUND_WAVE | SOUND_AUTODELETE);
							#else
							sndPlaySound(WAV_YOURTURN2, SND_SYNC);
							#endif
						}
					}

					PlaceTurnHarpoons();

					if (pDC != NULL) {                                  // get her outta there
						if (m_pOctopus != NULL) {
							m_pOctopus->UnlinkSprite();
							m_pOctopus->EraseSprite(pDC);
						}
					}
					ReleaseDC(pDC);
				} // end if m_bUsersTurn
			}

		} else {

			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_INVALID, SND_ASYNC);
		}

	} else {

		if (m_bUserEditMode) {

			if (!m_bMovingFish) {

				for (i = 0; i < MAX_FISH; i++) {
					tmpRect = m_pFish[i]->GetRect();

					if (tmpRect.PtInRect(point)) {

						m_bMovingFish = TRUE;
						m_pDragFish = m_pFish[i];
						m_cLastPoint = m_pDragFish->GetPosition();
						m_bLastRotated = m_pDragFish->GetTypeCode();

						if ((pDC = GetDC()) != NULL) {

							// Dragged fish must be topmost
							//
							m_pDragFish->EraseSprite(pDC);
							m_pDragFish->SetZOrder(SPRITE_TOPMOST);
							m_pDragFish->PaintSprite(pDC, m_cLastPoint);
							ReleaseDC(pDC);
						}
						break;
					}
				}
			}

		} else if (m_bGameActive && !m_bPause) {

			if (m_bUsersTurn) {

				//
				// Determine if user clicked into enemy grid (to select target)
				// if point is a valid grid location then handle shot
				//
				UsersTurn(GetEnemyGridIndex(point));

				assert(m_nTurns >= 0);

				if (m_nTurns == 0) {
					// Computers turn to shoot
					//
					m_bUsersTurn = FALSE;
					SendMessage(WM_COMMAND, COMPUTERS_TURN, BN_CLICKED);
				}

			} else {

				// user clicked on an invalid location
				//
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(WAV_INVALID, SND_ASYNC);
			}
		}
	}
}


/*****************************************************************
*
*  OnLButtonUp
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_LBUTTONUP messages
*
*  FORMAL PARAMETERS:
*
*       UINT nFlags  = Mouse button down flags
*       CPoint point = Point where the mouse was at time of this message
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
void CBFishWindow::OnLButtonUp(UINT, CPoint point) {
	CSize size;
	CRect rect, tmpRect;
	CDC *pDC;
	INT i;
	BOOL bRevert;

	if (m_bUserEditMode && m_bMovingFish) {

		assert(m_bGameActive != TRUE);
		assert(m_pDragFish != NULL);

		m_bMovingFish = FALSE;

		size = m_pDragFish->GetSize();
		point.x -= size.cx / 2;
		point.y -= size.cy / 2;

		point = SnapToGrid(point);
		bRevert = FALSE;

		// test for valid fish placement
		//
		rect.SetRect(point.x, point.y, point.x + size.cx, point.y + size.cy);

		if (!OkToPlaceFish(GetFishIndex(m_pDragFish), point, m_pDragFish->GetTypeCode())) {
			point = m_cLastPoint;
			bRevert = TRUE;
		}

		// drop fish
		//
		if (pGameParams->bSoundEffectsEnabled)
			sndPlaySound(WAV_PLACESHIP, SND_ASYNC);

		if ((pDC = GetDC()) != NULL) {

			m_pDragFish->EraseSprite(pDC);

			if (bRevert) {

				// should we paint normal or rotated?
				i = (m_bLastRotated ? IDB_FISHROT : IDB_FISH);

				// re-load BMP of fish
				m_pDragFish->LoadResourceSprite(pDC, i + GetFishIndex(m_pDragFish));

				// FIX
				m_pDragFish->SetTypeCode(m_bLastRotated);
			} else {
				PlaceFish(GetFishIndex(m_pDragFish), point);
			}

			m_pDragFish->SetZOrder(SPRITE_BACKGROUND);
			m_pDragFish->PaintSprite(pDC, point);
			ReleaseDC(pDC);
		}
		m_pDragFish = NULL;
	}
}

BOOL CBFishWindow::OkToPlaceFish(INT nFishIndex, CPoint point, BOOL bRotated) {
	INT i, nRow, nCol, nGridIndex, nID;
	BOOL bOk;

	nID = IndexToId(nFishIndex);
	nGridIndex = GetUserGridIndex(point);

	// assume the fish will fit
	bOk = TRUE;

	// if point is not in the grid, then we can't put the fish here
	//
	if (nGridIndex == -1) {
		bOk = FALSE;

	} else {

		for (i = 0; i < MAX_FISH_SIZE; i++) {
			if (m_aUserFishInfo[nFishIndex].nLoc[i].x == NONE)
				break;

			// is this fish rotated?
			//
			if (bRotated) {
				nRow = (nGridIndex / GRID_ROWS) + m_aUserFishInfo[nFishIndex].nLoc[i].y;
				nCol = (nGridIndex % GRID_COLS) + m_aUserFishInfo[nFishIndex].nLoc[i].x;
			} else {
				nRow = (nGridIndex / GRID_ROWS) + m_aUserFishInfo[nFishIndex].nLoc[i].x;
				nCol = (nGridIndex % GRID_COLS) + m_aUserFishInfo[nFishIndex].nLoc[i].y;
			}

			// do not exceed grid boundary
			//
			if (nRow < 0 || nRow >= GRID_ROWS || nCol < 0 || nCol >= GRID_COLS) {
				bOk = FALSE;
				break;
			}

			// can't put the new fish into a non-empty square
			//
			if ((m_nUserGrid[nRow][nCol] != EMPTY) && (m_nUserGrid[nRow][nCol] != (UBYTE)nID)) {
				bOk = FALSE;
				break;
			}
		}
	}

	return (bOk);
}

VOID CBFishWindow::PlaceFish(INT nFishIndex, CPoint point) {
	INT i, nRow, nCol, nGridIndex, nID;

	nID = IndexToId(nFishIndex);
	nGridIndex = GetUserGridIndex(point);

	assert(nGridIndex != -1);

	for (i = 0; i < GRID_ROWS * GRID_COLS; i++) {

		nRow = i / GRID_ROWS;
		nCol = i % GRID_COLS;

		if (m_nUserGrid[nRow][nCol] & nID) {
			m_nUserGrid[nRow][nCol] = EMPTY;
		}
	}

	for (i = 0; i < MAX_FISH_SIZE; i++) {
		if (m_aUserFishInfo[nFishIndex].nLoc[i].x == NONE)
			break;

		// is this fish rotated?
		//
		if (m_pFish[nFishIndex]->GetTypeCode()) {
			nRow = (nGridIndex / GRID_ROWS) + m_aUserFishInfo[nFishIndex].nLoc[i].y;
			nCol = (nGridIndex % GRID_COLS) + m_aUserFishInfo[nFishIndex].nLoc[i].x;
		} else {
			nRow = (nGridIndex / GRID_ROWS) + m_aUserFishInfo[nFishIndex].nLoc[i].x;
			nCol = (nGridIndex % GRID_COLS) + m_aUserFishInfo[nFishIndex].nLoc[i].y;
		}

		m_nUserGrid[nRow][nCol] = (UBYTE)nID;
	}
}

CPoint CBFishWindow::SnapToGrid(CPoint point) {
	INT i, j;
	INT iMin, jMin;
	INT nVal, nMin;

	// Inits
	nMin = INT_MAX;
	iMin = -1;

	// find the closest point in the grid to the specified point
	//
	for (i = 0; i < GRID_ROWS; i++) {
		nVal = abs(point.y - gLeftGrid[i][0].y);

		// if this is the best so far, then store the index
		//
		if (nVal < nMin) {
			iMin = i;
			nMin = nVal;
		}
	}

	assert(iMin != -1);

	// Inits
	nMin = INT_MAX;
	jMin = -1;

	for (j = 0; j < GRID_COLS; j++) {
		nVal = abs(point.x - gLeftGrid[iMin][j].x);

		// if this is the best so far, then store the index
		//
		if (nVal < nMin) {
			jMin = j;
			nMin = nVal;
		}
	}

	assert(jMin != -1);

	point = gLeftGrid[iMin][jMin];

	return (point);
}

/*****************************************************************
*
*  OnMouseMove
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_MOUSEMOVE messages
*
*  FORMAL PARAMETERS:
*
*       UINT nFlags  = Mouse button down flags
*       CPoint point = Point where the mouse was at time of message
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::OnMouseMove(UINT, CPoint point) {
	CSize size;
	HCURSOR hCursor;
	CDC *pDC;

	if (m_bUserEditMode && m_bMovingFish) {

		hCursor = NULL;

		assert(m_pDragFish != NULL);

		if ((pDC = GetDC()) != NULL) {
			size = m_pDragFish->GetSize();
			point.x -= size.cx / 2;
			point.y -= size.cy / 2;
			m_pDragFish->PaintSprite(pDC, point);
			ReleaseDC(pDC);
		}
	} else {

		hCursor = LoadCursor(NULL, IDC_ARROW);
	}

	MFC::SetCursor(hCursor);
}


VOID CBFishWindow::UsersTurn(INT nGridIndex) {
	//CSound *pSound;
	INT nRow, nCol, nSquare;
	INT nFishIndex;
	//INT nPick = 0;

	// validate the grid index

	if ((nGridIndex >= 0) && (nGridIndex < GRID_ROWS * GRID_COLS)) {

		// Shoot at nGridIndex - calc row and column for this grid location
		//
		nRow = nGridIndex / GRID_ROWS;
		nCol = nGridIndex % GRID_ROWS;

		nSquare = m_nEnemyGrid[nRow][nCol];

		assert(nSquare <= (IndexToId(FISH3) | SHOT));

		// can't shoot same square twice
		//
		if (nSquare & SHOT) {

			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_INVALID, SND_ASYNC);

		} else {

			m_nTurns--;

			// take one harpoon off the game board
			RemoveTurnHarpoon();

			// shoot harpoon
			if (pGameParams->bSoundEffectsEnabled) {
				#if CSOUND
				pSound = new CSound((CWnd *)this, WAV_SHOOT, SOUND_WAVE | SOUND_AUTODELETE);
				#else
				sndPlaySound(WAV_SHOOT, SND_SYNC);
				#endif
			}

			//
			// check to see if this location contains an enemy fish
			//
			if (nSquare != EMPTY) {

				// play the you hit sound
				if (pGameParams->bSoundEffectsEnabled) {
					#if CSOUND
					pSound = new CSound((CWnd *)this, WAV_YOUHIT, SOUND_WAVE | SOUND_AUTODELETE);
					#else
					sndPlaySound(WAV_YOUHIT, SND_ASYNC);
					#endif
				}

				// get index to fish
				nFishIndex = IdToIndex(nSquare);

				assert(nFishIndex >= FISH0 && nFishIndex <= FISH3);

				//
				// Put a Harpoon here
				//
				CreateHarpoon(gRightGrid[nRow][nCol]);

				assert(m_aEnemyFishInfo[nFishIndex].life > 0);

				//
				// indicate that this fish has one less life
				//
				if (--m_aEnemyFishInfo[nFishIndex].life == 0) {

					if (pGameParams->bSoundEffectsEnabled) {

						sndPlaySound(pszFishSound[nFishIndex], SND_SYNC);
					}

					SinkEnemyFish(nFishIndex);

					// one less fish
					//
					assert(m_nEnemyFish > 0);
					if (--m_nEnemyFish == 0) {

						GamePause();

						// Display any extra animation
						//

						// User Wins
						//
						if (pGameParams->bSoundEffectsEnabled)
							sndPlaySound(WAV_YOUWIN, SND_ASYNC);

						CMessageBox(this, m_pGamePalette, "Game over.", "You win!");

						GameReset();

						if (pGameParams->bPlayingMetagame) {
							pGameParams->lScore = 1;
							PostMessage(WM_CLOSE, 0, 0);
						}
					}
				}

			} else {

				// play the you missed sound
				if (pGameParams->bSoundEffectsEnabled) {
					sndPlaySound(WAV_YOUMISS, SND_ASYNC);
				}

				//
				// square was empty, so put a plume of water here
				//
				CreatePlume(gRightGrid[nRow][nCol]);
			}

			// indicate that this sqaure has been shot
			m_nEnemyGrid[nRow][nCol] |= SHOT;
		}
	} else {

		// invalid click
		//
		if (pGameParams->bSoundEffectsEnabled)
			sndPlaySound(WAV_INVALID, SND_ASYNC);
	}
}

VOID CBFishWindow::SinkEnemyFish(INT nFishIndex) {
	CSprite *pSprite;
	CDC *pDC;

	// display fish on hook

	// validate the input
	assert(nFishIndex >= 0 && nFishIndex < MAX_FISH);

	pSprite = m_pEnemyFish[nFishIndex];

	assert(pSprite != NULL);

	pSprite->SetTypeCode(TRUE);
	pSprite->LinkSprite();

	if ((pDC = GetDC()) != NULL) {
		pSprite->PaintSprite(pDC, pSprite->GetPosition());
		ReleaseDC(pDC);
	}
}


VOID CBFishWindow::ComputersTurn(VOID) {
	STATIC INT nLastRow, nLastCol;
	//CSound *pSound;
	INT nRow, nCol, nFishIndex, nGridIndex;
	INT nSquare;

	if (m_bGameActive && !m_bPause) {

		//
		// Perform some AI to find the next best target
		//
		nGridIndex = FindTarget(nLastRow, nLastCol);
		nRow = nGridIndex / GRID_ROWS;
		nCol = nGridIndex % GRID_COLS;

		nSquare = m_nUserGrid[nRow][nCol];

		// this square could not have been shot before
		assert((m_nUserGrid[nRow][nCol] & SHOT) != SHOT);

		// indicate that this square has now been shot
		m_nUserGrid[nRow][nCol] |= SHOT;

		// shoot harpoon
		//
		if (pGameParams->bSoundEffectsEnabled) {
			#if CSOUND
			pSound = new CSound((CWnd *)this, WAV_SHOOT, SOUND_WAVE | SOUND_AUTODELETE);
			#else
			sndPlaySound(WAV_SHOOT, SND_SYNC);
			#endif
		}

		//
		// check to see if this location contains an enemy fish
		//
		if (nSquare != EMPTY) {

			// play the you hit sound (harpoon)
			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_YOUHIT, SND_ASYNC);

			// get index to fish
			nFishIndex = IdToIndex(nSquare);

			if ((m_bStillCheck == FALSE) || (m_nDifficultyLevel == DIFF_AVERAGE)) {
				m_bStillCheck = TRUE;
				nLastRow = nRow;
				nLastCol = nCol;
			}

			//
			// Put a Harpoon here
			//
			CreateHarpoon(gLeftGrid[nRow][nCol]);

			//
			// indicate that this fish has one less life
			//

			assert(m_aUserFishInfo[nFishIndex].life > 0);

			//
			// indicate that this fish has one less life
			//
			if (--m_aUserFishInfo[nFishIndex].life == 0) {

				m_bStillCheck = FALSE;

				assert(m_nUserFish > 0);

				if (pGameParams->bSoundEffectsEnabled) {
					#if CSOUND
					pSound = new CSound((CWnd *)this, WAV_YOUSINK, SOUND_WAVE | SOUND_AUTODELETE);
					#else
					sndPlaySound(WAV_YOUSINK, SND_SYNC);
					#endif
				}

				// for DiffLevel2 (Computer AI), erase trace of fish from board
				//
				SinkUserFish(nFishIndex);

				// one less fish
				//
				if (--m_nUserFish == 0) {

					GamePause();

					// Display any extra animation
					//

					// User Lost
					//
					if (pGameParams->bSoundEffectsEnabled)
						sndPlaySound(WAV_GAMEOVER, SND_SYNC);

					CMessageBox(this, m_pGamePalette, "Game over.", "You have lost!");

					GameReset();

					if (pGameParams->bPlayingMetagame) {
						PostMessage(WM_CLOSE, 0, 0);
						PostMessage(WM_CLOSE, 0, 0);
					}
				}
			}

		} else {

			// play the "you missed" sound (splash of water)
			//
			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_YOUMISS, SND_ASYNC);

			//
			// square was empty, so put a plume of water here
			//
			CreatePlume(gLeftGrid[nRow][nCol]);
		}
	}
}


VOID CBFishWindow::SinkUserFish(INT nFishIndex) {
	INT i, nRow, nCol;

	for (i = 0; i < GRID_ROWS * GRID_COLS; i++) {

		nRow = i / GRID_ROWS;
		nCol = i % GRID_COLS;

		if (m_nUserGrid[nRow][nCol] & IndexToId(nFishIndex)) {
			m_nUserGrid[nRow][nCol] = (EMPTY | SHOT);
		}
	}
}


INT CBFishWindow::SelectRandomTarget(VOID) {
	INT  n;
	INT nRow, nCol;

	//
	// choose a random target, but favor targets
	// with maximum number of empty neighbors
	//
	n = 0;
	do {
		nRow = brand() % GRID_ROWS;
		nCol = brand() % GRID_COLS;

		if (m_nUserGrid[nRow][nCol] & SHOT)
			continue;

		n = GetNeighbors(nRow, nCol);

	} while (n == 0);

	assert(nRow >= 0 && nRow < GRID_ROWS);
	assert(nCol >= 0 && nCol < GRID_COLS);

	return ((nRow * GRID_ROWS) + nCol);
}


INT CBFishWindow::SelectBurningTarget(VOID) {
	INT i, nGridIndex, nRow, nCol;
	BOOL bFound;

	nRow = nCol = 0;

	// try to re-aquire a fish we have already damaged, but has not yet sank
	//
	bFound = FALSE;
	for (i = 0; i < GRID_ROWS * GRID_COLS; i++) {
		nRow = i / GRID_ROWS;
		nCol = i % GRID_COLS;

		if ((m_nUserGrid[nRow][nCol] & SHOT) && (m_nUserGrid[nRow][nCol] != (EMPTY | SHOT))) {
			bFound = TRUE;
			break;
		}
	}

	nGridIndex = (nRow * GRID_ROWS) + nCol;

	// if there are no such damaged fish, then just select a random target
	//
	if (!bFound)
		nGridIndex = SelectBestFitTarget();

	return (nGridIndex);
}

INT CBFishWindow::SelectBestFitTarget(VOID) {
	INT nRow, nCol, nFishIndex, nGridIndex;
	INT counter, i;

	nFishIndex = MAX_FISH;
	while (m_aUserFishInfo[--nFishIndex].life == 0) {
		assert(nFishIndex >= 0);
	}

	i = counter = 0;
	do {
		if (counter < 1000) {

			nGridIndex = SelectRandomTarget();
			assert((nGridIndex >= 0) && (nGridIndex < (GRID_ROWS * GRID_COLS)));
			nRow = nGridIndex / GRID_ROWS;
			nCol = nGridIndex % GRID_COLS;

			i = 0;

			counter++;

			// we have checked too many random locations
		} else {

			// now start at (0,0) and check until (7,7)

			assert((i >= 0) && (i < (GRID_ROWS * GRID_COLS)));

			nRow = i / GRID_ROWS;
			nCol = i % GRID_COLS;
			nGridIndex = i;
			i++;
		}

	} while (!FishFits(nFishIndex, nRow, nCol));

	return (nGridIndex);
}

BOOL CBFishWindow::FishFits(INT nFishIndex, INT row, INT col) {
	FISH cFishInfo;
	INT nRow, nCol, colTmp, rowTmp;
	INT i, j, k, rotate;
	BOOL bFound;

	// Try fish at both 0 and 90 degrees
	//
	bFound = FALSE;
	for (i = 0; i < 2; i++) {

		rotate = brand() & 1;
		for (j = 0; j < MAX_FISH_SIZE; j++) {

			// make a fresh copy of this fish
			memcpy(&cFishInfo, &gFishSizes[nFishIndex], sizeof(FISH));

			//
			// Fish didn't fit the first time thru, so try rotating
			// it by swapping row and column for each square in
			// fish.
			//
			if (i == rotate) {
				for (k = 0; k < MAX_FISH_SIZE; k++) {
					cFishInfo.nLoc[k].x = gFishSizes[nFishIndex].nLoc[k].y;
					cFishInfo.nLoc[k].y = gFishSizes[nFishIndex].nLoc[k].x;
				}
			}

			nRow = row - cFishInfo.nLoc[j].x;
			nCol = col - cFishInfo.nLoc[j].y;

			if ((nRow >= 0) && (nCol >= 0)) {

				// Does the fish fit at this location
				//
				bFound = TRUE;
				for (k = 0; k < MAX_FISH_SIZE; k++) {

					// if there are no more squares for this fish, then done
					//
					if (cFishInfo.nLoc[k].x == NONE)
						break;

					assert(cFishInfo.nLoc[k].y != NONE);

					rowTmp = (cFishInfo.nLoc[k].x += nRow);
					colTmp = (cFishInfo.nLoc[k].y += nCol);

					if ((rowTmp >= GRID_ROWS) || (colTmp >= GRID_COLS) || (m_nUserGrid[rowTmp][colTmp] == (EMPTY | SHOT))) {
						bFound = FALSE;
						break;
					}
				}
			}
		}
		if (bFound)
			break;
	}

	return (bFound);
}


INT CBFishWindow::GetNeighbors(INT nRow, INT nCol) {
	INT n;

	// validate the input
	//
	assert(nRow >= 0 && nRow < GRID_ROWS);
	assert(nCol >= 0 && nCol < GRID_COLS);

	// start with NO neighbors
	n = 0;

	// if specified square has an empty square to it's left then we have
	// a neighbor
	//
	if ((nRow + 1 >= GRID_ROWS) || (m_nUserGrid[nRow + 1][nCol] != (EMPTY | SHOT))) {
		n++;
		if (isodd(m_nUserGrid[nRow + 1][nCol]))
			n++;
	}

	// if specified square has an empty square to it's right then we have
	// another neighbor
	//
	if ((nRow - 1 < 0) || (m_nUserGrid[nRow - 1][nCol] != (EMPTY | SHOT))) {
		n++;
		if (isodd(m_nUserGrid[nRow - 1][nCol]))
			n++;
	}

	// if specified square has an empty square to it's bottom then we have
	// another neighbor
	//
	if ((nCol + 1 >= GRID_COLS) || (m_nUserGrid[nRow][nCol + 1] != (EMPTY | SHOT))) {
		n++;
		if (isodd(m_nUserGrid[nRow][nCol + 1]))
			n++;
	}

	// if specified square has an empty square to it's top then we have
	// another neighbor
	//
	if ((nCol - 1 < 0) || (m_nUserGrid[nRow][nCol - 1] != (EMPTY | SHOT))) {
		n++;
		if (isodd(m_nUserGrid[nRow][nCol - 1]))
			n++;
	}

	// can have no more than 4 neighbors, and no less than 0
	assert(n >= 0 && n <= 8);

	// return number of neighbors found for this square
	return (n);
}


INT CBFishWindow::FindTarget(INT nLastHitRow, INT nLastHitCol) {
	INT nGridIndex;

	assert(nLastHitRow >= 0 && nLastHitRow < GRID_ROWS);
	assert(nLastHitCol >= 0 && nLastHitCol < GRID_COLS);

	nGridIndex = -1;
	switch (m_nDifficultyLevel) {

	//
	// Easiest level: select random targets
	//
	case DIFF_WIMPY:
		nGridIndex = SelectRandomTarget();
		break;

	//
	// Medium level: shoot one of the surrounding squares of our last hit
	//
	case DIFF_AVERAGE:
		if (m_bStillCheck) {
			nGridIndex = FindNeighborTarget(nLastHitRow, nLastHitCol);
		} else {
			nGridIndex = SelectRandomTarget();
		}
		break;

	//
	// toughest level: uses pattern recognition to determine where
	// fish would best fit
	//
	default:
		assert(m_nDifficultyLevel == DIFF_HEFTY);

		nGridIndex = FindMatch(nLastHitRow, nLastHitCol);
		break;
	}

	// Grid Index can only be 0..63
	assert((nGridIndex >= 0) && (nGridIndex < (GRID_ROWS * GRID_COLS)));

	return (nGridIndex);
}


INT CBFishWindow::FindNeighborTarget(INT nLastHitRow, INT nLastHitCol) {
	INT nRow, nCol, nGridIndex;
	INT nState, nDisplacement;

	nState = 0;
	nRow = nCol = 0;

	assert(nLastHitRow >= 0 && nLastHitRow < GRID_ROWS);
	assert(nLastHitCol >= 0 && nLastHitCol < GRID_COLS);

	nDisplacement = 1;
	do {
		switch (nState) {

		case 0:
			nRow = nLastHitRow + nDisplacement;
			nCol = nLastHitCol;
			break;

		case 1:
			nRow = nLastHitRow - nDisplacement;
			nCol = nLastHitCol;
			break;

		case 2:
			nRow = nLastHitRow;
			nCol = nLastHitCol + nDisplacement;
			break;

		case 3:
			nRow = nLastHitRow;
			nCol = nLastHitCol - nDisplacement;
			break;

		case 4:
			nRow = nLastHitRow + nDisplacement;
			nCol = nLastHitCol + nDisplacement;
			break;

		case 5:
			nRow = nLastHitRow - nDisplacement;
			nCol = nLastHitCol - nDisplacement;
			break;

		case 6:
			nRow = nLastHitRow - nDisplacement;
			nCol = nLastHitCol + nDisplacement;
			break;

		case 7:
			nRow = nLastHitRow + nDisplacement;
			nCol = nLastHitCol - nDisplacement;
			break;

		default:

			if (nDisplacement == 1) {
				nDisplacement++;
				nState = -1;
			} else {
				nState--;

				nGridIndex = SelectRandomTarget();

				nRow = nGridIndex / GRID_ROWS;
				nCol = nGridIndex % GRID_COLS;
			}
			break;
		}

		if (nRow < 0 || nRow >= GRID_ROWS)
			nRow = nLastHitRow;

		if (nCol < 0 || nCol >= GRID_COLS)
			nCol = nLastHitCol;

		nState++;

	} while (m_nUserGrid[nRow][nCol] & SHOT);

	return ((nRow * GRID_ROWS) + nCol);
}

#if 1

INT CBFishWindow::FindMatch(INT nLastHitRow, INT nLastHitCol) {
	FISH cFishInfo;
	INT i, j, k, l, rotate;
	INT nRow, nCol, nBestRow, nBestCol;
	INT n, nLast;
	INT row, col, rowTmp, colTmp, nGridIndex;
	INT nUseRow, nUseCol;
	INT nHits, nBestHits;
	BOOL bFound, bUse;

	rowTmp = colTmp = 0;
	nBestRow = nBestCol = 0;
	nUseRow = nUseCol = 0;
	nBestHits = -1;
	nHits = 0;

	// validate the input
	//
	assert(nLastHitRow >= 0 && nLastHitRow < GRID_ROWS);
	assert(nLastHitCol >= 0 && nLastHitCol < GRID_COLS);

	if (m_bStillCheck) {
		row = nLastHitRow;
		col = nLastHitCol;
	} else {

		// select random starting square
		//
		nGridIndex = SelectBurningTarget();

		row = nGridIndex / GRID_ROWS;
		col = nGridIndex % GRID_COLS;
	}

	// Try to match a fish pattern onto the grid
	//
	bUse = FALSE;
	bFound = FALSE;
	for (i = MAX_FISH - 1; i >= 0; i--) {

		bFound = FALSE;
		if (m_aUserFishInfo[i].life != 0) {

			// Try fish at both 0 and 90 degrees
			//
			rotate = brand() & 1;
			for (j = 0; j < 2; j++) {

				bFound = FALSE;
				for (l = 0; l < MAX_FISH_SIZE; l++) {

					// make a fresh copy of this fish
					memcpy(&cFishInfo, &gFishSizes[i], sizeof(FISH));

					//
					// Fish didn't fit the first time thru, so try rotating
					// it by swapping row and column for each square in
					// fish.
					//
					if (j == rotate) {
						for (k = 0; k < MAX_FISH_SIZE; k++) {
							cFishInfo.nLoc[k].x = gFishSizes[i].nLoc[k].y;
							cFishInfo.nLoc[k].y = gFishSizes[i].nLoc[k].x;
						}
					}

					nRow = row - cFishInfo.nLoc[l].x;
					nCol = col - cFishInfo.nLoc[l].y;

					if ((nRow >= 0) && (nCol >= 0)) {

						// Does the fish fit at this location
						//
						bFound = TRUE;
						for (k = 0; k < MAX_FISH_SIZE; k++) {

							// if there are no more squares for this fish, then done
							//
							if (cFishInfo.nLoc[k].x == NONE)
								break;

							assert(cFishInfo.nLoc[k].y != NONE);

							rowTmp = (cFishInfo.nLoc[k].x += nRow);
							colTmp = (cFishInfo.nLoc[k].y += nCol);

							if ((rowTmp < 0) || (rowTmp >= GRID_ROWS) || (colTmp < 0) || (colTmp >= GRID_COLS) || (m_nUserGrid[rowTmp][colTmp] == (EMPTY | SHOT))) {
								bFound = FALSE;
								break;
							}
						}

						// Check to see if we can shoot one of these spots
						//
						if (bFound) {
							bFound = FALSE;
							nLast = -1;
							nBestRow = row;
							nBestCol = col;
							nHits = 0;
							for (k = 0; k < MAX_FISH_SIZE; k++) {

								// if there are no more squares for this fish, then done
								//
								if (cFishInfo.nLoc[k].x == NONE)
									break;

								assert(cFishInfo.nLoc[k].y < NONE);

								rowTmp = cFishInfo.nLoc[k].x;
								colTmp = cFishInfo.nLoc[k].y;

								if ((m_nUserGrid[rowTmp][colTmp] & SHOT) == EMPTY) {

									n = GetNeighbors(rowTmp, colTmp);

									if (n > nLast) {

										nBestRow = rowTmp;
										nBestCol = colTmp;
										nLast = n;
									}
									bFound = TRUE;
								} else if (m_nUserGrid[rowTmp][colTmp] & IndexToId(i)) {
									nHits++;
								}
							}

							//TRACE("Fish %d at (%d, %d) has %d hits\n", i, nBestRow, nBestCol, nHits);
						}


						// the fish fit - so set the grid and then go on to next fish
						//
						if (bFound) {

							if (nHits > nBestHits) {

								nBestHits = nHits;
								nUseRow = rowTmp = nBestRow;
								nUseCol = colTmp = nBestCol;
								bUse = TRUE;
							}
						}
					}
				}
			}
		}
	}

	//TRACE("Chose (%d, %d) with %d hits\n", nUseRow, nUseCol, nBestHits);
	rowTmp = nUseRow;
	colTmp = nUseCol;

	assert(bUse);

	assert(rowTmp >= 0 && rowTmp < GRID_ROWS);
	assert(colTmp >= 0 && colTmp < GRID_COLS);

	// this square could not have already been shot
	assert((m_nUserGrid[rowTmp][colTmp] & SHOT) == EMPTY);

	return ((rowTmp * GRID_ROWS) + colTmp);
}
#else

INT CBFishWindow::FindMatch(INT nLastHitRow, INT nLastHitCol) {
	FISH cFishInfo;
	INT i, j, k, l, rotate;
	INT nRow, nCol, nBestRow, nBestCol;
	INT n, nLast;
	INT row, col, rowTmp, colTmp, nGridIndex;
	BOOL bFound;

	rowTmp = colTmp = 0;
	nBestRow = nBestCol = 0;

	// validate the input
	//
	assert(nLastHitRow >= 0 && nLastHitRow < GRID_ROWS);
	assert(nLastHitCol >= 0 && nLastHitCol < GRID_COLS);

	if (m_bStillCheck) {
		row = nLastHitRow;
		col = nLastHitCol;
	} else {

		// select random starting square
		//
		nGridIndex = SelectBurningTarget();

		//nGridIndex = bob[fred++];

		row = nGridIndex / GRID_ROWS;
		col = nGridIndex % GRID_COLS;
	}

	// Try to match a fish pattern onto the grid
	//
	bFound = FALSE;
	for (i = MAX_FISH - 1; i >= 0; i--) {

		bFound = FALSE;
		if (m_aUserFishInfo[i].life != 0) {

			// Try fish at both 0 and 90 degrees
			//
			rotate = brand() & 1;
			for (j = 0; j < 2; j++) {

				bFound = FALSE;
				for (l = 0; l < MAX_FISH_SIZE; l++) {

					// make a fresh copy of this fish
					memcpy(&cFishInfo, &gFishSizes[i], sizeof(FISH));

					//
					// Fish didn't fit the first time thru, so try rotating
					// it by swapping row and column for each square in
					// fish.
					//
					if (j == rotate) {
						for (k = 0; k < MAX_FISH_SIZE; k++) {
							cFishInfo.nLoc[k].x = gFishSizes[i].nLoc[k].y;
							cFishInfo.nLoc[k].y = gFishSizes[i].nLoc[k].x;
						}
					}

					nRow = row - cFishInfo.nLoc[l].x;
					nCol = col - cFishInfo.nLoc[l].y;

					if ((nRow >= 0) && (nCol >= 0)) {

						// Does the fish fit at this location
						//
						bFound = TRUE;
						for (k = 0; k < MAX_FISH_SIZE; k++) {

							// if there are no more squares for this fish, then done
							//
							if (cFishInfo.nLoc[k].x == NONE)
								break;

							assert(cFishInfo.nLoc[k].y != NONE);

							rowTmp = (cFishInfo.nLoc[k].x += nRow);
							colTmp = (cFishInfo.nLoc[k].y += nCol);

							if ((rowTmp < 0) || (rowTmp >= GRID_ROWS) || (colTmp < 0) || (colTmp >= GRID_COLS) || (m_nUserGrid[rowTmp][colTmp] == (EMPTY | SHOT))) {
								bFound = FALSE;
								break;
							}
						}

						// Check to see if we can shoot one of these spots
						//
						if (bFound) {
							bFound = FALSE;
							nLast = -1;
							nBestRow = row;
							nBestCol = col;
							for (k = 0; k < MAX_FISH_SIZE; k++) {

								// if there are no more squares for this fish, then done
								//
								if (cFishInfo.nLoc[k].x == NONE)
									break;

								assert(cFishInfo.nLoc[k].y < NONE);

								rowTmp = cFishInfo.nLoc[k].x;
								colTmp = cFishInfo.nLoc[k].y;

								if ((m_nUserGrid[rowTmp][colTmp] & SHOT) == EMPTY) {

									n = GetNeighbors(rowTmp, colTmp);

									if (n > nLast) {
										nBestRow = rowTmp;
										nBestCol = colTmp;
										nLast = n;
									}
									bFound = TRUE;
								}
							}
							if (bFound) {
								rowTmp = nBestRow;
								colTmp = nBestCol;
								break;
							}
						}

						// the fish fit - so set the grid and then go on to next fish
						//
						if (bFound) {

							rowTmp = nBestRow;
							colTmp = nBestCol;
							break;
						}
					}
				}
				if (bFound)
					break;
			}
		}
		if (bFound)
			break;
	}

	assert(bFound);

	if (!bFound) {
		assert(m_bStillCheck == FALSE);
		rowTmp = row;
		colTmp = col;
	}

	assert(rowTmp >= 0 && rowTmp < GRID_ROWS);
	assert(colTmp >= 0 && colTmp < GRID_COLS);

	// this square could not have already been shot
	assert((m_nUserGrid[rowTmp][colTmp] & SHOT) == EMPTY);

	return ((rowTmp * GRID_ROWS) + colTmp);
}
#endif


VOID CBFishWindow::CreatePlume(CPoint point) {
	CDC *pDC;
	CSprite *pSprite;
	INT i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != NULL) {

		// Play a plume of water animation sequence inline
		//
		if ((pSprite = new CSprite) != NULL) {

			// attach good guy to the Game Palette
			//
			if (pSprite->SharePalette(m_pGamePalette) != FALSE) {

				if (pSprite->LoadResourceSprite(pDC, IDB_PLUME) != FALSE) {

					pSprite->LoadResourceCels(pDC, IDB_APLUME, N_PLUME_CELS);

					pSprite->SetMasked(TRUE);
					pSprite->SetMobile(TRUE);

					for (i = 0; i < N_PLUME_CELS; i++) {
						pSprite->PaintSprite(pDC, point);
						Sleep(100);
					}
				} else {
					errCode = ERR_UNKNOWN;
				}
			} else {
				errCode = ERR_UNKNOWN;
			}

			pSprite->EraseSprite(pDC);
			delete pSprite;

			// create a dup of the master plume of water
			//
			if ((pSprite = m_pMasterMiss->DuplicateSprite(pDC)) != NULL) {
				pSprite->LinkSprite();
				pSprite->PaintSprite(pDC, point);

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

	HandleError(errCode);
}


VOID CBFishWindow::CreateHarpoon(CPoint point) {
	CDC *pDC;
	CSprite *pSprite;
	INT i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != NULL) {

		// Play a harpoon hitting fish animation sequence inline
		//
		if ((pSprite = new CSprite) != NULL) {

			// attach good guy to the Game Palette
			//
			if (pSprite->SharePalette(m_pGamePalette) != FALSE) {

				if (pSprite->LoadResourceSprite(pDC, IDB_HARP) != FALSE) {

					pSprite->LoadResourceCels(pDC, IDB_AHARP, N_HARP_CELS);

					pSprite->SetMasked(TRUE);
					pSprite->SetMobile(TRUE);

					for (i = 0; i < N_HARP_CELS; i++) {
						pSprite->PaintSprite(pDC, point);
						Sleep(100);
					}
				} else {
					errCode = ERR_UNKNOWN;
				}
			} else {
				errCode = ERR_UNKNOWN;
			}

			pSprite->EraseSprite(pDC);
			delete pSprite;

			// create a dup of the master harpoon
			//
			if ((pSprite = m_pMasterHit->DuplicateSprite(pDC)) != NULL) {
				pSprite->LinkSprite();
				pSprite->PaintSprite(pDC, point);

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

	HandleError(errCode);
}


INT CBFishWindow::IndexToId(INT nFishIndex) {
	return (2 << nFishIndex);
}


INT CBFishWindow::IdToIndex(INT nId) {
	INT i;

	assert(iseven(nId));

	i = 0;
	while (nId > 2) {
		nId = nId >> 1;
		i++;
	}

	return (i);
}


INT CBFishWindow::GetEnemyGridIndex(CPoint point) {
	CRect rect;
	INT nIndex;
	INT i, j;
	INT iVal, jVal;

	nIndex = -1;
	rect.SetRect(RGRID_MIN_X, RGRID_MIN_Y, RGRID_MAX_X, RGRID_MAX_Y);

	if (rect.PtInRect(point)) {

		// determine if the given point is in one of the grid sqaures
		//
		iVal = -1;
		for (i = GRID_ROWS - 1; i >= 0; i--) {

			if (point.y >= gRightGrid[i][0].y) {
				iVal = i;
				break;
			}
		}

		if (iVal != -1) {

			jVal = -1;
			for (j = GRID_COLS - 1; j >= 0; j--) {
				if (point.x >= gRightGrid[iVal][j].x) {
					jVal = j;
					break;
				}
			}

			if (jVal != -1)
				nIndex = (iVal * GRID_ROWS) + jVal;
		}
	}

	return (nIndex);
}


/*****************************************************************
*
*  DeleteSprite
*
*  FUNCTIONAL DESCRIPTION:
*
*       Erases, Unlinks, and Deletes specified sprite
*
*  FORMAL PARAMETERS:
*
*       CSprite *pSprite = Pointer to sprite that is to be deleted
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::DeleteSprite(CSprite *pSprite) {
	CDC *pDC;

	// can't delete a null pointer
	assert(pSprite != NULL);

	if (pSprite != NULL) {

		if ((pDC = GetDC()) != NULL) {
			pSprite->EraseSprite(pDC);              // erase it from screen
			ReleaseDC(pDC);
		}
		pSprite->UnlinkSprite();                    // unlink it

		delete pSprite;                             // delete it
	}
}


/*****************************************************************
*
*  OnSysChar
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_SYSCHAR messages
*
*  FORMAL PARAMETERS:
*
*       UINT nChar   = key that was pressed
*       UINT nRepCnt = nunmber of times key was repeated
*       UINT nFlags  = ALT, CTRL, and SHFT key flags
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
void CBFishWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// terminate app on ALT_Q
	//
	if ((nChar == 'q') && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CFrameWnd ::OnSysChar(nChar, nRepCnt, nFlags);
	}
}

/*****************************************************************
*
*  OnSysKeyDown
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_SYSKEYDOWN messages
*
*  FORMAL PARAMETERS:
*
*       UINT nChar   = key that was pressed
*       UINT nRepCnt = nunmber of times key was repeated
*       UINT nFlags  = ALT, CTRL, and SHFT key flags
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
void CBFishWindow::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
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


/*****************************************************************
*
*  OnKeyDown
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_KEYDOWN messages
*
*  FORMAL PARAMETERS:
*
*       UINT nChar   = key that was pressed
*       UINT nRepCnt = nunmber of times key was repeated
*       UINT nFlags  = ALT, CTRL, and SHFT key flags
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
void CBFishWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// Handle keyboard input
	//
	switch (nChar) {

	//
	// Bring up the Rules
	//
	case VK_F1: {
		GamePause();
		CSound::waitWaveSounds();
		CRules  RulesDlg(this, "bfish.txt", m_pGamePalette, (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : NULL));
		PaintScreen();
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

		#ifdef DEBUG
	case VK_F3:

		INT i, j;

		//
		// Print user grid to show where fish were placed
		//
		TRACE("User Grid:\n");
		for (i = 0; i < GRID_ROWS; i++) {
			for (j = 0; j < GRID_COLS; j++) {
				TRACE("%02d ", m_nUserGrid[i][j]);
			}
			TRACE("\n");
		}
		TRACE("\n");

		//
		// Print enemy grid to show where fish were placed
		//
		TRACE("Enemy Grid:\n");
		for (i = 0; i < GRID_ROWS; i++) {
			for (j = 0; j < GRID_COLS; j++) {
				TRACE("%02d ", m_nEnemyGrid[i][j]);
			}
			TRACE("\n");
		}
		TRACE("\n");
		break;
		#endif

	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}


/*****************************************************************
*
*  OnActivate
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_ACTIVATE messages
*
*  FORMAL PARAMETERS:
*
*       UINT nState = WA_ACTIVE, WA_CLICKACTIVE or WA_INACTIVE
*       CWnd *pWnd  = Pointer to Window that is losing/gaining activation
*       BOOL bMin   = TRUE if this app is minimized
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
void CBFishWindow::OnActivate(UINT nState, CWnd *, BOOL bMinimized) {
	if (!bMinimized) {

		switch (nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			//InvalidateRect(NULL, FALSE);
			break;

		case WA_INACTIVE:
			break;

		default:
			break;
		}
	}
}

VOID CBFishWindow::FlushInputEvents(VOID) {
	MSG msg;

	// find and remove all keyboard events
	//
	while (TRUE) {
		if (!PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	// find and remove all mouse events
	//
	while (TRUE) {
		if (!PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}


/*****************************************************************
*
*  OnClose
*
*  FUNCTIONAL DESCRIPTION:
*
*       Handles WM_CLOSE messages
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CBFishWindow::OnClose() {
	CBrush myBrush;
	CRect rMyRect;
	CDC *pDC;

	// perform cleanup
	//
	GameReset();

	// delete the game theme song
	//
	if (m_pSoundTrack != NULL) {
		delete m_pSoundTrack;
		m_pSoundTrack = NULL;
	}

	CSound::clearSounds();              // Make sure to return cleanly to Metagame

	// de-allocate the master sprites
	ReleaseMasterSprites();

	if (m_pTxtClickHere != NULL) {
		delete m_pTxtClickHere;
		m_pTxtClickHere = NULL;
	}

	//
	// de-allocate any controls that we used
	//
	assert(m_pScrollSprite != NULL);
	if (m_pScrollSprite != NULL) {
		delete m_pScrollSprite;
		m_pScrollSprite = NULL;
	}

	//
	// need to de-allocate the game palette
	//
	assert(m_pGamePalette != NULL);
	if (m_pGamePalette != NULL) {
		m_pGamePalette->DeleteObject();
		delete m_pGamePalette;
		m_pGamePalette = NULL;
	}

	if ((pDC = GetDC()) != NULL) {              // paint black

		rMyRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
		myBrush.CreateStockObject(BLACK_BRUSH);
		pDC->FillRect(&rMyRect, &myBrush);
		ReleaseDC(pDC);
	}

	CFrameWnd::OnClose();

	#ifdef _USRDLL
	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
	#endif
}

//////////// Additional Sound Notify routines //////////////

long CBFishWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != NULL)
		OnSoundNotify(pSound);
	return (0L);
}


long CBFishWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != NULL)
		OnSoundNotify(pSound);
	return (0L);
}

void CBFishWindow::OnSoundNotify(CSound *) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

//
// CBFishWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CBFishWindow, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_MESSAGE(MM_MCINOTIFY, CBFishWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CBFishWindow::OnMMIONotify)
END_MESSAGE_MAP()

VOID CALLBACK GetGameParams(CWnd *pParentWnd) {
	//
	// Our user preference dialog box is self contained in this object
	//
	CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);
}

} // namespace Battlefish
} // namespace HodjNPodj
} // namespace Bagel
