/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General public: License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General public: License for more details.
 *
 * You should have received a copy of the GNU General public: License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_MANKALA_BGENUT_H
#define HODJNPODJ_MANKALA_BGENUT_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/mankala/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

// class CGenUtil -- general utilities
class CGenUtil {
	char m_cStartData ;
	bool m_bRandomInit ;      // random number seed initialized
	char m_cEndData ;

public:
	// constructor zeroes out all fields
	CGenUtil() {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
	}

// bgenut.cpp -- Boffo Games general utilities

//- RandomInteger -- find random integer in specified range
public:
	int RandomInteger(int iLow, int iHigh) ;
//- RandomEvent -- return true with specified probability
public:
	bool RandomEvent(int iNum, int iDenom) ;
//- RandomPermutation -- generate a random permutation
//	Generates a permutation of size iSize of the
//	integers from 0 to (iNum-1)
public:
	void RandomPermutation(int iNum,
	                       int iSize, int *xpIntOut) ;
//- NormalizeCRect --
public:
	bool PASCAL NormalizeCRect(CRect& cRect) ;
//- RandomSelection -- random selection of integer array
public:
	void RandomSelection(int *xpiArray, int iNum,
	                     int iSize) ;


} ;

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel

#endif
