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

#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/audiocfg.h"
#include "bagel/hodjnpodj/metagame/grand_tour/dialogs.h"
#include "bagel/hodjnpodj/metagame/grand_tour/grand_tour.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

#define SPLASHSPEC  ".\\ART\\MLSCROLL.BMP"

// Button positioning constants

#define F_WIDTH                 110
#define F_HEIGHT                20
#define F_TOP                   SCROLL_TOP + 325
#define PLAY_LEFT               SCROLL_LEFT + 75
#define SAVE_LEFT               PLAY_LEFT + F_WIDTH + 5
#define RESTORE_LEFT            SAVE_LEFT + F_WIDTH + 5
#define AUDIO_LEFT              PLAY_LEFT
#define TOP10_LEFT              AUDIO_LEFT + F_WIDTH + 5
#define LEAVE_LEFT              TOP10_LEFT + F_WIDTH + 5

#define S_WIDTH                 130
#define S_HEIGHT                21

#define HS_LEFT                 SCROLL_LEFT + 110

#define HSH_TOP                 SCROLL_TOP + 50
#define HSM_TOP                 HSH_TOP + S_HEIGHT + 2
#define HSL_TOP                 HSM_TOP + S_HEIGHT + 2
#define HSNP_TOP                HSL_TOP + S_HEIGHT + 2

#define PS_LEFT                 SCROLL_LEFT + 310

#define PSH_TOP                 SCROLL_TOP + 50
#define PSM_TOP                 PSH_TOP + S_HEIGHT + 2
#define PSL_TOP                 PSM_TOP + S_HEIGHT + 2
#define PSNP_TOP                PSL_TOP + S_HEIGHT + 2

#define G_WIDTH                 130
#define G_HEIGHT                21

#define G_TOP                   SCROLL_TOP + 185
#define GA_LEFT                 SCROLL_LEFT + 55
#define GG_LEFT                 GA_LEFT + G_WIDTH + 5
#define GR_LEFT                 GG_LEFT + G_WIDTH + 5

// Globals
extern HCURSOR          hGameCursor;

static CBitmap     *pSplashScreen = nullptr;
static CPalette    *pGamePalette = nullptr;        // Palette to be used throughout the game


static CColorButton    *pScoresResetButton = nullptr;
static CColorButton    *pScoresLeaveButton = nullptr;

static CColorButton    *pPlayButton = nullptr;
static CColorButton    *pSaveButton = nullptr;
static CColorButton    *pRestoreButton = nullptr;
static CColorButton    *pLeaveButton = nullptr;

static CColorButton    *pAudioButton = nullptr;
static CColorButton    *pTop10Button = nullptr;

static CRadioButton *pHSHButton = nullptr; // Hodj Skill High Radio Button
static CRadioButton *pHSMButton = nullptr; // Hodj Skill Medium Radio Button
static CRadioButton *pHSLButton = nullptr; // Hodj Skill Low Radio Button
static CRadioButton *pHSNPButton = nullptr;    // Hodj Not Playing Radio Button

static CRadioButton *pPSHButton = nullptr; // Podj Skill High Radio Button
static CRadioButton *pPSMButton = nullptr; // Podj Skill Medium Radio Button
static CRadioButton *pPSLButton = nullptr; // Podj Skill Low Radio Button
static CRadioButton *pPSNPButton = nullptr;    // Podj Not Playing Radio Button

static CRadioButton *pGAButton = nullptr;  // Game Played in Alphabetical Order Radio Button
static CRadioButton *pGGButton = nullptr;  // Game Played in Geographical Order Radio Button
static CRadioButton *pGRButton = nullptr;  // Game Played in Random Order Radio Button

static  bool        bActiveWindow = false;          // whether our window is active

int     nReturnValue = -1;       // the values to return to the main EXE to tell it what
// DLL to dispatch to

static const int GAME_VALUES[18] = {        // set the game values to return
	MG_GAME_ARCHEROIDS, MG_GAME_ARTPARTS, MG_GAME_BARBERSHOP, MG_GAME_BATTLEFISH,
	MG_GAME_BEACON, MG_GAME_CRYPTOGRAMS, MG_GAME_DAMFURRY, MG_GAME_FUGE,
	MG_GAME_GARFUNKEL, MG_GAME_LIFE, MG_GAME_MANKALA, MG_GAME_MAZEODOOM,
	MG_GAME_NOVACANCY, MG_GAME_PACRAT, MG_GAME_PEGGLEBOZ, MG_GAME_RIDDLES,
	MG_GAME_THGESNGGME, MG_GAME_WORDSEARCH
};

static const char *aszGames[18] = {      // set the display names for when the cursor passes over a game rect
	"Archeroids", "Art Parts", "Barbershop Quintet", "Battlefish", "Beacon", "Cryptograms",
	"Dam Furry Animals", "Fuge", "Garfunkel", "Life", "Mankala", "Maze O' Doom",
	"No Vacancy", "Pack-Rat", "Peggleboz", "Riddles", "TH GESNG GAM", "Word Search"
};

static const int anGeoOrder[18] = { 9, 12, 11, 0, 7, 13, 5, 16, 17, 1, 4, 14, 3, 10, 15, 2, 6, 8 };

static SCORESTRUCT astTopTenScores[10];
static bool        bDisplayTopTen = false;
static bool        bInsertPlayer = false;
static int         nNewRank = -1;
static int         nCurChar = 0;
static int         tmWidth = 0;
static int         tmHeight = 0;
static bool        bDonePodj = false;
static CText       *pText = nullptr;

/*****************************************************************
 *
 * CMainGTWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Create the window with the appropriate style, size, menu, etc.;
 * it will be later revealed by CTheApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB.
 *
 * FORMAL PARAMETERS:
 *
 *  lUserAmount = initial amount of money that user starts with
 *                              defaults to zero
 *  nRounds         = the number of rounds to play, if 0 then not playing rounds
 *                          = defaults to zero
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

CMainGTWindow::CMainGTWindow(HWND hCallingWnd, LPGRANDTRSTRUCT  pgtGrandTourStruct) :
		SplashRect(0, 0, SCROLL_WIDTH, SCROLL_HEIGHT),
		ScoresLeaveRect(PLAY_LEFT, F_TOP, SAVE_LEFT + F_WIDTH, F_TOP + F_HEIGHT),
		ScoresResetRect(RESTORE_LEFT, F_TOP, LEAVE_LEFT + F_WIDTH, F_TOP + F_HEIGHT),
	
		PlayRect(PLAY_LEFT, F_TOP - 25, PLAY_LEFT + F_WIDTH, F_TOP - 25 + F_HEIGHT),
		SaveRect(SAVE_LEFT, F_TOP - 25, SAVE_LEFT + F_WIDTH, F_TOP - 25 + F_HEIGHT),
		RestoreRect(RESTORE_LEFT, F_TOP - 25, RESTORE_LEFT + F_WIDTH, F_TOP - 25 + F_HEIGHT),
		AudioRect(AUDIO_LEFT, F_TOP, AUDIO_LEFT + F_WIDTH, F_TOP + F_HEIGHT),
		Top10Rect(TOP10_LEFT, F_TOP, TOP10_LEFT + F_WIDTH, F_TOP + F_HEIGHT),
		LeaveRect(LEAVE_LEFT, F_TOP, LEAVE_LEFT + F_WIDTH, F_TOP + F_HEIGHT),
	
		HSHRect(HS_LEFT, HSH_TOP, HS_LEFT + S_WIDTH, HSH_TOP + S_HEIGHT),
		HSMRect(HS_LEFT, HSM_TOP, HS_LEFT + S_WIDTH, HSM_TOP + S_HEIGHT),
		HSLRect(HS_LEFT, HSL_TOP, HS_LEFT + S_WIDTH, HSL_TOP + S_HEIGHT),
		HSNPRect(HS_LEFT, HSNP_TOP, HS_LEFT + S_WIDTH, HSNP_TOP + S_HEIGHT),
	
		PSHRect(PS_LEFT, PSH_TOP, PS_LEFT + S_WIDTH, PSH_TOP + S_HEIGHT),
		PSMRect(PS_LEFT, PSM_TOP, PS_LEFT + S_WIDTH, PSM_TOP + S_HEIGHT),
		PSLRect(PS_LEFT, PSL_TOP, PS_LEFT + S_WIDTH, PSL_TOP + S_HEIGHT),
		PSNPRect(PS_LEFT, PSNP_TOP, PS_LEFT + S_WIDTH, PSNP_TOP + S_HEIGHT),
	
		GARect(GA_LEFT, G_TOP, GA_LEFT + G_WIDTH, G_TOP + G_HEIGHT),
		GGRect(GG_LEFT, G_TOP, GG_LEFT + G_WIDTH, G_TOP + G_HEIGHT),
		GRRect(GR_LEFT, G_TOP, GR_LEFT + G_WIDTH, G_TOP + G_HEIGHT) {
	CDC         *pDC = nullptr;                     // device context for the screen
	CString     WndClass;
	CSize       mySize;
	bool        bSuccess;           // bool for testing the creation of each button
	CRect       rText(SCROLL_LEFT, SCROLL_TOP + 456, SCROLL_LEFT + SCROLL_WIDTH, SCROLL_TOP + 478);
	CBrush      cBrush(RGB(0, 0, 0));
	bool        bThereAreGamesToBePlayed = false;
	int         i;
	TEXTMETRIC  sTextMetic;

	BeginWaitCursor();
	initStatics();

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               hGameCursor, (HBRUSH)cBrush.m_hObject, nullptr);

	m_hCallAppWnd = hCallingWnd;
	m_pgtGTStruct = pgtGrandTourStruct;
	m_pgtGTStruct->bPlayFX = m_pgtGTStruct->stMiniGame.bSoundEffectsEnabled;
	m_pgtGTStruct->bPlayMusic = m_pgtGTStruct->stMiniGame.bMusicEnabled;

	AfxGetApp()->addResources("hnpgt.dll");

	AdjustScore();

	// set the seed for the random number generator
	//srand( (unsigned)time( nullptr ));

	// load splash screen
	pDC = GetDC();                                  // get a device context for our window

	// set window coordinates to center game on screeen
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	ReleaseDC(pDC);                                 // release our window context

	// Create the window as a POPUP so that no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games - Grand Tour", WS_POPUP, MainRect, nullptr, 0);

	//#ifndef _DEBUG
	//SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
	//#endif

	pDC = GetDC();                                  // get a device context for our window

	pSplashScreen = FetchBitmap(pDC, &pGamePalette, SPLASHSPEC);

	pDC->GetTextMetrics(&sTextMetic);
	tmWidth = sTextMetic.tmMaxCharWidth;
	tmHeight = sTextMetic.tmHeight;

	ReleaseDC(pDC);                                 // release our window context
	pDC = nullptr;

	for (i = 0; i < 18; i++) {
		if ((m_pgtGTStruct->nHodjSkillLevel != NOPLAY) && (m_pgtGTStruct->abHGamePlayed[i] == false)) {
			bThereAreGamesToBePlayed = true;
			break;
		}
		if ((m_pgtGTStruct->nPodjSkillLevel != NOPLAY) && (m_pgtGTStruct->abPGamePlayed[i] == false)) {
			bThereAreGamesToBePlayed = true;
			break;
		}
	}

	if (bThereAreGamesToBePlayed == false) {

		pScoresLeaveButton = new CColorButton();
		ASSERT(pScoresLeaveButton);
		bSuccess = pScoresLeaveButton->Create("Main Menu", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, ScoresLeaveRect, this, IDC_LEAVE);
		ASSERT(bSuccess);
		pScoresLeaveButton->SetPalette(pGamePalette);

		pScoresResetButton = new CColorButton();
		ASSERT(pScoresResetButton);
		bSuccess = pScoresResetButton->Create("Reset Scores", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, ScoresResetRect, this, IDC_RESET_SCORES);
		ASSERT(bSuccess);
		pScoresResetButton->SetPalette(pGamePalette);

		GetScores();

		bDisplayTopTen = true;

		for (i = 0; i < 10; i++) {
			if ((m_pgtGTStruct->nHodjSkillLevel != NOPLAY) && (m_pgtGTStruct->nHodjScore > astTopTenScores[i].nScore)) {
				int j;

				nNewRank = i;
				for (j = 9; j > i; j--) {
					Common::strcpy_s(astTopTenScores[j].acName, astTopTenScores[j - 1].acName);
					astTopTenScores[j].nScore = astTopTenScores[j - 1].nScore;
					astTopTenScores[j].nSkillLevel = astTopTenScores[j - 1].nSkillLevel;
				}
				Common::strcpy_s(astTopTenScores[i].acName, "HODJ");
				nCurChar = 5;
				astTopTenScores[i].acName[nCurChar - 1] = '|';
				astTopTenScores[i].acName[nCurChar] = 0;
				astTopTenScores[i].nScore = m_pgtGTStruct->nHodjScore;
				astTopTenScores[i].nSkillLevel = m_pgtGTStruct->nHodjSkillLevel;
				pDC = GetDC();
				pText = new CText();
				cTextRect.SetRect(SCROLL_LEFT + 69, SCROLL_TOP + (nNewRank * 20) + 90, SCROLL_LEFT + 349,  SCROLL_TOP + (nNewRank * 20) + 110);
				pText->SetupText(pDC, pGamePalette, &cTextRect, JUSTIFY_LEFT);
				ReleaseDC(pDC);
				pDC = nullptr;
				break;
			}
		}

		if (nNewRank == -1) {
			for (i = 0; i < 10; i++) {
				if ((m_pgtGTStruct->nPodjSkillLevel != NOPLAY) && (m_pgtGTStruct->nPodjScore > astTopTenScores[i].nScore)) {
					int j;

					nNewRank = i;
					for (j = 10; j > i; j--) {
						Common::strcpy_s(astTopTenScores[j].acName, astTopTenScores[j - 1].acName);
						astTopTenScores[j].nScore = astTopTenScores[j - 1].nScore;
						astTopTenScores[j].nSkillLevel = astTopTenScores[j - 1].nSkillLevel;
					}
					Common::strcpy_s(astTopTenScores[i].acName, "PODJ*");
					nCurChar = 5;
					astTopTenScores[i].acName[nCurChar - 1] = '|';
					astTopTenScores[i].acName[nCurChar] = 0;
					astTopTenScores[i].nScore = m_pgtGTStruct->nPodjScore;
					astTopTenScores[i].nSkillLevel = m_pgtGTStruct->nPodjSkillLevel;
					bDonePodj = true;
					break;
				}
			}
		}
		if (nNewRank > -1) {
			pScoresResetButton->EnableWindow(false);
			pScoresLeaveButton->EnableWindow(false);
			(*this).SetFocus();
		}
		//  else {
		//      pScoresLeaveButton->SetFocus();
		//  }
	} else {
		// create buttons
		pPlayButton = new CColorButton();
		ASSERT(pPlayButton);
		bSuccess = pPlayButton->Create("Play", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, PlayRect, this, IDC_PLAY);
		ASSERT(bSuccess);
		pPlayButton->SetPalette(pGamePalette);

		pSaveButton = new CColorButton();
		ASSERT(pSaveButton);
		bSuccess = pSaveButton->Create("Save", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, SaveRect, this, IDC_SAVE);
		ASSERT(bSuccess);
		pSaveButton->SetPalette(pGamePalette);

		pRestoreButton = new CColorButton();
		ASSERT(pRestoreButton);
		bSuccess = pRestoreButton->Create("Restore", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, RestoreRect, this, IDC_RESTORE);
		ASSERT(bSuccess);
		pRestoreButton->SetPalette(pGamePalette);

		pLeaveButton = new CColorButton();
		ASSERT(pLeaveButton);
		bSuccess = pLeaveButton->Create("Main Menu", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, LeaveRect, this, IDC_LEAVE);
		ASSERT(bSuccess);
		pLeaveButton->SetPalette(pGamePalette);

		pHSHButton = new CRadioButton;
		ASSERT(pHSHButton);
		bSuccess = pHSHButton->Create("Hard", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE | WS_GROUP, HSHRect, this, IDC_HODJ_SKILL_HARD);
		ASSERT(bSuccess);
		pHSHButton->SetPalette(pGamePalette);

		pHSMButton = new CRadioButton;
		ASSERT(pHSMButton);
		bSuccess = pHSMButton->Create("Medium", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, HSMRect, this, IDC_HODJ_SKILL_MEDIUM);
		ASSERT(bSuccess);
		pHSMButton->SetPalette(pGamePalette);

		pHSLButton = new CRadioButton;
		ASSERT(pHSLButton);
		bSuccess = pHSLButton->Create("Easy", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, HSLRect, this, IDC_HODJ_SKILL_EASY);
		ASSERT(bSuccess);
		pHSLButton->SetPalette(pGamePalette);

		pHSNPButton = new CRadioButton;
		ASSERT(pHSNPButton);
		bSuccess = pHSNPButton->Create("Count Me Out", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, HSNPRect, this, IDC_HODJ_NO_PLAY);
		ASSERT(bSuccess);
		pHSNPButton->SetPalette(pGamePalette);

		pPSHButton = new CRadioButton;
		ASSERT(pPSHButton);
		bSuccess = pPSHButton->Create("Hard", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE | WS_GROUP, PSHRect, this, IDC_PODJ_SKILL_HARD);
		ASSERT(bSuccess);
		pPSHButton->SetPalette(pGamePalette);

		pPSMButton = new CRadioButton;
		ASSERT(pPSMButton);
		bSuccess = pPSMButton->Create("Medium", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, PSMRect, this, IDC_PODJ_SKILL_MEDIUM);
		ASSERT(bSuccess);
		pPSMButton->SetPalette(pGamePalette);

		pPSLButton = new CRadioButton;
		ASSERT(pPSLButton);
		bSuccess = pPSLButton->Create("Easy", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, PSLRect, this, IDC_PODJ_SKILL_EASY);
		ASSERT(bSuccess);
		pPSLButton->SetPalette(pGamePalette);

		pPSNPButton = new CRadioButton;
		ASSERT(pPSNPButton);
		bSuccess = pPSNPButton->Create("Count Me Out", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, PSNPRect, this, IDC_PODJ_NO_PLAY);
		ASSERT(bSuccess);
		pPSNPButton->SetPalette(pGamePalette);

		pGAButton = new CRadioButton;
		ASSERT(pGAButton);
		bSuccess = pGAButton->Create("Alphabetically", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE | WS_GROUP, GARect, this, IDC_GAME_ALPHA);
		ASSERT(bSuccess);
		pGAButton->SetPalette(pGamePalette);

		pGGButton = new CRadioButton;
		ASSERT(pGGButton);
		bSuccess = pGGButton->Create("Geographically", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, GGRect, this, IDC_GAME_GEO);
		ASSERT(bSuccess);
		pGGButton->SetPalette(pGamePalette);

		pGRButton = new CRadioButton;
		ASSERT(pGRButton);
		bSuccess = pGRButton->Create("Randomly", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, GRRect, this, IDC_GAME_RAND);
		ASSERT(bSuccess);
		pGRButton->SetPalette(pGamePalette);

		pAudioButton = new CColorButton();
		ASSERT(pAudioButton);
		bSuccess = pAudioButton->Create("Audio Settings", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, AudioRect, this, IDC_AUDIO);
		ASSERT(bSuccess);
		pAudioButton->SetPalette(pGamePalette);

		pTop10Button = new CColorButton();
		ASSERT(pTop10Button);
		bSuccess = pTop10Button->Create("Top 10 List", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, Top10Rect, this, IDC_TOP10);
		ASSERT(bSuccess);
		pTop10Button->SetPalette(pGamePalette);

		if (m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_LOW) {
			pHSLButton->SetCheck(1);
			pHSMButton->SetCheck(0);
			pHSHButton->SetCheck(0);
			pHSNPButton->SetCheck(0);
		} else {
			if (m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_MEDIUM) {
				pHSLButton->SetCheck(0);
				pHSMButton->SetCheck(1);
				pHSHButton->SetCheck(0);
				pHSNPButton->SetCheck(0);
			} else {
				if (m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_HIGH) {
					pHSLButton->SetCheck(0);
					pHSMButton->SetCheck(0);
					pHSHButton->SetCheck(1);
					pHSNPButton->SetCheck(0);
				} else {
					pHSLButton->SetCheck(0);
					pHSMButton->SetCheck(0);
					pHSHButton->SetCheck(0);
					pHSNPButton->SetCheck(1);
				}
			}
		}

		if (m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_LOW) {
			pPSLButton->SetCheck(1);
			pPSMButton->SetCheck(0);
			pPSHButton->SetCheck(0);
			pPSNPButton->SetCheck(0);
		} else {
			if (m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_MEDIUM) {
				pPSLButton->SetCheck(0);
				pPSMButton->SetCheck(1);
				pPSHButton->SetCheck(0);
				pPSNPButton->SetCheck(0);
			} else {
				if (m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_HIGH) {
					pPSLButton->SetCheck(0);
					pPSMButton->SetCheck(0);
					pPSHButton->SetCheck(1);
					pPSNPButton->SetCheck(0);
				} else {
					pPSLButton->SetCheck(0);
					pPSMButton->SetCheck(0);
					pPSHButton->SetCheck(0);
					pPSNPButton->SetCheck(1);
				}
			}
		}

		if (m_pgtGTStruct->nGameSelection == GAME_ALPHA) {
			pGAButton->SetCheck(1);
			pGGButton->SetCheck(0);
			pGRButton->SetCheck(0);
		} else {
			if (m_pgtGTStruct->nGameSelection == GAME_GEO) {
				pGAButton->SetCheck(0);
				pGGButton->SetCheck(1);
				pGRButton->SetCheck(0);
			} else {
				pGAButton->SetCheck(0);
				pGGButton->SetCheck(0);
				pGRButton->SetCheck(1);
			}
		}

		if (m_pgtGTStruct->bMidGrandTour) {

			pHSHButton->EnableWindow(false);
			pHSMButton->EnableWindow(false);
			pHSLButton->EnableWindow(false);
			pHSNPButton->EnableWindow(false);
			pPSHButton->EnableWindow(false);
			pPSMButton->EnableWindow(false);
			pPSLButton->EnableWindow(false);
			pPSNPButton->EnableWindow(false);
			pGAButton->EnableWindow(false);
			pGGButton->EnableWindow(false);
			pGRButton->EnableWindow(false);

			if (m_pgtGTStruct->bPlayingHodj) {
				if (m_pgtGTStruct->nPodjSkillLevel != NOPLAY)
					m_pgtGTStruct->bPlayingHodj = false;
			} else {
				if (m_pgtGTStruct->nHodjSkillLevel != NOPLAY)
					m_pgtGTStruct->bPlayingHodj = true;
			}

			if (m_pgtGTStruct->bPlayingHodj)
				m_pgtGTStruct->nCurrGameCode = 0;

		}
	}
	(*this).SetFocus();

	EndWaitCursor();
}

CMainGTWindow::~CMainGTWindow() {
	AfxGetApp()->removeResources("hnpgt.dll");
}

void CMainGTWindow::initStatics() {
	pSplashScreen = nullptr;
	pGamePalette = nullptr;

	pScoresResetButton = nullptr;
	pScoresLeaveButton = nullptr;
	pPlayButton = nullptr;
	pSaveButton = nullptr;
	pRestoreButton = nullptr;
	pLeaveButton = nullptr;
	pAudioButton = nullptr;
	pTop10Button = nullptr;
	pHSHButton = nullptr;
	pHSMButton = nullptr;
	pHSLButton = nullptr;
	pHSNPButton = nullptr;
	pPSHButton = nullptr;
	pPSMButton = nullptr;
	pPSLButton = nullptr;
	pPSNPButton = nullptr;
	pGAButton = nullptr;
	pGGButton = nullptr;
	pGRButton = nullptr;

	bActiveWindow = false;
	nReturnValue = -1;

	bDisplayTopTen = false;
	bInsertPlayer = false;
	nNewRank = -1;
	nCurChar = 0;
	tmWidth = 0;
	tmHeight = 0;
	bDonePodj = false;
	pText = nullptr;
}

/*****************************************************************
 *
 * AdjustScore
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      increments the current players score by the return of the mini game
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CMainGTWindow::AdjustScore() {
	int     nGameScore = 0;
	long    lTemp = 0l;

	switch (m_pgtGTStruct->nCurrGameCode) {

	case  MG_GAME_ARCHEROIDS: // 1 or 0
	case  MG_GAME_ARTPARTS: // 1 or 0
	case  MG_GAME_BATTLEFISH: // 1 or 0
	case  MG_GAME_MANKALA: // 1 or 0
	case  MG_GAME_MAZEODOOM: // 1 or 0
	case  MG_GAME_RIDDLES: // 1 or 0
		nGameScore = ((int)m_pgtGTStruct->stMiniGame.lScore) * 100;
		break;

	case  MG_GAME_BEACON: //  %
		nGameScore = min(100, ((int)m_pgtGTStruct->stMiniGame.lScore * 2));
		break;

	case  MG_GAME_LIFE: // number
		nGameScore = min(100, (((int)m_pgtGTStruct->stMiniGame.lScore * 25) / 10));
		break;

	case  MG_GAME_THGESNGGME: // %
		nGameScore = min(100, (((int)m_pgtGTStruct->stMiniGame.lScore * 15) / 10));
		break;

	case  MG_GAME_CRYPTOGRAMS: // number
		nGameScore = ((int)m_pgtGTStruct->stMiniGame.lScore);
		break;

	case  MG_GAME_PEGGLEBOZ: // number
	case  MG_GAME_GARFUNKEL: // number
	case  MG_GAME_WORDSEARCH: // number
		nGameScore = (((int)m_pgtGTStruct->stMiniGame.lScore) * 100) / 25;
		break;

	case  MG_GAME_BARBERSHOP: // number of cards discarded
		nGameScore = (((int)m_pgtGTStruct->stMiniGame.lScore) * 100) / 62;
		break;

	case  MG_GAME_NOVACANCY: // number
		nGameScore = 100 - ((((int)m_pgtGTStruct->stMiniGame.lScore) * 100) / 45);
		break;

	case  MG_GAME_DAMFURRY: // number
		nGameScore = (((int)m_pgtGTStruct->stMiniGame.lScore) * 100) / 60;
		break;

	case  MG_GAME_FUGE: // number
		nGameScore = (((int)m_pgtGTStruct->stMiniGame.lScore) * 100) / 53;
		break;

	case  MG_GAME_PACRAT: // number
		lTemp = m_pgtGTStruct->stMiniGame.lScore * 100;
		switch (m_pgtGTStruct->stMiniGame.nSkillLevel) {
		case SKILLLEVEL_LOW:
			nGameScore = (int)(lTemp / 2373);
			break;
		case SKILLLEVEL_MEDIUM:
			nGameScore = (int)(lTemp / 14280);
			break;
		case SKILLLEVEL_HIGH:
			nGameScore = (int)(lTemp / 28584);
			break;
		}
		break;

	default:
		nGameScore = 0;
		break;
	}

	if (m_pgtGTStruct->bPlayingHodj) {
		m_pgtGTStruct->nHodjLastScore = nGameScore;
		m_pgtGTStruct->nHodjScore += nGameScore;
	} else {
		m_pgtGTStruct->nPodjLastScore = nGameScore;
		m_pgtGTStruct->nPodjScore += nGameScore;
	}
	return;
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.  Ensures that the entire client area
 * of the main screen window is repainted, not just the portion in the
 * update region; see SplashScreen();
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainGTWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	BeginPaint(&lpPaint);                           // bracket start of window update
	SplashScreen();                                 // repaint our window's content
	EndPaint(&lpPaint);                             // bracket end of window update

}

/*****************************************************************
 *
 * SplashScreen
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the background artwork, together with all sprites in the
 * sprite chain queue.  The entire window is redrawn, rather than just
 * the updated area, to ensure that the sprites end up with the correct
 * background bitmaps saved for their image areas.
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainGTWindow::SplashScreen() {
	CDC *pDC = GetDC();                                                                          // get a device context for the window
	CPalette *pOldPalette = pDC->SelectPalette(pGamePalette, false);    // load game palette;
	int         i;
	int         nLeft = SCROLL_LEFT;
	int         nTop = SCROLL_TOP;
	int         nOldBkMode;
	COLORREF    rgbOldColorRef;
	char        cTemp[64];
	char        cTemp1[64];
	char        cTemp2[64];

	ASSERT(pDC);
	pDC->RealizePalette();                      // realize game palette

	PaintMaskedBitmap(pDC, pGamePalette, pSplashScreen, nLeft, nTop, SplashRect.right, SplashRect.bottom);

	nOldBkMode = pDC->SetBkMode(TRANSPARENT);

	if (bDisplayTopTen) {
		pDC->TextOut(nLeft + 50, nTop + 50, "GRAND TOUR - TOP TEN SCORES", 27);
		pDC->TextOut(nLeft + 50, nTop + 70, "RANK", 4);

		for (i = 0; i < 10; i++) {
			Common::sprintf_s(cTemp, "%i", (i + 1));
			pDC->TextOut(nLeft + 50, nTop + (i * 20) + 90, cTemp, strlen(cTemp));
			if (i != nNewRank)
				pDC->TextOut(nLeft + 70, nTop + (i * 20) + 90, astTopTenScores[i].acName, strlen(astTopTenScores[i].acName));

			if (astTopTenScores[i].nSkillLevel == SKILLLEVEL_LOW) {
				pDC->TextOut(nLeft + 350, nTop + (i * 20) + 90, "Easy", 4);
			} else {
				if (astTopTenScores[i].nSkillLevel == SKILLLEVEL_MEDIUM) {
					pDC->TextOut(nLeft + 350, nTop + (i * 20) + 90, "Medium", 6);
				} else {
					pDC->TextOut(nLeft + 350, nTop + (i * 20) + 90, "Hard", 4);
				}
			}
			Common::sprintf_s(cTemp, "%i", astTopTenScores[i].nScore);
			pDC->TextOut(nLeft + 425, nTop + (i * 20) + 90, cTemp, strlen(cTemp));

		}

		if (nNewRank > -1) {

			if (pText != nullptr) {
				if (bDonePodj == false)
					pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(0, 0, 255));
				else
					pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(255, 0, 0));
			}

			pDC->TextOut(ScoresLeaveRect.left, nTop + 300, "Press Enter When Done.", 22);

			(*this).SetFocus();                         // Reset focus back to the main window

		} else {
			rgbOldColorRef = pDC->SetTextColor(RGB(0, 0, 255));
			Common::sprintf_s(cTemp1, "Hodj's Final Score : %i", m_pgtGTStruct->nHodjScore);
			pDC->TextOut(ScoresLeaveRect.left, nTop + 300, cTemp1, strlen(cTemp1));

			pDC->SetTextColor(RGB(255, 0, 0));
			Common::sprintf_s(cTemp1, "Podj's Final Score : %i", m_pgtGTStruct->nPodjScore);
			pDC->TextOut(ScoresResetRect.left, nTop + 300, cTemp1, strlen(cTemp1));

			pDC->SetTextColor(rgbOldColorRef);
			pScoresLeaveButton->SetFocus();
		}
	} else {

		unsigned int    nOldTextAlign;
		char    cNextGame[64];
		char    cLastGame[64];
		int     nGameCode;
		int     nGamesCompleted = 0;

		Common::strcpy_s(cNextGame, "RANDOM");
		nGameCode = GetNextGameCode(false);
		for (i = 0; i < 18; i++) {
			if ((m_pgtGTStruct->nGameSelection != GAME_RAND) && (GAME_VALUES[i] == nGameCode)) {
				Common::strcpy_s(cNextGame, aszGames[i]);
				break;
			}
		}

		//  Hodj's Text
		rgbOldColorRef = pDC->SetTextColor(RGB(0, 0, 255));
		pDC->TextOut(HS_LEFT - 40, HSH_TOP, "Hodj", 4);
		pDC->TextOut(HS_LEFT, HSNP_TOP + S_HEIGHT + 2, "Score :", 7);
		Common::sprintf_s(cTemp1, "%i", m_pgtGTStruct->nHodjScore);
		pDC->TextOut(HS_LEFT + 50, HSNP_TOP + S_HEIGHT + 2, cTemp1, strlen(cTemp1));
		if (m_pgtGTStruct->bPlayingHodj) {
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 96, "Hodj", 4);
			Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nHodjLastScore);
			Common::strcpy_s(cLastGame, "NONE");
			for (i = 0; i < 18; i++) {
				if (GAME_VALUES[i] == m_pgtGTStruct->nHodjLastGame) {
					Common::strcpy_s(cLastGame, aszGames[i]);
				}
				if (m_pgtGTStruct->abHGamePlayed[i])
					nGamesCompleted++;
			}

			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 24, cLastGame, strlen(cLastGame));
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 42, cTemp, strlen(cTemp));
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 60, cNextGame, strlen(cNextGame));
			Common::sprintf_s(cTemp, "%i Out Of 18", nGamesCompleted);
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 78, cTemp, strlen(cTemp));
		}

		//  Podj's Text
		pDC->SetTextColor(RGB(255, 0, 0));
		pDC->TextOut(PS_LEFT - 40, PSH_TOP, "Podj", 4);
		pDC->TextOut(PS_LEFT, PSNP_TOP + S_HEIGHT + 2, "Score :", 7);
		Common::sprintf_s(cTemp2, "%i", m_pgtGTStruct->nPodjScore);
		pDC->TextOut(PS_LEFT + 50, PSNP_TOP + S_HEIGHT + 2, cTemp2, strlen(cTemp2));
		if (m_pgtGTStruct->bPlayingHodj == false) {
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 96, "Podj", 4);
			Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nPodjLastScore);
			Common::strcpy_s(cLastGame, "NONE");
			for (i = 0; i < 18; i++) {
				if (GAME_VALUES[i] == m_pgtGTStruct->nPodjLastGame) {
					Common::strcpy_s(cLastGame, aszGames[i]);
				}
				if (m_pgtGTStruct->abPGamePlayed[i])
					nGamesCompleted++;
			}
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 24, cLastGame, strlen(cLastGame));
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 42, cTemp, strlen(cTemp));
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 60, cNextGame, strlen(cNextGame));
			Common::sprintf_s(cTemp, "%i Out Of 18", nGamesCompleted);
			pDC->TextOut(HS_LEFT + S_WIDTH + 5, G_TOP + 78, cTemp, strlen(cTemp));
		}

		//  Other Text
		pDC->SetTextColor(RGB(128, 0, 128));
		pDC->TextOut(GA_LEFT, G_TOP - 20, "Game Order:", 11);

		nOldTextAlign = pDC->SetTextAlign(TA_RIGHT);
		pDC->TextOut(HS_LEFT + S_WIDTH, G_TOP + 24, "Last Game :", 11);
		pDC->TextOut(HS_LEFT + S_WIDTH, G_TOP + 42, "Last Game's Score :", 19);
		pDC->TextOut(HS_LEFT + S_WIDTH, G_TOP + 60, "Next Game :", 11);
		pDC->TextOut(HS_LEFT + S_WIDTH, G_TOP + 78, "Games Completed :", 17);
		pDC->TextOut(HS_LEFT + S_WIDTH, G_TOP + 96, "About To Play :", 15);

		pDC->SetTextAlign(nOldTextAlign);
		pDC->SetTextColor(rgbOldColorRef);

	}

	pDC->SetBkMode(nOldBkMode);
	pDC->SelectPalette(pOldPalette, false);   // replace old palette
	ReleaseDC(pDC);                             // release the window's context

}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the QUIT and OKAY buttons when they are clicked.
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *  wParam      identifier for the button to be processed
 *  lParam      type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
bool CMainGTWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	// Redraw rect for Hodj/Podj difficulties
	const CRect r11 = pHSHButton ? pHSHButton->GetWindowRectInParentCoords() : CRect();
	const CRect r12 = pPSNPButton ? pPSNPButton->GetWindowRectInParentCoords() : CRect();
	const CRect cRect1(r11.left, r11.top, r12.right, r12.bottom);
	// Redraw rect for Game Order and everything below it
	const CRect r21 = pGAButton ? pGAButton->GetWindowRectInParentCoords() : CRect();
	const CRect r22 = pLeaveButton ? pLeaveButton->GetWindowRectInParentCoords() : CRect();
	const CRect cRect2(r21.left, r21.top, r22.right, r22.bottom);

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_HODJ_SKILL_HARD:
			pHSHButton->SetCheck(1);
			m_pgtGTStruct->nHodjSkillLevel = SKILLLEVEL_HIGH;
			if (m_pgtGTStruct->bPlayingHodj == false) {
				m_pgtGTStruct->bPlayingHodj = true;
				RedrawWindow(&cRect1);
				RedrawWindow(&cRect2);
			}
			break;
		case IDC_HODJ_SKILL_MEDIUM:
			pHSMButton->SetCheck(1);
			m_pgtGTStruct->nHodjSkillLevel = SKILLLEVEL_MEDIUM;
			if (m_pgtGTStruct->bPlayingHodj == false) {
				m_pgtGTStruct->bPlayingHodj = true;
				RedrawWindow(&cRect1);
				RedrawWindow(&cRect2);
			}
			break;
		case IDC_HODJ_SKILL_EASY:
			pHSLButton->SetCheck(1);
			m_pgtGTStruct->nHodjSkillLevel = SKILLLEVEL_LOW;
			if (m_pgtGTStruct->bPlayingHodj == false) {
				m_pgtGTStruct->bPlayingHodj = true;
				RedrawWindow(&cRect1);
				RedrawWindow(&cRect2);
			}
			break;
		case IDC_HODJ_NO_PLAY:
			if (m_pgtGTStruct->nPodjSkillLevel != NOPLAY) {
				pHSNPButton->SetCheck(1);
				m_pgtGTStruct->nHodjSkillLevel = NOPLAY;
				if (m_pgtGTStruct->bPlayingHodj) {
					m_pgtGTStruct->bPlayingHodj = false;
					RedrawWindow(&cRect1);
					RedrawWindow(&cRect2);
				}
			}
			break;

		case IDC_PODJ_SKILL_HARD:
			pPSHButton->SetCheck(1);
			m_pgtGTStruct->nPodjSkillLevel = SKILLLEVEL_HIGH;
			break;
		case IDC_PODJ_SKILL_MEDIUM:
			pPSMButton->SetCheck(1);
			m_pgtGTStruct->nPodjSkillLevel = SKILLLEVEL_MEDIUM;
			break;
		case IDC_PODJ_SKILL_EASY:
			pPSLButton->SetCheck(1);
			m_pgtGTStruct->nPodjSkillLevel = SKILLLEVEL_LOW;
			break;
		case IDC_PODJ_NO_PLAY:
			if (m_pgtGTStruct->nHodjSkillLevel != NOPLAY) {
				pPSNPButton->SetCheck(1);
				m_pgtGTStruct->nPodjSkillLevel = NOPLAY;
				m_pgtGTStruct->bPlayingHodj = true;
			}
			break;

		case IDC_GAME_ALPHA:
			pGAButton->SetCheck(1);
			m_pgtGTStruct->nGameSelection = GAME_ALPHA;
			RedrawWindow(cRect1);
			RedrawWindow(cRect2);
			break;
		case IDC_GAME_GEO:
			pGGButton->SetCheck(1);
			m_pgtGTStruct->nGameSelection = GAME_GEO;
			RedrawWindow(cRect1);
			RedrawWindow(cRect2);
			break;
		case IDC_GAME_RAND:
			pGRButton->SetCheck(1);
			m_pgtGTStruct->nGameSelection = GAME_RAND;
			RedrawWindow(cRect1);
			RedrawWindow(cRect2);
			break;

		case IDC_AUDIO: {
			CAudioCfgDlg dlgAudioCfg((CWnd *)this, pGamePalette, IDD_AUDIOCFG);
			m_pgtGTStruct->bPlayMusic = GetPrivateProfileInt("Meta", "Music", true, "HODJPODJ.INI");
			m_pgtGTStruct->bPlayFX = GetPrivateProfileInt("Meta", "SoundEffects", true, "HODJPODJ.INI");
		}
		break;

		case IDC_TOP10: {
			CTop10Dlg   cTopTenDlg((CWnd *)this, pGamePalette);
			cTopTenDlg.DoModal();
		}
		break;

		case IDC_PLAY:
			m_pgtGTStruct->bMidGrandTour = true;
			nReturnValue = GetNextGameCode();
			m_pgtGTStruct->nCurrGameCode = nReturnValue;
			PostMessage(WM_CLOSE);
			break;

		case IDC_SAVE:
			SaveCurrGame();
			break;
		case IDC_RESTORE:
			RestoreGame();
			break;

		case IDC_RESET_SCORES:
			ResetScores();
			break;

		case IDC_LEAVE:
			nReturnValue = -1;

			if (bDisplayTopTen)
				SaveScores();

			PostMessage(WM_CLOSE);
			break;
		}
	}

	(*this).SetFocus();                         // Reset focus back to the main window
	return true;
}

void CMainGTWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CWnd::OnLButtonDown(nFlags, point);
}

void CMainGTWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	CWnd::OnMouseMove(nFlags, point);
}

void CMainGTWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	CWnd::OnLButtonUp(nFlags, point);
}

void CMainGTWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMainGTWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CRect   rTempRect;
	char    cAlpha[27];
	int     i;
	uint32       dwTextWidth;
	int         nLeft = SCROLL_LEFT;
	int         nTop = SCROLL_TOP;
	CDC *pDC = nullptr;


	if (nNewRank == -1)
		return;

	Common::strcpy_s(cAlpha, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	pDC = GetDC();
	int nOldBKMode = pDC->SetBkMode(TRANSPARENT);

	dwTextWidth = MFC::GetTextExtent(pDC->m_hDC, astTopTenScores[nNewRank].acName, strlen(astTopTenScores[nNewRank].acName));
	rTempRect.SetRect((nLeft + 70 + LOWORD(dwTextWidth)) - tmWidth - 1, nTop + (nNewRank * 20) + 90 - 1, nLeft + (70 + LOWORD(dwTextWidth) + (tmWidth * 2)) + 1, nTop + (nNewRank * 20) + 110 + 1);

	if ((nChar >= 0x0030) && (nChar <= 0x0039)) { // numbers
		if (nCurChar < 30) {
			astTopTenScores[nNewRank].acName[nCurChar - 1] = nChar;
			astTopTenScores[nNewRank].acName[nCurChar] = '|';
			astTopTenScores[nNewRank].acName[nCurChar + 1] = 0;
			nCurChar++;
			if (bDonePodj == false)
				pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(0, 0, 255));
			else
				pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(255, 0, 0));
		}
	} else {
		if (nChar == 0x0020) { // space
			if (nCurChar < 30) {
				astTopTenScores[nNewRank].acName[nCurChar - 1] = ' ';
				astTopTenScores[nNewRank].acName[nCurChar] = '|';
				astTopTenScores[nNewRank].acName[nCurChar + 1] = 0;
				nCurChar++;
				if (bDonePodj == false)
					pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(0, 0, 255));
				else
					pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(255, 0, 0));
			}
		} else {
			if (nChar == VK_BACK) { // back space
				if (nCurChar > 1) {
					nCurChar--;
					astTopTenScores[nNewRank].acName[nCurChar - 1] = '|';
					astTopTenScores[nNewRank].acName[nCurChar] = 0;
					if (bDonePodj == false)
						pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(0, 0, 255));
					else
						pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(255, 0, 0));
				}
			} else {
				if (nChar == VK_RETURN) {   // return key
					nCurChar--;
					astTopTenScores[nNewRank].acName[nCurChar] = 0;
					if (pText != nullptr) {
						delete pText;
						pText = nullptr;
					}
					SaveScores();
					nCurChar = -1;
					nNewRank = -1;
					rTempRect.SetRect(SCROLL_LEFT + 40, SCROLL_TOP + 50, SCROLL_LEFT + 500, SCROLL_TOP + 320);
					if (bDonePodj == false) {
						for (i = 0; i < 10; i++) {
							if ((m_pgtGTStruct->nPodjSkillLevel != NOPLAY) && (m_pgtGTStruct->nPodjScore > astTopTenScores[i].nScore)) {
								int j;

								nNewRank = i;
								for (j = 10; j > i; j--) {
									Common::strcpy_s(astTopTenScores[j].acName, astTopTenScores[j - 1].acName);
									astTopTenScores[j].nScore = astTopTenScores[j - 1].nScore;
									astTopTenScores[j].nSkillLevel = astTopTenScores[j - 1].nSkillLevel;
								}
								Common::strcpy_s(astTopTenScores[i].acName, "PODJ");
								nCurChar = 5;
								astTopTenScores[i].acName[nCurChar - 1] = '|';
								astTopTenScores[i].acName[nCurChar] = 0;
								astTopTenScores[i].nScore = m_pgtGTStruct->nPodjScore;
								astTopTenScores[i].nSkillLevel = m_pgtGTStruct->nPodjSkillLevel;
								bDonePodj = true;
								pText = new CText();
								cTextRect.SetRect(SCROLL_LEFT + 69, SCROLL_TOP + (nNewRank * 20) + 90, SCROLL_LEFT + 349, SCROLL_TOP + (nNewRank * 20) + 110);
								pText->SetupText(pDC, pGamePalette, &cTextRect, JUSTIFY_LEFT);
								(*this).SetFocus();
								break;
							}
						}
						if (bDonePodj == false) {
							pScoresResetButton->EnableWindow(true);
							pScoresLeaveButton->EnableWindow(true);
						}
					} else {
						pScoresResetButton->EnableWindow(true);
						pScoresLeaveButton->EnableWindow(true);
						pScoresLeaveButton->SetFocus();
					}
					if (pDC != nullptr) {
						pDC->SetBkMode(nOldBKMode);
						ReleaseDC(pDC);
						pDC = nullptr;
					}
					RedrawWindow(&rTempRect);
				} else {
					if ((nChar >= 97) && (nChar <= 122)) { // letters
						if (nCurChar < 30) {

							astTopTenScores[nNewRank].acName[nCurChar - 1] = cAlpha[nChar - 97];
							astTopTenScores[nNewRank].acName[nCurChar] = '|';
							astTopTenScores[nNewRank].acName[nCurChar + 1] = 0;
							nCurChar++;
							if (bDonePodj == false)
								pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(0, 0, 255));
							else
								pText->DisplayString(pDC, astTopTenScores[nNewRank].acName, 16, FW_BOLD, (COLORREF)RGB(255, 0, 0));
						}
					}
				}
			}
		}
	}

	if (pDC != nullptr) {
		pDC->SetBkMode(nOldBKMode);
		ReleaseDC(pDC);
		pDC = nullptr;
	}
}


/*****************************************************************
 *
 * ResetScores
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      resets the top 10 list of top scores
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CMainGTWindow::ResetScores() {
	int     i;
	CRect   rTemp(SCROLL_LEFT + 40, SCROLL_TOP + 50, SCROLL_LEFT + 500, SCROLL_TOP + 320);
	char    cTemp1[64];


	for (i = 0; i < 10; i++) {
		Common::sprintf_s(cTemp1, "acName%i", i);
		WritePrivateProfileString("GRAND TOUR TOP TEN", cTemp1, ":", INI_FILENAME);
		Common::sprintf_s(cTemp1, "nSkillLevel%i", i);
		WritePrivateProfileString("GRAND TOUR TOP TEN", cTemp1, "0", INI_FILENAME);
		Common::sprintf_s(cTemp1, "nScore%i", i);
		WritePrivateProfileString("GRAND TOUR TOP TEN", cTemp1, "0", INI_FILENAME);
	}
	nNewRank = -1;
	nCurChar = -1;
	GetScores();
	RedrawWindow(&rTemp);
	return;
}

/*****************************************************************
 *
 * GetScores
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      gets the top 10 list of top scores from the hodjpodj.ini
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CMainGTWindow::GetScores() {
	int     i;
	char    cTemp1[64];


	for (i = 0; i < 10; i++) {
		Common::sprintf_s(cTemp1, "acName%i", i);
		GetPrivateProfileString("GRAND TOUR TOP TEN", cTemp1, ":", astTopTenScores[i].acName, 20, INI_FILENAME);
		if (astTopTenScores[i].acName[0] == ':') {
			Common::strcpy_s(astTopTenScores[i].acName, " ");
		}
		Common::sprintf_s(cTemp1, "nSkillLevel%i", i);
		astTopTenScores[i].nSkillLevel = GetPrivateProfileInt("GRAND TOUR TOP TEN", cTemp1, 0, INI_FILENAME);
		Common::sprintf_s(cTemp1, "nScore%i", i);
		astTopTenScores[i].nScore = GetPrivateProfileInt("GRAND TOUR TOP TEN", cTemp1, 0, INI_FILENAME);
	}
}

/*****************************************************************
 *
 * SaveScores
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      writes out updates top 10 list of top scores to HODDJPODJ.INI
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CMainGTWindow::SaveScores() {
	int     i;
	char    cTemp1[64];
	char    cTemp2[64];

	for (i = 0; i < 10; i++) {
		Common::sprintf_s(cTemp1, "acName%i", i);
		WritePrivateProfileString("GRAND TOUR TOP TEN", cTemp1, astTopTenScores[i].acName, INI_FILENAME);
		Common::sprintf_s(cTemp1, "nSkillLevel%i", i);
		Common::sprintf_s(cTemp2, "%i", astTopTenScores[i].nSkillLevel);
		WritePrivateProfileString("GRAND TOUR TOP TEN", cTemp1, cTemp2, INI_FILENAME);
		Common::sprintf_s(cTemp1, "nScore%i", i);
		Common::sprintf_s(cTemp2, "%i", astTopTenScores[i].nScore);
		WritePrivateProfileString("GRAND TOUR TOP TEN", cTemp1, cTemp2, INI_FILENAME);
	}
}

/*****************************************************************
 *
 * GetNextGameCode
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      determines the next game to be played
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
int CMainGTWindow::GetNextGameCode(bool bExecute) {
	int i;
	int result = -1;
	bool bThereAreGamesToBePlayed = false;

	if (m_pgtGTStruct->nGameSelection == GAME_ALPHA) {
		for (i = 0; i < 18; i++) {
			if (m_pgtGTStruct->bPlayingHodj) {
				if (m_pgtGTStruct->abHGamePlayed[i] == false) {
					if (bExecute) {
						m_pgtGTStruct->nHodjLastGame = GAME_VALUES[i];
						m_pgtGTStruct->abHGamePlayed[i] = true;
					}
					result = GAME_VALUES[i];
					break;
				}
			} else {
				if (m_pgtGTStruct->abPGamePlayed[i] == false) {
					if (bExecute) {
						m_pgtGTStruct->nPodjLastGame = GAME_VALUES[i];
						m_pgtGTStruct->abPGamePlayed[i] = true;
					}
					result = GAME_VALUES[i];
					break;
				}
			}
		}
	} else {
		if (m_pgtGTStruct->nGameSelection == GAME_GEO) {
			for (i = 0; i < 18; i++) {
				if (m_pgtGTStruct->bPlayingHodj) {
					if (m_pgtGTStruct->abHGamePlayed[anGeoOrder[i]] == false) {
						if (bExecute) {
							m_pgtGTStruct->nHodjLastGame = GAME_VALUES[anGeoOrder[i]];
							m_pgtGTStruct->abHGamePlayed[anGeoOrder[i]] = true;
						}
						result = GAME_VALUES[anGeoOrder[i]];
						break;
					}
				} else {
					if (m_pgtGTStruct->abPGamePlayed[anGeoOrder[i]] == false) {
						if (bExecute) {
							m_pgtGTStruct->nPodjLastGame = GAME_VALUES[anGeoOrder[i]];
							m_pgtGTStruct->abPGamePlayed[anGeoOrder[i]] = true;
						}
						result = GAME_VALUES[anGeoOrder[i]];
						break;
					}
				}
			}
		} else {
			if ((m_pgtGTStruct->bPlayingHodj == false) && (m_pgtGTStruct->nHodjSkillLevel != NOPLAY)) {
				i = 0;
				result = m_pgtGTStruct->nCurrGameCode;
				while (GAME_VALUES[i] != result) {
					i++;
				};
				if (bExecute) {
					m_pgtGTStruct->nPodjLastGame = GAME_VALUES[i];
					m_pgtGTStruct->abPGamePlayed[i] = true;
				}
			} else {
				for (i = 0; i < 18; i++) {
					if (m_pgtGTStruct->bPlayingHodj) {
						if (m_pgtGTStruct->abHGamePlayed[i] == false) {
							bThereAreGamesToBePlayed = true;
							break;
						}
					} else {
						if (m_pgtGTStruct->abPGamePlayed[i] == false) {
							bThereAreGamesToBePlayed = true;
							break;
						}
					}
				}
				if (bThereAreGamesToBePlayed) {
					do {
						i = brand() % 18;
						if (m_pgtGTStruct->bPlayingHodj) {
							if (m_pgtGTStruct->abHGamePlayed[i] == false) {
								if (bExecute) {
									m_pgtGTStruct->nHodjLastGame = GAME_VALUES[i];
									m_pgtGTStruct->abHGamePlayed[i] = true;
								}
								result = GAME_VALUES[i];
							}
						} else {
							if (m_pgtGTStruct->abPGamePlayed[i] == false) {
								if (bExecute) {
									m_pgtGTStruct->nPodjLastGame = GAME_VALUES[i];
									m_pgtGTStruct->abPGamePlayed[i] = true;
								}
								result = GAME_VALUES[i];
							}
						}
					} while (result == -1);
				}
			}
		}
	}

	return result;
}

/*****************************************************************
 *
 * SaveCurrGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Save current game to HODJPODJ.INI file
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CMainGTWindow::SaveCurrGame() {
	int     i;
	char    cTemp[64];

	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->bPlayingHodj);
	WritePrivateProfileString("GRAND TOUR", "bPlayingHodj", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->bPlayMusic);
	WritePrivateProfileString("GRAND TOUR", "bPlayMusic", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->bPlayFX);
	WritePrivateProfileString("GRAND TOUR", "bPlayFX", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->bMidGrandTour);
	WritePrivateProfileString("GRAND TOUR", "bMidGrandTour", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nHodjSkillLevel);
	WritePrivateProfileString("GRAND TOUR", "nHodjSkillLevel", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nPodjSkillLevel);
	WritePrivateProfileString("GRAND TOUR", "nPodjSkillLevel", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nGameSelection);
	WritePrivateProfileString("GRAND TOUR", "nGameSelection", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nCurrGameCode);
	WritePrivateProfileString("GRAND TOUR", "nCurrGameCode", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nHodjScore);
	WritePrivateProfileString("GRAND TOUR", "nHodjScore", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nPodjScore);
	WritePrivateProfileString("GRAND TOUR", "nPodjScore", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nHodjLastGame);
	WritePrivateProfileString("GRAND TOUR", "nHodjLastGame", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nPodjLastGame);
	WritePrivateProfileString("GRAND TOUR", "nPodjLastGame", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nHodjLastScore);
	WritePrivateProfileString("GRAND TOUR", "nHodjLastScore", cTemp, INI_FILENAME);
	Common::sprintf_s(cTemp, "%i", m_pgtGTStruct->nPodjLastScore);
	WritePrivateProfileString("GRAND TOUR", "nPodjLastScore", cTemp, INI_FILENAME);

	for (i = 0; i < 18; i++) {
		Common::sprintf_s(cTemp, "abHGamePlayed[%i]", i);
		if (m_pgtGTStruct->abHGamePlayed[i]) {
			WritePrivateProfileString("GRAND TOUR", cTemp, "1", INI_FILENAME);
		} else {
			WritePrivateProfileString("GRAND TOUR", cTemp, "0", INI_FILENAME);
		}
		Common::sprintf_s(cTemp, "abPGamePlayed[%i]", i);
		if (m_pgtGTStruct->abPGamePlayed[i]) {
			WritePrivateProfileString("GRAND TOUR", cTemp, "1", INI_FILENAME);
		} else {
			WritePrivateProfileString("GRAND TOUR", cTemp, "0", INI_FILENAME);
		}
	}

	// Flush settings changes
	AfxGetApp()->SaveAllModified();
}

/*****************************************************************
 *
 * RestoreGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Restores previously saved game from the HODJPODJ.INI
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
void CMainGTWindow::RestoreGame() {
	int     i;
	char    cTemp[64];
	CRect   cRect1(SCROLL_LEFT + 50, PSNP_TOP + S_HEIGHT + 1, SCROLL_LEFT + 500, G_TOP - 21);
	CRect   cRect2(SCROLL_LEFT + 50, G_TOP + 23, SCROLL_LEFT + 500, F_TOP - 26);

	pHSHButton->EnableWindow(true);
	pHSMButton->EnableWindow(true);
	pHSLButton->EnableWindow(true);
	pHSNPButton->EnableWindow(true);
	pPSHButton->EnableWindow(true);
	pPSMButton->EnableWindow(true);
	pPSLButton->EnableWindow(true);
	pPSNPButton->EnableWindow(true);
	pGAButton->EnableWindow(true);
	pGGButton->EnableWindow(true);
	pGRButton->EnableWindow(true);

	m_pgtGTStruct->bPlayingHodj = GetPrivateProfileInt("GRAND TOUR", "bPlayingHodj", 1, INI_FILENAME);
	m_pgtGTStruct->bPlayMusic = GetPrivateProfileInt("GRAND TOUR", "bPlayMusic", 1, INI_FILENAME);
	m_pgtGTStruct->bPlayFX = GetPrivateProfileInt("GRAND TOUR", "bPlayFX", 1, INI_FILENAME);
	m_pgtGTStruct->bMidGrandTour = GetPrivateProfileInt("GRAND TOUR", "bMidGrandTour", 0, INI_FILENAME);
	m_pgtGTStruct->nHodjScore = GetPrivateProfileInt("GRAND TOUR", "nHodjScore", 0, INI_FILENAME);
	m_pgtGTStruct->nPodjScore = GetPrivateProfileInt("GRAND TOUR", "nPodjScore", 0, INI_FILENAME);
	m_pgtGTStruct->nHodjSkillLevel = GetPrivateProfileInt("GRAND TOUR", "nHodjSkillLevel", 0, INI_FILENAME);
	m_pgtGTStruct->nPodjSkillLevel = GetPrivateProfileInt("GRAND TOUR", "nPodjSkillLevel", 0, INI_FILENAME);
	m_pgtGTStruct->nGameSelection = GetPrivateProfileInt("GRAND TOUR", "nGameSelection", 0, INI_FILENAME);
	m_pgtGTStruct->nCurrGameCode = GetPrivateProfileInt("GRAND TOUR", "nCurrGameCode", 0, INI_FILENAME);
	m_pgtGTStruct->nHodjLastGame = GetPrivateProfileInt("GRAND TOUR", "nHodjLastGame", -1, INI_FILENAME);
	m_pgtGTStruct->nPodjLastGame = GetPrivateProfileInt("GRAND TOUR", "nPodjLastGame", -1, INI_FILENAME);
	m_pgtGTStruct->nHodjLastScore = GetPrivateProfileInt("GRAND TOUR", "nHodjLastScore", 0, INI_FILENAME);
	m_pgtGTStruct->nPodjLastScore = GetPrivateProfileInt("GRAND TOUR", "nPodjLastScore", 0, INI_FILENAME);

	for (i = 0; i < 18; i++) {
		Common::sprintf_s(cTemp, "abHGamePlayed[%i]", i);
		m_pgtGTStruct->abHGamePlayed[i] = GetPrivateProfileInt("GRAND TOUR", cTemp, 0, INI_FILENAME);
		Common::sprintf_s(cTemp, "abPGamePlayed[%i]", i);
		m_pgtGTStruct->abPGamePlayed[i] = GetPrivateProfileInt("GRAND TOUR", cTemp, 0, INI_FILENAME);
	}

	if (m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_LOW) {
		pHSLButton->SetCheck(1);
		pHSMButton->SetCheck(0);
		pHSHButton->SetCheck(0);
		pHSNPButton->SetCheck(0);
	} else {
		if (m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_MEDIUM) {
			pHSLButton->SetCheck(0);
			pHSMButton->SetCheck(1);
			pHSHButton->SetCheck(0);
			pHSNPButton->SetCheck(0);
		} else {
			if (m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_HIGH) {
				pHSLButton->SetCheck(0);
				pHSMButton->SetCheck(0);
				pHSHButton->SetCheck(1);
				pHSNPButton->SetCheck(0);
			} else {
				pHSLButton->SetCheck(0);
				pHSMButton->SetCheck(0);
				pHSHButton->SetCheck(0);
				pHSNPButton->SetCheck(1);
			}
		}
	}

	if (m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_LOW) {
		pPSLButton->SetCheck(1);
		pPSMButton->SetCheck(0);
		pPSHButton->SetCheck(0);
		pPSNPButton->SetCheck(0);
	} else {
		if (m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_MEDIUM) {
			pPSLButton->SetCheck(0);
			pPSMButton->SetCheck(1);
			pPSHButton->SetCheck(0);
			pPSNPButton->SetCheck(0);
		} else {
			if (m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_HIGH) {
				pPSLButton->SetCheck(0);
				pPSMButton->SetCheck(0);
				pPSHButton->SetCheck(1);
				pPSNPButton->SetCheck(0);
			} else {
				pPSLButton->SetCheck(0);
				pPSMButton->SetCheck(0);
				pPSHButton->SetCheck(0);
				pPSNPButton->SetCheck(1);
			}
		}
	}

	if (m_pgtGTStruct->nGameSelection == GAME_ALPHA) {
		pGAButton->SetCheck(1);
		pGGButton->SetCheck(0);
		pGRButton->SetCheck(0);
	} else {
		if (m_pgtGTStruct->nGameSelection == GAME_GEO) {
			pGAButton->SetCheck(0);
			pGGButton->SetCheck(1);
			pGRButton->SetCheck(0);
		} else {
			pGAButton->SetCheck(0);
			pGGButton->SetCheck(0);
			pGRButton->SetCheck(1);
		}
	}

	if (m_pgtGTStruct->bMidGrandTour) {

		pHSHButton->EnableWindow(false);
		pHSMButton->EnableWindow(false);
		pHSLButton->EnableWindow(false);
		pHSNPButton->EnableWindow(false);
		pPSHButton->EnableWindow(false);
		pPSMButton->EnableWindow(false);
		pPSLButton->EnableWindow(false);
		pPSNPButton->EnableWindow(false);
		pGAButton->EnableWindow(false);
		pGGButton->EnableWindow(false);
		pGRButton->EnableWindow(false);

	}

	RedrawWindow(&cRect1);
	RedrawWindow(&cRect2);
}

/*****************************************************************
 *
 * OnXXXXXX
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      These functions are called when ever the corresponding WM_
 *      event message is generated for the mouse.
 *
 *      (Add game-specific processing)
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/

bool CMainGTWindow::OnEraseBkgnd(CDC *pDC) {
// eat this
	return true;
}


void CMainGTWindow::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {
	bool    bUpdateNeeded;

	switch (nState) {
	case WA_INACTIVE:
		bActiveWindow = false;
		BagelMetaEngine::setKeybindingMode(KBMODE_NORMAL);
		break;

	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = true;
		bUpdateNeeded = GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			InvalidateRect(nullptr, false);

		BagelMetaEngine::setKeybindingMode(KBMODE_MINIMAL);
		break;

	default:
		break;
	}
}


/*****************************************************************
 *
 * OnClose
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when a Close event is generated.  For
 *  this sample application we need only kill our event timer;
 *  The ExitInstance will handle releasing resources.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

void CMainGTWindow::OnClose() {
	CDC *pDC = GetDC();
	CRect       rctFillRect(0, 0, 640, 480);
	CBrush  Brush(RGB(0, 0, 0));

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseDC(pDC);
	ReleaseResources();
	CFrameWnd::OnClose();
}

/*****************************************************************
 *
 * OnDestroy
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when after the window has been destroyed.
 *  For poker, we post a message bak to the calling app to tell it
 * that the user has quit the game, and therefore the app can unload
 * this DLLL
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMainGTWindow::OnDestroy() {
	//  send a message to the calling app to tell it the user has quit the game

	m_pgtGTStruct->stMiniGame.lCrowns = 0;
	m_pgtGTStruct->stMiniGame.lScore = 0;
	m_pgtGTStruct->stMiniGame.bPlayingMetagame = true;
	m_pgtGTStruct->stMiniGame.bPlayingHodj = m_pgtGTStruct->bPlayingHodj;
	m_pgtGTStruct->stMiniGame.bSoundEffectsEnabled = m_pgtGTStruct->bPlayFX;
	m_pgtGTStruct->stMiniGame.bMusicEnabled = m_pgtGTStruct->bPlayMusic;

	if (m_pgtGTStruct->bPlayingHodj)
		m_pgtGTStruct->stMiniGame.nSkillLevel = m_pgtGTStruct->nHodjSkillLevel;
	else
		m_pgtGTStruct->stMiniGame.nSkillLevel = m_pgtGTStruct->nPodjSkillLevel;

	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)nReturnValue);
	CFrameWnd::OnDestroy();
}


/*****************************************************************
 *
 * ReleaseResources
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Release all resources that were created and retained during the
 *  course of the game.  This includes sprites in the sprite chain,
 *  the game color palette, and button controls.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

#define DEL(X) delete X; X = nullptr

void CMainGTWindow::ReleaseResources() {
	DEL(pText);

	DEL(pScoresResetButton);
	DEL(pScoresLeaveButton);
	DEL(pPlayButton);
	DEL(pSaveButton);
	DEL(pRestoreButton);
	DEL(pLeaveButton);

	DEL(pHSHButton);
	DEL(pHSMButton);
	DEL(pHSLButton);
	DEL(pHSNPButton);
	DEL(pPSHButton);
	DEL(pPSMButton);
	DEL(pPSLButton);
	DEL(pPSNPButton);
	DEL(pGAButton);
	DEL(pGGButton);
	DEL(pGRButton);

	DEL(pAudioButton);
	DEL(pTop10Button);
	DEL(pSplashScreen);

	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();         // release the game color palette
		DEL(pGamePalette);	
	}
}


/*****************************************************************
 *
 * FlushInputEvents
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Remove all keyboard and mouse related events from the message
 *  so that they will not be sent to us for processing; i.e. this
 *  flushes keyboard type ahead and extra mouse clicks and movement.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/

void CMainGTWindow::FlushInputEvents() {
	MSG msg;

	while (true) {                                      // find and remove all keyboard events
		if (!PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	while (true) {                                       // find and remove all mouse events
		if (!PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}

// CMainGTWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainGTWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainGTWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
