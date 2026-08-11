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
#include "bagel/hodjnpodj/barbershop/stats.h"
#include "bagel/hodjnpodj/barbershop/resource.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

CStats::CStats() {
	m_pTime = nullptr;
	m_pScore = nullptr;

	m_nLettersSolved = GetPrivateProfileInt(
	                       INI_SECTION,
	                       INI_LETTERSSOLVED,
	                       LSLVD_DEFAULT,
	                       INI_FILENAME
	                   );

	if (m_nLettersSolved < MIN_LSLVD || m_nLettersSolved > MAX_LSLVD)
		m_nLettersSolved = LSLVD_DEFAULT;

	m_nScore = 0;

	m_nTime = GetPrivateProfileInt(
	              INI_SECTION,
	              INI_TIME,
	              TIME_DEFAULT,
	              INI_FILENAME
	          );


	if (m_nTime < MIN_TIME || m_nTime > MAX_TIME)
		m_nTime = TIME_DEFAULT;

	m_nCountDown = m_nTime;

	m_nIsUsedGram = GetPrivateProfileInt(
	                    INI_SECTION,
	                    INI_REC,
	                    REC_DEFAULT,
	                    INI_FILENAME
	                );
}


CStats::~CStats() {
	if (m_pScore != nullptr) {
		delete m_pScore;
		m_pScore = nullptr;
	}

	if (m_pTime != nullptr) {
		delete m_pTime;
		m_pTime = nullptr;
	}
}


int CStats::ResetGame() {
	//char  chResetUsedGram;
	int     nID;
	//int   i;

	/*************************
	* Reset crytogram stats. *
	*************************/
	m_nCountDown    = m_nTime;
	m_nScore        = 0;

	do {                                                            // Get random unused cryptogram
		nID = brand() % CRYPT_RECS;
	} while (m_nIsUsedGram == nID);

	m_nIsUsedGram = nID;                                            // Mark as used

	WritePrivateProfileString(
	    INI_SECTION,
	    INI_REC,
	    Common::String::format("%d", m_nIsUsedGram).c_str(),
	    INI_FILENAME
	);                                          // Save used list back

	return nID;
}


void CStats::SaveStats(int nLttrsSlvd, int nTime) {
	m_nLettersSolved    = nLttrsSlvd;
	m_nTime             = nTime;

	WritePrivateProfileString(
	    INI_SECTION,
	    INI_LETTERSSOLVED,
	    Common::String::format("%d", m_nLettersSolved).c_str(),
	    INI_FILENAME
	);

	WritePrivateProfileString(
	    INI_SECTION,
	    INI_TIME,
	    Common::String::format("%d", m_nTime).c_str(),
	    INI_FILENAME
	);
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
