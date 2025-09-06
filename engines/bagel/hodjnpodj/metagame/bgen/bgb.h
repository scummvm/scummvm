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
	char        m_cStartData;
	CView *m_xpcView;                 // current window, when non-null
	CDC *m_xpDc;                    // current device context
	COLORREF    m_cPenColor, m_cBrushColor;
	int         m_iPenWidth, m_iBrushStyle;
	CBrush *m_xpOldBrush;
	CPen *m_xpOldPen;
	bool        m_bCreatePen, m_bCreateBrush;
	bool        m_bReleaseDc;               // flag: release DC when done
	int         m_iLockCount;               // if positive, don't release DC

	CBsuSet *m_xpBsuSet;                 // scroll bar set, if any

	CPalette *m_xpGamePalette;            // game pallet
	CPalette *m_xpOldPalette;

	CBgbObject *m_lpBgbChain;               // chain of bgb objects

	CLList *m_pFXList;                  // Special Effects list

	bool        m_bInMenu;                  // within options menu
	char        m_szDataDirectory[100];     // directory for data files
	char        m_cEndData;

private:
	bool        m_bAnimationsPaused;        // Animations Paused/UnPaused

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
	char        m_cStartData;
	char        m_szFileName[MAX_FNAME_LENGTH]; // filename of object's bitmap

	int         m_iBgbType;             // BGBT_xxxx -- object type
	CBgbObject *m_xpcNext;              // pointer to next in chain
	CBgbObject *m_xpcPrev;              // pointer to Previous in chain
	bool        m_bChained : 1;          // on m_xBgbChain
	bool        m_bNoDelete : 1;         // not allocated with "new"
	bool        m_bMasked : 1;           // mask white areas of bitmap
	bool        m_bInit : 1;             // true if object is initialized
	bool        m_bCleared : 1;          // false if need a ClearBackground()
	bool        m_bVisible : 1;          // bitmap is at least visible
	bool        m_bEdge : 1;             // bitmap is on edge of phys wnd
	bool        m_bLoaded : 1;           // true if currently in cache
	bool        m_bAnimated : 1;         // true if this sprite is animated
	bool        m_bSpecial : 1;          // true if Special Animation
	bool        m_bLocked : 1;           // true if object is locked in cache
	// (i.e. Cannot be released)

	int         m_nCels;                // number of cels in this cel strip
	int         m_nZOrder;              // Sprite Z-Order
	unsigned int        m_nFreq;                // How often Animation should start
	unsigned int        m_nRepeat;              // Number of Paint cycles to perform
	unsigned int        m_nFreqCount;
	CRPoint     m_crPosition;           // relocatable position to paint bitmap
	CSize       m_cSize;                // size of bitmap
	CObject *m_pObject;              // can be CSprite or CBitmap...
	unsigned long       m_lAge;                 // age of this object
	char        m_cEndData;

public:
	CBgbObject() {
		memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
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
