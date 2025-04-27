// bbt.h -- header file for Boffo Button handling
// Written by John J. Xenakis, 1994, for Boffo Games Inc.

#ifndef __bbt_H__
#define __bbt_H__

#include "bgb.h"

#define BBTT_DIB 1
#define BBTT_SPRITE 2

class CBbutton ;

// CBbtMgr -- boffo games button manager
class CBbtMgr {
public:
    char m_cStartData ;
    CBgbMgr FAR * m_lpBgbMgr ;
    CBbutton FAR * m_lpBbtChain ; // chain of bbt objects
    CBbutton FAR * m_lpDownBbt ;
    char m_cEndData ;

// methods
public:
    CBbtMgr::CBbtMgr(CBgbMgr FAR * lpBgbMgr = NULL)
    		{memset(&m_cStartData, 0,
				&m_cEndData - &m_cStartData) ;
			m_lpBgbMgr = lpBgbMgr ; }


// bbt.cpp -- Boffo button handling

//- CBbtMgr::~CBbtMgr -- destructor
public: CBbtMgr::~CBbtMgr(void) ;
//- CBbtMgr::LinkButton -- link button into button manager
public: BOOL CBbtMgr::LinkButton(CBbutton FAR * lpBbt,
			CBgbObject FAR * lpcBgbObject1,
			CBgbObject FAR * lpcBgbObject2) ;
//- CBbtMgr::AcceptClick -- process mouse click or mouse move
public: int CBbtMgr::AcceptClick(CRPoint crPoint, int iClickType) ;
//- CBbtMgr::MoveDown -- move button down
private: BOOL CBbtMgr::MoveDown(CBbutton FAR * lpDownBbt PDFT(NULL)) ;
//- CBbtMgr::GetCurrentBitmap -- get current bitmap for button
public: CBgbObject FAR * CBbtMgr::GetCurrentBitmap(CBbutton * xpBbt) ;
} ;


// CBbutton -- Boffo button object
class FAR CBbutton {
    friend class CBbtMgr ;
public:
    char m_cStartData ;
    int m_iBbtId ;		// button id
    CBbutton FAR * m_lpBbtNext ;	// pointer to next in chain
    BOOL m_bChained:1 ;		// on m_xBbtChain
    BOOL m_bNoDelete ;		// not allocated with "new"
    BOOL m_bDummy:1 ;		// dummy object -- no bitmap
    BOOL m_bInit:1 ;		// flag: object is initialized
    CBgbObject FAR * m_lpcBgbObject1, FAR * m_lpcBgbObject2 ;
			// graphics objects for on/off
    char m_cEndData ;

public:
    CBbutton(int iId = 0) {_fmemset(&m_cStartData, 0,
	    			&m_cEndData - &m_cStartData) ;
		m_iBbtId = iId ; }
    BOOL PtInButton(CRPoint crPoint)
	{ return(m_lpcBgbObject1 ?
		m_lpcBgbObject1->GetRect().PtInRect(crPoint) : FALSE) ;}
} ;


#endif // __bbt_H__
