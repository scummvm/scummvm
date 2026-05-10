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

#include "graphics/riscoscursor.h"
#include "common/util.h"

namespace Graphics {

class RiscOSCursor : public Cursor {
public:
    uint16 getWidth() const override { return _w; }
    uint16 getHeight() const override { return _h; }
    uint16 getHotspotX() const override { return _hotX; }
    uint16 getHotspotY() const override { return _hotY; }

    const byte *getSurface() const override { return _riscOSCursor; }
    const byte *getPalette() const override { return _riscOSPalette; }

    byte getKeyColor() const override { return _key; }
    byte getPaletteStartIndex() const override { return 0; }
    uint16 getPaletteCount() const override { return 3; }

private:
    static const uint16 _w = 11;
    static const uint16 _h = 11;
    static const uint16 _hotX = 0;
    static const uint16 _hotY = 0;
    static const byte _key = 0;

    static const byte _riscOSPalette[3 * 3];
    static const byte _riscOSCursor[_w * _h];
};

// Palette: transparent, light blue, dark blue
const byte RiscOSCursor::_riscOSPalette[3 * 3] = {
    0x00, 0x00, 0x00,
    0x00, 0xFF, 0xFF,
    0x00, 0x00, 0x99
};

// 11 x 11 RISC OS 3.11 cursor
const byte RiscOSCursor::_riscOSCursor[RiscOSCursor::_w * RiscOSCursor::_h] = {
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1,
    1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
    1, 2, 1, 1, 1, 2, 2, 1, 0, 0, 0,
    1, 1, 1, 0, 0, 1, 2, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1
};

Cursor *makeRiscOSCursor() {
    return new RiscOSCursor();
}

} // namespace Graphics
