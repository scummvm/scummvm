
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

extern CBofPoint g_cInitLoc; // This is the initial location for the next new pan
extern BOOL g_bUseInitLoc;

//
// CBagPanWindow -
//  CBagPanWindow is a window that contains a slide bitmap object.  It has specialize
//  functions for handling slide bitmaps and slide objects.
//
class CBagPanWindow : public CBagStorageDevWnd {
private:
	static INT m_nCorrection;
	static INT m_nPanSpeed;

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
	CBofPalette *m_pPalette;

	CBofPoint m_bDraggingStart; // Starting location of the dragged object
	BOOL m_bDraggingObject;     // Whether or not the first FG object is being dragged

	BOOL m_bStretchToScreen;                 // Whether or not the backdrop is painted full screen
	BOOL m_bPaintToBackdrop;                 // Whether or not the backdrop is a extra paint surface
	// Access via CSprite::GetBackdrop()
	CBofList<CBagObject *> *m_pFGObjectList; // Objects to be painted to the window

	// CPoint              m_xCursorLocation;           // Current cursor location in bmp.
	// double                m_fxAspectRation;          // Maintain the current aspect ratio
	// double                m_fyAspectRation;          // Maintain the current aspect ratio

public:
	static CBagWield *m_pWieldBmp; // Pointer to the WEILD object

	CBagPanWindow();
	virtual ~CBagPanWindow();
	static void initialize();

	virtual ERROR_CODE AttachActiveObjects();

	virtual ERROR_CODE OnRender(CBofBitmap *pBmp, CRect *pRect = nullptr);

	/**
	 * The modal object is painted to the screen until it is done with
	 * its modal loop.  This is mainly used with characters.  It is called
	 * when the loop value of a modal character is set, before execution
	 * of the next line of script.
	 */
	ERROR_CODE RunModal(CBagObject *pObj);

	ERROR_CODE InsertFGObjects(CBagObject *pBmp);
	VOID DeleteFGObjects();
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

	VOID ActivateView();
	VOID DeActivateView();

	static VOID SetPanSpeed(INT nSpeed) {
		m_nPanSpeed = nSpeed;
	}
	static INT GetPanSpeed() {
		return m_nPanSpeed;
	}

#if BOF_MAC
	// We need to be able to call activate view and deactivate view outside
	// of the on_timer mechanism, use these methods to do it.
	VOID OnActivate();
	VOID OnDeActivate();
#endif

	virtual VOID Enable();
	virtual VOID Disable();

	ERROR_CODE PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp, CRect &viewOffsetRect,
	                        CBofList<CRect> * = nullptr, BOOL tempVar = TRUE);
	ERROR_CODE PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp) {
		CRect emptyRect;
		return PaintObjects(list, pBmp, emptyRect);
	}

	VOID SetViewPortPos(const CPoint &pos) {
		m_xVeiwPortPos = pos;    // Position of the viewport
	}
	VOID SetViewPortSize(const CSize &xViewSize) {
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
	VOID SetMovementRect(const CRect &rect) {
		m_xMovementRect = rect;
	}
	const CRect &GetMovementRect() {
		return m_xMovementRect;
	}
	VOID SetPaintToBackdrop(const BOOL bPaint) {
		m_bPaintToBackdrop = bPaint;
	}

	virtual const CBofPoint DevPtToViewPort(const CBofPoint &xPoint);
	virtual const CBofPoint ViewPortToDevPt(const CBofPoint &xPoint);
	BOOL GetStretchToScreen() {
		return m_bStretchToScreen;
	}
	BOOL SetStretchToScreen(BOOL val = TRUE) {
		return m_bStretchToScreen = val;
	}

	static INT GetRealCorrection() {
		return m_nCorrection;
	}
	static VOID SetRealCorrection(INT n) {
		m_nCorrection = n;
	}

	INT GetCorrection() {
		return m_pSlideBitmap->GetCorrWidth();
	}
	VOID SetCorrection(INT nCorr) {
		m_pSlideBitmap->SetCorrWidth(nCorr);
	}
	double GetFOV() {
		return m_pSlideBitmap->GetFOV();
	}
	VOID SetFOV(double fov) {
		m_pSlideBitmap->SetFOV(fov);
	}

	CBofPalette *GetPalette() {
		return m_pSlideBitmap->GetPalette();
	}

	uint32 Benchmark();

protected:
	BOOL CheckMessages();

public:
	static VOID FlushInputEvents();

	VOID OnClose();
	VOID OnMouseMove(uint32 nFlags, CBofPoint *p, void * = nullptr);
	VOID OnLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr);
	VOID OnLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr);
	VOID OnKeyHit(uint32 lKey, uint32 lRepCount);
	VOID OnSize(uint32 nType, int cx, int cy);
	VOID OnWindowPosChanging(WINDOWPOS *lpwndpos);
#if !BOF_MAC && defined(PAINT_TIMER)
	VOID OnTimer(uint32);
#endif
#if !BOF_MAC
	// Undefined on mac
	LONG OnDefWinProc(uint32 nMessage, int16 wParam, int32 lParam);
#endif
	ERROR_CODE OnCursorUpdate(INT nCurrObj);

	static CBagPDA *m_pPDABmp; // Pointer to the PDA object

	/**
	 * Deactivate the PDA by calling the PDA->Deactivate() directly.
	 * This is called from the PDA ON/OFF button
	 */
	BOOL DeactivatePDA();

	/**
	 * Activate the PDA by calling the PDA->Deactivate() directly.
	 * This is called on a mouse down anywhere on the deactivated PDA.
	 */
	BOOL ActivatePDA();

	void WaitForPDA() {
#if BOF_MAC
		{
			// Make sure our grafport is current
			STBofPort stSave(GetMacWindow());
#endif
			// Make sure we have a non-null pda
			while (m_pPDABmp && m_pPDABmp->IsActivating()) {
				SetPreFilterPan(TRUE);
				m_pPDABmp->SetDirty(TRUE);
				PaintScreen();
			}
#if BOF_MAC
		}
#endif
	}
};

} // namespace Bagel

#endif
