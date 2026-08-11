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

#ifndef HODJNPODJ_BARBERSHOP_PAINT_H
#define HODJNPODJ_BARBERSHOP_PAINT_H

#include "bagel/hodjnpodj/barbershop/deck.h"
#include "bagel/hodjnpodj/barbershop/card.h"
#include "bagel/hodjnpodj/barbershop/stack.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define STD_CARD_WTH    72      // standard card width (x-val)
#define STD_CARD_HEI    97      // standard card height (y-val)
#define STD_HALF_WTH    37
#define STD_HALF_HEI    48

#define BITMAP_WTH      119
#define BITMAP_HEI      84

#define FND_LEF         500     // Foundation rect
#define FND_TOP         200
#define FND_RIG         (FND_LEF + BITMAP_WTH)
#define FND_BOT         (FND_TOP + BITMAP_HEI)

#define TAB_LEF         60      // Tableau rect
#define TAB_TOP         300
#define TAB_RIG         (TAB_LEF + STD_CARD_WTH)
#define TAB_BOT         (TAB_TOP + STD_CARD_HEI)

#define TAB_OFFSET      110     // offset from one tab stack to next

#define TAB_INDENT      1       // # of cards before next card is shifted
#define TAB_DX          -2      // how much to shift card right/left
#define TAB_DY          2       // how much shift up/down

#define STACK_INDENT    5       // # of cards before next card is shifted
#define STACK_DX        0       // how much to shift card right/left
#define STACK_DY        -2      // how much shift up/down
#define STACK_FACE_DX   -7      // for Tableau face card shifting
#define STACK_FACE_DY   15

#define STOC_LEF        110     // Stock stack rect
#define STOC_TOP        200
#define STOC_RIG        (STOC_LEF + BITMAP_WTH)
#define STOC_BOT        (STOC_TOP + BITMAP_HEI)

#define USED_OFFSET     BITMAP_WTH      // Used stack offset from stock stack
#define USED_INDENT     3       // # of cards before next card is shifted
#define USED_DX         20      // how much to shift card right/left
#define USED_DY         0       // how much shift up/down

#define DECK_RES_ID     100         // corresp to crypt.rc alphabet id's
#define CARD_SET        (DECK + 2)  // 62 cards, 2 reverse sides
#define CARD_BACK1      DECK        // reverse side of card #1
#define CARD_BACK2      DECK + 1    // reverse side of card #2

#define CARD_HOT_X      62      // Hotspot info for card positioning/moving
#define CARD_HOT_Y      41

#define CARDS_PER_COL   8           // amount of cards in the column for of the deck bitmap
#define CARDS_PER_ROW   8           // amount of cards in the row for of the deck bitmap

#define CARD_BACK1A_BMP "art\\card-bk1.bmp"     // card back option #1
#define CARD_BACK2A_BMP "art\\card-bk2.bmp"     // card back option #2
#define CARD_BACK1B_BMP "art\\card-bk3.bmp"     // card back option #1
#define CARD_BACK2B_BMP "art\\card-bk4.bmp"     // card back option #2

class CPaint {
private:    // vars
	OSpr *m_cCardSet[CARD_SET] = {};

public:     // func
	CPaint(CDC*);
	~CPaint();

	void    Board(CDC*, CBoard*);
	void    Refresh(CDC*, CBoard*);
	CCard  *IsOnCard(CPoint);
	void    MoveCard(CDC*, CCard*, CPoint);
	void    UpdateCard(CDC*, CCard*);
	void    FlipCard(CDC*, CCard*);
	void    Stack(CDC*, CCard*, int nSize = -1);
	bool    IsNewBack(int);
	void    ChangeBack(CDC*, CBoard*, int);

public:     // vars
	OSpr    *m_pSprite = nullptr;
	CRect   m_cFound;
	CRect   m_cTab[TAB_COUNT];
	CRect   m_cStock;
	CRect   m_cUsed;
	int     m_nUsedInd = 0;
	int     m_nCardBack = 0;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
