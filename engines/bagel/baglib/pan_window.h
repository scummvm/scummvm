
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
	static int _nCorrection;
	static int _nPanSpeed;

	//
	// private data members
	//
	CPoint _xViewPortPos; // Position of the viewport
	CRect _xMovementRect; // if mouse is in this region will not scroll
	CRect _cLeftRect;
	CRect _cRightRect;
	CRect _cTopRect;
	CRect _cBottomRect;

	CBagPanBitmap *_pSlideBitmap; // Contains the full bitmap of the world
	CBofBitmap *_pViewPortBitmap; // Contains the current viewport bitmap
	CBofPalette *_pPalette;

	CBofPoint _bDraggingStart; // Starting location of the dragged object
	bool _bDraggingObject;     // Whether or not the first FG object is being dragged

	bool _bStretchToScreen;                 // Whether or not the backdrop is painted full screen
	bool _bPaintToBackdrop;                 // Whether or not the backdrop is a extra paint surface

	CBofList<CBagObject *> *_pFGObjectList; // Objects to be painted to the window

public:
	static CBagWield *_pWieldBmp; // Pointer to the WIELD object

	CBagPanWindow();
	virtual ~CBagPanWindow();
	static void initialize();

	ErrorCode attachActiveObjects() override;

	ErrorCode onRender(CBofBitmap *pBmp, CRect *pRect = nullptr) override;

	/**
	 * The modal object is painted to the screen until it is done with
	 * its modal loop.  This is mainly used with characters.  It is called
	 * when the loop value of a modal character is set, before execution
	 * of the next line of script.
	 */
	ErrorCode runModal(CBagObject *pObj) override;

	ErrorCode insertFGObjects(CBagObject *pBmp);
	void deleteFGObjects();
	CBagObject *getFGObjects(const CBofString &sObjName);

	CBofPalette *setSlideBitmap(const CBofString &xSlideBmp, const CRect &xSlideRect = CRect(0, 0, 0, 0));

	/**
	 * Clear the last pan bitmaps and return a rect of the current view
	 */
	CBofRect unSetSlideBitmap();

	uint32 rotateTo(CPoint &xPoint, int nRate = 8);

	void activateView();
	void deActivateView();

	void enable() override;
	void disable() override;

	ErrorCode paintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp, CRect &viewOffsetRect,
	                        CBofList<CRect> * = nullptr, bool tempVar = true) override;

	const CBofPoint devPtToViewPort(const CBofPoint &xPoint) override;
	const CBofPoint viewPortToDevPt(const CBofPoint &xPoint) override;

	void setViewPortSize(const CSize &xViewSize);
	const CRect getViewPort();
	const CSize getViewPortSize();

	CBagPanBitmap *getSlideBitmap() const {
		return _pSlideBitmap;
	}
	void setViewPortPos(const CPoint &pos) {
		_xViewPortPos = pos;    // Position of the viewport
	}

	static void setPanSpeed(int nSpeed) {
		_nPanSpeed = nSpeed;
	}
	static int getPanSpeed() {
		return _nPanSpeed;
	}

	const CPoint getViewPortPos() {
		return _xViewPortPos;
	}

	const CRect getMaxView() {
		return _pSlideBitmap->getMaxView();
	}
	void setMovementRect(const CRect &rect) {
		_xMovementRect = rect;
	}
	const CRect &getMovementRect() {
		return _xMovementRect;
	}
	void setPaintToBackdrop(const bool bPaint) {
		_bPaintToBackdrop = bPaint;
	}
	bool getStretchToScreen() {
		return _bStretchToScreen;
	}
	bool setStretchToScreen(bool val = true) {
		return _bStretchToScreen = val;
	}
	static int getRealCorrection() {
		return _nCorrection;
	}
	static void setRealCorrection(int n) {
		_nCorrection = n;
	}
	int getCorrection() {
		return _pSlideBitmap->getCorrWidth();
	}
	void setCorrection(int nCorr) {
		_pSlideBitmap->setCorrWidth(nCorr);
	}
	double getFOV() {
		return _pSlideBitmap->getFOV();
	}
	void setFOV(double fov) {
		_pSlideBitmap->setFOV(fov);
	}

	CBofPalette *getPalette() {
		return _pSlideBitmap->getPalette();
	}

	uint32 benchmark();

public:
	static void flushInputEvents();

	void onClose() override;
	void onMouseMove(uint32 nFlags, CBofPoint *p, void * = nullptr) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *point, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *point, void * = nullptr) override;
	void onKeyHit(uint32 lKey, uint32 lRepCount) override;
	virtual void onSize(uint32 nType, int cx, int cy);
	void OnWindowPosChanging(WindowPos *lpwndpos);

	ErrorCode onCursorUpdate(int nCurrObj) override;

	static CBagPDA *_pPDABmp; // Pointer to the PDA object

	/**
	 * Deactivate the PDA by calling the PDA->deactivate() directly.
	 * This is called from the PDA ON/OFF button
	 */
	bool deactivatePDA();

	/**
	 * Activate the PDA by calling the PDA->deactivate() directly.
	 * This is called on a mouse down anywhere on the deactivated PDA.
	 */
	bool activatePDA();

	void waitForPDA();
};

} // namespace Bagel

#endif
