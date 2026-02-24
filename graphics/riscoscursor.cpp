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

static const uint16 w = 11;
static const uint16 h = 11;
static const uint16 hotX = 0;
static const uint16 hotY = 0;
static const byte key = 0xFF;

static const byte riscOSCursor[w * h] = {
    0xE5, 0xE5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xE5, 0xE6, 0xE5, 0xE5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xE5, 0xE6, 0xE6, 0xE6, 0xE5, 0xE5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xE5, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6, 0xE5, 0xE5, 0xE5, 0xFF, 0xFF,
    0xE5, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6, 0xE5, 0xE5,
    0xE5, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6, 0xE5, 0xE5, 0xE5, 0xE5, 0xE5,
    0xE5, 0xE6, 0xE5, 0xE5, 0xE5, 0xE6, 0xE6, 0xE5, 0xFF, 0xFF, 0xFF,
    0xE5, 0xE5, 0xE5, 0xFF, 0xFF, 0xE5, 0xE6, 0xE6, 0xE5, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5, 0xE6, 0xE6, 0xE5, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5, 0xE6, 0xE6, 0xE5,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5, 0xE5, 0xE5,
};

class RiscOSCursor : public Cursor {
public:
    RiscOSCursor() {
        memset(_palette, 0, sizeof(_palette));

        // 0xE5 = light blue
        _palette[0xE5 * 3 + 0] = 0x00;
        _palette[0xE5 * 3 + 1] = 0xFF;
        _palette[0xE5 * 3 + 2] = 0xFF;

        // 0xE6 = dark blue
        _palette[0xE6 * 3 + 0] = 0x00;
        _palette[0xE6 * 3 + 1] = 0x00;
        _palette[0xE6 * 3 + 2] = 0x99;
    }

    uint16 getWidth() const override { return w; }
    uint16 getHeight() const override { return h; }
    uint16 getHotspotX() const override { return hotX; }
    uint16 getHotspotY() const override { return hotY; }

    const byte *getSurface() const override { return riscOSCursor; }
    const byte *getPalette() const override { return _palette; }

    byte getKeyColor() const override { return key; }
    byte getPaletteStartIndex() const override { return 0; }
    uint16 getPaletteCount() const override { return 256; }

private:
    byte _palette[256 * 3];
};

Cursor *makeRiscOSCursor() {
    return new RiscOSCursor();
}

} // namespace Graphics
