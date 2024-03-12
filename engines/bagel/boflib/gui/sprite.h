
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

#ifndef BAGEL_BOFLIB_GUI_SPRITE_H
#define BAGEL_BOFLIB_GUI_SPRITE_H

#include "bagel/boflib/gfx/sprite.h"

namespace Bagel {

class CSprite : public CBofSprite {
public:
	CSprite();
	virtual ~CSprite();

	CSprite *DuplicateSprite();
	BOOL DuplicateSprite(CSprite *pSprite);

	BOOL PaintSprite(CBofWindow *pWnd, const INT x, const INT y);
	BOOL PaintSprite(CBofWindow *pWnd, CBofPoint point) {
		return (PaintSprite(pWnd, point.x, point.y));
	}

	BOOL CropImage(CBofWindow *pWnd, CBofRect *pRect, BOOL bUpdateNow = TRUE);

	BOOL RefreshSprite(CBofWindow *pWnd) {
		return (PaintSprite(pWnd, m_cPosition.x, m_cPosition.y));
	}

	BOOL RefreshBackground(CBofWindow *pWnd);

	BOOL EraseSprite(CBofWindow *pWnd);

	VOID ClearBackground();

	VOID SetTypeCode(INT nValue) {
		m_nType = nValue;
	}
	INT GetTypeCode() {
		return (m_nType);
	}

	INT GetZPosition() {
		return (m_nZPosition);
	}

	VOID SetRetainBackground(BOOL bValue);
	BOOL GetRetainBackground() {
		return (m_bRetainBackground);
	}

	static BOOL EraseSprites(CBofWindow *pWnd);
	static VOID ClearBackgrounds();

private:
	BOOL UpdateSprite(CBofWindow *pWnd, BOOL bSaveBackground = TRUE);

	BOOL SaveBackground(CBofWindow *pWnd);

	BOOL CreateBackground();

	BOOL DoOptimizedPainting(CBofWindow *pWnd, CBofRect *pDst);
	BOOL DoOverlapPainting(CBofWindow *pWnd, CBofRect *myRect);
	BOOL ReconstructBackground(CBofWindow *pWnd, CBofRect *myRect);

private:
	CBofBitmap *m_pBackground; // bitmap for the sprite's background

	INT m_nType;      // user defined information
	INT m_nZPosition; // foreground / background placement

	CSprite *m_pZNext; // pointer to next sprite in z chain
	CSprite *m_pZPrev; // pointer to previous sprite in z chain

	BOOL m_bOverlaps : 1;         // sprite covers other sprites
	BOOL m_bPaintOverlap : 1;     // whether to paint sprite for overlaps
	BOOL m_bRetainBackground : 1; // retain background for screen updates
	BOOL m_bOverlapTest : 1;      // used for positional testing
};

} // namespace Bagel

#endif
