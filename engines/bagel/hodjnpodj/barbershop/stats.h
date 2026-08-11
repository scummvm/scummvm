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

#ifndef HODJNPODJ_BARBERSHOP_STATS_H
#define HODJNPODJ_BARBERSHOP_STATS_H

#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define STATS_COLOR         RGB(255,255,255)

#define SCORE_FACTOR        2
#define SCORE_JACKPOT       100

class CStats {
public:
	CStats();
	~CStats();

	void    SaveStats(int nLttrsSlvd, int nTime);
	int     ResetGame();

	CText   *m_pScore;
	CText   *m_pTime;

	int     m_nLettersSolved;
	int     m_nScore;
	int     m_nTime;
	int     m_nCountDown;
	char    m_nIsUsedGram;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
