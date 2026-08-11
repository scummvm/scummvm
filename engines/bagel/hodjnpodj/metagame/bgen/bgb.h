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

#ifndef BAGEL_METAGAME_BGEN_BGB_H
#define BAGEL_METAGAME_BGEN_BGB_H

#include "bagel/boflib/llist.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/metagame/bgen/bsutl.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define BGBT_DIB 1
#define BGBT_SPRITE 2

#define MAX_FNAME_LENGTH 32

class CBgbObject;

// CBgbMgr -- boffo games object manager class
class CBgbMgr {

public:
	CPen        m_cPen;
	CBrush      m_cBrush;

	// Following should be in view-specific block
	//
	CView *m_xpcView = nullptr;				// current window, when non-null
	CDC *m_xpDc = nullptr;					// current device context
	COLORREF    m_cPenColor = 0, m_cBrushColor = 0;
	int         m_iPenWidth = 0, m_iBrushStyle = 0;
	CBrush *m_xpOldBrush = nullptr;
	CPen *m_xpOldPen = nullptr;
	bool        m_bCreatePen = false, m_bCreateBrush = false;
	bool        m_bReleaseDc = false;		// flag: release DC when done
	int         m_iLockCount = 0;			// if positive, don't release DC

	CBsuSet *m_xpBsuSet = nullptr;			// scroll bar set, if any

	CPalette *m_xpGamePalette = nullptr;	// game pallet
	CPalette *m_xpOldPalette = nullptr;

	CBgbObject *m_lpBgbChain = nullptr;		// chain of bgb objects

	CLList *m_pFXList = nullptr;			// Special Effects list

	bool        m_bInMenu = false;			// within options menu
	char        m_szDataDirectory[100] = {};	// directory for data files

private:
	bool        m_bAnimationsPaused = false;	// Animations Paused/UnPaused

	// methods
public:
	CBgbMgr();
	~CBgbMgr();

	// bgb.cpp -- Boffo Game Objects handling routines
	//
	bool InitBitmapObject(CBgbObject *, XPSTR);
	bool SetPosition(CBgbObject *, CRPoint);
	bool PaintBitmapObject(CBgbObject *, bool bPaint PDFT(false), CRect *p = nullptr);
	bool AnimateSprite(CBgbObject *, CPoint, CPoint);
	bool InitDc(CView *xpView, CBsuSet *xpBsuSet PDFT(nullptr), CDC *xpDc PDFT(nullptr));
	bool ReInitDc();
	bool AdjustLockCount(int iIncr);
	bool SetBrush(COLORREF cBrushColor, int iBrushStyle PDFT(BS_SOLID));
	bool SetPen(COLORREF cPenColor, int iPenWidth PDFT(0));
	bool ReleaseDc();
	bool ClearBitmapObject(CBgbObject *lpcBgbObject);
	bool ReleaseResources();
	void DoAnimations();
	void PauseAnimations() {
		m_bAnimationsPaused = true;
	}
	void ResumeAnimations() {
		m_bAnimationsPaused = false;
	}
	bool AnimationsActive() const {
		return !m_bAnimationsPaused;
	}

	void CacheLoadObject(CBgbObject *);
	void CacheReleaseObject(CBgbObject *);
	void CacheFlush();
	void CacheOptimize(unsigned long);
};


// CBgbObject -- bitmap object
class CBgbObject {
public:
	char        m_szFileName[MAX_FNAME_LENGTH] = {}; // filename of object's bitmap

	int         m_iBgbType = 0;			// BGBT_xxxx -- object type
	CBgbObject *m_xpcNext = nullptr;	// pointer to next in chain
	CBgbObject *m_xpcPrev = nullptr;	// pointer to Previous in chain
	bool        m_bChained : 1;			// on m_xBgbChain
	bool        m_bNoDelete : 1;		// not allocated with "new"
	bool        m_bMasked : 1;			// mask white areas of bitmap
	bool        m_bInit : 1;			// true if object is initialized
	bool        m_bCleared : 1;			// false if need a ClearBackground()
	bool        m_bVisible : 1;			// bitmap is at least visible
	bool        m_bEdge : 1;			// bitmap is on edge of phys wnd
	bool        m_bLoaded : 1;			// true if currently in cache
	bool        m_bAnimated : 1;		// true if this sprite is animated
	bool        m_bSpecial : 1;			// true if Special Animation
	bool        m_bLocked : 1;			// true if object is locked in cache
	// (i.e. Cannot be released)

	int         m_nCels = 0;			// number of cels in this cel strip
	int         m_nZOrder = 0;			// Sprite Z-Order
	unsigned int        m_nFreq = 0;	// How often Animation should start
	unsigned int        m_nRepeat = 0;	// Number of Paint cycles to perform
	unsigned int        m_nFreqCount = 0;
	CRPoint     m_crPosition;           // relocatable position to paint bitmap
	CSize       m_cSize;                // size of bitmap
	CObject *m_pObject = nullptr;		// can be CSprite or CBitmap...
	unsigned long       m_lAge = 0;		// age of this object

public:
	CBgbObject() {
		m_bChained = false;
		m_bNoDelete = false;
		m_bMasked = false;
		m_bInit = false;
		m_bCleared = false;
		m_bVisible = false;
		m_bEdge = false;
		m_bLoaded = false;
		m_bAnimated = false;
		m_bSpecial = false;
		m_bLocked = false;
	}

	CRRect GetRect() {
		return CRRect(m_crPosition, m_cSize);
	}
	bool IfRelocatable() {
		return m_crPosition.m_bRelocatable;
	}
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
