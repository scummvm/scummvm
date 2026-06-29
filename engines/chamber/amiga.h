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

#ifndef CHAMBER_AMIGA_H
#define CHAMBER_AMIGA_H

#include "chamber/renderer.h"

namespace Chamber {

class AmigaRenderer : public EGARenderer {
public:
	void switchToGraphicsMode() override;
	void colorSelect(byte csel) override;     // csel is the real palette index
	void selectCursor(uint16 num) override;   // SOURI.BIN: big-endian cursor planes
	void drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) override;
	void drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) override;
};

#define AMIGA_NUM_PALETTES 31

extern const byte *amiga_palette_table;

// Master's-Orbit full 16-colour palette (the one scene not using base+delta)
extern const byte *amiga_room_palette_table;

void amigaApplyPalette(byte index);

// Compose a room palette from the zone palette_index (base + per-index delta)
void amigaApplyRoomPalette(byte index);

// Slice the static resources out of the uncompressed KULT executable; returns 1 on success
int16 loadAmigaStaticData();

} // End of namespace Chamber

#endif // CHAMBER_AMIGA_H
