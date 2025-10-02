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
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/riddles/riddles.h"
#include "bagel/hodjnpodj/riddles/usercfg.h"
//#include "bagel/hodjnpodj/libs/copyrite.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/error.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace HodjNPodj {
namespace Riddles {

#define WORD_WRAP 1
#define RES       1

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP   ".\\ART\\RIDDLES.BMP"

#define WAV_YOUWIN      ".\\SOUND\\CONGRATS.WAV"
#define WAV_NOPE        ".\\SOUND\\NOPE.WAV"
#define WAV_TRYAGAIN    ".\\SOUND\\TRYAGAIN.WAV"
#define WAV_TICK        ".\\SOUND\\TICK.WAV"
#define WAV_GAMEOVER    ".\\SOUND\\TIMEOUT.WAV"
#define WAV_NARRATION   ".\\SOUND\\RIDD.WAV"

#define DATA_FILE       "RIDDLES.DAT"

#define DEFAULT_TIME_LIMIT  60

#define TIMER_ID            10
#define TIMER_INTERVAL    1000

#define MAX_RIDDLES        200

#define MIN_SOUND_ID         1
#define MAX_SOUND_ID       201

#define LEVEL_EASY   0
#define LEVEL_MEDIUM 1
#define LEVEL_HARD   2
#define LEVEL_RANDOM 3

#define EASY_START   0
#define MEDIUM_START (EASY_START+60)
#define HARD_START   (MEDIUM_START+70)

//
// Button ID constants
//
#define IDC_MENU     100

#define IDC_EDITTEXT 201

//
// Letter Bitmap IDs
//
#define TYPE_A      0
#define TYPE_Z      25
#define TYPE_PERIOD 26
#define TYPE_COMMA  27
#define TYPE_APOST  28
#define TYPE_QUOTE  29
#define TYPE_QUOTE2 30
#define TYPE_DASH   31
#define TYPE_EXCLAM 32
#define TYPE_QMARK  33
#define TYPE_SEMIC  34
#define TYPE_COLON  35
#define TYPE_0      36
#define TYPE_1      37
#define TYPE_2      38
#define TYPE_3      39
#define TYPE_4      40
#define TYPE_5      41
#define TYPE_6      42
#define TYPE_7      43
#define TYPE_8      44
#define TYPE_9      45

#define N_SPRITECHARS       46

#define RIDDLE_TOP          291
#define RIDDLE_LEFT         123
#define RIDDLE_BOTTOM       450
#define RIDDLE_RIGHT        405

#define LETTERSIZE_X        10
#define LETTERSIZE_Y        9
#define LETTERSPACING_X     1
#define LETTERSPACING_Y     2

// SunDial info
//
#define DIAL_START_X        64
#define DIAL_START_Y        395
#define DIAL_SEGMENTS       12

// local prototypes
//
void CALLBACK GetGameParams(CWnd *);

//
// Globals
//
CRiddlesWindow *gMainWindow;
CPalette    *pGamePalette;
const char  *INI_SECTION = "Riddles";
LPGAMESTRUCT pGameParams;


extern HWND ghParentWnd;

STATIC RIDDLE curRiddle;

STATIC CSprite *aMasterSpriteList[N_SPRITECHARS];


CRiddlesWindow::CRiddlesWindow() :
		cBrush(PALETTEINDEX(11)) {
	CString  WndClass;
	CRect    tmpRect;
	CPalette *pPalOld;
	CDC     *pDC;
	CDibDoc *pDibDoc;
	ERROR_CODE errCode;
	bool bSuccess;

	// assume no error
	errCode = ERR_NONE;

	// Initialize members
	//
	m_pGamePalette = nullptr;
	m_pScrollButton = nullptr;
	m_pEditText = nullptr;
	m_pSunDial = nullptr;
	m_pRiddle = nullptr;
	m_pSoundTrack = nullptr;
	m_bGameActive = false;
	m_bPause = false;
	m_nRiddleNumber = 0;
	m_nTimer = 0;
	gMainWindow = this;

	BeginWaitCursor();

	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_SAVEBITS | CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	// the game cannot continue if this bitmap does not exist
	//
	if (FileExists(MINI_GAME_MAP)) {

		// Acquire the shared palette for our game from the splash screen art
		//
		if ((pDibDoc = new CDibDoc()) != nullptr) {
			if (pDibDoc->OpenDocument(MINI_GAME_MAP) != false)
				pGamePalette = m_pGamePalette = pDibDoc->DetachPalette();
			else {
				// we don't know why OpenDocument failed, but it's still a fatal error
				//
				errCode = ERR_UNKNOWN;
			}
			delete pDibDoc;
		} else {
			// not enough memory to allocate a new CDibDoc
			errCode = ERR_MEMORY;
		}
	} else {
		// the file designated by "RIDDLES.BMP" does not exist
		errCode = ERR_FFIND;
	}

	// Center our window on the screen
	//
	tmpRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	Create(WndClass, "Boffo Games -- Riddles", WS_POPUP, tmpRect, nullptr, 0);

	if (errCode == ERR_NONE) {

		if ((pDC = GetDC()) != nullptr) {

			pPalOld = pDC->SelectPalette(m_pGamePalette, false);

			if ((m_pScrollButton = new CBmpButton) != nullptr) {

				m_bIgnoreScrollClick = false;
				tmpRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y, SCROLL_BUTTON_X + SCROLL_BUTTON_DX, SCROLL_BUTTON_Y + SCROLL_BUTTON_DY);
				bSuccess = m_pScrollButton->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, tmpRect, this, IDC_MENU);
				assert(bSuccess);
				if (bSuccess) {

					bSuccess = m_pScrollButton->LoadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);
					assert(bSuccess);
					if (!bSuccess)
						errCode = ERR_UNKNOWN;

				} else {
					errCode = ERR_UNKNOWN;
				}
			} else {
				errCode = ERR_MEMORY;
			}

			pDC->SelectPalette(pPalOld, false);
			ReleaseDC(pDC);
		}
	}

	//
	// Give 'em something to look at while they wait
	//
	ShowWindow(SW_SHOWNORMAL);                   // Put up the window
	UpdateWindow();                              // Generate an OnPaint message

	if (errCode == ERR_NONE) {

		// create the user guess edit text control
		//
		tmpRect.SetRect(330, 427, 560, 447);
		if ((m_pEditText = new CMyEdit) != nullptr) {
			bSuccess = m_pEditText->Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT | ES_UPPERCASE, tmpRect, this, IDC_EDITTEXT);
			assert(bSuccess);
			m_pEditText->LimitText(MAX_ANSWER_LENGTH);
			m_pEditText->SetFocus();
		} else {
			errCode = ERR_MEMORY;
		}
	}

	if (errCode == ERR_NONE) {

		// if background music is enabled
		//
		if (pGameParams->bMusicEnabled) {

			// start this game's sound track
			//
			m_pSoundTrack = new CSound(this, ".\\sound\\riddles.mid", SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
			if (m_pSoundTrack != nullptr) {
				(*m_pSoundTrack).midiLoopPlaySegment(1930, 32870, 0, FMT_MILLISEC);
			} else {
				errCode = ERR_MEMORY;
			}
		}

		// seed the random number generator
		////srand((unsigned)time(nullptr));

		// load the 32 character sprites into masters
		//
		if (!errCode)
			errCode = LoadMasterSprites();

		// if we are not playing from the metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			pGameParams->lScore = 0L;
			// Automatically bring up the main menu
			//
			PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		}
	}

	EndWaitCursor();

	HandleError(errCode);
}

void CRiddlesWindow::HandleError(ERROR_CODE errCode) {
	//
	// Exit this application on fatal errors
	//
	if (errCode != ERR_NONE) {

		// pause the game
		GamePause();

		// Display Error Message to the user
		MessageBox(errList[errCode], "Fatal Error!", MB_OK | MB_ICONSTOP);

		// Force this application to terminate
		PostMessage(WM_CLOSE, 0, 0);

		// Don't allow a repaint (remove all WM_PAINT messages)
		ValidateRect(nullptr);
	}
}


ERROR_CODE CRiddlesWindow::LoadMasterSprites() {
	CBitmap *pBmp, *pFontBmp;
	CSprite *pSprite;
	CDC *pDC;
	int i;
	ERROR_CODE errCode;

	// assume no erorr
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		if ((pFontBmp = FetchBitmap(pDC, nullptr, ".\\ART\\RIDFONT.BMP")) != nullptr) {

			// Load all of the letters used in the riddles
			//
			for (i = 0; i < N_SPRITECHARS; i++) {

				if ((pSprite = aMasterSpriteList[i] = new CSprite) != nullptr) {

					pBmp = ExtractBitmap(pDC, pFontBmp, m_pGamePalette, i * LETTERSIZE_X, 0, LETTERSIZE_X, LETTERSIZE_Y);

					(void)pSprite->LoadSprite(pBmp, m_pGamePalette);

					pSprite->SetMasked(true);
					pSprite->SetMobile(true);

				} else {
					errCode = ERR_MEMORY;
					break;
				}
			}
			delete pFontBmp;
		}

		if (errCode == ERR_NONE) {

			if ((m_pSunDial = new CSprite) != nullptr) {

				if (m_pSunDial->LoadCels(pDC, ".\\ART\\DIALCEL.BMP", DIAL_SEGMENTS) != false) {

					m_pSunDial->SharePalette(m_pGamePalette);

					m_pSunDial->SetMasked(true);
					m_pSunDial->SetMobile(true);
					m_pSunDial->LinkSprite();

					m_pSunDial->PaintSprite(pDC, DIAL_START_X, DIAL_START_Y);
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


void CRiddlesWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	Invalidate(false);
	BeginPaint(&lpPaint);
	PaintScreen();
	EndPaint(&lpPaint);
}


void CRiddlesWindow::PaintScreen() {
	CDibDoc myDoc;
	CRect   rcDest;
	CRect   rcDIB;
	HDIB    hDIB;
	CDC     *pDC;

	//
	// Paint the background art and upadate any sprites
	// called by OnPaint
	//
	if (FileExists(MINI_GAME_MAP)) {

		myDoc.OpenDocument(MINI_GAME_MAP);
		hDIB = myDoc.GetHDIB();

		pDC = GetDC();
		assert(pDC != nullptr);

		if (pDC != nullptr) {

			if (hDIB && (m_pGamePalette != nullptr)) {

				GetClientRect(rcDest);

				rcDIB.top = rcDIB.left = 0;
				rcDIB.right = (int) DIBWidth(hDIB);
				rcDIB.bottom = (int) DIBHeight(hDIB);
				
				PaintDIB(pDC->m_hDC, &rcDest, hDIB, &rcDIB, m_pGamePalette);
			}

			ReleaseDC(pDC);
		}

		if (m_pSunDial != nullptr) {
			assert((m_nTimer >= 0) && (m_nTimer <= DIAL_SEGMENTS));
			m_pSunDial->SetCel(m_nTimer - 1);
		}

		// Re-display the riddle
		//
		RepaintSpriteList();
	}
}


bool CRiddlesWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CMainMenu COptionsWind((CWnd *)this,
	                       m_pGamePalette,
	                       (pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0) | (m_bGameActive ? 0 : NO_RETURN),
	                       GetGameParams, "riddles.txt", (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr), pGameParams);

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
				CSound::clearWaveSounds();

				// Create the commands menu
				//

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

				//
				// Check to see if the music state was changed and adjust to match it
				//
				if ((pGameParams->bMusicEnabled == false) && (m_pSoundTrack != nullptr)) {
					if (m_pSoundTrack->playing())
						m_pSoundTrack->stop();
				} else if (pGameParams->bMusicEnabled) {
					if (m_pSoundTrack == nullptr) {
						m_pSoundTrack = new CSound(this, ".\\sound\\riddles.mid", SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					}
					if (m_pSoundTrack != nullptr) {
						if (!m_pSoundTrack->playing())
							(*m_pSoundTrack).midiLoopPlaySegment(1930, 32870, 0, FMT_MILLISEC);
					}
				}

				GameResume();
			}

			return true;
		}
	}

	return false;
}


void CRiddlesWindow::GamePause() {
	m_bPause = true;
}


void CRiddlesWindow::GameResume() {
	m_bPause = false;
}

void CRiddlesWindow::PlayGame() {
	char szBuf[40];
	ERROR_CODE errCode;
	CSound  *pRiddleReading = nullptr;

	// assume no error
	errCode = ERR_NONE;

	// load the .INI settings
	//
	LoadIniSettings();

	// reset all game parameters
	//
	GameReset();

	//
	// Load a new riddle
	//
	if ((errCode = LoadRiddle()) == ERR_NONE) {

		// Speak the riddle (as .WAV)
		//
		if (pGameParams->bSoundEffectsEnabled) {
			Common::sprintf_s(szBuf, ".\\SOUND\\RD%03d.WAV", m_pRiddle->nSoundId);
//          sndPlaySound(szBuf, SND_SYNC);

			if (FileExists(szBuf)) {                                        // Make sure we have the file
				CSound::clearWaveSounds();
				pRiddleReading = new CSound((CWnd *)this, szBuf,                                // Load up the sound file as a
				                            SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
				if (pRiddleReading != nullptr)
					pRiddleReading->play();                                                          //...play the sound effect
			}
		}

		// Set the timer for 1 second intervals
		//
		if (m_nInitTimeLimit != 0)
			SetTimer(TIMER_ID, 1000 / DIAL_SEGMENTS * m_nInitTimeLimit, nullptr);

		m_bGameActive = true;
	}

	HandleError(errCode);
}

void CRiddlesWindow::LoadIniSettings() {
	int nVal;

	if (pGameParams->bPlayingMetagame) {

		switch (pGameParams->nSkillLevel) {

		case SKILLLEVEL_LOW:
			m_nInitTimeLimit = 90;
			m_nDifficultyLevel = 0;
			break;

		case SKILLLEVEL_MEDIUM:
			m_nInitTimeLimit = 60;
			m_nDifficultyLevel = 1;
			break;

		case SKILLLEVEL_HIGH:
			m_nInitTimeLimit = 20;
			m_nDifficultyLevel = 2;
			break;

		default:
			assert(0);
			break;
		}

	} else {

		// Get the Time Limit in Seconds (10...Infinite = 0)
		//
		//
		nVal = GetPrivateProfileInt(INI_SECTION, "TimeLimit", DEFAULT_TIME_LIMIT, INI_FILENAME);
		m_nInitTimeLimit = nVal;
		if ((nVal != 0) && (nVal < LIMIT_MIN || nVal > LIMIT_MAX))
			m_nInitTimeLimit = DEFAULT_TIME_LIMIT;

		m_nDifficultyLevel = GetPrivateProfileInt(INI_SECTION, "DifficultyLevel", LEVEL_DEF, INI_FILENAME);
		if ((m_nDifficultyLevel < LEVEL_MIN) || (m_nDifficultyLevel > LEVEL_MAX))
			m_nDifficultyLevel = LEVEL_DEF;
	}
}


void CRiddlesWindow::GameReset() {
	CDC *pDC;

	pDC = GetDC();                              // get the current device context

	sndPlaySound(nullptr, 0);                   // stop any sndPlaySound waves in play

	m_nTimer = 0;                               // reset current time

	m_pEditText->SetWindowText("");             // erase any previous answer
	m_pEditText->UpdateWindow();

	CSprite::EraseSprites(pDC);                 // erase any on-screen letters

	assert(m_pSunDial != nullptr);                 // take sun dial out of chain
	m_pSunDial->UnlinkSprite();                 // so it's not flushed
	m_pSunDial->SetCel(m_nTimer - 1);           // revert to original Sun dial

	CSprite::FlushSpriteChain();                // delete all sprites in chain

	m_pSunDial->LinkSprite();
	m_pSunDial->PaintSprite(pDC, m_pSunDial->GetPosition());

	ReleaseDC(pDC);                             // release current device context

	m_pEditText->SetWindowText("");         // erase any text in edit ctrl

	m_bGameActive = false;                      // there is no current game

	m_bPause = false;                           // the game is not pauses

	m_nTimeLimit = m_nInitTimeLimit;            // get time limit

	m_nRiddleNumber = 0;                        // set the riddle number to a valid number

	memset(&curRiddle, 0, sizeof(RIDDLE));      // reset current riddle

	KillTimer(TIMER_ID);                        // kill any timers

	m_pRiddle = nullptr;                           // there is no current riddle
}


ERROR_CODE CRiddlesWindow::LoadRiddle() {
	STATIC unsigned int nLast;
	int n, nMin, nMax;
	ERROR_CODE errCode;

	errCode = ERR_NONE;                                     // assume no error

	if (FileExists(DATA_FILE)) {

		m_pRiddle = &curRiddle;                             // get a suitable storage area for this riddle

		n = (int)(FileLength(DATA_FILE) / sizeof(RIDDLE));  // determine number of riddles in the data store

		assert(n > 0 && n <= MAX_RIDDLES);                   // verify # of riddles

		// 60, 70, 71 for Easy, Medium and Hard
		//
		switch (m_nDifficultyLevel) {

		case LEVEL_EASY:
			nMin = EASY_START;
			nMax = MEDIUM_START;
			break;

		case LEVEL_MEDIUM:
			nMin = MEDIUM_START;
			nMax = HARD_START;
			break;

		case LEVEL_HARD:
			nMin = HARD_START;
			nMax = MAX_RIDDLES;
			break;

		default:
			assert(m_nDifficultyLevel == LEVEL_RANDOM);
			nMin = EASY_START;
			nMax = MAX_RIDDLES;
			break;
		}

		// get riddle from difficulty range
		n = nMax - nMin;

		// don't load same riddle twice in a row
		do {
			m_nRiddleNumber = nMin + (brand() % n);
		} while (m_nRiddleNumber == nLast);

		// remember last riddle #
		nLast = m_nRiddleNumber;

		// Load m_nRiddleNumber from the data store
		//
		ifstream inFile;
		inFile.open(DATA_FILE, ios::binary);                // open the data store

		inFile.seekg((long)m_nRiddleNumber * sizeof(RIDDLE));     // seek to the riddle we want

		inFile.read((char *)m_pRiddle, sizeof(RIDDLE));     // load that riddle
		if (inFile.gcount() != sizeof(RIDDLE))
			errCode = ERR_FREAD;

		inFile.close();                                     // close the data store

		if (errCode == ERR_NONE) {

			Decrypt(m_pRiddle, sizeof(RIDDLE));             // decrypt the riddle

			strUpper(m_pRiddle->text);                        // convert riddle to uppercase

			errCode = ValidateRiddle(m_pRiddle);            // make sure the data file is not corrupt

			if (errCode == ERR_NONE)
				errCode = BuildSpriteList();                // each letter in the riddle is a sprite
		}

	} else {
		errCode = ERR_FFIND;
	}

	return errCode;
}

ERROR_CODE CRiddlesWindow::ValidateRiddle(RIDDLE *pRiddle) {
	ERROR_CODE errCode;
	int i, n;
	char c;

	// assume no error
	errCode = ERR_NONE;

	assert(pRiddle != nullptr);

	if (pRiddle->nSoundId < MIN_SOUND_ID || pRiddle->nSoundId > MAX_SOUND_ID) {
		errCode = ERR_FTYPE;
		ErrorLog("ERROR.LOG", "%d has bad Sound ID", m_nRiddleNumber);

	} else {

		do {
			n = strlen(pRiddle->text);

			if (n > MAX_RIDDLE_LENGTH) {
				errCode = ERR_FTYPE;
				ErrorLog("ERROR.LOG", "%d is too big", pRiddle->nSoundId);
				break;
			}

			if (n < MIN_RIDDLE_LENGTH) {
				errCode = ERR_FTYPE;
				ErrorLog("ERROR.LOG", "%d is too small", pRiddle->nSoundId);
				break;
			}

			for (i = 0; i < n; i++) {
				c = pRiddle->text[i];
				if (c > 'Z' || (c < 'A' && c != 34 && c != ';' && c != ':' && c != ' ' && c != ',' && c != '.' && c != 39 && c != '-' && c != '?' && (c < '0' || c > '9'))) {
					errCode = ERR_FTYPE;
					ErrorLog("ERROR.LOG", "%d has invalid char", pRiddle->nSoundId);
					break;
				}
			}

			if (errCode)
				break;

			for (i = 0; i < MAX_ANSWERS; i++) {
				if (strlen(pRiddle->answers[i]) > MAX_ANSWER_LENGTH) {
					errCode = ERR_FTYPE;
					ErrorLog("ERROR.LOG", "%d answer is too big", pRiddle->nSoundId);
					break;
				}
			}

		} while (0);
	}

	return errCode;
}


ERROR_CODE CRiddlesWindow::BuildSpriteList() {
	char *pRiddle, *p;
	int x, y;
	unsigned int nCharsPerLine;
	ERROR_CODE errCode;

	// can't access a null pointer
	assert(m_pRiddle != nullptr);

	// assume no error
	errCode = ERR_NONE;

	// use local pointer
	pRiddle = m_pRiddle->text;

	nCharsPerLine = (RIDDLE_RIGHT - RIDDLE_LEFT) / (LETTERSIZE_X + LETTERSPACING_X);
	y = RIDDLE_TOP;

	//
	// build a sprite list for this phrase
	//
	while (*pRiddle != '\0') {

		x = RIDDLE_LEFT;

		p = (pRiddle + min(nCharsPerLine, strlen(pRiddle)));

		#if WORD_WRAP
		if (nCharsPerLine < strlen(pRiddle)) {
			while (*p != ' ')
				p--;
		}
		#endif
		if ((errCode = DisplayLine(pRiddle, p - pRiddle, x, y)) != ERR_NONE)
			break;

		#if WORD_WRAP
		pRiddle = p + 1;
		#else
		pRiddle = p;
		#endif
		y += LETTERSIZE_Y + LETTERSPACING_Y;
	}

	return errCode;
}


ERROR_CODE CRiddlesWindow::DisplayLine(const char *pszText, int nChars, int x, int y) {
	CSprite *pSprite;
	CDC *pDC;
	int nID, i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	pDC = GetDC();

	if (pDC != nullptr) {

		for (i = 0; i < nChars; i++, pszText++) {

			// ignore spaces
			//
			if (*pszText != ' ') {

				nID = CharToIndex(*pszText);

				// validate the master sprites and their IDs
				//
				assert(nID >= 0 && nID <= N_SPRITECHARS);
				assert(aMasterSpriteList[nID] != nullptr);

				//
				// create a new sprite for this letter
				//
				if ((pSprite = aMasterSpriteList[nID]->DuplicateSprite(pDC)) != nullptr) {

					pSprite->SetMasked(true);
					pSprite->LinkSprite();

					//
					// set this letter's location
					//
					pSprite->PaintSprite(pDC, x, y);

				} else {
					errCode = ERR_MEMORY;
					break;
				}
			}

			// increment letter position
			//
			x += LETTERSIZE_X + LETTERSPACING_X;
		}

		ReleaseDC(pDC);
	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}

int CRiddlesWindow::CharToIndex(char c) {
	int nIndex = -1;

	switch (c) {

	// character is a period
	//
	case '.':
		nIndex = TYPE_PERIOD;
		break;

	// character is a comma
	//
	case ',':
		nIndex = TYPE_COMMA;
		break;

	// character is an apostrophe
	//
	case 39:
		nIndex = TYPE_APOST;
		break;

	// character is a double quote
	//
	case 34:
		nIndex = TYPE_QUOTE;
		break;

	// char is a dash
	//
	case '-':
		nIndex = TYPE_DASH;
		break;

	// char is an exclamation mark
	//
	case '!':
		nIndex = TYPE_EXCLAM;
		break;

	// character is a question mark
	//
	case '?':
		nIndex = TYPE_QMARK;
		break;

	// character is a semi-colon
	//
	case ';':
		nIndex = TYPE_SEMIC;
		break;

	// character is a colon
	//
	case ':':
		nIndex = TYPE_COLON;
		break;

	case '0':
		nIndex = TYPE_0;
		break;

	case '1':
		nIndex = TYPE_1;
		break;

	case '2':
		nIndex = TYPE_2;
		break;

	case '3':
		nIndex = TYPE_3;
		break;

	case '4':
		nIndex = TYPE_4;
		break;

	case '5':
		nIndex = TYPE_5;
		break;

	case '6':
		nIndex = TYPE_6;
		break;

	case '7':
		nIndex = TYPE_7;
		break;

	case '8':
		nIndex = TYPE_8;
		break;

	case '9':
		nIndex = TYPE_9;
		break;

	// character must be an uppercase letter
	//
	default:
		assert(Common::isAlpha(c));
		assert(Common::isUpper(c));

		nIndex = c - 65;

		break;
	}

	// Sprite ID is index into the master sprite list
	//
	assert((nIndex >= 0) && (nIndex < N_SPRITECHARS));

	return (nIndex);
}


ERROR_CODE CRiddlesWindow::RepaintSpriteList() {
	CSprite *pSprite;
	CDC *pDC;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	pDC = GetDC();
	if (pDC != nullptr) {

		//
		// Paint each sprite
		//
		pSprite = CSprite::GetSpriteChain();
		while (pSprite != nullptr) {

			pSprite->ClearBackground();
			pSprite->RefreshSprite(pDC);

			pSprite = pSprite->GetNextSprite();
		}
		ReleaseDC(pDC);
	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}


void CRiddlesWindow::OnTimer(uintptr nEvent) {
	CDC *pDC;

	// there should be only one timer
	assert(nEvent == TIMER_ID);

	//
	// continue as long as there is a currently active non-paused game
	//
	if (m_bGameActive && !m_bPause) {
		// Tick of the clock counting down
		if (pGameParams->bSoundEffectsEnabled) {
			// Don't tick whilst speech of riddle clue is playing
			if (CBofSound::waveSoundPlaying())
				return;

			sndPlaySound(WAV_TICK, SND_ASYNC);
		}

		// keep track of how long it takes
		m_nTimer++;

		// does this game have a time limit ?
		//
		if (m_nInitTimeLimit != 0) {

			if ((pDC = GetDC()) != nullptr) {

				// display next sun dial formation
				//
				assert((m_nTimer >= 0) && (m_nTimer <= DIAL_SEGMENTS));
				assert(m_pSunDial != nullptr);

				m_pSunDial->SetCel(m_nTimer - 1);
				m_pSunDial->PaintSprite(pDC, m_pSunDial->GetPosition());

				ReleaseDC(pDC);
			}

			// if users time has expired then reveal answer
			//
			if (m_nTimer == DIAL_SEGMENTS) {

				// User has lost
				//
				GamePause();

				CSound::clearWaveSounds();                      // Make sure nothing else is playing
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(WAV_GAMEOVER, SND_ASYNC);

				// can't read from a null pointer
				assert(m_pRiddle != nullptr);

				FlushInputEvents();
				CMessageBox dlgYouLose((CWnd *)this, m_pGamePalette, "Time's up.", "");

				GameReset();

				if (pGameParams->bPlayingMetagame) {
					if (pGameParams->bSoundEffectsEnabled)
						sndPlaySound(nullptr, SND_ASYNC);
					pGameParams->lScore = 0;
					PostMessage(WM_CLOSE, 0, 0);
				} else                                          // In stand-alone
					PlayGame();                                 //...continuous play
			}
		}
	}
}


void CRiddlesWindow::OnMouseMove(unsigned int, CPoint) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

HBRUSH CRiddlesWindow::OnCtlColor(CDC *pDC, CWnd *pWnd, unsigned int nCtlColor) {
	switch (nCtlColor) {
	case CTLCOLOR_MSGBOX:
		pDC->SetTextColor(PALETTEINDEX(5));
		return ((HBRUSH)cBrush.m_hObject);

	case CTLCOLOR_EDIT:

		pDC->SetTextColor(PALETTEINDEX(5));
		pDC->SetBkColor(PALETTEINDEX(11));
		return ((HBRUSH)cBrush.m_hObject);

	default:
		return (CWnd::OnCtlColor(pDC, pWnd, nCtlColor));
	}
}

void CMyEdit::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_Q
	//
	if ((nChar == 'q') && (nFlags & 0x2000)) {

		GetParent()->PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CEdit::OnSysChar(nChar, nRepCnt, nFlags);
	}
}

void CMyEdit::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_F4
	//
	if ((nChar == VK_F4) && (nFlags & 0x2000)) {

		GetParent()->PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CEdit::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
}


void CMyEdit::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CWnd *pParent;

	pParent = GetParent();

	// Handle keyboard input
	//
	switch (nChar) {

	//
	// Bring up the Rules
	//
	case VK_F1: {
		CSound::waitWaveSounds();
		gMainWindow->GamePause();
		CRules  RulesDlg(pParent, "riddles.txt", pGamePalette, (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr));
		RulesDlg.DoModal();
		gMainWindow->GameResume();
	}
	break;

	//
	// Bring up the options menu
	//
	case VK_F2:
		pParent->SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		break;

	default:
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}


void CMyEdit::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	char szBuf[MAX_ANSWER_LENGTH + 1];

	//
	// User is typing in his/her guess to the riddle
	//
	switch (nChar) {

	// check the users answer
	//
	case VK_RETURN:

		// get the contents of the edit control
		//
		GetWindowText(szBuf, MAX_ANSWER_LENGTH);

		assert(gMainWindow != nullptr);
		gMainWindow->ParseAnswer(szBuf);

		// erase the contents of the edit control
		//
		SetWindowText("");
		break;

	// clear users answer
	//
	case VK_ESCAPE:
		// erase the contents of the edit control
		//
		SetWindowText("");
		break;

	default:
		CEdit::OnChar(nChar, nRepCnt, nFlags);
		break;
	}
	SetFocus();
}

BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
END_MESSAGE_MAP()

void CRiddlesWindow::ParseAnswer(const char *pszAnswer) {
	if (m_bGameActive && !m_bPause) {

		GamePause();

		//
		// Check user's guess with the actual phrase
		//
		if (CheckUserGuess(pszAnswer)) {

			CSound::clearWaveSounds();                      // Make sure the reading stops first
			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_YOUWIN, SND_ASYNC);

			CMessageBox dlgYouWin((CWnd *)this, m_pGamePalette, "You are correct!", "You have won.");
			GameReset();

			if (pGameParams->bPlayingMetagame) {
				pGameParams->lScore = 1;
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(nullptr, SND_ASYNC);
				PostMessage(WM_CLOSE, 0, 0);
			} else                                          // In stand-alone
				PlayGame();                                 //...continuous play

		} else {
			CSound::clearWaveSounds();                      // Make sure the reading stops first
			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_NOPE, SND_SYNC);
			sndPlaySound(WAV_TRYAGAIN, SND_SYNC);
		}
		GameResume();
	}
}

bool CRiddlesWindow::CheckUserGuess(const char *pszGuess) {
	int i;

	assert(pszGuess != nullptr);
	assert(strlen(pszGuess) <= MAX_ANSWER_LENGTH);

	//
	// Check the user's guess with each of the possible answers
	//
	for (i = 0; i < MAX_ANSWERS; i++) {

		if (m_pRiddle->answers[i][0] != '\0') {

			if (StrCompare(m_pRiddle->answers[i], pszGuess, strlen(m_pRiddle->answers[i])))
				return true;
		}
	}

	return false;
}

void CRiddlesWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC     *pDC;
	CRect   animRect;                   // All three Easter Egg animations are in the same place
	CRect   col1Rect,
	        col2Rect,
	        col3Rect,
	        col4Rect,
	        col5Rect,
	        col6Rect,
	        col7Rect;                   // There are seven column areas for an audio easter egg
	CPoint  animLoc;                    // Where the specific cel is located
	CSprite *pSprite;                   // Pointer to animation cel strip
	CSound  *pEffect;                   // Sound effect for easter egg
	bool    bSuccess;                   // Flag to check construction & loading success
	char    animBuf[64],                // Buffer to hold file spec for bitmap file
	        soundBuf[64];               // Buffer to hold file spec for sound file
	int     i;                          // Frame counter
	int     nSelector;                  // Which easter egg to use
	int     nNumCels = 0;                   // Number of cels in the easter egg

	animRect.SetRect(ANIM_X, ANIM_Y, ANIM_X + ANIM_DX, ANIM_Y + ANIM_DY);
	col1Rect.SetRect(COLUMN_1_X, COLUMN_1_Y, COLUMN_1_X + COLUMN_1_DX, COLUMN_1_Y + COLUMN_1_DY);
	col2Rect.SetRect(COLUMN_2_X, COLUMN_2_Y, COLUMN_2_X + COLUMN_2_DX, COLUMN_2_Y + COLUMN_2_DY);
	col3Rect.SetRect(COLUMN_3_X, COLUMN_3_Y, COLUMN_3_X + COLUMN_3_DX, COLUMN_3_Y + COLUMN_3_DY);
	col4Rect.SetRect(COLUMN_4_X, COLUMN_4_Y, COLUMN_4_X + COLUMN_4_DX, COLUMN_4_Y + COLUMN_4_DY);
	col5Rect.SetRect(COLUMN_5_X, COLUMN_5_Y, COLUMN_5_X + COLUMN_5_DX, COLUMN_5_Y + COLUMN_5_DY);
	col6Rect.SetRect(COLUMN_6_X, COLUMN_6_Y, COLUMN_6_X + COLUMN_6_DX, COLUMN_6_Y + COLUMN_6_DY);
	col7Rect.SetRect(COLUMN_7_X, COLUMN_7_Y, COLUMN_7_X + COLUMN_7_DX, COLUMN_7_Y + COLUMN_7_DY);

	// User clicked on the Title - NewGame button
	//
	if (m_rNewGameButton.PtInRect(point)) {

		// if we are not playing from the metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			// start a new game
			PlayGame();
		}
	} else if (animRect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		nSelector = brand() % 3;                     // Pick one of the 3 easter eggs randomly
		switch (nSelector) {

		case 0:
			Common::sprintf_s(animBuf, FISH_ANIM);
			Common::sprintf_s(soundBuf, FISH_WAV);
			animLoc.x = FISH_X;
			animLoc.y = FISH_Y;
			nNumCels = NUM_FISH_CELS;
			break;

		case 1:
			Common::sprintf_s(animBuf, NESS_ANIM);
			Common::sprintf_s(soundBuf, NESS_WAV);
			animLoc.x = NESSIE_X;
			animLoc.y = NESSIE_Y;
			nNumCels = NUM_NESSIE_CELS;
			break;

		case 2:
			Common::sprintf_s(animBuf, SKIER_ANIM);
			Common::sprintf_s(soundBuf, SKIER_WAV);
			animLoc.x = SKIER_X;
			animLoc.y = SKIER_Y;
			nNumCels = NUM_SKIER_CELS;
			break;
		} // end Switch

		bSuccess = (*pSprite).LoadCels(pDC, animBuf, nNumCels);
		ASSERT(bSuccess);
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if (bSuccess) {
			if (pGameParams->bSoundEffectsEnabled) {
				pEffect = new CSound((CWnd *)this, soundBuf,                                // Load up the sound file as a
				                     SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
				(*pEffect).play();                                                          //...play the sound effect
			}
			for (i = 0; i < nNumCels; i++) {
				(*pSprite).PaintSprite(pDC, animLoc.x, animLoc.y);
				Sleep(ANIM_SLEEP);
			}
		} // end if bSuccess

		delete pSprite;
		ReleaseDC(pDC);

	} else if ((((((col1Rect.PtInRect(point) || col2Rect.PtInRect(point)) || col3Rect.PtInRect(point)) ||
	              col4Rect.PtInRect(point)) || col5Rect.PtInRect(point)) || col6Rect.PtInRect(point)) ||
	           col7Rect.PtInRect(point)) {
		if (pGameParams->bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, COLUMN_WAV,                              // Load up the sound file as a
			                     SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                          //...play the sound effect
		}
	} else {

		// is this needed ?
		CFrameWnd::OnLButtonDown(nFlags, point);
	}
}

void CRiddlesWindow::OnSetFocus(CWnd *) {
	if (m_pEditText != nullptr)
		m_pEditText->SetFocus();
}

void CRiddlesWindow::DeleteSprite(CSprite *pSprite) {
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

void CRiddlesWindow::FlushInputEvents() {
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

void CRiddlesWindow::OnClose() {
	CBrush myBrush;
	CRect myRect;
	CDC *pDC;
	int i;

	// perform cleanup
	//
	GameReset();

	// delete the game theme song
	//
	if (m_pSoundTrack != nullptr) {
		delete m_pSoundTrack;
		m_pSoundTrack = nullptr;
	}

	CSound::clearSounds();

	// release the master sprites
	//
	if (m_pSunDial != nullptr) {
		delete m_pSunDial;
		m_pSunDial = nullptr;
	}

	for (i = 0; i < N_SPRITECHARS; i++) {
		if (aMasterSpriteList[i] != nullptr) {
			delete aMasterSpriteList[i];
			aMasterSpriteList[i] = nullptr;
		}
	}

	// delete the guess edit text control
	//
	assert(m_pEditText);
	if (m_pEditText != nullptr) {
		delete m_pEditText;
		m_pEditText = nullptr;
	}

	//
	// de-allocate any controls that we used
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

//////////// Additional Sound Notify routines //////////////

LRESULT CRiddlesWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CRiddlesWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CRiddlesWindow::OnSoundNotify(CSound *) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

//
// CRiddlesWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CRiddlesWindow, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(MM_MCINOTIFY, CRiddlesWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CRiddlesWindow::OnMMIONotify)
END_MESSAGE_MAP()

void CALLBACK GetGameParams(CWnd *pParentWnd) {
	//
	// Our user preference dialog box is self contained in this object
	//
	CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);
}

} // namespace Riddles
} // namespace HodjNPodj
} // namespace Bagel
