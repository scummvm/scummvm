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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/gfx/bmp_button.h"

namespace Bagel {
namespace HodjNPodj {

void BmpButton::loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled) {
	_base.loadBitmap(base);
	_selected.loadBitmap(selected);
	_focused.loadBitmap(focused);
	_disabled.loadBitmap(disabled);
}

void BmpButton::clear() {
	_base.clear();
	_selected.clear();
	_focused.clear();
	_disabled.clear();
}

void BmpButton::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_base);
}

} // namespace HodjNPodj
} // namespace Bagel
