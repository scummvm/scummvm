/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General License for more details.
 *
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_METAGAME_BGEN_UTIL_H
#define HODJNPODJ_METAGAME_BGEN_UTIL_H

#include "common/rect.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

// class CGenUtil -- general utilities
class CGenUtil {
	bool m_bRandomInit = false;	  // random number seed initialized

public:
	// bgenut.cpp -- Boffo Games general utilities

	//- RandomInteger -- find random integer in specified range
	int RandomInteger(int iLow, int iHigh);
	//- RandomEvent -- return TRUE with specified probability
	bool RandomEvent(int iNum, int iDenom);
	//- RandomPermutation -- generate a random permutation
	//	Generates a permutation of size iSize of the
	//	integers from 0 to (iNum-1)
	void RandomPermutation(int iNum,
		int iSize, int *xpIntOut);
	//- NormalizeCRect -- 
	static bool NormalizeCRect(Common::Rect &cRect);
	//- RandomSelection -- random selection of integer array
	void RandomSelection(int *xpiArray, int iNum,
		int iSize);
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
