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
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/pdq/game.h"
#include "bagel/hodjnpodj/pdq/usercfg.h"
#include "bagel/hodjnpodj/pdq/main.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

/*
*
* Local data types
*
*/
typedef struct {
	char   text[MAX_PLENGTH_S + 1];
	byte  order[MAX_PLENGTH];
} PHRASES;

typedef struct {
	unsigned int     gameSpeed;             // 1 to 5 = 2.5 to .5 seconds
	unsigned int     nShown;                // 1 to 9   as of 9/9/94 0 to 6
	bool  bRandomLetters;         // true if letter ordering shall be random
	bool  bShowNames;            // true if we are to display category names
} USERCFG;

typedef struct {
	CSprite *sprite;
	bool bUsed;
} SPRITE_LIST;


STATIC const char *pszCategorySounds[N_CATEGORIES] = {
	".\\SOUND\\TGG5.WAV",
	".\\SOUND\\TGG6.WAV",
	".\\SOUND\\TGG7.WAV",
	".\\SOUND\\TGG8.WAV"
};


/*
*
* Local prototypes
*
*/
ERROR_CODE  CleanScreen(CDC *);
ERROR_CODE  LoadNewPhrase();
ERROR_CODE  BuildSpriteList(CDC *);
void        KillCurPhrase();
void        BuildRandomPhraseOrder();
bool     RevealNextLetter();
void CALLBACK GameTimerHook(HWND, unsigned int, uintptr, uint32);
int         StrLenNoSpaces(const char *);
int         GetIndex(CSprite *);
void        LoadGameCfg();
void        SaveGameCfg();
ERROR_CODE  ValidatePhrase(PHRASES *);
void        UpdateScore(unsigned int, unsigned int, unsigned int, unsigned int);
int         NumLinkedSprites();

#define TIMER_ID        50
#define START_X_ODD     318
#define START_X_EVEN    334
#define LETTER_SPACING  3
#define SIGN_LENGTH     460
#define SIGN_START_X    93
#define LETTER_START_Y  78
#define DATA_FILE       "PHRASES.DAT"

#define TYPE_PERSON 0
#define TYPE_PLACE  1
#define TYPE_PHRASE 2
#define TYPE_TITLE  3

#define START_TITLE    0
#define START_PERSON 103
#define START_PHRASE 172
#define START_PLACE  197

extern CPalette *pMyGamePalette;
extern bool bInGame;
extern CMainWindow *gMain;
extern LPGAMESTRUCT pGameParams;

/*
* Globals
*/
USERCFG     gGameCfg;
PHRASES    *curPhrase, gPhrase;
SPRITE_LIST spriteList[MAX_PLENGTH];
int         iNextLetter;
int         timerInterval;
HWND        gGameWnd;
bool     bPause;
unsigned int        gLeftAvg, gTotalAvg;
const char *INI_SECTION = "ThGesngGme";
int         nPhrasePixelLength;


/**
*  name      LoadNewPhrase - loads a new phrase from the data store
*
*  synopsis  LoadNewPhrase()
*
*
*  purpose   To randomly choose a new phrase for the game
*
*
*  returns   errCode - Error return code
*
**/
ERROR_CODE LoadNewPhrase() {
	STATIC int nLast;
	char *p, buf[MAX_PLENGTH_S + 2];
	int i, n, nType;
	ERROR_CODE errCode;

	/* assume no error */
	errCode = ERR_NONE;

	/* reset letter index */
	iNextLetter = 0;

	curPhrase = &gPhrase;

	/*
	* Load a new randomly selected phrase from the data store.
	*/

	/* determine the number of phrases in the data store */
	n = (int)(FileLength(DATA_FILE) / sizeof(PHRASES));

	if (n > 0) {

		/*
		* pick one at random, but not same one twice in a row
		*/
		do {
			i = brand() % n;
		} while (i == nLast);
		nLast = i;

		nType = 0;
		if (i >= START_TITLE && i < START_PERSON) {
			nType = TYPE_TITLE;

		} else if (i >= START_PERSON && i < START_PHRASE) {
			nType = TYPE_PERSON;

		} else if (i >= START_PHRASE && i < START_PLACE) {
			nType = TYPE_PHRASE;

		} else if (i >= START_PLACE) {
			nType = TYPE_PLACE;
		}

		// Are we supposed to show the category names?
		//
		if (gGameCfg.bShowNames) {

			// display category
			gMain->PaintCategory(nType);

			// play category narration
			sndPlaySound(pszCategorySounds[nType], SND_ASYNC);
		}

		ifstream inFile;
		inFile.open(DATA_FILE, ios::binary);                // open the data store

		inFile.seekg(i * sizeof(PHRASES));                  // seek to the phrase we want

		inFile.read((char *)curPhrase, sizeof(PHRASES));    // load that phrase
		if (inFile.gcount() != sizeof(PHRASES))
			errCode = ERR_FREAD;
		inFile.close();                                     // close the data store

		if (!errCode) {

			if ((errCode = ValidatePhrase(curPhrase)) == ERR_NONE) {

				/*
				* convert all instances of "the", "an", and "a" to upper case
				*/
				Common::sprintf_s(p = buf, MAX_PLENGTH_S + 2, " %s",
				                  strLower(curPhrase->text));

				StrUprStr(p, " the ");
				StrUprStr(p, " an ");
				StrUprStr(p, " a ");
				p++;
				Common::strcpy_s(curPhrase->text, p);

				/*
				* if user wants a random ordering
				*/
				if (gGameCfg.bRandomLetters)
					BuildRandomPhraseOrder();
			}
		}
	} else {
		errCode = ERR_FFIND;
	}

	return errCode;
}


/**
*  name      BuildRandomPhraseOrder - randomizes the order of the letters to
*                                     be revealed.
*
*  synopsis  BuildRandomPhraseOrder()
*
*
*  purpose   To provide a random ordering of letters that are revealed with
*            RevealNextLetter() instead of the fixed order prescribed in the
*            data store.
*
*  returns   Nothing
*
**/
void BuildRandomPhraseOrder() {
	byte *curPhraseOrder;
	int i, j, n, newIndex;
	bool use;

	/*
	* if there is a current active game
	*/
	if (curPhrase != nullptr) {

		curPhraseOrder = curPhrase->order;

		memset(curPhraseOrder, 0, MAX_PLENGTH * sizeof(byte));

		/*
		* each entry must be a unique index into the spriteList
		*/
		n = StrLenNoSpaces(curPhrase->text);
		for (i = 0; i < n; i++) {
			do {
				use = true;
				newIndex = brand() % n + 1;
				for (j = 0; j < i; j++) {
					if (curPhraseOrder[j] == newIndex) {
						use = false;
						break;
					}
				}
			} while (!use);

			curPhraseOrder[i] = (byte)newIndex;
		}
	}
}


/**
*  name      BuildSpriteList - builds a sprite list from curPhrase
*
*  synopsis  BuildSpriteList(pDC)
*            CDC *pDC                   pointer to this game's Device Context
*
*
*  purpose   To fill an array of pointers to sprites that represent letters
*            from the currently loaded phrase.
*
*  returns   errCode = Error return code
*
**/
ERROR_CODE BuildSpriteList(CDC *pDC) {
	CSprite *pNewSprite;
	char *pText;
	int i;
	ERROR_CODE errCode;

	/* can't access null pointers */
	assert(pDC != nullptr);

	/* assume no error */
	errCode = ERR_NONE;

	/*
	* if there is a game currently active
	*/
	if (curPhrase != nullptr) {

		if (pDC == nullptr) {
			errCode = ERR_UNKNOWN;

		} else {

			/* use local pointer to this global object */
			pText = curPhrase->text;

			memset(spriteList, 0, sizeof(SPRITE_LIST) * MAX_PLENGTH);

			/*
			* build a sprite list for this phrase
			*/
			i = 0;
			while (*pText != '\0') {

				/*
				* don't include spaces
				*/
				if (*pText != ' ') {

					/*
					* create a new sprite for this letter
					*/
					if ((spriteList[i].sprite = pNewSprite = new CSprite) == nullptr) {
						errCode = ERR_MEMORY;
						break;

					} else {

						spriteList[i].bUsed = false;

						/*
						* load this letter's bitmap into the sprite
						*/
						if (pNewSprite->LoadResourceSprite(pDC, toupper(*pText) +36) == false) {
							errCode = ERR_UNKNOWN;
							break;

						} else {

							if (pNewSprite->SharePalette(pMyGamePalette) == false) {
								errCode = ERR_UNKNOWN;
								break;

							} else {

								pNewSprite->SetMasked(true);
								pNewSprite->SetMobile(true);
								i++;
							}
						}
					}
				}
				pText++;
			}
		}
	}

	return errCode;
}


/**
*  name      KillCurPhrase - destroys all valid sprites from the sprite list
*
*  synopsis  KillCurPhrase()
*
*
*  purpose   To de-allocate any sprites in the sprite list that have not yet
*            been revealed.  NOTE: this must be called to de-allocate
*            sprites that were allocated via BuildSpriteList().
*
*  returns   Nothing
*
**/
void KillCurPhrase() {
	int i, n;

	if (curPhrase != nullptr) {

		/*
		* delete each of the remaining letters that have not yet been revealed
		*/
		n = StrLenNoSpaces(curPhrase->text);
		for (i = 0; i < n; i++) {
			if ((spriteList[i].sprite != nullptr) && !spriteList[i].bUsed) {
				delete spriteList[i].sprite;
				spriteList[i].sprite = nullptr;
			}
		}
		curPhrase =  nullptr;
	}
}


/**
*  name      RevealNextLetter() - reveals the next letter in the phrase
*
*  synopsis  done = RevealNextLetter()
*
*
*  purpose   To display the next available letter from the sprite list
*
*
*  returns   false if this was the last letter to be revealed, else true
*
**/
bool RevealNextLetter() {
	CSize size;
	int index;
	bool lastLetter;

	lastLetter = false;
	if (curPhrase != nullptr) {

		/* get next valid letter to reveal */
		index = curPhrase->order[iNextLetter++] - 1;

		/* validate the index */
		assert((index >= 0) && (index < MAX_PLENGTH));

		/* validate this sprite */
		assert(spriteList[index].sprite != nullptr);
		assert(spriteList[index].bUsed != true);

		/*
		* add this letter to the list
		*/
		spriteList[index].sprite->LinkSprite();
		spriteList[index].bUsed = true;
		size = spriteList[index].sprite->GetSize();
		nPhrasePixelLength += size.cx + LETTER_SPACING;

		if (iNextLetter >= StrLenNoSpaces(curPhrase->text))
			lastLetter = true;
	}

	return (lastLetter);
}


/**
*  name      RecalcDisplay - re-paints all sprites that are visible
*
*  synopsis  RecalcDisplay(pDC)
*            CDC *pDC                   pointer to this game's Device Context
*
*  purpose   To re-fresh all on screen sprites
*
*
*  returns   errCode = Error return code
*
**/
ERROR_CODE RecalcDisplay(CDC *pDC) {
	CSize size;
	int i, gap, last, n;
	ERROR_CODE errCode;

	/* can't use a null pointer */
	assert(pDC != nullptr);

	/* assume no error */
	errCode = ERR_NONE;

	/*
	* as long as there is a currently active unpaused game, we can paint some letters
	*/
	if (bInGame) {

		if (pDC == nullptr) {
			errCode = ERR_UNKNOWN;
		} else {

			assert(nPhrasePixelLength > 0);

			if (nPhrasePixelLength > 0) {

				/* clean up the screan so we can re-paint the sprites */
				CSprite::EraseSprites(pDC);

				n = NumLinkedSprites();

				gap = (SIGN_LENGTH - nPhrasePixelLength) / 2;
				last = SIGN_START_X + gap;
				for (i = 0; i < MAX_PLENGTH; i++) {
					if (spriteList[i].bUsed) {
						assert(spriteList[i].sprite != nullptr);
						spriteList[i].sprite->PaintSprite(pDC, last, LETTER_START_Y);
						size = spriteList[i].sprite->GetSize();

						// letter spacing is greater when there are less letters
						last += size.cx + (LETTER_SPACING + 5 - (n + 4) / 5);
					}
				}
			}
		}
	}

	return errCode;
}

ERROR_CODE RepaintSpriteList(CDC *pDC) {
	CSprite *pSprite;
	ERROR_CODE errCode;

	/* can't use a null pointer */
	assert(pDC != nullptr);

	/* assume no error */
	errCode = ERR_NONE;

	if (pDC == nullptr) {
		errCode = ERR_UNKNOWN;
	} else {

		/*
		* Paint each sprite
		*/
		pSprite = CSprite::GetSpriteChain();
		while (pSprite) {

			pSprite->ClearBackground();
			pSprite->RefreshSprite(pDC);

			pSprite = pSprite->GetNextSprite();
		}
	}
	return errCode;
}

int GetIndex(CSprite *pSprite) {
	int i;

	assert(pSprite != nullptr);

	for (i = 0; i < MAX_PLENGTH; i++) {
		if (pSprite == spriteList[i].sprite) {
			break;
		}
	}

	assert(i >= 0 && i < MAX_PLENGTH);

	if (i >= MAX_PLENGTH)
		i = -1;

	return (i);
}


/**
*  name      InitGame - Initialize the game parameters
*
*  synopsis  status = InitGame(pDc);
*            CDC *pDC                   pointer to this game's Device Context
*
*
*  purpose   To initilize game parameters - This must be done before each new
*            game is played
*
*  returns   errCode = Error return code
*
**/
ERROR_CODE InitGame(HWND hWnd, CDC *pDC) {
	int i;
	ERROR_CODE errCode;

	assert(pDC != nullptr);

	/* assume no error */
	errCode = ERR_NONE;

	/* keep a copy of this game's window handle */
	gGameWnd = hWnd;

	if (pDC == nullptr) {
		errCode = ERR_UNKNOWN;

	} else {

		/* clear the screen of any previous games */
		EndGame(pDC);

		/* load INI settings */
		LoadGameCfg();

		/*
		* load a new randomly selected phrase as the current phrase
		*/
		if ((errCode = LoadNewPhrase()) == ERR_NONE) {

			if ((errCode = BuildSpriteList(pDC)) == ERR_NONE) {
				// Remove category name from screen
				if (gGameCfg.bShowNames)
					gMain->EraseCategory();

				/* reveal 1st 3 letters */
				for (i = 0; i < (int)gGameCfg.nShown; i++) {
					RevealNextLetter();
				}
			}
		}
	}

	return errCode;
}


/**
*  name      CleanScreen - removes all sprites from the screen
*
*  synopsis  CleanScreen(pDC)
*            CDC *pDC                   pointer to this game's Device Context
*
*  purpose   To refresh the screen by erasing all sprites
*
*
*  returns   errCode = Error return code
*
**/
ERROR_CODE CleanScreen(CDC *pDC) {
	ERROR_CODE errCode;

	assert(pDC != nullptr);

	/* assume no error */
	errCode = ERR_NONE;

	if (pDC == nullptr) {
		errCode  = ERR_UNKNOWN;
	} else {

		KillCurPhrase();
		CSprite::EraseSprites(pDC);
		CSprite::FlushSpriteChain();
	}

	return errCode;
}


/**
*  name      GameTimerHook - hooks WM_TIMER message for PDQ
*
*  synopsis  GameTimerHook(hWnd, nMsg, nEventID, dwTime)
*            HWND  hWnd                 Handle of window that received WM_TIMER
*            unsigned int  nMsg                 WM_TIMER
*            unsigned int  nEventID             TIMER_ID
*            uint32 dwTime               time that event occured
*
*  purpose   To provide a callback routine for hooking the WM_TIMER.
*            Usage: SetTimer(hWnd, tID, time, GameTimerHook);
*            We do not call this routine, Windows does.
*
*  returns   Nothing
*
**/
void CALLBACK GameTimerHook(HWND hWnd, unsigned int, uintptr nEventID, uint32) {
	CDC *pDC;
	HDC hDC;
	bool done;
	unsigned int nLeft, nTotal, nLeftAvg, nTotalAvg;

	assert(nEventID == TIMER_ID);

	if (bInGame && !bPause) {

		hDC = GetDC(hWnd);

		pDC = CDC::FromHandle(hDC);

		/* beep once for each new letter revealed */
		if (pGameParams->bSoundEffectsEnabled)
			sndPlaySound(WAV_REVEAL, SND_ASYNC);

		/* reveal another letter */
		done = RevealNextLetter();

		/* re-calculate letter positions */

		/* refresh screen by displaying all letters/sprites in list */
		RecalcDisplay(pDC);

		GameGetScore(&nLeft, &nTotal, &nLeftAvg, &nTotalAvg);
		UpdateScore(nLeft, nTotal, nLeftAvg, nTotalAvg);

		/*
		* if this was the last letter to be revealed, then user has lost the game
		*/
		if (done) {
			gTotalAvg += StrLenNoSpaces(curPhrase->text);

			GameGetScore(&nLeft, &nTotal, &nLeftAvg, &nTotalAvg);
			UpdateScore(nLeft, nTotal, nLeftAvg, nTotalAvg);

			bInGame = false;
			GameStopTimer();
			if (pGameParams->bSoundEffectsEnabled)
				sndPlaySound(WAV_GAMEOVER, SND_ASYNC);

			CMessageBox dlgGameOver((CWnd *)gMain, pMyGamePalette, "Game over.", "You have lost.");
			gMain->SetFocus();

			CleanScreen(pDC);

			pGameParams->lScore += (100 * nLeft) / nTotal;

			// if in metagame then quit dll when game is over
			//
			if ((pGameParams->bPlayingMetagame) && (gMain->m_nTurnCount == MAX_TURNS)) {
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			} else {
				gMain->PlayGame();
			}
		}

		ReleaseDC(hWnd, hDC);
	}
}


/**
*  name      GameStopTimer - stops the timer for the current game
*
*  synopsis  GameStopTimer()
*
*  purpose   To stop or pause the Game Timer (TIMER_ID)
*
*
*  returns   Nothing
*
**/
void GameStopTimer() {
	KillTimer(gGameWnd, TIMER_ID);
}


ERROR_CODE GameStartTimer() {
	ERROR_CODE errCode = ERR_NONE;

	if (curPhrase != nullptr) {
		if (SetTimer(gGameWnd, TIMER_ID, timerInterval, GameTimerHook) != TIMER_ID)
			errCode = ERR_UNKNOWN;
	}

	return errCode;
}

void GamePauseTimer() {
	bPause = true;
}

void GameResumeTimer() {
	bPause = false;
}


/**
*  name      StartGame - starts the game by activating the timer
*
*  synopsis  StartGame(hWnd, pDC)
*            HWND hWnd
*            CDC *pDC
*
*  purpose
*
*
*  returns   errCode = Error return code
*
**/
ERROR_CODE StartGame(CDC *pDC) {
	ERROR_CODE errCode;

	assert(pDC != nullptr);

	/*
	* start the timer
	*/
	if ((errCode = GameStartTimer()) == ERR_NONE) {

		/* show the first 3 letters */
		RecalcDisplay(pDC);
	}

	return errCode;
}


/**
*  name      EndGame - ends the currently loaded game (performs cleanup, etc...)
*
*  synopsis  EndGame(pDC)
*            CDC *pDC
*
*  purpose   To Halt, and clean up the current game
*
*
*  returns   errCode = Error return code
*
**/
ERROR_CODE EndGame(CDC *pDC) {
	ERROR_CODE errCode;

	assert(pDC != nullptr);

	/* assume no error */
	errCode = ERR_NONE;

	if (pDC == nullptr) {
		errCode = ERR_UNKNOWN;
	} else {

		/* reset phrase length in pixels */
		nPhrasePixelLength = 0;

		/* stop timer */
		GameStopTimer();

		/* clean up the screen */
		CleanScreen(pDC);
	}

	return errCode;
}

void WinGame() {
	unsigned int n;

	// update the final score
	//
	gTotalAvg += (n = StrLenNoSpaces(curPhrase->text));
	gLeftAvg += n - iNextLetter;
}


/**
*  name      GetGameParams - Retrieves game preferences from the user
*
*  synopsis  GetGameParams()
*
*
*  purpose   To get user input for the game parameters (via a dialog box)
*
*
*  returns   Nothing
*
**/
void CALLBACK GetGameParams(CWnd *pParentWnd) {
	/*
	* Our user preference dialog box is self contained in this object
	*/
	CUserCfgDlg dlgUserCfg(pParentWnd, pMyGamePalette, IDD_USERCFG);

	//dlgUserCfg.DoModal();
}


/**
*  name      LoadGameCfg - Loads game configuration info from INI file
*
*  synopsis  LoadGameCfg();
*
*
*  purpose   To load configuration information from this game's INI file
*
*
*  returns   Nothing
*
**/
void LoadGameCfg() {
	char buf[10];
	int n;

	if (pGameParams->bPlayingMetagame) {

		// set defaults
		//
		gGameCfg.bRandomLetters = false;
		gGameCfg.nShown = SHOWN_DEF;
		gGameCfg.bShowNames = true;

		switch (pGameParams->nSkillLevel) {

		case SKILLLEVEL_LOW:
			gGameCfg.gameSpeed = 3; //2;
			break;

		case SKILLLEVEL_MEDIUM:
			gGameCfg.gameSpeed = 6; //5;
			break;

		case SKILLLEVEL_HIGH:
			gGameCfg.gameSpeed = 9; //8;
			break;

		default:
			assert(0);
			break;
		}

	} else {

		/*
		* User can specify if he/she wants the letters to appear in a random order
		* or in the predefined fixed order set by the MetaGame
		*/
		GetPrivateProfileString(INI_SECTION, "RandomLetters", "No", buf, 10, INI_FILENAME);
		assert(strlen(buf) < 10);
		gGameCfg.bRandomLetters = false;
		if (!scumm_stricmp(buf, "Yes"))
			gGameCfg.bRandomLetters = true;

		/*
		* get the number of letters that are intially displayed (default is SHOWN_DEF = 3)
		*/
		n = gGameCfg.nShown = GetPrivateProfileInt(INI_SECTION, "NumStartingLetters", SHOWN_DEF, INI_FILENAME);

		/* validate this setting */
		if ((n < SHOWN_MIN) || (n > SHOWN_MAX))
			gGameCfg.nShown = SHOWN_DEF;

		/*
		* Get the game speed (1..10) (default is SPEED_DEF = 8)
		*/
		n = gGameCfg.gameSpeed = GetPrivateProfileInt(INI_SECTION, "GameSpeed", SPEED_DEF, INI_FILENAME);

		/* validate this setting */
		if ((n < SPEED_MIN) || (n > SPEED_MAX))
			gGameCfg.gameSpeed = SPEED_DEF;

		GetPrivateProfileString(INI_SECTION, "ShowCategoryNames", "Yes", buf, 10, INI_FILENAME);
		assert(strlen(buf) < 10);
		gGameCfg.bShowNames = false;
		if (!scumm_stricmp(buf, "Yes"))
			gGameCfg.bShowNames = true;
	}

	timerInterval = (10 - gGameCfg.gameSpeed) * 500 + 500;
}


/**
*  name      SaveGameCfg - Saves configuraton info
*
*  synopsis  SaveGameCfg();
*
*
*  purpose   To Save this game's configuration info to an INI file.
*            The INI file contains user defineable aspects of this game.
*
*  returns   Nothing
*
**/
void SaveGameCfg() {
	WritePrivateProfileString(INI_SECTION, "RandomLetters",
	                          gGameCfg.bRandomLetters ? "Yes" : "No", INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "NumStartingLetters",
	                          Common::String::format("%d", gGameCfg.nShown).c_str(),
	                          INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "GameSpeed",
	                          Common::String::format("%d", gGameCfg.gameSpeed).c_str(),
	                          INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "ShowCategoryNames",
	                          gGameCfg.bShowNames ? "Yes" : "No",
	                          INI_FILENAME);
}


/**
*  name      CheckUserGuess - compares user's guess with current phrase
*
*  synopsis  winStatus = CheckUserGuess(guess)
*            const char *guess                  users guess to check vs phrase
*
*  purpose
*
*
*  returns   match/unmatch condition (True if users guess matches the phrase)
*
**/
bool CheckUserGuess(const char *guess) {
	return (StrCompare(curPhrase->text, guess, MAX_PLENGTH_S + 1));
}

/**
*  name      StrLenNoSpaces - gets the length of a string after stripping
*                             out any spaces.
*
*  synopsis  StrLenNoSpaces(string)
*            const char *string             string to get length of
*
*  purpose   To determine the length of a string while not counting spaces
*
*
*  returns   len = length of string without spaces
*
**/
int StrLenNoSpaces(const char *str) {
	int len;

	/* can't access a null pointer */
	assert(str != nullptr);

	len = 0;
	while (*str) {
		if (*str++ != ' ') {
			len++;
		}
	}
	return (len);
}


ERROR_CODE ValidatePhrase(PHRASES *phrase) {
	bool bList[MAX_PLENGTH];
	int i, n, order;
	char c;
	ERROR_CODE errCode;

	assert(phrase != nullptr);

	/* set all entries to false */
	memset(bList, 0, sizeof(bool)*MAX_PLENGTH);

	/* assume no error */
	errCode  = ERR_NONE;

	if ((n = strlen(phrase->text)) > MAX_PLENGTH_S) {
		errCode = ERR_FTYPE;
	} else {
		for (i = 0; i < n; i++) {

			c = phrase->text[i];

			/*
			* verify that all characters in this phrase are valid.
			* valid chars are are '\0', ' ' or a letter
			*/
			if ((c != 0) && (c != 32) && !Common::isAlpha(c)) {
				errCode = ERR_FTYPE;
				break;
			}
		}

		/*
		* continues as long as there was no error
		*/
		if (errCode == ERR_NONE) {

			if ((n = StrLenNoSpaces(phrase->text)) > MAX_PLENGTH) {
	
				errCode = ERR_FTYPE;
			} else {

				/*
				* check to make sure that the indexing order values are valid
				*/
				for (i = 0; i < n; i++) {
					order = (int)phrase->order[i] - 1;

					if ((order >= n) || (order < 0) || bList[order]) {

						errCode = ERR_FTYPE;
						break;
					}
					bList[order] = true;
				}
			}
		}
	}

	return errCode;
}


void GameGetScore(unsigned int *nLeft, unsigned int *nTotal, unsigned int *nLeftAvg, unsigned int *nTotalAvg) {
	/* can't write to null pointers */
	assert(nLeft != nullptr);
	assert(nTotal != nullptr);
	assert(nLeftAvg != nullptr);
	assert(nTotalAvg != nullptr);

	*nLeft = *nTotal = 0;
	if (bInGame) {

		*nTotal = StrLenNoSpaces(curPhrase->text);
		*nLeft = *nTotal - iNextLetter;
	}
	*nLeftAvg = gLeftAvg;
	*nTotalAvg = gTotalAvg;
}

/**
*  name      NumLinkedSprites - gets number of sprites currently in the sprite
*                               chain
*
*  synopsis  nSprites = NumLinkedSprites()
*
*
*  purpose   To return the number of linked sprites
*
*
*  returns   nSprites = number of sprites in linked list
*
**/
int NumLinkedSprites() {
	CSprite *pSprite;
	int i = 0;

	pSprite = CSprite::GetSpriteChain();
	while (pSprite) {
		pSprite = pSprite->GetNextSprite();
		i++;
	}

	return (i);
}

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel
