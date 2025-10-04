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
#include "graphics/mfc/afxwin.h"

namespace Graphics {
namespace MFC {

CBrush::CBrush() {
}

CBrush::CBrush(CBitmap *pBitmap) {
	m_hObject = new Impl(pBitmap);
}

CBrush::CBrush(COLORREF crColor) {
	CreateSolidBrush(crColor);
}

CBrush::CBrush(int nIndex, COLORREF crColor) {
	m_hObject = new Impl(nIndex, crColor);
}

bool CBrush::CreateSolidBrush(COLORREF crColor) {
	DeleteObject();
	m_hObject = new Impl(crColor);

	AfxHookObject();
	return true;
}

bool CBrush::CreateBrushIndirect(const LOGBRUSH *lpLogBrush) {
	DeleteObject();
	m_hObject = new Impl(lpLogBrush->lbStyle,
	    lpLogBrush->lbColor);

	AfxHookObject();
	return true;
}

bool CBrush::CreateStockObject(int nIndex) {
	switch (nIndex) {
	case WHITE_BRUSH:
		CreateSolidBrush(RGB(255, 255, 255));
		break;
	case LTGRAY_BRUSH:
		CreateSolidBrush(RGB(192, 192, 192));
		break;
	case GRAY_BRUSH:
		CreateSolidBrush(RGB(160, 160, 160));
		break;
	case DKGRAY_BRUSH:
		CreateSolidBrush(RGB(128, 128, 128));
		break;
	case BLACK_BRUSH:
		CreateSolidBrush(RGB(0, 0, 0));
		break;
	default:
		error("TODO: CreateStockObject");
		break;
	}

	return true;
}

/*--------------------------------------------*/

CBrush::Impl::Impl() : _type(HS_HORIZONTAL) {
}

CBrush::Impl::Impl(COLORREF crColor) : _color(crColor),
	_type(HS_HORIZONTAL) {
}

CBrush::Impl::Impl(int nIndex, COLORREF crColor) :
	_type(nIndex), _color(crColor) {
}

CBrush::Impl::Impl(CBitmap *pBitmap) :
	_type(HS_HORIZONTAL) {
	error("TODO: CBrush::Impl for bitmaps");
}

byte CBrush::Impl::getColor() const {
	return AfxGetApp()->getColor(_color);
}

} // namespace MFC
} // namespace Graphics
