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

#ifndef HODJNPODJ_GFX_PALETTE_H
#define HODJNPODJ_GFX_PALETTE_H

#include "graphics/palette.h"
#include "bagel/boflib/palette.h"

namespace Bagel {
namespace HodjNPodj {

constexpr byte BLACK = 0;
constexpr byte WHITE = 255;
#define RED     PALETTERGB(255, 0, 255)
#define BLUE    PALETTERGB(0, 0, 255)
#define CYAN    PALETTERGB(0, 255, 255)
#define PURPLE  PALETTERGB(0x80, 0, 0x80)

class Palette {
private:
	Graphics::Palette _gamePalette;

protected:
	void loadInitialPalette();

public:
	Palette();
	virtual ~Palette() {}

	/**
	 * Set the palette to use, without changing _gamePalette
	 */
	virtual void setPalette(const byte *palette);
	virtual void setPalette(const Graphics::Palette &palette);

	/**
	 * Gets the currently active palette
	 */
	virtual Graphics::Palette getPalette() const;

	/**
	 * Set the palette to use, saving it in the internal _gamePalette
	 */
	virtual void loadPalette(const byte *palette);
	virtual void loadPalette(const Graphics::Palette &palette);

	/**
	 * Gets the closest palette index to a given rgb color
	 */
	virtual byte getPaletteIndex(uint32 color) const;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
