/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "access/asurface.h"

namespace Access {


/*------------------------------------------------------------------------*/

int ASurface::_leftSkip;
int ASurface::_rightSkip;
int ASurface::_topSkip;
int ASurface::_bottomSkip;
int ASurface::_clipWidth;
int ASurface::_clipHeight;
int ASurface::_lastBoundsX;
int ASurface::_lastBoundsY;
int ASurface::_lastBoundsW;
int ASurface::_lastBoundsH;

void ASurface::clearBuffer() {
	byte *pSrc = (byte *)getPixels();
	Common::fill(pSrc, pSrc + w * h, 0);
}

bool ASurface::clip(Common::Rect &r) {
	int skip;
	_leftSkip = _rightSkip = 0;
	_topSkip = _bottomSkip = 0;

	if (r.left > _clipWidth) {
		skip = -r.left;
		r.setWidth(r.width() - skip);
		_leftSkip = skip;
		r.moveTo(0, r.top);
	}
	else if (r.left >= 0)
		return true;

	int right = r.right - 1;
	if (right < 0)
		return true;
	else if (right > _clipWidth) {
		skip = right - _clipWidth;
		r.setWidth(r.width() - skip);
		_rightSkip = skip;
	}

	if (r.top > _clipHeight) {
		skip = -r.top;
		r.setHeight(r.height() - skip);
		_topSkip = skip;
		r.moveTo(r.left, 0);
	}
	else if (r.top >= 0)
		return true;

	int bottom = r.bottom - 1;
	if (bottom < 0)
		return true;
	else if (bottom > _clipHeight) {
		skip = bottom - _clipHeight;
		_bottomSkip = skip;
		r.setHeight(r.height() - skip);
	}

	return false;
}

void ASurface::plotImage(SpriteResource *sprite, int frameNum, const Common::Point &pt) {
	SpriteFrame *frame = sprite->getFrame(frameNum);
	Common::Rect r(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h);

	if (!clip(r)) {
		_lastBoundsX = r.left;
		_lastBoundsY = r.top;
		_lastBoundsW = r.width();
		_lastBoundsH = r.height();

//		plotImage(frame, , )
	}
}

void ASurface::plotF(SpriteFrame *frame, const Common::Point &pt) {

}

} // End of namespace Access
