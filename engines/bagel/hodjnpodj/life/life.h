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

#ifndef HODJNPODJ_LIFE_LIFE_H
#define HODJNPODJ_LIFE_LIFE_H

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/life/grid.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

#define VILLAGES        5   // Constants
#define MONTHS          12
#define FASTEST         0   // Changing this doesn't effectly change it
// everywhere.  speed of evolution.
#define LARGE       32468   //= 2^15 - 300
//  when to reset counter increments

#define WAV_EVOLVE      ".\\sound\\sound178.wav"    // Wave sounds
#define WAV_GAMEOVER    ".\\sound\\sound283.wav"
#define WAV_NEWVILLAGE  ".\\sound\\colony.wav"
#define WAV_DEATH       ".\\sound\\sound259.wav"
#define WAV_CANTDO      ".\\sound\\sorry.wav"

class CCalendar {
public:
	CSprite *pMonthSprite[MONTHS];
	bool    bIsOn[MONTHS];
};

class CLife {
public:
	// Constructor
	CLife(CDC *pDC);
	// Destructor
	~CLife();
	void    change_board(unsigned int, CPoint, CDC*, bool);
	void    evolution(CDC *pDC);
	void    NewGame(CDC *pDC);
	void    DisplayMonth(int nMonth, CDC *pDC);
	void    ResetMonths(CDC *pDC);
	int     ColonyPlaced() {
		return (*pColony).m_nColony_count;
	}

public:
	colony      *pColony;
	double      m_dScore;
	int         m_nCumLife;
	int         m_nYears;
	bool        m_bIsEvolving;
	bool        m_bPrePlace;

	CText       *pYears, *pYearsText1, *pYearsText2,
	            *pColonyStat, *pColonyStatText1,
	            *pScore, *pColonyPlaced, *pTime;

	CSprite     *pBaseSprite[VILLAGES];
	CCalendar   m_cCalendar;
};

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel

#endif
