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

#ifndef CHEWY_EFFECT_H
#define CHEWY_EFFECT_H

namespace Chewy {

enum BlendMode {
	BLEND_NONE = 0,
	BLEND1 = 1,
	BLEND2 = 2,
	BLEND3 = 3,
	BLEND4 = 4
};

class effect {
public:
	effect();
	~effect();

	void blende1(byte *sram_speicher, byte *screen,
		byte *palette, int16 frames, uint8 mode, int16 col);
	void border(byte *screen, int16 val1, uint8 mode, int16 farbe);

	void rnd_blende(byte *rnd_speicher, byte *sram_speicher,
		byte *screen, byte *palette, int16 col, int16 skip_line);

	void spr_blende(byte *workpage, byte *ablage,
		byte *palette, int16 frames, int16 col);

private:

};

} // namespace Chewy

#endif
