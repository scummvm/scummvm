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

#include "bagel/baglib/bmp_object.h"
#include "bagel/boflib/app.h"

namespace Bagel {

CBagBmpObject::CBagBmpObject() : CBagObject() {
	_xObjType = BMP_OBJ;
	_bmp = nullptr;
	_transparency = -1;

	CBagObject::setOverCursor(1);
	setTimeless(true);
}

CBagBmpObject::~CBagBmpObject() {
	CBagBmpObject::detach();
}

ErrorCode CBagBmpObject::attach(CBofPalette *palette) {
	_bmp = new CBofBitmap(getFileName(), palette);

	return CBagObject::attach();
}

ErrorCode CBagBmpObject::detach() {
	delete _bmp;
	_bmp = nullptr;

	return CBagObject::detach();
}

CBofRect CBagBmpObject::getRect() {
	const CBofPoint curPos = getPosition();
	CBofSize size;
	if (_bmp)
		size = _bmp->getSize();
	return CBofRect(curPos, size);
}

ErrorCode CBagBmpObject::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	if (bmp) {
		_transparency = maskColor;
		if (srcRect) {
			const CBofSize size = bmp->getSize();
			if (pt.x < 0) {
				srcRect->left -= pt.x;
				pt.x = 0;
			}
			if (pt.y < 0) {
				srcRect->top -= pt.y;
				pt.y = 0;
			}

			int offset = srcRect->right + pt.x - size.cx;
			if (offset >= 0) {
				srcRect->right -= offset + 1;
			}

			offset = srcRect->bottom + pt.y - size.cy;
			if (offset >= 0) {
				srcRect->bottom -= offset + 1;
			}
		}
		if (_bmp->paint(bmp, pt.x, pt.y, srcRect, maskColor))
			return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

bool CBagBmpObject::isInside(const CBofPoint &pt) {
	if (_bmp && getRect().ptInRect(pt)) {
		if (_transparency >= 0) {
			const int x = pt.x - getRect().left;
			const int y = pt.y - getRect().top;
			const int color = _bmp->readPixel(x, y);
			return color != _transparency;
		}

		return true;
	}

	return false;
}

} // namespace Bagel
