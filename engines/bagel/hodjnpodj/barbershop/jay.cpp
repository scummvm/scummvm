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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/barbershop/resource.h"
#include "bagel/hodjnpodj/barbershop/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

void MyFocusRect(CDC *pDC, CRect rect, int nDrawMode) {
	CBrush      *pMyBrush = nullptr;                   // New Brush
	CBrush      *pOldBrush = nullptr;                  // Pointer to old brush
	CPen        *pMyPen = nullptr;                     // New Pen
	CPen        *pOldPen = nullptr;                    // Pointer to old pen
	CPalette    *pPalOld = nullptr;                    // Pointer to old palette
	int         OldDrawMode;                        // Holder for old draw mode

	pMyBrush = new CBrush();                        // Construct new brush
	pMyPen = new CPen();                            // Construct new pen

	LOGBRUSH lb;                                    // log brush type
	lb.lbStyle = BS_HOLLOW;                         // Don't fill in area
	pMyBrush->CreateBrushIndirect(&lb);              // Create a new brush
	pMyPen->CreatePen(PS_INSIDEFRAME, HILITE_BORDER, RGBCOLOR_DARKRED);      // Create a new pen

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);     // Select in game palette
	(*pDC).RealizePalette();                                // Use it
	pOldPen = pDC->SelectObject(pMyPen);         // Select the new pen & save old
	pOldBrush = pDC->SelectObject(pMyBrush);     // Select the new brush & save old
	OldDrawMode = pDC->SetROP2(nDrawMode);       // Set pen mode, saving old state
	pDC->Rectangle(rect);                        // Draw the Rectangle to the DC
	pDC->SelectObject(pOldPen);                  // Select the old pen
	pDC->SelectObject(pOldBrush);                // Select the old brush
	pDC->SetROP2(OldDrawMode);                   // Set pen mode back to old state
	(*pDC).SelectPalette(pPalOld, false);           // Select back the old palette

	if (pMyBrush != nullptr) {                         // If the brush was constructed, delete it
		pMyBrush->DeleteObject();
		delete pMyBrush;
	}

	if (pMyPen != nullptr) {                           // If the pen was constructed, delete it
		pMyPen->DeleteObject();
		delete pMyPen;
	}

} // End MyFocusRect()

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
