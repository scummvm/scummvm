
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

#ifndef BAGEL_BAGLIB_PAN_WINDOW_H
#define BAGEL_BAGLIB_PAN_WINDOW_H

#include "bagel/baglib/pda.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/wield.h"
#include "bagel/baglib/pan_bitmap.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

#define BOFCURS 1

#define DEF_WIDTH 639
#define DEF_HEIGHT 479

#define EVAL_EXPR 303

// The height and width of the pan area
#define PAN_AREA_WIDTH 480
#define PAN_AREA_HEIGHT 360

//
// CBagPanWindow -
//  CBagPanWindow is a window that contains a slide bitmap object.  It has specialize
//  functions for handling slide bitmaps and slide objects.
//
class CBagPanWindow : public CBagStorageDevWnd {
private:
	static int m_nCorrection;
	static int m_nPanSpeed;

	//
	// private data members
	//
	CPoint m_xVeiwPortPos; // Position of the viewport
	CRect m_xMovementRect; // if mouse is in this region will not scroll
	CRect m_cLeftRect;
	CRect m_cRightRect;
	CRect m_cTopRect;
	CRect m_cBottomRect;

	CBagPanBitmap *m_pSlideBitmap; // Contains the full bitmap of the world
	CBofBitmap *m_pVeiwPortBitmap; // Contains the current viewport bitmap
	CBofPalette *_pPalette;

	CBofPoint m_bDraggingStart; // Starting location of the dragged object
	bool m_bDraggingObject;     // Whether or not the first FG object is being dragged

	bool m_bStretchToScreen;                 // Whether or not the backdrop is painted full screen
	bool m_bPaintToBackdrop;                 // Whether or not the backdrop is a extra paint surface
	// Access via CSprite::getBackdrop()
	CBofList<CBagObject *> *m_pFGObjectList; // Objects to be painted to the window

	// CPoint              m_xCursorLocation;           // Current cursor location in bmp.
	// double                m_fxAspectRation;          // Maintain the current aspect ratio
	// double                m_fyAspectRation;          // Maintain the current aspect ratio

public:
	static CBagWield *_pWieldBmp; // Pointer to the WEILD object

	CBagPanWindow();
	virtual ~CBagPanWindow();
	static void initialize();

	virtual ErrorCode AttachActiveObjects();

	virtual ErrorCode onRender(CBofBitmap *pBmp, CRect *pRect = nullptr);

	/**
	 * The modal object is painted to the screen until it is done with
	 * its modal loop.  This is mainly used with characters.  It is called
	 * when the loop value of a modal character is set, before execution
	 * of the next line of script.
	 */
	ErrorCode RunModal(CBagObject *pObj);

	ErrorCode InsertFGObjects(CBagObject *pBmp);
	void DeleteFGObjects();
	CBagObject *GetFGObjects(const CBofString &sObjName);

	CBofPalette *SetSlidebitmap(const CBofString &xSlideBmp, const CRect &xSlideRect = CRect(0, 0, 0, 0));

	/**
	 * Clear the last pan bitmaps and return a rect of the current view
	 */
	CBofRect UnSetSlidebitmap();

	CBagPanBitmap *GetSlideBitmap() {
		return m_pSlideBitmap;
	}

	uint32 RotateTo(CPoint xPoint, int nRate = 8);

	void ActivateView();
	void DeActivateView();

	static void SetPanSpeed(int nSpeed) {
		m_nPanSpeed = nSpeed;
	}
	static int GetPanSpeed() {
		return m_nPanSpeed;
	}

	virtual void enable();
	virtual void disable();

	ErrorCode PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp, CRect &viewOffsetRect,
	                        CBofList<CRect> * = nullptr, bool tempVar = true);
	ErrorCode PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CRect emptyRect;
		return PaintObjects(list, pBmp, emptyRect);
	}

	void SetViewPortPos(const CPoint &pos) {
		m_xVeiwPortPos = pos;    // Position of the viewport
	}
	void SetViewPortSize(const CSize &xViewSize) {
		if (m_pSlideBitmap)
			m_pSlideBitmap->SetViewSize(xViewSize);
	}
	const CRect GetViewPort() {
		CRect r;
		if (m_pSlideBitmap)
			r = m_pSlideBitmap->GetCurrView();
		return r;
	}
	const CPoint GetViewPortPos() {
		return m_xVeiwPortPos;
	}
	const CSize GetViewPortSize() {
		CRect r;
		if (m_pSlideBitmap)
			r = m_pSlideBitmap->GetCurrView();
		return r.Size();
	}
	const CRect GetMaxView() {
		return m_pSlideBitmap->GetMaxView();
	}
	void SetMovementRect(const CRect &rect) {
		m_xMovementRect = rect;
	}
	const CRect &GetMovementRect() {
		return m_xMovementRect;
	}
	void SetPaintToBackdrop(const bool bPaint) {
		m_bPaintToBackdrop = bPaint;
	}

	virtual const CBofPoint DevPtToViewPort(const CBofPoint &xPoint);
	virtual const CBofPoint ViewPortToDevPt(const CBofPoint &xPoint);
	bool GetStretchToScreen() {
		return m_bStretchToScreen;
	}
	bool SetStretchToScreen(bool val = true) {
		return m_bStretchToScreen = val;
	}

	static int GetRealCorrection() {
		return m_nCorrection;
	}
	static void SetRealCorrection(int n) {
		m_nCorrection = n;
	}

	int GetCorrection() {
		return m_pSlideBitmap->getCorrWidth();
	}
	void SetCorrection(int nCorr) {
		m_pSlideBitmap->setCorrWidth(nCorr);
	}
	double GetFOV() {
		return m_pSlideBitmap->GetFOV();
	}
	void SetFOV(double fov) {
		m_pSlideBitmap->SetFOV(fov);
	}

	CBofPalette *getPalette() {
		return m_pSlideBitmap->getPalette();
	}

	uint32 Benchmark();

public:
	static void FlushInputEvents();

	void onClose();
	void onMouseMove(uint32 nFlags, CBofPoint *p, void * = nullptr);
	void onLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr);
	void onLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr);
	void onKeyHit(uint32 lKey, uint32 lRepCount);
	void onSize(uint32 nType, int cx, int cy);
	void OnWindowPosChanging(WindowPos *lpwndpos);

	ErrorCode onCursorUpdate(int nCurrObj);

	static CBagPDA *_pPDABmp; // Pointer to the PDA object

	/**
	 * Deactivate the PDA by calling the PDA->deactivate() directly.
	 * This is called from the PDA ON/OFF button
	 */
	bool DeactivatePDA();

	/**
	 * Activate the PDA by calling the PDA->deactivate() directly.
	 * This is called on a mouse down anywhere on the deactivated PDA.
	 */
	bool ActivatePDA();

	void WaitForPDA() {
		// Make sure we have a non-null pda
		while (_pPDABmp && _pPDABmp->isActivating()) {
			SetPreFilterPan(true);
			_pPDABmp->SetDirty(true);
			PaintScreen();
		}
	}
};

} // namespace Bagel

#endif
