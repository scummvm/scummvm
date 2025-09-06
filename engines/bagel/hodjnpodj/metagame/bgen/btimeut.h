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

#ifndef BAGEL_METAGAME_BGEN_BTIMEUT_H
#define BAGEL_METAGAME_BGEN_BTIMEUT_H

#include "bagel/hodjnpodj/metagame/bgen/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

extern void DoPendingEvents();

// class CTimeUtil -- general utilities
class CTimeUtil {
public:
	char m_cStartData ;
	bool m_bTimeDelayPassed ;   // flag: DelayMs function time expired

	char m_cEndData ;

public:
	// constructor zeroes out all fields
	CTimeUtil() {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
	}

//- DelayMs -- delay for specified # of milliseconds
	bool DelayMs(unsigned int uMs) ;

private:
	/**
	 * SetTimer callback routine for DelayMs
	 * @param hWnd      handle of window (always nullptr in this case)
	 * @param uMsg      WM_TIMER message
	 * @param uTimerId  timer identifier
	 * @param dwTime    current system time
	 */
	static void DelayMsCallback(HWND hWnd, unsigned int uMsg,
	    uintptr uTimerId, uint32 dwTime);
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
