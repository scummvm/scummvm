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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"

#include "graphics/macgui/macwindowborder.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {

using namespace Graphics::MacGUIConstants;

MacWindowBorder::MacWindowBorder() : _activeInitialized(false), _inactiveInitialized(false) {
	_activeBorder = nullptr;
	_inactiveBorder = nullptr;

	_borderOffsets.left = -1;
	_borderOffsets.right = -1;
	_borderOffsets.top = -1;
	_borderOffsets.bottom = -1;
	_borderOffsets.titleTop = -1;
	_borderOffsets.titleBottom = -1;
	_borderOffsets.dark = false;
}

MacWindowBorder::~MacWindowBorder() {
	if (_activeBorder)
		delete _activeBorder;
	if (_inactiveBorder)
		delete _inactiveBorder;
}

bool MacWindowBorder::hasBorder(bool active) {
	return active ? _activeInitialized : _inactiveInitialized;
}

void MacWindowBorder::addActiveBorder(TransparentSurface *source) {
	if (_activeBorder)
		delete _activeBorder;

	_activeBorder = new NinePatchBitmap(source, true);
	_activeInitialized = true;

	if (_activeBorder->getPadding().isValidRect())
		setOffsets(_activeBorder->getPadding());
}

void MacWindowBorder::addInactiveBorder(TransparentSurface *source) {
	if (_inactiveBorder)
		delete _inactiveBorder;

	_inactiveBorder = new NinePatchBitmap(source, true);
	_inactiveInitialized = true;

	if (!_inactiveBorder->getPadding().isValidRect())
		setOffsets(_inactiveBorder->getPadding());
}

bool MacWindowBorder::hasOffsets() {
	return _borderOffsets.left > -1 && _borderOffsets.right > -1
		&& _borderOffsets.top > -1 && _borderOffsets.bottom > -1;
}

void MacWindowBorder::setOffsets(int left, int right, int top, int bottom) {
	_borderOffsets.left = left;
	_borderOffsets.right = right;
	_borderOffsets.top = top;
	_borderOffsets.bottom = bottom;
}

void MacWindowBorder::setOffsets(Common::Rect &rect) {
	_borderOffsets.left = rect.left;
	_borderOffsets.right = rect.right;
	_borderOffsets.top = rect.top;
	_borderOffsets.bottom = rect.bottom;
}

void MacWindowBorder::setOffsets(const BorderOffsets &offsets) {
	_borderOffsets = offsets;
}

BorderOffsets &MacWindowBorder::getOffset() {
	return _borderOffsets;
}

void MacWindowBorder::blitBorderInto(ManagedSurface &destination, bool active, MacWindowManager *wm) {

	TransparentSurface srf;
	NinePatchBitmap *src = active ? _activeBorder : _inactiveBorder;

	if ((active && !_activeInitialized) || (!active && !_inactiveInitialized)) {
		warning("Attempt to blit unitialised border");
	}

	if (destination.w == 0 || destination.h == 0) {
		warning("Attempt to draw %d x %d window", destination.w, destination.h);
		return;
	}

	srf.create(destination.w, destination.h, destination.format);
	srf.fillRect(Common::Rect(srf.w, srf.h), wm->_colorGreen2);

	src->blit(srf, 0, 0, srf.w, srf.h, NULL, 0, wm);
	destination.transBlitFrom(srf, wm->_colorGreen2);
	srf.free();
}

} // End of namespace Graphics
