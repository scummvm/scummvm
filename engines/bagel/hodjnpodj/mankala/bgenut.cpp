// bgenut.cpp -- Boffo Games general utilities
// Written by John J. Xenakis, 1994, for Boffo Games

#include "stdafx.h"
#include "copyrite.h"	// mandatory internal copyright notice

#include "bgenut.h"

///DEFS bgenut.h


//* CGenUtil::RandomInteger -- find random integer in specified range
PUBLIC int CGenUtil::RandomInteger(int iLow, int iHigh)
// iLow, iHigh -- range
// returns: integer which is ge iLow and le iHigh.
{
    JXENTER(CGenUtil::RandomInteger) ;
    int iError = 0 ;
    int iRetval ;		// integer return value
    time_t tTime ;		// time -- for initializing random # gen

    // for first call, initialize the random number generator
    if (!m_bRandomInit)
	    //srand((unsigned) time(&tTime)), m_bRandomInit = TRUE ;

    // Note: since brand() returns a value uniform in [0, 2^15-1], the
    // following method, though simple, gives a slight preference
    // to lower numbers.  Might be worth fixing sometime.
    if (iLow < iHigh)		// if there's a valid range
	iRetval = iLow + (brand() % (iHigh - iLow + 1)) ;  // compute value
    else		// invalid args
	iRetval = iLow ;
    JXELEAVE(CGenUtil::RandomInteger) ;
    return(iRetval) ;
}

//* CGenUtil::RandomEvent -- return TRUE with specified probability
PUBLIC BOOL CGenUtil::RandomEvent(int iNum, int iDenom)
// iNum / iDenom -- numerator and denominator of probability
// returns: TRUE with probability iNum / iDenom, FALSE otherwise
{
    JXENTER(CGenUtil::RandomEvent) ;
    int iError = 0 ;
    BOOL bRetval ;		// return value

    bRetval = (iNum >= 1 + (brand() % iDenom)) ;
    JXELEAVE(CGenUtil::RandomEvent) ;
    return(bRetval) ;
}


//* CGenUtil::RandomPermutation -- generate a random permutation
//	Generates a permutation of size iSize of the
//	integers from 0 to (iNum-1)
PUBLIC void CGenUtil::RandomPermutation(int iNum,
			int iSize, int * xpIntOut)
// iNum -- number of numbers in permutation
// iSize -- size of output array
// xpIntOut -- pointer to output array (size is iSize)
{
    int iError = 0 ;
    int * xpInt ;	// array pointer variable
    int iI, iJ, iK ;	// loop variables
    int iRanVal ;	// random value to make a choice
    BOOL bDup ;		// duplicate found flag

    JXENTER(CGenUtil::RandomPermutation) ;
    if (iSize > iNum)	// can't handle more array than # of numbers
	iSize = iNum ;

    for (iI = 0 ; iI < iSize ; ++iI)		// count numbers
    {
	iRanVal = RandomInteger(0, iNum - iI - 1) ;
	    		// get random number in interval
	for (iJ = 0 ; iJ < iNum && iRanVal >= 0 ; ++iJ )
	{
	    bDup = FALSE ;		// not a duplicate yet
	    for (xpInt = xpIntOut, iK = 0 ; iK < iI ; ++iK, ++xpInt)
		if (*xpInt == iJ)
		    bDup = TRUE ;
	    if (!bDup && iRanVal-- == 0)	// if not a duplicate
			// and we've counted down the random integer
		*xpInt = iJ ;		// store into array
	}
    }
    JXELEAVE(CGenUtil::RandomPermutation) ;
}

//* CGenUtil::NormalizeCRect -- 
PUBLIC STATIC BOOL PASCAL CGenUtil::NormalizeCRect(CRect& cRect)
// cRect -- rectangle to be normalized
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGenUtil::NormalizeCRect) ;
    int iError = 0 ;		// error code
    int iSwap ;

    if ( (iSwap = cRect.left) > cRect.right)
	cRect.left = cRect.right, cRect.right = iSwap ;
    if ( (iSwap = cRect.top) > cRect.bottom)
	cRect.top = cRect.bottom, cRect.bottom = iSwap ;

// cleanup:

    JXELEAVE(CGenUtil::NormalizeCRect) ;
    RETURN(iError != 0) ;
}


//* CGenUtil::RandomSelection -- random selection of integer array
PUBLIC void CGenUtil::RandomSelection(int * xpiArray, int iNum,
			int iSize)
// xpiArray (input/output) -- array of integers to make selection from
// iNum -- number of numbers in array
// iSize -- number of integers in desired selection
{
    JXENTER(CGenUtil::RandomSelection) ;
    int iError = 0 ;
    int * xpiPermutation = new int[iSize] ;
    int iK ;

    RandomPermutation(iNum, iSize, xpiPermutation) ;

    for (iK = 0 ; iK < iSize ; ++iK)
	xpiPermutation[iK] = xpiArray[xpiPermutation[iK]] ;

    for (iK = 0 ; iK < iSize ; ++iK)
	xpiArray[iK] = xpiPermutation[iK] ;

    if (xpiPermutation)
	delete [] xpiPermutation ;

    JXELEAVE(CGenUtil::RandomSelection) ;
}

