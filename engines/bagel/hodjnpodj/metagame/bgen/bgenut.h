// bgenut.h -- header file for general utilities
// Written by John J. Xenakis, 1994, for Boffo Games


#ifndef __bgenut_H__
#define __bgenut_H__


#include <time.h>
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bgen.h"

// class CGenUtil -- general utilities
class CGenUtil {

    char m_cStartData ;
    BOOL m_bRandomInit ;          // random number seed initialized
    char m_cEndData ;

public:

    // constructor zeroes out all fields
    CGenUtil() {memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;}

    // bgenut.cpp -- Boffo Games general utilities

    int RandomInteger(int iLow, int iHigh) ;
    void RandomPermutation(int iNum, int iSize, int *xpIntOut);
    STATIC BOOL PASCAL NormalizeCRect(CRect& cRect) ;
    BOOL RandomSelection(int *xpiArray, int iNum, int iSize);
} ;

#endif // __bgenut_H__
