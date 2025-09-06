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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/life/life.h"
#include "bagel/hodjnpodj/life/game.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

// Globals!
extern int nSpeed, nCountDown, nLife;
extern int nTurnCounter, nLifeCounter;
extern bool bIsInfiniteLife, bIsInfiniteTurns;
extern CMainWindow *gMainWnd;
extern CPalette *pGamePalette;
extern CSprite *pScrollSprite;

/*****************************************************************
 *
 * CLife
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
CLife::CLife(CDC *pDC) {
	int     i;
	bool    bCheck;

	m_dScore        = 0.0;
	m_nYears        = 0;
	m_nCumLife      = 0;
	m_bIsEvolving   = false;
	m_bPrePlace     = false;

	pColony = new colony(ROWS, COLS);
	// Because this change came bout later

	for (i = 0 ; i < VILLAGES ; i++) {
		pBaseSprite[i] = new CSprite();
		(*pBaseSprite[i]).SharePalette(pGamePalette);
		bCheck = (*pBaseSprite[i]).LoadSprite(pDC, curly[i]);
		ASSERT(bCheck);

		(*pBaseSprite[i]).SetMobile(true);
		(*pBaseSprite[i]).SetMasked(true);
		(*pBaseSprite[i]).SetOptimizeSpeed(true);
	}

	for (i = 0 ; i < MONTHS ; i++) {
		m_cCalendar.pMonthSprite[i] = new CSprite();
		m_cCalendar.bIsOn[i] = false;

		m_cCalendar.pMonthSprite[i]->SharePalette(pGamePalette);
		bCheck = m_cCalendar.pMonthSprite[i]->LoadSprite(pDC, months[i]);
		ASSERT(bCheck);

		m_cCalendar.pMonthSprite[i]->SetMobile(true);
		m_cCalendar.pMonthSprite[i]->SetMasked(true);
	}

	//srand((unsigned) time(nullptr));     // seed the random number generator
}

/*****************************************************************
 *
 * ~CLife
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
CLife::~CLife() {
	int i;

	for (i = 0 ; i < VILLAGES ; i++)
		delete pBaseSprite[i];

	for (i = 0 ; i < MONTHS ; i++)
		delete m_cCalendar.pMonthSprite[i];

	CSprite::FlushSpriteChain();
	delete pColony;

	delete pYears;
	delete pYearsText1;
	delete pYearsText2;
	delete pColonyStat;
	delete pColonyStatText1;
	delete pScore;
	delete pColonyPlaced;
}

/*****************************************************************
 *
 * change_board
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CLife::change_board(unsigned int nFlags, CPoint point, CDC *pDC, bool bPlayingMeta) {
	int i;
	CSprite *pSprite;
	CRect   myArea;
	CSize mySize;
	CPoint sprite_loc;                  // center location of sprite on board
	int row, col;                           // indices help find board cell
	char buf[10];
	bool bAssertCheck;      // used to check for assertion failures

	// added routines to capture if button click was inside
	//  legal board area

	if (point.x <= BOARD_START_COL ||  // Is pnt outside of range?
	        point.x >= BOARD_START_COL + COLS * BOARD_SPACING_TIMES_TWO + COLS * CURLY_X ||
	        point.y <= BOARD_START_ROW ||
	        point.y >= BOARD_START_ROW + ROWS * BOARD_SPACING_TIMES_TWO + ROWS * CURLY_Y) {
//			MessageBeep(-1);
		sndPlaySound(WAV_CANTDO, SND_SYNC);
		return;
	}

	// track down cell of board that was clicked
	row = (point.y - BOARD_START_ROW) / (BOARD_SPACING_TIMES_TWO + CURLY_Y);
	col = (point.x - BOARD_START_COL) / (BOARD_SPACING_TIMES_TWO + CURLY_X);

	if (!(*pColony).islife(row, col)) { //life at this cell now?
		// No - so do we have colonies left to place?
		if (!nLifeCounter && !bIsInfiniteLife) {
			// No - so indicate this to user
//			MessageBeep(-1);
			sndPlaySound(WAV_CANTDO, SND_SYNC);
			return;
		}
	} else if (
	    bPlayingMeta == true ||     // can't remove life in meta OR
	    m_bIsEvolving == true       // if evolve was clicked.
	) {
//		MessageBeep(-1);
		sndPlaySound(WAV_CANTDO, SND_SYNC);
		return;
	} // end if

	// flip it on/off
	(*pColony).flip(row, col);

	// Update colony placed count on the board
	gMainWnd->RefreshStats();

	//set it up to be centered
	sprite_loc.y = BOARD_START_ROW + (CURLY_Y + BOARD_SPACING_TIMES_TWO) * row;
	sprite_loc.x = BOARD_START_COL + (CURLY_X + BOARD_SPACING_TIMES_TWO) * col;

	pSprite = CSprite::GetSpriteChain();
	// User want to turn life off?
	if ((*pColony).islife(row, col)) {
		if (bIsInfiniteLife != true) {
			//decrement colony counter if not infinite
			nLifeCounter--;
			Common::sprintf_s(buf, "%d", nLifeCounter);
			bAssertCheck = pColonyStat->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
			ASSERT(bAssertCheck);   // paint the text
		}
		i = brand() % VILLAGES;
		pSprite = (*pBaseSprite[i]).DuplicateSprite(pDC);
		mySize = (*pSprite).GetSize();
		(*pSprite).LinkSprite();

		// paint the sprite in its new location
		(*pSprite).PaintSprite(pDC,
		                       sprite_loc.x,  // finding center of cell
		                       sprite_loc.y);

		sndPlaySound(WAV_NEWVILLAGE, SND_ASYNC);
	} else {
		CRect   testRect;       // sprite area to be tested

		if (bIsInfiniteLife != true) {
			//Add life back to colony counter if finite
			nLifeCounter++;
			Common::sprintf_s(buf, "%d", nLifeCounter);
			bAssertCheck = pColonyStat->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
			ASSERT(bAssertCheck);   // paint the text
		}

		pSprite = CSprite::Touched(sprite_loc);
		if (pSprite != nullptr) { // See if the point is in the sprite's rectangle
			(*pSprite).RefreshBackground(pDC);      // ... simply repaint background
			(*pSprite).UnlinkSprite(); // ... and if so, unlink it from chain
			delete pSprite;
		}
	}
}

/*****************************************************************
 *
 * evolution
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CLife::evolution(CDC *pDC) {
	CSprite *pSprite;
	CSize   mySize;
	CRect   myArea;
	CPoint sprite_loc;                  // center location of sprite on board
	int i;                              // used to gen random village
	int row, col;                           // indexs help find board cell
	colony pColonyCopy((*pColony).row(), (*pColony).col());

	// make copy of original pColony for later ref
	pColonyCopy = (*pColony);
	// Update stats
	if (m_nYears == 0) //just starting evolution?
		m_nCumLife = (*pColony).m_nColony_count;
	else
		m_nCumLife += (*pColony).m_nColony_count;

	m_nYears++;
	m_dScore = ((double) m_nCumLife) / ((double) m_nYears);

	//Evolve internal board
	(*pColony).evolve(*pColony);

	// update score
	if (m_nCumLife > LARGE && bIsInfiniteTurns) { // This prevents int overflow
		// reset it
		m_nCumLife = (int) m_dScore;
		m_nYears = 1;
	}

	// update stats visually
	gMainWnd->RefreshStats();

	//Update visual board
	if (pColonyCopy != (*pColony))
		for (row = 0; row < (*pColony).row(); row++)
			for (col = 0; col < (*pColony).col(); col++) {
				pSprite = CSprite::GetSpriteChain();

				// Any change for this particular cell?
				if (pColonyCopy.islife(row, col) == (*pColony).islife(row, col))
					continue;  // no change -- loop

				if ((*pColony).islife(row, col)) { //Need to put a sprite there?
					// Yes, paint new sprite in cell
					i = brand() % VILLAGES;
					pSprite = (*pBaseSprite[i]).DuplicateSprite(pDC);
					mySize = (*pSprite).GetSize();
					//set it up to be centered

					sprite_loc.y = BOARD_START_ROW + (CURLY_Y + BOARD_SPACING_TIMES_TWO) * row;
					sprite_loc.x = BOARD_START_COL + (CURLY_X + BOARD_SPACING_TIMES_TWO) * col;

					(*pSprite).LinkSprite();

					// paint the sprite in its new location
					(*pSprite).PaintSprite(pDC,
					                       sprite_loc.x,  // finding center of cell
					                       sprite_loc.y);
				} else {
					CRect   testRect;       // sprite area to be tested

					sprite_loc.y = BOARD_START_ROW + (CURLY_Y + BOARD_SPACING_TIMES_TWO) * row;
					sprite_loc.x = BOARD_START_COL + (CURLY_X + BOARD_SPACING_TIMES_TWO) * col;

					pSprite = CSprite::Touched(sprite_loc);
					if (pSprite != nullptr) { // See if the point is in the sprite's rectangle
						(*pSprite).RefreshBackground(pDC);      // ... simply repaint background
						(*pSprite).UnlinkSprite(); // ... and if so, unlink it from chain
						delete pSprite;
					}
				}
			}
}

/*****************************************************************
 *
 * NewGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CLife::NewGame(CDC *pDC) {
	colony  pColonyCopy((*pColony).row(), (*pColony).col());

	// Reset internal grid
	(*pColony)      = pColonyCopy;

	// Reset Stats
	m_nYears        = 0;
	m_dScore        = 0.0;
	m_nCumLife      = 0;
	nCountDown      = nSpeed * MONTHS;
	nLifeCounter    = nLife;

	// Refresh Calendar
	ResetMonths(pDC);
	DisplayMonth(nCountDown, pDC);

	// Update visual stats
	gMainWnd->RefreshStats();

	//Reset visual board
	CSprite::EraseSprites(pDC);
	CSprite::FlushSpriteChain();
}

/*****************************************************************
 *
 * DisplayMonth
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CLife::DisplayMonth(int nMonth, CDC *pDC) {
	int i, j;

	/************************************************************************************
	 * In order to get the visual calendar to change exactly 12 times, regardless       *
	 * of the how nSpeed was set, I could achieve this in at least two different ways.  *
	 * First way was to modify how the WM_TIMER messages are passed around, but this    *
	 * would introduce many more changes that I wanted to make.  Second way was         *
	 * to watch the nMonth (equivalent to the nCountDown global) and change the         *
	 * calendar only when it hit specific values defined by nSpeed (speed of clock).    *
	 * I am doing this second way here.                                                 *
	 ************************************************************************************/

	if (nSpeed != FASTEST) {    // don't want divide by 0 errors if FASTEST = 0
		/*
		* nMonths counts down, i.e. 24, 23, 22, 21....
		* j is a number between 0 and 11 counting as follows:
		*   0, 1, 2, 3, 4....
		* j is indexing the current month
		*/
		j = (MONTHS - ((int) nMonth / nSpeed) - 1);

		if (j < 0)
			j = 0;

		if (j == 0)
			i = MONTHS - 1;  // i indexes previous month
		else
			i = j - 1;


		if (m_cCalendar.bIsOn[i] == true) {
			m_cCalendar.pMonthSprite[i]->EraseSprite(pDC);
			m_cCalendar.bIsOn[i] = false;
		}
	} else
		j = 0;
//b
	m_cCalendar.pMonthSprite[j]->PaintSprite(pDC, MONTH_COL_POS + 1, MONTH_ROW_POS + 1);
	m_cCalendar.bIsOn[j] = true;
}

/*****************************************************************
 *
 * ResetMonths
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CLife::ResetMonths(CDC *pDC) {
	int i;

	// reset all calendars
	for (i = 0 ; i < MONTHS ; i++) {
		if (m_cCalendar.bIsOn[i] == true) {
			m_cCalendar.pMonthSprite[i]->EraseSprite(pDC);
			m_cCalendar.bIsOn[i] = false;
			break;
		}
	}

	// repaint current calendar
	//DisplayMonth(nMonth, pDC);
}

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel
