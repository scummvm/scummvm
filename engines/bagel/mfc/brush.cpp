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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/gfx/gdi_objects.h"

namespace Bagel {
namespace MFC {

CBrush::CBrush() {
}

CBrush::CBrush(CBitmap *pBitmap) {
	m_hObject = new Gfx::CBrushImpl(pBitmap);
}

CBrush::CBrush(COLORREF crColor) {
	m_hObject = new Gfx::CBrushImpl(crColor);
}

CBrush::CBrush(int nIndex, COLORREF crColor) {
	m_hObject = new Gfx::CBrushImpl(nIndex, crColor);
}

BOOL CBrush::CreateSolidBrush(COLORREF crColor) {
	error("TODO: CreateSolidBrush");
}

BOOL CBrush::CreateBrushIndirect(const LOGBRUSH *lpLogBrush) {
	error("TODO: CBrush::CreateBrushIndirect");
}

BOOL CBrush::CreateStockObject(int nIndex) {
	error("TODO: CreateStockObject");
}

} // namespace MFC
} // namespace Bagel
