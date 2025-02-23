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
#include "bagel/mfc/dc.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {

int CDC::GetDeviceCaps(int field) const {
	switch (field) {
	case HORZRES:
		return 640;
	case VERTRES:
		return 480;
	default:
		return 0;
	}
}

CPalette *CDC::SelectPalette(CPalette *pPalette, bool bForceBackground) {
	assert(!bForceBackground);
	_palette = *pPalette;
	return &_palette;
}

void CDC::RealizePalette() {
	g_system->getPaletteManager()->setPalette(_palette.data(), 0, _palette.size());
}

bool CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, uint32 dwRop) {
#ifdef TODO
	Graphics::Screen &screen = *g_engine->_screen;
	const Common::Rect srcRect(xSrc, ySrc, xSrc + nWidth, ySrc + nHeight);
	const Common::Rect destRect(x, y, x + nWidth, y + nHeight);

	screen.blitFrom(*pSrcDC, srcRect, destRect);
#endif
	return true;
}

} // namespace HodjNPodj
} // namespace Bagel
