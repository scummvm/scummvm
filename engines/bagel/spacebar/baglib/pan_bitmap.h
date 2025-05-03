
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

#ifndef BAGEL_BAGLIB_PAN_BITMAP_H
#define BAGEL_BAGLIB_PAN_BITMAP_H

#include "bagel/spacebar/boflib/gfx/bitmap.h"
#include "bagel/spacebar/boflib/fixed.h"

namespace Bagel {
namespace SpaceBar {

#define MAX_DIV_VIEW (12.8 / 3)     // Ratio for 480x380 Screen
#define DEF_FOV (360 / MAX_DIV_VIEW) //    1TO1 Paint FOV

class CBagPanBitmap : public CBofBitmap {
public:
	enum Direction {
		kDirNONE = 0x0,
		kDirUP = 0x01,
		kDirDOWN = 0x02,
		kDirLEFT = 0x04,
		kDirRIGHT = 0x08,
		kDirVIEW = 0x10
	};

private:
	CBofRect _xCurrView;      // Viewport Window size (0->Width-1,0->Height-1,1->Width+Width/4,1->Height)
	double _xFOVAngle;        // Field of view in radians
	CBofPoint _xRotateRate;   // Rate of rotation on increment left, right ...
	bool _bActiveScrolling;   // True when there should be screen updates
	bool _bPanorama;          // True when the bitmap is a 360 panorama
	Direction _xDirection;    // Set direction for next update
	int _nCorrWidth;          // Size of each correction band
	CBofFixed *_pCosineTable; // Lookup table for cosine values
	int _nNumDegrees;         // Number of lookups in the cosine table
	bool _bIsValid;           // Is the bmp a valid object

	void normalizeViewSize();
	void generateCosineTable();

public:
	CBagPanBitmap(const char *pszFileName, CBofPalette *pPalette, const CBofRect &xViewSize = CBofRect());
	virtual ~CBagPanBitmap();

	bool isValid() const {
		return _bIsValid;
	}
	bool isPan() const {
		return _bPanorama;
	}

	ErrorCode paint(CBofBitmap *pBmp, CBofPoint xDstOffset = CBofPoint(0, 0));
	ErrorCode paintUncorrected(CBofBitmap *pBmp, CBofRect &dstRect);
	ErrorCode paintWarped(CBofBitmap *pBmp, const CBofRect &dstRect, const CBofRect &srcRect, int offset = 0, CBofBitmap *pSrcBmp = nullptr, const CBofRect &preSrcRect = CBofRect());

	CBofRect getWarpSrcRect();
	CBofPoint warpedPoint(CBofPoint &xPoint);

	const CBofRect getMaxView(CBofSize s = CBofSize(640, 480));

	void setCorrWidth(int nWidth, bool bUpdate = true);

	void rotateRight(int nXRotRate = 0);
	void rotateLeft(int nXRotRate = 0);
	void rotateUp(int nYRotRate = 0);
	void rotateDown(int nYRotRate = 0);

	Direction updateView();

	void setCurrView(const CBofRect &xCurrView);
	void offsetCurrView(const CBofPoint &xOffset);
	void setFOV(double degrees, bool bUpdate = true);

	void setViewSize(const CBofSize &xViewSize, bool bUpdate = true);
	CBofSize setUnityViewSize();
	double setUnityFOV();

	double getFOV() const {
		return _xFOVAngle;
	}
	const CBofSize getViewSize() const {
		return CBofSize(_xCurrView.size());
	}
	const CBofRect getCurrView() const {
		return _xCurrView;
	}

	Direction getDirection() const {
		return _xDirection;
	}

	void setRotateRate(const CBofPoint &xRotRate) {
		_xRotateRate = xRotRate;
	}
	void setDirection(const Direction xDirection) {
		_xDirection = xDirection;
	}
	int getCorrWidth() {
		return _nCorrWidth;
	}

	void activateScrolling(bool val = true) {
		_bActiveScrolling = val;
	}
	void deActivateScrolling() {
		activateScrolling(false);
	}

};

} // namespace SpaceBar
} // namespace Bagel

#endif
