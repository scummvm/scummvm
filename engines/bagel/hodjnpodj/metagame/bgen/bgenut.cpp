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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/bgen/bgenut.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

///DEFS bgenut.h

//* CGenUtil::RandomInteger -- find random integer in specified range
int CGenUtil::RandomInteger(int iLow, int iHigh) {
// iLow, iHigh -- range
// returns: integer which is ge iLow and le iHigh.
	JXENTER(CGenUtil::RandomInteger);
	int iRetval = 0;            // integer return value
//	time_t tTime;              // time -- for initializing random # gen

	// for first call, initialize the random number generator
	if (!m_bRandomInit) {
		//srand((unsigned) time(&tTime)), m_bRandomInit = true;

		// Note: since brand() returns a value uniform in [0, 2^15-1], the
		// following method, though simple, gives a slight preference
		// to lower numbers.  Might be worth fixing sometime.
		if (iLow < iHigh) {           // if there's a valid range
			iRetval = iLow + (brand() % (iHigh - iLow + 1));  // compute value
		} else {               // invalid args
			iRetval = iLow;
		}
	}

	JXELEAVE(CGenUtil::RandomInteger);
	return iRetval;
}


//* CGenUtil::RandomPermutation -- generate a random permutation
//      Generates a permutation of size iSize of the
//      integers from 0 to (iNum-1)
void CGenUtil::RandomPermutation(int iNum,
                                 int iSize, int *xpIntOut) {
// iNum -- number of numbers in permutation
// iSize -- size of output array
// xpIntOut -- pointer to output array (size is iSize)
	int *xpInt;        // array pointer variable
	int iI, iJ, iK;    // loop variables
	int iRanVal;       // random value to make a choice
	bool bDup;         // duplicate found flag

	JXENTER(CGenUtil::RandomPermutation);
	if (iSize > iNum)   // can't handle more array than # of numbers
		iSize = iNum;

	for (iI = 0; iI < iSize; ++iI) {          // count numbers
		iRanVal = RandomInteger(0, iNum - iI - 1);
		// get random number in interval
		for (iJ = 0; iJ < iNum && iRanVal >= 0; ++iJ) {
			bDup = false;              // not a duplicate yet
			for (xpInt = xpIntOut, iK = 0; iK < iI; ++iK, ++xpInt)
				if (*xpInt == iJ)
					bDup = true;
			if (!bDup && iRanVal-- == 0)        // if not a duplicate
				// and we've counted down the random integer
				*xpInt = iJ;           // store into array
		}
	}
	JXELEAVE(CGenUtil::RandomPermutation);
}

//* CGenUtil::NormalizeCRect --
bool PASCAL CGenUtil::NormalizeCRect(CRect &cRect) {
// cRect -- rectangle to be normalized
// returns: true if error, false otherwise
	JXENTER(CGenUtil::NormalizeCRect);
	int iError = 0;            // error code
	int iSwap;

	if ((iSwap = cRect.left) > cRect.right)
		cRect.left = cRect.right, cRect.right = iSwap;

	if ((iSwap = cRect.top) > cRect.bottom)
		cRect.top = cRect.bottom, cRect.bottom = iSwap;

// cleanup:

	JXELEAVE(CGenUtil::NormalizeCRect);
	RETURN(iError != 0);
}


//* CGenUtil::RandomSelection -- random selection of integer array
bool CGenUtil::RandomSelection(int *xpiArray, int iNum, int iSize) {
// xpiArray (input/output) -- array of integers to make selection from
// iNum -- number of numbers in array
// iSize -- number of integers in desired selection
	JXENTER(CGenUtil::RandomSelection);
	int iError = 0;
	int *xpiPermutation;
	int iK;

	if (iSize > 0) {
		if ((xpiPermutation = new int [iSize]) != nullptr) {
			RandomPermutation(iNum, iSize, xpiPermutation);

			for (iK = 0; iK < iSize; ++iK)
				xpiPermutation[iK] = xpiArray[xpiPermutation[iK]];

			for (iK = 0; iK < iSize; ++iK)
				xpiArray[iK] = xpiPermutation[iK];

			delete [] xpiPermutation;
		} else {
			iError = 100;              // can't allocate array
		}
	}

	JXELEAVE(CGenUtil::RandomSelection);
	RETURN(iError != 0);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
