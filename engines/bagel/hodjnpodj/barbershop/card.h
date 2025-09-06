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

#ifndef HODJNPODJ_BARBERSHOP_CARD_H
#define HODJNPODJ_BARBERSHOP_CARD_H

#include "bagel/hodjnpodj/hnplibs/sprite.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define TAB_COUNT   5
#define TAB_STACK   7
#define TABLEAU     35

#define STOCK       27
#define STOCK_DRAW  3

#define SUITS       5
#define PIPS        12

#define ONE_CARD    0
#define TWO_CARD    1
#define THREE_CARD  2
#define FOUR_CARD   3
#define FIVE_CARD   4
#define SIX_CARD    5
#define SEVEN_CARD  6
#define EIGHT_CARD  7
#define NINE_CARD   8
#define TEN_CARD    9
#define CUST_CARD   10
#define BARB_CARD   11
#define MANI_CARD   60

enum suit {suit_none = -1, brush, comb, mirror, razor, sissor, manicurist};
enum loc {loc_none = -1, fnd, tab, stock = 6, used};

class CStack;
class CCard;

class OSpr : public CSprite {
public:
	OSpr();

	CCard   *m_cCard;
};

class CCard {
private:    // vars
	suit    m_enSuit;
	int     m_nPip;

public:     // functions

	CCard(suit enSuit, int nPip);
	CCard(int);
	CCard();
	~CCard();

	int GetValue() const;
	int GetPip() const {
		return m_nPip;
	}
	bool IsFace() const {
		return m_nPip >= CUST_CARD;
	}
	bool IsCustomer() const {
		return m_nPip == CUST_CARD;
	}
	bool IsBarber() const {
		return m_nPip == BARB_CARD;
	}
	bool IsManicurist() {
		return m_nPip == MANI_CARD;
	}
	bool operator==(int nValue) const {
		return nValue == GetValue();
	}
	CCard &operator=(const CCard &);

public:     // vars
	CCard   *m_pPrevCard;
	CCard   *m_pNextCard;
	OSpr    *m_pSprite;
	CStack  *m_pStack;
	bool    m_bIsBack;
	CPoint  m_cOrigin;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
