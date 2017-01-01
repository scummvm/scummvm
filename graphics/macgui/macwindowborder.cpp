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
	_hasOffsets = false;
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
	assert(!_activeBorder);
	_activeBorder = new NinePatchBitmap(source, true);
	_activeInitialized = true;
}

void MacWindowBorder::addInactiveBorder(TransparentSurface *source) {
	assert(!_inactiveBorder);
	_inactiveBorder = new NinePatchBitmap(source, true);
	_inactiveInitialized = true;
}

bool MacWindowBorder::hasOffsets() {
	return _hasOffsets;
}

void MacWindowBorder::setOffsets(int left, int right, int top, int bottom) {
	_borderOffsets[0] = left;
	_borderOffsets[1] = right;
	_borderOffsets[2] = top;
	_borderOffsets[3] = bottom;
	_hasOffsets = true;
}

int MacWindowBorder::getOffset(MacBorderOffset offset) {
	return _borderOffsets[offset];
}

void MacWindowBorder::blitBorderInto(ManagedSurface &destination, bool active) {

	TransparentSurface srf;
	NinePatchBitmap *src = active ? _activeBorder : _inactiveBorder;

	srf.create(destination.w, destination.h, destination.format);
	srf.fillRect(Common::Rect(0, 0, srf.w, srf.h), kColorGreen2);

	byte palette[kColorCount];
	g_system->getPaletteManager()->grabPalette(palette, 0, kColorCount);

	src->blit(srf, 0, 0, srf.w, srf.h, palette, kColorCount);
	destination.transBlitFrom(srf, kColorGreen2);
	srf.free();
}

} // End of namespace Graphics
