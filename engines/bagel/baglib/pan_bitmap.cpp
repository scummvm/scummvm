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

#include "bagel/baglib/pan_bitmap.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/paint_table.h"

namespace Bagel {

const CBofRect CBagPanBitmap::GetMaxView(CBofSize s) {
	CBofRect r;

	if (m_bPanorama) {
		int h = 3 * (int)((double)Width() / (MAXDIVVIEW * 4));
		if (h > Height())
			h = Height();

		r.SetRect(0, 0, (INT)(Width() / MAXDIVVIEW - 2), h - 1);

	} else {
		r.SetRect(0, 0, Width() - 1, Height() - 1);
	}

	if (s.cx > 0 && r.right > s.cx) {
		// r.left = s.cx;
		r.right = s.cx - 1;
	}

	if (s.cy > 0 && r.bottom > s.cy) {
		r.bottom = s.cy - 1;
	}

	return r;
}

CBagPanBitmap::CBagPanBitmap(const CHAR *pszFileName, CBofPalette *pPalette, const CBofRect &xViewSize) :
#ifdef COMPRESSED
	CBofCompressedBitmap(pszFileName, pPalette)
#else
	CBofBitmap(pszFileName, pPalette, TRUE)
#endif
{
	int nW = Width();
	int nH = Height();

	if (nW && nH) {
		CBofRect xMaxViewSize(0, 0, nW - 1, nH - 1);
		if (nW > 1000) {
			xMaxViewSize.left = (long)(nW / MAXDIVVIEW);
			m_bPanorama = TRUE;
		} else
			m_bPanorama = FALSE;

		m_pCosineTable = nullptr;
		m_bActiveScrolling = FALSE; // The scrolling is not active
		m_xDirection = kDirNONE;        // Direction is not moving

		pPalette = GetPalette();

		if (xViewSize.IsRectEmpty())
			m_xCurrView = xMaxViewSize;
		else
			m_xCurrView = xViewSize;

		if (m_xCurrView.Width() > xMaxViewSize.Width()) {
			m_xCurrView.SetRect(0, m_xCurrView.top, xMaxViewSize.Width() - 1, m_xCurrView.bottom);
		}

		m_xRotateRate.x = (nW - m_xCurrView.Width()) / 64 + 1;
		m_xRotateRate.y = (nH - m_xCurrView.Height()) / 64 + 1;

		NormalizeViewSize();

		SetFOV(DEFFOV); // If FOV is set to 0 then unity FOV is assumed (faster redraws)

		// m_nCorrWidth is uninitialized for the call to SetFOV below,
		// this causes the cosinetable to be allocated incorrectly in
		// GenerateCosineTable.  Move the initialization before SetFOV.
		if (m_bPanorama)
			SetCorrWidth(4);
		else
			SetCorrWidth(0);

		m_bIsValid = TRUE;

		return;
	}
	m_bIsValid = FALSE;
}

CBagPanBitmap::CBagPanBitmap(INT dx, INT dy, CBofPalette *pPalette, const CBofRect &xViewSize) :
#ifdef COMPRESSED
	CBofCompressedBitmap(dx, dy, pPalette)
#else
	CBofBitmap(dx, dy, pPalette)
#endif
{
	int nW = Width();
	int nH = Height();

	if (nW <= 0 || nH <= 0) {
		// BofMessageBox(pszFileName, __FILE__);
		// Assert(FALSE);
	}

	if (nW && nH) {
		CBofRect xMaxViewSize(0, 0, nW - 1, nH - 1);
		if (nW > 1000) {
			xMaxViewSize.left = (long)(nW / MAXDIVVIEW);
			m_bPanorama = TRUE;
		} else
			m_bPanorama = FALSE;

		m_pCosineTable = nullptr;
		m_bActiveScrolling = FALSE; // The scrolling is not active
		m_xDirection = kDirNONE;        // Direction is not moving

		pPalette = GetPalette();

		if (xViewSize.IsRectEmpty())
			m_xCurrView = xMaxViewSize;
		else
			m_xCurrView = xViewSize;

		if (m_xCurrView.Width() > xMaxViewSize.Width()) {
			m_xCurrView.SetRect(0, m_xCurrView.top, xMaxViewSize.Width() - 1, m_xCurrView.bottom);
		}

		m_xRotateRate.x = (nW - m_xCurrView.Width()) / 64 + 1;
		m_xRotateRate.y = (nH - m_xCurrView.Height()) / 64 + 1;

		NormalizeViewSize();

		SetFOV(DEFFOV); // If FOV is set to 0 then unity FOV is assumed (faster redraws)

		if (m_bPanorama)
			SetCorrWidth(4);
		else
			SetCorrWidth(0);

		m_bIsValid = TRUE;

		return;
	}
	m_bIsValid = FALSE;
}

CBagPanBitmap::~CBagPanBitmap() {
	Assert(IsValidObject(this));

	if (m_pCosineTable) {
		delete[] m_pCosineTable;
		m_pCosineTable = nullptr;
	}
}

// this must be updated whenever the size, view size, or correction witdh changes
void CBagPanBitmap::GenerateCosineTable() {
	int nWidth = 1 << m_nCorrWidth;
	int offset = nWidth >> 1; // This is not really needed just more correction to move angle to center
	int viewWidth = m_xCurrView.Width();

	m_nNumDegrees = (viewWidth >> m_nCorrWidth) + 1;

	if (m_pCosineTable) {
		delete[] m_pCosineTable;
		m_pCosineTable = nullptr;
	}

#if BOF_MAC
	Assert(m_nNumDegrees <= 720);
	if (m_nNumDegrees <= 90) {
		m_pCosineTable = new CBofFixed[90];
	} else {
		if (m_nNumDegrees <= 360) {
			m_pCosineTable = new CBofFixed[360];
		} else {
			m_pCosineTable = new CBofFixed[720];
		}
	}
#else
	m_pCosineTable = new CBofFixed[m_nNumDegrees];
#endif

	for (int i = 0; i < m_nNumDegrees; i++) {
		double inArea = (double)(offset + i * nWidth) / viewWidth;
		m_pCosineTable[i] = (CBofFixed)(cos(m_xFOVAngle * (-1.0 + 2.0 * inArea)));
	}
}

ERROR_CODE CBagPanBitmap::Paint(CBofWindow * /*pWnd*/, const CBofPoint /*xDstOffset*/) {
	return m_errCode;
}

CBofRect CBagPanBitmap::GetWarpSrcRect() {
	//CBofRect r = GetRect();
	int nH2 = Height() >> 1;

	return CBofRect(m_xCurrView.left,
	                nH2 + (int)(*m_pCosineTable * CBofFixed(m_xCurrView.top - nH2)),
	                m_xCurrView.right,
	                nH2 + (int)(*m_pCosineTable * CBofFixed(m_xCurrView.bottom - nH2)));
}

CBofPoint CBagPanBitmap::WarpedPoint(CBofPoint &xPoint) {
	CBofRect r = GetRect();
	int nH2 = Height() >> 1;
	int nW = Width();
	int nWidth = 1 << m_nCorrWidth; // It may no longer be nessasary to store corr witdh as a shift arg
	int nCenter = r.top + nH2;

	int nOffset = (xPoint.x - m_xCurrView.left); // nWidth;
	if (nOffset < 0)
		nOffset += nW;
	nOffset /= nWidth;

	if ((nOffset < 0) || ((xPoint.x - m_xCurrView.left) > m_xCurrView.Width()) || (nOffset >= m_nNumDegrees))
		return CBofPoint(0, 0);

	CBofFixed srcHeight = m_pCosineTable[nOffset];

	return CBofPoint(xPoint.x, nCenter + (int)(srcHeight * CBofFixed(xPoint.y - nH2)));
}

ERROR_CODE CBagPanBitmap::PaintWarped(CBofBitmap *pBmp, const CBofRect &dstRect, const CBofRect &srcRect, const int offset, CBofBitmap *pSrcBmp, const CBofRect &preSrcRect) {
	INT nH2 = m_nDY >> 1;
	INT nWidth = 1 << m_nCorrWidth; // It may no longer be nessasary to store corr witdh as a shift arg
	CBofFixed *pSrcHeight = &m_pCosineTable[offset >> m_nCorrWidth];
	CBofFixed srcTop = preSrcRect.top + srcRect.top - nH2;
	CBofFixed srcBottom = preSrcRect.top + srcRect.bottom - nH2;
	INT nCenter = nH2;
	INT i;
	INT nTop;
	INT nRight;

	nTop = nCenter - preSrcRect.top;
	nRight = (srcRect.left + nWidth) - 1;

	CBofRect PanSrcRect;
	CBofRect WndDstRect(dstRect.left + 0, dstRect.top, (dstRect.left + 0 + nWidth) - 1, dstRect.bottom);

	pBmp->Lock();
	pSrcBmp->Lock();

	INT nIncrement = 1;

	if (nWidth < 4) {
		for (i = 0; i < dstRect.Width(); i += nWidth) {
			// Set the source
			//
			PanSrcRect.SetRect(srcRect.left + i,
			                   nTop + (INT)(*pSrcHeight * srcTop),
			                   nRight + i,
			                   nTop + (INT)(*pSrcHeight * srcBottom));

			pSrcBmp->Paint(pBmp, &WndDstRect, &PanSrcRect);

			WndDstRect.left = WndDstRect.right + 1;
			WndDstRect.right = WndDstRect.right + nWidth;
			pSrcHeight += nIncrement;
		}
	} else if (nWidth == 4) {
		INT tableSlot = srcRect.top + preSrcRect.top;
		INT stripNumber;

		for (i = 0, stripNumber = 0; i < dstRect.Width(); i += nWidth, stripNumber++) {
			// Set the source
			PanSrcRect.SetRect(srcRect.left + i,
			                   STRIP_POINTS[tableSlot][stripNumber].top,
			                   nRight + i,
			                   STRIP_POINTS[tableSlot][stripNumber].bottom);

			pSrcBmp->PaintStretch4(pBmp, &WndDstRect, &PanSrcRect);

			WndDstRect.left = WndDstRect.right + 1;
			WndDstRect.right = WndDstRect.right + nWidth;
			pSrcHeight += nIncrement;
		}
	} else { // nWidth > 4
		for (i = 0; i < dstRect.Width(); i += nWidth) {
			// Set the source
			//
			PanSrcRect.SetRect(srcRect.left + i,
			                   nTop + (INT)(*pSrcHeight * srcTop),
			                   nRight + i,
			                   nTop + (INT)(*pSrcHeight * srcBottom));

			pSrcBmp->PaintStretchOpt(pBmp, &WndDstRect, &PanSrcRect, nWidth);

			WndDstRect.left = WndDstRect.right + 1;
			WndDstRect.right = WndDstRect.right + nWidth;
			pSrcHeight += nIncrement;
		}
	}

	pSrcBmp->UnLock();
	pBmp->UnLock();

	return m_errCode;
}

ERROR_CODE CBagPanBitmap::PaintUncorrected(CBofBitmap *pBmp, CBofRect &dstRect) {
	int tmp = m_nCorrWidth;
	CBofRect tmpRect;
	m_nCorrWidth = 0;

	CBofFixed fONE(1);
	CBofFixed fH2(Height() / 2);
	CBofFixed fCos(m_pCosineTable[0]);
	int nOffset = (int)((fONE - fCos) * fH2);

	dstRect = GetCurrView();
	tmpRect = dstRect;
	dstRect.top -= nOffset;
	dstRect.bottom += nOffset;

	if (dstRect.top < 0) {
		dstRect.bottom = dstRect.Height() - 1;
		dstRect.top = 0;
	}
	if (dstRect.Height() >= pBmp->Height()) {
		dstRect.bottom = dstRect.top + pBmp->Height() - 2;
	}
	m_xCurrView = dstRect;

	Paint(pBmp);
	m_xCurrView = tmpRect;

	m_nCorrWidth = tmp;

	return m_errCode;
}

ERROR_CODE CBagPanBitmap::Paint(CBofBitmap *pBmp, const CBofPoint xDstOffset) {
	CBofRect dstRect;
	CBofRect srcRect = m_xCurrView;
	int nW = Width();
	int viewWidth = m_xCurrView.Width();
#ifdef RESIZEABLE
	double viewAngle = 3.14159 * (double)viewWidth / nW;
	double rescaleFactor = viewAngle / m_xFOVAngle;
	srcRect.right = m_xCurrView.left + (int)(viewWidth / rescaleFactor);
#else
	srcRect.right = m_xCurrView.left + viewWidth - 1;
#endif
	int nOffset = srcRect.right - nW;
	//CBofRect r = GetRect();

	dstRect.top = xDstOffset.y;
	dstRect.bottom = dstRect.top + srcRect.Height() - 1;

	// If the right side of the view is the begining of the panorama
	// paint the un-wrapped side (right) first.

	dstRect.left = xDstOffset.x;
	dstRect.right = xDstOffset.x + viewWidth - 1;

	if (nOffset > 0) {
		CBofRect srcRect2 = srcRect;
		srcRect2.left = 0;

#ifdef RESIZEABLE
		srcRect2.right = nOffset / rescaleFactor;
		dstRect.right = xDstOffset.x + viewWidth;
		dstRect.left = dstRect.right - nOffset * rescaleFactor;
#else
		srcRect2.right = nOffset;
		dstRect.right = xDstOffset.x + viewWidth - 1;
		dstRect.left = dstRect.right - nOffset;
#endif

		CBofBitmap::Paint(pBmp, &dstRect, &srcRect2);

		srcRect.right = nW - 1;
		dstRect.right = dstRect.left;
		dstRect.left = xDstOffset.x;
	}

	CBofBitmap::Paint(pBmp, &dstRect, &srcRect);

	return m_errCode;
}

CBagPanBitmap::Direction CBagPanBitmap::UpdateView() {
	if (m_bActiveScrolling) {

		if (m_xDirection & kDirLEFT)
			RotateRight();
		else if (m_xDirection & kDirRIGHT)
			RotateLeft();

		if (m_xDirection & kDirUP)
			RotateUp();
		else if (m_xDirection & kDirDOWN)
			RotateDown();

		return m_xDirection;
	}

	return kDirNONE;
}

VOID CBagPanBitmap::SetCorrWidth(int nWidth, BOOL bUpdate) {
	int i = 0;

	while (nWidth >>= 1)
		++i;

	if (i >= 0 && i < 6) {
		m_nCorrWidth = i;
		if (bUpdate) {
			GenerateCosineTable();
		}
	}
}

void CBagPanBitmap::RotateRight(int nXRotRate) {
	if (nXRotRate > 0)
		OffsetCurrView(CBofPoint(nXRotRate, 0));
	else
		OffsetCurrView(CBofPoint(m_xRotateRate.x, 0));
}

void CBagPanBitmap::RotateLeft(int nXRotRate) {
	if (nXRotRate > 0)
		OffsetCurrView(CBofPoint(-1 * nXRotRate, 0));
	else
		OffsetCurrView(CBofPoint(-1 * m_xRotateRate.x, 0));
}

void CBagPanBitmap::RotateUp(int nYRotRate) {
	if (nYRotRate > 0)
		OffsetCurrView(CBofPoint(0, nYRotRate));
	else
		OffsetCurrView(CBofPoint(0, m_xRotateRate.y));
}

void CBagPanBitmap::RotateDown(int nYRotRate) {
	if (nYRotRate > 0)
		OffsetCurrView(CBofPoint(0, -1 * nYRotRate));
	else
		OffsetCurrView(CBofPoint(0, -1 * m_xRotateRate.y));
}

void CBagPanBitmap::NormalizeViewSize() {
	int nW = Width();
	int nH = Height();

	if (m_bPanorama) {
		// The CurrView can not be more than 0.25Width x Height of the Bitmap
		if ((m_xCurrView.Width() >= nW / MAXDIVVIEW) || (m_xCurrView.Width() <= 0))
			m_xCurrView.right = (long)(m_xCurrView.left + nW / MAXDIVVIEW - 1);
		if ((m_xCurrView.Height() >= nH) || (m_xCurrView.Height() <= 0))
			m_xCurrView.bottom = m_xCurrView.top + nH - 1;

		// The Base coords of CurrView must exist within the rectange
		while (m_xCurrView.left < 0)
			m_xCurrView.OffsetRect(nW, 0);
		while (m_xCurrView.left >= nW)
			m_xCurrView.OffsetRect(-1 * nW, 0);
	} else { // Not a panorama
		// The Base coords of CurrView must exist within the rectange
		if (m_xCurrView.left < 0)
			m_xCurrView.OffsetRect(-m_xCurrView.left, 0);
		else if (m_xCurrView.right > nW)
			m_xCurrView.OffsetRect(nW - m_xCurrView.right, 0);
	}
	// We never have up and down wrap around
	if (m_xCurrView.top < 0)
		m_xCurrView.OffsetRect(0, -m_xCurrView.top);
	if (m_xCurrView.bottom >= nH)
		m_xCurrView.OffsetRect(0, (nH - m_xCurrView.bottom) - 1);
}

} // namespace Bagel
