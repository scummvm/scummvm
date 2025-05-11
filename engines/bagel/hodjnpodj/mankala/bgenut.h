// bgenut.h -- header file for general utilities
// Written by John J. Xenakis, 1994, for Boffo Games


#ifndef __bgenut_H__
#define __bgenut_H__


#include <time.h>
//#include "globals.h"
//#include "resource.h"
//#include "dibdoc.h"
//#include "sprite.h"

#include "bgen.h"

#include "hhead.h"

// class CGenUtil -- general utilities
class CGenUtil {

    char m_cStartData ;
    BOOL m_bRandomInit ;	  // random number seed initialized
    char m_cEndData ;

public:

    // constructor zeroes out all fields
    CGenUtil() {memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;}

// bgenut.cpp -- Boffo Games general utilities

//- CGenUtil::RandomInteger -- find random integer in specified range
PUBLIC int CGenUtil::RandomInteger(int iLow, int iHigh) ;
//- CGenUtil::RandomEvent -- return TRUE with specified probability
PUBLIC BOOL CGenUtil::RandomEvent(int iNum, int iDenom) ;
//- CGenUtil::RandomPermutation -- generate a random permutation
//	Generates a permutation of size iSize of the
//	integers from 0 to (iNum-1)
PUBLIC void CGenUtil::RandomPermutation(int iNum,
			int iSize, int * xpIntOut) ;
//- CGenUtil::NormalizeCRect -- 
PUBLIC STATIC BOOL PASCAL CGenUtil::NormalizeCRect(CRect& cRect) ;
//- CGenUtil::RandomSelection -- random selection of integer array
PUBLIC void CGenUtil::RandomSelection(int * xpiArray, int iNum,
			int iSize) ;


} ;

#include "htail.h"

#endif // __bgenut_H__

