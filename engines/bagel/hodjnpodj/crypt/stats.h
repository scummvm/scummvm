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

#ifndef HODJNPODJ_CRYPT_STATS_H
#define HODJNPODJ_CRYPT_STATS_H

#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/crypt/rec.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

#define DECIMAL_BASE        10          // used in atoi() c function

#define TIME_LEFT_COL       44          // time left box
#define TIME_LEFT_ROW       30
#define TIME_RIGHT_COL      220
#define TIME_RIGHT_ROW      52

#define SCORE_LEFT_COL      459         // score pos
#define SCORE_LEFT_ROW      30
#define SCORE_RIGHT_COL     574
#define SCORE_RIGHT_ROW     52

#define MINUTE              60
#define STATS_COLOR         RGB(255,255,255)

#define INI_FNAME           "boffo.ini"
#define INI_SECTION         "Cryptograms"

#define INI_LETTERSSOLVED   "LettersSolved"
#define LSLVD_DEFAULT       6
#define MIN_LSLVD           0
#define MAX_LSLVD           20

#define INI_TIME            "Time"
#define TIME_DEFAULT        180
#define MIN_TIME            15
#define MAX_TIME            601

#define INI_REC             "Record"
#define REC_DEFAULT         0

#define STAT_TIMER_ID       987         // wm_timer messages
#define INTERVAL            1000        // one second intervals

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

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel

#endif
