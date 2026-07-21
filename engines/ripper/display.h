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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_DISPLAY_H
#define RIPPER_DISPLAY_H

#include "common/array.h"
#include "common/rect.h"

namespace Ripper {

enum {
	kRipperScreenWidth = 640,
	kRipperScreenHeight = 400,
	kRipperPaletteColorCount = 256,
	kRipperPaletteByteCount = kRipperPaletteColorCount * 3
};

class IndexedDisplaySnapshot {
public:
	IndexedDisplaySnapshot();

	bool capture();
	bool capture(const Common::Rect &bounds);
	bool restore(bool restorePalette = true, bool updateScreen = true) const;
	bool restorePixels() const;
	bool restorePalette() const;
	bool isValid() const;
	void clear();

	const Common::Rect &bounds() const { return _bounds; }
	const Common::Array<byte> &pixels() const { return _pixels; }
	const Common::Array<byte> &palette() const { return _palette; }

private:
	Common::Rect _bounds;
	Common::Array<byte> _pixels;
	Common::Array<byte> _palette;
};

} // End of namespace Ripper

#endif // RIPPER_DISPLAY_H
