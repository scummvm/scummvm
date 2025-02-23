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

#ifndef BAGEL_HODJNPODJ_MFC_DC_H
#define BAGEL_HODJNPODJ_MFC_DC_H

#include "graphics/palette.h"
#include "bagel/mfc/mfc_types.h"

namespace Bagel {
namespace MFC {

class CPalette {
private:
	Graphics::Palette _palette;
public:
	CPalette(uint size = PALETTE_COUNT) : _palette(size) {}

	void set(const byte *colors, int start, int count) {
		_palette.set(colors, start, count);
	}

	const byte *data() const {
		return _palette.data();
	}
	int getEntryCount() const {
		return _palette.size();
	}
	int size() const {
		return _palette.size();
	}
	void DeleteObject() {}
};

} // namespace MFC
} // namespace Bagel

#endif
